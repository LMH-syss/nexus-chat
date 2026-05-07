#include "ChatServiceImpl.h"

#include "LogicSystem.h"
#include "const.h"

grpc::Status ChatServiceImpl::NotifyAddFriend(grpc::ServerContext* context,
                                              const message::AddFriendReq* request,
                                              message::AddFriendRsp* reply)
{
    bool delivered = LogicSystem::GetInstance()->DeliverAddFriendNotify(
        request->applyuid(),
        request->name(),
        request->touid(),
        request->desc());

    reply->set_error(delivered ? ErrorCodes::Success : ErrorCodes::UidInvalid);
    reply->set_applyuid(request->applyuid());
    reply->set_touid(request->touid());
    return grpc::Status::OK;
}

grpc::Status ChatServiceImpl::RplyAddFriend(grpc::ServerContext* context,
                                            const message::RplyFriendReq* request,
                                            message::RplyFriendRsp* reply)
{
    bool delivered = LogicSystem::GetInstance()->DeliverAuthFriendNotify(
        request->rplyuid(),
        request->touid(),
        request->agree());

    reply->set_error(delivered ? ErrorCodes::Success : ErrorCodes::UidInvalid);
    reply->set_rplyuid(request->rplyuid());
    reply->set_touid(request->touid());
    return grpc::Status::OK;
}

grpc::Status ChatServiceImpl::SendChatMsg(grpc::ServerContext* context,
                                          const message::SendChatMsgReq* request,
                                          message::SendChatMsgRsp* reply)
{
    bool delivered = LogicSystem::GetInstance()->DeliverTextChatNotify(
        request->fromuid(),
        "",
        request->touid(),
        request->message());

    reply->set_error(delivered ? ErrorCodes::Success : ErrorCodes::UidInvalid);
    reply->set_fromuid(request->fromuid());
    reply->set_touid(request->touid());
    return grpc::Status::OK;
}

grpc::Status ChatServiceImpl::NotifyAuthFriend(grpc::ServerContext* context,
                                               const message::AuthFriendReq* request,
                                               message::AuthFriendRsp* reply)
{
    bool delivered = LogicSystem::GetInstance()->DeliverAuthFriendNotify(
        request->fromuid(),
        request->touid(),
        true);

    reply->set_error(delivered ? ErrorCodes::Success : ErrorCodes::UidInvalid);
    reply->set_fromuid(request->fromuid());
    reply->set_touid(request->touid());
    return grpc::Status::OK;
}

grpc::Status ChatServiceImpl::NotifyTextChatMsg(grpc::ServerContext* context,
                                                const message::TextChatMsgReq* request,
                                                message::TextChatMsgRsp* reply)
{
    bool delivered = true;
    for (const auto& text_msg : request->textmsgs()) {
        delivered = LogicSystem::GetInstance()->DeliverTextChatNotify(
            request->fromuid(),
            "",
            request->touid(),
            text_msg.msgcontent()) && delivered;
        auto rsp_msg = reply->add_textmsgs();
        rsp_msg->set_msgid(text_msg.msgid());
        rsp_msg->set_msgcontent(text_msg.msgcontent());
    }

    reply->set_error(delivered ? ErrorCodes::Success : ErrorCodes::UidInvalid);
    reply->set_fromuid(request->fromuid());
    reply->set_touid(request->touid());
    return grpc::Status::OK;
}
