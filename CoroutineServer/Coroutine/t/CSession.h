#pragma once

#include <memory>
#include <queue>
#include <mutex>

#include "boost/asio/co_spawn.hpp"
#include "boost/asio/detached.hpp"
#include "boost/asio.hpp"
#include "boost/uuid/uuid_generators.hpp"
#include "boost/uuid/uuid_io.hpp"

#include "CServer.h"
#include "const.h"
#include "MsgNode.h"

class CServer;
class SendNode;
class RecvNode;
class MsgNode;

class CSession : public std::enable_shared_from_this<CSession>
{
public:
	CSession(boost::asio::io_context& _io_context, CServer* server);
	~CSession();

	void Start();
	void Close();
	boost::asio::ip::tcp::socket& GetSocket();
	const std::string& GetUuid() const;
	void Send(const char* msg, short msg_len, short msg_id);
	void Send(std::string msg, short msg_id);

private:
	void HandleWrite(const boost::system::error_code& error, std::shared_ptr<CSession> share_self);


	boost::asio::io_context& _io_context;
	CServer* _server;
	boost::asio::ip::tcp::socket _socket;
	std::string _uuid;
	bool _b_close;                                     
	std::queue<std::shared_ptr<SendNode>> _send_que;  // 发送队列
	std::shared_ptr<RecvNode> _recv_msg_node;         // 接收消息的节点
	std::shared_ptr<MsgNode> _recv_head_node;         // 消息的头部

	std::mutex _send_lock;
};

class LogicNode
{
public:
	friend class LogicSystem;
	LogicNode(std::shared_ptr<CSession> session, std::shared_ptr<RecvNode> recv_node);
private:
	std::shared_ptr<CSession> _session;
	std::shared_ptr<RecvNode> _recv_node;
};

