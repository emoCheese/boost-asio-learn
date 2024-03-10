#include "MsgNode.h"

MsgNode::MsgNode(int max_len)
	: _total_len(max_len)
{
	_data = new char[max_len + 1]();
	_data[max_len] = 0;
}

void MsgNode::Clear()
{
	::memset(_data, 0, _total_len);
	_cur_len = 0;
}


RecvNode::RecvNode(short total_len, short msg_id)
	: MsgNode(total_len), _msg_id(msg_id)
{

}

SendNode::SendNode(const char* data, short total_len, short msg_id)
	: MsgNode(total_len + HEAD_TOTAL_LEN), _msg_id(msg_id)
{
	short msg_id_network = boost::asio::detail::socket_ops::host_to_network_short(msg_id);
	memcpy(_data, &msg_id_network, HEAD_ID_LEN);

	short msg_len_network = boost::asio::detail::socket_ops::host_to_network_short(total_len);
	memcpy(_data + HEAD_ID_LEN, &msg_len_network, HEAD_DATA_LEN);

	memcpy(_data + HEAD_TOTAL_LEN, data, total_len);
}
