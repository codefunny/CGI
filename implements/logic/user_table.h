//======================================================================
//
//        Copyright (C) 2014 回收宝
//        All rights reserved
//
//        filename : user_table.h
//        description : 用户表
//
//        created by 叶飞 at  2014-08-01 10:00:00
//        modified by 张世柏 at 2014-10-20 17:00:00
//
//======================================================================

#ifndef _USER_TABLE_H_
#define _USER_TABLE_H_

#include <string>
#include <vector>
#include <map>

namespace logic
{

bool SelectUserTable(unsigned int iUserID, std::map<std::string, std::string>& userInfo);
bool OrderID2OrganizationID(const std::vector<unsigned int>& vecOrderIDList, std::vector<unsigned int>& vecOrganizationID);
//modified by 张世柏 at 2014-10-20
bool CheckAndSelectUserByUserName(const std::string& strUserName, std::map<std::string, std::string>& mapUserInfo, bool& isOldUser);
unsigned int InsertUserTable(const std::string& strOpenID, std::map<std::string, std::string>& mapUserInfo);
bool SelectUserNameByUserID(unsigned int iUserID, std::string& strUserName);
bool UpdateUserTableAccountListAndPhoneNum(unsigned int iUserID, const std::string& strAccountList, const std::string& strPhoneNum);
//end modify

}

#endif


