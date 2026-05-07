#include "chatpage.h"
#include "ui_chatpage.h"

#include "ChatItemBase.h"
#include "TextBubble.h"
#include "clickedbtn.h"
#include "clickedlabel.h"

ChatPage::ChatPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ChatPage)
{
    ui->setupUi(this);

    ui->emo_lb->SetState("normal", "hover", "pressed", "normal", "hover", "pressed");
    ui->file_lb->SetState("normal", "hover", "pressed", "normal", "hover", "pressed");
    ui->send_btn->SetState("normal", "hover", "press");
}

ChatPage::~ChatPage()
{
    delete ui;
}

void ChatPage::SetUserInfo(int uid, const QString& name, const QString& avatarPath)
{
    _toUid = uid;
    _toName = name;
    _toAvatar = avatarPath;
    ui->title_lb->setText(name.isEmpty() ? tr("Select a chat") : name);
    ClearChatMsgs();
}

void ChatPage::AppendChatMsg(const QString& sender, const QString& content,
                             bool self, const QString& avatarPath)
{
    const auto role = self ? ChatRole::Self : ChatRole::Other;
    auto item = new ChatItemBase(role, sender, avatarPath, ui->chat_data_list);
    auto bubble = new TextBubble(role, content, item);
    item->setBubble(bubble);
    ui->chat_data_list->appendChatItem(item);
}

void ChatPage::ClearChatMsgs()
{
    ui->chat_data_list->clearItems();
}

void ChatPage::ScrollToBottom()
{
    ui->chat_data_list->scrollToBottom();
}

void ChatPage::slot_show_incoming_msg(int from_uid, const QString& from_name, const QString& message)
{
    if (from_uid == _toUid) {
        AppendChatMsg(from_name, message, false, ":/res/head_1.jpg");
    }
}

void ChatPage::on_send_btn_clicked()
{
    const auto content = ui->chatEdit->toPlainText().trimmed();
    if (content.isEmpty() || _toUid <= 0) {
        return;
    }

    ui->chatEdit->clear();
    emit sig_send_text_msg(_toUid, content);
}
