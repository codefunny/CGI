//======================================================================
//
//        Copyright (C) 2014 回收宝
//        All rights reserved
//
//        filename : logistics_table.h
//        description : 物流表
//
//        created by 叶飞 at  2014-08-01 10:00:00
//        modified by XX at  
//
//======================================================================

#ifndef _LOGISTICS_TABLE_H_
#define _LOGISTICS_TABLE_H_

#include <string>

namespace logic
{

bool InsertLogisticsTable(unsigned int iChannelType, const std::string& strChannelID, unsigned int& iLogisticsID);

}

#endif


