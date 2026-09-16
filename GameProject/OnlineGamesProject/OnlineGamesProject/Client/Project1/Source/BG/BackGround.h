#pragma once

//”wŒiƒNƒ‰ƒX
class BackGround
{
public:
	BackGround();
	~BackGround();

public:
	void Load();
	void Draw();
	void Fin();
private:
	int m_BGHandle;
	int m_LogoHandle;
};
