#pragma once

#include <memory>
#include <mutex>
#include <iostream>

/*
* 单例模版类
* 继承使用
*/

template<typename T>
class Singleton
{
protected:                    // 继承，子类需要使用
	Singleton() = default;
	Singleton(const Singleton<T>&) = delete;
	Singleton& operator=(const Singleton<T>&) = delete;

	static std::shared_ptr<T> _instance;
public:
	static std::shared_ptr<T> GetInstance()
	{
		// call_once 线程安全，保证只调用一次回调函数，此处为lambda，实例化一次
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
