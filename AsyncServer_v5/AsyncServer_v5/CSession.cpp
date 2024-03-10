#include "CSession.h"


CSession::CSession(boost::asio::io_context& ioc, CServer* server)
	: _socket(ioc), _server(server), _b_head_parse(false), _b_close(false)
{
	boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
	_uuid = boost::uuids::to_string(a_uuid);
	_recv_head_node = make_shared<MsgNode>(HEAD_TOTAL_LEN);
}

ip::tcp::socket& CSession::GetSocket()
{
	return _socket;
}

void CSession::Start()
{
	::memset(_data, 0, MAX_LENGTH);
	//_socket.async_read_some(
	//	boost::asio::buffer(_data, MAX_LENGTH),
	//	std::bind( 
	//		&CSession::HandleRead,
	//		this,
	//		std::placeholders::_1,
	//		std::placeholders::_2,
	//		shared_from_this()  // 没有传入shared_ptr，此处不可使用 share_ptr<>() 操作，使用 shared_from_this()
	//	)
	//);
	// 若2个或多个shared_ptr指向同一个内存，它们的引用计数是不一样的，会造成多次析构


	_recv_head_node->Clear();
	boost::asio::async_read(_socket, boost::asio::buffer(_recv_head_node->_data, HEAD_TOTAL_LEN),
		std::bind(&CSession::HandleReadHead, this, std::placeholders::_1, std::placeholders::_2, shared_from_this())
	);
}

void CSession::Close()
{
	_socket.close();
	_b_close = true;
}

const std::string& CSession::GetUuid() const { return _uuid; }

void CSession::Send(char* msg, short msg_len, short msg_id)
{
	std::lock_guard<std::mutex> lock(_send_lock);
	int que_size = _send_que.size();

	if (que_size > MAX_SENDQUE)
	{
		std::cout << "session: " << _uuid << " send que fulled, size is " << MAX_SENDQUE << endl;
		return;
	}
		
	_send_que.push(make_shared<SendNode>(msg, msg_len, msg_id));
	if (que_size > 0)
		return;

	const auto& msg_node = _send_que.front();
	boost::asio::async_write(
		_socket,
		boost::asio::buffer(msg_node->_data, msg_node->_total_len),
		std::bind(&CSession::HandleWrite, this, std::placeholders::_1, shared_from_this())
	);
}


void CSession::HandleRead(
	const boost::system::error_code& error,
	std::size_t bytes_transforred,
	std::shared_ptr<CSession> _self_shared  // 接受一个指向自己的shared_ptr 用于绑定，增加引用延长生命周期
)  
{
	/*if (!error) {
		std::cout << "Server recive data is: " << _data << std::endl;
		Send(_data, bytes_transformed);

		memset(_data, 0, MAX_LENGTH);
		_socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
			std::bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, _self_shared)
		);
	}
	else {
		std::cout << "handle read failed! error id " << error.what() << std::endl;
		_server->ClearSession(_uuid);
	}*/

}

void CSession::HandleReadHead(const boost::system::error_code& error, std::size_t bytes_transforred, std::shared_ptr<CSession> _self_shared)
{
	if (!error)
	{
		if (bytes_transforred < HEAD_TOTAL_LEN)
		{
			cout << "read head length error";
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
			std::cout << "invalid data length is " << data_len << endl;
			_server->ClearSession(_uuid);
			return;
		}

		_recv_msg_node = std::make_shared<RecvNode>(data_len, msg_id);
		boost::asio::async_read(_socket, boost::asio::buffer(_recv_msg_node->_data, _recv_msg_node->_total_len),
			std::bind(&CSession::HandleReadMsg, this, std::placeholders::_1, std::placeholders::_2, _self_shared)
		);
	}
	else
	{
		std::cout << "handle read head failed, error is " << error.what() << endl;
		Close();
		_server->ClearSession(_uuid);
	}
}

void CSession::HandleReadMsg(const boost::system::error_code& error, std::size_t bytes_transformed, std::shared_ptr<CSession> _self_shared)
{
	if (!error)
	{
		// 处理 _recv_msg_node 中的消息
		std::cout << "recvice message is " << _recv_msg_node->_data << endl;
		Send(_recv_msg_node->_data, _recv_msg_node->_total_len, _recv_msg_node->_msg_id);

		// 回调
		_recv_head_node->Clear();
		boost::asio::async_read(_socket, boost::asio::buffer(_data, HEAD_TOTAL_LEN),
			std::bind(&CSession::HandleReadHead, this, std::placeholders::_1, std::placeholders::_2, _self_shared)
		);
	}
	else
	{
		std::cout << "handle read message failed, error is " << error.what() << endl;
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
			// 调用 HandleWrite 表示数据已经发送完，将已发送完的首个MsgNode pop出
			_send_que.pop();
			// 队列非空，取出首元素继续调用 async_write
			if (!_send_que.empty()) {
				auto& msg_node = _send_que.front();
				boost::asio::async_write(
					_socket,
					boost::asio::buffer(msg_node->_data, msg_node->_cur_len),
					std::bind(&CSession::HandleWrite, this, std::placeholders::_1, _self_shared)
				);
			}
		}
		else {
			std::cout << "handle write failed! error is " << error.what() << std::endl;
			_server->ClearSession(_uuid);
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << "Exception code : " << e.what() << endl;
	}
	
}