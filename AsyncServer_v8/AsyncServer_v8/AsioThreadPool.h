#pragma once

#include <iostream>
#include <memory>
#include <thread>
#include <vector>

#include "boost/asio.hpp"

#include "Singleton.h"

class AsioThreadPool : public Singleton<AsioThreadPool>
{
public:
	using Work = boost::asio::io_context::work;

	friend class Singleton<AsioThreadPool>;
	~AsioThreadPool();
	boost::asio::io_context& GetIOService();
	void Stop();
private:
	AsioThreadPool(const AsioThreadPool&) = delete;
	AsioThreadPool& operator=(const AsioThreadPool&) = delete;

private:
	AsioThreadPool(int thread_nums = std::thread::hardware_concurrency());

	boost::asio::io_context _service;
	std::unique_ptr<Work> _work_ptr;
	std::vector<std::thread> _threads;
};

