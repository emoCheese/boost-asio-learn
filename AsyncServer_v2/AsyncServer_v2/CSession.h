#pragma once
#include <iostream>
#include <boost/asio.hpp>
#include <queue>
#include "CServer.h"
#include "boost/uuid/uuid_generators.hpp"
#include "boost/uuid/uuid_io.hpp"


using namespace boost::asio;
using namespace std;

class CServer;

class MsgNode
{
	friend class CSession;
public:
	MsgNode(char* msg, int max_len)
		: _max_len(max_len)
	{
		_data = new char[max_len];
		memcpy(_data, msg, max_len);
	}
	~MsgNode() { delete[] _data; }

private:
	int _cur_len;   // 已发送或已接收的数据长度
	int _max_len;   // 最大数据长度
	char* _data;    // 数据首地址
};


class CSession : public std::enable_shared_from_this<CSession>
{
public:
	CSession(boost::asio::io_context& ioc, CServer* server)
		: _socket(ioc),
		_server(server)
	{
		boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
		_uuid = boost::uuids::to_string(a_uuid);
	}

	~CSession() { cout << "CSession destructed " << this << endl; }

	ip::tcp::socket& Socket();  // 获取 socket
	void Start();
	const std::string& GetUuid() const;
	void Send(char* msg, std::size_t max_length);

private:
	void HandleRead(
		const boost::system::error_code& error,
		std::size_t bytes_transformed,
		std::shared_ptr<CSession> _self_shared
	);
	void HandleWrite(const boost::system::error_code& error, std::shared_ptr<CSession> _self_shared);

private:
	ip::tcp::socket _socket;
	enum { max_length = 1024 };
	char _data[max_length];
	CServer* _server;
	std::string _uuid;

	std::mutex _send_lock;
	std::queue<std::shared_ptr<MsgNode>> _send_que;  // 发送队列 
};
