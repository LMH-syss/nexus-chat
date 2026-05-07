#include "usermgr.h"

void UserMgr::SetUserInfo(const UserInfo& user_info)
{
    _user_info = user_info;
}

UserInfo UserMgr::GetUserInfo() const
{
    return _user_info;
}
