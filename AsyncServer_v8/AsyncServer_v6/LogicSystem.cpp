#include "LogicSystem.h"


LogicSystem::~LogicSystem()
{
	_b_stop = true;
	_consume.notify_one(); // 唤醒 工作线程
	_work_thread.join();   // 等待工作线程退出
}

void LogicSystem::PostMsgToQue(std::shared_ptr<LogicNode> msg)
{
	std::unique_lock<std::mutex> unique_lk(_mutex);
	_msg_que.push(msg);

	if (_msg_que.size() == 1)
	{
		unique_lk.unlock();
		_consume.notify_one();
	}
}

LogicSystem::LogicSystem()
	: _b_stop(false)
{
	RegisterCallBack();
	// 启动线程，处理消息
	_work_thread = std::thread(&LogicSystem::DealMsg, this);
}

void LogicSystem::RegisterCallBack()
{
	_fun_callback[MSG_HELLO_WORLD] =
		std::bind(
			&LogicSystem::HelloWorldCallBack, 
			this, 
			std::placeholders::_1, std::placeholders::_2, std::placeholders::_3
		);
}


void LogicSystem::HelloWorldCallBack(std::shared_ptr<CSession> session, const short& msg_id, const std::string& msg_data)
{
	Json::Reader reader;
	Json::Value root;
	reader.parse(msg_data, root);
	std::cout << "recvice msg id is " << root["id"].asInt() << " msg data is " << root["data"].asString() << std::endl;

	root["data"] = "server has recevied message, message data is " + root["data"].asString();
	std::string return_str = root.toStyledString();
	session->Send(return_str.c_str(), return_str.length(), msg_id);
}

void LogicSystem::DealMsg()
{
	while (true)
	{
		std::unique_lock<std::mutex> unique_lk(_mutex);  // 配合 condition_variable 使用，可随时解锁

		// 判断队列为空则用条件变量等待
		while (_msg_que.empty() && !_b_stop)
			_consume.wait(unique_lk); // 先释放资源再解锁并挂起

		if (_b_stop)  // 若服务器停止，取出逻辑队列里所有数据并退出循环
		{
			while (!_msg_que.empty()) // empty 为 true 时队列为空
			{
				auto msg_node = _msg_que.front();
				std::cout << "recv msg id is " << msg_node->_recv_node->_msg_id << std::endl;

				auto call_back_iter = _fun_callback.find(msg_node->_recv_node->_msg_id);
				if (call_back_iter == _fun_callback.end())
				{
					_msg_que.pop();
					continue;
				}
				call_back_iter->second(
					msg_node->_session, 
					msg_node->_recv_node->_msg_id, 
					std::string(msg_node->_recv_node->_data, msg_node->_recv_node->_cur_len)
				);
				_msg_que.pop();
			}
			break;
		}
		auto msg_node = _msg_que.front();
		std::cout << "recv msg id is " << msg_node->_recv_node->_msg_id << std::endl;

		// 找到对应消息的 回调处理函数
		auto call_back_iter = _fun_callback.find(msg_node->_recv_node->_msg_id);
		if (call_back_iter == _fun_callback.end())
		{
			_msg_que.pop();
			continue;
		}
		// 调用对应消息的 回调处理函数 对消息进行处理
		call_back_iter->second(
			msg_node->_session,
			msg_node->_recv_node->_msg_id,
			std::string(msg_node->_recv_node->_data, msg_node->_recv_node->_cur_len)
		);
		_msg_que.pop();
	}
}
