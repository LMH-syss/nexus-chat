#ifndef RESETDIALOG_H
#define RESETDIALOG_H

#include <QWidget>
#include <QPoint>
#include "global.h"

namespace Ui {
class ResetDialog;
}

class ResetDialog : public QWidget
{
    Q_OBJECT

public:
    explicit ResetDialog(QWidget *parent = nullptr);
    ~ResetDialog();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void on_reset_close_btn_clicked();
    void on_return_btn_clicked();

    void on_varify_btn_clicked();

    void slot_reset_mod_finish(ReqId id, QString res, ErrorCodes err);
    void on_sure_btn_clicked();

private:
    bool checkUserValid();
    bool checkPassValid();
    void showTip(QString str,bool b_ok);
    bool checkEmailValid();
    bool checkVarifyValid();
    void AddTipErr(TipErr te,QString tips);
    void DelTipErr(TipErr te);
    void initHandlers();
    Ui::ResetDialog *ui;
    QMap<TipErr, QString> _tip_errs;
    QMap<ReqId, std::function<void(const QJsonObject&)>> _handlers;
    bool _windowDragging = false;
    QPoint _dragStartGlobalPos;
    QPoint _dragStartFramePos;

signals:
    void switchLogin();
};

#endif // RESETDIALOG_H