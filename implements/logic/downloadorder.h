//======================================================================
//
//        Copyright (C) 2014 回收宝
//        All rights reserved
//
//        filename : downloadorder.h
//        description : 订单信息下载
//
//        created by 叶飞 at  2014-08-01 10:00:00
//        modified by XX at  
//
//======================================================================

#ifndef _DOWNLOAD_ORDER_H_
#define _DOWNLOAD_ORDER_H_

#include <string>
#include <vector>
#include <json/json.h>
#include "order_table.h"

namespace logic
{

bool DownloadOrderList(const std::string& strUserID, const std::string& strUserKey, std::string& strFileName);

}

#endif


