#ifndef TEXTBUBBLE_H
#define TEXTBUBBLE_H

#include "BubbleFrame.h"

class QLabel;

class TextBubble : public BubbleFrame
{
    Q_OBJECT
public:
    explicit TextBubble(ChatRole role, const QString& text, QWidget *parent = nullptr);
};

#endif // TEXTBUBBLE_H
