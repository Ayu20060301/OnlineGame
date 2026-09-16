#pragma once

namespace Network
{
	// しりとり通信の種類
	enum ShiritoriPacketType
	{
		SHIRITORI_START,
		SHIRITORI_WORD,
		SHIRITORI_RESULT,
		SHIRITORI_TURN,
		SHIRITORI_FINISH,
	};

	// しりとり結果
	enum ShiritoriResult
	{
		SHIRITORI_OK,
		SHIRITORI_WRONG_TURN,
		SHIRITORI_WRONG_WORD,
		SHIRITORI_ALREADY_USED,
		SHIRITORI_END_N,
	};
}


// ポート番号
constexpr int PORT_NUMBER = 50000;

constexpr int PLAYER_MAX = 2;

// 残す履歴の最大数
constexpr int CHAT_LOG_MAX = 10;

// ユーザー名の最大文字数
constexpr int NETWORK_USER_NAME_MAX = 10;

// ユーザー名のバッファサイズ
constexpr int NETWORK_USER_NAME_BUFFER_MAX =
NETWORK_USER_NAME_MAX + 1;

// 単語の最大バッファサイズ
constexpr int NETWORK_WORD_BUFFER_MAX = 64;


//--------------------------------------------------
// クライアント → サーバー
//--------------------------------------------------

struct ChatData
{
	// ユーザー名
	char name[NETWORK_USER_NAME_BUFFER_MAX];

	// メッセージ
	char message[NETWORK_WORD_BUFFER_MAX];
};


//--------------------------------------------------
// サーバー → クライアント
//--------------------------------------------------

struct ServerData
{
	// 現在のターンプレイヤーID
	int turnPlayerID;

	// プレイヤー名
	// ※人数制限ではなく、ターン表示用
	char playerNames[2][NETWORK_USER_NAME_BUFFER_MAX];

	// チャットログ
	ChatData chatData[CHAT_LOG_MAX];
};


//--------------------------------------------------
// しりとりデータ
//--------------------------------------------------

struct ShiritoriData
{
	// 通信の種類
	Network::ShiritoriPacketType type;

	// 判定結果
	Network::ShiritoriResult result;

	// 単語を送信したプレイヤー
	int playerID;

	// 次のターンプレイヤー
	int turnPlayerID;

	// ユーザー名
	char name[NETWORK_USER_NAME_BUFFER_MAX];

	// 入力された単語
	char word[NETWORK_WORD_BUFFER_MAX];
};


//--------------------------------------------------
// しりとり開始データ
//--------------------------------------------------

struct ShiritoriStartData
{
	char startChar[NETWORK_WORD_BUFFER_MAX];
};