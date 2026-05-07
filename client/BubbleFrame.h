#ifndef BUBBLEFRAME_H
#define BUBBLEFRAME_H

#include <QFrame>

enum class ChatRole
{
    Self,
    Other
};

class BubbleFrame : public QFrame
{
    Q_OBJECT
public:
    explicit BubbleFrame(ChatRole role, QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    ChatRole _role;
};

#endif // BUBBLEFRAME_H
