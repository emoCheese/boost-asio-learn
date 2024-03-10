#pragma once
#include <iostream>
#include <boost/asio.hpp>
#include <queue>
#include "CServer.h"
#include "MsgNode.h"
#include "boost/uuid/uuid_generators.hpp"
#include "boost/uuid/uuid_io.hpp"


using namespace boost::asio;
using namespace std;

class CServer;


class CSession : public std::enable_shared_from_this<CSession>
{
public:
	CSession(boost::asio::io_context& ioc, CServer* server);
		

	~CSession() { cout << "CSession destructed " << this << endl; }

	ip::tcp::socket& GetSocket();  // 获取 socket
	void Start();
	void Close();
	const std::string& GetUuid() const;
	void Send(char* msg, short msg_len, short msg_id);

private:
	void HandleRead(
		const boost::system::error_code& error,
		std::size_t bytes_transformed,
		std::shared_ptr<CSession> _self_shared
	);

	void HandleReadHead(
		const boost::system::error_code& error,
		std::size_t bytes_transformed,
		std::shared_ptr<CSession> _self_shared
	);

	void HandleReadMsg(
		const boost::system::error_code& error,
		std::size_t bytes_transformed,
		std::shared_ptr<CSession> _self_shared
	);

	void HandleWrite(const boost::system::error_code& error, std::shared_ptr<CSession> _self_shared);

private:
	boost::asio::ip::tcp::socket _socket;
	
	char _data[MAX_LENGTH];
	CServer* _server;
	std::string _uuid;

	std::mutex _send_lock;
	std::queue<std::shared_ptr<MsgNode>> _send_que;  // 发送队列 
	bool _b_head_parse;
	bool _b_close;

	std::shared_ptr<RecvNode> _recv_msg_node;
	std::shared_ptr<MsgNode> _recv_head_node;
};
