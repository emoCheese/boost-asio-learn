#pragma once
#include <iostream>
#include <boost/asio.hpp>
#include <queue>
#include "CServer.h"
#include "MsgNode.h"
#include "LogicSystem.h"
#include "boost/uuid/uuid_generators.hpp"
#include "boost/uuid/uuid_io.hpp"


class CServer;
class RecvNode;
class MsgNode;

class CSession : public std::enable_shared_from_this<CSession>
{
public:
	CSession(boost::asio::io_context& ioc, CServer* server);
		

	~CSession() { std::cout << "CSession destructed " << this << std::endl; }

	boost::asio::ip::tcp::socket& GetSocket();  // 获取 socket
	void Start();
	void Close();
	const std::string& GetUuid() const;
	void Send(std::string msg, short msgid);
	void Send(const char* msg, short msg_len, short msg_id);

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
	
	std::string _uuid;
	char _data[MAX_LENGTH];
	CServer* _server;
	bool _b_close;
	std::queue<std::shared_ptr<SendNode>> _send_que;
	std::mutex _send_lock;
	//收到的消息结构
	std::shared_ptr<RecvNode> _recv_msg_node;
	bool _b_head_parse;
	//收到的头部结构
	std::shared_ptr<MsgNode> _recv_head_node;

	boost::asio::strand<boost::asio::io_context::executor_type> _strand;
};



class LogicNode
{
	friend class LogicSystem;
public:
	LogicNode(std::shared_ptr<CSession> session, std::shared_ptr<RecvNode> recv_node);

private:
	std::shared_ptr<CSession> _session;
	std::shared_ptr<RecvNode> _recv_node;
};