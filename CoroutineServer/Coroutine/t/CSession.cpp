#include "CSession.h"


#if defined(BOOST_ASIO_ENABLE_HANDLER_TRACKING)
# define use_awaitable \
  boost::asio::use_awaitable_t(__FILE__, __LINE__, __PRETTY_FUNCTION__)
#endif

CSession::CSession(boost::asio::io_context& io_context, CServer* server)
	: _io_context(io_context), _server(server), _socket(io_context), _b_close(false)
{
	_recv_head_node = std::make_shared<MsgNode>(HEAD_TOTAL_LEN);
	boost::uuids::uuid uuid = boost::uuids::random_generator()();
	_uuid = boost::uuids::to_string(uuid);
}

CSession::~CSession()
{
	try
	{
		std::cout << "CSession destructed" << std::endl;
		Close();
	}
	catch (const std::exception& e)
	{
		std::cout << "exception is " << e.what() << std::endl;
	}
}

void CSession::Start()
{
	auto shared_this = shared_from_this();
	// ����Э�̽���
	boost::asio::co_spawn(_io_context, 
		[shared_this,this]()->boost::asio::awaitable<void> {
			try
			{
				while (!_b_close)
				{
					_recv_head_node->Clear();
					std::size_t n = 
						co_await boost::asio::async_read(
							_socket, 
							boost::asio::buffer(_recv_head_node->GetData(), HEAD_TOTAL_LEN), 
							boost::asio::use_awaitable);
					if (n == 0)
					{
						std::cout << "recvice peer closed" << std::endl;
						Close();
						_server->ClearSession(_uuid);
						co_return;
					}

					short msg_id = 0;
					memcpy(&msg_id, _recv_head_node->GetData(), HEAD_ID_LEN);
					msg_id = boost::asio::detail::socket_ops::network_to_host_short(msg_id);
					short msg_len = 0;
					memcpy(&msg_len, _recv_head_node->GetData() + HEAD_ID_LEN, HEAD_DATA_LEN);
					msg_len = boost::asio::detail::socket_ops::network_to_host_short(msg_len);

					// ���ݳ��ȷǷ�
					if (msg_len > MAX_LENGTH) 
					{
						std::cout << "invalid message length is " << msg_len << std::endl;
						Close();
						_server->ClearSession(_uuid);
						co_return;
					}


					_recv_msg_node = std::make_shared<RecvNode>(msg_len, msg_id);
					n = co_await 
						boost::asio::async_read(
							_socket,
							boost::asio::buffer(_recv_msg_node->GetData(), _recv_msg_node->GetTotalLen()), 
							boost::asio::use_awaitable
						);

					if (n == 0)
					{
						std::cout << "recvice peer closed" << std::endl;
						Close();
						_server->ClearSession(_uuid);
						co_return;
					}
					_recv_msg_node->GetData()[_recv_msg_node->GetTotalLen()] = '\0';

					// ���յ������ݽڵ㷢�͸��߼��̴߳���
					LogicSystem::GetInstance()
						.PostMsgToQue(std::make_shared<LogicNode>(shared_from_this(), _recv_msg_node));
				}
			}
			catch (const std::exception& e)
			{
				std::cout << "exception is " << e.what() << std::endl;
				Close();
				_server->ClearSession(_uuid);
			}
		}, 
		boost::asio::detached);   // detached ʹЭ�������̷߳��룬��Э������ʱ����ִ�����߳�
}

void CSession::Close()
{
	_socket.close();
	_b_close = true;
}

boost::asio::ip::tcp::socket& CSession::GetSocket()
{
	return _socket;
}

const std::string& CSession::GetUuid() const
{
	return _uuid;
}

void CSession::Send(const char* msg, short msg_len, short msg_id)
{
	std::unique_lock<std::mutex> u_lock(_send_lock);
	int send_que_size = _send_que.size();
	if (send_que_size > MAX_SENDQUE_SIZE)
	{
		std::cout << "session: " << _uuid << " send queue fulled, size is " 
			<< MAX_SENDQUE_SIZE << std::endl;
		return;
	}
	_send_que.push(std::make_shared<SendNode>(msg, msg_len, msg_id));
	// ���Ͷ�������δ���������
	if (send_que_size > 0) 
		return;
	
	auto msg_node = _send_que.front();
	// �ڷ��ʷ��Ͷ���ʱ��ʱ������ȡ�������
	u_lock.unlock(); 
	// async_write һ���Է��������ٵ��ûص�����
	boost::asio::async_write(
		_socket,
		boost::asio::buffer(msg_node->GetData(), msg_node->GetTotalLen()),
		std::bind(&CSession::HandleWrite, this, std::placeholders::_1, shared_from_this())
	);
}

void CSession::Send(std::string msg, short msg_id)
{
	Send(msg.c_str(), msg.length(), msg_id);
}

void CSession::HandleWrite(const boost::system::error_code& error, std::shared_ptr<CSession> share_self)
{
	try
	{
		if (!error)
		{
			std::unique_lock<std::mutex> u_lock(_send_lock);
			// ���ѷ���������� pop ������
			_send_que.pop();
			// ���Ͷ��в�Ϊ�գ���������
			if (!_send_que.size())
			{
				auto msg_node = _send_que.front(); 
				u_lock.unlock();
				// async_write һ���Է��������ٵ��ûص�����
				boost::asio::async_write(
					_socket,
					boost::asio::buffer(msg_node->GetData(), msg_node->GetTotalLen()),
					std::bind(&CSession::HandleWrite, this, std::placeholders::_1, shared_from_this())
				);
			}
		}
		else
		{
			std::cout << "handle write failed, error is " << error.what() << std::endl;
			Close();
			_server->ClearSession(_uuid);
		}
	}
	catch (const std::exception& e)
	{
		std::cout << "exception is " << e.what() << std::endl;
	}
}

LogicNode::LogicNode(std::shared_ptr<CSession> session, std::shared_ptr<RecvNode> recv_node)
	: _session(session), _recv_node(recv_node)
{
}
