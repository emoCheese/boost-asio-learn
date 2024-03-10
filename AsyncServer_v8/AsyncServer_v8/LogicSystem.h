#pragma once

#include <queue>
#include <thread>
#include <functional>
#include <mutex>
#include <map>

// 第三方库
#include "json/reader.h"
#include "json/writer.h"
#include "json/json.h"

// 自己编写头文件
#include "Singleton.h"
#include "CSession.h"
#include "const.h"

class LogicNode;
class CSession;

// 为要注册的回调函数类型，其参数为绘画类智能指针，消息id，以及消息内容。
typedef std::function<void(std::shared_ptr<CSession> session, const short& msg_id, const std::string& msg_data)> FunCallBack;     

class LogicSystem : public Singleton<LogicSystem>
{
	friend class Singleton<LogicSystem>;
public:
	~LogicSystem();
	void PostMsgToQue(std::shared_ptr<LogicNode> msg);
private:
	LogicSystem();

	// 将所有处理函数注册到 map 中
	void RegisterCallBack();       
	// 对id为MSG_HELLO_WORLD的消息进行处理
	void HelloWorldCallBack(std::shared_ptr<CSession> session, const short& msg_id, const std::string& msg_data);   
	void DealMsg();  // 处理消息，工作线程调用 

	std::queue<std::shared_ptr<LogicNode>> _msg_que;        // 为逻辑队列
	std::mutex _mutex;
	std::condition_variable _consume;                       // 表示消费者条件变量，用来控制当逻辑队列为空时保证线程暂时挂起等待，不要干扰其他线程。

	std::thread _work_thread;                               // 表示工作线程，用来从逻辑队列中取数据并执行回调函数。在构造函数中启用
	bool _b_stop;                                           // 表示收到外部的停止信号，逻辑类要中止工作线程并优雅退出
	std::map<short, FunCallBack> _fun_callback;            // 表示回调函数的map，根据id查找对应的逻辑处理函数。
};
