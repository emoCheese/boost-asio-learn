#pragma once
#include <iostream>
#include <boost/asio.hpp>
#include "Server.h"
#include "boost/uuid/uuid_generators.hpp"
#include "boost/uuid/uuid_io.hpp"

using namespace boost::asio;
using namespace std;

class Server;

class Session : public std::enable_shared_from_this<Session>
{
public:
	Session(boost::asio::io_context& ioc, Server* server)
		: _socket(ioc),
		_server(server)
	{
		boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
		_uuid = boost::uuids::to_string(a_uuid);
	}

	ip::tcp::socket& Socket();  // ªÒ»° socket
	void Start();
	const std::string& GetUuid() const;

private:
	void handle_read(
		const boost::system::error_code& error, 
		std::size_t bytes_transformed,
		std::shared_ptr<Session> _self_shared
	);
	void handle_write(const boost::system::error_code& error, std::shared_ptr<Session> _self_shared);

private:
	ip::tcp::socket _socket;
	enum { max_length = 1024 };
	char _data[max_length];
	Server* _server;
	std::string _uuid;
};


