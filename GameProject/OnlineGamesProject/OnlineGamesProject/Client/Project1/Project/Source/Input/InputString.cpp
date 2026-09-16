#include "InputString.h"

InputString::InputString()
{
	m_Handle = 0;
	m_Pos = {};
	memset(m_InputString, 0, sizeof(m_InputString));
}

void InputString::Start()
{
	m_Handle = MakeKeyInput(STRING_SIZE, FALSE, FALSE, FALSE);
	SetActiveKeyInput(m_Handle);
}

void InputString::Update()
{
	// DxLibの仕様（バグ？）で0文字確定→再Makeすると
	// Makeしたのにアクティブにならないことがあるのでその対策
	if (m_Handle > 0 && GetActiveKeyInput() == -1)
	{
		SetActiveKeyInput(m_Handle);
	}

	GetKeyInputString(m_InputString, m_Handle);
}

void InputString::Draw()
{
	if (m_Handle)
	{
		DrawKeyInputString((int)m_Pos.x, (int)m_Pos.y, m_Handle);
	}
}

void InputString::Fin()
{
	DeleteKeyInput(m_Handle);
	m_Handle = 0;
}
