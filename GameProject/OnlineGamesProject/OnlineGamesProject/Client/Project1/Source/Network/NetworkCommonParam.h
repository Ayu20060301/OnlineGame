#pragma once

namespace Network
{
	//==================================================
	// パケットの種類
	//==================================================

	enum PacketType
	{
		// Client → Server
		PACKET_CLIENT_CHAT,
		PACKET_CLIENT_SHIRITORI,

		// Server → Client
		PACKET_SERVER_DATA,
		PACKET_SHIRITORI_START,
		PACKET_SHIRITORI_DATA,
		PACKET_SHIRITORI_HISTORY,
		PACKET_CONNECTION_RESULT,
	};


	//==================================================
	// しりとり通信の種類
	//==================================================

	enum ShiritoriPacketType
	{
		SHIRITORI_WORD,
	};


	//==================================================
	// しりとり結果
	//==================================================

	enum ShiritoriResult
	{
		SHIRITORI_OK,
		SHIRITORI_WRONG_TURN,
		SHIRITORI_WRONG_WORD,
		SHIRITORI_ALREADY_USED,
		SHIRITORI_END_N,
		SHIRITORI_WRONG_START
	};


	//==================================================
	// サーバー接続結果
	//==================================================

	enum ConnectionResult
	{
		CONNECTION_OK,
		CONNECTION_FULL,
		CONNECTION_NAME_USED,
	};
}


//==================================================
// ネットワーク設定
//==================================================

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


//==================================================
// パケットヘッダー
//==================================================

struct PacketHeader
{
	// パケットの種類
	Network::PacketType type;

	// 後ろに続くデータサイズ
	int dataSize;
};


//==================================================
// Client → Server
//==================================================

struct ChatData
{
	// ユーザー名
	char name[NETWORK_USER_NAME_BUFFER_MAX];

	// メッセージ
	char message[NETWORK_WORD_BUFFER_MAX];
};


//==================================================
// Server → Client
//==================================================

struct ServerData
{
	// 接続人数
	int playerCount;

	// 現在のターンプレイヤーID
	int turnPlayerID;

	// プレイヤー名
	char playerNames[
		PLAYER_MAX
	][NETWORK_USER_NAME_BUFFER_MAX];

		// チャットログ
		ChatData chatData[CHAT_LOG_MAX];
};


//==================================================
// しりとりデータ
//==================================================

struct ShiritoriData
{
	// しりとり通信の種類
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


//==================================================
// しりとり開始データ
//==================================================

struct ShiritoriStartData
{
	// 最初の文字
	char startChar[NETWORK_WORD_BUFFER_MAX];
};


//==================================================
// 接続結果
//==================================================

struct ConnectionData
{
	Network::ConnectionResult result;
};