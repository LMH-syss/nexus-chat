#pragma once

#include <grpcpp/grpcpp.h>

#include "message.grpc.pb.h"

class ChatServiceImpl final : public message::ChatService::Service {
public:
    grpc::Status NotifyAddFriend(grpc::ServerContext* context,
                                 const message::AddFriendReq* request,
                                 message::AddFriendRsp* reply) override;
    grpc::Status RplyAddFriend(grpc::ServerContext* context,
                               const message::RplyFriendReq* request,
                               message::RplyFriendRsp* reply) override;
    grpc::Status SendChatMsg(grpc::ServerContext* context,
                             const message::SendChatMsgReq* request,
                             message::SendChatMsgRsp* reply) override;
    grpc::Status NotifyAuthFriend(grpc::ServerContext* context,
                                  const message::AuthFriendReq* request,
                                  message::AuthFriendRsp* reply) override;
    grpc::Status NotifyTextChatMsg(grpc::ServerContext* context,
                                   const message::TextChatMsgReq* request,
                                   message::TextChatMsgRsp* reply) override;
};
