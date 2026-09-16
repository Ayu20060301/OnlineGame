#pragma once
#include "DxLib.h"

constexpr int STRING_SIZE = 256;

class InputString
{
public:
	InputString();
	~InputString() { Fin(); }

public:
	void Start();
	void Update();
	void Draw();
	void Fin();
	void Clear() { SetKeyInputString("\0", m_Handle); }

	void SetPos(VECTOR pos) { m_Pos = pos; }

	const char* GetInputString() { return m_InputString; }

private:
	int m_Handle;
	VECTOR m_Pos;
	char m_InputString[STRING_SIZE];
};

