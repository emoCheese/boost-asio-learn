#include <iostream>
#include "CServer.h"
#include "AsioIOServicePool.h"
#include <csignal>
#include "boost/asio.hpp"


int main()
{
	try
	{
		auto& pool = AsioIOServicePool::GetInstance();
		boost::asio::io_context io_context;
		boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);

		// 捕获几个信号，lambda就有几个参数
		signals.async_wait([&io_context, &pool](auto, auto) {
			io_context.stop();
			pool.Stop();
			});
		CServer server(io_context, 9998);
		io_context.run();
	}
	catch (const std::exception& e)
	{
		std::cout << "Exception is " << e.what() << std::endl;
	}
}