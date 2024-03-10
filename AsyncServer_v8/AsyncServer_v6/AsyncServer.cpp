#include <iostream>
#include "CSession.h"
#include "CServer.h"
#include "AsioIOServicePool.h"

/*
* 外部一个io_context用于监听连接事件，并且有连接事件了还是这个io_context来连接，
* 之后这个连接上的读写事件将由IOServicePool中的线程（对应新的io_context）来处理。
*/

int main()
{
	try {
		auto pool = AsioIOServicePool::GetInstance();  // GetGetInstance 会调用AsioIOServicePool的构造函数

		// 用于监听连接事件
		boost::asio::io_context io_context;
		// 将信号作为一个 event 注册给 iocp 或 epoll
		boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);   
		// 若 iocp 或 epoll 模型做轮询时判断 注册的信号(CTRL + C)触发了就会调用注册的回调函数
		signals.async_wait([&io_context, pool](auto, auto) {
			io_context.stop();
			pool->Stop();
			});

		// CServer 会取 pool 中的io_context用于连接
		// 在构造函数里，会取main函数中的io_context用于监听连接
		CServer s(io_context, 9998);
		io_context.run();
	}
	catch (std::exception& e) {
		std::cerr << "Exception: " << e.what() << std::endl;
	}
	return 0;
}