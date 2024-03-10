#include "Session.h"

ip::tcp::socket& Session::Socket()
{
	return _socket;
}
 
void Session::Start()
{
	memset(_data, 0, max_length);
	_socket.async_read_some(
		boost::asio::buffer(_data, max_length),
		std::bind(
			&Session::handle_read,
			this,
			std::placeholders::_1,
			std::placeholders::_2,
			shared_from_this()  // �˴�����ʹ�� share_ptr<>() ������ʹ�� shared_from_this()
		)
	);
	// ��2������shared_ptrָ��ͬһ���ڴ棬���ǵ����ü����ǲ�һ���ģ��ײ����ͷ��Ѿ��ͷŵ��ڴ�
}

const std::string& Session::GetUuid() const
{
	return _uuid;
}

void Session::handle_read(
	const boost::system::error_code& error,
	std::size_t bytes_transformed, 
	std::shared_ptr<Session> _self_shared)
{
	if (!error) {
		std::cout << "Server recive data is: " << _data << std::endl;
		boost::asio::async_write(
			_socket,
			boost::asio::buffer(_data, bytes_transformed),
			std::bind(&Session::handle_write, this, std::placeholders::_1, _self_shared)
		);
	}
	else {
		std::cout << "read error" << std::endl;
		_server->ClearSession(_uuid);
	}

}

void Session::handle_write(const boost::system::error_code& error, std::shared_ptr<Session> _self_shared)
{
	memset(_data, 0, max_length);
	if (!error) {
		_socket.async_read_some(
			boost::asio::buffer(_data, max_length),
			std::bind(&Session::handle_read, this, std::placeholders::_1, std::placeholders::_2, _self_shared)
		);
	}
	else {
		std::cout << "write error" << std::endl;
		_server->ClearSession(_uuid);
	}
}