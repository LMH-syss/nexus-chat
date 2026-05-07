#include "clickedbtn.h"
#include "global.h"

#include <QEnterEvent>
#include <QMouseEvent>

ClickedBtn::ClickedBtn(QWidget *parent)
    : QPushButton(parent)
{
    setCursor(Qt::PointingHandCursor);
}

void ClickedBtn::SetState(const QString& normal, const QString& hover, const QString& press)
{
    _normal = normal;
    _hover = hover;
    _press = press;
    setProperty("state", _normal);
    repolish(this);
}

void ClickedBtn::enterEvent(QEnterEvent *event)
{
    setProperty("state", _hover);
    repolish(this);
    QPushButton::enterEvent(event);
}

void ClickedBtn::leaveEvent(QEvent *event)
{
    setProperty("state", _normal);
    repolish(this);
    QPushButton::leaveEvent(event);
}

void ClickedBtn::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        setProperty("state", _press);
        repolish(this);
    }
    QPushButton::mousePressEvent(event);
}

void ClickedBtn::mouseReleaseEvent(QMouseEvent *event)
{
    setProperty("state", rect().contains(event->pos()) ? _hover : _normal);
    repolish(this);
    QPushButton::mouseReleaseEvent(event);
}
