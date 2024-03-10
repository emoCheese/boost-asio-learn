#include "endpoint.h"
#include <iostream>
#include <boost/asio.hpp>

using namespace boost;


// 创建端点，端点中包含 协议版本，ip地址，端口号
int client_end_point()
{
	// 定义原始 IP 和 端口
	std::string raw_ip_address = "127.0.0.1";
	unsigned short port = 9999;

	// error发生时用来存储error
	boost::system::error_code ec;
	
	// 地址转换 使用 IP 协议版本无关的地址表示
	asio::ip::address ip_address = asio::ip::address::from_string(raw_ip_address, ec);

	if (ec.value() != 0) {
		// Provided IP address is invalid. Breaking execution.
		std::cout
			<< "Failed to parse the IP address. Error code = "
			<< ec.value() << ". Message: " << ec.message();
		return ec.value();
	}

	// 创建端点
	asio::ip::tcp::endpoint ep(ip_address, port);
	return 0;
}

int server_end_point()
{
	// 假设服务器以获得协议端口号
	unsigned short port_num = 9999;

                                      //    选择ip版本  指定主机上所有IP地址
	asio::ip::address ip_address = asio::ip::address_v6::any();

	// 创建端点
	asio::ip::tcp::endpoint ep(ip_address, port_num);
	return 0;
}


// 创建通信socket，猜测用于监听
// 创建socket分为4步，创建上下文iocontext，选择协议，生成socket，打开socket。
int create_tcp_socket()
{
	asio::io_context ioc;  // 创建上下文iocontext

	asio::ip::tcp protocol = asio::ip::tcp::v4();  // 选择协议

	asio::ip::tcp::socket sock(ioc);      // 生成socket

	boost::system::error_code ec;
	sock.open(protocol, ec);          // 打开socket
	if (ec.value() != 0) {
		// 打开 socket 失败
		std::cout
			<< "Failed to open the socket! Error code = "
			<< ec.value() << ". Message: " << ec.message();
		return ec.value();
	}
	return 0;
}


// 用来接收新的连接。与创建socket流程相同
int create_acceptor_socket()
{
	asio::io_context ioc;    // 创建上下文

	// 老写法
	//asio::ip::tcp::acceptor acceptor(ioc); // 用于接收新连接

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

	// 新写法  指定协议和端口生成 acceptor 并绑定
	asio::ip::tcp::acceptor a(ioc, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 9999));

	return 0;
}



// 相当于 bind  绑定 acceptor，分开操作
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


// 客户端连接服务器
int connect_to_end()
{
	// 假设已知服务器地址
	std::string raw_ip_address = "127.0.0.1";
	unsigned short port_num = 9999;

	try
	{
		// 生成端点，客户端用于连接，服务端用于绑定
		asio::ip::tcp::endpoint ep(asio::ip::address::from_string(raw_ip_address), port_num);

		// 创建socket
		asio::io_context ioc;
		asio::ip::tcp::socket sock(ioc, ep.protocol());

		// 连接
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

// 客户端通过域名连接到服务器
int dns_connect_to_end()
{
	std::string host = "www.bing.com";
	std::string port_num = "9999";
	asio::io_context ioc;
	asio::ip::tcp::resolver::query resolver_query(
		host, 
		port_num, 
		asio::ip::tcp::resolver::query::numeric_service  // 传入该参数用于域名解析
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
	
	// 监听队列大小，最多60
	const int BACKLOG_SIZE = 30;

	unsigned short port_num = 9999;

	// 创建服务器端点
	asio::ip::tcp::endpoint ep(asio::ip::address_v4::any(), port_num);
	asio::io_context ios;

	try {
		// 创建 acceptor.
		asio::ip::tcp::acceptor acceptor(ios, ep.protocol());
		// Step 4. Binding the acceptor socket to the 
		// 服务器端点绑定
		acceptor.bind(ep);

		// 监听 设置监听队列长度
		acceptor.listen(BACKLOG_SIZE);

		// 创建 socket
		asio::ip::tcp::socket sock(ios);

		// 接收连接，交给socket处理
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
