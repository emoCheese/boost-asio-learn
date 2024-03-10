#include "CServer.h"

// 传入的 io_context 绑定 acceptor 用于接收连接
CServer::CServer(boost::asio::io_context& io_context, short port)
	: _io_context(io_context), 
	_port(port), 
	_acceptor(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
{
	std::cout << "server start on port " << _port << std::endl;
	StartAccept();
}

CServer::~CServer()
{
	std::cout << "CServer destructed" << std::endl;
}

void CServer::ClearSession(const std::string& uuid)
{
	std::lock_guard<std::mutex> lock(_mutex);
	_sessions.erase(uuid);
}

// AsioIOServicePool中的io_context创建CSession接受连接
void CServer::StartAccept()
{
	auto& io_context = AsioIOServicePool::GetInstance().GetIOService();
	std::shared_ptr<CSession> new_session = std::make_shared<CSession>(io_context, this);
	_acceptor.async_accept(
		new_session->GetSocket(), 
		std::bind(&CServer::HandlerAccept, this, new_session, std::placeholders::_1)
	);
}

void CServer::HandlerAccept(std::shared_ptr<CSession> new_session, const boost::system::error_code& error)
{
	if (!error)
	{
		new_session->Start();
		std::lock_guard<std::mutex> lock(_mutex);
		_sessions.insert(std::make_pair(new_session->GetUuid(), new_session));
	}
	else
	{
		std::cout << "session accept failed, error is " << error.what() << std::endl;
	}
	StartAccept();
}