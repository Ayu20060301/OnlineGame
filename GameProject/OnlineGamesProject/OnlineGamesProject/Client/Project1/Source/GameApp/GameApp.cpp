#include "GameApp.h"
#include "../Input/Input.h"
#include "../BG/BackGround.h"
#include "../Network/Client.h"

GameApp::GameApp()
{
	m_Client = nullptr;
	m_BackGround = new BackGround;
	m_State = MAIN_STATE_SELECT_MODE;
	m_SelectIndex = 0;
	m_IsRunning = true;
	m_LogoHandle = -1;
}

GameApp::~GameApp()
{
	Fin();
}

/// <summary>
/// 初期化
/// </summary>
bool GameApp::Init()
{
	// 入力初期化
	Input::Init();

	// 背景・ロゴなどを読み込む
	Load();

	return true;
}

/// <summary>
/// リソース読み込み
/// </summary>
void GameApp::Load()
{
	if (m_BackGround)
	{
		m_BackGround->Load();
	}

	m_LogoHandle = LoadGraph("Data/Play/Logo/Logo.png");
}

/// <summary>
/// 更新
/// </summary>
void GameApp::Update()
{

	Input::Update();

	switch (m_State)
	{
	case MAIN_STATE_SELECT_MODE:
		UpdateSelectMode();
		break;

	case MAIN_STATE_CHAT:
		if (m_Client)
		{
			m_Client->Update();
		}
		break;
	}
}

/// <summary>
/// 描画
/// </summary>
void GameApp::Draw()
{
	// 背景を最初に描画
	if (m_BackGround)
	{
		m_BackGround->Draw();
	}

	switch (m_State)
	{
	case MAIN_STATE_SELECT_MODE:

		// ロゴ
		if (m_LogoHandle != -1)
		{
			DrawExtendGraph(
				300,
				100,
				1300,
				500,
				m_LogoHandle,
				TRUE
			);
		}

		SetFontSize(32);

		DrawString(700,500,"オンラインでプレイ",GetColor(255, 255, 255));

		DrawString(700,660,"ゲームをやめる",GetColor(255, 255, 255));

		// 選択カーソル
		DrawString(650,500 + m_SelectIndex * 160,">>",GetColor(255, 255, 255));

		break;

	case MAIN_STATE_CHAT:

		if (m_Client)
		{
			m_Client->Draw();
		}

		break;
	}
}

/// <summary>
/// 終了処理
/// </summary>
void GameApp::Fin()
{
	if (m_Client != nullptr)
	{
		delete m_Client;
		m_Client = nullptr;
	}

	if (m_BackGround != nullptr)
	{
		delete m_BackGround;
		m_BackGround = nullptr;
	}

	if (m_LogoHandle != -1)
	{
		DeleteGraph(m_LogoHandle);
		m_LogoHandle = -1;
	}

	Input::Fin();
}

/// <summary>
/// モード選択更新
/// </summary>
void GameApp::UpdateSelectMode()
{
	// 上
	if (Input::IsTriggerKey(KEY_UP))
	{
		m_SelectIndex--;

		if (m_SelectIndex < 0)
		{
			m_SelectIndex = 1;
		}
	}

	// 下
	if (Input::IsTriggerKey(KEY_DOWN))
	{
		m_SelectIndex++;

		if (m_SelectIndex > 1)
		{
			m_SelectIndex = 0;
		}
	}

	// 決定
	if (Input::IsTriggerKey(KEY_RETURN))
	{
		switch (m_SelectIndex)
		{
		case 0:
			// オンラインでプレイ
			if (m_Client == nullptr)
			{
				m_Client = new Client();

				if (m_Client == nullptr)
				{
					return;
				}

				m_Client->Init();

				// IPアドレス設定
				IPDATA ipData;


				//-----------------
				///ここで設定します
				//-----------------
				ipData.d1 = 192;
				ipData.d2 = 168;
				ipData.d3 = 0;
				ipData.d4 = 54;

				m_Client->SetIPAddress(ipData);
			}

			m_State = MAIN_STATE_CHAT;
			break;

		case 1:
			// ゲームをやめる
			m_IsRunning = false;
			break;
		}
	}
}