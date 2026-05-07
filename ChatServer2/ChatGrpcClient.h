#pragma once

#include <string>

class ChatGrpcClient {
public:
    static bool NotifyAddFriend(const std::string& server_name, int apply_uid,
                                const std::string& name, const std::string& desc,
                                int to_uid);
    static bool RplyAddFriend(const std::string& server_name, int rply_uid,
                              bool agree, int to_uid);
    static bool NotifyTextChatMsg(const std::string& server_name, int from_uid,
                                  int to_uid, const std::string& message);

private:
    static std::string BuildServerAddress(const std::string& server_name);
};
