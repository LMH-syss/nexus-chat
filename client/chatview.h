#ifndef CHATVIEW_H
#define CHATVIEW_H

#include <QWidget>

class QScrollArea;
class QVBoxLayout;

class ChatView : public QWidget
{
    Q_OBJECT
public:
    explicit ChatView(QWidget *parent = nullptr);

    void appendChatItem(QWidget *item);
    void clearItems();
    void scrollToBottom();

private:
    QScrollArea *_scrollArea = nullptr;
    QWidget *_centerWidget = nullptr;
    QVBoxLayout *_layout = nullptr;
};

#endif // CHATVIEW_H
