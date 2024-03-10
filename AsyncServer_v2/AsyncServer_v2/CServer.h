#pragma once
#include <iostream>
#include <boost/asio.hpp>
#include "CSession.h"
#include <map>

using namespace boost::asio;
using namespace std;

class CSession;

class CServer
{
public:
	CServer(boost::asio::io_context& ioc, short port);
	void ClearSession(std::string uuid);
private:
	void start_accept();
	void handler_accept(std::shared_ptr<CSession> new_session, const boost::system::error_code& error);
	boost::asio::io_context& _ioc;
	ip::tcp::acceptor _acceptor;
	std::map<std::string, std::shared_ptr<CSession>> _sessions;
};
