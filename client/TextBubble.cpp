#include "TextBubble.h"

#include <QLabel>
#include <QVBoxLayout>

TextBubble::TextBubble(ChatRole role, const QString& text, QWidget *parent)
    : BubbleFrame(role, parent)
{
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
    setMaximumWidth(420);

    auto layout = new QVBoxLayout(this);
    if (role == ChatRole::Self) {
        layout->setContentsMargins(12, 8, 20, 8);
    } else {
        layout->setContentsMargins(20, 8, 12, 8);
    }

    auto label = new QLabel(text, this);
    label->setObjectName("bubble_text");
    label->setProperty("chatRole", role == ChatRole::Self ? "self" : "other");
    label->setWordWrap(true);
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    label->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
    layout->addWidget(label);
}
