#ifndef CONTACTUSERITEM_H
#define CONTACTUSERITEM_H

#include <QWidget>

namespace Ui {
class ContactUserItem;
}

class ContactUserItem : public QWidget
{
    Q_OBJECT
public:
    explicit ContactUserItem(QWidget *parent = nullptr);
    ~ContactUserItem();

    void SetInfo(int uid, const QString& name, const QString& avatarPath);
    void ShowRedPoint(bool show);
    int uid() const;
    QString userName() const;
    QString avatarPath() const;

private:
    Ui::ContactUserItem *ui;
    int _uid = 0;
    QString _avatarPath;
};

#endif // CONTACTUSERITEM_H
