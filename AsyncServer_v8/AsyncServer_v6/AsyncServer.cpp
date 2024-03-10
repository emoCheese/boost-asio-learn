#include <iostream>
#include "CSession.h"
#include "CServer.h"
#include "AsioIOServicePool.h"

/*
* �ⲿһ��io_context���ڼ��������¼��������������¼��˻������io_context�����ӣ�
* ֮����������ϵĶ�д�¼�����IOServicePool�е��̣߳���Ӧ�µ�io_context��������
*/

int main()
{
	try {
		auto pool = AsioIOServicePool::GetInstance();  // GetGetInstance �����AsioIOServicePool�Ĺ��캯��

		// ���ڼ��������¼�
		boost::asio::io_context io_context;
		// ���ź���Ϊһ�� event ע��� iocp �� epoll
		boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);   
		// �� iocp �� epoll ģ������ѯʱ�ж� ע����ź�(CTRL + C)�����˾ͻ����ע��Ļص�����
		signals.async_wait([&io_context, pool](auto, auto) {
			io_context.stop();
			pool->Stop();
			});

		// CServer ��ȡ pool �е�io_context��������
		// �ڹ��캯�����ȡmain�����е�io_context���ڼ�������
		CServer s(io_context, 9998);
		io_context.run();
	}
	catch (std::exception& e) {
		std::cerr << "Exception: " << e.what() << std::endl;
	}
	return 0;
}