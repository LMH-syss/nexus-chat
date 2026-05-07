#include "tcpmgr.h"
#include <QDataStream>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QNetworkProxy>
#include "usermgr.h"

namespace {
constexpr int kMaxTcpBodyLen = 1024 * 2;
}

TcpMgr::TcpMgr()
    :_host(""),
     _port(0),
     _b_recv_pending(false),
     _message_id(0),
     _message_len(0)
{
    _socket.setProxy(QNetworkProxy(QNetworkProxy::NoProxy));
    qDebug() << "[TcpMgr] socket proxy set to NoProxy";

    QObject::connect(&_socket, &QTcpSocket::connected, [this](){
        qDebug() << "Connected to server!"
                 << " host=" << _host
                 << " port=" << _port
                 << " state=" << _socket.state();
        emit sig_con_success(true);
    });

    QObject::connect(&_socket, &QTcpSocket::disconnected, [this]() {
        qDebug() << "Disconnected from server.";
        slot_stop_heartbeat();
    });

    QObject::connect(&_socket, &QTcpSocket::readyRead, [&](){
        _buffer.append(_socket.readAll());//

        forever{//
            if(!_b_recv_pending){
                if (_buffer.size() < static_cast<int>(sizeof(quint16) * 2)) {
                    return; //
                }
                //
                const auto* data = reinterpret_cast<const uchar*>(_buffer.constData());
                _message_id = static_cast<quint16>((data[0] << 8) | data[1]);
                _message_len = static_cast<quint16>((data[2] << 8) | data[3]);
                if (_message_len > kMaxTcpBodyLen) {
                    qWarning() << "[TcpMgr] invalid message length:" << _message_len
                               << ", max=" << kMaxTcpBodyLen;
                    resetRecvState();
                    _socket.abort();
                    return;
                }
                //
                _buffer.remove(0, sizeof(quint16) * 2);

                qDebug() << "Message ID:" << _message_id << ", Length:" << _message_len;
            }

            //
            if(_buffer.size() < _message_len){
                _b_recv_pending = true;
                return;
            }

             _b_recv_pending = false;

            QByteArray messageBody = _buffer.mid(0, _message_len);

            qDebug() << "receive body msg is " << messageBody ;

            if (_message_id == ReqId::ID_CHAT_LOGIN_RSP) {
                QJsonParseError parse_error;
                auto json_doc = QJsonDocument::fromJson(messageBody, &parse_error);
                if (parse_error.error != QJsonParseError::NoError || !json_doc.isObject()) {
                    emit sig_login_failed(ErrorCodes::ERR_JSON);
                } else {
                    auto json_obj = json_doc.object();
                    auto error = json_obj["error"].toInt();
                    if (error == ErrorCodes::SUCCESS) {
                        UserInfo user_info;
                        user_info.uid = json_obj["uid"].toInt();
                        user_info.name = json_obj["user"].toString();
                        user_info.email = json_obj["email"].toString();
                        user_info.token = json_obj["token"].toString();
                        UserMgr::GetInstance()->SetUserInfo(user_info);
                        emit sig_login_success();
                    } else {
                        emit sig_login_failed(error);
                    }
                }
            } else if (_message_id == ReqId::ID_SEARCH_USER_RSP) {
                QJsonParseError parse_error;
                auto json_doc = QJsonDocument::fromJson(messageBody, &parse_error);
                if (parse_error.error == QJsonParseError::NoError && json_doc.isObject()) {
                    emit sig_search_user_result(json_doc.object());
                }
            } else if (_message_id == ReqId::ID_HEARTBEAT_RSP) {
                qDebug() << "heartbeat response received";
            } else if (_message_id == ReqId::ID_ADD_FRIEND_RSP) {
                QJsonParseError parse_error;
                auto json_doc = QJsonDocument::fromJson(messageBody, &parse_error);
                if (parse_error.error == QJsonParseError::NoError && json_doc.isObject()) {
                    emit sig_add_friend_result(json_doc.object());
                }
            } else if (_message_id == ReqId::ID_NOTIFY_ADD_FRIEND_REQ) {
                QJsonParseError parse_error;
                auto json_doc = QJsonDocument::fromJson(messageBody, &parse_error);
                if (parse_error.error == QJsonParseError::NoError && json_doc.isObject()) {
                    emit sig_notify_add_friend(json_doc.object());
                }
            } else if (_message_id == ReqId::ID_GET_APPLY_LIST_RSP) {
                QJsonParseError parse_error;
                auto json_doc = QJsonDocument::fromJson(messageBody, &parse_error);
                if (parse_error.error == QJsonParseError::NoError && json_doc.isObject()) {
                    emit sig_apply_list_result(json_doc.object());
                }
            } else if (_message_id == ReqId::ID_AUTH_FRIEND_RSP) {
                QJsonParseError parse_error;
                auto json_doc = QJsonDocument::fromJson(messageBody, &parse_error);
                if (parse_error.error == QJsonParseError::NoError && json_doc.isObject()) {
                    emit sig_auth_friend_result(json_doc.object());
                }
            } else if (_message_id == ReqId::ID_NOTIFY_AUTH_FRIEND_REQ) {
                QJsonParseError parse_error;
                auto json_doc = QJsonDocument::fromJson(messageBody, &parse_error);
                if (parse_error.error == QJsonParseError::NoError && json_doc.isObject()) {
                    emit sig_notify_auth_friend(json_doc.object());
                }
            } else if (_message_id == ReqId::ID_GET_FRIEND_LIST_RSP) {
                QJsonParseError parse_error;
                auto json_doc = QJsonDocument::fromJson(messageBody, &parse_error);
                if (parse_error.error == QJsonParseError::NoError && json_doc.isObject()) {
                    emit sig_friend_list_result(json_doc.object());
                }
            } else if (_message_id == ReqId::ID_TEXT_CHAT_MSG_RSP) {
                QJsonParseError parse_error;
                auto json_doc = QJsonDocument::fromJson(messageBody, &parse_error);
                if (parse_error.error == QJsonParseError::NoError && json_doc.isObject()) {
                    emit sig_text_chat_msg_result(json_doc.object());
                }
            } else if (_message_id == ReqId::ID_NOTIFY_TEXT_CHAT_MSG_REQ) {
                QJsonParseError parse_error;
                auto json_doc = QJsonDocument::fromJson(messageBody, &parse_error);
                if (parse_error.error == QJsonParseError::NoError && json_doc.isObject()) {
                    emit sig_notify_text_chat_msg(json_doc.object());
                }
            } else if (_message_id == ReqId::ID_GET_CHAT_HISTORY_RSP) {
                QJsonParseError parse_error;
                auto json_doc = QJsonDocument::fromJson(messageBody, &parse_error);
                if (parse_error.error == QJsonParseError::NoError && json_doc.isObject()) {
                    emit sig_chat_history_result(json_doc.object());
                }
            }

            _buffer.remove(0, _message_len);

        }

    });

    QObject::connect(&_socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred), [&](QAbstractSocket::SocketError socketError) {
        qDebug() << "[TcpMgr] socket error=" << socketError
                 << ", errorString=" << _socket.errorString()
                 << ", state=" << _socket.state()
                 << ", host=" << _host
                 << ", port=" << _port;
        resetRecvState();
        _socket.abort();
        emit sig_con_success(false);
    });

    QObject::connect(this,&TcpMgr::sig_send_data,this,&TcpMgr::slot_send_data);

    _heartbeat_timer = new QTimer(this);
    QObject::connect(this, &TcpMgr::sig_login_success,
                     this, &TcpMgr::slot_start_heartbeat);
    QObject::connect(_heartbeat_timer, &QTimer::timeout,
                     this, &TcpMgr::slot_send_heartbeat);
}

void TcpMgr::slot_tcp_connect(ServerInfo si)
{
    qDebug()<< "receive tcp connect signal";
    qDebug() << "Connecting to server...";
    _host = si.Host.trimmed();
    _port = static_cast<uint16_t>(si.Port.toUInt());

    if (_socket.state() != QAbstractSocket::UnconnectedState) {
        _socket.abort();
    }

    resetRecvState();
    qDebug() << "[TcpMgr] connectToHost host="
             << _host
             << ", host.length=" << _host.length()
             << ", rawHost=" << si.Host
             << ", rawHost.length=" << si.Host.length()
             << ", port=" << _port
             << ", rawPort=" << si.Port;
    _socket.connectToHost(_host, _port);
}

void TcpMgr::slot_send_data(ReqId reqId, QString data)
{
    uint16_t id = reqId;

    QByteArray dataBytes = data.toUtf8();

    quint16 len = static_cast<quint16>(dataBytes.size());

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);

    out.setByteOrder(QDataStream::BigEndian);

    out << id << len;

    block.append(dataBytes);

    _socket.write(block);
}

void TcpMgr::slot_start_heartbeat()
{
    qDebug() << "heartbeat timer started";
    _heartbeat_timer->start(5000);
}

void TcpMgr::slot_stop_heartbeat()
{
    qDebug() << "heartbeat timer stopped";
    _heartbeat_timer->stop();
}

void TcpMgr::slot_send_heartbeat()
{
    slot_send_data(ReqId::ID_HEART_BEAT_REQ, "{}");
}

void TcpMgr::resetRecvState()
{
    _buffer.clear();
    _b_recv_pending = false;
    _message_id = 0;
    _message_len = 0;
}



