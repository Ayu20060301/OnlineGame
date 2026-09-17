#include "DxLib.h"
#include "../Input/Input.h"
#include "Client.h"
#include "NetworkCommonParam.h"
#include "../Input/InputString.h"

Client::Client()
{
	m_ServerHandle = 0;

	m_NWState = NW_STATE_NAME_INPUT;

	m_IPAddress = {};

	m_SendChatData = {};
	m_SendShiritoriData = {};

	m_UserNameInput = nullptr;
	m_MessageInput = nullptr;

	m_WordList.clear();
	m_ServerChatData.clear();

	m_LoadingAngle = 0.0f;

	m_TurnPlayerID = 0;
	m_TurnPlayerName[0] = '\0';

	m_StartChar[0] = '\0';

	m_ResultMessage[0] = '\0';

	m_MyName[0] = '\0';

	m_PlayerCount = 0;

	m_FirstPlayerName[0] = '\0';
	m_SecondPlayerName[0] = '\0';
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

	m_UserNameInput->SetPos(VGet(5.0f,55.0f,0.0f));

	m_MessageInput->SetPos(VGet(70.0f,740.0f,0.0f));

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

	//入力した名前を右上に追加
	if (strlen(m_MyName) > 0)
	{
		DrawFormatString(1350,20,GetColor(255, 255, 255),"名前 : %s",m_MyName);
	}

	//==================================================
	// 先行・後行
	//==================================================

	if (strlen(m_FirstPlayerName) > 0)
	{
		DrawFormatString(1200,60,GetColor(255, 255, 0),"先行 : %s",m_FirstPlayerName);
	}

	if (strlen(m_SecondPlayerName) > 0)
	{
		DrawFormatString(1200,90,GetColor(100, 200, 255),"後行 : %s",m_SecondPlayerName);
	}


	//==================================================
	// 名前入力
	//==================================================

	if (m_NWState == NW_STATE_NAME_INPUT)
	{
		DrawFormatString(0,0,GetColor(255, 255, 255),"名前を入力");


		DrawBox(0,50,600,100,GetColor(50, 50, 50),TRUE);

		DrawBox(0,50,600,100,GetColor(255, 255, 255),FALSE);

		m_UserNameInput->Draw();

		// 名前重複などのエラー
		if (strlen(m_ResultMessage) > 0)
		{
			DrawFormatString(0,120,GetColor(255, 100, 100),"%s",m_ResultMessage);
		}
	}

	//==================================================
	// 接続失敗
	//==================================================

	else if (m_NWState == NW_STATE_CONNECTION_FAILED)
	{
		m_MyName[0] = '\0';

		DrawFormatString(650,400,GetColor(255, 255, 255),"接続に失敗しました");

		DrawFormatString(650,480,GetColor(255, 255, 255),"Enterでもどる");
	}

	//==================================================
	// しりとり
	//==================================================
	else if (m_NWState ==NW_STATE_MESSAGE_INPUT)
	{
		DrawShiritori();

		DrawChat();


		// ゲーム開始後だけ入力欄
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
	m_ServerHandle =ConnectNetWork(m_IPAddress,PORT_NUMBER);

	if (m_ServerHandle == -1)
	{
		m_NWState = NW_STATE_CONNECTION_FAILED;
	}
	else
	{
		m_NWState = NW_STATE_WAITING_CONNECTION;
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


	if (m_MessageInput != nullptr)
	{
		m_MessageInput->Fin();
	}


	if (m_UserNameInput != nullptr)
	{
		m_UserNameInput->Start();
	}


	m_SendChatData = {};
	m_SendShiritoriData = {};

	m_StartChar[0] = '\0';

	m_ResultMessage[0] = '\0';

	m_TurnPlayerID = 0;

	m_TurnPlayerName[0] = '\0';

	m_PlayerCount = 0;

	m_MyName[0] = '\0';

	m_WordList.clear();

	m_ServerChatData.clear();
}


/// <summary>
/// 名前入力
/// </summary>
void Client::UpdateNameInput()
{
	m_UserNameInput->Update();


	if (Input::IsTriggerKey(KEY_ENTER))
	{
		const char* name = m_UserNameInput->GetInputString();


		int nameLen = static_cast<int>(strlen(name));

		if (nameLen > 0)
		{
			//エラー表示を消す
			m_ResultMessage[0] = '\0';

			//==================================================
			// 名前設定
			//==================================================

			strcpy_s(m_SendChatData.name,NETWORK_USER_NAME_BUFFER_MAX,name);

			strcpy_s(m_MyName,NETWORK_USER_NAME_BUFFER_MAX,name);

			m_SendChatData.message[0] = '\0';


			m_UserNameInput->Fin();

			//==================================================
			// 接続
			//==================================================

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
		m_NWState = NW_STATE_MESSAGE_INPUT;


		//==================================================
		// 名前を送信
		//==================================================

		PacketHeader header = {};

		header.type = Network::PACKET_CLIENT_CHAT;

		header.dataSize = sizeof(ChatData);


		NetWorkSend(m_ServerHandle,&header,sizeof(header));


		NetWorkSend(m_ServerHandle,&m_SendChatData,sizeof(m_SendChatData)
	);


		//==================================================
		// 入力開始
		//==================================================

		m_MessageInput->Start();


		//==================================================
		// サーバーから受信
		//==================================================

		ReceiveData();
	}
}


/// <summary>
/// しりとり更新
/// </summary>
void Client::UpdateMessageInput()
{
	//==================================================
	// サーバーから受信
	//==================================================

	ReceiveData();


	//==================================================
	// ゲーム開始前
	//==================================================

	if (strlen(m_StartChar) == 0)
	{
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


	if (Input::IsTriggerKey(KEY_ENTER))
	{
		const char* word = m_MessageInput->GetInputString();

		int messageLen =static_cast<int>(strlen(word));

		if (messageLen > 0)
		{
			m_ResultMessage[0] = '\0';


			m_SendShiritoriData = {};


			m_SendShiritoriData.type = Network::SHIRITORI_WORD;


			m_SendShiritoriData.playerID = 0;


			strcpy_s(m_SendShiritoriData.word,NETWORK_WORD_BUFFER_MAX,word);

			//==================================================
			// ヘッダー
			//==================================================

			PacketHeader header = {};

			header.type = Network::PACKET_CLIENT_SHIRITORI;

			header.dataSize = sizeof(ShiritoriData);

			NetWorkSend(m_ServerHandle,&header,sizeof(header));

			NetWorkSend(m_ServerHandle,&m_SendShiritoriData,sizeof(m_SendShiritoriData));

			m_MessageInput->Clear();
		}
	}


	//==================================================
	// Esc
	//==================================================

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
		int dataLength = GetNetWorkDataLength(m_ServerHandle);

		//==================================================
		// データなし
		//==================================================

		if (dataLength <= 0) break;
		
		//==================================================
		// ヘッダー未満
		//==================================================

		if (dataLength < sizeof(PacketHeader)) break;

		//==================================================
		// ヘッダー受信
		//==================================================

		PacketHeader header = {};

		NetWorkRecv(m_ServerHandle,&header,sizeof(header));

		printf("Client Receive : type = %d, size = %d\n",header.type,header.dataSize);


		//==================================================
		// ServerData
		//==================================================

		if (header.type ==Network::PACKET_SERVER_DATA)
		{
			if (header.dataSize !=sizeof(ServerData))
			{
				printf("Invalid ServerData size.\n");

				continue;
			}

			ServerData receiveData = {};


			NetWorkRecv(m_ServerHandle,&receiveData,sizeof(receiveData));

			//==================================================
			// 接続人数
			//==================================================

			m_PlayerCount = receiveData.playerCount;


			//==================================================
			// ターン
			//==================================================

			m_TurnPlayerID = receiveData.turnPlayerID;


			//==================================================
			// ターンプレイヤー名
			//==================================================

			m_TurnPlayerName[0] = '\0';


			if (m_TurnPlayerID >= 0 &&m_TurnPlayerID < PLAYER_MAX)
			{
				strcpy_s(m_TurnPlayerName,NETWORK_USER_NAME_BUFFER_MAX,receiveData.playerNames[m_TurnPlayerID]);
			}

			//==================================================
			// チャットログ
			//==================================================

			m_ServerChatData.clear();


			for (const ChatData& data :receiveData.chatData)
			{
				if (strlen(data.message) > 0)
				{
					m_ServerChatData.push_back(data);
				}
			}


			//==================================================
			// 2人未満ならゲーム待機状態
			//==================================================

			if (m_PlayerCount < PLAYER_MAX)
			{
				m_StartChar[0] = '\0';

				m_ResultMessage[0] = '\0';

				m_TurnPlayerID = 0;

				m_TurnPlayerName[0] = '\0';

				m_WordList.clear();
			}
		}


		//==================================================
		// しりとり開始
		//==================================================

		else if (header.type ==Network::PACKET_SHIRITORI_START)
		{
			if (header.dataSize !=sizeof(ShiritoriStartData))
			{
				printf("Invalid ShiritoriStartData size.\n");

				continue;
			}


			ShiritoriStartData receiveData = {};


			NetWorkRecv(m_ServerHandle,&receiveData,sizeof(receiveData));


			strcpy_s(m_StartChar,NETWORK_WORD_BUFFER_MAX,receiveData.startChar);


			printf("=================================\n");

			printf("Shiritori Start : %s\n",m_StartChar);

			printf("=================================\n");
		}


		//==================================================
		// しりとり履歴
		//==================================================

		else if (
			header.type ==
			Network::PACKET_SHIRITORI_HISTORY
			)
		{
			if (
				header.dataSize !=
				sizeof(ShiritoriData) * CHAT_LOG_MAX
				)
			{
				printf(
					"Invalid ShiritoriHistory size.\n"
				);

				continue;
			}


			ShiritoriData serializedData[
				CHAT_LOG_MAX
			] = {};


				NetWorkRecv(
					m_ServerHandle,
					serializedData,
					sizeof(serializedData)
				);


				m_WordList.clear();


				for (
					const ShiritoriData& data :
					serializedData
					)
				{
					if (strlen(data.word) > 0)
					{
						m_WordList.push_back(
							data
						);
					}
				}
		}


		//==================================================
		// しりとり結果
		//==================================================

		else if (header.type ==Network::PACKET_SHIRITORI_DATA)
		{
			if (header.dataSize !=sizeof(ShiritoriData))
			{
				printf("Invalid ShiritoriData size.\n");

				continue;
			}


			ShiritoriData receiveData = {};


			NetWorkRecv(
				m_ServerHandle,
				&receiveData,
				sizeof(receiveData)
			);


			//==================================================
			// ターン更新
			//==================================================

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
				strcpy_s(m_ResultMessage,NETWORK_WORD_BUFFER_MAX,"その単語は既に使われています");

				m_MessageInput->Clear();
			}


			//==================================================
			// ターン違い
			//==================================================

			else if (receiveData.result ==Network::SHIRITORI_WRONG_TURN)
			{
				strcpy_s(m_ResultMessage,NETWORK_WORD_BUFFER_MAX,"あなたの番ではありません");

				m_MessageInput->Clear();
			}

			//==================================================
            // 語尾が違う
            //==================================================

			else if (receiveData.result ==Network::SHIRITORI_WRONG_START)
			{
				strcpy_s(m_ResultMessage,NETWORK_WORD_BUFFER_MAX,"前の単語の最後の文字から始めてください");

				m_MessageInput->Clear();
			}
		}




		//==================================================
		// 接続結果
		//==================================================

		else if (header.type ==Network::PACKET_CONNECTION_RESULT)
		{
			if (header.dataSize !=sizeof(ConnectionData))
			{
				printf("Invalid ConnectionData size.\n");

				continue;
			}


			ConnectionData receiveData = {};


			NetWorkRecv(m_ServerHandle,&receiveData,sizeof(receiveData));


			if (receiveData.result ==Network::CONNECTION_FULL)
			{
				printf("Server is full.\n");

				strcpy_s(m_ResultMessage, NETWORK_WORD_BUFFER_MAX, "サーバーが満員です");;
			}
			else if (receiveData.result == Network::CONNECTION_NAME_USED)
			{
				printf("Name already used.\n");

				strcpy_s(m_ResultMessage,NETWORK_WORD_BUFFER_MAX,"その名前は既に使われています");

				//名前を再入力できる状態へ戻す
				m_NWState = NW_STATE_NAME_INPUT;

				m_UserNameInput->Start();
			}
		}

		//==================================================
		// 不明なパケット
		//==================================================

		else
		{
			printf("Unknown packet type : %d\n",header.type);

			// 不明なパケットを読み捨て
			if (header.dataSize > 0)
			{
				char dummy[1024];

				if (header.dataSize <=sizeof(dummy))
				{
					NetWorkRecv(m_ServerHandle,dummy,header.dataSize);
				}
			}
		}
	}
}


/// <summary>
/// チャット描画
/// </summary>
void Client::DrawChat()
{
	int row = 0;


	for (const ChatData& data :m_ServerChatData)
	{
		DrawFormatString(0,40 + row * 20,GetColor(255, 255, 255),"%s: %s",data.name,data.message);

		row++;
	}
}


/// <summary>
/// しりとり描画
/// </summary>
void Client::DrawShiritori()
{
	//==================================================
	// ゲーム開始前
	//==================================================

	if (strlen(m_StartChar) == 0)
	{
		DrawFormatString(0,250,GetColor(255, 255, 0),"ゲーム開始待ち");


		DrawFormatString(0,300,GetColor(255, 255, 255),"接続人数 : %d / %d",m_PlayerCount,PLAYER_MAX);


		DrawFormatString(0,350,GetColor(255, 255, 255),"2人揃うとゲームが開始されます");


		DrawFormatString(0,800,GetColor(255, 255, 255),"Escキーで切断");

		return;
	}


	//==================================================
	// ゲーム画面
	//==================================================

	DrawFormatString(0,0,GetColor(255, 255, 255),"しりとり");


	//==================================================
	// 接続人数
	//==================================================

	DrawFormatString(0,30,GetColor(255, 255, 255),"接続人数 : %d / %d",m_PlayerCount,PLAYER_MAX);


	//==================================================
	// 最初の文字
	//==================================================

	DrawFormatString(0,70,GetColor(0, 255, 255),"最初の文字 : %s",m_StartChar);

	//==================================================
	// ターン
	//==================================================

	if (strlen(m_TurnPlayerName) > 0)
	{
		DrawFormatString(0,100,GetColor(255, 255, 0),"%sさんの番です",m_TurnPlayerName);
	}


	//==================================================
	// 履歴
	//==================================================

	int y = 150;


	for (const ShiritoriData& data :m_WordList)
	{
		DrawFormatString(0,y,GetColor(255, 255, 255),"%s : %s",data.name,data.word);

		y += 30;
	}


	//==================================================
	// 結果
	//==================================================

	if (strlen(m_ResultMessage) > 0)
	{
		DrawFormatString(0,770,GetColor(255, 100, 100),"%s",m_ResultMessage);
	}


//==================================================
// 入力エリア
//==================================================

	DrawFormatString(50,650,GetColor(255, 255, 255),"入力");

	DrawBox(50,730,1000,780,GetColor(50, 50, 50),TRUE);

	DrawBox(50,730,1000,780,GetColor(255, 255, 255),FALSE);

	DrawFormatString(1020,745,GetColor(255, 255, 255),"Enter : 決定");

	DrawFormatString(50,820,GetColor(255, 255, 255),"Escキーで切断");
}
