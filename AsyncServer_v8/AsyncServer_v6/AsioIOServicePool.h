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
	using WorkPtr = std::unique_ptr<Work>;         // unqiue_ptr ʹ work���ܱ�����

	~AsioIOServicePool();
	boost::asio::io_context& GetIOService();
	void Stop();
private:
	// Ĭ��Ϊcpu����
	AsioIOServicePool(std::size_t size = std::thread::hardware_concurrency());

	AsioIOServicePool(const AsioIOServicePool&) = delete;
	AsioIOServicePool& operator=(const AsioIOServicePool&) = delete;

private:
	std::mutex _mutex;
	std::vector<IOService> _io_services;    // vector ����ö�Ӧ��Ĭ�Ϲ��캯����ʼ��
	std::vector<WorkPtr> _works;            // ��ֹboost::asio::io_context;�Զ��ر�
	std::vector<std::thread> _threads;
	
	std::size_t _nextIOServcie;
};

