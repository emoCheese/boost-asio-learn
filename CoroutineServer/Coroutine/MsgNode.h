#pragma once
#include <iostream>
#include <boost/asio.hpp>
#include "const.h"
#include "LogicSystem.h"

class MsgNode
{
public:
	friend class RecvNode;
	friend class SendNode;

	MsgNode(short max_len)
		: _total_len(max_len), _cur_len(0)
	{
		_data = new char[_total_len + 1]();
		_data[_total_len] = '\0';
	}
	~MsgNode() { std::cout << "MsgNode destructed" << std::endl; delete[] _data; }

	char* GetData();
	const char* GetData() const;
	short GetTotalLen();
	short GetCurLen();
	void Clear();
//private:
	short _cur_len;
	short _total_len;
	char* _data;
};

class RecvNode : public MsgNode
{
public:
	friend class LogicSystem;
	RecvNode(short max_len, short msg_id);
//private:
	short _msg_id;
};

class SendNode : public MsgNode
{
public:
	SendNode(const char* msg, short max_len, short msg_id);
//private:
	short _msg_id;
};
