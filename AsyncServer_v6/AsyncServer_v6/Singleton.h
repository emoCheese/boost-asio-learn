#pragma once

#include <memory>
#include <mutex>
#include <iostream>

/*
* ����ģ����
* �̳�ʹ��
*/

template<typename T>
class Singleton
{
protected:                    // �̳У�������Ҫʹ��
	Singleton() = default;
	Singleton(const Singleton<T>&) = delete;
	Singleton& operator=(const Singleton<T>&) = delete;

	static std::shared_ptr<T> _instance;
public:
	static std::shared_ptr<T> GetInstance()
	{
		// call_once �̰߳�ȫ����ֻ֤����һ�λص��������˴�Ϊlambda��ʵ����һ��
		static std::once_flag s_flag;   
		std::call_once(s_flag, [&]() {
			_instance = std::shared_ptr<T>(new T);
			} 
		);
		return _instance;
	}   
	void PrintAddress() { std::cout << _instance->get() << std::endl; }
	~Singleton()
	{
		std::cout << "singleton destruct" << std::endl;
	}
};

template<typename T>
std::shared_ptr<T> Singleton<T>::_instance = nullptr;
