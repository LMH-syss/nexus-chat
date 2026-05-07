#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QBitmap>
#include <QGuiApplication>
#include <QMenuBar>
#include <QPainter>
#include <QPainterPath>
#include <QResizeEvent>
#include <QScreen>
#include <QStatusBar>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setAutoFillBackground(true);
    setAttribute(Qt::WA_StyledBackground, true);
    if (ui->menubar) {
        ui->menubar->hide();
    }
    if (ui->statusbar) {
        ui->statusbar->setSizeGripEnabled(false);
        ui->statusbar->hide();
    }
    //
    setWindowFlag(Qt::FramelessWindowHint, true);
    _login_dlg = new LoginDialog(this);
    setCentralWidget(_login_dlg);
    applyAuthWindowShape();

    //
    connect(_login_dlg, &LoginDialog::switchRegister, this, &MainWindow::SlotSwitchReg);
    //
    connect(_login_dlg, &LoginDialog::switchReset, this, &MainWindow::SlotSwitchReset);
    connect(_login_dlg, &LoginDialog::switchChat, this, &MainWindow::SlotSwitchChat);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::SlotSwitchReg()
{
    _auth_window_shape_enabled = true;
    _chat_dlg = nullptr;
    _reg_dlg = new RegisterDialog(this);
    _reg_dlg->hide();

    //
    connect(_reg_dlg, &RegisterDialog::sigSwitchLogin, this, &MainWindow::SlotSwitchLogin);
    setMaximumSize(300, 410);
    resize(300, 410);
    setCentralWidget(_reg_dlg);
    applyAuthWindowShape();
    _login_dlg->hide();
    _reg_dlg->show();
}

//
void MainWindow::SlotSwitchLogin()
{
    _auth_window_shape_enabled = true;
    _chat_dlg = nullptr;
    //
    _login_dlg = new LoginDialog(this);
    setMaximumSize(300, 380);
    resize(300, 380);
    setCentralWidget(_login_dlg);
    applyAuthWindowShape();

    _reg_dlg->hide();
    _login_dlg->show();
    //
    connect(_login_dlg, &LoginDialog::switchRegister, this, &MainWindow::SlotSwitchReg);
    //
    connect(_login_dlg, &LoginDialog::switchReset, this, &MainWindow::SlotSwitchReset);
    connect(_login_dlg, &LoginDialog::switchChat, this, &MainWindow::SlotSwitchChat);
}

void MainWindow::SlotSwitchLogin2()
{
    _auth_window_shape_enabled = true;
    _chat_dlg = nullptr;
    //
    _login_dlg = new LoginDialog(this);
    setMaximumSize(300, 380);
    resize(300, 380);
    setCentralWidget(_login_dlg);
    applyAuthWindowShape();

    _reset_dlg->hide();
    _login_dlg->show();
    //
    connect(_login_dlg, &LoginDialog::switchReset, this, &MainWindow::SlotSwitchReset);
    connect(_login_dlg, &LoginDialog::switchChat, this, &MainWindow::SlotSwitchChat);
    //
    connect(_login_dlg, &LoginDialog::switchRegister, this, &MainWindow::SlotSwitchReg);
}

void MainWindow::SlotSwitchReset()
{
    _auth_window_shape_enabled = true;
    _chat_dlg = nullptr;
    //
    _reset_dlg = new ResetDialog(this);
    setMaximumSize(300, 380);
    resize(300, 380);
    setCentralWidget(_reset_dlg);
    applyAuthWindowShape();

    _login_dlg->hide();
    _reset_dlg->show();
    //
    connect(_reset_dlg, &ResetDialog::switchLogin, this, &MainWindow::SlotSwitchLogin2);
}
void MainWindow::SlotSwitchChat()
{
    _auth_window_shape_enabled = false;
    clearAuthWindowShape();
    _chat_dlg = new ChatDialog(this);
    setWindowFlag(Qt::FramelessWindowHint, true);
    clearAuthWindowShape();
    setMinimumSize(QSize(900, 650));
    setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    resize(1000, 700);
    centerOnCurrentScreen();
    setCentralWidget(_chat_dlg);
    connect(_chat_dlg, &ChatDialog::sig_minimize_window, this, &MainWindow::showMinimized);
    connect(_chat_dlg, &ChatDialog::sig_max_restore_window, this, [this]() {
        if (isMaximized()) {
            showNormal();
        } else {
            showMaximized();
        }

        if (_chat_dlg) {
            _chat_dlg->SetMaximizedState(isMaximized());
        }
    });
    connect(_chat_dlg, &ChatDialog::sig_close_window, this, &MainWindow::close);
    _chat_dlg->show();
    show();
    clearAuthWindowShape();
    update();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    if (_auth_window_shape_enabled) {
        applyAuthWindowShape();
    }
}

void MainWindow::applyAuthWindowShape()
{
    if (!_auth_window_shape_enabled) {
        return;
    }

    const int radius = 12;
    if (width() <= 0 || height() <= 0) {
        return;
    }

    QBitmap mask(size());
    mask.fill(Qt::color0);

    QPainter painter(&mask);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(Qt::color1);
    painter.setPen(Qt::NoPen);

    QPainterPath path;
    path.addRoundedRect(rect(), radius, radius);
    painter.drawPath(path);
    setMask(mask);
}

void MainWindow::clearAuthWindowShape()
{
    clearMask();
    setMask(QRegion());
}

void MainWindow::centerOnCurrentScreen()
{
    QScreen *screen = QGuiApplication::screenAt(frameGeometry().center());
    if (!screen) {
        screen = QGuiApplication::primaryScreen();
    }
    if (!screen) {
        return;
    }

    const QRect available = screen->availableGeometry();
    move(available.center() - rect().center());
}
