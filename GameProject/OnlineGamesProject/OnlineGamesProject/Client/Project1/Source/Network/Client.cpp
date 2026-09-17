#include "DxLib.h"
#include "../Input/Input.h"
#include "Client.h"
#include "NetworkCommonParam.h"
#include "../Input/InputString.h"
#include "../GameSetting/GameSetting.h"
#include <string>

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
	// 名前入力
	//==================================================

	if (m_NWState == NW_STATE_NAME_INPUT)
	{
		DrawFormatString(0,0,GetColor(255, 255, 255),"名前を入力してください");


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
			DrawShiritori();
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

	//==================================================
// ゲーム開始後
//==================================================

// 自分のターンか確認
	bool isMyTurn = strcmp(m_TurnPlayerName, m_MyName) == 0;

	// 相手のターンなら入力しない
	if (!isMyTurn)
	{
		if (Input::IsTriggerKey(KEY_ESCAPE))
		{
			Disconnect();
		}

		return;
	}


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


            // 1Pの名前
			strcpy_s(m_FirstPlayerName,NETWORK_USER_NAME_BUFFER_MAX,receiveData.playerNames[0]);

			// 2Pの名前
			strcpy_s(m_SecondPlayerName,NETWORK_USER_NAME_BUFFER_MAX,receiveData.playerNames[1]);


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

		else if (header.type ==Network::PACKET_SHIRITORI_HISTORY)
		{
			if (header.dataSize !=sizeof(ShiritoriData) * CHAT_LOG_MAX)
			{
				printf("Invalid ShiritoriHistory size.\n");

				continue;
			}


			ShiritoriData serializedData[CHAT_LOG_MAX] = {};


				NetWorkRecv(m_ServerHandle,serializedData,sizeof(serializedData));

				m_WordList.clear();

				for (const ShiritoriData& data :serializedData)
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

		else if (header.type ==Network::PACKET_SHIRITORI_DATA)
		{
			if (header.dataSize !=sizeof(ShiritoriData))
			{
				printf("Invalid ShiritoriData size.\n");

				continue;
			}


			ShiritoriData receiveData = {};


			NetWorkRecv(m_ServerHandle,&receiveData,sizeof(receiveData));


			//==================================================
			// ターン更新
			//==================================================

			m_TurnPlayerID = receiveData.turnPlayerID;


			//==================================================
			// 使用済み
			//==================================================

			if (receiveData.result == Network::SHIRITORI_ALREADY_USED)
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

	//画面サイズ
	const int screenWidth = 1600;
	const int screenHeight = 900;

	//==================================================
	// 色
	//==================================================
	const unsigned int white = GetColor(255, 255, 255);
	const unsigned int black = GetColor(0, 0, 0);
	const unsigned int gray = GetColor(235, 240, 245);
	const unsigned int blue = GetColor(70, 140, 220);
	const unsigned int darkGray = GetColor(100, 100, 100);


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


	// =========================================================
	// 相手
	// =========================================================

	DrawBox(30, 30,500, 120,white,TRUE);

	// =========================================================
    // 相手の名前
    // =========================================================

	const char* enemyName = nullptr;

	// 自分が1Pの場合、相手は2P
	if (strcmp(m_MyName, m_FirstPlayerName) == 0)
	{
		enemyName = m_SecondPlayerName;
	}
	// 自分が2Pの場合、相手は1P
	else if (strcmp(m_MyName, m_SecondPlayerName) == 0)
	{
		enemyName = m_FirstPlayerName;
	}

	if (enemyName != nullptr && strlen(enemyName) > 0)
	{
		DrawFormatString(60,45,black,"あいて : %s",enemyName);
	}
	else
	{
		DrawFormatString(60,45,black,"あいて : 待機中");
	}

	// =========================================================
	// 自分
	// =========================================================

	DrawBox(1100, 430,1570, 520,white,TRUE);

	DrawFormatString(1130,455,black,"じぶん : %s",m_MyName);

	// =========================================================
	// 現在の単語
	// =========================================================

	if (!m_WordList.empty())
	{
		const ShiritoriData& currentData = m_WordList.back();

		int wordWidth =GetDrawStringWidth(currentData.word,-1);

		// 白い楕円
		DrawOval(screenWidth / 2,320,250,80,white,TRUE);

		// 単語を中央寄せ
		int wordX = screenWidth / 2 - wordWidth / 2;

		DrawString(wordX,290,currentData.word,black);
	}

	//==================================================
	// 次に入力する文字
	//==================================================

	std::string guideText = "文字は「";
	guideText += m_StartChar;
	guideText += "」です";

	int guideWidth =GetDrawStringWidth(guideText.c_str(),-1);

	DrawString(screenWidth / 2 - guideWidth / 2,450,guideText.c_str(),black);


	// =========================================================
	// これまでのことば
	// =========================================================

	const int historyX = 1250;
	const int historyY = 80;

	DrawBox(historyX,historyY,1570,380,white,TRUE);

	DrawString(historyX + 25,historyY + 20,"これまでのことば",black);

	int drawY = historyY + 65;

	// 最新の単語から表示
	for (auto it = m_WordList.rbegin();it != m_WordList.rend();++it)
	{
		// ShiritoriDataなので word を表示する
		DrawString(historyX + 30,drawY,it->word,black);

		drawY += 35;

		// 最大7個程度
		if (drawY > historyY + 270)
		{
			break;
		}
	}

	//==================================================
    // 自分のターンか判定
    //==================================================

	bool isMyTurn = strcmp(m_TurnPlayerName, m_MyName) == 0;


	// =========================================================
	// 入力エリア
	// =========================================================

	const int inputX = 50;
	const int inputY = 700;
	const int inputWidth = 1100;
	const int inputHeight = 80;

	DrawBox(inputX,inputY,inputX + inputWidth,inputY + inputHeight,gray,TRUE);

	if (isMyTurn)
	{
		// InputStringの位置を変更
		if (m_MessageInput != nullptr)
		{
			m_MessageInput->SetPos(VGet(static_cast<float>(inputX + 20),static_cast<float>(inputY + 20),0.0f));

			m_MessageInput->Draw();
		}
	}
	else
	{
		DrawString(inputX + 25,inputY + 25,"相手の入力を待っています...",darkGray);
	}



	// =========================================================
	// ターン表示
	// =========================================================

	if (isMyTurn)
	{
		DrawString(50,810,"あなたのターンです",black);
	}
	else
	{
		DrawString(50,810,"相手のターンです",black);
	}
}
