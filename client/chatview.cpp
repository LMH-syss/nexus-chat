#include "chatview.h"

#include <QScrollArea>
#include <QScrollBar>
#include <QTimer>
#include <QVBoxLayout>

ChatView::ChatView(QWidget *parent)
    : QWidget(parent)
{
    auto rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);

    _scrollArea = new QScrollArea(this);
    _scrollArea->setObjectName("chat_scroll_area");
    _scrollArea->setWidgetResizable(true);
    _scrollArea->setFrameShape(QFrame::NoFrame);
    _scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    _centerWidget = new QWidget(_scrollArea);
    _centerWidget->setObjectName("chat_scroll_content");
    _layout = new QVBoxLayout(_centerWidget);
    _layout->setContentsMargins(16, 16, 16, 16);
    _layout->setSpacing(12);
    _layout->addStretch();

    _scrollArea->setWidget(_centerWidget);
    rootLayout->addWidget(_scrollArea);
}

void ChatView::appendChatItem(QWidget *item)
{
    _layout->insertWidget(_layout->count() - 1, item);
    scrollToBottom();
}

void ChatView::scrollToBottom()
{
    const auto doScroll = [this]() {
        auto bar = _scrollArea->verticalScrollBar();
        bar->setValue(bar->maximum());
    };

    QTimer::singleShot(0, this, doScroll);
    QTimer::singleShot(30, this, doScroll);
}

void ChatView::clearItems()
{
    while (_layout->count() > 1) {
        auto item = _layout->takeAt(0);
        if (auto widget = item->widget()) {
            widget->deleteLater();
        }
        delete item;
    }
}
