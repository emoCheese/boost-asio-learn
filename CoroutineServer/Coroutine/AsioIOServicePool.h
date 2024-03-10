#pragma once
#include "boost/asio.hpp"
#include <mutex>
#include <thread>
#include <memory>
#include <vector>
#include <iostream>


class AsioIOServicePool
{
public:
	using Work = boost::asio::io_context::work;
	using WorkPtr = std::unique_ptr<Work>;
	using IOService = boost::asio::io_context;

	AsioIOServicePool(const AsioIOServicePool&) = delete;
	AsioIOServicePool& operator=(const AsioIOServicePool&) = delete;

	~AsioIOServicePool();

	static AsioIOServicePool& GetInstance();
	boost::asio::io_context& GetIOService();
	void Stop();

private:
	AsioIOServicePool(int thread_num = std::thread::hardware_concurrency());

	std::vector<IOService> _services;
	std::vector<WorkPtr> _works;
	std::vector<std::thread> _threads;
	std::size_t _nextIOService;
};

