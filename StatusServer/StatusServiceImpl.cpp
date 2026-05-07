#include "StatusServiceImpl.h"
#include "ConfigMgr.h"
#include "const.h"
#include "RedisMgr.h"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

std::string generate_unique_string() {
	// 创建UUID对象
	boost::uuids::uuid uuid = boost::uuids::random_generator()();

	// 将UUID转换为字符串
	std::string unique_string = to_string(uuid);

	return unique_string;
}

Status StatusServiceImpl::GetChatServer(ServerContext* context, const GetChatServerReq* request, GetChatServerRsp* reply)
{
	if (request->uid() <= 0 || _servers.empty()) {
		reply->set_error(ErrorCodes::UidInvalid);
		return Status::OK;
	}

	auto token_key = std::string(USERTOKENPREFIX) + std::to_string(request->uid());
	auto server_key = std::string(USERSERVERPREFIX) + std::to_string(request->uid());

	// 检查 Redis 中是否已有有效 token 和 server 分配记录
	std::string existing_token;
	std::string existing_server_name;
	bool has_token = RedisMgr::GetInstance()->Get(token_key, existing_token);
	bool has_server = RedisMgr::GetInstance()->Get(server_key, existing_server_name);

	if (has_token && has_server) {
		// 验证 server name 是否仍在 _servers 列表中
		for (auto& srv : _servers) {
			if (srv.name == existing_server_name) {
				reply->set_host(srv.host);
				reply->set_port(srv.port);
				reply->set_token(existing_token);
				reply->set_error(ErrorCodes::Success);
				std::cout << "uid " << request->uid() << " already assigned to " << existing_server_name << ", reuse" << std::endl;
				std::cout << "[StatusServer] return server name=[" << srv.name
					<< "], host=[" << srv.host
					<< "], host.size=" << srv.host.size()
					<< ", port=[" << srv.port << "]" << std::endl;
				return Status::OK;
			}
		}
		// server 不存在于当前列表，走新分配逻辑
		std::cout << "uid " << request->uid() << " server " << existing_server_name << " not found, reassign" << std::endl;
	}

	// 新分配：轮询选择服务器
	std::string host, port, server_name;
	{
		std::lock_guard<std::mutex> lock(_server_mtx);
		auto& server = _servers[_server_index];
		_server_index = (_server_index + 1) % _servers.size();
		host = server.host;
		port = server.port;
		server_name = server.name;
	}

	auto token = generate_unique_string();

	if (!RedisMgr::GetInstance()->SetEx(token_key, token, 600)) {
		reply->set_error(ErrorCodes::RPCGetFailed);
		std::cout << "Failed to set token in Redis for uid: " << request->uid() << std::endl;
		return Status::OK;
	}

	if (!RedisMgr::GetInstance()->SetEx(server_key, server_name, 600)) {
		reply->set_error(ErrorCodes::RPCGetFailed);
		std::cout << "Failed to set server mapping in Redis for uid: " << request->uid() << std::endl;
		return Status::OK;
	}

	reply->set_host(host);
	reply->set_port(port);
	reply->set_error(ErrorCodes::Success);
	reply->set_token(token);
	std::cout << "uid " << request->uid() << " assigned to " << server_name << " (" << host << ":" << port << ")" << std::endl;
	std::cout << "[StatusServer] return server name=[" << server_name
		<< "], host=[" << host
		<< "], host.size=" << host.size()
		<< ", port=[" << port << "]" << std::endl;
	return Status::OK;
}

Status StatusServiceImpl::Login(ServerContext* context, const LoginReq* request, LoginRsp* reply)
{
	if (request->uid() <= 0) {
		reply->set_error(ErrorCodes::UidInvalid);
		return Status::OK;
	}

	std::string stored_token;
	auto token_key = std::string(USERTOKENPREFIX) + std::to_string(request->uid());
	if (!RedisMgr::GetInstance()->Get(token_key, stored_token) || stored_token != request->token()) {
		reply->set_error(ErrorCodes::TokenInvalid);
		reply->set_uid(request->uid());
		std::cout << "Token validation failed for uid: " << request->uid() << std::endl;
		return Status::OK;
	}

	reply->set_error(ErrorCodes::Success);
	reply->set_uid(request->uid());
	reply->set_token(request->token());
	return Status::OK;
}
StatusServiceImpl::StatusServiceImpl() :_server_index(0)
{
	auto& cfg = ConfigMgr::Inst();
	ChatServer server;
	server.name = "ChatServer1";
	server.port = cfg["ChatServer1"]["Port"];
	server.host = cfg["ChatServer1"]["Host"];
	_servers.push_back(server);
	std::cout << "[StatusServer] load server name=[" << server.name
		<< "], host=[" << server.host
		<< "], host.size=" << server.host.size()
		<< ", port=[" << server.port << "]" << std::endl;

	server.name = "ChatServer2";
	server.port = cfg["ChatServer2"]["Port"];
	server.host = cfg["ChatServer2"]["Host"];
	_servers.push_back(server);
	std::cout << "[StatusServer] load server name=[" << server.name
		<< "], host=[" << server.host
		<< "], host.size=" << server.host.size()
		<< ", port=[" << server.port << "]" << std::endl;
}
