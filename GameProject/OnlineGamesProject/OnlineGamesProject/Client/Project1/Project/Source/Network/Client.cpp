#include "DxLib.h"
#include "../Input/Input.h"
#include "Client.h"
#include "NetworkCommonParam.h"
#include "../Input/InputString.h"

Client::Client()
{
	m_ServerHandle = 0;
	m_NWState = NW_STATE_NAME_INPUT; //初期状態は名前入力
	m_IPAddress = {};
	m_SendChatData = {};
	m_UserNameInput = nullptr;
	m_MessageInput = nullptr;

	m_SendChatData = {};
	m_PlayerCount = 0;
	m_SendShiritoriData = {};
	m_LoadingAngle = 0.0f;

	//現在のターン
	m_TurnPlayerID = 0;
	m_TurnPlayerName[0] = '\0';

	//最初の文字
	m_StartChar[0] = '\0';

	//結果メッセージ
	m_ResultMessage[0] = '\0';
}

Client::~Client()
{
	Fin();
}

void Client::Init()
{
	// キー文字列入力システム設定
	m_UserNameInput = new InputString;
	m_MessageInput = new InputString;

	m_UserNameInput->SetPos(VGet(0.0f, 20.0f, 0.0f));
	m_MessageInput->SetPos(VGet(0.0f, 20.0f, 0.0f));

	// ユーザー名入力開始
	m_UserNameInput->Start();
}

void Client::Update()
{
	switch (m_NWState)
	{
		case NW_STATE_NAME_INPUT:			UpdateNameInput(); break;
		case NW_STATE_WAITING_CONNECTION:	UpdateWaitingConnection(); break;
		case NW_STATE_MESSAGE_INPUT:		UpdateMessageInput(); break;
		case NW_STATE_CONNECTION_FAILED:
			//Enterで名前入力に戻る
			if (Input::IsTriggerKey(KEY_ENTER))
			{
				m_NWState = NW_STATE_NAME_INPUT;
				m_UserNameInput->Start();
			}
			break;
	}
}

void Client::Draw()
{
	//接続人数
	DrawFormatString(800, 0, GetColor(255, 255, 255), "接続人数 : %d人 / %d人", m_PlayerCount, PLAYER_MAX);

	if (m_NWState == NW_STATE_NAME_INPUT)
	{
		DrawFormatString(0, 0, GetColor(255, 255, 255), "名前を入力");
	}
	else if (m_NWState == NW_STATE_CONNECTION_FAILED)
	{
		DrawFormatString(650, 400, GetColor(255, 255, 255), "接続に失敗しました");
		DrawFormatString(650, 480, GetColor(255, 255, 255), "Enterでもどる");
	}
	else if (m_NWState == NW_STATE_MESSAGE_INPUT)
	{
		DrawShiritori();
		DrawChat();
	}

	m_UserNameInput->Draw();
	m_MessageInput->Draw();
}

void Client::Fin()
{
	// 状態が接続中以降であれば切断
	if (m_NWState >= NW_STATE_WAITING_CONNECTION)
	{
		Disconnect();
	}

	// 入力システム削除
	delete m_UserNameInput;
	m_UserNameInput = nullptr;

	delete m_MessageInput;
	m_MessageInput = nullptr;
}

/// <summary>
/// サーバーに接続する
/// </summary>
void Client::Connect()
{
	// 指定したIPアドレスの端末に接続
	m_ServerHandle = ConnectNetWork(m_IPAddress, PORT_NUMBER);

	// ハンドルが-1なら接続できてない
	if (m_ServerHandle == -1)
	{
		// 名前入力に戻る
		m_UserNameInput->Start();
	}
	else
	{
		// 接続待ちへ
		m_NWState = NW_STATE_WAITING_CONNECTION;
	}
}

/// <summary>
/// サーバーから切断する
/// </summary>
void Client::Disconnect()
{
	// 切断
	CloseNetWork(m_ServerHandle);
	m_ServerHandle = 0;
	m_NWState = NW_STATE_NAME_INPUT;

	// メッセージ入力終了
	m_MessageInput->Fin();
	// ユーザー名入力開始
	m_UserNameInput->Start();

	//最初の文字をクリア
	m_StartChar[0] = '\0';

	//結果メッセージをクリア
	m_ResultMessage[0] = '\0';
}

/// <summary>
/// 切断中の更新処理
/// </summary>
void Client::UpdateNameInput()
{
	// ユーザー名入力更新
	m_UserNameInput->Update();

	// Enterで接続
	if (Input::IsTriggerKey(KEY_ENTER))
	{
		// 入力した名前を取得
		const char* name = m_UserNameInput->GetInputString();

		// 文字数チェック
		int nameLen = (int)strlen(name);
		if (nameLen > 0)
		{
			// ユーザー名をチャットデータに記録
			strcpy_s(m_SendChatData.name, NETWORK_USER_NAME_BUFFER_MAX, name);

			// ユーザー名入力終了
			m_UserNameInput->Fin();

			// 接続
			Connect();
		}
	}
}

/// <summary>
/// 接続待機中の更新処理
/// </summary>
void Client::UpdateWaitingConnection()
{
	// 接続できたかチェック
	if (GetNetWorkAcceptState(m_ServerHandle))
	{
		// 接続完了
		m_NWState = NW_STATE_MESSAGE_INPUT;

		// メッセージ入力開始
		m_MessageInput->Start();
	}
}

/// <summary>
/// 接続中の更新処理
/// </summary>
void Client::UpdateMessageInput()
{
	// メッセージ入力更新
	m_MessageInput->Update();

	// Enterキーでユーザー名とメッセージをサーバーに送信
	if (Input::IsTriggerKey(KEY_ENTER))
	{
		// 入力メッセージを取得
		const char* word = m_MessageInput->GetInputString();

		// 文字数チェック
		int messageLen = (int)strlen(word);

		if (messageLen > 0)
		{
			//結果メッセージをクリア
			m_ResultMessage[0] = '\0';

			//しりとりデータを設定
			m_SendShiritoriData = {};

			m_SendShiritoriData.type = Network::SHIRITORI_WORD;

			//プレイヤーIDはサーバー側で設定する
			m_SendShiritoriData.playerID = 0;

			// メッセージをチャットデータに設定
			strcpy_s(m_SendShiritoriData.word, NETWORK_WORD_BUFFER_MAX, word);

			// サーバーにチャットデータを送信
			NetWorkSend(m_ServerHandle, &m_SendShiritoriData, sizeof(m_SendShiritoriData));

			// メッセージをクリア
			m_MessageInput->Clear();
		}
	}

	// 受信処理
	ReceiveData();

	// Ctrl + Q で切断
	if ((Input::IsInputKey(KEY_CTRL_L) || Input::IsInputKey(KEY_CTRL_R)) && Input::IsTriggerKey(KEY_Q))
	{
		// 切断
		Disconnect();
	}
}

/// <summary>
/// サーバーから送られるデータを受信する
/// </summary>
void Client::ReceiveData()
{
	//サーバーから送られたデータのサイズを取得
	int dataLength = GetNetWorkDataLength(m_ServerHandle);

	//データが送られてきたかチェック
	if (dataLength <= 0) return;

	//チャットデータ
	if (dataLength == sizeof(ServerData))
	{
		ServerData receiveData = {};

		NetWorkRecv(m_ServerHandle, &receiveData, sizeof(receiveData));

		//接続人数を更新
		m_PlayerCount = receiveData.playerCount;

		//現在のターンプレイヤーIDを更新
		m_TurnPlayerID = receiveData.turnPlayerID;

		//チャットログを更新
		m_ServerChatData.clear();

		for (const ChatData& data : receiveData.chatData)
		{
			if (strlen(data.message) > 0)
			{
				m_ServerChatData.push_back(data);
			}
		}
	}

	//-----------------------------
	//しりとり開始データ
	//-----------------------------
	else if (dataLength == sizeof(ShiritoriStartData))
	{
		ShiritoriStartData receiveData = {};

		NetWorkRecv(m_ServerHandle, &receiveData, sizeof(receiveData));

		//最初の文字を保存
		strcpy_s(m_StartChar, NETWORK_WORD_BUFFER_MAX, receiveData.startChar);
	}

	//-----------------------------
	//しりとり履歴
	//-----------------------------
	else if (dataLength == sizeof(ShiritoriData) * CHAT_LOG_MAX)
	{
		ShiritoriData serializedData[CHAT_LOG_MAX] = {};

		NetWorkRecv(m_ServerHandle, serializedData, sizeof(serializedData));

		//しりとり履歴を更新
		m_WordList.clear();

		for (const ShiritoriData& data : serializedData)
		{
			if (strlen(data.word) > 0)
			{
				m_WordList.push_back(data);
			}
		}
	}

	//-----------------------------
	//しりとり結果
	//----------------------------
	else if (dataLength == sizeof(ShiritoriData))
	{
		ShiritoriData receiveData = {};

		NetWorkRecv(m_ServerHandle, &receiveData, sizeof(receiveData));

		//同じ単語が入力された
		if (receiveData.result == Network::SHIRITORI_ALREADY_USED)
		{
			strcpy_s(m_ResultMessage, NETWORK_WORD_BUFFER_MAX, "その単語は既に使われています");

			//入力欄をクリア
			m_MessageInput->Clear();
		}
	}

}

void Client::DrawChat()
{
	int raw = 0;
	for (ChatData data : m_ServerChatData)
	{
		DrawFormatString(0, 40 + raw * 20, GetColor(255, 255, 255), "%s: %s", data.name, data.message);
		raw++;
	}
}

void Client::DrawShiritori()
{
	DrawFormatString(0, 40, GetColor(255, 255, 255), "しりとり");

	//------------------
	//最初の文字
	//------------------
	if (strlen(m_StartChar) > 0)
	{
		DrawFormatString(0, 65, GetColor(0, 255, 255), "最初の文字 : %s", m_StartChar);
	}

	//-----------------
	//現在のターンを表示
	//-----------------
	DrawFormatString(0, 90, GetColor(255, 255, 0), "%sさんの番です", m_TurnPlayerName);

	//---------------
	//しりとり履歴
	//---------------
	int y = 125;

	for (const ShiritoriData& data : m_WordList)
	{
		DrawFormatString(0, y, GetColor(255, 255, 255), "%s : %s", data.name, data.word);

		y += 30;
	}


	//---------------------
	//結果メッセージ
	//---------------------
	if (strlen(m_ResultMessage) > 0)
	{
		DrawFormatString(0, 770, GetColor(255, 100, 100), "%s", m_ResultMessage);
	}

	DrawFormatString(0, 800, GetColor(255, 255, 255), "単語を入力してEnter");

	DrawFormatString(0, 840, GetColor(255, 255, 255), "Ctrl + Qで切断");
}

