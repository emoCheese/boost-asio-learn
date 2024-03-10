#pragma once
#include <iostream>
#include <boost/asio.hpp>
#include "Session.h"
#include <map>

using namespace boost::asio;
using namespace std;

class Session;

class Server
{
public:
	Server(boost::asio::io_context& ioc, short port);
	void ClearSession(std::string uuid);
private:
	void start_accept();
	void handler_accept(std::shared_ptr<Session> new_session, const boost::system::error_code& error);
	boost::asio::io_context& _ioc;
	ip::tcp::acceptor _acceptor;
	std::map<std::string, std::shared_ptr<Session>> _sessions;
};
