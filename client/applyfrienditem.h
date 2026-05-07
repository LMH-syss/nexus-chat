#ifndef APPLYFRIENDITEM_H
#define APPLYFRIENDITEM_H

#include <QWidget>

namespace Ui {
class ApplyFriendItem;
}

class ApplyFriendItem : public QWidget
{
    Q_OBJECT
public:
    explicit ApplyFriendItem(QWidget *parent = nullptr);
    ~ApplyFriendItem();

    void SetInfo(int uid, const QString& name, const QString& desc,
                 const QString& avatarPath, bool handled);
    void ShowAddBtn(bool show);
    int uid() const;

signals:
    void sig_accept_friend(int uid);

private:
    Ui::ApplyFriendItem *ui;
    int _uid = 0;
};

#endif // APPLYFRIENDITEM_H
