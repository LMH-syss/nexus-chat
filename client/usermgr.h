#ifndef USERMGR_H
#define USERMGR_H

#include "singleton.h"
#include <QString>

struct UserInfo
{
    int uid = 0;
    QString name;
    QString email;
    QString token;
};

class UserMgr : public Singleton<UserMgr>
{
    friend class Singleton<UserMgr>;

public:
    void SetUserInfo(const UserInfo& user_info);
    UserInfo GetUserInfo() const;

private:
    UserMgr() = default;

private:
    UserInfo _user_info;
};

#endif // USERMGR_H
