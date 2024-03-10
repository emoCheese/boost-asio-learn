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
	short _cur_len;   // �ѷ��ͻ��ѽ��յ����ݳ���
	short _total_len;   // ������ݳ���
	char* _data;    // �����׵�ַ
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
