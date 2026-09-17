#pragma once

#include "DxLib.h"
#include "NetworkCommonParam.h"
#include <list>

class InputString;


// 通信状態
enum NetworkState
{
	NW_STATE_NAME_INPUT,
	NW_STATE_WAITING_CONNECTION,
	NW_STATE_MESSAGE_INPUT,
	NW_STATE_CONNECTION_FAILED
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

	void SetIPAddress(IPDATA address){m_IPAddress = address;}

private:

	// 更新処理
	void UpdateNameInput();
	void UpdateWaitingConnection();
	void UpdateMessageInput();

	// 受信
	void ReceiveData();

	// 描画
	void DrawChat();
	void DrawShiritori();

private:

	// サーバーハンドル
	int m_ServerHandle;

	// 名前登録用データ
	ChatData m_SendChatData;

	// しりとり送信用
	ShiritoriData m_SendShiritoriData;

	// しりとり履歴
	std::list<ShiritoriData> m_WordList;

	// 入力システム
	InputString* m_UserNameInput;
	InputString* m_MessageInput;

	// 通信状態
	NetworkState m_NWState;

	// IPアドレス
	IPDATA m_IPAddress;

	// チャット履歴
	std::list<ChatData> m_ServerChatData;

	// ローディング用
	float m_LoadingAngle;

	// 現在のターン
	int m_TurnPlayerID;

	// ターンプレイヤー名
	char m_TurnPlayerName[NETWORK_USER_NAME_BUFFER_MAX];

	// 最初の文字
	char m_StartChar[NETWORK_WORD_BUFFER_MAX];

	// 結果メッセージ
	char m_ResultMessage[NETWORK_WORD_BUFFER_MAX];

	char m_MyName[NETWORK_USER_NAME_BUFFER_MAX];

	//接続人数
	int m_PlayerCount;

	char m_FirstPlayerName[NETWORK_USER_NAME_BUFFER_MAX];
	char m_SecondPlayerName[NETWORK_USER_NAME_BUFFER_MAX];
};