#pragma once
#include <iostream>
#include <boost/asio.hpp>

using namespace boost::asio;

class Session
{
public:
	Session(boost::asio::io_context& ioc)
		: _socket(ioc)
	{}

	ip::tcp::socket& Socket();  // ªÒ»° socket
	void Start();

private:
	void handle_read(const boost::system::error_code& error, std::size_t bytes_transformed);
	void handle_write(const boost::system::error_code& error);

private:
	ip::tcp::socket _socket;
	enum{ max_length = 1024 };
	char _data[max_length];
};

class Server
{
public:
	Server(boost::asio::io_context& ioc, short port);
		
private:
	void start_accept();
	void handler_acceptor(Session* session, const boost::system::error_code& error);
	boost::asio::io_context& _ioc;
	ip::tcp::acceptor _acceptor;
};