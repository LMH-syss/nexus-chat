#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QWidget>
#include <QPoint>
#include "global.h"
namespace Ui {
class LoginDialog;
}

class LoginDialog : public QWidget
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void initHead();
    Ui::LoginDialog *ui;
    bool checkUserValid();
    bool checkPwdValid();
    void AddTipErr(TipErr te, QString tips);
    void DelTipErr(TipErr te);
    QMap<TipErr,QString> _tip_errs;
    void showTips(QString str,bool b_ok);
    void initHttpHandlers();
    void setLoginPending(bool pending);
    QMap<ReqId, std::function<void(const QJsonObject&)>> _handlers;
    int _uid;
    QString _token;
    bool _login_pending = false;
    bool _windowDragging = false;
    QPoint _dragStartGlobalPos;
    QPoint _dragStartFramePos;

private slots:
    void on_login_close_btn_clicked();
    void slot_forget_pwd();
    void on_pushButton_clicked();
    void slot_login_mod_finish(ReqId id, QString res, ErrorCodes err);
    void slot_tcp_con_finish(bool bsuccess);
    void slot_chat_login_success();
    void slot_chat_login_failed(int error);
signals:
    void switchRegister();
    void switchReset();
    void switchChat();
    void sig_connect_tcp(ServerInfo);
};

#endif // LOGINDIALOG_H
