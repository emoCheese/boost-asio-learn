#include "AsioIOServicePool.h"

AsioIOServicePool::AsioIOServicePool(int thread_num)
	: _services(thread_num), _works(thread_num), _nextIOService(0)
{
	for (int i = 0; i < thread_num; ++i)
		_works[i] = std::unique_ptr<Work>(new Work(_services[i]));

	// �������ioService����������̣߳�ÿ���߳�����һ��ioServcie
	for (int i = 0; i < thread_num; ++i)
		_threads.emplace_back([this, i]() {
		_services[i].run();
			});
} 

AsioIOServicePool::~AsioIOServicePool()
{
	std::cout << "AsioIOServicePool destructed" << std::endl;
}

// C++11 ��ֱ�ӷ��ؾ�̬���󲻴����̰߳�ȫ����
AsioIOServicePool& AsioIOServicePool::GetInstance()
{
	static AsioIOServicePool instance(1);
	return instance;                                                  
}

// ����ѯ�ķ�ʽ��ȡio_context
boost::asio::io_context& AsioIOServicePool::GetIOService()
{
	auto& service = _services[_nextIOService++];
	if (_nextIOService == _services.size())
		_nextIOService = 0;
	return service;
}

void AsioIOServicePool::Stop()
{
	// reset ���� unique_ptr ʹ��ʧЧ��������ָ�������������
	for (auto& w : _works)
		w.reset();
	for (auto& t : _threads)
		t.join();
}
