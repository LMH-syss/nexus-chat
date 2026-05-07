#ifndef GLOBAL_H
#define GLOBAL_H
#include <QWidget>
#include <functional>
#include "QStyle"
#include <QRegularExpression>
#include <QDir>
#include <QSettings>
#include <QJsonObject>

// repolish 用于刷新 qss
extern std::function<void(QWidget*)>repolish;


enum ReqId{
    ID_GET_VARIFY_CODE = 1001, // 获取验证码
    ID_REG_USER = 1002, // 注册用户
    ID_RESET_PWD = 1003, // 重置密码
    ID_LOGIN_USER = 1004, // 用户登录
    ID_CHAT_LOGIN = 1005, // 登录聊天服务器
    ID_CHAT_LOGIN_RSP= 1006, // 聊天服务器登录响应
    ID_SEARCH_USER_REQ = 1007, // 搜索用户请求
    ID_SEARCH_USER_RSP = 1008, // 搜索用户响应
    ID_ADD_FRIEND_REQ = 1009,  // 添加好友请求
    ID_ADD_FRIEND_RSP = 1010, // 添加好友响应
    ID_NOTIFY_ADD_FRIEND_REQ = 1011,  // 通知添加好友请求
    ID_GET_APPLY_LIST_REQ = 1012, //获取好友申请列表请求
    ID_AUTH_FRIEND_REQ = 1013,  // 认证好友请求
    ID_AUTH_FRIEND_RSP = 1014,  // 认证好友响应
    ID_NOTIFY_AUTH_FRIEND_REQ = 1015, // 通知认证好友请求
    ID_GET_APPLY_LIST_RSP = 1016, //获取好友申请列表响应
    ID_TEXT_CHAT_MSG_REQ  = 1017,  // 文本聊天消息请求
    ID_TEXT_CHAT_MSG_RSP  = 1018,  // 文本聊天消息响应
    ID_NOTIFY_TEXT_CHAT_MSG_REQ = 1019, // 通知文本聊天消息
    ID_GET_FRIEND_LIST_REQ = 1020, //获取好友列表请求
    ID_NOTIFY_OFF_LINE_REQ = 1021, // 通知用户离线
    ID_GET_FRIEND_LIST_RSP = 1022, //获取好友列表响应
    ID_HEART_BEAT_REQ = 1023,      // 心跳请求
    ID_HEARTBEAT_RSP = 1024,       // 心跳响应
    ID_GET_CHAT_HISTORY_REQ = 1025,
    ID_GET_CHAT_HISTORY_RSP = 1026,
};
enum ErrorCodes{
    SUCCESS = 0,
    ERR_JSON = 1, // JSON 解析失败
    ERR_NETWORK = 2,

};

enum TipErr{
    TIP_SUCCESS = 0,
    TIP_EMAIL_ERR = 1,
    TIP_PWD_ERR = 2,
    TIP_CONFIRM_ERR = 3,
    TIP_PWD_CONFIRM = 4,
    TIP_VARIFY_ERR = 5,
    TIP_USER_ERR = 6
};

enum Modules{
    REGISTERMOD = 0,
    RESETMOD = 1,
    LOGINMOD = 2
};

enum ClickLbState{
    Normal = 0,
    Selected = 1
};

struct ServerInfo
{
    QString Host;
    QString Port;
    QString Token;
    int Uid;
};

extern QString gate_url_prefix;

#endif // GLOBAL_H
