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


	short _cur_len;   // �������ݵĳ��ȣ��ѷ��ͻ��ѽ��յ����ݳ���
	short _total_len;   // ������ݳ���
	char* _data;    // �����׵�ַ
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

