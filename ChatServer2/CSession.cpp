#include "CSession.h"
#include "CServer.h"
#include "LogicSystem.h"
#include <boost/asio/error.hpp>
#include <iostream>

CSession::CSession(boost::asio::io_context& io_context, CServer* server)
	: _socket(io_context), _server(server), _data{}, _user_uid(0) {
	boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
	_session_id = boost::uuids::to_string(a_uuid);
	_recv_head_node = make_shared<MsgNode>(HEAD_TOTAL_LEN);
}

CSession::~CSession()
{
}

void CSession::AsyncReadBody(int total_len)
{
	auto self = shared_from_this();

	asyncReadFull(total_len,
		[self, this, total_len](const boost::system::error_code& ec, std::size_t bytes_transfered) {
			try {
				if (ec) {
					if (ec == boost::asio::error::eof ||
						ec == boost::asio::error::operation_aborted ||
						ec == boost::asio::error::connection_reset) {
						std::cout << "session disconnected, session_id is " << _session_id << std::endl;
					}
					else {
						std::cout << "handle read body failed, error is " << ec.message() << std::endl;
					}
					Close();
					_server->ClearSession(_session_id);
					return;
				}

				if (bytes_transfered < total_len) {
					std::cout << "read body length not match, read ["
						<< bytes_transfered << "], total ["
						<< total_len << "]" << endl;
					Close();
					_server->ClearSession(_session_id);
					return;
				}

				// 
				memcpy(_recv_msg_node->_data, _data, bytes_transfered);
				_recv_msg_node->_cur_len += bytes_transfered;
				_recv_msg_node->_data[_recv_msg_node->_total_len] = '\0';

				std::cout << "receive data is " << _recv_msg_node->_data << std::endl;

				// 把完整消息交给逻辑层处理
				LogicSystem::GetInstance()->PostMsgToQue(
					make_shared<LogicNode>(shared_from_this(), _recv_msg_node)
				);

				// 
				AsyncReadHead(HEAD_TOTAL_LEN);
			}
			catch (std::exception& e) {
				std::cout << "Exception in AsyncReadBody: " << e.what() << std::endl;
			}
		});
}

void CSession::AsyncReadHead(int total_len)
{
	auto self = shared_from_this();
	asyncReadFull(HEAD_TOTAL_LEN, [self, this](const boost::system::error_code& ec, std::size_t bytes_transfered) {
		try {
			if (ec) {
				if (ec == boost::asio::error::eof ||
					ec == boost::asio::error::operation_aborted ||
					ec == boost::asio::error::connection_reset) {
					std::cout << "session disconnected, session_id is " << _session_id << std::endl;
				}
				else {
					std::cout << "handle read failed, error is " << ec.message() << endl;
				}
				Close();
				_server->ClearSession(_session_id);
				return;
			}

			if (bytes_transfered < HEAD_TOTAL_LEN) {
				std::cout << "read length not match, read [" << bytes_transfered << "] , total ["
					<< HEAD_TOTAL_LEN << "]" << endl;
				Close();
				_server->ClearSession(_session_id);
				return;
			}

			_recv_head_node->Clear();
			memcpy(_recv_head_node->_data, _data, bytes_transfered);

		
			short msg_id = 0;
			memcpy(&msg_id, _recv_head_node->_data, HEAD_ID_LEN);
			// 将消息 id 从网络字节序转换为主机字节序
			msg_id = boost::asio::detail::socket_ops::network_to_host_short(msg_id);
			std::cout << "msg_id is " << msg_id << endl;

			if (msg_id > MAX_LENGTH) {
				std::cout << "invalid msg_id is " << msg_id << endl;
				_server->ClearSession(_session_id);
				return;
			}
			short msg_len = 0;
			memcpy(&msg_len, _recv_head_node->_data + HEAD_ID_LEN, HEAD_DATA_LEN);

			msg_len = boost::asio::detail::socket_ops::network_to_host_short(msg_len);
			std::cout << "msg_len is " << msg_len << endl;

			
			if (msg_len > MAX_LENGTH) {
				std::cout << "invalid data length is " << msg_len << endl;
				_server->ClearSession(_session_id);
				return;
			}

			_recv_msg_node = make_shared<RecvNode>(msg_len, msg_id);
			AsyncReadBody(msg_len);
		}
		catch (std::exception& e) {
			std::cout << "Exception code is " << e.what() << endl;
		}
		});
}


void CSession::Start()
{
	//开始读取这个连接上的数据
	AsyncReadHead(HEAD_TOTAL_LEN);
}

void CSession::SetUserId(int uid)
{
	_user_uid = uid;
}

int CSession::GetUserId()
{
	return _user_uid;
}

tcp::socket& CSession::GetSocket()
{
	return _socket;
}

std::string& CSession::GetSessionId()
{
	return _session_id;
}

void CSession::asyncReadFull(
	std::size_t maxLength,
	std::function<void(const boost::system::error_code&, std::size_t)> handler)
{
	::memset(_data, 0, MAX_LENGTH);
	asyncReadLen(0, maxLength, handler);
}

void CSession::asyncReadLen(
	std::size_t read_len,
	std::size_t total_len,
	std::function<void(const boost::system::error_code&, std::size_t)> handler)
{
	auto self = shared_from_this();

	_socket.async_read_some(
		boost::asio::buffer(_data + read_len, total_len - read_len),
		[self, read_len, total_len, handler](const boost::system::error_code& ec, std::size_t bytesTransfered) {
			if (ec) {
				handler(ec, read_len + bytesTransfered);
				return;
			}

			if (read_len + bytesTransfered >= total_len) {
				handler(ec, read_len + bytesTransfered);
				return;
			}

			// 
			self->asyncReadLen(read_len + bytesTransfered, total_len, handler);
		});
}

LogicNode::LogicNode(shared_ptr<CSession> session, shared_ptr<RecvNode> recvnode)
	: _session(session), _recvnode(recvnode)
{
}

void CSession::Send(std::string msg, short msgid)
{
	short msg_len = static_cast<short>(msg.size());
	auto send_node = make_shared<SendNode>(msg.c_str(), msg_len, msgid);

	bool start_write = false;
	{
		std::lock_guard<std::mutex> lock(_send_lock);
		_send_que.push(send_node);
		if (!_send_pending) {
			_send_pending = true;
			start_write = true;
		}
	}

	if (start_write) {
		auto self = shared_from_this();
		boost::asio::post(_socket.get_executor(), [self]() {
			self->asyncWriteQueued();
		});
	}
}

void CSession::asyncWriteQueued()
{
	std::shared_ptr<SendNode> send_node;
	{
		std::lock_guard<std::mutex> lock(_send_lock);
		if (_send_que.empty()) {
			_send_pending = false;
			return;
		}
		send_node = _send_que.front();
	}

	auto self = shared_from_this();
	boost::asio::async_write(
		_socket,
		boost::asio::buffer(send_node->_data, send_node->_total_len),
		[self, send_node](const boost::system::error_code& ec, std::size_t bytes_transferred) {
			if (ec) {
				cout << "send failed, error is " << ec.what() << endl;
				std::lock_guard<std::mutex> lock(self->_send_lock);
				self->_send_pending = false;
				while (!self->_send_que.empty()) {
					self->_send_que.pop();
				}
				return;
			}

			{
				std::lock_guard<std::mutex> lock(self->_send_lock);
				if (!self->_send_que.empty()) {
					self->_send_que.pop();
				}
			}
			self->asyncWriteQueued();
		});
}

void CSession::UpdateHeartbeat() {
	_last_heartbeat = std::time(nullptr);
}

void CSession::Close() {
	boost::system::error_code ec;
	_socket.close(ec);
}
