#include "chatuserwid.h"
#include "ui_chatuserwid.h"

#include <QPixmap>

ChatUserWid::ChatUserWid(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ChatUserWid)
{
    ui->setupUi(this);
    ui->red_point->hide();
}

ChatUserWid::~ChatUserWid()
{
    delete ui;
}

void ChatUserWid::SetInfo(int uid, const QString& name, const QString& lastMsg,
                          const QString& time, const QString& avatarPath)
{
    _uid = uid;
    _avatarPath = avatarPath;
    ui->user_name_lb->setText(name);
    ui->user_chat_lb->setText(lastMsg);
    ui->time_lb->setText(time);
    ui->icon_lb->setPixmap(QPixmap(avatarPath));
}

void ChatUserWid::UpdateLastMsg(const QString& lastMsg, const QString& time)
{
    ui->user_chat_lb->setText(lastMsg);
    ui->time_lb->setText(time);
}

void ChatUserWid::ShowRedPoint(bool show)
{
    ui->red_point->setVisible(show);
}

int ChatUserWid::uid() const
{
    return _uid;
}

QString ChatUserWid::userName() const
{
    return ui->user_name_lb->text();
}

QString ChatUserWid::avatarPath() const
{
    return _avatarPath;
}
