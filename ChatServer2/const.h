#pragma once
#include <functional>


enum ErrorCodes {
	Success = 0,
	Error_Json = 1001,  //Json��������
	RPCFailed = 1002,  //RPC�������
	VarifyExpired = 1003, //��֤�����
	VarifyCodeErr = 1004, //��֤�����
	UserExist = 1005,       //�û��Ѿ�����
	PasswdErr = 1006,    //�������
	EmailNotMatch = 1007,  //���䲻ƥ��
	PasswdUpFailed = 1008,  //��������ʧ��
	PasswdInvalid = 1009,   //�������ʧ��
	TokenInvalid = 1010,   //TokenʧЧ
	UidInvalid = 1011,  //uid��Ч
};


// Defer��
class Defer {
public:
	// ����һ��lambda����ʽ���ߺ���ָ��
	Defer(std::function<void()> func) : func_(func) {}

	// ����������ִ�д���ĺ���
	~Defer() {
		func_();
	}

private:
	std::function<void()> func_;
};

#define MAX_LENGTH  1024*2
//ͷ���ܳ���
#define HEAD_TOTAL_LEN 4
//ͷ��id����
#define HEAD_ID_LEN 2
//ͷ�����ݳ���
#define HEAD_DATA_LEN 2
#define MAX_RECVQUE  10000
#define MAX_SENDQUE 1000


enum MSG_IDS {
	MSG_CHAT_LOGIN = 1005, //�û���½
	MSG_CHAT_LOGIN_RSP = 1006, //�û���½�ذ�
	ID_SEARCH_USER_REQ = 1007, //�û���������
	ID_SEARCH_USER_RSP = 1008, //�����û��ذ�
	ID_ADD_FRIEND_REQ = 1009, //�������Ӻ�������
	ID_ADD_FRIEND_RSP = 1010, //�������Ӻ��ѻظ�
	ID_NOTIFY_ADD_FRIEND_REQ = 1011,  //֪ͨ�û����Ӻ�������
	ID_GET_APPLY_LIST_REQ = 1012, //��ȡ���������б�
	ID_AUTH_FRIEND_REQ = 1013,  //��֤��������
	ID_AUTH_FRIEND_RSP = 1014,  //��֤���ѻظ�
	ID_NOTIFY_AUTH_FRIEND_REQ = 1015, //֪ͨ�û���֤��������
	ID_GET_APPLY_LIST_RSP = 1016, //��ȡ���������б��ظ�
	ID_TEXT_CHAT_MSG_REQ = 1017, //�ı�������Ϣ����
	ID_TEXT_CHAT_MSG_RSP = 1018, //�ı�������Ϣ�ظ�
	ID_NOTIFY_TEXT_CHAT_MSG_REQ = 1019, //֪ͨ�û��ı�������Ϣ
	ID_GET_FRIEND_LIST_REQ = 1020, //��ȡ������ϵ�˱�
	ID_NOTIFY_OFF_LINE_REQ = 1021, //֪ͨ�û�����
	ID_HEART_BEAT_REQ = 1023,      //��������
	ID_GET_FRIEND_LIST_RSP = 1022, //��ȡ������ϵ�˱��ظ�
	ID_HEARTBEAT_RSP = 1024,       //�����ظ�
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

//�ֲ�ʽ���ĳ���ʱ��
#define LOCK_TIME_OUT 10
//�ֲ�ʽ��������ʱ��
#define ACQUIRE_TIME_OUT 5
