#ifndef GLOBAL_H
#define GLOBAL_H
#include <QWidget>
#include <functional>
#include "QStyle"
#include <QRegularExpression>
#include <QDir>
#include <QSettings>
#include <QJsonObject>

//repolish鐢ㄦ潵鍒锋柊qss
extern std::function<void(QWidget*)>repolish;


enum ReqId{
    ID_GET_VARIFY_CODE = 1001, //鑾峰彇楠岃瘉鐮?
    ID_REG_USER = 1002, //娉ㄥ唽鐢ㄦ埛
    ID_RESET_PWD = 1003, //閲嶇疆瀵嗙爜
    ID_LOGIN_USER = 1004, //鐢ㄦ埛鐧诲綍
    ID_CHAT_LOGIN = 1005, //鐧婚檰鑱婂ぉ鏈嶅姟鍣?
    ID_CHAT_LOGIN_RSP= 1006, //鐧婚檰鑱婂ぉ鏈嶅姟鍣ㄥ洖鍖?
    ID_SEARCH_USER_REQ = 1007, //鐢ㄦ埛鎼滅储璇锋眰
    ID_SEARCH_USER_RSP = 1008, //鎼滅储鐢ㄦ埛鍥炲寘
    ID_ADD_FRIEND_REQ = 1009,  //娣诲姞濂藉弸鐢宠
    ID_ADD_FRIEND_RSP = 1010, //鐢宠娣诲姞濂藉弸鍥炲
    ID_NOTIFY_ADD_FRIEND_REQ = 1011,  //閫氱煡鐢ㄦ埛娣诲姞濂藉弸鐢宠
    ID_GET_APPLY_LIST_REQ = 1012, //获取好友申请列表请求
    ID_AUTH_FRIEND_REQ = 1013,  //璁よ瘉濂藉弸璇锋眰
    ID_AUTH_FRIEND_RSP = 1014,  //璁よ瘉濂藉弸鍥炲
    ID_NOTIFY_AUTH_FRIEND_REQ = 1015, //閫氱煡鐢ㄦ埛璁よ瘉濂藉弸鐢宠
    ID_GET_APPLY_LIST_RSP = 1016, //获取好友申请列表响应
    ID_TEXT_CHAT_MSG_REQ  = 1017,  //鏂囨湰鑱婂ぉ淇℃伅璇锋眰
    ID_TEXT_CHAT_MSG_RSP  = 1018,  //鏂囨湰鑱婂ぉ淇℃伅鍥炲
    ID_NOTIFY_TEXT_CHAT_MSG_REQ = 1019, //閫氱煡鐢ㄦ埛鏂囨湰鑱婂ぉ淇℃伅
    ID_GET_FRIEND_LIST_REQ = 1020, //获取好友列表请求
    ID_NOTIFY_OFF_LINE_REQ = 1021, //閫氱煡鐢ㄦ埛涓嬬嚎
    ID_GET_FRIEND_LIST_RSP = 1022, //获取好友列表响应
    ID_HEART_BEAT_REQ = 1023,      //蹇冭烦璇锋眰
    ID_HEARTBEAT_RSP = 1024,       //蹇冭烦鍥炲
    ID_GET_CHAT_HISTORY_REQ = 1025,
    ID_GET_CHAT_HISTORY_RSP = 1026,
};
enum ErrorCodes{
    SUCCESS = 0,
    ERR_JSON = 1, //Json瑙ｆ瀽澶辫触
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
