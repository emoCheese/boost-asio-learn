#include "Session.h"


Session::Session(std::shared_ptr<asio::ip::tcp::socket> sock)
	: _socket(std::make_shared<asio::ip::tcp::socket>(sock))
{}

void Session::Connect(const asio::ip::tcp::endpoint& ep) {
	_socket->connect(ep);
}