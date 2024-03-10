#include "endpoint.h"
#include <iostream>
#include <boost/asio.hpp>

using namespace boost;


// �����˵㣬�˵��а��� Э��汾��ip��ַ���˿ں�
int client_end_point()
{
	// ����ԭʼ IP �� �˿�
	std::string raw_ip_address = "127.0.0.1";
	unsigned short port = 9999;

	// error����ʱ�����洢error
	boost::system::error_code ec;
	
	// ��ַת�� ʹ�� IP Э��汾�޹صĵ�ַ��ʾ
	asio::ip::address ip_address = asio::ip::address::from_string(raw_ip_address, ec);

	if (ec.value() != 0) {
		// Provided IP address is invalid. Breaking execution.
		std::cout
			<< "Failed to parse the IP address. Error code = "
			<< ec.value() << ". Message: " << ec.message();
		return ec.value();
	}

	// �����˵�
	asio::ip::tcp::endpoint ep(ip_address, port);
	return 0;
}

int server_end_point()
{
	// ����������Ի��Э��˿ں�
	unsigned short port_num = 9999;

                                      //    ѡ��ip�汾  ָ������������IP��ַ
	asio::ip::address ip_address = asio::ip::address_v6::any();

	// �����˵�
	asio::ip::tcp::endpoint ep(ip_address, port_num);
	return 0;
}


// ����ͨ��socket���²����ڼ���
// ����socket��Ϊ4��������������iocontext��ѡ��Э�飬����socket����socket��
int create_tcp_socket()
{
	asio::io_context ioc;  // ����������iocontext

	asio::ip::tcp protocol = asio::ip::tcp::v4();  // ѡ��Э��

	asio::ip::tcp::socket sock(ioc);      // ����socket

	boost::system::error_code ec;
	sock.open(protocol, ec);          // ��socket
	if (ec.value() != 0) {
		// �� socket ʧ��
		std::cout
			<< "Failed to open the socket! Error code = "
			<< ec.value() << ". Message: " << ec.message();
		return ec.value();
	}
	return 0;
}


// ���������µ����ӡ��봴��socket������ͬ
int create_acceptor_socket()
{
	asio::io_context ioc;    // ����������

	// ��д��
	//asio::ip::tcp::acceptor acceptor(ioc); // ���ڽ���������

	//asio::ip::tcp protocol = asio::ip::tcp::v6();
	//boost::system::error_code ec;
	//
	//acceptor.open(protocol, ec);
	//if (ec.value() != 0) {
	//	// Failed to open the socket.
	//	std::cout
	//		<< "Failed to open the acceptor socket!"
	//		<< "Error code = "
	//		<< ec.value() << ". Message: " << ec.message();
	//	return ec.value();
	//}

	// ��д��  ָ��Э��Ͷ˿����� acceptor ����
	asio::ip::tcp::acceptor a(ioc, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 9999));

	return 0;
}



// �൱�� bind  �� acceptor���ֿ�����
int bind_acceptor_socket()
{
	unsigned short port_num = 9999;
	asio::ip::tcp::endpoint ep(asio::ip::address_v4::any() , port_num);
	asio::io_context ioc;
	asio::ip::tcp::acceptor acceptor(ioc, ep.protocol());

	boost::system::error_code ec;
	acceptor.bind(ep, ec);
	return 0;
}


// �ͻ������ӷ�����
int connect_to_end()
{
	// ������֪��������ַ
	std::string raw_ip_address = "127.0.0.1";
	unsigned short port_num = 9999;

	try
	{
		// ���ɶ˵㣬�ͻ����������ӣ���������ڰ�
		asio::ip::tcp::endpoint ep(asio::ip::address::from_string(raw_ip_address), port_num);

		// ����socket
		asio::io_context ioc;
		asio::ip::tcp::socket sock(ioc, ep.protocol());

		// ����
		sock.connect(ep);
	}
	catch (const system::system_error& e)
	{
		std::cout << "Error occured! Error code = " << e.code()
			<< ". Message: " << e.what();
		return e.code().value();
	}
	return 0;
}

// �ͻ���ͨ���������ӵ�������
int dns_connect_to_end()
{
	std::string host = "www.bing.com";
	std::string port_num = "9999";
	asio::io_context ioc;
	asio::ip::tcp::resolver::query resolver_query(
		host, 
		port_num, 
		asio::ip::tcp::resolver::query::numeric_service  // ����ò���������������
	);

	asio::ip::tcp::resolver resolver(ioc);
	try {
		asio::ip::tcp::resolver::iterator it = resolver.resolve(resolver_query);
		asio::ip::tcp::socket sock(ioc);
		asio::connect(sock, it);
	}
	catch (const system::system_error& e) {
		std::cout << "Error occured! Error code = " << e.code()
			<< ". Message: " << e.what();
		return e.code().value();
	}
	return 0;
}

int accept_new_connection() {
	
	// �������д�С�����60
	const int BACKLOG_SIZE = 30;

	unsigned short port_num = 9999;

	// �����������˵�
	asio::ip::tcp::endpoint ep(asio::ip::address_v4::any(), port_num);
	asio::io_context ios;

	try {
		// ���� acceptor.
		asio::ip::tcp::acceptor acceptor(ios, ep.protocol());
		// Step 4. Binding the acceptor socket to the 
		// �������˵��
		acceptor.bind(ep);

		// ���� ���ü������г���
		acceptor.listen(BACKLOG_SIZE);

		// ���� socket
		asio::ip::tcp::socket sock(ios);

		// �������ӣ�����socket����
		acceptor.accept(sock);
		// At this point 'sock' socket is connected to 
		//the client application and can be used to send data to
		// or receive data from it.
	}
	catch (system::system_error& e) {
		std::cout << "Error occured! Error code = " << e.code()
			<< ". Message: " << e.what();
		return e.code().value();
	}
}
