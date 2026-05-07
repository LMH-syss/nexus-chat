#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QWidget>
#include <QPoint>
#include <global.h>
namespace Ui {
class RegisterDialog;
}

class RegisterDialog : public QWidget
{
    Q_OBJECT

public:
    explicit RegisterDialog(QWidget *parent = nullptr);
    ~RegisterDialog();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void on_reg_close_btn_clicked();
    void on_get_code_clicked();
    void slot_reg_mod_finish(ReqId id,QString res,ErrorCodes err);
    void on_sure_btn_clicked();
    void on_return_btn_clicked();
    void on_cancel_btn_clicked();
private:
    void initHttpHandlers();
    void showTip(QString str, bool ok);
    bool checkUserValid();
    bool checkPassValid();
    bool checkConfirmValid();
    bool checkEmailValid();
    bool checkVarifyValid();
    void AddTipErr(TipErr te, QString tips);
    void DelTipErr(TipErr te);
    void ChangeTipPage();

    QMap<TipErr, QString> _tip_errs;
    Ui::RegisterDialog *ui;
    QMap<ReqId, std::function<void(const QJsonObject&)>> _handlers;
    QTimer * _countdown_timer;
    int _countdown = 5;
    bool _windowDragging = false;
    QPoint _dragStartGlobalPos;
    QPoint _dragStartFramePos;

signals:
    void sigSwitchLogin();
};

#endif // REGISTERDIALOG_H
