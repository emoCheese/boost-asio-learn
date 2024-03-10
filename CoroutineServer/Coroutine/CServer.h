#pragma once
#include <iostream>
#include <mutex>
#include <memory>
#include <map>

#include "boost/asio.hpp"

#include "CSession.h"
#include "AsioIOServicePool.h"

class CSession;

class CServer
{
public:
	CServer(boost::asio::io_context& io_context, short port);
	~CServer();

	void ClearSession(const std::string& uuid);
private:
	void StartAccept();
	void HandlerAccept(std::shared_ptr<CSession> new_session, const boost::system::error_code& error);
	
	boost::asio::io_context& _io_context;
	boost::asio::ip::tcp::acceptor _acceptor;    // 接收器，绑定一个io_context 用于接收连接
	std::mutex _mutex;
	short _port;
	std::map<std::string, std::shared_ptr<CSession>> _sessions;
};

