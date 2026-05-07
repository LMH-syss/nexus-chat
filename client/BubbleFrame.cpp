#include "BubbleFrame.h"

#include <QApplication>
#include <QPainter>
#include <QPainterPath>

BubbleFrame::BubbleFrame(ChatRole role, QWidget *parent)
    : QFrame(parent)
    , _role(role)
{
    setAttribute(Qt::WA_TranslucentBackground);
}

void BubbleFrame::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    constexpr int triangleWidth = 8;
    constexpr int radius = 4;
    const bool darkMode = qApp && qApp->property("llfc_theme").toString() == "dark";
    const QColor fill = _role == ChatRole::Self
                            ? QColor("#9eea6a")
                            : (darkMode ? QColor("#3d3d3d") : QColor("#ffffff"));

    QRect bubbleRect = rect().adjusted(_role == ChatRole::Other ? triangleWidth : 0,
                                       0,
                                       _role == ChatRole::Self ? -triangleWidth : 0,
                                       0);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(fill);

    QPainterPath path;
    path.addRoundedRect(bubbleRect, radius, radius);

    if (_role == ChatRole::Other) {
        QPolygon triangle;
        triangle << QPoint(triangleWidth, 12)
                 << QPoint(0, 18)
                 << QPoint(triangleWidth, 24);
        path.addPolygon(triangle);
    } else {
        const int right = width() - triangleWidth;
        QPolygon triangle;
        triangle << QPoint(right, 12)
                 << QPoint(width(), 18)
                 << QPoint(right, 24);
        path.addPolygon(triangle);
    }

    painter.drawPath(path);
}
