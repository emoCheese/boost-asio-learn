#pragma once
#include <iostream>
#include "const.h"
#include "boost/asio.hpp"

class LogicSystem;

class MsgNode
{
public:
	MsgNode(short max_len);

	MsgNode(short max_len, short cur_len);
		
	~MsgNode() { std::cout << "MsgNode destruct\n"; delete[] _data; }

	void Clear();


	short _cur_len;   // 现有数据的长度，已发送或已接收的数据长度
	short _total_len;   // 最大数据长度
	char* _data;    // 数据首地址
};

class RecvNode : public MsgNode
{
	friend class LogicSystem;
	friend class CSession;
public:
	RecvNode(short total_len, short cur_len, short msg_id);
	RecvNode(short total_len, short msg_id);
private:
	short _msg_id;
};

class SendNode : public MsgNode
{
	friend class LogicSystem;
public:
	SendNode(const char* data, short total_len, short msg_id);
private:
	short _msg_id;
};

