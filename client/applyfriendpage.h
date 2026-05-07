#ifndef APPLYFRIENDPAGE_H
#define APPLYFRIENDPAGE_H

#include <QWidget>
#include <QJsonObject>

namespace Ui {
class ApplyFriendPage;
}

class ApplyFriendPage : public QWidget
{
    Q_OBJECT
public:
    explicit ApplyFriendPage(QWidget *parent = nullptr);
    ~ApplyFriendPage();

    void AddApply(int uid, const QString& name, const QString& desc,
                  const QString& avatarPath, bool handled = false);

public slots:
    void slot_incoming_apply(QJsonObject data);
    void slot_apply_list_result(QJsonObject data);
    void slot_auth_friend_result(QJsonObject data);

signals:
    void sig_show_search(bool show);
    void sig_auth_friend(int from_uid);
    void sig_friend_accepted(int from_uid);

private:
    void loadApplyList();
    void onAcceptClicked(int from_uid);

private:
    Ui::ApplyFriendPage *ui;
};

#endif // APPLYFRIENDPAGE_H
