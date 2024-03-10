#pragma once

#include <queue>
#include <thread>
#include <functional>
#include <mutex>
#include <map>

// ��������
#include "json/reader.h"
#include "json/writer.h"
#include "json/json.h"

// �Լ���дͷ�ļ�
#include "Singleton.h"
#include "CSession.h"
#include "const.h"

class LogicNode;
class CSession;

// ΪҪע��Ļص��������ͣ������Ϊ�滭������ָ�룬��Ϣid���Լ���Ϣ���ݡ�
typedef std::function<void(std::shared_ptr<CSession> session, const short& msg_id, const std::string& msg_data)> FunCallBack;     

class LogicSystem : public Singleton<LogicSystem>
{
	friend class Singleton<LogicSystem>;
public:
	~LogicSystem();
	void PostMsgToQue(std::shared_ptr<LogicNode> msg);
private:
	LogicSystem();

	// �����д�����ע�ᵽ map ��
	void RegisterCallBack();       
	// ��idΪMSG_HELLO_WORLD����Ϣ���д���
	void HelloWorldCallBack(std::shared_ptr<CSession> session, const short& msg_id, const std::string& msg_data);   
	void DealMsg();  // ������Ϣ�������̵߳��� 

	std::queue<std::shared_ptr<LogicNode>> _msg_que;        // Ϊ�߼�����
	std::mutex _mutex;
	std::condition_variable _consume;                       // ��ʾ�����������������������Ƶ��߼�����Ϊ��ʱ��֤�߳���ʱ����ȴ�����Ҫ���������̡߳�

	std::thread _work_thread;                               // ��ʾ�����̣߳��������߼�������ȡ���ݲ�ִ�лص��������ڹ��캯��������
	bool _b_stop;                                           // ��ʾ�յ��ⲿ��ֹͣ�źţ��߼���Ҫ��ֹ�����̲߳������˳�
	std::map<short, FunCallBack> _fun_callback;            // ��ʾ�ص�������map������id���Ҷ�Ӧ���߼���������
};
