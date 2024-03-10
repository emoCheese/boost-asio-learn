#include "CServer.h"


// io_context 不能被复制，只能引用
CServer::CServer(boost::asio::io_context& ioc, short port)
	: _ioc(ioc),
	_acceptor(ioc, ip::tcp::endpoint(ip::tcp::v4(), port)), _port(port)
{
	std::cout << "Server start success, on port: " << port << std::endl;
	StartAccept();
}

void CServer::ClearSession(std::string uuid)
{
	_sessions.erase(uuid);
}

void CServer::StartAccept()
{
	std::shared_ptr<CSession> new_session = std::make_shared<CSession>(_ioc, this);

	// 接受一个 socket 和 一个只带 error 的回调函数；
	_acceptor.async_accept(new_session->GetSocket(), 
		std::bind(&CServer::HandlerAccept, this, new_session, std::placeholders::_1)
	);
}

void CServer::HandlerAccept(std::shared_ptr<CSession> new_session, const boost::system::error_code& error)
{
	if (!error) {
		new_session->Start();
		_sessions.insert(std::make_pair(new_session->GetUuid(), new_session));
	}
	else {
		//delete session;
	}
	// 继续等待连接
	StartAccept();
}
