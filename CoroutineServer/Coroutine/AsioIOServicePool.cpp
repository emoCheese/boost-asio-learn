#include "AsioIOServicePool.h"

AsioIOServicePool::AsioIOServicePool(int thread_num)
	: _services(thread_num), _works(thread_num), _nextIOService(0)
{
	for (int i = 0; i < thread_num; ++i)
		_works[i] = std::unique_ptr<Work>(new Work(_services[i]));

	// 遍历多个ioService，创建多个线程，每个线程启动一个ioServcie
	for (int i = 0; i < thread_num; ++i)
		_threads.emplace_back([this, i]() {
		_services[i].run();
			});
} 

AsioIOServicePool::~AsioIOServicePool()
{
	std::cout << "AsioIOServicePool destructed" << std::endl;
}

// C++11 后直接返回静态对象不存在线程安全问题
AsioIOServicePool& AsioIOServicePool::GetInstance()
{
	static AsioIOServicePool instance(1);
	return instance;                                                  
}

// 以轮询的方式获取io_context
boost::asio::io_context& AsioIOServicePool::GetIOService()
{
	auto& service = _services[_nextIOService++];
	if (_nextIOService == _services.size())
		_nextIOService = 0;
	return service;
}

void AsioIOServicePool::Stop()
{
	// reset 重置 unique_ptr 使其失效，调用所指对象的析构函数
	for (auto& w : _works)
		w.reset();
	for (auto& t : _threads)
		t.join();
}
