#include "DxLib.h"
#include "Server.h"
#include "NetworkCommonParam.h"


Server::Server()
{
	m_ClientData.clear();
	m_ChatData.clear();
	m_WordList.clear();

	m_TurnPlayerID = 0;

	m_StartChar[0] = '\0';

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
	//==================================================
	// 新しい接続
	//==================================================

	int acceptHandle = GetNewAcceptNetWork();

	if (acceptHandle != -1)
	{
		AddUserData(acceptHandle);
	}


	//==================================================
	// 切断
	//==================================================

	int lostHandle = GetLostNetWork();

	if (lostHandle != -1)
	{
		RemoveUserData(lostHandle);
	}


	//==================================================
	// データ受信
	//==================================================

	ReceiveData();
}


/// <summary>
/// 描画
/// </summary>
void Server::Draw()
{
	//==================================================
	// 接続人数
	//==================================================

	DrawFormatString(0,0,GetColor(255, 255, 255),"接続人数 : %d / %d",static_cast<int>(m_ClientData.size()),PLAYER_MAX);


	//==================================================
	// ゲーム状態
	//==================================================

	if (m_IsGameStarted)
	{
		DrawFormatString(0,30,GetColor(0, 255, 255),"ゲーム開始");

		DrawFormatString(0,60,GetColor(0, 255, 255),"最初の文字 : %s",m_StartChar);

		DrawFormatString(0,90,GetColor(255, 255, 0),"ターンプレイヤー : %d",m_TurnPlayerID);
	}
	else
	{
		DrawFormatString(0,30,GetColor(255, 255, 0),"ゲーム開始待ち");
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
/// 空いているプレイヤーIDを取得
/// </summary>
int Server::GetFreePlayerID()
{
	for (int id = 0; id < PLAYER_MAX; id++)
	{
		bool isUsed = false;

		for (const ClientData& client : m_ClientData)
		{
			if (client.playerID == id)
			{
				isUsed = true;
				break;
			}
		}

		if (!isUsed)
		{
			return id;
		}
	}

	return -1;
}


/// <summary>
/// ユーザーデータ追加
/// </summary>
void Server::AddUserData(int handle)
{
	//==================================================
	// 最大人数チェック
	//==================================================

	if (m_ClientData.size() >= PLAYER_MAX)
	{
		printf("Server is full.\n");

		ConnectionData data = {};

		data.result =
			Network::CONNECTION_FULL;

		// ヘッダー
		PacketHeader header = {};

		header.type =
			Network::PACKET_CONNECTION_RESULT;

		header.dataSize =
			sizeof(ConnectionData);


		NetWorkSend(handle,&header,sizeof(header));

		NetWorkSend(handle,&data,sizeof(data));

		CloseNetWork(handle);

		return;
	}


	//==================================================
	// ClientData作成
	//==================================================

	ClientData client = {};

	client.handle = handle;

	GetNetWorkIP(handle,&client.ip);

	//==================================================
	// PlayerID取得
	//==================================================

	client.playerID = GetFreePlayerID();

	if (client.playerID == -1)
	{
		CloseNetWork(handle);
		return;
	}


	//==================================================
	// 名前初期化
	//==================================================

	client.name[0] = '\0';


	//==================================================
	// クライアント追加
	//==================================================

	m_ClientData.push_back(client);


	printf("Player connected : ID = %d\n",client.playerID);

	printf("Connected players = %d / %d\n",static_cast<int>(m_ClientData.size()),PLAYER_MAX);

	//==================================================
	// 最初のプレイヤー
	//==================================================

	if (m_ClientData.size() == 1)
	{
		m_TurnPlayerID =
		client.playerID;
	}


	//==================================================
	// 現在の状態を送信
	//==================================================

	SendChatData();

	SendShiritoriData();
}


/// <summary>
/// ユーザーデータ削除
/// </summary>
void Server::RemoveUserData(int handle)
{
	for (auto itr = m_ClientData.begin();itr != m_ClientData.end();++itr)
	{
		if ((*itr).handle == handle)
		{
			int removePlayerID = (*itr).playerID;


			printf("Player disconnected : ID = %d\n",removePlayerID);

			//==================================================
			// 削除
			//==================================================

			m_ClientData.erase(itr);


			//==================================================
			// ゲームリセット
			//==================================================

			m_IsGameStarted = false;

			m_WordList.clear();

			m_StartChar[0] = '\0';

			m_TurnPlayerID = 0;


			printf("Shiritori reset.\n");

			printf("Connected players = %d / %d\n",static_cast<int>(m_ClientData.size()),PLAYER_MAX);

			//==================================================
			// 残ったクライアントへ通知
			//==================================================

			SendChatData();

			SendShiritoriData();

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


	for (ClientData& client : m_ClientData)
	{
		int dataLength = GetNetWorkDataLength(client.handle);

		//==================================================
		// データなし
		//==================================================

		if (dataLength <= 0) continue;
		
		//==================================================
		// ヘッダーサイズ未満
		//==================================================

		if (dataLength < sizeof(PacketHeader)) continue;
		
		//==================================================
		// パケットヘッダー受信
		//==================================================

		PacketHeader header = {};

		NetWorkRecv(client.handle,&header,sizeof(header));


		printf("Server Receive : type = %d, size = %d\n",header.type,header.dataSize);

		//==================================================
		// Client → Server
		// ChatData
		//==================================================

		if (header.type ==Network::PACKET_CLIENT_CHAT)
		{
			if (header.dataSize != sizeof(ChatData))
			{
				printf("Invalid ChatData size.\n");

				// 不正データを読み捨て
				char dummy[1024];

				if (header.dataSize > 0 && header.dataSize <= sizeof(dummy))
				{
					NetWorkRecv(client.handle,dummy,header.dataSize);
				}
				continue;
			}


			ChatData receiveData = {};

			NetWorkRecv(client.handle,&receiveData,sizeof(receiveData));

			//==================================================
			// 名前登録
			//==================================================

			if (strlen(receiveData.message) == 0)
			{
				//名前重複チェック
				if (IsNameUsed(receiveData.name))
				{
					ConnectionData resultData = {};

					resultData.result = Network::CONNECTION_NAME_USED;

					PacketHeader resultHeader = {};

					resultHeader.type = Network::PACKET_CONNECTION_RESULT;

					resultHeader.dataSize = sizeof(ConnectionData);

					NetWorkSend(client.handle, &resultHeader, sizeof(resultHeader));

					//名前は登録しない
					continue;
				}

				//==================================================
			    // 名前登録
			    //==================================================
				strcpy_s(client.name,NETWORK_USER_NAME_BUFFER_MAX,receiveData.name);


				printf("Player name registered : ID = %d, Name = %s\n",client.playerID,client.name);

				//==================================================
				// 2人揃ったらゲーム開始
				//==================================================

				if (!m_IsGameStarted &&m_ClientData.size() >= PLAYER_MAX)
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


		//==================================================
		// Client → Server
		// しりとり
		//==================================================

		else if (header.type == Network::PACKET_CLIENT_SHIRITORI)
		{
			if (header.dataSize !=sizeof(ShiritoriData))
			{
				printf("Invalid ShiritoriData size.\n");

				continue;
			}


			ShiritoriData receiveData = {};

			NetWorkRecv(client.handle,&receiveData,sizeof(receiveData));


			//==================================================
			// ゲーム開始前
			//==================================================

			if (!m_IsGameStarted)
			{
				continue;
			}


			//==================================================
			// ターンチェック
			//==================================================

			if (client.playerID !=m_TurnPlayerID)
			{
				receiveData.result = Network::SHIRITORI_WRONG_TURN;

				receiveData.turnPlayerID = m_TurnPlayerID;


				NetWorkSend(client.handle,&receiveData,sizeof(receiveData));

				continue;
			}


			//==================================================
			// PlayerID設定
			//==================================================

			receiveData.playerID = client.playerID;


			//==================================================
			// 名前設定
			//==================================================

			strcpy_s(receiveData.name,NETWORK_USER_NAME_BUFFER_MAX,client.name);

			//==================================================
			// 使用済みチェック
			//==================================================

			if (IsUseWord(receiveData.word))
			{
				receiveData.result = Network::SHIRITORI_ALREADY_USED;

				receiveData.turnPlayerID = m_TurnPlayerID;


				// 結果パケット
				PacketHeader sendHeader = {};

				sendHeader.type = Network::PACKET_SHIRITORI_DATA;

				sendHeader.dataSize = sizeof(ShiritoriData);


				NetWorkSend(
					client.handle,
					&sendHeader,
					sizeof(sendHeader)
				);

				NetWorkSend(
					client.handle,
					&receiveData,
					sizeof(receiveData)
				);

				continue;
			}

			//==================================================
            // 語尾チェック
            //==================================================

			if (!IsCorrectStartChar(receiveData.word))
			{
				receiveData.result = Network::SHIRITORI_WRONG_START;

				receiveData.turnPlayerID = m_TurnPlayerID;

				// 結果パケット
				PacketHeader sendHeader = {};

				sendHeader.type = Network::PACKET_SHIRITORI_DATA;

				sendHeader.dataSize = sizeof(ShiritoriData);

				NetWorkSend(client.handle,&sendHeader,sizeof(sendHeader));

				NetWorkSend(client.handle,&receiveData,sizeof(receiveData));

				continue;
			}


			//==================================================
			// 正常
			//==================================================

			receiveData.result =
				Network::SHIRITORI_OK;


			//==================================================
			// 次のプレイヤー
			//==================================================

			int nextPlayerID =
				m_TurnPlayerID + 1;


			if (nextPlayerID >= PLAYER_MAX)
			{
				nextPlayerID = 0;
			}


			//==================================================
			// 次のプレイヤーが存在するか
			//==================================================

			bool nextPlayerExists = false;


			for (
				const ClientData& nextClient :
				m_ClientData
				)
			{
				if (
					nextClient.playerID ==
					nextPlayerID
					)
				{
					nextPlayerExists = true;
					break;
				}
			}


			//==================================================
			// 存在しない場合
			//==================================================

			if (!nextPlayerExists)
			{
				for (
					const ClientData& nextClient :
					m_ClientData
					)
				{
					if (
						nextClient.playerID !=
						m_TurnPlayerID
						)
					{
						nextPlayerID =
							nextClient.playerID;

						break;
					}
				}
			}


			//==================================================
			// ターン更新
			//==================================================

			m_TurnPlayerID = nextPlayerID;

			receiveData.turnPlayerID =m_TurnPlayerID;


			//==================================================
			// 履歴追加
			//==================================================

			m_WordList.push_back(receiveData);


			if (m_WordList.size() > CHAT_LOG_MAX)
			{
				m_WordList.pop_front();
			}


			isShiritoriUpdate = true;
		}
	}


	//==================================================
	// 更新情報送信
	//==================================================

	if (isChatUpdate)
	{
		SendChatData();
	}


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


	for (const ShiritoriData& data :
		m_WordList)
	{
		serialize[i] = data;

		i++;

		if (i >= CHAT_LOG_MAX)
		{
			break;
		}
	}


	//==================================================
	// ヘッダー
	//==================================================

	PacketHeader header = {};

	header.type = Network::PACKET_SHIRITORI_HISTORY;

	header.dataSize = sizeof(serialize);


	//==================================================
	// 全員へ送信
	//==================================================

	for (ClientData& client :
		m_ClientData)
	{
		NetWorkSend(
			client.handle,
			&header,
			sizeof(header)
		);

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
	//==================================================
	// すでに開始している
	//==================================================

	if (m_IsGameStarted) return;
	
	//==================================================
	// 2人未満
	//==================================================

	if (m_ClientData.size() < PLAYER_MAX) return;
	
	//==================================================
	// 最初の文字
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


	int index = GetRand(static_cast<int>(_countof(startChars)) - 1);


	strcpy_s(m_StartChar,NETWORK_WORD_BUFFER_MAX,startChars[index]);


	//==================================================
	// 初期化
	//==================================================

	m_WordList.clear();

	m_IsGameStarted = true;

	m_TurnPlayerID = GetRand(PLAYER_MAX - 1);

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
		"Player Count : %d / %d\n",
		static_cast<int>(m_ClientData.size()),
		PLAYER_MAX
	);

	printf(
		"=================================\n"
	);


	//==================================================
	// 開始文字送信
	//==================================================

	SendStartShiritoriData();


	//==================================================
	// 人数・ターン情報送信
	//==================================================

	SendChatData();


	//==================================================
	// 履歴送信
	//==================================================

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


	PacketHeader header = {};

	header.type =
		Network::PACKET_SHIRITORI_START;

	header.dataSize =
		sizeof(ShiritoriStartData);


	for (ClientData& client :
		m_ClientData)
	{
		NetWorkSend(
			client.handle,
			&header,
			sizeof(header)
		);

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
	for (const ShiritoriData& data :
		m_WordList)
	{
		if (strcmp(data.word, word) == 0)
		{
			return true;
		}
	}

	return false;
}


/// <summary>
/// 全クライアントにサーバー情報送信
/// </summary>
void Server::SendChatData()
{
	ServerData serialize = {};


	//==================================================
	// ターン
	//==================================================

	serialize.turnPlayerID = m_TurnPlayerID;


	//==================================================
	// 接続人数
	//==================================================

	serialize.playerCount =static_cast<int>(m_ClientData.size());


	//==================================================
	// プレイヤー名
	//==================================================

	for (const ClientData& client :
		m_ClientData)
	{
		if (
			client.playerID < 0 ||
			client.playerID >= PLAYER_MAX
			)
		{
			continue;
		}


		strcpy_s(
			serialize.playerNames[
				client.playerID
			],
			NETWORK_USER_NAME_BUFFER_MAX,
					client.name
					);
	}


	//==================================================
	// チャットログ
	//==================================================

	int i = 0;


	for (const ChatData& data :
		m_ChatData)
	{
		if (i >= CHAT_LOG_MAX)
		{
			break;
		}


		serialize.chatData[i] =
			data;

		i++;
	}


	//==================================================
	// ヘッダー
	//==================================================

	PacketHeader header = {};

	header.type =
		Network::PACKET_SERVER_DATA;

	header.dataSize =
		sizeof(ServerData);


	//==================================================
	// 全員へ送信
	//==================================================

	for (ClientData& client :
		m_ClientData)
	{
		NetWorkSend(
			client.handle,
			&header,
			sizeof(header)
		);

		NetWorkSend(
			client.handle,
			&serialize,
			sizeof(serialize)
		);
	}
}

/// <summary>
/// 名前が既に使用されているか
/// </summary>
/// <param name="name"></param>
/// <returns></returns>
bool Server::IsNameUsed(const char* name)
{
	for (const ClientData& client : m_ClientData)
	{
		//まだ名前登録されていないクライアントは無視
		if (strlen(client.name) == 0) continue;

		if (strcmp(client.name, name) == 0) return true;
	}
}

bool Server::IsCorrectStartChar(const char* word)
{
	// 単語が空
	if (word == nullptr || word[0] == '\0')	return false;
	
	//次に必要な文字
	const char* expectedChar = m_StartChar;

	// まだ単語が存在しない場合は、最初の文字を使用
	if (!m_WordList.empty())
	{
		const char* lastWord = m_WordList.back().word;

		size_t length = strlen(lastWord);

		if (length == 0) return false;

		// 最後の文字
		const char* lastChar = &lastWord[length - 2];

		//==================================================
		// 語尾が「ー」の場合
		//==================================================

		if (strncmp(lastChar, "ー", 2) == 0)
		{
			// 「ー」を含めて最後の2文字を次の開始文字にする
			if (length < 4)	return false;
			

			expectedChar = &lastWord[length - 4];
		}
		else
		{
			// 通常は最後の1文字
			expectedChar = lastChar;
		}
	}

	// 入力単語の先頭2バイトと比較
	return strncmp(word, expectedChar, 2) == 0;
}
