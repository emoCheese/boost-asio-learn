#include "MsgNode.h"

void MsgNode::Clear()
{
	::memset(_data, 0, _total_len);
	_cur_len = 0;
}

char* MsgNode::GetData()
{
	return _data;
}

const char* MsgNode::GetData() const
{
	return _data;
}

short MsgNode::GetTotalLen()
{
	return _total_len;
}

short MsgNode::GetCurLen()
{
	return _cur_len;
}

RecvNode::RecvNode(short max_len, short msg_id)
	:MsgNode(max_len), _msg_id(msg_id)
{
}

// 要发送的消息，要发送的消息长度，要发送的消息id
SendNode::SendNode(const char* msg, short msg_len, short msg_id)
	: MsgNode(msg_len + HEAD_TOTAL_LEN), _msg_id(msg_id)
{
	short msg_id_net = boost::asio::detail::socket_ops::host_to_network_short(msg_id);
	memcpy(_data, &msg_id_net, HEAD_ID_LEN);
	short msg_len_net = boost::asio::detail::socket_ops::host_to_network_short(msg_len);
	memcpy(_data + HEAD_ID_LEN, &msg_len_net, HEAD_DATA_LEN);
	memcpy(_data + HEAD_TOTAL_LEN, msg, msg_len);
}


