#include "CSession.h"


ip::tcp::socket& CSession::Socket()
{
	return _socket;
}

void CSession::Start()
{
	memset(_data, 0, max_length);
	_socket.async_read_some(
		boost::asio::buffer(_data, max_length),
		std::bind( 
			&CSession::HandleRead,
			this,
			std::placeholders::_1,
			std::placeholders::_2,
			shared_from_this()  // 没有传入shared_ptr，此处不可使用 share_ptr<>() 操作，使用 shared_from_this()
		)
	);
	// 若2个或多个shared_ptr指向同一个内存，它们的引用计数是不一样的，会造成多次析构
}

const std::string& CSession::GetUuid() const { return _uuid; }

void CSession::Send(char* msg, std::size_t max_length)
{
	std::lock_guard<std::mutex> lock(_send_lock);
	bool panding = false; // 变量为 false 表示发送队列里无数据，上次发送的数据已经发送完

	if (_send_que.size() > 0) 
		panding = true;
	
	_send_que.push(make_shared<MsgNode>(msg, max_length));
	if (panding)
		return;

	boost::asio::async_write(
		_socket,
		boost::asio::buffer(msg, max_length),
		std::bind(&CSession::HandleWrite, this, std::placeholders::_1, shared_from_this())
	);
}

void CSession::HandleRead(
	const boost::system::error_code& error,
	std::size_t bytes_transformed,
	std::shared_ptr<CSession> _self_shared  // 接受一个指向自己的shared_ptr 用于绑定，增加引用延长生命周期
)  
{
	if (!error) {
		std::cout << "Server recive data is: " << _data << std::endl;
		Send(_data, bytes_transformed);

		memset(_data, 0, max_length);
		_socket.async_read_some(boost::asio::buffer(_data, max_length),
			std::bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, _self_shared)
		);
	}
	else {
		std::cout << "handle read failed! error id " << error.what() << std::endl;
		_server->ClearSession(_uuid);
	}

}

// boost::asio::async_write 的回调函数，async_write是一次性将数据发送完成
void CSession::HandleWrite(const boost::system::error_code& error, std::shared_ptr<CSession> _self_shared)
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