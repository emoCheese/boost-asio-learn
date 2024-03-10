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
			shared_from_this()  // û�д���shared_ptr���˴�����ʹ�� share_ptr<>() ������ʹ�� shared_from_this()
		)
	);
	// ��2������shared_ptrָ��ͬһ���ڴ棬���ǵ����ü����ǲ�һ���ģ�����ɶ������
}

const std::string& CSession::GetUuid() const { return _uuid; }

void CSession::Send(char* msg, std::size_t max_length)
{
	std::lock_guard<std::mutex> lock(_send_lock);
	bool panding = false; // ����Ϊ false ��ʾ���Ͷ����������ݣ��ϴη��͵������Ѿ�������

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
	std::shared_ptr<CSession> _self_shared  // ����һ��ָ���Լ���shared_ptr ���ڰ󶨣����������ӳ���������
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

// boost::asio::async_write �Ļص�������async_write��һ���Խ����ݷ������
void CSession::HandleWrite(const boost::system::error_code& error, std::shared_ptr<CSession> _self_shared)
{
	
	if (!error) {
		std::lock_guard<std::mutex> lock(_send_lock);
		// ���� HandleWrite ��ʾ�����Ѿ������꣬���ѷ�������׸�MsgNode pop��
		_send_que.pop();
		// ���зǿգ�ȡ����Ԫ�ؼ������� async_write
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