#include "CServer.h"


// io_context ���ܱ����ƣ�ֻ������
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

	// ����һ�� socket �� һ��ֻ�� error �Ļص�������
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
	// �����ȴ�����
	StartAccept();
}
