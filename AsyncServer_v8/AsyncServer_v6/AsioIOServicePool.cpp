#include "AsioIOServicePool.h"


AsioIOServicePool::AsioIOServicePool(std::size_t size)
	: _io_services(size), _works(size), _nextIOServcie(0)    
{
	for (std::size_t i = 0; i < size; i++)
	{
		_works[i] = std::unique_ptr<Work>(new Work(_io_services[i]));
	}

	// 遍历 _io_service加入线程 并启动线程
	for (std::size_t i = 0; i < size; i++)
	{
		// C++11 起，只要创建了thread，thread就开始运行了
		_threads.emplace_back([this, i]() {
			_io_services[i].run();
			}
		);
	}
}

AsioIOServicePool::~AsioIOServicePool()
{
	std::cout << "AsioIOServicePool destruct" << std::endl;
}


boost::asio::io_context& AsioIOServicePool::GetIOService()
{
	auto& service = _io_services[_nextIOServcie++];
	if (_nextIOServcie == _io_services.size())
		_nextIOServcie = 0;
	return service;
}

void AsioIOServicePool::Stop()
{
	for (auto& work : _works)
		work.reset();
	for (auto& t : _threads)
		t.join();
}

