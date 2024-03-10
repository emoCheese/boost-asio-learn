#include "CSession.h"


CSession::CSession(boost::asio::io_context& ioc, CServer* server)
	: _socket(ioc), _server(server), _b_head_parse(false), _b_close(false)
{
	boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
	_uuid = boost::uuids::to_string(a_uuid);
	_recv_head_node = std::make_shared<MsgNode>(HEAD_TOTAL_LEN);
	_recv_head_node->Clear();
}

boost::asio::ip::tcp::socket& CSession::GetSocket()
{
	return _socket;
}

void CSession::Start()
{
	::memset(_data, 0, MAX_LENGTH);
	_socket.async_read_some(
		boost::asio::buffer(_data, MAX_LENGTH),
		std::bind( 
			&CSession::HandleRead,
			this,
			std::placeholders::_1,
			std::placeholders::_2,
			shared_from_this()  // 没有传入shared_ptr，此处不可使用 share_ptr<>() 操作，使用 shared_from_this()
		)
	);
	// 若2个或多个shared_ptr指向同一个内存，它们的引用计数是不一样的，会造成多次析构
	/*_recv_head_node->Clear();
	boost::asio::async_read(_socket, boost::asio::buffer(_recv_head_node->_data, HEAD_TOTAL_LEN),
		std::bind(&CSession::HandleReadHead, this, std::placeholders::_1, std::placeholders::_2, shared_from_this())
	);*/
}

void CSession::Close()
{
	_socket.close();
	_b_close = true;
}

const std::string& CSession::GetUuid() const { return _uuid; }

void CSession::Send(std::string msg, short msgid) {
	std::lock_guard<std::mutex> lock(_send_lock);
	int send_que_size = _send_que.size();
	if (send_que_size > MAX_SENDQUE) {
		std::cout << "session: " << _uuid << " send que fulled, size is " << MAX_SENDQUE << std::endl;
		return;
	}

	_send_que.push(std::make_shared<SendNode>(msg.c_str(), msg.length(), msgid));
	if (send_que_size > 0) {
		return;
	}
	auto& msgnode = _send_que.front();
	boost::asio::async_write(_socket, boost::asio::buffer(msgnode->_data, msgnode->_total_len),
		std::bind(&CSession::HandleWrite, this, std::placeholders::_1, shared_from_this()));
}

void CSession::Send(const char* msg, short msg_len, short msg_id)
{
	std::lock_guard<std::mutex> lock(_send_lock);
	int que_size = _send_que.size();
	if (que_size > MAX_SENDQUE)
	{
		std::cout << "session: " << _uuid << " send que fulled, size is " << MAX_SENDQUE << std::endl;
		return;
	}
		
	_send_que.push(std::make_shared<SendNode>(msg, msg_len, msg_id));
	if (que_size > 0)
		return;
	auto& msg_node = _send_que.front();
	boost::asio::async_write(
		_socket,
		boost::asio::buffer(msg_node->_data, msg_node->_total_len),
		std::bind(&CSession::HandleWrite, this, std::placeholders::_1, shared_from_this())
	);
}


void CSession::HandleRead(
	const boost::system::error_code& error, 
	size_t  bytes_transferred, 
	std::shared_ptr<CSession> shared_self
) 
{
	try {
		if (!error) {
			//已经移动的字符数
			int copy_len = 0;
			while (bytes_transferred > 0) {
				if (!_b_head_parse) {
					//收到的数据不足头部大小
					if (bytes_transferred + _recv_head_node->_cur_len < HEAD_TOTAL_LEN) {
						memcpy(_recv_head_node->_data + _recv_head_node->_cur_len, _data + copy_len, bytes_transferred);
						_recv_head_node->_cur_len += bytes_transferred;
						::memset(_data, 0, MAX_LENGTH);
						_socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
							std::bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, shared_self));
						return;
					}
					//收到的数据比头部多
					//头部剩余未复制的长度
					int head_remain = HEAD_TOTAL_LEN - _recv_head_node->_cur_len;
					memcpy(_recv_head_node->_data + _recv_head_node->_cur_len, _data + copy_len, head_remain);
					//更新已处理的data长度和剩余未处理的长度
					copy_len += head_remain;
					bytes_transferred -= head_remain;
					//获取头部MSGID数据
					short msg_id = 0;
					memcpy(&msg_id, _recv_head_node->_data, HEAD_ID_LEN);
					//网络字节序转化为本地字节序
					msg_id = boost::asio::detail::socket_ops::network_to_host_short(msg_id);
					std::cout << "msg_id is " << msg_id << std::endl;
					//id非法
					if (msg_id > MAX_LENGTH) {
						std::cout << "invalid msg_id is " << msg_id << std::endl;
						_server->ClearSession(_uuid);
						return;
					}
					short msg_len = 0;
					memcpy(&msg_len, _recv_head_node->_data + HEAD_ID_LEN, HEAD_DATA_LEN);
					//网络字节序转化为本地字节序
					msg_len = boost::asio::detail::socket_ops::network_to_host_short(msg_len);
					std::cout << "msg_len is " << msg_len << std::endl;
					//id非法
					if (msg_len > MAX_LENGTH) {
						std::cout << "invalid data length is " << msg_len << std::endl;
						_server->ClearSession(_uuid);
						return;
					}

					_recv_msg_node = std::make_shared<RecvNode>(msg_len, msg_id);

					//消息的长度小于头部规定的长度，说明数据未收全，则先将部分消息放到接收节点里
					if (bytes_transferred < msg_len) {
						memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len, bytes_transferred);
						_recv_msg_node->_cur_len += bytes_transferred;
						::memset(_data, 0, MAX_LENGTH);
						_socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
							std::bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, shared_self));
						//头部处理完成
						_b_head_parse = true;
						return;
					}

					memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len, msg_len);
					_recv_msg_node->_cur_len += msg_len;
					copy_len += msg_len;
					bytes_transferred -= msg_len;
					_recv_msg_node->_data[_recv_msg_node->_total_len] = '\0';
					//cout << "receive data is " << _recv_msg_node->_data << endl;
					//此处将消息投递到逻辑队列中
					LogicSystem::GetInstance()->PostMsgToQue(std::make_shared<LogicNode>(shared_from_this(), _recv_msg_node));

					//继续轮询剩余未处理数据
					_b_head_parse = false;
					_recv_head_node->Clear();
					if (bytes_transferred <= 0) {
						::memset(_data, 0, MAX_LENGTH);
						_socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
							std::bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, shared_self));
						return;
					}
					continue;
				}

				//已经处理完头部，处理上次未接受完的消息数据
				//接收的数据仍不足剩余未处理的
				int remain_msg = _recv_msg_node->_total_len - _recv_msg_node->_cur_len;
				if (bytes_transferred < remain_msg) {
					memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len, bytes_transferred);
					_recv_msg_node->_cur_len += bytes_transferred;
					::memset(_data, 0, MAX_LENGTH);
					_socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
						std::bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, shared_self));
					return;
				}
				memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len, remain_msg);
				_recv_msg_node->_cur_len += remain_msg;
				bytes_transferred -= remain_msg;
				copy_len += remain_msg;
				_recv_msg_node->_data[_recv_msg_node->_total_len] = '\0';
				//cout << "receive data is " << _recv_msg_node->_data << endl;
				//此处将消息投递到逻辑队列中
				LogicSystem::GetInstance()->PostMsgToQue(std::make_shared<LogicNode>(shared_from_this(), _recv_msg_node));

				//继续轮询剩余未处理数据
				_b_head_parse = false;
				_recv_head_node->Clear();
				if (bytes_transferred <= 0) {
					::memset(_data, 0, MAX_LENGTH);
					_socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
						std::bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, shared_self));
					return;
				}
				continue;
			}
		}
		else {
			std::cout << "handle read failed, error is " << error.what() << std::endl;
			Close();
			_server->ClearSession(_uuid);
		}
	}
	catch (std::exception& e) {
		std::cout << "Exception code is " << e.what() << std::endl;
	}
}

void CSession::HandleReadHead(const boost::system::error_code& error, std::size_t bytes_transforred, std::shared_ptr<CSession> _self_shared)
{
	if (!error)
	{
		if (bytes_transforred < HEAD_TOTAL_LEN)
		{
			std::cout << "read head length error";
			Close();
			_server->ClearSession(_uuid);
			return;
		}

		short data_len = 0;
		short msg_id = 0;
		memcpy(&msg_id, _recv_head_node->_data, HEAD_ID_LEN);
		memcpy(&data_len, _recv_head_node->_data + HEAD_ID_LEN, HEAD_DATA_LEN);
		data_len = boost::asio::detail::socket_ops::network_to_host_short(data_len);
		msg_id = boost::asio::detail::socket_ops::network_to_host_short(msg_id);

		std::cout << "Message id is " << msg_id << std::endl;
		std::cout << "Message length is " << data_len << std::endl;

		if (data_len > MAX_LENGTH) {
			std::cout << "invalid data length is " << data_len << std::endl;
			_server->ClearSession(_uuid);
			return;
		}

		_recv_msg_node = std::make_shared<RecvNode>(data_len, data_len, msg_id);
		boost::asio::async_read(_socket, boost::asio::buffer(_recv_msg_node->_data, _recv_msg_node->_total_len),
			std::bind(&CSession::HandleReadMsg, this, std::placeholders::_1, std::placeholders::_2, _self_shared)
		);
	}
	else
	{
		std::cout << "handle read head failed, error is " << error.what() << std::endl;
		Close();
		_server->ClearSession(_uuid);
	}
}

void CSession::HandleReadMsg(const boost::system::error_code& error, std::size_t bytes_transformed, std::shared_ptr<CSession> _self_shared)
{
	if (!error)
	{
		// 处理 _recv_msg_node 中的消息
		std::cout << "recvice message is " << _recv_msg_node->_data << std::endl;

		// 解耦合，调用逻辑层处理消息
		// 将消息节点构造为逻辑节点并放入逻辑队列
		LogicSystem::GetInstance()->PostMsgToQue(std::make_shared<LogicNode>(shared_from_this(), _recv_msg_node));

		// 回调
		_recv_head_node->Clear();
		boost::asio::async_read(_socket, boost::asio::buffer(_data, HEAD_TOTAL_LEN),
			std::bind(&CSession::HandleReadHead, this, std::placeholders::_1, std::placeholders::_2, _self_shared)
		);
	}
	else
	{
		std::cout << "handle read message failed, error is " << error.what() << std::endl;
		Close();
		_server->ClearSession(_uuid);
	}
}

// boost::asio::async_write 的回调函数，async_write是一次性将数据发送完成
void CSession::HandleWrite(const boost::system::error_code& error, std::shared_ptr<CSession> _self_shared)
{
	try
	{
		if (!error) {
			std::lock_guard<std::mutex> lock(_send_lock);
			_send_que.pop();
			if (!_send_que.empty()) {
				auto& msgnode = _send_que.front();
				boost::asio::async_write(_socket, boost::asio::buffer(msgnode->_data, msgnode->_total_len),
					std::bind(&CSession::HandleWrite, this, std::placeholders::_1, _self_shared));
			}
		}
		else {
			std::cout << "handle write failed, error is " << error.what() << std::endl;
			Close();
			_server->ClearSession(_uuid);
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << "Exception code : " << e.what() << std::endl;
	}
	
}

LogicNode::LogicNode(std::shared_ptr<CSession> session, std::shared_ptr<RecvNode> recv_node)
	: _session(session), _recv_node(recv_node)
{
}
