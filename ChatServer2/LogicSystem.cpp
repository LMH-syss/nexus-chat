#include "LogicSystem.h"
#include <iostream>
#include "ChatGrpcClient.h"
#include "ConfigMgr.h"
#include "UserMgr.h"
#include "RedisMgr.h"
#include "message.grpc.pb.h"
#include <grpcpp/grpcpp.h>

using namespace std;

static message::LoginRsp VerifyLoginWithStatusServer(int uid, const std::string& token) {
    grpc::ClientContext context;
    message::LoginReq request;
    message::LoginRsp reply;
    request.set_uid(uid);
    request.set_token(token);

    auto& cfg = ConfigMgr::Inst();
    auto host = cfg["StatusServer"]["Host"];
    auto port = cfg["StatusServer"]["Port"];
    auto channel = grpc::CreateChannel(host + ":" + port, grpc::InsecureChannelCredentials());
	auto stub = message::StatusService::NewStub(channel);
	auto status = stub->Login(&context, request, &reply);
    if (!status.ok()) {
        reply.set_error(ErrorCodes::RPCFailed);
    }
    return reply;
}
LogicSystem::LogicSystem() : _b_stop(false) {
    RegisterCallBacks();
    _worker_thread = std::thread(&LogicSystem::DealMsg, this);
}

LogicSystem::~LogicSystem() {
    _b_stop = true;
    _consume.notify_one();
    if (_worker_thread.joinable()) {
        _worker_thread.join();
    }
}

void LogicSystem::PostMsgToQue(shared_ptr<LogicNode> msg) {
    std::unique_lock<std::mutex> unique_lk(_mutex);
    _msg_que.push(msg);

    if (_msg_que.size() == 1) {
        unique_lk.unlock();
        _consume.notify_one();
    }
}

void LogicSystem::DealMsg() {
    for (;;) {
        std::unique_lock<std::mutex> unique_lk(_mutex);

        while (_msg_que.empty() && !_b_stop) {
            _consume.wait(unique_lk);
        }

        if (_b_stop) {
            while (!_msg_que.empty()) {
                auto msg_node = _msg_que.front();
                cout << "recv_msg id is " << msg_node->_recvnode->_msg_id << endl;

                auto call_back_iter = _fun_callbacks.find(msg_node->_recvnode->_msg_id);
                if (call_back_iter != _fun_callbacks.end()) {
                    call_back_iter->second(
                        msg_node->_session,
                        msg_node->_recvnode->_msg_id,
                        std::string(msg_node->_recvnode->_data, msg_node->_recvnode->_cur_len)
                    );
                }
                _msg_que.pop();
            }
            break;
        }

        auto msg_node = _msg_que.front();
        cout << "recv_msg id is " << msg_node->_recvnode->_msg_id << endl;

        auto call_back_iter = _fun_callbacks.find(msg_node->_recvnode->_msg_id);
        if (call_back_iter == _fun_callbacks.end()) {
            _msg_que.pop();
            cout << "msg id [" << msg_node->_recvnode->_msg_id << "] handler not found" << endl;
            continue;
        }

        call_back_iter->second(
            msg_node->_session,
            msg_node->_recvnode->_msg_id,
            std::string(msg_node->_recvnode->_data, msg_node->_recvnode->_cur_len)
        );

        _msg_que.pop();
    }
}

void LogicSystem::RegisterCallBacks() {
    _fun_callbacks[MSG_CHAT_LOGIN] = std::bind(
        &LogicSystem::LoginHandler,
        this,
        placeholders::_1,
        placeholders::_2,
        placeholders::_3
    );
    _fun_callbacks[ID_SEARCH_USER_REQ] = std::bind(
        &LogicSystem::SearchUserHandler,
        this,
        placeholders::_1,
        placeholders::_2,
        placeholders::_3
    );
    _fun_callbacks[ID_HEART_BEAT_REQ] = std::bind(
        &LogicSystem::HeartBeatHandler,
        this,
        placeholders::_1,
        placeholders::_2,
        placeholders::_3
    );
    _fun_callbacks[ID_ADD_FRIEND_REQ] = std::bind(
        &LogicSystem::AddFriendApplyHandler,
        this,
        placeholders::_1,
        placeholders::_2,
        placeholders::_3
    );
    _fun_callbacks[ID_GET_APPLY_LIST_REQ] = std::bind(
        &LogicSystem::GetApplyListHandler,
        this,
        placeholders::_1,
        placeholders::_2,
        placeholders::_3
    );
    _fun_callbacks[ID_AUTH_FRIEND_REQ] = std::bind(
        &LogicSystem::AuthFriendHandler,
        this,
        placeholders::_1,
        placeholders::_2,
        placeholders::_3
    );
    _fun_callbacks[ID_GET_FRIEND_LIST_REQ] = std::bind(
        &LogicSystem::GetFriendListHandler,
        this,
        placeholders::_1,
        placeholders::_2,
        placeholders::_3
    );
    _fun_callbacks[ID_TEXT_CHAT_MSG_REQ] = std::bind(
        &LogicSystem::TextChatMsgHandler,
        this,
        placeholders::_1,
        placeholders::_2,
        placeholders::_3
    );
    _fun_callbacks[ID_GET_CHAT_HISTORY_REQ] = std::bind(
        &LogicSystem::GetChatHistoryHandler,
        this,
        placeholders::_1,
        placeholders::_2,
        placeholders::_3
    );
}

bool LogicSystem::GetUserServerName(int uid, std::string& server_name)
{
    if (uid <= 0) {
        return false;
    }

    auto server_key = std::string(USERSERVERPREFIX) + std::to_string(uid);
    return RedisMgr::GetInstance()->Get(server_key, server_name);
}

bool LogicSystem::IsSelfServer(const std::string& server_name)
{
    auto& cfg = ConfigMgr::Inst();
    return cfg["SelfServer"]["Name"] == server_name;
}

bool LogicSystem::DeliverAddFriendNotify(int from_uid, const std::string& from_name,
                                         int to_uid, const std::string& msg)
{
    auto target_session = UserMgr::GetInstance()->GetSession(to_uid);
    if (!target_session) {
        return false;
    }

    Json::Value notify;
    notify["error"] = ErrorCodes::Success;
    notify["from_uid"] = from_uid;
    notify["from_name"] = from_name;
    notify["msg"] = msg.empty() ? "Request to add you as friend" : msg;
    target_session->Send(notify.toStyledString(), ID_NOTIFY_ADD_FRIEND_REQ);
    return true;
}

bool LogicSystem::DeliverAuthFriendNotify(int from_uid, int to_uid, bool agree)
{
    auto target_session = UserMgr::GetInstance()->GetSession(to_uid);
    if (!target_session) {
        return false;
    }

    UserInfo from_user;
    _mysql_dao.GetUserByUid(from_uid, from_user);

    Json::Value notify;
    notify["error"] = ErrorCodes::Success;
    notify["from_uid"] = from_uid;
    notify["from_name"] = from_user.name;
    notify["msg"] = agree ? "Your friend request was accepted" : "Your friend request was rejected";
    target_session->Send(notify.toStyledString(), ID_NOTIFY_AUTH_FRIEND_REQ);
    return true;
}

bool LogicSystem::DeliverTextChatNotify(int from_uid, const std::string& from_name,
                                        int to_uid, const std::string& message,
                                        long long msg_id,
                                        const std::string& send_time)
{
    auto target_session = UserMgr::GetInstance()->GetSession(to_uid);
    if (!target_session) {
        return false;
    }

    std::string sender_name = from_name;
    if (sender_name.empty()) {
        UserInfo from_user;
        if (_mysql_dao.GetUserByUid(from_uid, from_user)) {
            sender_name = from_user.name;
        }
    }

    Json::Value notify;
    notify["error"] = ErrorCodes::Success;
    notify["from_uid"] = from_uid;
    notify["from_name"] = sender_name;
    notify["message"] = message;
    notify["msg_id"] = static_cast<int>(msg_id);
    notify["send_time"] = send_time;
    target_session->Send(notify.toStyledString(), ID_NOTIFY_TEXT_CHAT_MSG_REQ);
    return true;
}

bool LogicSystem::NotifyRemoteAddFriend(int to_uid, int from_uid,
                                        const std::string& from_name,
                                        const std::string& msg)
{
    std::string server_name;
    if (!GetUserServerName(to_uid, server_name)) {
        return false;
    }

    if (IsSelfServer(server_name)) {
        return DeliverAddFriendNotify(from_uid, from_name, to_uid, msg);
    }

    return ChatGrpcClient::NotifyAddFriend(server_name, from_uid, from_name, msg, to_uid);
}

bool LogicSystem::NotifyRemoteAuthFriend(int to_uid, int from_uid, bool agree)
{
    std::string server_name;
    if (!GetUserServerName(to_uid, server_name)) {
        return false;
    }

    if (IsSelfServer(server_name)) {
        return DeliverAuthFriendNotify(from_uid, to_uid, agree);
    }

    return ChatGrpcClient::RplyAddFriend(server_name, from_uid, agree, to_uid);
}

bool LogicSystem::NotifyRemoteTextChatMsg(int to_uid, int from_uid,
                                          const std::string& from_name,
                                          const std::string& message)
{
    std::string server_name;
    if (!GetUserServerName(to_uid, server_name)) {
        return false;
    }

    if (IsSelfServer(server_name)) {
        return DeliverTextChatNotify(from_uid, from_name, to_uid, message);
    }

    return ChatGrpcClient::NotifyTextChatMsg(server_name, from_uid, to_uid, message);
}

void LogicSystem::LoginHandler(shared_ptr<CSession> session, const short& msg_id, const string& msg_data) {
    Json::Reader reader;
    Json::Value root;
    Json::Value rsp;
    if (!reader.parse(msg_data, root) || !root.isMember("uid") || !root.isMember("token")) {
        rsp["error"] = ErrorCodes::Error_Json;
        session->Send(rsp.toStyledString(), MSG_CHAT_LOGIN_RSP);
        return;
    }

    int uid = root["uid"].asInt();
    auto token = root["token"].asString();
    cout << "user login uid is " << uid << " user token is " << token << endl;

    auto login_rsp = VerifyLoginWithStatusServer(uid, token);
    if (login_rsp.error() != ErrorCodes::Success) {
        rsp["error"] = login_rsp.error();
        rsp["uid"] = uid;
        session->Send(rsp.toStyledString(), MSG_CHAT_LOGIN_RSP);
        return;
    }

    // 确认当前用户属于本 ChatServer
    auto& cfg = ConfigMgr::Inst();
    auto self_name = cfg["SelfServer"]["Name"];
    auto server_key = std::string(USERSERVERPREFIX) + std::to_string(uid);
    std::string assigned_server;
    if (!RedisMgr::GetInstance()->Get(server_key, assigned_server) || assigned_server != self_name) {
        std::cout << "uid " << uid << " assigned to " << assigned_server << " but this is " << self_name << std::endl;
        rsp["error"] = ErrorCodes::TokenInvalid;
        rsp["uid"] = uid;
        session->Send(rsp.toStyledString(), MSG_CHAT_LOGIN_RSP);
        return;
    }

    UserInfo user_info;
    if (!_mysql_dao.GetUserByUid(uid, user_info)) {
        rsp["error"] = ErrorCodes::UidInvalid;
        rsp["uid"] = uid;
        session->Send(rsp.toStyledString(), MSG_CHAT_LOGIN_RSP);
        return;
    }

    session->SetUserId(uid);
    UserMgr::GetInstance()->SetUserSession(uid, session);

    // 刷新 Redis 中 token 和 server 映射的 TTL
    auto token_key = std::string(USERTOKENPREFIX) + std::to_string(uid);
    RedisMgr::GetInstance()->SetEx(token_key, token, 600);
    RedisMgr::GetInstance()->SetEx(server_key, self_name, 600);

    rsp["error"] = ErrorCodes::Success;
    rsp["uid"] = user_info.uid;
    rsp["token"] = token;
    rsp["user"] = user_info.name;
    rsp["email"] = user_info.email;
    session->Send(rsp.toStyledString(), MSG_CHAT_LOGIN_RSP);
}

void LogicSystem::SearchUserHandler(shared_ptr<CSession> session, const short& msg_id, const string& msg_data) {
    Json::Reader reader;
    Json::Value root;
    Json::Value rsp;
    if (!reader.parse(msg_data, root) || !root.isMember("keyword")) {
        rsp["error"] = ErrorCodes::Error_Json;
        rsp["results"] = Json::arrayValue;
        session->Send(rsp.toStyledString(), ID_SEARCH_USER_RSP);
        return;
    }

    auto keyword = root["keyword"].asString();
    cout << "Search user keyword: " << keyword << endl;

    std::vector<UserInfo> results;
    if (!_mysql_dao.SearchUserByKeyword(keyword, results)) {
        rsp["error"] = ErrorCodes::RPCFailed;
        rsp["results"] = Json::arrayValue;
        cout << "Search user failed, send ID_SEARCH_USER_RSP" << endl;
        session->Send(rsp.toStyledString(), ID_SEARCH_USER_RSP);
        return;
    }

    cout << "Search user success, result count: " << results.size() << endl;
    rsp["error"] = ErrorCodes::Success;
    Json::Value result_arr(Json::arrayValue);
    for (const auto& user : results) {
        Json::Value item;
        item["uid"] = user.uid;
        item["name"] = user.name;
        item["email"] = user.email;
        item["nick"] = user.name;
        result_arr.append(item);
    }
    rsp["results"] = result_arr;
    cout << "Search user send ID_SEARCH_USER_RSP" << endl;
    session->Send(rsp.toStyledString(), ID_SEARCH_USER_RSP);
}

void LogicSystem::HeartBeatHandler(shared_ptr<CSession> session, const short& msg_id, const string& msg_data) {
    int uid = session->GetUserId();
    Json::Value rsp;
    if (uid <= 0) {
        rsp["error"] = ErrorCodes::UidInvalid;
        session->Send(rsp.toStyledString(), ID_HEARTBEAT_RSP);
        return;
    }

    session->UpdateHeartbeat();

    rsp["error"] = ErrorCodes::Success;
    session->Send(rsp.toStyledString(), ID_HEARTBEAT_RSP);
}

void LogicSystem::AddFriendApplyHandler(shared_ptr<CSession> session, const short& msg_id, const string& msg_data) {
    Json::Reader reader;
    Json::Value root;
    Json::Value rsp;
    if (!reader.parse(msg_data, root) || !root.isMember("to_uid")) {
        rsp["error"] = ErrorCodes::Error_Json;
        rsp["msg"] = "Request format error";
        session->Send(rsp.toStyledString(), ID_ADD_FRIEND_RSP);
        return;
    }

    int to_uid = root["to_uid"].asInt();
    int from_uid = session->GetUserId();

    if (from_uid <= 0) {
        rsp["error"] = ErrorCodes::UidInvalid;
        rsp["msg"] = "Not logged in";
        session->Send(rsp.toStyledString(), ID_ADD_FRIEND_RSP);
        return;
    }

    if (to_uid == from_uid) {
        rsp["error"] = ErrorCodes::UidInvalid;
        rsp["msg"] = "Cannot add yourself";
        session->Send(rsp.toStyledString(), ID_ADD_FRIEND_RSP);
        return;
    }

    if (_mysql_dao.CheckPendingApply(from_uid, to_uid)) {
        rsp["error"] = ErrorCodes::UserExist;
        rsp["msg"] = "Already has pending application";
        session->Send(rsp.toStyledString(), ID_ADD_FRIEND_RSP);
        return;
    }

    UserInfo target_user;
    if (!_mysql_dao.GetUserByUid(to_uid, target_user)) {
        rsp["error"] = ErrorCodes::UidInvalid;
        rsp["msg"] = "Target user not found";
        session->Send(rsp.toStyledString(), ID_ADD_FRIEND_RSP);
        return;
    }

    if (!_mysql_dao.InsertFriendApply(from_uid, to_uid)) {
        rsp["error"] = ErrorCodes::RPCFailed;
        rsp["msg"] = "Server internal error";
        session->Send(rsp.toStyledString(), ID_ADD_FRIEND_RSP);
        return;
    }

    rsp["error"] = ErrorCodes::Success;
    rsp["msg"] = "Friend request sent";
    session->Send(rsp.toStyledString(), ID_ADD_FRIEND_RSP);

    UserInfo from_user;
    if (_mysql_dao.GetUserByUid(from_uid, from_user)) {
        const std::string notify_msg = "Request to add you as friend";
        if (!DeliverAddFriendNotify(from_uid, from_user.name, to_uid, notify_msg)) {
            NotifyRemoteAddFriend(to_uid, from_uid, from_user.name, notify_msg);
        }
    }
}

void LogicSystem::GetApplyListHandler(shared_ptr<CSession> session, const short& msg_id, const string& msg_data) {
    Json::Value rsp;
    int uid = session->GetUserId();

    if (uid <= 0) {
        rsp["error"] = ErrorCodes::UidInvalid;
        rsp["applies"] = Json::arrayValue;
        session->Send(rsp.toStyledString(), ID_GET_APPLY_LIST_RSP);
        return;
    }

    std::vector<ApplyInfo> results;
    if (!_mysql_dao.GetApplyList(uid, results)) {
        rsp["error"] = ErrorCodes::RPCFailed;
        rsp["applies"] = Json::arrayValue;
        session->Send(rsp.toStyledString(), ID_GET_APPLY_LIST_RSP);
        return;
    }

    rsp["error"] = ErrorCodes::Success;
    Json::Value apply_arr(Json::arrayValue);
    for (const auto& info : results) {
        Json::Value item;
        item["from_uid"] = info.from_uid;
        item["from_name"] = info.from_name;
        item["status"] = info.status;
        item["apply_time"] = info.apply_time;
        apply_arr.append(item);
    }
    rsp["applies"] = apply_arr;
    session->Send(rsp.toStyledString(), ID_GET_APPLY_LIST_RSP);
}

void LogicSystem::AuthFriendHandler(shared_ptr<CSession> session, const short& msg_id, const string& msg_data) {
    Json::Reader reader;
    Json::Value root;
    Json::Value rsp;
    if (!reader.parse(msg_data, root) || !root.isMember("from_uid") || !root.isMember("agree")) {
        rsp["error"] = ErrorCodes::Error_Json;
        rsp["from_uid"] = 0;
        rsp["msg"] = "Request format error";
        session->Send(rsp.toStyledString(), ID_AUTH_FRIEND_RSP);
        return;
    }

    int from_uid = root["from_uid"].asInt();
    bool agree = root["agree"].asBool();
    int responder_uid = session->GetUserId();

    if (responder_uid <= 0) {
        rsp["error"] = ErrorCodes::UidInvalid;
        rsp["from_uid"] = 0;
        rsp["msg"] = "Not logged in";
        session->Send(rsp.toStyledString(), ID_AUTH_FRIEND_RSP);
        return;
    }

    if (from_uid == responder_uid) {
        rsp["error"] = ErrorCodes::UidInvalid;
        rsp["from_uid"] = from_uid;
        rsp["msg"] = "Cannot authorize yourself";
        session->Send(rsp.toStyledString(), ID_AUTH_FRIEND_RSP);
        return;
    }

    if (!_mysql_dao.CheckPendingApply(from_uid, responder_uid)) {
        rsp["error"] = ErrorCodes::UidInvalid;
        rsp["from_uid"] = from_uid;
        rsp["msg"] = "No pending application found";
        session->Send(rsp.toStyledString(), ID_AUTH_FRIEND_RSP);
        return;
    }

    int new_status = agree ? 1 : 2;
    if (!_mysql_dao.UpdateApplyStatus(from_uid, responder_uid, new_status)) {
        rsp["error"] = ErrorCodes::RPCFailed;
        rsp["from_uid"] = from_uid;
        rsp["msg"] = "Server internal error";
        session->Send(rsp.toStyledString(), ID_AUTH_FRIEND_RSP);
        return;
    }

    if (agree) {
        if (!_mysql_dao.InsertFriend(from_uid, responder_uid) ||
            !_mysql_dao.InsertFriend(responder_uid, from_uid)) {
            rsp["error"] = ErrorCodes::RPCFailed;
            rsp["from_uid"] = from_uid;
            rsp["msg"] = "Server internal error";
            session->Send(rsp.toStyledString(), ID_AUTH_FRIEND_RSP);
            return;
        }
    }

    rsp["error"] = ErrorCodes::Success;
    rsp["from_uid"] = from_uid;
    rsp["msg"] = agree ? "Friend request accepted" : "Friend request rejected";
    session->Send(rsp.toStyledString(), ID_AUTH_FRIEND_RSP);

    if (!DeliverAuthFriendNotify(responder_uid, from_uid, agree)) {
        NotifyRemoteAuthFriend(from_uid, responder_uid, agree);
    }
}

void LogicSystem::GetFriendListHandler(shared_ptr<CSession> session, const short& msg_id, const string& msg_data) {
    Json::Value rsp;
    int uid = session->GetUserId();

    if (uid <= 0) {
        rsp["error"] = ErrorCodes::UidInvalid;
        rsp["friends"] = Json::arrayValue;
        session->Send(rsp.toStyledString(), ID_GET_FRIEND_LIST_RSP);
        return;
    }

    std::vector<FriendChatInfo> results;
    if (!_mysql_dao.GetFriendListWithChatStatus(uid, results)) {
        rsp["error"] = ErrorCodes::RPCFailed;
        rsp["friends"] = Json::arrayValue;
        session->Send(rsp.toStyledString(), ID_GET_FRIEND_LIST_RSP);
        return;
    }

    rsp["error"] = ErrorCodes::Success;
    Json::Value friend_arr(Json::arrayValue);
    for (const auto& info : results) {
        Json::Value item;
        item["uid"] = info.user.uid;
        item["name"] = info.user.name;
        item["email"] = info.user.email;
        item["last_msg"] = info.last_msg;
        item["last_time"] = info.last_time;
        item["unread_count"] = info.unread_count;
        friend_arr.append(item);
    }
    rsp["friends"] = friend_arr;
    session->Send(rsp.toStyledString(), ID_GET_FRIEND_LIST_RSP);
}

void LogicSystem::TextChatMsgHandler(shared_ptr<CSession> session, const short& msg_id, const string& msg_data) {
    Json::Reader reader;
    Json::Value root;
    Json::Value rsp;
    if (!reader.parse(msg_data, root) || !root.isMember("to_uid") || !root.isMember("message")) {
        rsp["error"] = ErrorCodes::Error_Json;
        rsp["to_uid"] = 0;
        rsp["msg"] = "Request format error";
        session->Send(rsp.toStyledString(), ID_TEXT_CHAT_MSG_RSP);
        return;
    }

    int to_uid = root["to_uid"].asInt();
    auto message = root["message"].asString();
    int from_uid = session->GetUserId();

    if (from_uid <= 0) {
        rsp["error"] = ErrorCodes::UidInvalid;
        rsp["to_uid"] = to_uid;
        rsp["msg"] = "Not logged in";
        session->Send(rsp.toStyledString(), ID_TEXT_CHAT_MSG_RSP);
        return;
    }

    if (to_uid == from_uid) {
        rsp["error"] = ErrorCodes::UidInvalid;
        rsp["to_uid"] = to_uid;
        rsp["msg"] = "Cannot send message to yourself";
        session->Send(rsp.toStyledString(), ID_TEXT_CHAT_MSG_RSP);
        return;
    }

    long long saved_msg_id = 0;
    std::string send_time;
    if (!_mysql_dao.SaveChatMessage(from_uid, to_uid, message, saved_msg_id, send_time)) {
        rsp["error"] = ErrorCodes::RPCFailed;
        rsp["to_uid"] = to_uid;
        rsp["msg"] = "Save message failed";
        session->Send(rsp.toStyledString(), ID_TEXT_CHAT_MSG_RSP);
        return;
    }

    UserInfo from_user;
    auto has_from_user = _mysql_dao.GetUserByUid(from_uid, from_user);

    std::string from_name = has_from_user ? from_user.name : "";
    bool delivered = DeliverTextChatNotify(from_uid, from_name, to_uid, message,
        saved_msg_id, send_time);
    if (!delivered) {
        delivered = NotifyRemoteTextChatMsg(to_uid, from_uid, from_name, message);
    }

    rsp["error"] = ErrorCodes::Success;
    rsp["to_uid"] = to_uid;
    rsp["msg"] = delivered ? "Message sent" : "User not online";
    rsp["msg_id"] = static_cast<int>(saved_msg_id);
    rsp["send_time"] = send_time;
    session->Send(rsp.toStyledString(), ID_TEXT_CHAT_MSG_RSP);
}

void LogicSystem::GetChatHistoryHandler(shared_ptr<CSession> session, const short& msg_id, const string& msg_data) {
    Json::Reader reader;
    Json::Value root;
    Json::Value rsp;
    if (!reader.parse(msg_data, root) || !root.isMember("peer_uid")) {
        rsp["error"] = ErrorCodes::Error_Json;
        rsp["peer_uid"] = 0;
        rsp["messages"] = Json::arrayValue;
        session->Send(rsp.toStyledString(), ID_GET_CHAT_HISTORY_RSP);
        return;
    }

    int self_uid = session->GetUserId();
    int peer_uid = root["peer_uid"].asInt();
    int limit = root.isMember("limit") ? root["limit"].asInt() : 30;
    long long before_msg_id = root.isMember("before_msg_id") ? root["before_msg_id"].asInt() : 0;
    bool mark_read = root.isMember("mark_read") ? root["mark_read"].asBool() : true;
    bool only_mark_read = root.isMember("only_mark_read") ? root["only_mark_read"].asBool() : false;

    if (self_uid <= 0 || peer_uid <= 0 || self_uid == peer_uid) {
        rsp["error"] = ErrorCodes::UidInvalid;
        rsp["peer_uid"] = peer_uid;
        rsp["messages"] = Json::arrayValue;
        session->Send(rsp.toStyledString(), ID_GET_CHAT_HISTORY_RSP);
        return;
    }

    if (only_mark_read) {
        if (mark_read) {
            _mysql_dao.MarkChatMessagesRead(self_uid, peer_uid);
        }
        rsp["error"] = ErrorCodes::Success;
        rsp["peer_uid"] = peer_uid;
        rsp["only_mark_read"] = true;
        rsp["messages"] = Json::arrayValue;
        session->Send(rsp.toStyledString(), ID_GET_CHAT_HISTORY_RSP);
        return;
    }

    std::vector<ChatMessageInfo> results;
    if (!_mysql_dao.GetChatHistory(self_uid, peer_uid, limit, before_msg_id, results)) {
        rsp["error"] = ErrorCodes::RPCFailed;
        rsp["peer_uid"] = peer_uid;
        rsp["messages"] = Json::arrayValue;
        session->Send(rsp.toStyledString(), ID_GET_CHAT_HISTORY_RSP);
        return;
    }

    if (mark_read) {
        _mysql_dao.MarkChatMessagesRead(self_uid, peer_uid);
    }

    rsp["error"] = ErrorCodes::Success;
    rsp["peer_uid"] = peer_uid;
    Json::Value msg_arr(Json::arrayValue);
    for (const auto& info : results) {
        Json::Value item;
        item["msg_id"] = static_cast<int>(info.msg_id);
        item["from_uid"] = info.from_uid;
        item["to_uid"] = info.to_uid;
        item["message"] = info.content;
        item["send_time"] = info.send_time;
        item["read_status"] = info.read_status;
        msg_arr.append(item);
    }
    rsp["messages"] = msg_arr;
    session->Send(rsp.toStyledString(), ID_GET_CHAT_HISTORY_RSP);
}
