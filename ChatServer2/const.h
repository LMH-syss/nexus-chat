#pragma once
#include <functional>


enum ErrorCodes {
	Success = 0,
	Error_Json = 1001,  // JSON 解析错误
	RPCFailed = 1002,  // RPC 请求失败
	VarifyExpired = 1003, // 验证码过期
	VarifyCodeErr = 1004, // 验证码错误
	UserExist = 1005,       // 用户已存在
	PasswdErr = 1006,    // 密码错误
	EmailNotMatch = 1007,  // 邮箱不匹配
	PasswdUpFailed = 1008,  // 密码更新失败
	PasswdInvalid = 1009,   // 密码不合法
	TokenInvalid = 1010,   // 令牌失效
	UidInvalid = 1011,  // uid 无效
};


// 延迟执行辅助类
class Defer {
public:
	// 保存一个匿名函数或函数指针
	Defer(std::function<void()> func) : func_(func) {}

	// 析构时执行保存的函数
	~Defer() {
		func_();
	}

private:
	std::function<void()> func_;
};

#define MAX_LENGTH  1024*2
// 包头总长度
#define HEAD_TOTAL_LEN 4
// 包头消息 id 长度
#define HEAD_ID_LEN 2
// 包头数据长度字段长度
#define HEAD_DATA_LEN 2
#define MAX_RECVQUE  10000
#define MAX_SENDQUE 1000


enum MSG_IDS {
	MSG_CHAT_LOGIN = 1005, // 用户登录
	MSG_CHAT_LOGIN_RSP = 1006, // 用户登录响应
	ID_SEARCH_USER_REQ = 1007, // 搜索用户请求
	ID_SEARCH_USER_RSP = 1008, // 搜索用户响应
	ID_ADD_FRIEND_REQ = 1009, // 添加好友请求
	ID_ADD_FRIEND_RSP = 1010, // 添加好友响应
	ID_NOTIFY_ADD_FRIEND_REQ = 1011,  // 通知添加好友请求
	ID_GET_APPLY_LIST_REQ = 1012, // 获取好友申请列表
	ID_AUTH_FRIEND_REQ = 1013,  // 认证好友请求
	ID_AUTH_FRIEND_RSP = 1014,  // 认证好友响应
	ID_NOTIFY_AUTH_FRIEND_REQ = 1015, // 通知认证好友请求
	ID_GET_APPLY_LIST_RSP = 1016, // 获取好友申请列表响应
	ID_TEXT_CHAT_MSG_REQ = 1017, // 文本聊天消息请求
	ID_TEXT_CHAT_MSG_RSP = 1018, // 文本聊天消息响应
	ID_NOTIFY_TEXT_CHAT_MSG_REQ = 1019, // 通知文本聊天消息
	ID_GET_FRIEND_LIST_REQ = 1020, // 获取好友列表请求
	ID_NOTIFY_OFF_LINE_REQ = 1021, // 通知用户离线
	ID_HEART_BEAT_REQ = 1023,      // 心跳请求
	ID_GET_FRIEND_LIST_RSP = 1022, // 获取好友列表响应
	ID_HEARTBEAT_RSP = 1024,       // 心跳响应
	ID_GET_CHAT_HISTORY_REQ = 1025,
	ID_GET_CHAT_HISTORY_RSP = 1026,
};

#define USERIPPREFIX  "uip_"
#define USERTOKENPREFIX  "utoken_"
#define USERSERVERPREFIX  "user_server_"
#define IPCOUNTPREFIX  "ipcount_"
#define USER_BASE_INFO "ubaseinfo_"
#define LOGIN_COUNT  "logincount"
#define NAME_INFO  "nameinfo_"
#define LOCK_PREFIX "lock_"
#define USER_SESSION_PREFIX "usession_"
#define LOCK_COUNT "lockcount"

// 分布式锁超时时间
#define LOCK_TIME_OUT 10
// 获取分布式锁超时时间
#define ACQUIRE_TIME_OUT 5
