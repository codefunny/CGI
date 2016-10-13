//======================================================================
//
//        Copyright (C) 2014 回收宝
//        All rights reserved
//
//        filename : channel_table.h
//        description : 渠道表
//
//        created by 叶飞 at  2014-08-01 10:00:00
//        modified by XX at  
//
//======================================================================

#ifndef _CHANNEL_TABLE_H_
#define _CHANNEL_TABLE_H_

#include <string>
#include <map>

namespace logic
{

bool SelectChannelTable(unsigned int iChannelID, std::map<std::string, std::string>& channelInfo);
bool ChannelTable_UserBegins2ChannelID(const std::string& strUserBegins, unsigned int& iChannelID);

}

#endif


