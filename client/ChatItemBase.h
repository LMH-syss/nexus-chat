#ifndef CHATITEMBASE_H
#define CHATITEMBASE_H

#include "BubbleFrame.h"

#include <QWidget>

class QHBoxLayout;
class QVBoxLayout;

class ChatItemBase : public QWidget
{
    Q_OBJECT
public:
    explicit ChatItemBase(ChatRole role, const QString& userName,
                          const QString& avatarPath, QWidget *parent = nullptr);

    void setBubble(QWidget *bubble);

private:
    ChatRole _role;
    QVBoxLayout *_bubbleLayout = nullptr;
};

#endif // CHATITEMBASE_H
