#pragma once

// 入力ボタン定義
enum InputKey
{
	KEY_UP		= (1 << 0),	// 0x0000 0000 0000 0001
	KEY_DOWN	= (1 << 1),	// 0x0000 0000 0000 0010
	KEY_LEFT	= (1 << 2),	// 0x0000 0000 0000 0100
	KEY_RIGHT	= (1 << 3),	// 0x0000 0000 0000 1000
	KEY_Z		= (1 << 4),	// 0x0000 0000 0001 0000
	KEY_A		= (1 << 5),	// 0x0000 0000 0010 0000
	KEY_W		= (1 << 6),	// 0x0000 0000 0100 0000
	KEY_S		= (1 << 7),	// 0x0000 0000 1000 0000
	KEY_D		= (1 << 8),	// 0x0000 0001 0000 0000
	KEY_1		= (1 << 9),	// 0x0000 0010 0000 0000
	KEY_ENTER	= (1 << 10),// 0x0000 0100 0000 0000
	KEY_CTRL_L  = (1 << 11),// 0x0000 1000 0000 0000
	KEY_CTRL_R  = (1 << 12),// 0x0001 0000 0000 0000
	KEY_Q		= (1 << 13),// 0x0010 0000 0001 0000
};

class Input
{
public:
	// 関数のプロトタイプ宣言 
	static void Init();
	static void Update();
	static void Draw();
	static void Fin();

	static bool IsInputKey(InputKey key);		// キー入力判定
	static bool IsTriggerKey(InputKey key);	// キー押した瞬間判定

private:
	// 入力ビットフラグ
	static int m_InputState;
	// 前回の入力ビット
	static int m_PrevInputState;

};




