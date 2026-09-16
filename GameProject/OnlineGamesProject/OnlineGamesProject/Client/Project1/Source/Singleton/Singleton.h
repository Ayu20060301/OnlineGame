#pragma once
#include <memory>

//---定期的な部分を省略できるクラス---

template <typename T>
class Singleton
{
protected:
	//コンストラクタをprotectedにすると単体で生成することを禁止にできる
	Singleton() {};
	virtual ~Singleton() {};

public:
	static void CreateInstance() { if (!m_Instance) m_Instance = new T; }
	static T* GetInstance() { return m_Instance; }
	static void DeleteInstance() { if (m_Instance)delete m_Instance; m_Instance = nullptr; }

protected:
	static T* m_Instance;
};

template <typename T>
T* Singleton<T>::m_Instance = nullptr;
