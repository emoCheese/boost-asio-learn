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

		// ���ڼ���ֹͣ�ź�
		boost::asio::io_context io_context;
		// ���ź���Ϊһ�� event ע��� iocp �� epoll
		boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);   
		// �� iocp �� epoll ģ������ѯʱ�ж� ע����ź�(CTRL + C)�����˾ͻ����ע��Ļص�����
		signals.async_wait([pool, &io_context](auto, auto) {
			io_context.stop();
			pool->Stop();
			std::unique_lock<std::mutex> lock(mutex_quit);
			bstop = true;
			cv.notify_one();  // �������߳�
			});

		CServer s(pool->GetIOService(), 9998);

		{
			std::unique_lock<std::mutex> lock(mutex_quit);
			while (!bstop)
				cv.wait(lock);  // �����̹߳��� ��ֹ����δ��ʼ�����˳� ���յ�ֹͣ�ź�ʱ����
		}

	}
	catch (std::exception& e) {
		std::cerr << "Exception: " << e.what() << std::endl;
	}
	return 0;
}