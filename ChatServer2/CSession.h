#pragma once
#include <boost/asio.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast.hpp>
#include <queue>
#include <mutex>
#include <memory>
#include <atomic>
#include <ctime>
#include "const.h"
#include "MsgNode.h"

using namespace std;

namespace beast = boost::beast;         // 来自 <boost/beast.hpp>
namespace http = beast::http;           // 来自 <boost/beast/http.hpp>
namespace net = boost::asio;            // 来自 <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // 来自 <boost/asio/ip/tcp.hpp>

class CServer;
class LogicSystem;

class CSession : public std::enable_shared_from_this<CSession>
{
public:
    CSession(boost::asio::io_context& io_context, CServer* server);
    ~CSession();

    tcp::socket& GetSocket();
    std::string& GetSessionId();

    void Start();
    void Close();

    void AsyncReadHead(int total_len);
    void AsyncReadBody(int total_len);

    void SetUserId(int uid);
    int GetUserId();
    void UpdateHeartbeat();
     
    // 向客户端发送消息
    void Send(std::string msg, short msgid);

private:
    void asyncReadFull(std::size_t maxLength,
        std::function<void(const boost::system::error_code&, std::size_t)> handler);

    void asyncReadLen(std::size_t read_len, std::size_t total_len,
        std::function<void(const boost::system::error_code&, std::size_t)> handler);
    void asyncWriteQueued();

    int _user_uid;

private:
    tcp::socket _socket;
    std::string _session_id;
    char _data[MAX_LENGTH];
    CServer* _server;

    std::shared_ptr<RecvNode> _recv_msg_node;
    std::shared_ptr<MsgNode> _recv_head_node;
    std::atomic<time_t> _last_heartbeat = 0;
    std::queue<std::shared_ptr<SendNode>> _send_que;
    std::mutex _send_lock;
    bool _send_pending = false;
};

class LogicNode {
    friend class LogicSystem;
public:
    LogicNode(shared_ptr<CSession>, shared_ptr<RecvNode>);
private:
    shared_ptr<CSession> _session;
    shared_ptr<RecvNode> _recvnode;
};
