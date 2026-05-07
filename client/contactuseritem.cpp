#include "contactuseritem.h"
#include "ui_contactuseritem.h"

#include <QPixmap>

ContactUserItem::ContactUserItem(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ContactUserItem)
{
    ui->setupUi(this);
    ui->red_point->hide();
}

ContactUserItem::~ContactUserItem()
{
    delete ui;
}

void ContactUserItem::SetInfo(int uid, const QString& name, const QString& avatarPath)
{
    _uid = uid;
    _avatarPath = avatarPath;
    ui->user_name_lb->setText(name);
    ui->icon_lb->setPixmap(QPixmap(avatarPath));
}

void ContactUserItem::ShowRedPoint(bool show)
{
    ui->red_point->setVisible(show);
}

int ContactUserItem::uid() const
{
    return _uid;
}

QString ContactUserItem::userName() const
{
    return ui->user_name_lb->text();
}

QString ContactUserItem::avatarPath() const
{
    return _avatarPath;
}
