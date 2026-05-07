#pragma once
#include <grpcpp/grpcpp.h>
#include <mutex>
#include <vector>
#include <string>
#include "message.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using message::GetChatServerReq;
using message::GetChatServerRsp;
using message::LoginReq;
using message::LoginRsp;
using message::StatusService;

struct ChatServer {
	std::string name;
	std::string host;
	std::string port;
};
class StatusServiceImpl final : public StatusService::Service
{
public:
	StatusServiceImpl();
	Status GetChatServer(ServerContext* context, const GetChatServerReq* request,
		GetChatServerRsp* reply) override;
	Status Login(ServerContext* context, const LoginReq* request,
		LoginRsp* reply) override;

	std::vector<ChatServer> _servers;
	int _server_index;
	std::mutex _server_mtx;
};
