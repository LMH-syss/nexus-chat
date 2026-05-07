#include "ChatItemBase.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QVBoxLayout>

ChatItemBase::ChatItemBase(ChatRole role, const QString& userName,
                           const QString& avatarPath, QWidget *parent)
    : QWidget(parent)
    , _role(role)
{
    Q_UNUSED(userName);

    auto rootLayout = new QHBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(8);

    auto avatar = new QLabel(this);
    avatar->setObjectName("chat_user_icon");
    avatar->setFixedSize(36, 36);
    avatar->setScaledContents(true);
    avatar->setPixmap(QPixmap(avatarPath));

    _bubbleLayout = new QVBoxLayout;
    _bubbleLayout->setContentsMargins(0, 0, 0, 0);
    _bubbleLayout->setSpacing(0);

    if (_role == ChatRole::Other) {
        rootLayout->addWidget(avatar, 0, Qt::AlignTop);
        rootLayout->addLayout(_bubbleLayout, 0);
        rootLayout->addStretch();
    } else {
        rootLayout->addStretch();
        rootLayout->addLayout(_bubbleLayout, 0);
        rootLayout->addWidget(avatar, 0, Qt::AlignTop);
    }
}

void ChatItemBase::setBubble(QWidget *bubble)
{
    _bubbleLayout->addWidget(bubble, 0, _role == ChatRole::Self ? Qt::AlignRight : Qt::AlignLeft);
}
