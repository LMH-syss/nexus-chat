#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <logindialog.h>
#include <registerdialog.h>
#include <resetdialog.h>
#include <chatdialog.h>
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;
public slots:
    void SlotSwitchReg();
    void SlotSwitchLogin();
    void SlotSwitchLogin2();
    void SlotSwitchReset();
    void SlotSwitchChat();
protected:
    void resizeEvent(QResizeEvent *event) override;
private:
    void applyAuthWindowShape();
    void clearAuthWindowShape();
    void centerOnCurrentScreen();

    Ui::MainWindow *ui;
    LoginDialog * _login_dlg = nullptr;
    RegisterDialog * _reg_dlg = nullptr;
    ResetDialog * _reset_dlg = nullptr;
    ChatDialog * _chat_dlg = nullptr;
    bool _auth_window_shape_enabled = true;
};
#endif // MAINWINDOW_H
