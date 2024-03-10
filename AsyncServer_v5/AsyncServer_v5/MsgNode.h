#pragma once
#include <iostream>
#include "const.h"
#include "boost/asio.hpp"

class MsgNode
{
	friend class CSession;
public:
	MsgNode(int max_len);
		
	~MsgNode() { delete[] _data; }

	void Clear();

protected:
	short _cur_len;   // 已发送或已接收的数据长度
	short _total_len;   // 最大数据长度
	char* _data;    // 数据首地址
};

class RecvNode : public MsgNode
{
public:
	RecvNode(short total_len, short msg_id);
	short _msg_id;
};

class SendNode : public MsgNode
{
public:
	SendNode(const char* data, short total_len, short msg_id);
	short _msg_id;
};
