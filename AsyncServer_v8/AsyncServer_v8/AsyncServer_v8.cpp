#include <iostream>
#include <memory>
#include <thread>
#include <mutex>
#include "CSession.h"
#include "CServer.h"
#include "AsioThreadPool.h"

static bool bstop = false;
static std::condition_variable cv;
static std::mutex mutex_quit;

int main()
{
	try {
		

		auto pool = AsioThreadPool::GetInstance();

		// 用于监听停止信号
		boost::asio::io_context io_context;
		// 将信号作为一个 event 注册给 iocp 或 epoll
		boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);   
		// 若 iocp 或 epoll 模型做轮询时判断 注册的信号(CTRL + C)触发了就会调用注册的回调函数
		signals.async_wait([pool, &io_context](auto, auto) {
			io_context.stop();
			pool->Stop();
			std::unique_lock<std::mutex> lock(mutex_quit);
			bstop = true;
			cv.notify_one();  // 激活主线程
			});

		CServer s(pool->GetIOService(), 9998);

		{
			std::unique_lock<std::mutex> lock(mutex_quit);
			while (!bstop)
				cv.wait(lock);  // 将主线程挂起 防止程序未初始化就退出 在收到停止信号时激活
		}

	}
	catch (std::exception& e) {
		std::cerr << "Exception: " << e.what() << std::endl;
	}
	return 0;
}