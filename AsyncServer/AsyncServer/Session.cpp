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
			shared_from_this()  // 此处不可使用 share_ptr<>() 操作，使用 shared_from_this()
		)
	);
	// 若2个或多个shared_ptr指向同一个内存，它们的引用计数是不一样的，易产生释放已经释放的内存
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