#include "addfriendsearchdlg.h"
#include "ui_addfriendsearchdlg.h"
#include "global.h"

#include <QColor>
#include <QJsonArray>
#include <QMouseEvent>
#include <QPalette>
#include <QPixmap>

AddFriendSearchDlg::AddFriendSearchDlg(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AddFriendSearchDlg)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_StyledBackground, true);
    setModal(true);

    auto palette = ui->search_edit->palette();
    palette.setColor(QPalette::PlaceholderText, QColor("#8c8c8c"));
    ui->search_edit->setPalette(palette);

    ui->result_tip_lb->setText(tr("请输入用户名或邮箱搜索联系人"));
    ui->result_card_wid->hide();
}

AddFriendSearchDlg::~AddFriendSearchDlg()
{
    delete ui;
}

void AddFriendSearchDlg::on_close_btn_clicked()
{
    close();
}

void AddFriendSearchDlg::on_search_btn_clicked()
{
    const auto keyword = ui->search_edit->text().trimmed();
    if (keyword.isEmpty()) {
        ui->result_tip_lb->setText(tr("请输入用户名或邮箱"));
        ui->result_tip_lb->show();
        ui->result_card_wid->hide();
        return;
    }

    ui->result_card_wid->hide();
    ui->result_tip_lb->setText(tr("正在搜索..."));
    ui->result_tip_lb->show();
    emit sig_search_user(keyword);
}

void AddFriendSearchDlg::on_add_friend_btn_clicked()
{
    if (_search_result_uid <= 0) {
        return;
    }
    ui->add_friend_btn->setEnabled(false);
    ui->add_friend_btn->setText(tr("正在发送..."));
    emit sig_add_friend(_search_result_uid);
}

void AddFriendSearchDlg::slot_add_friend_result(QJsonObject data)
{
    const auto error = data["error"].toInt();
    if (error == ErrorCodes::SUCCESS) {
        ui->result_tip_lb->setText(data["msg"].toString());
    } else {
        ui->result_tip_lb->setText(data["msg"].toString());
        ui->add_friend_btn->setEnabled(true);
        ui->add_friend_btn->setText(tr("添加到通讯录"));
    }
    ui->result_tip_lb->show();
}

void AddFriendSearchDlg::slot_search_user_result(QJsonObject data)
{
    const auto error = data["error"].toInt();
    if (error != ErrorCodes::SUCCESS) {
        ui->result_tip_lb->setText(tr("搜索失败"));
        ui->result_tip_lb->show();
        ui->result_card_wid->hide();
        return;
    }

    const auto results = data["results"].toArray();
    if (results.isEmpty()) {
        ui->result_tip_lb->setText(tr("未找到匹配用户"));
        ui->result_tip_lb->show();
        ui->result_card_wid->hide();
        return;
    }

    const auto user = results[0].toObject();
    _search_result_uid = user["uid"].toInt();
    const auto name = user["nick"].toString();
    if (name.isEmpty()) {
        ui->name_lb->setText(user["name"].toString());
    } else {
        ui->name_lb->setText(name);
    }
    ui->avatar_lb->setPixmap(QPixmap(":/res/head_1.jpg"));
    ui->gender_lb->setPixmap(QPixmap(":/res/male.png"));
    ui->add_friend_btn->setEnabled(true);
    ui->add_friend_btn->setText(tr("添加到通讯录"));
    ui->result_tip_lb->hide();
    ui->result_card_wid->show();
}

void AddFriendSearchDlg::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        const auto titlePos = ui->title_wid->mapFrom(this, event->position().toPoint());
        if (ui->title_wid->rect().contains(titlePos)) {
            _dragging = true;
            _dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
            event->accept();
            return;
        }
    }

    QDialog::mousePressEvent(event);
}

void AddFriendSearchDlg::mouseMoveEvent(QMouseEvent *event)
{
    if (_dragging && (event->buttons() & Qt::LeftButton)) {
        move(event->globalPosition().toPoint() - _dragPosition);
        event->accept();
        return;
    }

    QDialog::mouseMoveEvent(event);
}

void AddFriendSearchDlg::mouseReleaseEvent(QMouseEvent *event)
{
    _dragging = false;
    QDialog::mouseReleaseEvent(event);
}
