#include <iostream>
#include <boost/asio.hpp>
using namespace std;
using namespace boost::asio::ip;
const int MAX_LENGTH = 1024 * 2;
const int HEAD_LENGTH = 2;
const int HEAD_TOTAL_LEN = 4;
int main()
{
	try {
		//创建上下文服务
		boost::asio::io_context   ioc;
		//构造endpoint
		tcp::endpoint  remote_ep(address::from_string("127.0.0.1"), 9998);
		tcp::socket  sock(ioc);
		boost::system::error_code   error = boost::asio::error::host_not_found; ;
		sock.connect(remote_ep, error);
		if (error) {
			cout << "connect failed, code is " << error.value() << " error msg is " << error.message();
			return 0;
		}

		std::cout << "Enter message: ";
		char request[MAX_LENGTH];
		std::cin.getline(request, MAX_LENGTH);
		size_t request_length = strlen(request);
		char send_data[MAX_LENGTH] = { 0 };

		short msg_id = 1005;

		// 转变为网络字节序
		short net_msg_id = boost::asio::detail::socket_ops::host_to_network_short(msg_id);
		short net_request_length = boost::asio::detail::socket_ops::host_to_network_short(request_length);

		memcpy(send_data, &net_msg_id, 2);
		memcpy(send_data + 2, &net_request_length, 2);
		memcpy(send_data + 4, request, request_length);



		boost::asio::write(sock, boost::asio::buffer(send_data, request_length + 4));

		char reply_head[HEAD_TOTAL_LEN];
		size_t reply_length = boost::asio::read(sock, boost::asio::buffer(reply_head, HEAD_TOTAL_LEN));
		short msgid = 0;
		memcpy(&msgid, reply_head, 2);
		short msglen = 0;
		memcpy(&msglen, reply_head + 2, HEAD_LENGTH);

		msgid = boost::asio::detail::socket_ops::network_to_host_short(msgid);
		msglen = boost::asio::detail::socket_ops::network_to_host_short(msglen);

		char msg[MAX_LENGTH] = { 0 };
		size_t  msg_length = boost::asio::read(sock, boost::asio::buffer(msg, msglen));
		std::cout << "Message id is " << msgid << endl;
		std::cout << "Message length is " << msglen << endl;

		std::cout << "Reply is: ";
		std::cout.write(msg, msglen) << endl;
		std::cout << "\n";
		getchar();
	}
	catch (std::exception& e) {
		std::cerr << "Exception: " << e.what() << endl;
	}
	return 0;
}