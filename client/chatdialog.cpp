#include "chatdialog.h"
#include "ui_chatdialog.h"

#include "addfriendsearchdlg.h"
#include "applyfriendpage.h"
#include <QJsonArray>
#include "chatpage.h"
#include "chatuserwid.h"
#include "clickedbtn.h"
#include "contactuseritem.h"
#include "global.h"
#include "statewidget.h"
#include "tcpmgr.h"
#include "usermgr.h"

#include <algorithm>
#include <QAction>
#include <QAbstractItemView>
#include <QApplication>
#include <QDate>
#include <QDateTime>
#include <QDebug>
#include <QIcon>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QSet>
#include <QMouseEvent>
#include <QPixmap>
#include <QPushButton>
#include <QResizeEvent>
#include <QSignalBlocker>
#include <QVBoxLayout>

ChatDialog::ChatDialog(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ChatDialog)
{
    ui->setupUi(this);
    setAutoFillBackground(true);
    setAttribute(Qt::WA_StyledBackground, true);
    ui->stackedWidget->setAutoFillBackground(true);
    ui->stackedWidget->setAttribute(Qt::WA_StyledBackground, true);
    ui->chat_page->setAutoFillBackground(true);
    ui->chat_page->setAttribute(Qt::WA_StyledBackground, true);
    ui->friend_apply_page->setAutoFillBackground(true);
    ui->friend_apply_page->setAttribute(Qt::WA_StyledBackground, true);
    ui->friend_info_page->setAutoFillBackground(true);
    ui->friend_info_page->setAttribute(Qt::WA_StyledBackground, true);
    initUiState();
    initChatUsers();
    initContactUsers();
}

ChatDialog::~ChatDialog()
{
    qApp->removeEventFilter(this);
    delete ui;
}

void ChatDialog::SetMaximizedState(bool maximized)
{
    if (!ui || !ui->win_max_btn) {
        return;
    }

    ui->win_max_btn->setText(maximized ? "❐" : "□");
    ui->win_max_btn->setToolTip(maximized ? tr("还原") : tr("最大化"));
}

void ChatDialog::initUiState()
{
    const auto userInfo = UserMgr::GetInstance()->GetUserInfo();
    if (!userInfo.name.isEmpty()) {
        setWindowTitle(userInfo.name);
    }

    ui->side_head_lb->setPixmap(QPixmap(":/res/head.png"));
    ui->side_chat_lb->SetState("normal", "hover", "pressed",
                               "selected_normal", "selected_hover", "selected_pressed");
    ui->side_contact_lb->SetState("normal", "hover", "pressed",
                                  "selected_normal", "selected_hover", "selected_pressed");
    ui->side_chat_lb->SetSelected(true);
    ui->add_btn->SetState("normal", "hover", "press");

    auto searchAction = new QAction(ui->search_edit);
    searchAction->setIcon(QIcon(":/res/search.png"));
    ui->search_edit->addAction(searchAction, QLineEdit::LeadingPosition);

    auto clearAction = new QAction(ui->search_edit);
    clearAction->setIcon(QIcon(":/res/close_transparent.png"));
    ui->search_edit->addAction(clearAction, QLineEdit::TrailingPosition);

    connect(ui->search_edit, &QLineEdit::textChanged, this, [clearAction](const QString& text) {
        clearAction->setIcon(QIcon(text.isEmpty() ? ":/res/close_transparent.png"
                                                  : ":/res/close_search.png"));
    });
    connect(clearAction, &QAction::triggered, this, [this, clearAction]() {
        ui->search_edit->clear();
        clearAction->setIcon(QIcon(":/res/close_transparent.png"));
        showSearch(false);
    });

    ui->chat_user_list->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui->chat_user_list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->search_list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->con_user_list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->search_list->hide();
    ui->con_user_list->hide();
    ui->stackedWidget->setCurrentWidget(ui->chat_page);
    _lastContactPage = ui->friend_apply_page;
    updateWindowControlGeometry();
    ui->window_ctrl_wid->raise();
    ui->win_min_btn->setToolTip(tr("最小化"));
    SetMaximizedState(false);
    ui->win_close_btn->setToolTip(tr("关闭"));

    connect(ui->chat_user_list, &QListWidget::currentItemChanged,
            this, &ChatDialog::slot_chat_user_changed);
    connect(ui->con_user_list, &QListWidget::itemClicked,
            this, &ChatDialog::slot_contact_item_clicked);
    connect(ui->search_list, &QListWidget::itemClicked,
            this, &ChatDialog::slot_search_item_clicked);
    connect(ui->side_chat_lb, &StateWidget::clicked,
            this, &ChatDialog::slot_side_chat_clicked);
    connect(ui->side_contact_lb, &StateWidget::clicked,
            this, &ChatDialog::slot_side_contact_clicked);
    connect(ui->search_edit, &QLineEdit::textChanged,
            this, &ChatDialog::slot_search_text_changed);
    connect(ui->add_btn, &ClickedBtn::clicked, this, [this]() {
        AddFriendSearchDlg dlg(this);
        dlg.setModal(true);

        connect(&dlg, &AddFriendSearchDlg::sig_search_user, [](const QString& keyword) {
            QJsonObject json;
            json["keyword"] = keyword;
            QJsonDocument doc(json);
            TcpMgr::GetInstance()->slot_send_data(
                ReqId::ID_SEARCH_USER_REQ,
                QString::fromUtf8(doc.toJson(QJsonDocument::Compact))
            );
        });

        connect(&dlg, &AddFriendSearchDlg::sig_add_friend, [](int to_uid) {
            QJsonObject json;
            json["to_uid"] = to_uid;
            QJsonDocument doc(json);
            TcpMgr::GetInstance()->slot_send_data(
                ReqId::ID_ADD_FRIEND_REQ,
                QString::fromUtf8(doc.toJson(QJsonDocument::Compact))
            );
        });

        connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_search_user_result,
                &dlg, &AddFriendSearchDlg::slot_search_user_result);

        connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_add_friend_result,
                &dlg, &AddFriendSearchDlg::slot_add_friend_result);

        dlg.exec();
    });
    connect(ui->friend_apply_page, &ApplyFriendPage::sig_show_search,
            this, &ChatDialog::slot_show_search);
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_notify_add_friend,
            ui->friend_apply_page, &ApplyFriendPage::slot_incoming_apply);
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_apply_list_result,
            ui->friend_apply_page, &ApplyFriendPage::slot_apply_list_result);
    connect(ui->friend_apply_page, &ApplyFriendPage::sig_auth_friend, [](int from_uid) {
        QJsonObject json;
        json["from_uid"] = from_uid;
        json["agree"] = true;
        QJsonDocument doc(json);
        TcpMgr::GetInstance()->slot_send_data(
            ReqId::ID_AUTH_FRIEND_REQ,
            QString::fromUtf8(doc.toJson(QJsonDocument::Compact))
        );
    });
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_auth_friend_result,
            ui->friend_apply_page, &ApplyFriendPage::slot_auth_friend_result);
    connect(ui->friend_apply_page, &ApplyFriendPage::sig_friend_accepted, [](int) {
        TcpMgr::GetInstance()->slot_send_data(ReqId::ID_GET_FRIEND_LIST_REQ, QString("{}"));
    });
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_notify_auth_friend,
            this, &ChatDialog::slot_notify_auth_friend);
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_friend_list_result,
            this, &ChatDialog::slot_friend_list_result);
    connect(ui->chat_page, &ChatPage::sig_send_text_msg,
            this, &ChatDialog::sig_send_text_msg);
    connect(this, &ChatDialog::sig_send_text_msg, [this](int toUid, QString content) {
        _pendingTextMessages[toUid].append(content);

        QJsonObject json;
        json["to_uid"] = toUid;
        json["message"] = content;
        QJsonDocument doc(json);
        TcpMgr::GetInstance()->slot_send_data(
            ReqId::ID_TEXT_CHAT_MSG_REQ,
            QString::fromUtf8(doc.toJson(QJsonDocument::Compact))
        );
    });
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_notify_text_chat_msg,
            this, &ChatDialog::slot_notify_text_chat_msg);
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_text_chat_msg_result,
            this, &ChatDialog::slot_text_chat_msg_result);
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_chat_history_result,
            this, &ChatDialog::slot_chat_history_result);
    connect(ui->win_min_btn, &QPushButton::clicked,
            this, &ChatDialog::sig_minimize_window);
    connect(ui->win_max_btn, &QPushButton::clicked,
            this, &ChatDialog::sig_max_restore_window);
    connect(ui->win_close_btn, &QPushButton::clicked,
            this, &ChatDialog::sig_close_window);

    qApp->installEventFilter(this);

    TcpMgr::GetInstance()->slot_send_data(ReqId::ID_GET_APPLY_LIST_REQ, QString("{}"));
    TcpMgr::GetInstance()->slot_send_data(ReqId::ID_GET_FRIEND_LIST_REQ, QString("{}"));
}

void ChatDialog::initChatUsers()
{
    ui->chat_user_list->clear();
    _chatItems.clear();

    addSearchTipItem(tr("搜索"), tr("输入昵称或 UID 后，会在这里显示会话和联系人"));

}

void ChatDialog::initContactUsers()
{
    ui->con_user_list->clear();

    auto applyItem = new QListWidgetItem;
    applyItem->setData(RoleKind, KindApplyFriend);
    applyItem->setSizeHint(QSize(250, 70));
    auto applyWid = new ContactUserItem(ui->con_user_list);
    applyWid->SetInfo(0, tr("新的朋友"), ":/res/add_friend.png");
    applyWid->ShowRedPoint(false);
    ui->con_user_list->addItem(applyItem);
    ui->con_user_list->setItemWidget(applyItem, applyWid);

    addGroupTipItem(ui->con_user_list, tr("联系人"));
}

void ChatDialog::addChatUserItem(int uid, const QString& name, const QString& lastMsg,
                                 const QString& time, const QString& avatarPath,
                                 int unreadCount, qint64 lastTimestamp)
{
    if (_chatItems.contains(uid)) {
        updateChatItemStatus(uid, lastMsg, time, unreadCount, lastTimestamp);
        return;
    }

    auto item = new QListWidgetItem;
    item->setData(RoleUid, uid);
    item->setData(RoleAvatar, avatarPath);
    item->setData(RoleName, name);
    item->setData(RoleKind, KindChat);
    item->setData(RoleLastMsg, lastMsg);
    item->setData(RoleLastTime, time);
    item->setData(RoleUnread, unreadCount);
    item->setData(RoleLastTimestamp, lastTimestamp);
    item->setSizeHint(QSize(250, 70));

    auto userWid = new ChatUserWid(ui->chat_user_list);
    userWid->SetInfo(uid, name, lastMsg, time, avatarPath);
    userWid->ShowRedPoint(unreadCount > 0);

    ui->chat_user_list->addItem(item);
    ui->chat_user_list->setItemWidget(item, userWid);
    _chatItems.insert(uid, item);
    sortChatItemsKeepSelection();
}

void ChatDialog::addContactUserItem(int uid, const QString& name, const QString& avatarPath)
{
    if (_contactItems.contains(uid)) {
        return;
    }

    auto item = new QListWidgetItem;
    item->setData(RoleUid, uid);
    item->setData(RoleAvatar, avatarPath);
    item->setData(RoleName, name);
    item->setData(RoleKind, KindContact);
    item->setSizeHint(QSize(250, 70));

    auto userWid = new ContactUserItem(ui->con_user_list);
    userWid->SetInfo(uid, name, avatarPath);

    ui->con_user_list->addItem(item);
    ui->con_user_list->setItemWidget(item, userWid);
    _contactItems.insert(uid, item);
}

void ChatDialog::addGroupTipItem(QListWidget *list, const QString& text)
{
    auto item = new QListWidgetItem;
    item->setData(RoleKind, KindGroupTip);
    item->setSizeHint(QSize(250, 28));
    item->setFlags(item->flags() & ~Qt::ItemIsSelectable);

    auto label = new QLabel(text, list);
    label->setObjectName("contact_group_tip_lb");
    label->setContentsMargins(10, 6, 0, 0);

    list->addItem(item);
    list->setItemWidget(item, label);
}

void ChatDialog::addSearchResultItem(int uid, const QString& name, const QString& desc,
                                     const QString& avatarPath, ItemKind kind)
{
    auto item = new QListWidgetItem;
    item->setData(RoleUid, uid);
    item->setData(RoleAvatar, avatarPath);
    item->setData(RoleName, name);
    item->setData(RoleKind, kind);
    item->setSizeHint(QSize(250, 64));

    auto row = new QWidget(ui->search_list);
    row->setObjectName("search_result_item");
    auto layout = new QHBoxLayout(row);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(8);

    auto icon = new QLabel(row);
    icon->setFixedSize(40, 40);
    icon->setScaledContents(true);
    icon->setPixmap(QPixmap(avatarPath));

    auto textLayout = new QVBoxLayout;
    textLayout->setContentsMargins(0, 0, 0, 0);
    textLayout->setSpacing(4);
    auto titleLabel = new QLabel(name, row);
    titleLabel->setObjectName("search_title_lb");
    auto descLabel = new QLabel(desc, row);
    descLabel->setObjectName("search_desc_lb");
    textLayout->addWidget(titleLabel);
    textLayout->addWidget(descLabel);

    layout->addWidget(icon);
    layout->addLayout(textLayout, 1);

    ui->search_list->addItem(item);
    ui->search_list->setItemWidget(item, row);
}

void ChatDialog::addSearchTipItem(const QString& title, const QString& desc)
{
    auto item = new QListWidgetItem;
    item->setSizeHint(QSize(250, 54));
    item->setData(RoleKind, KindGroupTip);
    item->setFlags(item->flags() & ~Qt::ItemIsSelectable);

    auto row = new QWidget(ui->search_list);
    auto layout = new QVBoxLayout(row);
    layout->setContentsMargins(8, 6, 8, 6);
    layout->setSpacing(4);

    auto titleLabel = new QLabel(title, row);
    titleLabel->setObjectName("search_title_lb");
    auto descLabel = new QLabel(desc, row);
    descLabel->setObjectName("search_desc_lb");
    layout->addWidget(titleLabel);
    layout->addWidget(descLabel);

    ui->search_list->addItem(item);
    ui->search_list->setItemWidget(item, row);
}

void ChatDialog::slot_chat_user_changed(QListWidgetItem *current, QListWidgetItem *previous)
{
    Q_UNUSED(previous);
    if (!current) {
        ui->chat_page->SetUserInfo(0, tr("Select a chat"), QString());
        return;
    }

    const auto uid = current->data(RoleUid).toInt();
    const auto name = current->data(RoleName).toString();
    const auto avatar = current->data(RoleAvatar).toString();
    ui->chat_page->SetUserInfo(uid, name, avatar);
    setChatItemUnread(uid, 0);
    requestChatHistory(uid);
}

void ChatDialog::slot_contact_item_clicked(QListWidgetItem *item)
{
    if (!item) {
        return;
    }

    const auto kind = item->data(RoleKind).toInt();
    if (kind == KindApplyFriend) {
        _lastContactPage = ui->friend_apply_page;
        ui->stackedWidget->setCurrentWidget(ui->friend_apply_page);
        return;
    }

    if (kind == KindContact) {
        const auto uid = item->data(RoleUid).toInt();
        const auto name = item->data(RoleName).toString();
        const auto avatar = item->data(RoleAvatar).toString();
        updateFriendInfo(uid, name, avatar);
        _lastContactPage = ui->friend_info_page;
        ui->stackedWidget->setCurrentWidget(ui->friend_info_page);
    }
}

void ChatDialog::slot_search_item_clicked(QListWidgetItem *item)
{
    if (!item || item->data(RoleKind).toInt() == KindGroupTip) {
        return;
    }

    const auto uid = item->data(RoleUid).toInt();
    const auto name = item->data(RoleName).toString();
    const auto avatar = item->data(RoleAvatar).toString();
    jumpToChatItem(uid, name, avatar);
}

void ChatDialog::slot_side_chat_clicked()
{
    switchToChatMode();
}

void ChatDialog::slot_side_contact_clicked()
{
    switchToContactMode();
}

void ChatDialog::slot_search_text_changed(const QString& text)
{
    if (text.trimmed().isEmpty()) {
        showSearch(false);
        return;
    }

    rebuildSearchList(text.trimmed());
    showSearch(true);
}

void ChatDialog::slot_show_search(bool show)
{
    showSearch(show);
}

void ChatDialog::rebuildSearchList(const QString& keyword)
{
    ui->search_list->clear();
    bool hasResult = false;
    QSet<int> addedUids;

    const auto matchItem = [&keyword](QListWidgetItem *item) {
        const auto name = item->data(ChatDialog::RoleName).toString();
        const auto uid = QString::number(item->data(ChatDialog::RoleUid).toInt());
        return name.contains(keyword, Qt::CaseInsensitive) || uid.contains(keyword);
    };

    for (int i = 0; i < ui->chat_user_list->count(); ++i) {
        auto item = ui->chat_user_list->item(i);
        if (!item || item->data(RoleKind).toInt() != KindChat || !matchItem(item)) {
            continue;
        }
        addedUids.insert(item->data(RoleUid).toInt());
        hasResult = true;
        addSearchResultItem(item->data(RoleUid).toInt(),
                            item->data(RoleName).toString(),
                            tr("聊天"),
                            item->data(RoleAvatar).toString(),
                            KindChat);
    }

    for (int i = 0; i < ui->con_user_list->count(); ++i) {
        auto item = ui->con_user_list->item(i);
        if (!item || item->data(RoleKind).toInt() != KindContact || !matchItem(item)) {
            continue;
        }
        if (addedUids.contains(item->data(RoleUid).toInt())) {
            continue;
        }
        addedUids.insert(item->data(RoleUid).toInt());
        hasResult = true;
        addSearchResultItem(item->data(RoleUid).toInt(),
                            item->data(RoleName).toString(),
                            tr("联系人"),
                            item->data(RoleAvatar).toString(),
                            KindContact);
    }

    if (!hasResult) {
        addSearchTipItem(tr("没有找到相关结果"), tr("当前只搜索已加载的会话和联系人"));
    }
}

void ChatDialog::showSearch(bool show)
{
    if (show) {
        ui->chat_user_list->hide();
        ui->con_user_list->hide();
        ui->search_list->show();
        _mode = UIMode::Search;
        return;
    }

    ui->search_list->hide();
    if (_state == UIMode::Contact) {
        ui->chat_user_list->hide();
        ui->con_user_list->show();
        _mode = UIMode::Contact;
    } else {
        ui->chat_user_list->show();
        ui->con_user_list->hide();
        _mode = UIMode::Chat;
    }

    ui->search_list->clear();
    addSearchTipItem(tr("搜索"), tr("输入昵称或 UID 后，会在这里显示会话和联系人"));
    ui->search_edit->blockSignals(true);
    ui->search_edit->clear();
    ui->search_edit->blockSignals(false);
    ui->search_edit->clearFocus();
}

void ChatDialog::switchToChatMode()
{
    ui->side_chat_lb->SetSelected(true);
    ui->side_contact_lb->SetSelected(false);
    _state = UIMode::Chat;
    ui->stackedWidget->setCurrentWidget(ui->chat_page);
    showSearch(false);
}

void ChatDialog::switchToContactMode()
{
    ui->side_chat_lb->SetSelected(false);
    ui->side_contact_lb->SetSelected(true);
    _state = UIMode::Contact;
    ui->stackedWidget->setCurrentWidget(_lastContactPage ? _lastContactPage : ui->friend_apply_page);
    showSearch(false);
}

void ChatDialog::jumpToChatItem(int uid, const QString& name, const QString& avatarPath)
{
    if (!_chatItems.contains(uid)) {
        addChatUserItem(uid, name, tr("Start chatting"), tr("now"), avatarPath);
    }

    auto item = _chatItems.value(uid, nullptr);
    if (!item) {
        return;
    }

    ui->chat_user_list->scrollToItem(item);
    ui->chat_user_list->setCurrentItem(item);
    switchToChatMode();
}

void ChatDialog::updateFriendInfo(int uid, const QString& name, const QString& avatarPath)
{
    ui->friend_info_avatar_lb->setPixmap(QPixmap(avatarPath));
    ui->friend_info_name_lb->setText(name);
    ui->friend_info_uid_lb->setText(tr("UID: %1").arg(uid));
}

void ChatDialog::requestChatHistory(int peerUid)
{
    if (peerUid <= 0) {
        return;
    }

    QJsonObject json;
    json["peer_uid"] = peerUid;
    json["limit"] = 30;
    json["before_msg_id"] = 0;
    json["mark_read"] = true;
    QJsonDocument doc(json);
    TcpMgr::GetInstance()->slot_send_data(
        ReqId::ID_GET_CHAT_HISTORY_REQ,
        QString::fromUtf8(doc.toJson(QJsonDocument::Compact))
    );
}

void ChatDialog::markChatRead(int peerUid)
{
    if (peerUid <= 0) {
        return;
    }

    QJsonObject json;
    json["peer_uid"] = peerUid;
    json["mark_read"] = true;
    json["only_mark_read"] = true;
    QJsonDocument doc(json);
    TcpMgr::GetInstance()->slot_send_data(
        ReqId::ID_GET_CHAT_HISTORY_REQ,
        QString::fromUtf8(doc.toJson(QJsonDocument::Compact))
    );
}

void ChatDialog::updateChatItemStatus(int uid, const QString& lastMsg,
                                      const QString& time, int unreadCount,
                                      qint64 lastTimestamp)
{
    auto item = _chatItems.value(uid, nullptr);
    if (!item) {
        return;
    }

    item->setData(RoleLastMsg, lastMsg);
    item->setData(RoleLastTime, time);
    item->setData(RoleUnread, unreadCount);
    if (lastTimestamp > 0) {
        item->setData(RoleLastTimestamp, lastTimestamp);
    }

    auto userWid = qobject_cast<ChatUserWid*>(ui->chat_user_list->itemWidget(item));
    if (!userWid) {
        return;
    }

    userWid->UpdateLastMsg(lastMsg, time);
    userWid->ShowRedPoint(unreadCount > 0);
    sortChatItemsKeepSelection();
}

void ChatDialog::setChatItemUnread(int uid, int unreadCount)
{
    auto item = _chatItems.value(uid, nullptr);
    if (!item) {
        return;
    }

    const auto lastMsg = item->data(RoleLastMsg).toString();
    const auto lastTime = item->data(RoleLastTime).toString();
    const auto lastTimestamp = item->data(RoleLastTimestamp).toLongLong();
    updateChatItemStatus(uid, lastMsg, lastTime, unreadCount, lastTimestamp);
}

void ChatDialog::sortChatItemsKeepSelection()
{
    struct ChatItemSnapshot {
        int uid = 0;
        QString avatar;
        QString name;
        QString lastMsg;
        QString lastTime;
        int unreadCount = 0;
        qint64 lastTimestamp = 0;
    };

    QList<ChatItemSnapshot> snapshots;
    snapshots.reserve(_chatItems.size());

    const int selectedUid = currentToUid();
    for (auto it = _chatItems.cbegin(); it != _chatItems.cend(); ++it) {
        auto item = it.value();
        if (!item) {
            continue;
        }

        ChatItemSnapshot snapshot;
        snapshot.uid = item->data(RoleUid).toInt();
        snapshot.avatar = item->data(RoleAvatar).toString();
        snapshot.name = item->data(RoleName).toString();
        snapshot.lastMsg = item->data(RoleLastMsg).toString();
        snapshot.lastTime = item->data(RoleLastTime).toString();
        snapshot.unreadCount = item->data(RoleUnread).toInt();
        snapshot.lastTimestamp = item->data(RoleLastTimestamp).toLongLong();
        snapshots.append(snapshot);
    }

    std::sort(snapshots.begin(), snapshots.end(),
              [](const ChatItemSnapshot& left, const ChatItemSnapshot& right) {
                  if (left.lastTimestamp == right.lastTimestamp) {
                      return left.uid < right.uid;
                  }
                  return left.lastTimestamp > right.lastTimestamp;
              });

    QSignalBlocker blocker(ui->chat_user_list);
    ui->chat_user_list->clear();
    _chatItems.clear();

    for (const auto& snapshot : snapshots) {
        auto item = new QListWidgetItem;
        item->setData(RoleUid, snapshot.uid);
        item->setData(RoleAvatar, snapshot.avatar);
        item->setData(RoleName, snapshot.name);
        item->setData(RoleKind, KindChat);
        item->setData(RoleLastMsg, snapshot.lastMsg);
        item->setData(RoleLastTime, snapshot.lastTime);
        item->setData(RoleUnread, snapshot.unreadCount);
        item->setData(RoleLastTimestamp, snapshot.lastTimestamp);
        item->setSizeHint(QSize(250, 70));

        auto userWid = new ChatUserWid(ui->chat_user_list);
        userWid->SetInfo(snapshot.uid, snapshot.name, snapshot.lastMsg,
                         snapshot.lastTime, snapshot.avatar);
        userWid->ShowRedPoint(snapshot.unreadCount > 0);

        ui->chat_user_list->addItem(item);
        ui->chat_user_list->setItemWidget(item, userWid);
        _chatItems.insert(snapshot.uid, item);

        if (snapshot.uid == selectedUid) {
            ui->chat_user_list->setCurrentItem(item);
        }
    }
}

qint64 ChatDialog::parseChatTimestamp(const QString& rawTime) const
{
    const auto trimmed = rawTime.trimmed();
    if (trimmed.isEmpty()) {
        return 0;
    }

    auto dt = QDateTime::fromString(trimmed, "yyyy-MM-dd HH:mm:ss");
    if (!dt.isValid()) {
        dt = QDateTime::fromString(trimmed, Qt::ISODate);
    }

    if (!dt.isValid()) {
        return 0;
    }

    return dt.toMSecsSinceEpoch();
}

QString ChatDialog::formatChatTime(const QString& rawTime) const
{
    if (rawTime.trimmed().isEmpty()) {
        return QString();
    }

    auto dt = QDateTime::fromString(rawTime, "yyyy-MM-dd HH:mm:ss");
    if (!dt.isValid()) {
        return rawTime;
    }

    const auto today = QDate::currentDate();
    if (dt.date() == today) {
        return dt.toString("HH:mm");
    }

    return dt.toString("MM-dd");
}

int ChatDialog::currentToUid() const
{
    auto item = ui->chat_user_list->currentItem();
    return item ? item->data(RoleUid).toInt() : 0;
}

void ChatDialog::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateWindowControlGeometry();
}

void ChatDialog::updateWindowControlGeometry()
{
    if (!ui || !ui->window_ctrl_wid) {
        return;
    }

    const int marginRight = 10;
    const int marginTop = 8;
    ui->window_ctrl_wid->move(width() - ui->window_ctrl_wid->width() - marginRight,
                              marginTop);
    ui->window_ctrl_wid->raise();
}

bool ChatDialog::isInWindowDragArea(const QPoint& globalPos) const
{
    const QPoint localPos = mapFromGlobal(globalPos);
    if (!rect().contains(localPos)) {
        return false;
    }

    if (ui->window_ctrl_wid && ui->window_ctrl_wid->geometry().contains(localPos)) {
        return false;
    }

    if (ui->side_bar && ui->side_bar->geometry().contains(localPos)) {
        return false;
    }

    if (ui->chat_user_wid && ui->chat_user_wid->geometry().contains(localPos)) {
        return false;
    }

    return localPos.y() >= 0 && localPos.y() <= 70;
}

bool ChatDialog::eventFilter(QObject *watched, QEvent *event)
{
    Q_UNUSED(watched);
    if (event->type() == QEvent::MouseButtonPress && _mode == UIMode::Search) {
        auto mouseEvent = static_cast<QMouseEvent*>(event);
        const auto searchListPos = ui->search_list->mapFromGlobal(mouseEvent->globalPosition().toPoint());
        const auto searchEditPos = ui->search_edit->mapFromGlobal(mouseEvent->globalPosition().toPoint());
        const bool inSearchList = ui->search_list->rect().contains(searchListPos);
        const bool inSearchEdit = ui->search_edit->rect().contains(searchEditPos);
        if (!inSearchList && !inSearchEdit) {
            showSearch(false);
        }
    }

    if (event->type() == QEvent::MouseButtonPress) {
        auto mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton
            && window()
            && !window()->isMaximized()
            && isInWindowDragArea(mouseEvent->globalPosition().toPoint())) {
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

void ChatDialog::slot_notify_auth_friend(QJsonObject data)
{
    int from_uid = data["from_uid"].toInt();
    QString from_name = data["from_name"].toString();
    qDebug() << "Friend request accepted by" << from_name << "uid:" << from_uid;

    addContactUserItem(from_uid, from_name, ":/res/head.png");
    addChatUserItem(from_uid, from_name, tr("Start chatting"), QString(), ":/res/head.png");
}

void ChatDialog::slot_friend_list_result(QJsonObject data)
{
    // 只移除 KindContact 类型的 item，保留"新的朋友"和"联系人"分组提示
    for (int i = ui->con_user_list->count() - 1; i >= 0; --i) {
        auto item = ui->con_user_list->item(i);
        if (item && item->data(RoleKind).toInt() == KindContact) {
            delete ui->con_user_list->takeItem(i);
        }
    }
    _contactItems.clear();

    auto friends = data["friends"].toArray();
    for (const auto& val : friends) {
        auto obj = val.toObject();
        int uid = obj["uid"].toInt();
        QString name = obj["name"].toString();
        QString lastMsg = obj["last_msg"].toString();
        const auto rawLastTime = obj["last_time"].toString();
        QString lastTime = formatChatTime(rawLastTime);
        const auto lastTimestamp = parseChatTimestamp(rawLastTime);
        int unreadCount = obj["unread_count"].toInt();
        if (lastMsg.isEmpty()) {
            lastMsg = tr("Start chatting");
        }
        addContactUserItem(uid, name, ":/res/head.png");
        addChatUserItem(uid, name, lastMsg, lastTime, ":/res/head.png",
                        unreadCount, lastTimestamp);
    }
}

#if 0
void ChatDialog::slot_notify_text_chat_msg(QJsonObject data)
{
    int from_uid = data["from_uid"].toInt();
    QString from_name = data["from_name"].toString();
    QString message = data["message"].toString();
    QString sendTime = formatChatTime(data["send_time"].toString());
    if (sendTime.isEmpty()) {
        sendTime = QDateTime::currentDateTime().toString("HH:mm");
    }

    // 如果发送者不在会话列表中，动态添加
    if (!_chatItems.contains(from_uid)) {
    if (!_chatItems.contains(from_uid)) {
        addChatUserItem(from_uid, from_name, message, sendTime, ":/res/head.png");
    }

    // 显示到聊天区域
    const bool isCurrentChat = currentToUid() == from_uid;
    const int unreadCount = isCurrentChat ? 0 : _chatItems.value(from_uid)->data(RoleUnread).toInt() + 1;
    updateChatItemStatus(from_uid, message, sendTime, unreadCount);

    ui->chat_page->slot_show_incoming_msg(from_uid, from_name, message);
    if (isCurrentChat) {
        markChatRead(from_uid);
    }
}

#endif

void ChatDialog::slot_notify_text_chat_msg(QJsonObject data)
{
    int from_uid = data["from_uid"].toInt();
    QString from_name = data["from_name"].toString();
    QString message = data["message"].toString();
    const auto rawSendTime = data["send_time"].toString();
    QString sendTime = formatChatTime(rawSendTime);
    auto sendTimestamp = parseChatTimestamp(rawSendTime);
    if (sendTime.isEmpty()) {
        const auto now = QDateTime::currentDateTime();
        sendTime = now.toString("HH:mm");
        sendTimestamp = now.toMSecsSinceEpoch();
    }

    if (!_chatItems.contains(from_uid)) {
        addChatUserItem(from_uid, from_name, message, sendTime, ":/res/head.png",
                        0, sendTimestamp);
    }

    const bool isCurrentChat = currentToUid() == from_uid;
    const int unreadCount = isCurrentChat ? 0 : _chatItems.value(from_uid)->data(RoleUnread).toInt() + 1;
    updateChatItemStatus(from_uid, message, sendTime, unreadCount, sendTimestamp);

    ui->chat_page->slot_show_incoming_msg(from_uid, from_name, message);
    if (isCurrentChat) {
        markChatRead(from_uid);
    }
}

void ChatDialog::slot_text_chat_msg_result(QJsonObject data)
{
    const auto error = data["error"].toInt();
    const auto toUid = data["to_uid"].toInt();
    auto messages = _pendingTextMessages.value(toUid);
    const auto content = messages.isEmpty() ? QString() : messages.takeFirst();

    if (messages.isEmpty()) {
        _pendingTextMessages.remove(toUid);
    } else {
        _pendingTextMessages[toUid] = messages;
    }

    if (error != ErrorCodes::SUCCESS) {
        qWarning() << "send text message failed:" << data["msg"].toString();
        return;
    }

    if (content.isEmpty()) {
        return;
    }

    const auto rawSendTime = data["send_time"].toString();
    auto sendTime = formatChatTime(rawSendTime);
    auto sendTimestamp = parseChatTimestamp(rawSendTime);
    if (sendTime.isEmpty()) {
        const auto now = QDateTime::currentDateTime();
        sendTime = now.toString("HH:mm");
        sendTimestamp = now.toMSecsSinceEpoch();
    }
    updateChatItemStatus(toUid, content, sendTime, 0, sendTimestamp);

    if (currentToUid() != toUid) {
        return;
    }

    const auto currentUser = UserMgr::GetInstance()->GetUserInfo();
    const auto selfName = currentUser.name.isEmpty() ? tr("Me") : currentUser.name;
    ui->chat_page->AppendChatMsg(selfName, content, true, ":/res/head.png");
}

void ChatDialog::slot_chat_history_result(QJsonObject data)
{
    if (data["only_mark_read"].toBool()) {
        return;
    }

    const auto error = data["error"].toInt();
    const auto peerUid = data["peer_uid"].toInt();
    if (error != ErrorCodes::SUCCESS || peerUid <= 0 || currentToUid() != peerUid) {
        return;
    }

    auto item = _chatItems.value(peerUid, nullptr);
    if (!item) {
        return;
    }

    const auto peerName = item->data(RoleName).toString();
    const auto peerAvatar = item->data(RoleAvatar).toString();
    const auto currentUser = UserMgr::GetInstance()->GetUserInfo();
    const auto selfName = currentUser.name.isEmpty() ? tr("Me") : currentUser.name;
    const auto selfUid = currentUser.uid;

    ui->chat_page->ClearChatMsgs();

    auto messages = data["messages"].toArray();
    QList<QJsonObject> orderedMessages;
    orderedMessages.reserve(messages.size());
    for (const auto& val : messages) {
        orderedMessages.append(val.toObject());
    }
    std::sort(orderedMessages.begin(), orderedMessages.end(),
              [](const QJsonObject& left, const QJsonObject& right) {
                  return left["msg_id"].toVariant().toLongLong()
                         < right["msg_id"].toVariant().toLongLong();
              });

    QString lastMsg;
    QString lastTime;
    qint64 lastTimestamp = 0;
    for (const auto& obj : orderedMessages) {
        const auto fromUid = obj["from_uid"].toInt();
        const auto content = obj["message"].toString();
        const auto rawSendTime = obj["send_time"].toString();
        const auto isSelf = fromUid == selfUid;
        ui->chat_page->AppendChatMsg(isSelf ? selfName : peerName,
                                     content,
                                     isSelf,
                                     isSelf ? ":/res/head.png" : peerAvatar);
        lastMsg = content;
        lastTime = formatChatTime(rawSendTime);
        lastTimestamp = parseChatTimestamp(rawSendTime);
    }

    if (!lastMsg.isEmpty()) {
        updateChatItemStatus(peerUid, lastMsg, lastTime, 0, lastTimestamp);
    } else {
        setChatItemUnread(peerUid, 0);
    }
    ui->chat_page->ScrollToBottom();
}
