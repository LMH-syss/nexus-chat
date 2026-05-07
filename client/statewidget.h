#ifndef STATEWIDGET_H
#define STATEWIDGET_H

#include "global.h"

#include <QWidget>

class QLabel;

class StateWidget : public QWidget
{
    Q_OBJECT
public:
    explicit StateWidget(QWidget *parent = nullptr);

    void SetState(const QString& normal, const QString& hover, const QString& press,
                  const QString& selected, const QString& selectedHover,
                  const QString& selectedPress);
    void SetSelected(bool selected);
    ClickLbState GetCurState() const;

protected:
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

signals:
    void clicked();

private:
    void applyState(const QString& state);

private:
    QString _normal;
    QString _normalHover;
    QString _normalPress;
    QString _selected;
    QString _selectedHover;
    QString _selectedPress;
    ClickLbState _curState = ClickLbState::Normal;
};

#endif // STATEWIDGET_H
