//======================================================================
//
//        Copyright (C) 2014 回收宝
//        All rights reserved
//
//        filename : order.h
//        description : 订单接口
//
//        created by 叶飞 at  2014-08-01 10:00:00
//        modified by XX at  
//
//======================================================================

#ifndef _ORDER_H_
#define _ORDER_H_

#include <string>
#include <vector>
#include <json/json.h>
#include "order_table.h"

namespace logic
{

unsigned int PlaceOrder(const std::string& strUserID, const std::string& strUserKey, unsigned int iGoodsID, unsigned int iChannelID, unsigned int iPayWays, unsigned int iTradeWays, const std::string& strIMEI, const std::string& strRemarks, unsigned int& iOrderID);
bool GetOrderList(const std::string& strUserID, const std::string& strUserKey, const logic::CGetOrderFilter& oFilter, Json::Value& jsonOutput);
unsigned int SendGoods(const std::string& strUserID, const std::string& strUserKey, unsigned int iLogisticsType, const std::string& strLogisticsID, const std::vector<unsigned int>& vecOrderIDList, const logic::CSendGoodsInfo& oSendGoodsInfo, std::vector<unsigned int>& vecOrderIDSentList);
unsigned int GetinGoods(const std::string& strUserID, const std::string& strUserKey, const std::vector<unsigned int>& vecOrderIDList, std::vector<unsigned int>& vecOrderIDReceivedList);
unsigned int CancelGoodsBeforeSend(const std::string& strUserID, const std::string& strUserKey, const std::vector<unsigned int>& vecOrderIDList, std::vector<unsigned int>& vecOrderIDCanceledList);
unsigned int CancelGoodsAfterGetin(const std::string& strUserID, const std::string& strUserKey, const std::vector<unsigned int>& vecOrderIDList, std::vector<unsigned int>& vecOrderIDCanceledList);

}

#endif


