//======================================================================
//
//        Copyright (C) 2014 回收宝
//        All rights reserved
//
//        filename : wechat_login.h
//        description : 获取微信用户的OpenID和AccessToken
//
//        created by 张世柏 at  2014-10-12 10:00:00
//        modified by XX at
//
//======================================================================
#ifndef _WECHAT_LOGIN_H_
#define _WECHAT_LOGIN_H_
#include <string>

namespace wechat
{

//获取微信用户的OpenID和AccessToken
bool GetOpenIDAccessToken(const std::string& strCode, std::string& strOpenid, std::string& strAccessToken);

//判断用户是否是刷新进入页面
bool IsRefreshenedPage(const std::string& strCookie);

//写cookie
std::string WriteInCookie(const std::string& strParamName, const std::string& strParam);

}

#endif
