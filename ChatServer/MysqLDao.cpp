#include "MysqLDao.h"
#include "ConfigMgr.h"

#include <algorithm>
#include <cctype>
#include <limits>

namespace {
std::string TrimKeyword(const std::string& keyword) {
	const auto begin = keyword.find_first_not_of(" \t\r\n");
	if (begin == std::string::npos) {
		return "";
	}

	const auto end = keyword.find_last_not_of(" \t\r\n");
	return keyword.substr(begin, end - begin + 1);
}

bool TryParseUid(const std::string& keyword, int& uid) {
	if (keyword.empty()) {
		return false;
	}

	long long value = 0;
	for (unsigned char ch : keyword) {
		if (!std::isdigit(ch)) {
			return false;
		}

		value = value * 10 + (ch - '0');
		if (value > std::numeric_limits<int>::max()) {
			return false;
		}
	}

	uid = static_cast<int>(value);
	return true;
}
}

MysqlDao::MysqlDao()
{
	auto& cfg = ConfigMgr::Inst();
	const auto& host = cfg["Mysql"]["Host"];
	const auto& port = cfg["Mysql"]["Port"];
	const auto& pwd = cfg["Mysql"]["Passwd"];
	const auto& schema = cfg["Mysql"]["Schema"];
	const auto& user = cfg["Mysql"]["User"];
	pool_.reset(new MySqlPool(host + ":" + port, user, pwd, schema, 5));
}

MysqlDao::~MysqlDao() {
	pool_->Close();
}

int MysqlDao::RegUser(const std::string& name, const std::string& email, const std::string& pwd)
{
	auto con = pool_->getConnection();
	try {
		if (con == nullptr) {
			return false;
		}
		// 准备调用存储过程
		std::unique_ptr<sql::PreparedStatement> stmt(con->_con->prepareStatement("CALL reg_user(?,?,?,@result)"));
		// 设置输入参数
		stmt->setString(1, name);
		stmt->setString(2, email);
		stmt->setString(3, pwd);

		// 由于PreparedStatement不直接支持注册输出参数，我们需要使用会话变量或其他方法来获取输出参数的值

		// 执行存储过程
		stmt->execute();
		// 如果存储过程设置了会话变量或有其他方式获取输出参数的值，你可以在这里执行SELECT查询来获取它们
	    // 例如，如果存储过程设置了一个会话变量@result来存储输出结果，可以这样获取：
		std::unique_ptr<sql::Statement> stmtResult(con->_con->createStatement());
		std::unique_ptr<sql::ResultSet> res(stmtResult->executeQuery("SELECT @result AS result"));
		if (res->next()) {
			int result = res->getInt("result");
			std::cout << "Result: " << result << std::endl;
			pool_->returnConnection(std::move(con));
			return result;
		}
		pool_->returnConnection(std::move(con));
		return -1;
	}
	catch (sql::SQLException& e) {
		pool_->returnConnection(std::move(con));
		std::cerr << "SQLException: " << e.what();
		std::cerr << " (MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		return -1;
	}
}

int MysqlDao::RegUserTransaction(const std::string& name, const std::string& email, const std::string& pwd,
	const std::string& icon)
{
	auto con = pool_->getConnection();
	if (con == nullptr) {
		return false;
	}

	Defer defer([this, &con] {
		pool_->returnConnection(std::move(con));
		});

	try {
		//开始事务
		con->_con->setAutoCommit(false);
		//执行第一个数据库操作，根据email查找用户
			// 准备查询语句

		std::unique_ptr<sql::PreparedStatement> pstmt_email(con->_con->prepareStatement("SELECT 1 FROM user WHERE email = ?"));

		// 绑定参数
		pstmt_email->setString(1, email);

		// 执行查询
		std::unique_ptr<sql::ResultSet> res_email(pstmt_email->executeQuery());

		auto email_exist = res_email->next();
		if (email_exist) {
			con->_con->rollback();
			std::cout << "email " << email << " exist";
			return 0;
		}

		// 准备查询用户名是否重复
		std::unique_ptr<sql::PreparedStatement> pstmt_name(con->_con->prepareStatement("SELECT 1 FROM user WHERE name = ?"));

		// 绑定参数
		pstmt_name->setString(1, name);

		// 执行查询
		std::unique_ptr<sql::ResultSet> res_name(pstmt_name->executeQuery());

		auto name_exist = res_name->next();
		if (name_exist) {
			con->_con->rollback();
			std::cout << "name " << name << " exist";
			return 0;
		}

		// 准备更新用户id
		std::unique_ptr<sql::PreparedStatement> pstmt_upid(con->_con->prepareStatement("UPDATE user_id SET id = id + 1"));

		// 执行更新
		pstmt_upid->executeUpdate();

		// 获取更新后的 id 值
		std::unique_ptr<sql::PreparedStatement> pstmt_uid(con->_con->prepareStatement("SELECT id FROM user_id"));
		std::unique_ptr<sql::ResultSet> res_uid(pstmt_uid->executeQuery());
		int newId = 0;
		// 处理结果集
		if (res_uid->next()) {
			newId = res_uid->getInt("id");
		}
		else {
			std::cout << "select id from user_id failed" << std::endl;
			con->_con->rollback();
			return -1;
		}

		// 插入user信息
		std::unique_ptr<sql::PreparedStatement> pstmt_insert(con->_con->prepareStatement("INSERT INTO user (uid, name, email, pwd, nick, icon) "
			"VALUES (?, ?, ?, ?,?,?)"));
		pstmt_insert->setInt(1, newId);
		pstmt_insert->setString(2, name);
		pstmt_insert->setString(3, email);
		pstmt_insert->setString(4, pwd);
		pstmt_insert->setString(5, name);
		pstmt_insert->setString(6, icon);
		//执行插入
		pstmt_insert->executeUpdate();
		// 提交事务
		con->_con->commit();
		std::cout << "newuser insert into user success" << std::endl;
		return newId;
	}
	catch (sql::SQLException& e) {
		// 如果发生错误，回滚事务
		if (con) {
			con->_con->rollback();
		}
		std::cerr << "SQLException: " << e.what();
		std::cerr << " (MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		return -1;
	}
}

bool MysqlDao::CheckEmail(const std::string& name, const std::string& email) {
	auto con = pool_->getConnection();
	if (con == nullptr) {
		return false;
	}

	Defer defer([this, &con]() {
		pool_->returnConnection(std::move(con));
		});

	try {
		// 准备查询语句
		std::unique_ptr<sql::PreparedStatement> pstmt(con->_con->prepareStatement("SELECT email FROM user WHERE name = ?"));

		// 绑定参数
		pstmt->setString(1, name);

		// 执行查询
		std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

		// 遍历结果集
		while (res->next()) {
			std::cout << "Check Email: " << res->getString("email") << std::endl;
			if (email != res->getString("email")) {
				return false;
			}
			return true;
		}
		return false;
	}
	catch (sql::SQLException& e) {
		std::cerr << "SQLException: " << e.what();
		std::cerr << " (MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		return false;
	}
}

bool MysqlDao::UpdatePwd(const std::string& name, const std::string& newpwd) {
	auto con = pool_->getConnection();
	try {
		if (con == nullptr) {
			return false;
		}

		// 准备查询语句
		std::unique_ptr<sql::PreparedStatement> pstmt(con->_con->prepareStatement("UPDATE user SET pwd = ? WHERE name = ?"));

		// 绑定参数
		pstmt->setString(2, name);
		pstmt->setString(1, newpwd);

		// 执行更新
		int updateCount = pstmt->executeUpdate();

		std::cout << "Updated rows: " << updateCount << std::endl;
		pool_->returnConnection(std::move(con));
		return true;
	}
	catch (sql::SQLException& e) {
		pool_->returnConnection(std::move(con));
		std::cerr << "SQLException: " << e.what();
		std::cerr << " (MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		return false;
	}
}

bool MysqlDao::CheckPwd(const std::string& email, const std::string& pwd, UserInfo& userInfo) {
	auto con = pool_->getConnection();
	if (con == nullptr) {
		std::cout << "SearchUserByKeyword getConnection failed" << std::endl;
		return false;
	}
	std::cout << "SearchUserByKeyword getConnection success" << std::endl;

	Defer defer([this, &con]() {
		pool_->returnConnection(std::move(con));
		});

	try {
		// 准备SQL语句
		std::unique_ptr<sql::PreparedStatement> pstmt(con->_con->prepareStatement("SELECT * FROM user WHERE email = ?"));
		pstmt->setString(1, email); // 将username替换为你要查询的用户名

		// 执行查询
		std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
		std::string origin_pwd = "";
		// 遍历结果集
		while (res->next()) {
			origin_pwd = res->getString("pwd");
			// 输出查询到的密码
			std::cout << "Password: " << origin_pwd << std::endl;
			break;
		}

		if (pwd != origin_pwd) {
			return false;
		}
		userInfo.name = res->getString("name");
		userInfo.email = res->getString("email");
		userInfo.uid = res->getInt("uid");
		userInfo.pwd = origin_pwd;
		return true;
	}
	catch (sql::SQLException& e) {
		std::cerr << "SQLException: " << e.what();
		std::cerr << " (MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		return false;
	}
}

bool MysqlDao::GetUserByUid(int uid, UserInfo& userInfo) {
	auto con = pool_->getConnection();
	if (con == nullptr) {
		return false;
	}

	Defer defer([this, &con]() {
		pool_->returnConnection(std::move(con));
		});

	try {
		std::unique_ptr<sql::PreparedStatement> pstmt(con->_con->prepareStatement("SELECT uid, name, email, pwd FROM user WHERE uid = ?"));
		pstmt->setInt(1, uid);
		std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
		if (!res->next()) {
			return false;
		}

		userInfo.uid = res->getInt("uid");
		userInfo.name = res->getString("name");
		userInfo.email = res->getString("email");
		userInfo.pwd = res->getString("pwd");
		return true;
	}
	catch (sql::SQLException& e) {
		std::cerr << "SQLException: " << e.what();
		std::cerr << " (MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		return false;
	}
}
bool MysqlDao::SearchUserByKeyword(const std::string& keyword, std::vector<UserInfo>& results) {
	results.clear();
	const auto normalized_keyword = TrimKeyword(keyword);
	if (normalized_keyword.empty()) {
		return true;
	}

	std::cout << "SearchUserByKeyword getConnection begin" << std::endl;
	auto con = pool_->getConnection();
	if (con == nullptr) {
		std::cout << "SearchUserByKeyword getConnection failed" << std::endl;
		return false;
	}
	std::cout << "SearchUserByKeyword getConnection success" << std::endl;

	Defer defer([this, &con]() {
		pool_->returnConnection(std::move(con));
	});

	try {
		int search_uid = 0;
		const bool search_by_uid = TryParseUid(normalized_keyword, search_uid);
		std::unique_ptr<sql::PreparedStatement> pstmt;
		if (search_by_uid) {
			pstmt.reset(con->_con->prepareStatement(
				"SELECT uid, name, email FROM user "
				"WHERE uid = ? OR name = ? OR email = ? "
				"ORDER BY CASE WHEN uid = ? THEN 0 WHEN name = ? THEN 1 ELSE 2 END "
				"LIMIT 20"
			));
			pstmt->setInt(1, search_uid);
			pstmt->setString(2, normalized_keyword);
			pstmt->setString(3, normalized_keyword);
			pstmt->setInt(4, search_uid);
			pstmt->setString(5, normalized_keyword);
		}
		else {
			pstmt.reset(con->_con->prepareStatement(
				"SELECT uid, name, email FROM user "
				"WHERE name = ? OR email = ? "
				"ORDER BY CASE WHEN name = ? THEN 0 ELSE 1 END "
				"LIMIT 20"
			));
			pstmt->setString(1, normalized_keyword);
			pstmt->setString(2, normalized_keyword);
			pstmt->setString(3, normalized_keyword);
		}

		pstmt->setQueryTimeout(3);
		std::cout << "SearchUserByKeyword executeQuery begin" << std::endl;
		std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
		std::cout << "SearchUserByKeyword executeQuery done" << std::endl;
		while (res->next()) {
			UserInfo info;
			info.uid = res->getInt("uid");
			info.name = res->getString("name");
			info.email = res->getString("email");
			results.push_back(info);
		}
		std::cout << "SearchUserByKeyword result count: " << results.size() << std::endl;
		return true;
	}
	catch (sql::SQLException& e) {
		std::cerr << "SQLException: " << e.what();
		std::cerr << " (MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		return false;
	}
}

bool MysqlDao::InsertFriendApply(int from_uid, int to_uid) {
	auto con = pool_->getConnection();
	if (con == nullptr) {
		return false;
	}

	Defer defer([this, &con]() {
		pool_->returnConnection(std::move(con));
	});

	try {
		std::unique_ptr<sql::PreparedStatement> pstmt(
			con->_con->prepareStatement(
				"INSERT INTO friend_apply (from_uid, to_uid) VALUES (?, ?)"
			)
		);
		pstmt->setInt(1, from_uid);
		pstmt->setInt(2, to_uid);
		pstmt->executeUpdate();
		return true;
	}
	catch (sql::SQLException& e) {
		std::cerr << "SQLException: " << e.what() << std::endl;
		return false;
	}
}

bool MysqlDao::CheckPendingApply(int from_uid, int to_uid) {
	auto con = pool_->getConnection();
	if (con == nullptr) {
		return false;
	}

	Defer defer([this, &con]() {
		pool_->returnConnection(std::move(con));
	});

	try {
		std::unique_ptr<sql::PreparedStatement> pstmt(
			con->_con->prepareStatement(
				"SELECT COUNT(*) FROM friend_apply WHERE from_uid = ? AND to_uid = ? AND status = 0"
			)
		);
		pstmt->setInt(1, from_uid);
		pstmt->setInt(2, to_uid);
		std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
		if (res->next()) {
			return res->getInt(1) > 0;
		}
		return false;
	}
	catch (sql::SQLException& e) {
		std::cerr << "SQLException: " << e.what() << std::endl;
		return false;
	}
}

bool MysqlDao::GetApplyList(int to_uid, std::vector<ApplyInfo>& results) {
	auto con = pool_->getConnection();
	if (con == nullptr) {
		return false;
	}

	Defer defer([this, &con]() {
		pool_->returnConnection(std::move(con));
	});

	try {
		std::unique_ptr<sql::PreparedStatement> pstmt(
			con->_con->prepareStatement(
				"SELECT fa.from_uid, fa.status, fa.apply_time, u.name AS from_name "
				"FROM friend_apply fa "
				"JOIN user u ON fa.from_uid = u.uid "
				"WHERE fa.to_uid = ? AND fa.status = 0 "
				"ORDER BY fa.apply_time DESC"
			)
		);
		pstmt->setInt(1, to_uid);
		std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
		while (res->next()) {
			ApplyInfo info;
			info.from_uid = res->getInt("from_uid");
			info.from_name = res->getString("from_name");
			info.status = res->getInt("status");
			info.apply_time = res->getString("apply_time");
			results.push_back(info);
		}
		return true;
	}
	catch (sql::SQLException& e) {
		std::cerr << "SQLException: " << e.what() << std::endl;
		return false;
	}
}

bool MysqlDao::UpdateApplyStatus(int from_uid, int to_uid, int status) {
	auto con = pool_->getConnection();
	if (con == nullptr) {
		return false;
	}

	Defer defer([this, &con]() {
		pool_->returnConnection(std::move(con));
	});

	try {
		std::unique_ptr<sql::PreparedStatement> pstmt(
			con->_con->prepareStatement(
				"UPDATE friend_apply SET status = ? WHERE from_uid = ? AND to_uid = ?"
			)
		);
		pstmt->setInt(1, status);
		pstmt->setInt(2, from_uid);
		pstmt->setInt(3, to_uid);
		pstmt->executeUpdate();
		return true;
	}
	catch (sql::SQLException& e) {
		std::cerr << "SQLException: " << e.what() << std::endl;
		return false;
	}
}

bool MysqlDao::InsertFriend(int self_id, int friend_id) {
	auto con = pool_->getConnection();
	if (con == nullptr) {
		return false;
	}

	Defer defer([this, &con]() {
		pool_->returnConnection(std::move(con));
	});

	try {
		std::unique_ptr<sql::PreparedStatement> pstmt(
			con->_con->prepareStatement(
				"INSERT INTO friend (self_id, friend_id) VALUES (?, ?)"
			)
		);
		pstmt->setInt(1, self_id);
		pstmt->setInt(2, friend_id);
		pstmt->executeUpdate();
		return true;
	}
	catch (sql::SQLException& e) {
		std::cerr << "SQLException: " << e.what() << std::endl;
		return false;
	}
}

bool MysqlDao::GetFriendList(int self_id, std::vector<UserInfo>& results) {
	auto con = pool_->getConnection();
	if (con == nullptr) {
		return false;
	}

	Defer defer([this, &con]() {
		pool_->returnConnection(std::move(con));
	});

	try {
		std::unique_ptr<sql::PreparedStatement> pstmt(
			con->_con->prepareStatement(
				"SELECT u.uid, u.name, u.email "
				"FROM friend f "
				"JOIN user u ON f.friend_id = u.uid "
				"WHERE f.self_id = ?"
			)
		);
		pstmt->setInt(1, self_id);
		std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
		while (res->next()) {
			UserInfo info;
			info.uid = res->getInt("uid");
			info.name = res->getString("name");
			info.email = res->getString("email");
			results.push_back(info);
		}
		return true;
	}
	catch (sql::SQLException& e) {
		std::cerr << "SQLException: " << e.what() << std::endl;
		return false;
	}
}

bool MysqlDao::GetFriendListWithChatStatus(int self_id, std::vector<FriendChatInfo>& results) {
	results.clear();
	auto con = pool_->getConnection();
	if (con == nullptr) {
		return false;
	}

	Defer defer([this, &con]() {
		pool_->returnConnection(std::move(con));
	});

	try {
		std::unique_ptr<sql::PreparedStatement> pstmt(
			con->_con->prepareStatement(
				"SELECT u.uid, u.name, u.email, "
				"COALESCE((SELECT cm.content FROM chat_message cm "
				"WHERE ((cm.from_uid = ? AND cm.to_uid = u.uid) OR (cm.from_uid = u.uid AND cm.to_uid = ?)) "
				"ORDER BY cm.id DESC LIMIT 1), '') AS last_msg, "
				"COALESCE((SELECT DATE_FORMAT(cm.send_time, '%Y-%m-%d %H:%i:%s') FROM chat_message cm "
				"WHERE ((cm.from_uid = ? AND cm.to_uid = u.uid) OR (cm.from_uid = u.uid AND cm.to_uid = ?)) "
				"ORDER BY cm.id DESC LIMIT 1), '') AS last_time, "
				"(SELECT COUNT(*) FROM chat_message cm "
				"WHERE cm.from_uid = u.uid AND cm.to_uid = ? AND cm.read_status = 0) AS unread_count "
				"FROM friend f "
				"JOIN user u ON f.friend_id = u.uid "
				"WHERE f.self_id = ? "
				"ORDER BY last_time DESC, u.uid ASC"
			)
		);
		pstmt->setInt(1, self_id);
		pstmt->setInt(2, self_id);
		pstmt->setInt(3, self_id);
		pstmt->setInt(4, self_id);
		pstmt->setInt(5, self_id);
		pstmt->setInt(6, self_id);
		std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
		while (res->next()) {
			FriendChatInfo info;
			info.user.uid = res->getInt("uid");
			info.user.name = res->getString("name");
			info.user.email = res->getString("email");
			info.last_msg = res->getString("last_msg");
			info.last_time = res->getString("last_time");
			info.unread_count = res->getInt("unread_count");
			results.push_back(info);
		}
		return true;
	}
	catch (sql::SQLException& e) {
		std::cerr << "SQLException: " << e.what() << std::endl;
		return false;
	}
}

bool MysqlDao::SaveChatMessage(int from_uid, int to_uid, const std::string& content,
	long long& msg_id, std::string& send_time) {
	msg_id = 0;
	send_time.clear();

	auto con = pool_->getConnection();
	if (con == nullptr) {
		return false;
	}

	Defer defer([this, &con]() {
		pool_->returnConnection(std::move(con));
	});

	try {
		std::unique_ptr<sql::PreparedStatement> insert_stmt(
			con->_con->prepareStatement(
				"INSERT INTO chat_message (from_uid, to_uid, content, read_status) "
				"VALUES (?, ?, ?, 0)"
			)
		);
		insert_stmt->setInt(1, from_uid);
		insert_stmt->setInt(2, to_uid);
		insert_stmt->setString(3, content);
		insert_stmt->executeUpdate();

		std::unique_ptr<sql::Statement> query_stmt(con->_con->createStatement());
		std::unique_ptr<sql::ResultSet> res(query_stmt->executeQuery(
			"SELECT id, DATE_FORMAT(send_time, '%Y-%m-%d %H:%i:%s') AS send_time "
			"FROM chat_message WHERE id = LAST_INSERT_ID()"
		));
		if (!res->next()) {
			return false;
		}

		msg_id = res->getInt64("id");
		send_time = res->getString("send_time");
		return true;
	}
	catch (sql::SQLException& e) {
		std::cerr << "SQLException: " << e.what() << std::endl;
		return false;
	}
}

bool MysqlDao::GetChatHistory(int self_uid, int peer_uid, int limit, long long before_msg_id,
	std::vector<ChatMessageInfo>& results) {
	results.clear();
	if (limit <= 0 || limit > 100) {
		limit = 30;
	}

	auto con = pool_->getConnection();
	if (con == nullptr) {
		return false;
	}

	Defer defer([this, &con]() {
		pool_->returnConnection(std::move(con));
	});

	try {
		std::unique_ptr<sql::PreparedStatement> pstmt;
		if (before_msg_id > 0) {
			pstmt.reset(con->_con->prepareStatement(
				"SELECT id, from_uid, to_uid, content, DATE_FORMAT(send_time, '%Y-%m-%d %H:%i:%s') AS send_time, read_status "
				"FROM chat_message "
				"WHERE (((from_uid = ? AND to_uid = ?) OR (from_uid = ? AND to_uid = ?)) AND id < ?) "
				"ORDER BY id DESC LIMIT ?"
			));
			pstmt->setInt(1, self_uid);
			pstmt->setInt(2, peer_uid);
			pstmt->setInt(3, peer_uid);
			pstmt->setInt(4, self_uid);
			pstmt->setInt64(5, before_msg_id);
			pstmt->setInt(6, limit);
		}
		else {
			pstmt.reset(con->_con->prepareStatement(
				"SELECT id, from_uid, to_uid, content, DATE_FORMAT(send_time, '%Y-%m-%d %H:%i:%s') AS send_time, read_status "
				"FROM chat_message "
				"WHERE ((from_uid = ? AND to_uid = ?) OR (from_uid = ? AND to_uid = ?)) "
				"ORDER BY id DESC LIMIT ?"
			));
			pstmt->setInt(1, self_uid);
			pstmt->setInt(2, peer_uid);
			pstmt->setInt(3, peer_uid);
			pstmt->setInt(4, self_uid);
			pstmt->setInt(5, limit);
		}

		std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
		while (res->next()) {
			ChatMessageInfo info;
			info.msg_id = res->getInt64("id");
			info.from_uid = res->getInt("from_uid");
			info.to_uid = res->getInt("to_uid");
			info.content = res->getString("content");
			info.send_time = res->getString("send_time");
			info.read_status = res->getInt("read_status");
			results.push_back(info);
		}

		std::reverse(results.begin(), results.end());
		return true;
	}
	catch (sql::SQLException& e) {
		std::cerr << "SQLException: " << e.what() << std::endl;
		return false;
	}
}

bool MysqlDao::MarkChatMessagesRead(int self_uid, int peer_uid) {
	auto con = pool_->getConnection();
	if (con == nullptr) {
		return false;
	}

	Defer defer([this, &con]() {
		pool_->returnConnection(std::move(con));
	});

	try {
		std::unique_ptr<sql::PreparedStatement> pstmt(
			con->_con->prepareStatement(
				"UPDATE chat_message SET read_status = 1 "
				"WHERE from_uid = ? AND to_uid = ? AND read_status = 0"
			)
		);
		pstmt->setInt(1, peer_uid);
		pstmt->setInt(2, self_uid);
		pstmt->executeUpdate();
		return true;
	}
	catch (sql::SQLException& e) {
		std::cerr << "SQLException: " << e.what() << std::endl;
		return false;
	}
}

bool MysqlDao::TestProcedure(const std::string& email, int& uid, std::string& name) {
	auto con = pool_->getConnection();
	try {
		if (con == nullptr) {
			return false;
		}

		Defer defer([this, &con]() {
			pool_->returnConnection(std::move(con));
			});
		// 准备调用存储过程
		std::unique_ptr < sql::PreparedStatement > stmt(con->_con->prepareStatement("CALL test_procedure(?,@userId,@userName)"));
		// 设置输入参数
		stmt->setString(1, email);

		// 由于PreparedStatement不直接支持注册输出参数，我们需要使用会话变量或其他方法来获取输出参数的值

		  // 执行存储过程
		stmt->execute();
		// 如果存储过程设置了会话变量或有其他方式获取输出参数的值，你可以在这里执行SELECT查询来获取它们
	   // 例如，如果存储过程设置了一个会话变量@result来存储输出结果，可以这样获取：
		std::unique_ptr<sql::Statement> stmtResult(con->_con->createStatement());
		std::unique_ptr<sql::ResultSet> res(stmtResult->executeQuery("SELECT @userId AS uid"));
		if (!(res->next())) {
			return false;
		}

		uid = res->getInt("uid");
		std::cout << "uid: " << uid << std::endl;

		stmtResult.reset(con->_con->createStatement());
		res.reset(stmtResult->executeQuery("SELECT @userName AS name"));
		if (!(res->next())) {
			return false;
		}

		name = res->getString("name");
		std::cout << "name: " << name << std::endl;
		return true;

	}
	catch (sql::SQLException& e) {
		std::cerr << "SQLException: " << e.what();
		std::cerr << " (MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		return false;
	}
}


bool MysqlDao::TestConnectionAndSchema() {
	auto con = pool_->getConnection();
	if (!con) {
		std::cout << "getConnection failed" << std::endl;
		return false;
	}

	Defer defer([this, &con]() {
		pool_->returnConnection(std::move(con));
		});

	try {
		std::unique_ptr<sql::Statement> stmt(con->_con->createStatement());

		// 1. 测试能不能正常执行SQL
		std::unique_ptr<sql::ResultSet> res1(stmt->executeQuery("SELECT 1"));
		if (res1->next()) {
			std::cout << "SELECT 1 result = " << res1->getInt(1) << std::endl;
		}

		// 2. 测试当前数据库是不是 chat
		std::unique_ptr<sql::ResultSet> res2(stmt->executeQuery("SELECT DATABASE()"));
		if (res2->next()) {
			std::cout << "current database = " << res2->getString(1) << std::endl;
		}

		// 3. 测试 user 表在不在
		std::unique_ptr<sql::ResultSet> res3(stmt->executeQuery("SHOW TABLES LIKE 'user'"));
		if (res3->next()) {
			std::cout << "table user exists" << std::endl;
		}
		else {
			std::cout << "table user not found" << std::endl;
		}

		return true;
	}
	catch (sql::SQLException& e) {
		std::cout << "sql exception: " << e.what() << std::endl;
		return false;
	}
	catch (std::exception& e) {
		std::cout << "std exception: " << e.what() << std::endl;
		return false;
	}
}
