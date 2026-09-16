#include "InputString.h"

InputString::InputString()
{
	//初期化
	m_Handle = 0;
	m_Pos = {};
	memset(m_InputString, 0, sizeof(m_InputString));
}

InputString::~InputString()
{
	Fin();
}

/// <summary>
/// 開始処理
/// </summary>
void InputString::Start()
{
	m_Handle = MakeKeyInput(STRING_SIZE, false, false, false);
	SetActiveKeyInput(m_Handle);
}

/// <summary>
/// 更新処理
/// </summary>
void InputString::Update()
{
	
	if (m_Handle > 0 && GetActiveKeyInput() == -1)
	{
		SetActiveKeyInput(m_Handle);
	}

	GetKeyInputString(m_InputString, m_Handle);
}

/// <summary>
/// 描画処理
/// </summary>
void InputString::Draw()
{
	if (m_Handle)
	{
		DrawKeyInputString((int)m_Pos.x, (int)m_Pos.y, m_Handle);
	}
}

/// <summary>
/// 終了処理
/// </summary>
void InputString::Fin()
{
	if (m_Handle > 0)
	{
		DeleteKeyInput(m_Handle);
		m_Handle = 0;
	}
}
