#pragma once
#include <vector>
#include <boost/asio.hpp>
#include "Singleton.h"
#include <thread>
#include <memory>
class AsioIOServicePool :public Singleton<AsioIOServicePool>
{
	friend Singleton<AsioIOServicePool>;	
public:
	using IOService = boost::asio::io_context;
	using Work = boost::asio::executor_work_guard<IOService::executor_type>;
	using WorkPtr = std::unique_ptr<Work>;
	~AsioIOServicePool();
	boost::asio::io_context& GetIOService();
	void Stop();
private:
	AsioIOServicePool(std::size_t size = std::thread::hardware_concurrency());//默认线程数为cpu核心数
	std::vector<IOService> _ioServices;
	std::vector<WorkPtr> _works;
	std::vector<std::thread> _threads;//多少个线程就有多少个io_service
	size_t _nextIOService;//轮询分配io_service
};

