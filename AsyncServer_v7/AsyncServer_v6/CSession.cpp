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
			shared_from_this()  // û�д���shared_ptr���˴�����ʹ�� share_ptr<>() ������ʹ�� shared_from_this()
		)
	);
	// ��2������shared_ptrָ��ͬһ���ڴ棬���ǵ����ü����ǲ�һ���ģ�����ɶ������
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
			//�Ѿ��ƶ����ַ���
			int copy_len = 0;
			while (bytes_transferred > 0) {
				if (!_b_head_parse) {
					//�յ������ݲ���ͷ����С
					if (bytes_transferred + _recv_head_node->_cur_len < HEAD_TOTAL_LEN) {
						memcpy(_recv_head_node->_data + _recv_head_node->_cur_len, _data + copy_len, bytes_transferred);
						_recv_head_node->_cur_len += bytes_transferred;
						::memset(_data, 0, MAX_LENGTH);
						_socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
							std::bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, shared_self));
						return;
					}
					//�յ������ݱ�ͷ����
					//ͷ��ʣ��δ���Ƶĳ���
					int head_remain = HEAD_TOTAL_LEN - _recv_head_node->_cur_len;
					memcpy(_recv_head_node->_data + _recv_head_node->_cur_len, _data + copy_len, head_remain);
					//�����Ѵ����data���Ⱥ�ʣ��δ����ĳ���
					copy_len += head_remain;
					bytes_transferred -= head_remain;
					//��ȡͷ��MSGID����
					short msg_id = 0;
					memcpy(&msg_id, _recv_head_node->_data, HEAD_ID_LEN);
					//�����ֽ���ת��Ϊ�����ֽ���
					msg_id = boost::asio::detail::socket_ops::network_to_host_short(msg_id);
					std::cout << "msg_id is " << msg_id << std::endl;
					//id�Ƿ�
					if (msg_id > MAX_LENGTH) {
						std::cout << "invalid msg_id is " << msg_id << std::endl;
						_server->ClearSession(_uuid);
						return;
					}
					short msg_len = 0;
					memcpy(&msg_len, _recv_head_node->_data + HEAD_ID_LEN, HEAD_DATA_LEN);
					//�����ֽ���ת��Ϊ�����ֽ���
					msg_len = boost::asio::detail::socket_ops::network_to_host_short(msg_len);
					std::cout << "msg_len is " << msg_len << std::endl;
					//id�Ƿ�
					if (msg_len > MAX_LENGTH) {
						std::cout << "invalid data length is " << msg_len << std::endl;
						_server->ClearSession(_uuid);
						return;
					}

					_recv_msg_node = std::make_shared<RecvNode>(msg_len, msg_id);

					//��Ϣ�ĳ���С��ͷ���涨�ĳ��ȣ�˵������δ��ȫ�����Ƚ�������Ϣ�ŵ����սڵ���
					if (bytes_transferred < msg_len) {
						memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len, bytes_transferred);
						_recv_msg_node->_cur_len += bytes_transferred;
						::memset(_data, 0, MAX_LENGTH);
						_socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
							std::bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, shared_self));
						//ͷ���������
						_b_head_parse = true;
						return;
					}

					memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len, msg_len);
					_recv_msg_node->_cur_len += msg_len;
					copy_len += msg_len;
					bytes_transferred -= msg_len;
					_recv_msg_node->_data[_recv_msg_node->_total_len] = '\0';
					//cout << "receive data is " << _recv_msg_node->_data << endl;
					//�˴�����ϢͶ�ݵ��߼�������
					LogicSystem::GetInstance()->PostMsgToQue(std::make_shared<LogicNode>(shared_from_this(), _recv_msg_node));

					//������ѯʣ��δ��������
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

				//�Ѿ�������ͷ���������ϴ�δ���������Ϣ����
				//���յ������Բ���ʣ��δ�����
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
				//�˴�����ϢͶ�ݵ��߼�������
				LogicSystem::GetInstance()->PostMsgToQue(std::make_shared<LogicNode>(shared_from_this(), _recv_msg_node));

				//������ѯʣ��δ��������
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
		// ���� _recv_msg_node �е���Ϣ
		std::cout << "recvice message is " << _recv_msg_node->_data << std::endl;

		// ����ϣ������߼��㴦����Ϣ
		// ����Ϣ�ڵ㹹��Ϊ�߼��ڵ㲢�����߼�����
		LogicSystem::GetInstance()->PostMsgToQue(std::make_shared<LogicNode>(shared_from_this(), _recv_msg_node));

		// �ص�
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

// boost::asio::async_write �Ļص�������async_write��һ���Խ����ݷ������
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
