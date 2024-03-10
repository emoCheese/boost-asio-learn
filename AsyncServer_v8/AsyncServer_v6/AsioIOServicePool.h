#pragma once
#include <memory>
#include <thread>
#include <mutex>
#include <vector>
#include <iostream>

#include "boost/asio.hpp"

#include "Singleton.h"

class AsioIOServicePool : public Singleton<AsioIOServicePool>
{
	friend class Singleton<AsioIOServicePool>;
public:
	using IOService = boost::asio::io_context;
	using Work = boost::asio::io_context::work;
	using WorkPtr = std::unique_ptr<Work>;         // unqiue_ptr 使 work不能被拷贝

	~AsioIOServicePool();
	boost::asio::io_context& GetIOService();
	void Stop();
private:
	// 默认为cpu核数
	AsioIOServicePool(std::size_t size = std::thread::hardware_concurrency());

	AsioIOServicePool(const AsioIOServicePool&) = delete;
	AsioIOServicePool& operator=(const AsioIOServicePool&) = delete;

private:
	std::mutex _mutex;
	std::vector<IOService> _io_services;    // vector 会调用对应的默认构造函数初始化
	std::vector<WorkPtr> _works;            // 防止boost::asio::io_context;自动关闭
	std::vector<std::thread> _threads;
	
	std::size_t _nextIOServcie;
};

