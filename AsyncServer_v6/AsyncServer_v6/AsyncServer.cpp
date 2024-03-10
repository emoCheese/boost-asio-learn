#include <iostream>
#include "CSession.h"
#include "CServer.h"


int main()
{
	try {
		boost::asio::io_context io_context;
		// 将信号作为一个 event 注册给 iocp 或 epoll
		boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);   
		// 若 iocp 或 epoll 模型做轮询时判断 注册的信号触发了就会调用 注册的回调函数
		signals.async_wait([&io_context](auto, auto) {
			io_context.stop();
			});


		CServer s(io_context, 9998);
		io_context.run();
	}
	catch (std::exception& e) {
		std::cerr << "Exception: " << e.what() << std::endl;
	}
	return 0;
}