#include "DxLib.h"
#include "GameSetting/GameSetting.h"
#include "GameApp/GameApp.h"

int WINAPI WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nShowCmd)
{
	// ウィンドウモードON
	ChangeWindowMode(TRUE);

	// 画面解像度の設定
	SetGraphMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_COLOR_DEPTH);

	// 多重起動を許可する
	SetDoubleStartValidFlag(TRUE);

	// バックグラウンドでも動作し続ける
	SetAlwaysRunFlag(TRUE);

	// DXライブラリ初期化
	if (DxLib_Init() == -1)
	{
		return -1;
	}

	// ウィンドウサイズ設定
	SetWindowSize(1600, 900);

	// 描画先を裏画面にする
	SetDrawScreen(DX_SCREEN_BACK);

	// GameApp生成
	GameApp gameApp;

	// ゲーム初期化
	if (!gameApp.Init())
	{
		DxLib_End();
		return -1;
	}

	// メインループ
	while (ProcessMessage() >= 0 && gameApp.IsRunning())
	{
		Sleep(1);

		// 画面クリア
		ClearDrawScreen();

		// 更新
		gameApp.Update();

		// 描画
		gameApp.Draw();

		// 画面フリップ
		ScreenFlip();
	}

	// GameApp終了処理
	gameApp.Fin();

	// DXライブラリ終了
	DxLib_End();

	return 0;
}
