//======================================================================
//
//        Copyright (C) 2014 回收宝
//        All rights reserved
//
//        filename : login.h
//        description : 登录接口
//
//        created by 叶飞 at  2014-08-01 10:00:00
//        modified by 张世柏 at 2014-10-20
//
//======================================================================

#ifndef _LOGIN_H_
#define _LOGIN_H_

#include <string>

namespace user
{

bool Login(unsigned int iUserChannel, const std::string& strUserName, const std::string& strUserPassword, std::string& strUserID, std::string& strUserKey);
bool ModifyPassword(unsigned int iUserChannel, const std::string& strUserName, const std::string& strOldPassword, const std::string& strNewPassword, std::string& strUserID, std::string& strUserKey);
unsigned int ChannelFuzzy(unsigned int iUserChannel, const std::string& strUserName);
//modified by 张世柏 at 2014-10-18
void SaveLandingState(const std::string& strUserID, const std::string& strUserKey);
//end modify
bool CheckLandingState(const std::string& strUserID, const std::string& strUserKey);
unsigned int CheckLandingStateWithChannel(unsigned int iUserID, const std::string& strUserKey, unsigned int iChannelID, unsigned int& iTrueChannelID, bool& bNeedLogin);

}

#endif

