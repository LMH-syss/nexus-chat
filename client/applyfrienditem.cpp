#include "applyfrienditem.h"
#include "ui_applyfrienditem.h"

#include "clickedbtn.h"

#include <QPixmap>

ApplyFriendItem::ApplyFriendItem(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ApplyFriendItem)
{
    ui->setupUi(this);
    ui->addBtn->SetState("normal", "hover", "press");
    connect(ui->addBtn, &ClickedBtn::clicked, this, [this]() {
        emit sig_accept_friend(_uid);
    });
}

ApplyFriendItem::~ApplyFriendItem()
{
    delete ui;
}

void ApplyFriendItem::SetInfo(int uid, const QString& name, const QString& desc,
                              const QString& avatarPath, bool handled)
{
    _uid = uid;
    ui->icon_lb->setPixmap(QPixmap(avatarPath));
    ui->user_name_lb->setText(name);
    ui->user_chat_lb->setText(desc);
    ShowAddBtn(!handled);
}

void ApplyFriendItem::ShowAddBtn(bool show)
{
    ui->addBtn->setVisible(show);
    ui->already_add_lb->setVisible(!show);
}

int ApplyFriendItem::uid() const
{
    return _uid;
}
