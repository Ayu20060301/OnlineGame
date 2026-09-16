#include "DxLib.h"
#include "Input/Input.h"
#include "Network/Client.h"

// クライアント用グローバル変数
Client* g_Client = nullptr;

// 関数のプロトタイプ宣言
void Update();			// 更新
void Draw();			// 描画

// プログラムは WinMain から始まります
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_  HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	// ウィンドウモードON
	ChangeWindowMode(TRUE);

	// 画面解像度の設定
	SetGraphMode(1600, 900, 32);

	// 多重起動を許可する
	SetDoubleStartValidFlag(TRUE);

	// バックグラウンドでも動作し続ける
	SetAlwaysRunFlag(TRUE);

	if (DxLib_Init() == -1)		// ＤＸライブラリ初期化処理
	{
		return -1;			// エラーが起きたら直ちに終了
	}

	// ウィンドウサイズ設定
	SetWindowSize(1600, 900);

	// 描画先を裏画面にする
	SetDrawScreen(DX_SCREEN_BACK);

	// 入力初期化
	Input::Init();

	// クライアント生成
	g_Client = new Client();
	g_Client->Init();

	// IPアドレスを設定
	IPDATA ipData;
	ipData.d1 = 10;
	ipData.d2 = 0;
	ipData.d3 = 80;
	ipData.d4 = 123;
	g_Client->SetIPAddress(ipData);

	// ゲームのメインループ
	while (ProcessMessage() >= 0)
	{
		Sleep(1);

		// 画面をクリア
		ClearDrawScreen();

		// 入力更新
		Input::Update();

		// 更新
		Update();

		// 描画
		Draw();

		// エスケープキーで終了
		if (CheckHitKey(KEY_INPUT_ESCAPE)) break;

		// 画面フリップ
		ScreenFlip();
	}

	// 入力終了
	Input::Fin();

	DxLib_End();				// ＤＸライブラリ使用の終了処理

	return 0;				// ソフトの終了 
}

void Update()
{
	// ネットワークマネージャー更新
	g_Client->Update();

}

void Draw()
{
	g_Client->Draw();
}
