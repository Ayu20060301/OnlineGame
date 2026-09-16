#include "DxLib.h"
#include "Server.h"
#include "NetworkCommonParam.h"

Server::Server()
{
	m_ClientData = {};
	m_ChatData = {};
	m_WordList = {};

	m_TurnPlayerID = 0;

	// しりとりの最初の文字
	m_StartChar[0] = '\0';

	// ゲーム開始状態
	m_IsGameStarted = false;
}

Server::~Server()
{
	Fin();
}


/// <summary>
/// 初期化
/// </summary>
void Server::Init()
{
	// 接続待ち状態
	int success = PreparationListenNetWork(PORT_NUMBER);

	if (success == -1)
	{
		printf("PreparationListenNetWork failed!\n");
		printf("PORT = %d\n", PORT_NUMBER);
	}
	else
	{
		printf("Server listening...\n");
		printf("PORT = %d\n", PORT_NUMBER);
	}
}


/// <summary>
/// 更新
/// </summary>
void Server::Update()
{
	//-----------------------------
	// 新しい接続
	//-----------------------------
	int acceptHandle = GetNewAcceptNetWork();

	if (acceptHandle != -1)
	{
		AddUserData(acceptHandle);
	}

	//-----------------------------
	// 切断
	//-----------------------------
	int lostHandle = GetLostNetWork();

	if (lostHandle != -1)
	{
		RemoveUserData(lostHandle);
	}

	//-----------------------------
	// データ受信
	//-----------------------------
	ReceiveData();
}


/// <summary>
/// 描画
/// </summary>
void Server::Draw()
{
	// 接続人数は表示しない

	// ゲーム開始済みなら開始文字を表示
	if (m_IsGameStarted)
	{
		DrawFormatString(
			0,
			30,
			GetColor(0, 255, 255),
			"最初の文字 : %s",
			m_StartChar
		);
	}
}


/// <summary>
/// 終了処理
/// </summary>
void Server::Fin()
{
	m_ClientData.clear();
	m_ChatData.clear();
	m_WordList.clear();

	m_StartChar[0] = '\0';

	m_TurnPlayerID = 0;

	m_IsGameStarted = false;
}


/// <summary>
/// ユーザーデータ追加
/// </summary>
void Server::AddUserData(int handle)
{
	ClientData client = {};

	client.handle = handle;

	// 接続してきたPCのIPアドレス
	GetNetWorkIP(handle, &client.ip);

	// プレイヤーID
	client.playerID =
		static_cast<int>(m_ClientData.size());

	// 名前はまだ登録されていない
	client.name[0] = '\0';

	// クライアント追加
	m_ClientData.push_back(client);

	printf(
		"Player connected : ID = %d\n",
		client.playerID
	);

	printf(
		"Connected players = %d\n",
		static_cast<int>(m_ClientData.size())
	);

	// 最初のプレイヤーならターンを0にする
	if (m_ClientData.size() == 1)
	{
		m_TurnPlayerID = 0;
	}

	// まだゲーム開始前なので
	// しりとり履歴だけ送信
	SendShiritoriData();
}


/// <summary>
/// ユーザーデータ削除
/// </summary>
void Server::RemoveUserData(int handle)
{
	for (auto itr = m_ClientData.begin();
		itr != m_ClientData.end();
		++itr)
	{
		if ((*itr).handle == handle)
		{
			int removePlayerID =
				(*itr).playerID;

			printf(
				"Player disconnected : ID = %d\n",
				removePlayerID
			);

			// 削除
			m_ClientData.erase(itr);

			//-----------------------------
			// ゲームをリセット
			//-----------------------------
			m_IsGameStarted = false;

			m_WordList.clear();

			m_StartChar[0] = '\0';

			m_TurnPlayerID = 0;

			printf(
				"Shiritori reset.\n"
			);

			return;
		}
	}
}


/// <summary>
/// データ受信
/// </summary>
void Server::ReceiveData()
{
	bool isChatUpdate = false;
	bool isShiritoriUpdate = false;

	// 接続中の全クライアントを確認
	for (ClientData& client : m_ClientData)
	{
		int dataLength =
			GetNetWorkDataLength(client.handle);

		if (dataLength <= 0)
		{
			continue;
		}

		//==================================================
		// しりとりデータ
		//==================================================
		if (dataLength == sizeof(ShiritoriData))
		{
			ShiritoriData receiveData = {};

			NetWorkRecv(
				client.handle,
				&receiveData,
				sizeof(receiveData)
			);

			// ゲーム開始前なら受け付けない
			if (!m_IsGameStarted)
			{
				continue;
			}

			// 現在のターンではない
			if (client.playerID != m_TurnPlayerID)
			{
				receiveData.result =
					Network::SHIRITORI_WRONG_TURN;

				receiveData.turnPlayerID =
					m_TurnPlayerID;

				NetWorkSend(
					client.handle,
					&receiveData,
					sizeof(receiveData)
				);

				continue;
			}

			// プレイヤーIDをサーバー側で設定
			receiveData.playerID = client.playerID;

			// プレイヤー名をサーバー側で設定
			strcpy_s(
				receiveData.name,
				NETWORK_USER_NAME_BUFFER_MAX,
				client.name
			);

			//==================================================
			// 同じ単語かチェック
			//==================================================
			if (IsUseWord(receiveData.word))
			{
				receiveData.result =
					Network::SHIRITORI_ALREADY_USED;

				receiveData.turnPlayerID =
					m_TurnPlayerID;

				NetWorkSend(
					client.handle,
					&receiveData,
					sizeof(receiveData)
				);

				continue;
			}

			//==================================================
			// 正常
			//==================================================
			receiveData.result =
				Network::SHIRITORI_OK;

			// 次のプレイヤー
			int nextPlayerID =
				m_TurnPlayerID + 1;

			// 現在の接続人数を超えたら0
			if (nextPlayerID >=
				static_cast<int>(m_ClientData.size()))
			{
				nextPlayerID = 0;
			}

			m_TurnPlayerID =
				nextPlayerID;

			receiveData.turnPlayerID =
				m_TurnPlayerID;

			// 履歴追加
			m_WordList.push_back(receiveData);

			// 最大数を超えたら古いものを削除
			if (m_WordList.size() > CHAT_LOG_MAX)
			{
				m_WordList.pop_front();
			}

			isShiritoriUpdate = true;
		}

		//==================================================
		// チャットデータ
		//==================================================
		else if (dataLength == sizeof(ChatData))
		{
			ChatData receiveData = {};

			NetWorkRecv(
				client.handle,
				&receiveData,
				sizeof(receiveData)
			);

			//==================================================
			// 名前登録
			//==================================================
			if (strlen(receiveData.message) == 0)
			{
				strcpy_s(
					client.name,
					NETWORK_USER_NAME_BUFFER_MAX,
					receiveData.name
				);

				printf(
					"Player name registered : ID = %d, Name = %s\n",
					client.playerID,
					client.name
				);

				//==================================================
				// 2人以上そろったらゲーム開始
				//==================================================
				if (!m_IsGameStarted &&
					m_ClientData.size() >= 2)
				{
					StartShiritori();
				}

				isChatUpdate = true;

				continue;
			}

			//==================================================
			// 通常チャット
			//==================================================
			m_ChatData.push_back(receiveData);

			if (m_ChatData.size() > CHAT_LOG_MAX)
			{
				m_ChatData.pop_front();
			}

			isChatUpdate = true;
		}
	}

	//==================================================
	// チャット・プレイヤー情報更新
	//==================================================
	if (isChatUpdate)
	{
		SendChatData();
	}

	//==================================================
	// しりとり更新
	//==================================================
	if (isShiritoriUpdate)
	{
		SendShiritoriData();

		SendChatData();
	}
}


/// <summary>
/// しりとり履歴送信
/// </summary>
void Server::SendShiritoriData()
{
	ShiritoriData serialize[CHAT_LOG_MAX] = {};

	int i = 0;

	for (const ShiritoriData& data : m_WordList)
	{
		serialize[i] = data;

		i++;

		if (i >= CHAT_LOG_MAX)
		{
			break;
		}
	}

	// 全員に送信
	for (ClientData& client : m_ClientData)
	{
		NetWorkSend(
			client.handle,
			serialize,
			sizeof(serialize)
		);
	}
}


/// <summary>
/// しりとり開始
/// </summary>
void Server::StartShiritori()
{
	// すでに開始していたら何もしない
	if (m_IsGameStarted)
	{
		return;
	}

	// 2人未満なら開始しない
	if (m_ClientData.size() < 2)
	{
		return;
	}

	//==================================================
	// 最初の文字候補
	//==================================================
	const char* startChars[] =
	{
		"あ", "い", "う", "え", "お",
		"か", "き", "く", "け", "こ",
		"さ", "し", "す", "せ", "そ",
		"た", "ち", "つ", "て", "と",
		"な", "に", "ぬ", "ね", "の",
		"は", "ひ", "ふ", "へ", "ほ",
		"ま", "み", "む", "め", "も",
		"や", "ゆ", "よ",
		"ら", "り", "る", "れ", "ろ",
		"わ"
	};

	// ランダム選択
	int index =
		GetRand(
			static_cast<int>(_countof(startChars)) - 1
		);

	// 最初の文字を保存
	strcpy_s(
		m_StartChar,
		NETWORK_WORD_BUFFER_MAX,
		startChars[index]
	);

	// 履歴をクリア
	m_WordList.clear();

	// ゲーム開始
	m_IsGameStarted = true;

	// 最初のターン
	m_TurnPlayerID = 0;

	printf(
		"=================================\n"
	);

	printf(
		"SHIRITORI START\n"
	);

	printf(
		"Start Char : %s\n",
		m_StartChar
	);

	printf(
		"Turn Player ID : %d\n",
		m_TurnPlayerID
	);

	printf(
		"=================================\n"
	);

	// 開始文字を全員に送信
	SendStartShiritoriData();

	// ターン情報を送信
	SendChatData();

	// 履歴も送信
	SendShiritoriData();
}


/// <summary>
/// しりとり開始情報送信
/// </summary>
void Server::SendStartShiritoriData()
{
	ShiritoriStartData startData = {};

	strcpy_s(
		startData.startChar,
		NETWORK_WORD_BUFFER_MAX,
		m_StartChar
	);

	for (ClientData& client : m_ClientData)
	{
		NetWorkSend(
			client.handle,
			&startData,
			sizeof(startData)
		);
	}
}


/// <summary>
/// 使用済み単語チェック
/// </summary>
bool Server::IsUseWord(const char* word)
{
	for (const ShiritoriData& data : m_WordList)
	{
		if (strcmp(data.word, word) == 0)
		{
			return true;
		}
	}

	return false;
}


/// <summary>
/// 全クライアントにデータ送信
/// </summary>
void Server::SendChatData()
{
	ServerData serialize = {};

	//==================================================
	// 現在のターン
	//==================================================
	serialize.turnPlayerID =
		m_TurnPlayerID;

	//==================================================
	// プレイヤー名
	//==================================================
	int playerIndex = 0;

	for (const ClientData& client : m_ClientData)
	{
		if (playerIndex >= 2)
		{
			break;
		}

		strcpy_s(
			serialize.playerNames[playerIndex],
			NETWORK_USER_NAME_BUFFER_MAX,
			client.name
		);

		playerIndex++;
	}

	//==================================================
	// チャットログ
	//==================================================
	int i = 0;

	for (const ChatData& data : m_ChatData)
	{
		if (i >= CHAT_LOG_MAX)
		{
			break;
		}

		serialize.chatData[i] = data;

		i++;
	}

	//==================================================
	// 全員に送信
	//==================================================
	for (ClientData& client : m_ClientData)
	{
		NetWorkSend(
			client.handle,
			&serialize,
			sizeof(serialize)
		);
	}
}