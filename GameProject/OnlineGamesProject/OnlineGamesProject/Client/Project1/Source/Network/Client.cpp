#include "DxLib.h"
#include "../Input/Input.h"
#include "Client.h"
#include "NetworkCommonParam.h"
#include "../Input/InputString.h"


Client::Client()
{
	m_ServerHandle = 0;

	// 初期状態
	m_NWState = NW_STATE_NAME_INPUT;

	m_IPAddress = {};

	m_SendChatData = {};
	m_SendShiritoriData = {};

	m_UserNameInput = nullptr;
	m_MessageInput = nullptr;

	m_WordList.clear();
	m_ServerChatData.clear();

	m_LoadingAngle = 0.0f;

	// ターン
	m_TurnPlayerID = 0;
	m_TurnPlayerName[0] = '\0';

	// 最初の文字
	m_StartChar[0] = '\0';

	// 結果メッセージ
	m_ResultMessage[0] = '\0';

	m_PlayerCount = 0;
}


Client::~Client()
{
	Fin();
}


/// <summary>
/// 初期化
/// </summary>
void Client::Init()
{
	m_UserNameInput = new InputString;
	m_MessageInput = new InputString;

	m_UserNameInput->SetPos(
		VGet(5.0f, 35.0f, 0.0f)
	);

	m_MessageInput->SetPos(
		VGet(0.0f, 20.0f, 0.0f)
	);

	// 名前入力開始
	m_UserNameInput->Start();
}


/// <summary>
/// 更新
/// </summary>
void Client::Update()
{
	switch (m_NWState)
	{
	case NW_STATE_NAME_INPUT:
		UpdateNameInput();
		break;

	case NW_STATE_WAITING_CONNECTION:
		UpdateWaitingConnection();
		break;

	case NW_STATE_MESSAGE_INPUT:
		UpdateMessageInput();
		break;

	case NW_STATE_CONNECTION_FAILED:

		if (Input::IsTriggerKey(KEY_ENTER))
		{
			m_NWState = NW_STATE_NAME_INPUT;

			m_UserNameInput->Start();
		}

		break;
	}
}


/// <summary>
/// 描画
/// </summary>
void Client::Draw()
{
	//==================================================
	// 名前入力
	//==================================================
	if (m_NWState == NW_STATE_NAME_INPUT)
	{
		DrawFormatString(
			0,
			0,
			GetColor(255, 255, 255),
			"名前を入力"
		);
	}

	//==================================================
	// 接続失敗
	//==================================================
	else if (m_NWState == NW_STATE_CONNECTION_FAILED)
	{
		DrawFormatString(
			650,
			400,
			GetColor(255, 255, 255),
			"接続に失敗しました"
		);

		DrawFormatString(
			650,
			480,
			GetColor(255, 255, 255),
			"Enterでもどる"
		);
	}

	//==================================================
	// しりとり画面
	//==================================================
	else if (m_NWState == NW_STATE_MESSAGE_INPUT)
	{
		DrawShiritori();
		DrawChat();
	}

	//==================================================
	// 名前入力欄
	//==================================================
	if (m_NWState == NW_STATE_NAME_INPUT)
	{
		m_UserNameInput->Draw();
	}

	//==================================================
	// 単語入力欄
	//==================================================
	else if (m_NWState == NW_STATE_MESSAGE_INPUT)
	{
		// ゲーム開始後だけ入力欄を表示
		if (strlen(m_StartChar) > 0)
		{
			m_MessageInput->Draw();
		}
	}
}


/// <summary>
/// 終了
/// </summary>
void Client::Fin()
{
	if (m_NWState >= NW_STATE_WAITING_CONNECTION)
	{
		Disconnect();
	}

	delete m_UserNameInput;
	m_UserNameInput = nullptr;

	delete m_MessageInput;
	m_MessageInput = nullptr;
}


/// <summary>
/// サーバーへ接続
/// </summary>
void Client::Connect()
{
	m_ServerHandle =
		ConnectNetWork(
			m_IPAddress,
			PORT_NUMBER
		);

	// 接続失敗
	if (m_ServerHandle == -1)
	{
		m_NWState =
			NW_STATE_CONNECTION_FAILED;
	}
	else
	{
		m_NWState =
			NW_STATE_WAITING_CONNECTION;
	}
}


/// <summary>
/// 切断
/// </summary>
void Client::Disconnect()
{
	if (m_ServerHandle != 0)
	{
		CloseNetWork(m_ServerHandle);
	}

	m_ServerHandle = 0;

	m_NWState = NW_STATE_NAME_INPUT;

	// メッセージ入力終了
	if (m_MessageInput != nullptr)
	{
		m_MessageInput->Fin();
	}

	// 名前入力開始
	if (m_UserNameInput != nullptr)
	{
		m_UserNameInput->Start();
	}

	// データクリア
	m_SendChatData = {};
	m_SendShiritoriData = {};

	m_StartChar[0] = '\0';

	m_ResultMessage[0] = '\0';

	m_TurnPlayerID = 0;

	m_PlayerCount = 0;

	m_TurnPlayerName[0] = '\0';

	m_WordList.clear();

	m_ServerChatData.clear();
}


/// <summary>
/// 名前入力
/// </summary>
void Client::UpdateNameInput()
{
	m_UserNameInput->Update();

	// Enter
	if (Input::IsTriggerKey(KEY_ENTER))
	{
		const char* name =
			m_UserNameInput->GetInputString();

		int nameLen =
			(int)strlen(name);

		if (nameLen > 0)
		{
			// 名前保存
			strcpy_s(
				m_SendChatData.name,
				NETWORK_USER_NAME_BUFFER_MAX,
				name
			);

			// メッセージは空
			m_SendChatData.message[0] = '\0';

			// 入力終了
			m_UserNameInput->Fin();

			// 接続
			Connect();
		}
	}
}


/// <summary>
/// 接続待機
/// </summary>
void Client::UpdateWaitingConnection()
{
	if (GetNetWorkAcceptState(m_ServerHandle))
	{
		// しりとり画面へ
		m_NWState =
			NW_STATE_MESSAGE_INPUT;

		// メッセージ入力開始
		m_MessageInput->Start();

		//==================================================
		// 名前をサーバーへ送信
		//==================================================
		NetWorkSend(
			m_ServerHandle,
			&m_SendChatData,
			sizeof(m_SendChatData)
		);

		// サーバーから受信
		ReceiveData();
	}
}


/// <summary>
/// しりとり更新
/// </summary>
void Client::UpdateMessageInput()
{
	// サーバーから受信
	ReceiveData();

	//==================================================
	// ゲーム開始前
	//==================================================
	if (strlen(m_StartChar) == 0)
	{
		// Escで切断
		if (Input::IsTriggerKey(KEY_ESCAPE))
		{
			Disconnect();
		}

		return;
	}

	//==================================================
	// ゲーム開始後
	//==================================================

	m_MessageInput->Update();

	// Enter
	if (Input::IsTriggerKey(KEY_ENTER))
	{
		const char* word =
			m_MessageInput->GetInputString();

		int messageLen =
			(int)strlen(word);

		if (messageLen > 0)
		{
			// 結果メッセージクリア
			m_ResultMessage[0] = '\0';

			// データ初期化
			m_SendShiritoriData = {};

			// 通信タイプ
			m_SendShiritoriData.type =
				Network::SHIRITORI_WORD;

			// プレイヤーIDはサーバー側で設定
			m_SendShiritoriData.playerID = 0;

			// 単語
			strcpy_s(
				m_SendShiritoriData.word,
				NETWORK_WORD_BUFFER_MAX,
				word
			);

			// サーバーへ送信
			NetWorkSend(
				m_ServerHandle,
				&m_SendShiritoriData,
				sizeof(m_SendShiritoriData)
			);

			// 入力欄クリア
			m_MessageInput->Clear();
		}
	}

	// Escで切断
	if (Input::IsTriggerKey(KEY_ESCAPE))
	{
		Disconnect();
	}
}


/// <summary>
/// サーバーからデータ受信
/// </summary>
void Client::ReceiveData()
{
	while (true)
	{
		int dataLength =
			GetNetWorkDataLength(m_ServerHandle);

		if (dataLength <= 0)
		{
			break;
		}

		printf(
			"Client Receive : dataLength = %d\n",
			dataLength
		);

		//==================================================
		// ServerData
		//==================================================
		if (dataLength == sizeof(ServerData))
		{
			ServerData receiveData = {};

			NetWorkRecv(
				m_ServerHandle,
				&receiveData,
				sizeof(receiveData)
			);

			// 接続人数
			m_PlayerCount = receiveData.playerCount;

			// ターンID
			m_TurnPlayerID = receiveData.turnPlayerID;

			// ターンプレイヤー名
			m_TurnPlayerName[0] = '\0';

			if (m_TurnPlayerID >= 0 && m_TurnPlayerID < PLAYER_MAX)
			{
				strcpy_s(
					m_TurnPlayerName,
					NETWORK_USER_NAME_BUFFER_MAX,
					receiveData.playerNames[
						m_TurnPlayerID
					]
				);
			}

			// チャットログ
			m_ServerChatData.clear();

			for (const ChatData& data :
				receiveData.chatData)
			{
				if (strlen(data.message) > 0)
				{
					m_ServerChatData.push_back(data);
				}
			}
		}

		//==================================================
		// しりとり開始
		//==================================================
		else if (
			dataLength ==
			sizeof(ShiritoriStartData)
			)
		{
			ShiritoriStartData receiveData = {};

			NetWorkRecv(
				m_ServerHandle,
				&receiveData,
				sizeof(receiveData)
			);

			// 開始文字を保存
			strcpy_s(
				m_StartChar,
				NETWORK_WORD_BUFFER_MAX,
				receiveData.startChar
			);

			printf(
				"=================================\n"
			);

			printf(
				"Shiritori Start : %s\n",
				m_StartChar
			);

			printf(
				"=================================\n"
			);
		}

		//==================================================
		// しりとり履歴
		//==================================================
		else if (
			dataLength ==
			sizeof(ShiritoriData) * CHAT_LOG_MAX
			)
		{
			ShiritoriData serializedData[
				CHAT_LOG_MAX
			] = {};

				NetWorkRecv(
					m_ServerHandle,
					serializedData,
					sizeof(serializedData)
				);

				m_WordList.clear();

				for (const ShiritoriData& data :
					serializedData)
				{
					if (strlen(data.word) > 0)
					{
						m_WordList.push_back(data);
					}
				}
		}

		//==================================================
		// しりとり結果
		//==================================================
		else if (
			dataLength ==
			sizeof(ShiritoriData)
			)
		{
			ShiritoriData receiveData = {};

			NetWorkRecv(
				m_ServerHandle,
				&receiveData,
				sizeof(receiveData)
			);

			// ターン更新
			m_TurnPlayerID =
				receiveData.turnPlayerID;

			//==================================================
			// 使用済み
			//==================================================
			if (
				receiveData.result ==
				Network::SHIRITORI_ALREADY_USED
				)
			{
				strcpy_s(
					m_ResultMessage,
					NETWORK_WORD_BUFFER_MAX,
					"その単語は既に使われています"
				);

				m_MessageInput->Clear();
			}

			//==================================================
			// ターン違い
			//==================================================
			else if (
				receiveData.result ==
				Network::SHIRITORI_WRONG_TURN
				)
			{
				strcpy_s(
					m_ResultMessage,
					NETWORK_WORD_BUFFER_MAX,
					"あなたの番ではありません"
				);

				m_MessageInput->Clear();
			}
		}

		//==================================================
		// 不明
		//==================================================
		else
		{
			printf(
				"Unknown packet size : %d\n",
				dataLength
			);

			break;
		}
	}
}


/// <summary>
/// チャット描画
/// </summary>
void Client::DrawChat()
{
	int row = 0;

	for (ChatData data : m_ServerChatData)
	{
		DrawFormatString(
			0,
			40 + row * 20,
			GetColor(255, 255, 255),
			"%s: %s",
			data.name,
			data.message
		);

		row++;
	}
}


/// <summary>
/// しりとり描画
/// </summary>
void Client::DrawShiritori()
{
	// 接続人数表示
	DrawFormatString(
		0,
		250,
		GetColor(255, 255, 255),
		"接続人数 : %d / %d",
		m_PlayerCount,
		PLAYER_MAX
	);

	// ゲーム開始前
	if (strlen(m_StartChar) == 0)
	{
		DrawFormatString(
			0,
			300,
			GetColor(255, 255, 0),
			"他のプレイヤーの参加を待っています..."
		);

		DrawFormatString(
			0,
			340,
			GetColor(255, 255, 255),
			"2人以上集まるとゲームが開始されます"
		);

		DrawFormatString(
			0,
			800,
			GetColor(255, 255, 255),
			"プレイヤーが揃うまでお待ちください"
		);

		DrawFormatString(
			0,
			840,
			GetColor(255, 255, 255),
			"Escキーで切断"
		);

		return;

	}

	//==================================================
	// 最初の文字
	//==================================================
	DrawFormatString(
		0,
		65,
		GetColor(0, 255, 255),
		"最初の文字 : %s",
		m_StartChar
	);

	//==================================================
	// 現在のターン
	//==================================================
	if (strlen(m_TurnPlayerName) > 0)
	{
		DrawFormatString(
			0,
			90,
			GetColor(255, 255, 0),
			"%sさんの番です",
			m_TurnPlayerName
		);
	}

	//==================================================
	// しりとり履歴
	//==================================================
	int y = 125;

	for (const ShiritoriData& data :
		m_WordList)
	{
		DrawFormatString(
			0,
			y,
			GetColor(255, 255, 255),
			"%s : %s",
			data.name,
			data.word
		);

		y += 30;
	}

	//==================================================
	// 結果メッセージ
	//==================================================
	if (strlen(m_ResultMessage) > 0)
	{
		DrawFormatString(
			0,
			770,
			GetColor(255, 100, 100),
			"%s",
			m_ResultMessage
		);
	}

	//==================================================
	// 入力案内
	//==================================================
	DrawFormatString(
		0,
		800,
		GetColor(255, 255, 255),
		"単語を入力してEnter"
	);

	DrawFormatString(
		0,
		840,
		GetColor(255, 255, 255),
		"Escキーで切断"
	);
}
