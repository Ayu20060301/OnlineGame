#pragma once
#include "DxLib.h"
#include "NetworkCommonParam.h"
#include <list>

class InputString;

// 通信の状態
enum NetworkState
{
	NW_STATE_NAME_INPUT,			// 名前入力
	NW_STATE_WAITING_CONNECTION,	// 接続待ち
	NW_STATE_MESSAGE_INPUT,			// メッセージ入力
	NW_STATE_CONNECTION_FAILED   //接続失敗
};

class Client
{
public:
	Client();
	~Client();

	void Init();
	void Update();
	void Draw();
	void Fin();

	void Connect();
	void Disconnect();

	void SetIPAddress(IPDATA address) { m_IPAddress = address; }

private:
	void UpdateNameInput();
	void UpdateWaitingConnection();
	void UpdateMessageInput();

	//受信処理
	void ReceiveData();

	//描画処理
	void DrawChat();
	void DrawShiritori();

private:
	int m_ServerHandle;
	ChatData m_SendChatData;
	ShiritoriData m_SendShiritoriData; //しりとり送信用
	std::list<ShiritoriData> m_WordList; //これまでに使われた単語
	InputString* m_UserNameInput;
	InputString* m_MessageInput;
	NetworkState m_NWState;
	IPDATA m_IPAddress;
	std::list<ChatData> m_ServerChatData;
	int m_PlayerCount;
	float m_LoadingAngle;
	int m_TurnPlayerID;
	char m_TurnPlayerName[NETWORK_USER_NAME_BUFFER_MAX];
	char m_StartChar[NETWORK_WORD_BUFFER_MAX];
	char m_ResultMessage[NETWORK_WORD_BUFFER_MAX];
};

