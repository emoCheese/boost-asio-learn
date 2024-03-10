#include <iostream>
#include "CSession.h"
#include "CServer.h"


int main()
{
	try {
		boost::asio::io_context io_context;
		// ���ź���Ϊһ�� event ע��� iocp �� epoll
		boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);   
		// �� iocp �� epoll ģ������ѯʱ�ж� ע����źŴ����˾ͻ���� ע��Ļص�����
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