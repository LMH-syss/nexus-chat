#include "AsioIOServicePool.h"
#include <iostream>

using namespace std;

AsioIOServicePool::AsioIOServicePool(std::size_t size) :_ioServices(size), _works(size), _nextIOService(0)
{
	for (std::size_t i = 0; i < size; ++i) {//创建io_service对象
		_works[i] = std::make_unique<Work>(boost::asio::make_work_guard(_ioServices[i]));
	}

	for (std::size_t i = 0; i < _ioServices.size(); ++i) {//创建线程并运行io_service
		_threads.emplace_back([this, i]() {
			try {
				_ioServices[i].run();
			}
			catch (std::exception& e) {
				std::cerr << "Exception in IOService thread: " << e.what() << std::endl;
			}
			});
	}
}

AsioIOServicePool::~AsioIOServicePool()
{
	Stop();
	std::cout << "AsioIOServicePool destruct" << std::endl;
}

boost::asio::io_context& AsioIOServicePool::GetIOService()
{
	auto& service = _ioServices[_nextIOService++];
	if (_nextIOService == _ioServices.size()) {
		_nextIOService = 0;
	}
	return service;
}

void AsioIOServicePool::Stop() {
	for(auto & work : _works) {
		work->reset();
	}

	for(auto & t : _threads) {
		if (t.joinable()) {
			t.join();
		}
	}
}
