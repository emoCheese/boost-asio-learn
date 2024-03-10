#pragma once
#include <iostream>
#include <boost/asio.hpp>
#include "CSession.h"
#include <map>

class CSession;

class CServer
{
public:
	CServer(boost::asio::io_context& ioc, short port);
	void ClearSession(std::string uuid);
private:
	void StartAccept();
	void HandleAccept(std::shared_ptr<CSession> new_session, const boost::system::error_code& error);
	boost::asio::io_context& _ioc;
	short _port;
	boost::asio::ip::tcp::acceptor _acceptor;
	std::map<std::string, std::shared_ptr<CSession>> _sessions;
};
