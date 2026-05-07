#ifndef CHATPAGE_H
#define CHATPAGE_H

#include <QWidget>

namespace Ui {
class ChatPage;
}

class ChatPage : public QWidget
{
    Q_OBJECT
public:
    explicit ChatPage(QWidget *parent = nullptr);
    ~ChatPage();

    void SetUserInfo(int uid, const QString& name, const QString& avatarPath);
    void AppendChatMsg(const QString& sender, const QString& content,
                       bool self, const QString& avatarPath);
    void ClearChatMsgs();
    void ScrollToBottom();

public slots:
    void slot_show_incoming_msg(int from_uid, const QString& from_name, const QString& message);

signals:
    void sig_send_text_msg(int toUid, QString content);

private slots:
    void on_send_btn_clicked();

private:
    Ui::ChatPage *ui;
    int _toUid = 0;
    QString _toName;
    QString _toAvatar;
};

#endif // CHATPAGE_H
