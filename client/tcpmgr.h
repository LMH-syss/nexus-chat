#ifndef TCPMGR_H
#define TCPMGR_H
#include <QTcpSocket>
#include <QTimer>
#include "singleton.h"
#include "global.h"
class TcpMgr:public QObject,
             public Singleton<TcpMgr>,
             public std::enable_shared_from_this<TcpMgr>
{
     Q_OBJECT
public:
    TcpMgr();
private:
    void resetRecvState();

private:
    QTcpSocket _socket;
    QString _host;
    uint16_t _port;
    QByteArray _buffer;
    QTimer* _heartbeat_timer;
    bool _b_recv_pending;//琛ㄧず宸茬粡璇昏繃浜嗗寘澶达紝浣嗗唴瀹规病鏈夎鍙栧畬鏁?
    quint16 _message_id;//娑堟伅绫诲瀷
    quint16 _message_len;//娑堟伅瀛楄妭闀垮害
public slots:
    void slot_tcp_connect(ServerInfo);
    void slot_send_data(ReqId reqid, QString data);

private slots:
    void slot_start_heartbeat();
    void slot_stop_heartbeat();
    void slot_send_heartbeat();

signals:
    void sig_con_success(bool bsuccess);
    void sig_send_data(ReqId reqid, QString data);
    void sig_login_success();
    void sig_login_failed(int error);
    void sig_search_user_result(QJsonObject data);
    void sig_add_friend_result(QJsonObject data);
    void sig_notify_add_friend(QJsonObject data);
    void sig_apply_list_result(QJsonObject data);
    void sig_auth_friend_result(QJsonObject data);
    void sig_notify_auth_friend(QJsonObject data);
    void sig_friend_list_result(QJsonObject data);
    void sig_text_chat_msg_result(QJsonObject data);
    void sig_notify_text_chat_msg(QJsonObject data);
    void sig_chat_history_result(QJsonObject data);

};




#endif // TCPMGR_H
