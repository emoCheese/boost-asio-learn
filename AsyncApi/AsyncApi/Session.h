#pragma once
#include <iostream>
#include <memory>
#include <functional>
#include "boost/asio.hpp"

using namespace boost;
using namespace std;

const int RECVSIZE = 1024;

class MsgNode
{
	
};

class Session
{
public:
	Session(std::shared_ptr<asio::ip::tcp::socket> socket);
	void Connect(const asio::ip::tcp::endpoint& ep);
	
private:
	std::shared_ptr<asio::ip::tcp::socket> _socket;
};

