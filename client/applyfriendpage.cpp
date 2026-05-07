#include "applyfriendpage.h"
#include "ui_applyfriendpage.h"

#include "applyfrienditem.h"
#include "global.h"

#include <QJsonArray>
#include <QListWidgetItem>

ApplyFriendPage::ApplyFriendPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ApplyFriendPage)
{
    ui->setupUi(this);
    ui->apply_friend_list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->apply_friend_list->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    connect(ui->apply_friend_list, &QListWidget::itemPressed,
            this, [this]() { emit sig_show_search(false); });
    loadApplyList();
}

ApplyFriendPage::~ApplyFriendPage()
{
    delete ui;
}

void ApplyFriendPage::AddApply(int uid, const QString& name, const QString& desc,
                               const QString& avatarPath, bool handled)
{
    auto applyItem = new ApplyFriendItem(ui->apply_friend_list);
    applyItem->SetInfo(uid, name, desc, avatarPath, handled);

    connect(applyItem, &ApplyFriendItem::sig_accept_friend, this, &ApplyFriendPage::onAcceptClicked);

    auto item = new QListWidgetItem(ui->apply_friend_list);
    item->setSizeHint(QSize(640, 80));
    item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
    ui->apply_friend_list->addItem(item);
    ui->apply_friend_list->setItemWidget(item, applyItem);
}

void ApplyFriendPage::slot_incoming_apply(QJsonObject data)
{
    int from_uid = data["from_uid"].toInt();
    QString from_name = data["from_name"].toString();
    QString msg = data["msg"].toString();
    AddApply(from_uid, from_name, msg, ":/res/head_1.jpg", false);
}

void ApplyFriendPage::loadApplyList()
{
}

void ApplyFriendPage::slot_apply_list_result(QJsonObject data)
{
    ui->apply_friend_list->clear();
    auto applies = data["applies"].toArray();
    for (const auto& val : applies) {
        auto obj = val.toObject();
        int from_uid = obj["from_uid"].toInt();
        QString from_name = obj["from_name"].toString();
        QString desc = tr("Request to add you as friend");
        AddApply(from_uid, from_name, desc, ":/res/head_1.jpg", false);
    }
}

void ApplyFriendPage::onAcceptClicked(int from_uid)
{
    emit sig_auth_friend(from_uid);
}

void ApplyFriendPage::slot_auth_friend_result(QJsonObject data)
{
    auto error = data["error"].toInt();
    auto from_uid = data["from_uid"].toInt();

    for (int i = 0; i < ui->apply_friend_list->count(); ++i) {
        auto item = ui->apply_friend_list->item(i);
        auto widget = ui->apply_friend_list->itemWidget(item);
        auto applyItem = qobject_cast<ApplyFriendItem*>(widget);
        if (applyItem && applyItem->uid() == from_uid) {
            if (error == ErrorCodes::SUCCESS) {
                applyItem->ShowAddBtn(false);
                emit sig_friend_accepted(from_uid);
            }
            break;
        }
    }
}
