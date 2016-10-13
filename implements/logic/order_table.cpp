#include "order_table.h"
#include <stdlib.h>
#include <sstream>
#include "configure.h"
#include "db.h"
#include "goods_table.h"
#include "string_tools.h"
#include "category_table.h"
#include "organization_table.h"
#include "price_character_table.h"

using namespace std;

static bool GetCondition(const logic::CGetOrderFilter& oFilter, string& strConditionWithLimit, string& strConditionForTotal)
{
    stringstream ssCondition;

    if (0 != oFilter.iMid)
    {
        vector<unsigned int> vecCategoryIDList = logic::SelectSubCategoryID(oFilter.iMid);
        size_t maxSize = vecCategoryIDList.size();
        for (size_t index = 0; index < maxSize; index++)
        {
            if (0 == index)
            {
                ssCondition << " AND (t_order.Fcategory_id = " << vecCategoryIDList[index];
            }
            else
            {
                ssCondition << " OR t_order.Fcategory_id = " << vecCategoryIDList[index];
            }
        }
        if (!vecCategoryIDList.empty())
        {
            ssCondition << ")";
        }
    }

    if (0 != oFilter.iStartTime)
    {
        ssCondition << " AND UNIX_TIMESTAMP(t_order.Forder_time) >= " << oFilter.iStartTime;
    }

    if (0 != oFilter.iEndTime)
    {
        ssCondition << " AND UNIX_TIMESTAMP(t_order.Forder_time) <= " << oFilter.iEndTime;
    }

    if (0 != oFilter.iOrderStatus)
    {
        ssCondition << " AND t_order.Forder_status = " << oFilter.iOrderStatus;
    }

    strConditionForTotal = ssCondition.str();
    ssCondition << " ORDER BY t_order.Forder_time DESC";
    if (0 != oFilter.iPagesize && 0 != oFilter.iPagesize)
    {
        ssCondition << " LIMIT " << oFilter.iPagesize * oFilter.iPageindex << ", " << oFilter.iPagesize;
    }

    strConditionWithLimit = ssCondition.str();

    return true;
}

/**
  *modified by 张世柏 at 2014-10-24 14:25:00
  *brief: select goods name by orderid
  *param: iOrderID: order id
  *param: strGoodsName: goods name
  *return: return true when query Ok, otherwise return false
  */
bool logic::GetGoodsNameAndQuotationByOrderID(unsigned int iOrderID, string& strGoodsName, int& iQuotation)
{
    DEBUG("======= GetGoodsNameByOrderID BEGIN ======");
    db::Mysql oMysql;
    stringstream ssSql;
    ssSql << "SELECT Fproduct_name, Fquotation FROM t_order "
          << "INNER JOIN t_goods ON t_goods.Fgoods_id = t_order.Fgoods_id "
          << "WHERE t_order.Forder_id = " << iOrderID;

    if (0 != oMysql.Query(ssSql.str()))
    {
        ERROR("There are some wrong with sql [" + ssSql.str() + "]!");
        return false;
    }
    else
    {
        DEBUG("Sql query OK!");
    }

    if(0 == oMysql.GetRecordCount())
    {
        ERROR("There is 0 results!");
        return false;
    }
    else
    {
        DEBUG("Gets the results!");
    }

    strGoodsName = oMysql.GetField(0, "Fproduct_name");
    iQuotation = ::atoi(oMysql.GetField(0, "Fquotation").c_str());
    DEBUG("Order: [" + tools::CStringTools::Int2String(iOrderID) + "], Fproduct_name: [" + strGoodsName + "], \
                                                                       Fquotation: [" + tools::CStringTools::Int2String(iQuotation) + "].");

    DEBUG("========== GetGoodsNameByOrderID END ==========");
    return true;
}

/**
  *@created by Zhang Shibo at 2014-10-29 16:40:00
  *@brief:update AccountList value in order table
  *@param:iOrderID: t_order key
  *@param:iAccountType: Account_id value
  *return:return true when success, else, return false
  */
bool logic::UpdateOrderTableAccountID(unsigned int iOrderID, unsigned int iAccountType)
{
    DEBUG("======= UpdateOrderTableAccountID START =======");
    db::Mysql oMysql;
    stringstream ssSql;
    ssSql << "UPDATE t_order"
          << " SET Faccount_id = " << iAccountType
          << " WHERE Forder_id = " << iOrderID;
    DEBUG("Sql is [" + ssSql.str() + "]");
    if (0 != oMysql.Execute(ssSql.str()))
    {
        ERROR("Sql is execute wrong. Sql is: [" + ssSql.str() +"]");
        return false;
    }
    if (1 != oMysql.GetAffectedRows())
    {
        ERROR("The numbers of affected rows isn't 1, and it should be 1.\nSql is: [" + ssSql.str() + "]");
        return false;
    }

    DEBUG("======= UpdateOrderTableAccountID END =======");
    return true;
}

bool logic::SelectOrderTable(unsigned int iUserID, const vector<unsigned int>& vecOrganizationID, const logic::CGetOrderFilter& oFilter, vector< map<string, string> >& vecResult, unsigned int& iTotal)
{
    DEBUG("=======SelectOrderTableByOrganizationID START=======");

    string strConditionWithLimit, strConditionForTotal;
    if (!GetCondition(oFilter, strConditionWithLimit, strConditionForTotal))
    {
        ERROR("GetCondition failed");
        return false;
    }
    //modified by Zhang Shibo at 2014-10-31 10:32:00
    stringstream ssSql;
    ssSql << "SELECT *, t_order.Fbackup AS t_order_Fbackup, UNIX_TIMESTAMP(Forder_time) FROM t_order"
          << " INNER JOIN t_user ON t_order.Fuser_id = t_user.Fuser_id"
          << " INNER JOIN t_goods on t_order.Fgoods_id = t_goods.Fgoods_id"
          << " LEFT JOIN t_logistics ON t_order.Flogistics_id = t_logistics.Flogistics_id"
          << " WHERE (t_user.Fuser_id = " << iUserID;
    size_t maxSize = vecOrganizationID.size();
    for (size_t index = 0; index < maxSize; index++)
    {
        ssSql << " OR t_user.Forganization_id = " << vecOrganizationID[index];
    }
    ssSql << ")";
    if ("" != oFilter.strTag)
    {
        ssSql << " AND (t_order.FIMEI LIKE '%" << oFilter.strTag << "%'"
              << " OR t_order.Fsender_phone LIKE '%" << oFilter.strTag << "%'"
              << " OR t_order.Fbackup LIKE '%" << oFilter.strTag << "%'"
              << " OR t_logistics.Fchannel_id LIKE '%" << oFilter.strTag << "%'"
              << ")";
    }
    //end modify
    ssSql << strConditionWithLimit;
    db::Mysql oMysql;
    DEBUG("sql is : " + ssSql.str());
    if (0 != oMysql.Query(ssSql.str()))
    {
        ERROR("failed to query for sql : " +  ssSql.str());
        return false;
    }

    for (unsigned int iIndex = 0; iIndex < oMysql.GetRecordCount(); iIndex++)
    {
        map<string, string> mapOrderItem;
        mapOrderItem.insert(make_pair<string, string>("Forder_id", oMysql.GetField(iIndex, "Forder_id")));
        mapOrderItem.insert(make_pair<string, string>("Fgoods_id", oMysql.GetField(iIndex, "Fgoods_id")));
        mapOrderItem.insert(make_pair<string, string>("Fproduct_id", oMysql.GetField(iIndex, "Fproduct_id")));
        mapOrderItem.insert(make_pair<string, string>("Fproduct_name", oMysql.GetField(iIndex, "Fproduct_name")));
        mapOrderItem.insert(make_pair<string, string>("Fcategory_id", oMysql.GetField(iIndex, "Fcategory_id")));
        mapOrderItem.insert(make_pair<string, string>("Fuser_id", oMysql.GetField(iIndex, "Fuser_id")));
        mapOrderItem.insert(make_pair<string, string>("Fsender", oMysql.GetField(iIndex, "Fsender")));
        mapOrderItem.insert(make_pair<string, string>("Flogistics_id", oMysql.GetField(iIndex, "Flogistics_id")));
        mapOrderItem.insert(make_pair<string, string>("Fpay_ways", oMysql.GetField(iIndex, "Fpay_ways")));
        mapOrderItem.insert(make_pair<string, string>("Ftrade_ways", oMysql.GetField(iIndex, "Ftrade_ways")));
        mapOrderItem.insert(make_pair<string, string>("FIMEI", oMysql.GetField(iIndex, "FIMEI")));
        mapOrderItem.insert(make_pair<string, string>("t_order_Fbackup", oMysql.GetField(iIndex, "t_order_Fbackup")));
        mapOrderItem.insert(make_pair<string, string>("Forder_status", oMysql.GetField(iIndex, "Forder_status")));
        //modified by Zhang Shibo at 2014-10-30 21:25:00
        mapOrderItem.insert(make_pair<string, string>("Faccount_id", oMysql.GetField(iIndex, "Faccount_id")));
        //end modify
        //modified by Zhang Shibo at 2014-11-6 12:15:00
        mapOrderItem.insert(make_pair<string, string>("Fbackup", oMysql.GetField(iIndex, "Fbackup")));
        //end modify
        
       //modified by shuting at 2014-11-13 15:18:00
        mapOrderItem.insert(make_pair<string, string>("Fchannel_id", oMysql.GetField(iIndex, "Fchannel_id")));
       //end modied by shuting	

        //时间戳
        mapOrderItem.insert(make_pair<string, string>("Forder_time", oMysql.GetField(iIndex, "Forder_time")));
        mapOrderItem.insert(make_pair<string, string>("UNIX_TIMESTAMP(Forder_time)", oMysql.GetField(iIndex, "UNIX_TIMESTAMP(Forder_time)")));

        //用户表
        mapOrderItem.insert(make_pair<string, string>("Fuser_name", oMysql.GetField(iIndex, "Fuser_name")));

        //商品表
        mapOrderItem.insert(make_pair<string, string>("Fquotation", oMysql.GetField(iIndex, "Fquotation")));

        string strCharacterNameList;
        if (!logic::GetPriceCharacterName(tools::CStringTools::Split2Uint(oMysql.GetField(iIndex, "Fprice_input_result"), "#"), strCharacterNameList))
        {
            ERROR("failed to GetPriceCharacterName");
            continue;
        }
        mapOrderItem.insert(make_pair<string, string>("Fprice_input_result", oMysql.GetField(iIndex, "Fprice_input_result")));
        mapOrderItem.insert(make_pair<string, string>("Fprice_input_result_for_name", strCharacterNameList));

        //机构表
        map<string, string> organizationInfo;
        if (!logic::SelectOrganizationTable(::atoi(oMysql.GetField(iIndex, "Forganization_id").c_str()), organizationInfo))
        {
            ERROR("failed to SelectOrganizationTable");
            continue;
        }
        if (organizationInfo.end() == organizationInfo.find("Forganization_name"))
        {
            ERROR("Forganization_name not found");
            continue;
        }
        mapOrderItem.insert(make_pair<string, string>("Forganization_name", organizationInfo["Forganization_name"]));
        if (organizationInfo.end() == organizationInfo.find("Forganization_desc"))
        {
            ERROR("Forganization_desc not found");
            continue;
        }
        mapOrderItem.insert(make_pair<string, string>("Forganization_desc", organizationInfo["Forganization_desc"]));

        tools::CStringTools::Show(mapOrderItem);

        vecResult.push_back(mapOrderItem);
    }

    ssSql.str("");
    ssSql << "SELECT COUNT(*) FROM t_order"
          << " INNER JOIN t_user ON t_order.Fuser_id = t_user.Fuser_id"
          << " INNER JOIN t_goods on t_order.Fgoods_id = t_goods.Fgoods_id"
          << " WHERE (t_user.Fuser_id = " << iUserID;
    maxSize = vecOrganizationID.size();
    for (size_t index = 0; index < maxSize; index++)
    {
        ssSql << " OR t_user.Forganization_id = " << vecOrganizationID[index];
    }
    ssSql << ")";
    ssSql << strConditionForTotal;
    DEBUG("sql is : " + ssSql.str());
    if (0 != oMysql.Query(ssSql.str()))
    {
        ERROR("failed to query for sql : " + ssSql.str());
        return false;
    }
    if (1 != oMysql.GetRecordCount())
    {
        ERROR("DB has error for sql : " + ssSql.str());
        return false;
    }
    iTotal = ::atoi(oMysql.GetField(0, "COUNT(*)").c_str());

    DEBUG("========SelectOrderTableByOrganizationID END========");
    return true;
}

bool logic::InsertOrderTable(const string& strUserID, unsigned int iGoodsID, unsigned int iPayWays, unsigned int iTradeWays, const string& strIMEI, const string& strRemarks, unsigned int iProductID, const string& strProductName, unsigned int iCategoryID, unsigned int& iOrderID)
{
    DEBUG("=======InsertOrderTable START=======");

    db::Mysql oMysql;
    if (strUserID != oMysql.EscapeString(strUserID) || strIMEI != oMysql.EscapeString(strIMEI))
    {
        ERROR("illegal request");
        return false;
    }

    stringstream ssSql;
    ssSql << "INSERT INTO t_order SET"
          << " Fgoods_id = " << iGoodsID
          << ", Fproduct_id = '" << iProductID << "'"
          << ", Fproduct_name = '" << oMysql.EscapeString(strProductName) << "'"
          << ", Fcategory_id = '" << iCategoryID << "'"
          << ", Fuser_id = '" << oMysql.EscapeString(strUserID) << "'"
          << ", Fpay_ways = " << iPayWays
          << ", Ftrade_ways = " << iTradeWays
          << ", FIMEI = '" << oMysql.EscapeString(strIMEI) << "'"
          << ", Fbackup = '" << oMysql.EscapeString(strRemarks) << "'"
          << ", Forder_status = " << ORDER_STATUS_PLACED_ORDER
          << ", Fupdater = '" << oMysql.EscapeString(strUserID) << "'"
          << ", Forder_time = NOW()"
          << ", Fupdate_time = NOW()"
          << ", Fupdate_count = 1";
    DEBUG("sql is : " + ssSql.str());
    if (0 != oMysql.Execute(ssSql.str()))
    {
        ERROR("failed to execute for sql : " + ssSql.str());
        return false;
    }
    if (1 != oMysql.GetAffectedRows())
    {
        ERROR("failed to execute for sql : " + ssSql.str());
        return false;
    }

    stringstream ssLastInsertID;
    ssLastInsertID << "SELECT LAST_INSERT_ID()";
    DEBUG("sql is : " + ssLastInsertID.str());
    if (0 != oMysql.Query(ssLastInsertID.str()))
    {
        ERROR("failed to query for sql : " +  ssLastInsertID.str());
        return false;
    }
    if (1 != oMysql.GetRecordCount())
    {
        ERROR("DB has error for sql : " + ssLastInsertID.str());
        return false;
    }
    DEBUG("Forder_id is : " + oMysql.GetField(0, 0));
    iOrderID = ::atoi(oMysql.GetField(0, 0).c_str());

    DEBUG("========InsertOrderTable END========");
    return true;
}

/*
UpdateOrderTableAccountID()
{
Faccount_id = oMysql.EscapeString();
}

*/

bool logic::SendGoodsByOrderTable(const vector<unsigned int>& vecOrderIDList, unsigned int iLogisticsID, unsigned int iSender, const logic::CSendGoodsInfo& oSendGoodsInfo, vector<unsigned int>& vecOrderIDSentList)
{
    size_t maxSize = vecOrderIDList.size();
    for (size_t index = 0; index < maxSize; index++)
    {
        if (SendGoodsByOrderTable(vecOrderIDList[index], iLogisticsID, iSender, oSendGoodsInfo))
        {
            vecOrderIDSentList.push_back(vecOrderIDList[index]);
        }
        else
        {
            ERROR("UpdateOrderTable failed");
            continue;
        }
    }
    return true;
}

bool logic::SendGoodsByOrderTable(unsigned int iOrderID, unsigned int iLogisticsID, unsigned int iSender, const logic::CSendGoodsInfo& oSendGoodsInfo)
{
    DEBUG("=======SendGoodsByOrderTable START=======");

    db::Mysql oMysql;
    stringstream ssSql;
    ssSql << "UPDATE t_order SET"
          << " Fsender = " << iSender
          << ", Flogistics_id = " << iLogisticsID
          << ", Fsender_phone = '" << oMysql.EscapeString(oSendGoodsInfo.strSenderPhone) << "'"
          << ", Forder_status = " << ORDER_STATUS_SENDING
          << ", Fsend_time = NOW()"
          << ", Fupdater = " << iSender
          << ", Fupdate_time = NOW()"
          << ", Fupdate_count = Fupdate_count + 1"
          << " WHERE Forder_id = " << iOrderID
          << " AND Forder_status = " << ORDER_STATUS_PLACED_ORDER;
    DEBUG("sql is : " + ssSql.str());
    if (0 != oMysql.Execute(ssSql.str()))
    {
        ERROR("failed to query for sql : " +  ssSql.str());
        return false;
    }
    if (1 != oMysql.GetAffectedRows())
    {
        ERROR("failed to execute for sql : " + ssSql.str());
        return false;
    }

    DEBUG("========SendGoodsByOrderTable END========");
    return true;
}

bool logic::GetinGoodsByOrderTable(const vector<unsigned int>& vecOrderIDList, unsigned int iReceiver, vector<unsigned int>& vecOrderIDReceivedList)
{
    size_t maxSize = vecOrderIDList.size();
    for (size_t index = 0; index < maxSize; index++)
    {
        if (GetinGoodsByOrderTable(vecOrderIDList[index], iReceiver))
        {
            vecOrderIDReceivedList.push_back(vecOrderIDList[index]);
        }
        else
        {
            ERROR("GetinGoodsByOrderTable failed");
            continue;
        }
    }
    return true;
}

bool logic::GetinGoodsByOrderTable(unsigned int iOrderID, unsigned int iReceiver)
{
    DEBUG("=======GetinGoodsByOrderTable START=======");

    db::Mysql oMysql;
    stringstream ssSql;
    ssSql << "UPDATE t_order SET"
          << " Freceiver = " << iReceiver
          << ", Forder_status = " << ORDER_STATUS_PRE_CHECK
          << ", Fgetin_time = NOW()"
          << ", Fupdater = " << iReceiver
          << ", Fupdate_time = NOW()"
          << ", Fupdate_count = Fupdate_count + 1"
          << " WHERE Forder_id = " << iOrderID
          << " AND (Forder_status = " << ORDER_STATUS_SENDING << " OR Forder_status = " << ORDER_STATUS_GOT_IN << ")";
    DEBUG("sql is : " + ssSql.str());
    if (0 != oMysql.Execute(ssSql.str()))
    {
        ERROR("failed to query for sql : " +  ssSql.str());
        return false;
    }
    if (1 != oMysql.GetAffectedRows())
    {
        ERROR("failed to execute for sql : " + ssSql.str());
        return false;
    }

    DEBUG("========GetinGoodsByOrderTable END========");
    return true;
}

bool logic::CancelGoodsByOrderTableBeforeSend(const vector<unsigned int>& vecOrderIDList, unsigned int iCanceler, vector<unsigned int>& vecOrderIDReceivedList)
{
    bool bResult = true;
    size_t maxSize = vecOrderIDList.size();
    for (size_t index = 0; index < maxSize; index++)
    {
        if (CancelGoodsByOrderTableBeforeSend(vecOrderIDList[index], iCanceler))
        {
            vecOrderIDReceivedList.push_back(vecOrderIDList[index]);
        }
        else
        {
            ERROR("CancelGoodsByOrderTable failed");
            bResult = false;
            continue;
        }
    }
    return bResult;
}

bool logic::CancelGoodsByOrderTableBeforeSend(unsigned int iOrderID, unsigned int iCanceler)
{
    DEBUG("=======CancelGoodsByOrderTableBeforeSend START=======");

    db::Mysql oMysql;
    stringstream ssSql;
    ssSql << "UPDATE t_order SET"
          << " Forder_status = " << ORDER_STATUS_CANCELED
          << ", Fupdater = " << iCanceler
          << ", Fupdate_time = NOW()"
          << ", Fupdate_count = Fupdate_count + 1"
          << " WHERE Forder_id = " << iOrderID
          << " AND Forder_status = " << ORDER_STATUS_PLACED_ORDER;
    DEBUG("sql is : " + ssSql.str());
    if (0 != oMysql.Execute(ssSql.str()))
    {
        ERROR("failed to query for sql : " +  ssSql.str());
        return false;
    }
    if (1 != oMysql.GetAffectedRows())
    {
        ERROR("failed to execute for sql : " + ssSql.str());
        return false;
    }

    DEBUG("========CancelGoodsByOrderTableBeforeSend END========");
    return true;
}

bool logic::CancelGoodsByOrderTableAfterGetin(const vector<unsigned int>& vecOrderIDList, unsigned int iCanceler, vector<unsigned int>& vecOrderIDReceivedList)
{
    bool bResult = true;
    size_t maxSize = vecOrderIDList.size();
    for (size_t index = 0; index < maxSize; index++)
    {
        if (CancelGoodsByOrderTableAfterGetin(vecOrderIDList[index], iCanceler))
        {
            vecOrderIDReceivedList.push_back(vecOrderIDList[index]);
        }
        else
        {
            ERROR("CancelGoodsByOrderTable failed");
            bResult = false;
            continue;
        }
    }
    return bResult;
}

bool logic::CancelGoodsByOrderTableAfterGetin(unsigned int iOrderID, unsigned int iCanceler)
{
    DEBUG("=======CancelGoodsByOrderTableAfterGetin START=======");

    db::Mysql oMysql;
    stringstream ssSql;
    ssSql << "UPDATE t_order SET"
          << " Forder_status = " << ORDER_STATUS_CANCELED
          << ", Fupdater = " << iCanceler
          << ", Fupdate_time = NOW()"
          << ", Fupdate_count = Fupdate_count + 1"
          << " WHERE Forder_id = " << iOrderID
          << " AND (Forder_status = " << ORDER_STATUS_SENDING << " OR Forder_status = " << ORDER_STATUS_GOT_IN << ")";
    DEBUG("sql is : " + ssSql.str());
    if (0 != oMysql.Execute(ssSql.str()))
    {
        ERROR("failed to query for sql : " +  ssSql.str());
        return false;
    }
    if (1 != oMysql.GetAffectedRows())
    {
        ERROR("failed to execute for sql : " + ssSql.str());
        return false;
    }

    DEBUG("========CancelGoodsByOrderTableAfterGetin END========");
    return true;
}


