#ifndef CHATUSERWID_H
#define CHATUSERWID_H

#include <QWidget>

namespace Ui {
class ChatUserWid;
}

class ChatUserWid : public QWidget
{
    Q_OBJECT
public:
    explicit ChatUserWid(QWidget *parent = nullptr);
    ~ChatUserWid();

    void SetInfo(int uid, const QString& name, const QString& lastMsg,
                 const QString& time, const QString& avatarPath);
    void UpdateLastMsg(const QString& lastMsg, const QString& time);
    void ShowRedPoint(bool show);
    int uid() const;
    QString userName() const;
    QString avatarPath() const;

private:
    Ui::ChatUserWid *ui;
    int _uid = 0;
    QString _avatarPath;
};

#endif // CHATUSERWID_H
