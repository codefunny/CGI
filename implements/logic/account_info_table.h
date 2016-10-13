//======================================================================
//
//        Copyright (C) 2014 回收宝
//        All rights reserved
//
//        filename : account_info_table.h
//        description : 账户信息表
//
//        created by 张世柏 at  2014-10-29 12:30:00
//        modified by XX at
//
//======================================================================
#ifndef ACCOUNT_INFO_TABLE_H
#define ACCOUNT_INFO_TABLE_H

#include <map>
#include <string>

namespace logic
{

bool InsertAccountInfoTable(std::map<std::string, std::string>& mapAccountInfo, unsigned int& iAccountID);
bool SelectAccountInfo(unsigned int iAccountID, std::map<std::string, std::string>& mapAccountInfo);

}

#endif /*ACCOUNT_INFO_TABLE_H*/