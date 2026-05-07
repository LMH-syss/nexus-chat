#include <iostream>
#include "LogicSystem.h"
#include <csignal>
#include <thread>
#include <mutex>
#include <memory>
#include "AsioIOServicePool.h"
#include "CServer.h"
#include "ChatServiceImpl.h"
#include "ConfigMgr.h"
#include <boost/beast/http.hpp>
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <grpcpp/grpcpp.h>
using namespace std;
bool bstop = false;
std::condition_variable cond_quit;
std::mutex mutex_quit;

std::unique_ptr<grpc::Server> StartGrpcServer(ChatServiceImpl& service)
{
	auto& cfg = ConfigMgr::Inst();
	std::string server_address = cfg["SelfServer"]["Host"] + ":" + cfg["SelfServer"]["RPCPort"];

	grpc::ServerBuilder builder;
	builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
	builder.RegisterService(&service);

	auto server = builder.BuildAndStart();
	if (server) {
		std::cout << "Chat grpc server listening on " << server_address << std::endl;
	}
	return server;
}

int main()
{
	try {
		auto& cfg = ConfigMgr::Inst();
		auto pool = AsioIOServicePool::GetInstance();
		ChatServiceImpl chat_service;
		auto grpc_server = StartGrpcServer(chat_service);
		std::thread grpc_thread;
		if (grpc_server) {
			grpc_thread = std::thread([&grpc_server]() {
				grpc_server->Wait();
				});
		}

		boost::asio::io_context  io_context;
		boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
		signals.async_wait([&io_context, pool, &grpc_server](auto, auto) {
			io_context.stop();
			pool->Stop();
			if (grpc_server) {
				grpc_server->Shutdown();
			}
			});
		auto port_str = cfg["SelfServer"]["Port"];
		CServer s(io_context, atoi(port_str.c_str()));
		io_context.run();
		if (grpc_server) {
			grpc_server->Shutdown();
		}
		if (grpc_thread.joinable()) {
			grpc_thread.join();
		}
	}
	catch (std::exception& e) {
		std::cerr << "Exception: " << e.what() << endl;
	}
}

