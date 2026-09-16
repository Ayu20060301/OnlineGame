#include "DxLib.h"
#include "Network/Server.h"

//サーバー用グローバル変数
Server* g_Server = nullptr;


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
	//ウィンドウサイズ設定
	SetWindowSize(1600, 900);

	//描画先を裏画面にする
	SetDrawScreen(DX_SCREEN_BACK);

	//サーバー生成
	g_Server = new Server;
	g_Server->Init();

	//ゲームのメインループ
	while (ProcessMessage() >= 0)
	{
		Sleep(1);

		//画面をクリア
		ClearDrawScreen();

		// エスケープキーで終了
		if (CheckHitKey(KEY_INPUT_ESCAPE)) break;

		// ネットワーク系更新
		g_Server->Update();

		// ネットワーク系描画
		g_Server->Draw();

		DrawString(0, 880, "サーバー側", GetColor(255, 255, 255));

		// 画面フリップ
		ScreenFlip();
	}

	// サーバー削除
	g_Server->Fin();
	delete g_Server;

	DxLib_End();				// ＤＸライブラリ使用の終了処理

	return 0;				// ソフトの終了 
}