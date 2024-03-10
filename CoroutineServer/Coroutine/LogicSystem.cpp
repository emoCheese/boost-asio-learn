#include "LogicSystem.h"

LogicSystem::~LogicSystem()
{
	_b_stop = true;
	_consume.notify_one();
	_work_thread.join();
}

void LogicSystem::PostMsgToQue(std::shared_ptr<LogicNode> msg)
{
	std::unique_lock<std::mutex> u_lock(_mutex);
	_logic_que.push(msg);
	if (_logic_que.size() == 1)
	{
		u_lock.unlock();
		_consume.notify_one();
	}
}

void LogicSystem::Stop()
{
}

// C++11 以上返回局部静态变量是线程安全的
LogicSystem& LogicSystem::GetInstance()
{
	static LogicSystem instance;
	return instance;
}

LogicSystem::LogicSystem()
	: _b_stop(false)
{
	RegisterCallBacks();
	_work_thread = std::thread(&LogicSystem::DealMsg, this);
}

void LogicSystem::DealMsg()
{
	while (true)
	{
		std::unique_lock<std::mutex> u_lock(_mutex);
		while (_logic_que.empty() && !_b_stop)
			_consume.wait(u_lock);

		// 系统停止
		if (_b_stop) 
		{
			while (!_logic_que.empty())
			{
				auto logic_node = _logic_que.front();
				std::cout << "recv msg id is " << logic_node->_recv_node->_msg_id << std::endl;
				auto call_back_iter = _fun_callbacks.find(logic_node->_recv_node->_msg_id);
				if (call_back_iter == _fun_callbacks.end())
				{
					_logic_que.pop();
					continue;
				}
				call_back_iter->second(
					logic_node->_session,
					logic_node->_recv_node->_msg_id,
					std::string(logic_node->_recv_node->GetData(), logic_node->_recv_node->GetTotalLen())
				);
				_logic_que.pop();
			}
			break;
		}

		auto logic_node = _logic_que.front();
		std::cout << "recv msg id is " << logic_node->_recv_node->_msg_id << std::endl;
		auto call_back_iter = _fun_callbacks.find(logic_node->_recv_node->_msg_id);
		if (call_back_iter == _fun_callbacks.end())
		{
			_logic_que.pop();
			continue;
		}
		call_back_iter->second(
			logic_node->_session,
			logic_node->_recv_node->_msg_id,
			std::string(logic_node->_recv_node->GetData(), logic_node->_recv_node->GetTotalLen())
		);
		_logic_que.pop();
	}
}

void LogicSystem::RegisterCallBacks()
{
	_fun_callbacks[MSG_HELLO_WORLD] = std::bind(
		&LogicSystem::HelloWorldCallBack,
		this,
		std::placeholders::_1,
		std::placeholders::_2,
		std::placeholders::_3
	);
}

void LogicSystem::HelloWorldCallBack(std::shared_ptr<CSession> session, const short msg_id, const std::string& msg_data)
{
	Json::Reader reader;
	Json::Value root;
	reader.parse(msg_data, root);
	std::cout << "receive msg id is " << root["id"].asInt()
		<< " msg data is " << root["data"].asString() << std::endl;
	root["data"] = "server hasn recevied message, messsage is " + root["data"].asString();

	std::string return_str = root.toStyledString();
	session->Send(return_str, root["id"].asInt());
}
