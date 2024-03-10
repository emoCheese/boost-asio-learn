#include "Server.h"


// io_context ���ܱ����ƣ�ֻ������
Server::Server(boost::asio::io_context& ioc, short port)
	: _ioc(ioc),
	_acceptor(ioc, ip::tcp::endpoint(ip::tcp::v4(), port))
{
	std::cout << "Server start success, on port: " << port << std::endl;
	start_accept();
}

void Server::ClearSession(std::string uuid)
{
	_sessions.erase(uuid);
}

void Server::start_accept()
{
	std::shared_ptr<Session> new_session 
		= std::make_shared<Session>(_ioc, this);

	// ����һ�� socket �� һ��ֻ�� error �Ļص�������
	_acceptor.async_accept(new_session->Socket(), std::bind(
		&Server::handler_accept, this, new_session, std::placeholders::_1
	));
}

void Server::handler_accept(std::shared_ptr<Session> new_session, const boost::system::error_code& error)
{
	if (!error) {
		new_session->Start();
		_sessions.insert(std::make_pair(new_session->GetUuid(), new_session));
	}
	else {
		//delete session;
	}
	start_accept();
}
