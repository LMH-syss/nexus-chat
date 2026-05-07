#ifndef ADDFRIENDSEARCHDLG_H
#define ADDFRIENDSEARCHDLG_H

#include <QDialog>
#include <QJsonObject>
#include <QPoint>

class QMouseEvent;

namespace Ui {
class AddFriendSearchDlg;
}

class AddFriendSearchDlg : public QDialog
{
    Q_OBJECT

public:
    explicit AddFriendSearchDlg(QWidget *parent = nullptr);
    ~AddFriendSearchDlg();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

signals:
    void sig_search_user(QString keyword);
    void sig_add_friend(int to_uid);

public slots:
    void slot_search_user_result(QJsonObject data);
    void slot_add_friend_result(QJsonObject data);

private slots:
    void on_close_btn_clicked();
    void on_search_btn_clicked();
    void on_add_friend_btn_clicked();

private:
    Ui::AddFriendSearchDlg *ui;
    int _search_result_uid = 0;
    bool _dragging = false;
    QPoint _dragPosition;
};

#endif // ADDFRIENDSEARCHDLG_H
