#include "ChatGrpcClient.h"

#include <chrono>
#include <iostream>

#include <grpcpp/grpcpp.h>

#include "ConfigMgr.h"
#include "const.h"
#include "message.grpc.pb.h"

using grpc::ClientContext;
using grpc::Status;

std::string ChatGrpcClient::BuildServerAddress(const std::string& server_name)
{
    auto& cfg = ConfigMgr::Inst();
    auto host = cfg[server_name]["Host"];
    auto rpc_port = cfg[server_name]["RPCPort"];
    if (host.empty() || rpc_port.empty()) {
        std::cout << "missing grpc config for " << server_name << std::endl;
        return "";
    }

    return host + ":" + rpc_port;
}

bool ChatGrpcClient::NotifyAddFriend(const std::string& server_name, int apply_uid,
                                     const std::string& name, const std::string& desc,
                                     int to_uid)
{
    auto address = BuildServerAddress(server_name);
    if (address.empty()) {
        return false;
    }

    auto channel = grpc::CreateChannel(address, grpc::InsecureChannelCredentials());
    auto stub = message::ChatService::NewStub(channel);

    ClientContext context;
    context.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(3));

    message::AddFriendReq request;
    message::AddFriendRsp reply;
    request.set_applyuid(apply_uid);
    request.set_name(name);
    request.set_desc(desc);
    request.set_touid(to_uid);

    Status status = stub->NotifyAddFriend(&context, request, &reply);
    if (!status.ok()) {
        std::cout << "NotifyAddFriend grpc failed: " << status.error_message() << std::endl;
        return false;
    }

    return reply.error() == ErrorCodes::Success;
}

bool ChatGrpcClient::RplyAddFriend(const std::string& server_name, int rply_uid,
                                   bool agree, int to_uid)
{
    auto address = BuildServerAddress(server_name);
    if (address.empty()) {
        return false;
    }

    auto channel = grpc::CreateChannel(address, grpc::InsecureChannelCredentials());
    auto stub = message::ChatService::NewStub(channel);

    ClientContext context;
    context.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(3));

    message::RplyFriendReq request;
    message::RplyFriendRsp reply;
    request.set_rplyuid(rply_uid);
    request.set_agree(agree);
    request.set_touid(to_uid);

    Status status = stub->RplyAddFriend(&context, request, &reply);
    if (!status.ok()) {
        std::cout << "RplyAddFriend grpc failed: " << status.error_message() << std::endl;
        return false;
    }

    return reply.error() == ErrorCodes::Success;
}

bool ChatGrpcClient::NotifyTextChatMsg(const std::string& server_name, int from_uid,
                                       int to_uid, const std::string& message)
{
    auto address = BuildServerAddress(server_name);
    if (address.empty()) {
        return false;
    }

    auto channel = grpc::CreateChannel(address, grpc::InsecureChannelCredentials());
    auto stub = message::ChatService::NewStub(channel);

    ClientContext context;
    context.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(3));

    message::TextChatMsgReq request;
    message::TextChatMsgRsp reply;
    request.set_fromuid(from_uid);
    request.set_touid(to_uid);
    auto text_msg = request.add_textmsgs();
    text_msg->set_msgid("");
    text_msg->set_msgcontent(message);

    Status status = stub->NotifyTextChatMsg(&context, request, &reply);
    if (!status.ok()) {
        std::cout << "NotifyTextChatMsg grpc failed: " << status.error_message() << std::endl;
        return false;
    }

    return reply.error() == ErrorCodes::Success;
}
