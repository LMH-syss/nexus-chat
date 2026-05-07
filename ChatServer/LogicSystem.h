//#pragma once
//#include "const.h"
//
//
//class HttpConnection;
//typedef std::function<void(std::shared_ptr<HttpConnection>)> HttpHandler;
//class LogicSystem :public Singleton<LogicSystem>
//{
//	friend class Singleton<LogicSystem>;
//public:
//	/*~LogicSystem();*/
//	bool HandleGet(std::string, std::shared_ptr<HttpConnection>);
//	bool HandlePost(std::string, std::shared_ptr<HttpConnection>);
//	void RegGet(std::string, HttpHandler handler);
//	void RegPost(std::string, HttpHandler handler);
//private:
//	LogicSystem();
//	std::map<std::string, HttpHandler> _post_handlers;
//	std::map<std::string, HttpHandler> _get_handlers;
//};

#pragma once
#include "Singleton.h"
#include <queue>
#include <thread>
#include <map>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>
#include "CSession.h"
#include "const.h"
#include "MysqLDao.h"

using namespace std;

typedef function<void(shared_ptr<CSession>, const short& msg_id, const string& msg_data)> FunCallBack;

class LogicSystem : public Singleton<LogicSystem>
{
    friend class Singleton<LogicSystem>;

public:
    ~LogicSystem();
    void PostMsgToQue(shared_ptr<LogicNode> msg);
    bool DeliverAddFriendNotify(int from_uid, const std::string& from_name,
                                int to_uid, const std::string& msg);
    bool DeliverAuthFriendNotify(int from_uid, int to_uid, bool agree);
    bool DeliverTextChatNotify(int from_uid, const std::string& from_name,
                               int to_uid, const std::string& message,
                               long long msg_id = 0,
                               const std::string& send_time = "");

private:
    LogicSystem();

    void DealMsg();
    void RegisterCallBacks();
    void LoginHandler(shared_ptr<CSession> session, const short& msg_id, const string& msg_data);
    void SearchUserHandler(shared_ptr<CSession> session, const short& msg_id, const string& msg_data);
    void HeartBeatHandler(shared_ptr<CSession> session, const short& msg_id, const string& msg_data);
    void AddFriendApplyHandler(shared_ptr<CSession> session, const short& msg_id, const string& msg_data);
    void GetApplyListHandler(shared_ptr<CSession> session, const short& msg_id, const string& msg_data);
    void AuthFriendHandler(shared_ptr<CSession> session, const short& msg_id, const string& msg_data);
    void GetFriendListHandler(shared_ptr<CSession> session, const short& msg_id, const string& msg_data);
    void TextChatMsgHandler(shared_ptr<CSession> session, const short& msg_id, const string& msg_data);
    void GetChatHistoryHandler(shared_ptr<CSession> session, const short& msg_id, const string& msg_data);
    bool GetUserServerName(int uid, std::string& server_name);
    bool IsSelfServer(const std::string& server_name);
    bool NotifyRemoteAddFriend(int to_uid, int from_uid, const std::string& from_name,
                               const std::string& msg);
    bool NotifyRemoteAuthFriend(int to_uid, int from_uid, bool agree);
    bool NotifyRemoteTextChatMsg(int to_uid, int from_uid, const std::string& from_name,
                                 const std::string& message);

private:
    std::thread _worker_thread;
    std::queue<shared_ptr<LogicNode>> _msg_que;
    std::mutex _mutex;
    std::condition_variable _consume;
    bool _b_stop;
    std::map<short, FunCallBack> _fun_callbacks;
	MysqlDao _mysql_dao;
};
