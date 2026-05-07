#include "logindialog.h"
#include "ui_logindialog.h"
#include "httpmgr.h"
#include "tcpmgr.h"
#include <QEvent>
#include <QJsonDocument>
#include <QMouseEvent>

LoginDialog::LoginDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    setAutoFillBackground(true);
    setAttribute(Qt::WA_StyledBackground, true);
    if (ui->widget) {
        ui->widget->setAutoFillBackground(true);
        ui->widget->setAttribute(Qt::WA_StyledBackground, true);
    }

    connect(ui->login_close_btn, &QPushButton::clicked, this, &LoginDialog::on_login_close_btn_clicked);
    ui->window_ctrl_wid->raise();
    qApp->installEventFilter(this);

    connect(ui->reg_btn, &QPushButton::clicked, this, &LoginDialog::switchRegister);

    ui->forget_label->SetState("normal", "hover", "", "selected", "selected_hover", "");
    ui->forget_label->setCursor(Qt::PointingHandCursor);
    connect(ui->forget_label, &ClickedLabel::clicked, this, &LoginDialog::slot_forget_pwd);

    initHttpHandlers();

    connect(HttpMgr::GetInstance().get(), &HttpMgr::sig_login_mod_finish, this,
            &LoginDialog::slot_login_mod_finish);

    connect(this, &LoginDialog::sig_connect_tcp,
            TcpMgr::GetInstance().get(), &TcpMgr::slot_tcp_connect);

    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_con_success,
            this, &LoginDialog::slot_tcp_con_finish);

    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_login_success,
            this, &LoginDialog::slot_chat_login_success);

    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_login_failed,
            this, &LoginDialog::slot_chat_login_failed);
}

LoginDialog::~LoginDialog()
{
    qApp->removeEventFilter(this);
    delete ui;
}

void LoginDialog::initHttpHandlers()
{
    _handlers.insert(ReqId::ID_LOGIN_USER, [this](QJsonObject jsonObj) {
        int error = jsonObj["error"].toInt();

        if (error != ErrorCodes::SUCCESS) {
            showTips(tr("参数错误"), false);
            setLoginPending(false);
            return;
        }

        auto email = jsonObj["email"].toString();

        ServerInfo si;
        si.Uid = jsonObj["uid"].toInt();
        si.Host = jsonObj["host"].toString();
        si.Port = jsonObj["port"].toString();
        si.Token = jsonObj["token"].toString();

        qDebug() << "[LoginDialog] login response host="
                 << si.Host
                 << ", host.length=" << si.Host.length()
                 << ", port=" << si.Port
                 << ", raw json=" << QJsonDocument(jsonObj).toJson(QJsonDocument::Compact);

        _uid = si.Uid;
        _token = si.Token;

        qDebug() << "email is " << email
                 << " uid is " << si.Uid
                 << " host is " << si.Host
                 << " Port is " << si.Port
                 << " Token is " << si.Token;

        showTips(tr("登录成功"), true);

        emit sig_connect_tcp(si);
    });
}

void LoginDialog::slot_login_mod_finish(ReqId id, QString res, ErrorCodes err)
{
    if (err != ErrorCodes::SUCCESS) {
        setLoginPending(false);
        showTips(tr("网络请求错误"), false);
        return;
    }

    QJsonDocument jsonDoc = QJsonDocument::fromJson(res.toUtf8());

    if (jsonDoc.isNull()) {
        setLoginPending(false);
        showTips(tr("JSON解析错误"), false);
        return;
    }

    if (!jsonDoc.isObject()) {
        setLoginPending(false);
        showTips(tr("JSON解析错误"), false);
        return;
    }

    _handlers[id](jsonDoc.object());

    return;
}

bool LoginDialog::checkUserValid()
{
    auto user = ui->user_edit->text();

    if (user.isEmpty()) {
        AddTipErr(TipErr::TIP_PWD_ERR, tr("用户名不能为空"));
        return false;
    }

    return true;
}

bool LoginDialog::checkPwdValid()
{
    auto pwd = ui->pass_edit->text();

    if (pwd.length() < 6 || pwd.length() > 15) {
        AddTipErr(TipErr::TIP_PWD_ERR, tr("密码长度应为6~15"));
        return false;
    }

    QRegularExpression regExp("^[a-zA-Z0-9!@#$%^&*.]{6,15}$");
    bool match = regExp.match(pwd).hasMatch();

    if (!match) {
        // 提示字符非法
        AddTipErr(TipErr::TIP_PWD_ERR, tr("不能包含非法字符，且长度应为6~15"));
        return false;
    }

    DelTipErr(TipErr::TIP_PWD_ERR);

    return true;
}

void LoginDialog::AddTipErr(TipErr te, QString tips)
{
    _tip_errs[te] = tips;
    showTips(tips, false);
}

void LoginDialog::DelTipErr(TipErr te)
{
    _tip_errs.remove(te);

    if (_tip_errs.empty()) {
        ui->err_tip->clear();
        return;
    }

    showTips(_tip_errs.first(), false);
}

void LoginDialog::showTips(QString str, bool b_ok)
{
    if (b_ok) {
        ui->err_tip->setProperty("state", "normal");
    } else {
        ui->err_tip->setProperty("state", "err");
    }

    ui->err_tip->setText(str);

    repolish(ui->err_tip);
}

void LoginDialog::slot_forget_pwd()
{
    qDebug() << "slot forget pwd";
    emit switchReset();
}

void LoginDialog::setLoginPending(bool pending)
{
    _login_pending = pending;
    ui->pushButton->setEnabled(!pending);
}

void LoginDialog::on_pushButton_clicked()
{
    qDebug() << "login btn clicked";

    if (_login_pending) {
        return;
    }

    if (checkUserValid() == false) {
        return;
    }

    if (checkPwdValid() == false) {
        return;
    }

    auto email = ui->user_edit->text();
    auto pwd = ui->pass_edit->text();

    QJsonObject json_obj;
    json_obj["email"] = email;
    json_obj["passwd"] = pwd;

    setLoginPending(true);
    HttpMgr::GetInstance()->PostHttpReq(QUrl(gate_url_prefix + "/user_login"),
                                        json_obj,
                                        ReqId::ID_LOGIN_USER,
                                        Modules::LOGINMOD);
}

void LoginDialog::slot_tcp_con_finish(bool bsuccess)
{
    // TCP连接完成后的槽函数
    if (bsuccess) {
        showTips(tr("已连接聊天服务器，正在登录..."), true);

        QJsonObject jsonObj;
        jsonObj["uid"] = _uid;
        jsonObj["token"] = _token;

        // 把 JSON 对象包装成文档，方便转成字符串
        QJsonDocument doc(jsonObj);

        // 发送 TCP 登录包
        TcpMgr::GetInstance()->slot_send_data(
            ReqId::ID_CHAT_LOGIN,
            QString::fromUtf8(doc.toJson(QJsonDocument::Compact))
            );
    } else {
        setLoginPending(false);
        showTips(tr("连接聊天服务器失败"), false);
    }
}

void LoginDialog::slot_chat_login_success()
{
    setLoginPending(false);
    showTips(tr("聊天登录成功"), true);
    emit switchChat();
}

void LoginDialog::slot_chat_login_failed(int error)
{
    setLoginPending(false);
    showTips(tr("聊天登录失败，错误码：%1").arg(error), false);
}

void LoginDialog::on_login_close_btn_clicked()
{
    if (window()) {
        window()->close();
    }
}

bool LoginDialog::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        auto mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton
            && window()
            && !window()->isMaximized()) {
            _windowDragging = true;
            _dragStartGlobalPos = mouseEvent->globalPosition().toPoint();
            _dragStartFramePos = window()->frameGeometry().topLeft();
        }
    } else if (event->type() == QEvent::MouseMove && _windowDragging) {
        auto mouseEvent = static_cast<QMouseEvent*>(event);
        if ((mouseEvent->buttons() & Qt::LeftButton) && window() && !window()->isMaximized()) {
            window()->move(_dragStartFramePos
                           + mouseEvent->globalPosition().toPoint()
                           - _dragStartGlobalPos);
            return true;
        }
        _windowDragging = false;
    } else if (event->type() == QEvent::MouseButtonRelease) {
        _windowDragging = false;
    }

    return QWidget::eventFilter(watched, event);
}
