#pragma once

#include "DxLib.h"
#include "../Singleton/Singleton.h"
#include "../Network/Client.h"

class BackGround;

enum MainState
{
	MAIN_STATE_NONE,
	MAIN_STATE_SELECT_MODE,
	MAIN_STATE_SET_IP,
	MAIN_STATE_CHAT,
	MAIN_STATE_NAME_INPUT,
};


class GameApp : public Singleton<GameApp>
{
public:
	GameApp();
	~GameApp();

	// 初期化
	bool Init();

	//ロード
	void Load();

	// 更新
	void Update();

	// 描画
	void Draw();

	// 終了処理
	void Fin();

	void UpdateSelectMode();

	bool IsRunning() const { return m_IsRunning; }

private:
	Client* m_Client;

	BackGround* m_BackGround;

	MainState m_State;

	//選択中の項目
	int m_SelectIndex;

	bool m_IsRunning;

	int m_LogoHandle;

};
