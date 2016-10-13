//======================================================================
//
//        Copyright (C) 2014 回收宝
//        All rights reserved
//
//        filename : order_table.h
//        description : 订单表
//
//        created by 叶飞 at  2014-08-01 10:00:00
//        modified by XX at  
//
//======================================================================

#ifndef _ORDER_TABLE_H_
#define _ORDER_TABLE_H_

#include <string>
#include <vector>
#include <map>

namespace logic
{

class CGetOrderFilter
{
public:
    CGetOrderFilter() : iMid(0), iStartTime(0), iEndTime(0), iOrderStatus(0), iPagesize(0), iPageindex(0){}
    std::string strTag;
    unsigned int iMid;
    unsigned int iStartTime;
    unsigned int iEndTime;
    unsigned int iOrderStatus;
    unsigned int iPagesize;
    unsigned int iPageindex;
};

class CSendGoodsInfo
{
public:
    CSendGoodsInfo(){}
    std::string strSenderPhone;
};

bool SelectOrderTable(unsigned int iUserID, const std::vector<unsigned int>& vecOrganizationID, const logic::CGetOrderFilter& oFilter, std::vector< std::map<std::string, std::string> >& vecResult, unsigned int& iTotal);
//modified by 张世柏 at 2014-10-23 21:38:00
bool GetGoodsNameAndQuotationByOrderID(unsigned int iOrderID, std::string& strGoodsName, int& iQuotation);
bool UpdateOrderTableAccountID(unsigned int iOrderID, unsigned int iAccountType);
//end modify

bool InsertOrderTable(const std::string& strUserID, unsigned int iGoodsID, unsigned int iPayWays, unsigned int iTradeWays, const std::string& strIMEI, const std::string& strRemarks, unsigned int iProductID, const std::string& strProductName, unsigned int iCategoryID, unsigned int& iOrderID);

bool SendGoodsByOrderTable(const std::vector<unsigned int>& vecOrderIDList, unsigned int iLogisticsID, unsigned int iSender, const logic::CSendGoodsInfo& oSendGoodsInfo, std::vector<unsigned int>& vecOrderIDSentList);
bool SendGoodsByOrderTable(unsigned int iOrderID, unsigned int iLogisticsID, unsigned int iSender, const logic::CSendGoodsInfo& oSendGoodsInfo);

bool GetinGoodsByOrderTable(const std::vector<unsigned int>& vecOrderIDList, unsigned int iReceiver, std::vector<unsigned int>& vecOrderIDReceivedList);
bool GetinGoodsByOrderTable(unsigned int iOrderID, unsigned int iReceiver);

bool CancelGoodsByOrderTableBeforeSend(const std::vector<unsigned int>& vecOrderIDList, unsigned int iCanceler, std::vector<unsigned int>& vecOrderIDReceivedList);
bool CancelGoodsByOrderTableBeforeSend(unsigned int iOrderID, unsigned int iCanceler);

bool CancelGoodsByOrderTableAfterGetin(const std::vector<unsigned int>& vecOrderIDList, unsigned int iCanceler, std::vector<unsigned int>& vecOrderIDReceivedList);
bool CancelGoodsByOrderTableAfterGetin(unsigned int iOrderID, unsigned int iCanceler);

}

#endif


