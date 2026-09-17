#pragma once

#include "DxLib.h"
#include "NetworkCommonParam.h"
#include <list>

struct ClientData
{
	int handle;                              // ネットワークハンドル
	IPDATA ip;                               // 接続元IPアドレス
	int playerID;                            // プレイヤーID
	char name[NETWORK_USER_NAME_BUFFER_MAX]; // プレイヤー名
};

class Server
{
public:
	Server();
	~Server();

	void Init();
	void Update();
	void Draw();
	void Fin();

private:

	// ユーザー管理
	void AddUserData(int handle);
	void RemoveUserData(int handle);

	// 空いているプレイヤーIDを取得
	int GetFreePlayerID();

	// 受信処理
	void ReceiveData();

	// しりとり
	void StartShiritori();
	void SendStartShiritoriData();
	void SendShiritoriData();

	// 単語チェック
	bool IsUseWord(const char* word);

	// 全員にデータ送信
	void SendChatData();

private:

	// 接続中のクライアント
	std::list<ClientData> m_ClientData;

	// チャット履歴
	std::list<ChatData> m_ChatData;

	// しりとり履歴
	std::list<ShiritoriData> m_WordList;

	// 現在のターンプレイヤーID
	int m_TurnPlayerID;

	// しりとり開始文字
	char m_StartChar[NETWORK_WORD_BUFFER_MAX];

	// ゲーム開始済みか
	bool m_IsGameStarted;

	bool IsNameUsed(const char* name);

	bool IsCorrectStartChar(const char* word);

};