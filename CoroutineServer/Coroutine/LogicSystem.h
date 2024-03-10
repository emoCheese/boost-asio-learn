#pragma once
#include <queue>
#include <map>
#include <thread>
#include <mutex>
#include <memory>
#include <functional>

#include "json/json.h"
#include "json/value.h"
#include "json/reader.h"

#include "const.h"
#include "CSession.h"

class CSession;
class LogicNode;

typedef 
std::function<void(std::shared_ptr<CSession> session, const short msg_id, const std::string& msg_data)> FunCallBack;

class LogicSystem
{
public:
	~LogicSystem();

	void PostMsgToQue(std::shared_ptr<LogicNode> msg);
	void Stop();

	static LogicSystem& GetInstance();

private:
	LogicSystem();
	LogicSystem(const LogicSystem&) = delete;
	LogicSystem& operator=(const LogicSystem&) = delete;

	void DealMsg();
	void RegisterCallBacks();
	void HelloWorldCallBack(std::shared_ptr<CSession> session, const short msg_id, const std::string& msg_data);

	std::thread _work_thread;
	std::queue<std::shared_ptr<LogicNode>> _logic_que;
	std::mutex _mutex;
	std::condition_variable _consume;
	bool _b_stop;
	std::map<short, FunCallBack> _fun_callbacks;
};

