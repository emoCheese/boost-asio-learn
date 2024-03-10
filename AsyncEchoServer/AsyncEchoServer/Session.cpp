#include "Session.h"


ip::tcp::socket& Session::Socket()
{
	return _socket;
}

void Session::Start()
{
	memset(_data, 0, max_length);
	_socket.async_read_some(
		boost::asio::buffer(_data, max_length),
		std::bind(&Session::handle_read, this, std::placeholders::_1, std::placeholders::_2)
	);
}

void Session::handle_read(const boost::system::error_code& error, std::size_t bytes_transformed)
{
	if (!error) {
		std::cout << "Server recive data is: " << _data << std::endl;
		boost::asio::async_write(
			_socket,
			boost::asio::buffer(_data, bytes_transformed),
			std::bind(&Session::handle_write, this, std::placeholders::_1)
		);
	}
	else {
		delete this;
	}

}

void Session::handle_write(const boost::system::error_code& error)
{
	memset(_data, 0, max_length);
	if (!error) {
		_socket.async_read_some(
			boost::asio::buffer(_data, max_length),
			std::bind(&Session::handle_read, this, std::placeholders::_1, std::placeholders::_2)
		);
	}
	else {
		delete this;
	}
}

// io_context 不能被复制，只能引用
Server::Server(boost::asio::io_context& ioc, short port)
	: _ioc(ioc),
	_acceptor(ioc, ip::tcp::endpoint(ip::tcp::v4(), port))
{
	std::cout << "Server start success, on port: " << port << std::endl;
	start_accept();
}

void Server::start_accept()
{
	Session* new_session = new Session(_ioc);

	// 接受一个 socket 和 一个只带 error 的回调函数；
	_acceptor.async_accept(new_session->Socket(), std::bind(
		&Server::handler_acceptor, this, new_session, std::placeholders::_1
	));

}

void Server::handler_acceptor(Session* session, const boost::system::error_code& error)
{
	if (!error) {
		session->Start();
	}
	else {
		delete session;
	}
	start_accept();
}
