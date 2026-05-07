#pragma once
#include <boost/beast/http.hpp>
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include "Singleton.h"
#include <functional>
#include <map>
#include <string>
#include <unordered_map>
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

class ConfigMgr;
extern ConfigMgr gCfgMgr;

#define CODEPREFIX  "code_"

enum ErrorCodes {//定义错误码
	Success = 0,
	Error_Json = 1001,//Json解析错误
	RPCFailed = 1002,//RPC请求错误
	VarifyExpired = 1003,//验证码过期
	VarifyCodeErr = 1004,//验证码错误
	UserExist = 1005,//用户已存在
	PasswdErr = 1006,//密码错误
	EmailNotMatch = 1007,//邮箱不匹配
	PasswdUpFailed = 1008,//密码更新失败
	PasswdInvalid = 1009,//密码不合法
	RPCGetFailed = 1010,
};

class Defer {
public:
	Defer(std::function<void()> func) : func_(func) {}

	~Defer() {
		func_();
	}
private:
	std::function<void()> func_;
};
