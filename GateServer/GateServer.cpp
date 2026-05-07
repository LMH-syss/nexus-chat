#include <iostream>
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>
#include "CServer.h"
#include "ConfigMgr.h"
#include "hiredis.h"
#include <cassert>
#include "RedisMgr.h"
#include "MysqlMgr.h"
int main()
{
	bool ok = MysqlMgr::GetInstance()->TestConnectionAndSchema();
	std::cout << "mysql test result = " << ok << std::endl;
	auto& gCfgMgr =ConfigMgr::Inst();
	std::string gate_port_str = gCfgMgr["GateServer"]["Port"];

	//int uid = MysqlMgr::GetInstance()->RegUser("cpp_test_user", "cpp_test_user@qq.com", "123456");
	//std::cout << "reg user uid = " << uid << std::endl;

	unsigned short gate_port = atoi(gate_port_str.c_str());
	try
	{
		unsigned short port = static_cast<unsigned short>(8080);
		net::io_context ioc{ 1 };
		boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
		signals.async_wait([&ioc](const boost::system::error_code& error, int signal_number) {

			if (error) {
				return;
			}
			ioc.stop();
			});
		std::make_shared<CServer>(ioc, port)->Start();
		std::cout << "Gate Serber listen on port: " << port << std::endl;
		ioc.run();
	}
	catch (std::exception const& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}