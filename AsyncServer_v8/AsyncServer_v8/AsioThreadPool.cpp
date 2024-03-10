#include "AsioThreadPool.h"

AsioThreadPool::AsioThreadPool(int thread_nums)
	: _work_ptr(new boost::asio::io_context::work(_service))
{
	for (int i = 0; i < thread_nums; ++i)
	{
		_threads.emplace_back([this]() {
			_service.run();
			});
	}
}

AsioThreadPool::~AsioThreadPool()
{
	std::cout << "AsioThreadPool destructed" << std::endl;
}

boost::asio::io_context& AsioThreadPool::GetIOService() { return _service; }

void AsioThreadPool::Stop()
{
	_work_ptr.reset();  // 重置指针，使其失效
	for (auto& t : _threads)
		t.join();
}
