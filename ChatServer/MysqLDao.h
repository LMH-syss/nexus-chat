#pragma once
#include <thread>
#include <mutex>
#include <queue>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <string>
#include <vector>
#include <jdbc/mysql_driver.h>
#include <jdbc/mysql_connection.h>
#include <jdbc/cppconn/prepared_statement.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/statement.h>
#include <jdbc/cppconn/exception.h>
#include "const.h"

class SqlConnection {
public:
	SqlConnection(sql::Connection* con, int64_t lasttime) :_con(con), _last_oper_time(lasttime) {}
	std::unique_ptr<sql::Connection> _con;
	int64_t _last_oper_time;
};

class MySqlPool {
public:
	MySqlPool(const std::string& url, const std::string& user, const std::string& pass, const std::string& schema, int poolSize)
		: url_(url), user_(user), pass_(pass), schema_(schema), poolSize_(poolSize), b_stop_(false), _fail_count(0) {
		try {
			for (int i = 0; i < poolSize_; ++i) {
				auto* con = createConnection();
				// 获取当前时间戳
				auto currentTime = std::chrono::system_clock::now().time_since_epoch();
				// 将时间戳转换为秒
				long long timestamp = std::chrono::duration_cast<std::chrono::seconds>(currentTime).count();
				pool_.push(std::make_unique<SqlConnection>(con, timestamp));
			}

			_check_thread = std::thread([this]() {
				while (!b_stop_) {
					checkConnectionPro();
					std::this_thread::sleep_for(std::chrono::seconds(60));
				}
			});

			_check_thread.detach();
		}
		catch (sql::SQLException& e) {
			// 处理初始化异常
			std::cout << "mysql pool init failed, error is " << e.what() << std::endl;
		}
	}

	void checkConnectionPro() {
		// 1) 读取本轮需要检查的连接数量
		size_t targetCount;
		{
			std::lock_guard<std::mutex> guard(mutex_);
			targetCount = pool_.size();
		}

		// 2) 已经处理的连接数量
		size_t processed = 0;

		// 3) 当前时间戳
		auto now = std::chrono::system_clock::now().time_since_epoch();
		long long timestamp = std::chrono::duration_cast<std::chrono::seconds>(now).count();

		while (processed < targetCount) {
			std::unique_ptr<SqlConnection> con;
			{
				std::lock_guard<std::mutex> guard(mutex_);
				if (pool_.empty()) {
					break;
				}
				con = std::move(pool_.front());
				pool_.pop();
			}

			bool healthy = true;
			// 解锁后检查连接，必要时执行重连
			if (timestamp - con->_last_oper_time >= 5) {
				try {
					std::unique_ptr<sql::Statement> stmt(con->_con->createStatement());
					stmt->executeQuery("SELECT 1");
					con->_last_oper_time = timestamp;
				}
				catch (sql::SQLException& e) {
					std::cout << "Error keeping connection alive: " << e.what() << std::endl;
					healthy = false;
					_fail_count++;
				}

			}

			if (healthy)
			{
				std::lock_guard<std::mutex> guard(mutex_);
				pool_.push(std::move(con));
				cond_.notify_one();
			}

			++processed;
		}

			// 修复点：避免死锁，重连逻辑需要在未持有 mutex_ 的情况下执行
		while (_fail_count > 0) {
			auto b_res = reconnect(timestamp);
			if (b_res) {
				_fail_count--;
			}
			else {
				break;
			}
		}
	}

	bool reconnect(long long timestamp) {
		try {

			auto* con = createConnection();

			auto newCon = std::make_unique<SqlConnection>(con, timestamp);
			{
				std::lock_guard<std::mutex> guard(mutex_);
				pool_.push(std::move(newCon));
			}

			std::cout << "mysql connection reconnect success" << std::endl;
			return true;

		}
		catch (sql::SQLException& e) {
			std::cout << "Reconnect failed, error is " << e.what() << std::endl;
			return false;
		}
	}

	void checkConnection() {
		std::lock_guard<std::mutex> guard(mutex_);
		int poolsize = pool_.size();
		// 鑾峰彇褰撳墠鏃堕棿鎴?
		auto currentTime = std::chrono::system_clock::now().time_since_epoch();
		// 灏嗘椂闂存埑杞崲涓虹
		long long timestamp = std::chrono::duration_cast<std::chrono::seconds>(currentTime).count();
		for (int i = 0; i < poolsize; i++) {
			auto con = std::move(pool_.front());
			pool_.pop();
			Defer defer([this, &con]() {
				pool_.push(std::move(con));
				});

			if (timestamp - con->_last_oper_time < 5) {
				continue;
			}

			try {
				std::unique_ptr<sql::Statement> stmt(con->_con->createStatement());
				stmt->executeQuery("SELECT 1");
				con->_last_oper_time = timestamp;
			}
			catch (sql::SQLException& e) {
				std::cout << "Error keeping connection alive: " << e.what() << std::endl;
				// 重新创建连接并替换旧连接
				sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
				auto* newcon = driver->connect(url_, user_, pass_);
				newcon->setSchema(schema_);
				con->_con.reset(newcon);
				con->_last_oper_time = timestamp;
			}
		}
	}

	std::unique_ptr<SqlConnection> getConnection() {
		std::unique_lock<std::mutex> lock(mutex_);
		const auto ready = cond_.wait_for(lock, std::chrono::seconds(3), [this] {
			if (b_stop_) {
				return true;
			}
			return !pool_.empty(); });
		if (!ready || b_stop_) {
			std::cout << "mysql pool getConnection timeout or stopped" << std::endl;
			return nullptr;
		}
		std::unique_ptr<SqlConnection> con(std::move(pool_.front()));
		pool_.pop();
		return con;
	}

	void returnConnection(std::unique_ptr<SqlConnection> con) {
		if (!con) {
			return;
		}
		std::unique_lock<std::mutex> lock(mutex_);
		if (b_stop_) {
			return;
		}
		pool_.push(std::move(con));
		cond_.notify_one();
	}

	void Close() {
		b_stop_ = true;
		cond_.notify_all();
	}

	~MySqlPool() {
		Close();
		if (_check_thread.joinable()) {
			_check_thread.join();
		}

		std::lock_guard<std::mutex> guard(mutex_);
		while (!pool_.empty()) {
			pool_.pop();
		}
	}

private:
	sql::Connection* createConnection() {
		auto host = url_;
		int port = 3306;
		const auto pos = url_.find_last_of(':');
		if (pos != std::string::npos && pos + 1 < url_.size()) {
			host = url_.substr(0, pos);
			port = std::stoi(url_.substr(pos + 1));
		}

		sql::ConnectOptionsMap options;
		options[OPT_HOSTNAME] = host;
		options[OPT_PORT] = port;
		options[OPT_USERNAME] = user_;
		options[OPT_PASSWORD] = pass_;
		options[OPT_SCHEMA] = schema_;
		options[OPT_CONNECT_TIMEOUT] = 3;
		options[OPT_READ_TIMEOUT] = 3;
		options[OPT_WRITE_TIMEOUT] = 3;
		options[OPT_RECONNECT] = true;

		sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
		return driver->connect(options);
	}

	std::string url_;
	std::string user_;
	std::string pass_;
	std::string schema_;
	int poolSize_;
	std::queue<std::unique_ptr<SqlConnection>> pool_;
	std::mutex mutex_;
	std::condition_variable cond_;
	std::atomic<bool> b_stop_;
	std::thread _check_thread;
	std::atomic<int> _fail_count;
};

struct UserInfo {
	std::string name;
	std::string pwd;
	int uid;
	std::string email;
};

struct ApplyInfo {
	int from_uid;
	std::string from_name;
	int status;
	std::string apply_time;
};

struct ChatMessageInfo {
	long long msg_id = 0;
	int from_uid = 0;
	int to_uid = 0;
	std::string content;
	std::string send_time;
	int read_status = 0;
};

struct FriendChatInfo {
	UserInfo user;
	std::string last_msg;
	std::string last_time;
	int unread_count = 0;
};

class MysqlDao
{
public:
	MysqlDao();
	~MysqlDao();
	int RegUser(const std::string& name, const std::string& email, const std::string& pwd);
	int RegUserTransaction(const std::string& name, const std::string& email, const std::string& pwd, const std::string& icon);
	bool CheckEmail(const std::string& name, const std::string& email);
	bool UpdatePwd(const std::string& name, const std::string& newpwd);
	bool CheckPwd(const std::string& name, const std::string& pwd, UserInfo& userInfo);
	bool GetUserByUid(int uid, UserInfo& userInfo);
	bool SearchUserByKeyword(const std::string& keyword, std::vector<UserInfo>& results);
	bool InsertFriendApply(int from_uid, int to_uid);
	bool CheckPendingApply(int from_uid, int to_uid);
	bool GetApplyList(int to_uid, std::vector<ApplyInfo>& results);
	bool UpdateApplyStatus(int from_uid, int to_uid, int status);
	bool InsertFriend(int self_id, int friend_id);
	bool GetFriendList(int self_id, std::vector<UserInfo>& results);
	bool GetFriendListWithChatStatus(int self_id, std::vector<FriendChatInfo>& results);
	bool SaveChatMessage(int from_uid, int to_uid, const std::string& content,
		long long& msg_id, std::string& send_time);
	bool GetChatHistory(int self_uid, int peer_uid, int limit, long long before_msg_id,
		std::vector<ChatMessageInfo>& results);
	bool MarkChatMessagesRead(int self_uid, int peer_uid);
	bool TestProcedure(const std::string& email, int& uid, std::string& name);

	bool TestConnectionAndSchema();

private:
	std::unique_ptr<MySqlPool> pool_;
};
