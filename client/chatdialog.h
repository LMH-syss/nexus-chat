#ifndef CHATDIALOG_H
#define CHATDIALOG_H

#include <QWidget>
#include <QMap>
#include <QJsonObject>
#include <QPoint>
#include <QStringList>

class QListWidgetItem;
class QListWidget;
class QMouseEvent;
class QEvent;

namespace Ui {
class ChatDialog;
}

class ChatDialog : public QWidget
{
    Q_OBJECT

public:
    explicit ChatDialog(QWidget *parent = nullptr);
    ~ChatDialog();
    void SetMaximizedState(bool maximized);

signals:
    void sig_send_text_msg(int toUid, QString content);
    void sig_minimize_window();
    void sig_max_restore_window();
    void sig_close_window();

private slots:
    void slot_chat_user_changed(QListWidgetItem *current, QListWidgetItem *previous);
    void slot_contact_item_clicked(QListWidgetItem *item);
    void slot_search_item_clicked(QListWidgetItem *item);
    void slot_side_chat_clicked();
    void slot_side_contact_clicked();
    void slot_search_text_changed(const QString& text);
    void slot_show_search(bool show);
    void slot_notify_auth_friend(QJsonObject data);
    void slot_friend_list_result(QJsonObject data);
    void slot_notify_text_chat_msg(QJsonObject data);
    void slot_text_chat_msg_result(QJsonObject data);
    void slot_chat_history_result(QJsonObject data);

private:
    enum class UIMode {
        Chat,
        Contact,
        Search
    };

    enum ItemRole {
        RoleUid = Qt::UserRole,
        RoleAvatar,
        RoleName,
        RoleKind,
        RoleLastMsg,
        RoleLastTime,
        RoleUnread,
        RoleLastTimestamp
    };

    enum ItemKind {
        KindChat = 1,
        KindContact = 2,
        KindApplyFriend = 3,
        KindGroupTip = 4
    };

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void initUiState();
    void initChatUsers();
    void initContactUsers();
    void addChatUserItem(int uid, const QString& name, const QString& lastMsg,
                         const QString& time, const QString& avatarPath,
                         int unreadCount = 0, qint64 lastTimestamp = 0);
    void addContactUserItem(int uid, const QString& name, const QString& avatarPath);
    void addGroupTipItem(QListWidget *list, const QString& text);
    void addSearchResultItem(int uid, const QString& name, const QString& desc,
                             const QString& avatarPath, ItemKind kind);
    void addSearchTipItem(const QString& title, const QString& desc);
    void rebuildSearchList(const QString& keyword);
    void showSearch(bool show);
    void switchToChatMode();
    void switchToContactMode();
    void jumpToChatItem(int uid, const QString& name, const QString& avatarPath);
    void updateFriendInfo(int uid, const QString& name, const QString& avatarPath);
    void requestChatHistory(int peerUid);
    void markChatRead(int peerUid);
    void updateChatItemStatus(int uid, const QString& lastMsg,
                              const QString& time, int unreadCount,
                              qint64 lastTimestamp = 0);
    void setChatItemUnread(int uid, int unreadCount);
    void sortChatItemsKeepSelection();
    qint64 parseChatTimestamp(const QString& rawTime) const;
    void updateWindowControlGeometry();
    bool isInWindowDragArea(const QPoint& globalPos) const;
    QString formatChatTime(const QString& rawTime) const;
    int currentToUid() const;

private:
    Ui::ChatDialog *ui;
    UIMode _mode = UIMode::Chat;
    UIMode _state = UIMode::Chat;
    QWidget *_lastContactPage = nullptr;
    QMap<int, QListWidgetItem*> _chatItems;
    QMap<int, QListWidgetItem*> _contactItems;
    QMap<int, QStringList> _pendingTextMessages;
    bool _windowDragging = false;
    QPoint _dragStartGlobalPos;
    QPoint _dragStartFramePos;
};

#endif // CHATDIALOG_H
