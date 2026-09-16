#include "DxLib.h"
#include "BackGround.h"
#include "../GameSetting/GameSetting.h"

BackGround::BackGround()
{
	m_BGHandle = -1;
}

BackGround::~BackGround()
{
	Fin();
}

void BackGround::Load()
{
	m_BGHandle = LoadGraph("Data/Play/BG/BG.png");
}

void BackGround::Draw()
{
	//”wŒi‚ð•\Ž¦
	DrawExtendGraph(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, m_BGHandle, 0);
}

void BackGround::Fin()
{
	if (m_BGHandle != -1 && m_LogoHandle != -1)
	{
		DeleteGraph(m_BGHandle);
		m_BGHandle = -1;
	}
}
