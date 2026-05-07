#include "statewidget.h"

#include <QEnterEvent>
#include <QMouseEvent>

StateWidget::StateWidget(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setCursor(Qt::PointingHandCursor);
}

void StateWidget::SetState(const QString& normal, const QString& hover, const QString& press,
                           const QString& selected, const QString& selectedHover,
                           const QString& selectedPress)
{
    _normal = normal;
    _normalHover = hover;
    _normalPress = press;
    _selected = selected;
    _selectedHover = selectedHover;
    _selectedPress = selectedPress;
    applyState(_curState == ClickLbState::Selected ? _selected : _normal);
}

void StateWidget::SetSelected(bool selected)
{
    _curState = selected ? ClickLbState::Selected : ClickLbState::Normal;
    applyState(selected ? _selected : _normal);
}

ClickLbState StateWidget::GetCurState() const
{
    return _curState;
}

void StateWidget::enterEvent(QEnterEvent *event)
{
    applyState(_curState == ClickLbState::Selected ? _selectedHover : _normalHover);
    QWidget::enterEvent(event);
}

void StateWidget::leaveEvent(QEvent *event)
{
    applyState(_curState == ClickLbState::Selected ? _selected : _normal);
    QWidget::leaveEvent(event);
}

void StateWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        applyState(_curState == ClickLbState::Selected ? _selectedPress : _normalPress);
    }
    QWidget::mousePressEvent(event);
}

void StateWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && rect().contains(event->pos())) {
        _curState = ClickLbState::Selected;
        applyState(_selectedHover);
        emit clicked();
        return;
    }

    applyState(_curState == ClickLbState::Selected ? _selected : _normal);
    QWidget::mouseReleaseEvent(event);
}

void StateWidget::applyState(const QString& state)
{
    setProperty("state", state);
    repolish(this);
    update();
}
