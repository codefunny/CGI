#include "order.h"
#include <stdlib.h>
#include <algorithm>
#include <map>
#include <sstream>
#include <json/json.h>
#include "configure.h"
#include "error_num.h"
#include "string_tools.h"
#include "time_tools.h"
#include "login.h"
#include "goods_table.h"
#include "order_table.h"
#include "user_table.h"
#include "organization_table.h"
#include "account_info_table.h"
#include "logistics_table.h"
#include "product_table.h"
#include "category_table.h"
#include "authority.h"
#include "group_table.h"

using namespace std;

bool GetAccountInfo(const string& strUserID, Json::Value& jsonPaywayData);

unsigned int logic::PlaceOrder(const string& strUserID, const string& strUserKey, unsigned int iGoodsID, unsigned int iChannelID, unsigned int iPayWays, unsigned int iTradeWays, const string& strIMEI, const string& strRemarks, unsigned int& iOrderID)
{
    //1.0、校验登录态
    if (!user::CheckLandingState(strUserID, strUserKey))
    {
        ERROR("need to login first");
        return error_num::USER_UNLOGIN;
    }
    //1.1、校验用户权限
    if (!logic::HasAuthority(::atoi(strUserID.c_str()), AUTHORITY_CREATE_ORDER))
    {
        ERROR("do NOT have authority");
        return error_num::AUTHORITY_NO;
    }



    //2.0、校验商品信息
    map<string, string> mapGoodsTable;
    if (!logic::SelectGoodsTable(iGoodsID, mapGoodsTable))
    {
        ERROR("SelectGoodsTable failed");
        return error_num::GOODS_UNKNOWN;
    }
    //2.1、校验卖家
    if (mapGoodsTable.end() == mapGoodsTable.find("Fseller"))
    {
        ERROR("Fseller is not found");
        return error_num::DB_GOODS;
    }
    DEBUG("Fseller is : " + mapGoodsTable["Fseller"]);
    if (0 != ::atoi(mapGoodsTable["Fseller"].c_str()))//已经有卖家了
    {
        if (mapGoodsTable["Fseller"] != strUserID)
        {
            ERROR("seller is not the same");
            return error_num::HOSTILE_ATTACK;
        }
    }

    //2.2、校验渠道
    if (mapGoodsTable.end() == mapGoodsTable.find("Fchannel_id"))
    {
        ERROR("Fchannel_id is not found");
        return error_num::DB_GOODS;
    }
    DEBUG("Fchannel_id is : " + mapGoodsTable["Fchannel_id"]);
    if (tools::CStringTools::Int2String(iChannelID) != mapGoodsTable["Fchannel_id"])
    {
        ERROR("channel ID is not the same");
        return error_num::HOSTILE_ATTACK;
    }



    //3.0、获取其他冗余信息
    //3.1、产品类目信息
    map<string, string> productInfo;
    if (mapGoodsTable.end() == mapGoodsTable.find("Fproduct_id"))
    {
        ERROR("Fproduct_id not found");
        return error_num::DB_GOODS;
    }
    if (!logic::SelectProductTable(::atoi(mapGoodsTable["Fproduct_id"].c_str()), productInfo))
    {
        ERROR("SelectProductTable failed");
        return error_num::DB_PRODUCT;
    }
    if (productInfo.end() == productInfo.find("Fcategory_id"))
    {
        ERROR("Fcategory_id not found");
        return error_num::DB_PRODUCT;
    }
    if (productInfo.end() == productInfo.find("Fproduct_name"))
    {
        ERROR("Fproduct_name not found");
        return error_num::DB_PRODUCT;
    }
    unsigned int iProductID = ::atoi(mapGoodsTable["Fproduct_id"].c_str());
    unsigned int iCategoryID = ::atoi(productInfo["Fcategory_id"].c_str());
    string strProductName = productInfo["Fproduct_name"];



    //4.0、插入订单表
    if (!logic::InsertOrderTable(strUserID, iGoodsID, iPayWays, iTradeWays, strIMEI, strRemarks, iProductID, strProductName, iCategoryID, iOrderID))
    {
        ERROR("create order failed");
        return error_num::ORDER_FAILED;
    }

    return error_num::SUCCEED;
}

/*
PlaceOrder()
{
    if (!PlaceOrder(iOrderID))
    {
        return;
    }

    if (账户信息有更改)
    {
        InsertAccountInfoTable(Faccount_id);
    }
    UpdateOrderTableAccountID(iOrderID, Faccount_id);

    if (账户信息有更改)
    {
        //insert memcache(strUserID, Faccount_id);//下次进入时getcache(strUserID, Faccount_id); SelectAccountInfoTable(Faccount_id);
        UpdateUserTable(Faccount_id)
    }
}

user_table.cpp 增加Faccount_id对应的信息读取，级联查找
*/

bool logic::GetOrderList(const string& strUserID, const string& strUserKey, const logic::CGetOrderFilter& oFilter, Json::Value& jsonOutput)
{
    //1.0、校验登录态
    if (!user::CheckLandingState(strUserID, strUserKey))
    {
        ERROR("need to login first");
        jsonOutput["ret"] = "1";
        jsonOutput["errcode"] = tools::CStringTools::Int2String(error_num::USER_UNLOGIN);
        jsonOutput["errstr"] = "用户未登录";
        return false;
    }

    map<string, string> mapUserTable;
    if (!logic::SelectUserTable(::atoi(strUserID.c_str()), mapUserTable))//用户ID转到机构ID
    {
        ERROR("SelectUserTable failed");
        jsonOutput["ret"] = "1";
        jsonOutput["errcode"] = tools::CStringTools::Int2String(error_num::USER_UNKNOWN);
        jsonOutput["errstr"] = "用户不存在";
        return false;
    }
    if (mapUserTable.end() == mapUserTable.find("Forganization_id"))
    {
        ERROR("Forganization_id not found");
        jsonOutput["ret"] = "1";
        jsonOutput["errcode"] = tools::CStringTools::Int2String(error_num::DB_USER);
        jsonOutput["errstr"] = "用户不存在";
        return false;
    }
    vector<unsigned int> vecOrganizationID = logic::SelectSubOrganizationID(::atoi(mapUserTable["Forganization_id"].c_str()));//旗下的全量机构ID

    vector< map<string, string> > vecOrderList;
    unsigned int iTotal;
    if (!logic::SelectOrderTable(::atoi(strUserID.c_str()), vecOrganizationID, oFilter, vecOrderList, iTotal))
    {
        ERROR("SelectOrderTableByOrganizationID failed");
        jsonOutput["ret"] = "1";
        jsonOutput["errcode"] = tools::CStringTools::Int2String(error_num::DB_ORDER);
        jsonOutput["errstr"] = "机构错误";
        return false;
    }

    Json::Value jsonData;

    Json::Value jsonOrderList;
    size_t maxSize = vecOrderList.size();
    for (size_t index = 0; index < maxSize; index++)
    {
        Json::Value jsonOrderItem;
        map<string, string> orderItem = vecOrderList[index];
        if (orderItem.end() == orderItem.find("Forder_id"))
        {
            ERROR("Forder_id not found");
            continue;
        }
        jsonOrderItem["oid"] = orderItem["Forder_id"];
        DEBUG("Order_id : " + orderItem["Forder_id"]);

        if (orderItem.end() == orderItem.find("Fproduct_name"))
        {
            ERROR("Fproduct_name not found");
            continue;
        }
        DEBUG("Fproduct_name : " + orderItem["Fproduct_name"]);
        jsonOrderItem["goods"] = orderItem["Fproduct_name"];

        if (orderItem.end() == orderItem.find("Fquotation"))
        {
            ERROR("Fquotation not found");
            continue;
        }
        jsonOrderItem["price"] = orderItem["Fquotation"];

        if (orderItem.end() == orderItem.find("Fpay_ways"))
        {
            ERROR("Fpay_ways not found");
            continue;
        }
        jsonOrderItem["payways"] = orderItem["Fpay_ways"];

        if (orderItem.end() == orderItem.find("Fuser_name"))
        {
            ERROR("Fuser_name not found");
            continue;
        }
        jsonOrderItem["userName"] = orderItem["Fuser_name"];

        if (orderItem.end() == orderItem.find("UNIX_TIMESTAMP(Forder_time)"))
        {
            ERROR("UNIX_TIMESTAMP(Forder_time) not found");
            continue;
        }
        jsonOrderItem["ts"] = orderItem["UNIX_TIMESTAMP(Forder_time)"];

        if (orderItem.end() == orderItem.find("Forder_status"))
        {
            ERROR("Forder_status not found");
            continue;
        }
        jsonOrderItem["status"] = orderItem["Forder_status"];
        jsonOrderItem["statusname"] = CGlobalData::GetInstance()->GetOrderName(::atoi(orderItem["Forder_status"].c_str()));

        if (orderItem.end() == orderItem.find("Fproduct_id"))
        {
            ERROR("Fproduct_id not found");
            continue;
        }
        map<string, string> productInfo;
        if (!logic::SelectProductTable(::atoi(orderItem["Fproduct_id"].c_str()), productInfo))
        {
            ERROR("SelectProductTable faield");
            continue;
        }
        if (productInfo.end() == productInfo.find("Fpic_id"))
        {
            ERROR("Fpic_id not found");
            continue;
        }
        jsonOrderItem["picurl"] = productInfo["Fpic_id"];

        if (orderItem.end() == orderItem.find("t_order_Fbackup"))
        {
            ERROR("t_order_Fbackup not found");
            continue;
        }
        jsonOrderItem["backup"] = orderItem["t_order_Fbackup"];

        if (orderItem.end() == orderItem.find("FIMEI"))
        {
            ERROR("FIMEI not found");
            continue;
        }
        jsonOrderItem["IMEI"] = orderItem["FIMEI"];

        //modified by Zhang Shibo at 2014-10-30 18:25:00
        if (orderItem.end() == orderItem.find("Fuser_id"))
        {
            ERROR("Fuser_id not found");
            continue;
        }
        map<string, string> mapUserInfo;
        unsigned int iUserID = ::atoi(orderItem["Fuser_id"].c_str());
        DEBUG("User ID : " + orderItem["Fuser_id"]);
        if (!logic::SelectUserTable(iUserID, mapUserInfo))
        {
            ERROR("Get user [" + orderItem["Fuser_id"] + "] info failed!");
            continue;
        }
        else
        {
            DEBUG("Get user [" + orderItem["Fuser_id"] + "] info successed!");
        }
        if (mapUserInfo.end() == mapUserInfo.find("Fuser_channel"))
        {
            ERROR("Fuser_channel not found");
            continue;
        }
        string strUserChannel = mapUserInfo["Fuser_channel"];
        DEBUG("User channel : " + strUserChannel);
        if (WECHAT_USER_CHANNEL != strUserChannel)
        {
            DEBUG("This user isn't wechat user.");
            jsonOrderList.append(jsonOrderItem);
            DEBUG("Find a new order [" + orderItem["Forder_id"] + "]");
            continue;
        }
        else
        {
            DEBUG("This user is a wechat user.");
        }

        Json::Value jsonPaywayData;
        if (orderItem.end() == orderItem.find("Faccount_id"))
        {
            ERROR("Faccount_id not found");
            continue;
        }
        DEBUG("Faccount_id : " + orderItem["Faccount_id"]);
        if (!GetAccountInfo(orderItem["Faccount_id"], jsonPaywayData))
        {
            ERROR("Get account_info[" + orderItem["Faccount_id"] + "] failed!");
            continue;
        }
        jsonOrderItem["payway"] = jsonPaywayData;
        //end modify

        jsonOrderList.append(jsonOrderItem);
        DEBUG("Find a new order [" + orderItem["Forder_id"] + "]");
    }
    jsonData["orderList"] = jsonOrderList;


    Json::Value jsonOrderHeader_goods;
    vector< map<string, string> > vecCategoryList;
    if (logic::SelectSubCategoryDepthForOne(CATEGORY_ROOT, vecCategoryList))
    {
        maxSize = vecCategoryList.size();
        for (size_t index = 0; index < maxSize; index++)
        {
            Json::Value jsonOrderHeader_goods_item;
            if (vecCategoryList[index].end() == vecCategoryList[index].find("Fcategory_id"))
            {
                continue;
            }
            if (vecCategoryList[index].end() == vecCategoryList[index].find("Fcategory_name"))
            {
                continue;
            }
            jsonOrderHeader_goods_item["mid"] = vecCategoryList[index]["Fcategory_id"];
            jsonOrderHeader_goods_item["name"] = vecCategoryList[index]["Fcategory_name"];
            jsonOrderHeader_goods.append(jsonOrderHeader_goods_item);
        }
    }

    Json::Value jsonPayways;
    Json::Value jsonPayways_item;
    jsonPayways_item["id"] = "1";
    jsonPayways_item["name"] = "折换成现金";
    jsonPayways.append(jsonPayways_item);

    Json::Value jsonOrderstatus;
    Json::Value jsonOrderstatus_item;
    map<unsigned int, string> allOrderStatus = CGlobalData::GetInstance()->GetAllOrderStatus();
    map<unsigned int, string>::iterator iter = allOrderStatus.begin();
    map<unsigned int, string>::const_iterator iterEnd = allOrderStatus.end();
    for (; iter != iterEnd; ++iter)
    {
        jsonOrderstatus_item["id"] = tools::CStringTools::Int2String(iter->first);
        jsonOrderstatus_item["name"] = iter->second;
        jsonOrderstatus.append(jsonOrderstatus_item);
    }

    Json::Value jsonOrderHeader;
    jsonOrderHeader["goods"] = jsonOrderHeader_goods;
    jsonOrderHeader["payways"] = jsonPayways;
    jsonOrderHeader["orderstatus"] = jsonOrderstatus;
    jsonData["orderheader"] = jsonOrderHeader;



    Json::Value jsonPermition;
    vector< map<string, string> > roleInfoList;
    if (!logic::SelectGroupTable(::atoi(strUserID.c_str()), roleInfoList))
    {
        ERROR("SelectGroupTable failed");
        jsonOutput["ret"] = "1";
        jsonOutput["errcode"] = tools::CStringTools::Int2String(error_num::DB_GROUP);
        jsonOutput["errstr"] = "用户错误";
        return false;
    }
    maxSize = roleInfoList.size();
    for (size_t index = 0; index < maxSize; index++)
    {
        if (roleInfoList[index].end() == roleInfoList[index].find("Frole_id"))
        {
            ERROR("Frole_id not found");
            continue;
        }
        if (roleInfoList[index].end() == roleInfoList[index].find("Frole_name"))
        {
            ERROR("Frole_name not found");
            continue;
        }
        Json::Value jsonPermitionItem;
        jsonPermitionItem["pid"] = roleInfoList[index]["Frole_id"];
        jsonPermitionItem["pname"] = roleInfoList[index]["Frole_name"];
        jsonPermition.append(jsonPermitionItem);
    }
    jsonData["permition"] = jsonPermition;



    jsonData["suspending"] = tools::CStringTools::Int2String(iTotal);//不准确，全量非待处理的



    jsonData["now"] = tools::CTimeTools::GetTime();



    Json::Value jsonPageinfo;
    jsonPageinfo["pagesize"] = tools::CStringTools::Int2String(oFilter.iPagesize);
    jsonPageinfo["pageindex"] = tools::CStringTools::Int2String(oFilter.iPageindex);
    jsonPageinfo["total"] = tools::CStringTools::Int2String(iTotal);
    jsonData["pageinfo"] = jsonPageinfo;

    jsonOutput["ret"] = "0";
    jsonOutput["errcode"] = "0";
    jsonOutput["errstr"] = "";
    jsonOutput["data"] = jsonData;

    return true;
}

unsigned int logic::SendGoods(const string& strUserID, const string& strUserKey, unsigned int iLogisticsType, const string& strLogisticsID, const vector<unsigned int>& vecOrderIDList, const logic::CSendGoodsInfo& oSendGoodsInfo, vector<unsigned int>& vecOrderIDSentList)
{
    //1.0、校验登录态
    if (!user::CheckLandingState(strUserID, strUserKey))
    {
        ERROR("need to login first");
        return error_num::USER_UNLOGIN;
    }
    //1.1、校验用户权限
    if (!logic::HasAuthority(::atoi(strUserID.c_str()), AUTHORITY_SEND_GOODS))
    {
        ERROR("do NOT have authority");
        return error_num::AUTHORITY_NO;
    }



    //2.0、检验这些订单表的用户是不是隶属于strUserID旗下
    map<string, string> mapUserTable;
    if (!logic::SelectUserTable(::atoi(strUserID.c_str()), mapUserTable))//用户ID转到机构ID
    {
        ERROR("SelectUserTable failed");
        return error_num::USER_UNKNOWN;
    }
    if (mapUserTable.end() == mapUserTable.find("Forganization_id"))
    {
        ERROR("invalid user");
        return error_num::DB_USER;
    }
    vector<unsigned int> vecAllOrganizationID = logic::SelectSubOrganizationID(::atoi(mapUserTable["Forganization_id"].c_str()));//旗下的全量机构ID
    vecAllOrganizationID.push_back(::atoi(mapUserTable["Forganization_id"].c_str()));//还要包括自己

    vector<unsigned int> vecSubOrganizationID;
    if (!logic::OrderID2OrganizationID(vecOrderIDList, vecSubOrganizationID))//订单所属机构
    {
        ERROR("OrderID2OrganizationID failed");
        return error_num::DB_ORGANIZATION;
    }

    //开始校验vecSubOrganizationID是不是全部都在vecAllOrganizationID里面
    if (vecSubOrganizationID.empty())
    {
        ERROR("invalid order id list");
        return error_num::DB_ORGANIZATION;
    }
    sort(vecSubOrganizationID.begin(), vecSubOrganizationID.end());
    vector<unsigned int>::iterator iter = unique(vecSubOrganizationID.begin(),vecSubOrganizationID.end());
    vecSubOrganizationID.erase(iter,vecSubOrganizationID.end());
    sort(vecAllOrganizationID.begin(), vecAllOrganizationID.end());
    vector<unsigned int> vecDifference(vecSubOrganizationID.size() + vecAllOrganizationID.size());
    iter = set_difference(vecSubOrganizationID.begin(), vecSubOrganizationID.end(), vecAllOrganizationID.begin(), vecAllOrganizationID.end(), vecDifference.begin());

    if (0)
    {
        DEBUG("vecSubOrganizationID :");tools::CStringTools::Show(vecSubOrganizationID);
        DEBUG("vecAllOrganizationID :");tools::CStringTools::Show(vecAllOrganizationID);
        DEBUG("vecDifference :");tools::CStringTools::Show(vecDifference);
    }

    if (iter != vecDifference.begin())
    {
        ERROR("no right to send goods");
        return error_num::USER_UNKNOWN;
    }



    //3.0、创建物流单
    unsigned int iLogisticsID = 0;
    if (!logic::InsertLogisticsTable(iLogisticsType, strLogisticsID, iLogisticsID))
    {
        ERROR("create logistics faield");
        return error_num::LOGISTICS_FAILED;
    }




    //4.0、更新多张订单表
    if (!logic::SendGoodsByOrderTable(vecOrderIDList, iLogisticsID, ::atoi(strUserID.c_str()), oSendGoodsInfo, vecOrderIDSentList))
    {
        ERROR("SendGoodsByOrderTable faield");
        return error_num::ORDER_FAILED;
    }

    return error_num::SUCCEED;
}

unsigned int logic::GetinGoods(const string& strUserID, const string& strUserKey, const vector<unsigned int>& vecOrderIDList, vector<unsigned int>& vecOrderIDReceivedList)
{
    if (!user::CheckLandingState(strUserID, strUserKey))
    {
        ERROR("need to login first");
        return error_num::USER_UNLOGIN;
    }

    //校验用户权限
    if (!logic::HasAuthority(::atoi(strUserID.c_str()), AUTHORITY_CONFIRM_GETIN))
    {
        ERROR("do NOT have authority");
        return error_num::AUTHORITY_NO;
    }

    if (!logic::GetinGoodsByOrderTable(vecOrderIDList, ::atoi(strUserID.c_str()), vecOrderIDReceivedList))
    {
        ERROR("GetinGoodsByOrderTable failed");
        return error_num::ORDER_FAILED;
    }
    return error_num::SUCCEED;
}

unsigned int logic::CancelGoodsBeforeSend(const string& strUserID, const string& strUserKey, const vector<unsigned int>& vecOrderIDList, vector<unsigned int>& vecOrderIDCanceledList)
{
    if (!user::CheckLandingState(strUserID, strUserKey))
    {
        ERROR("need to login first");
        return error_num::USER_UNLOGIN;
    }

    //校验用户权限
    if (!logic::HasAuthority(::atoi(strUserID.c_str()), AUTHORITY_CANCEL_BEFORE_SEND))
    {
        ERROR("do NOT have authority");
        return error_num::AUTHORITY_NO;
    }

    if (!logic::CancelGoodsByOrderTableBeforeSend(vecOrderIDList, ::atoi(strUserID.c_str()), vecOrderIDCanceledList))
    {
        ERROR("CancelGoodsByOrderTable failed");
        return error_num::ORDER_FAILED;
    }
    return error_num::SUCCEED;
}

unsigned int logic::CancelGoodsAfterGetin(const string& strUserID, const string& strUserKey, const vector<unsigned int>& vecOrderIDList, vector<unsigned int>& vecOrderIDCanceledList)
{
    if (!user::CheckLandingState(strUserID, strUserKey))
    {
        ERROR("need to login first");
        return error_num::USER_UNLOGIN;
    }

    //校验用户权限
    if (!logic::HasAuthority(::atoi(strUserID.c_str()), AUTHORITY_CANCEL_AFTER_GETIN))
    {
        ERROR("do NOT have authority");
        return error_num::AUTHORITY_NO;
    }

    if (!logic::CancelGoodsByOrderTableAfterGetin(vecOrderIDList, ::atoi(strUserID.c_str()), vecOrderIDCanceledList))
    {
        ERROR("CancelGoodsByOrderTable failed");
        return error_num::ORDER_FAILED;
    }
    return error_num::SUCCEED;
}

bool GetAccountInfo(const string& strAccountID, Json::Value& jsonPaywayData)
{
    DEBUG("========== GetAccountInfo BEGIN ========");
    Json::Value jsonAliPayData, jsonBankPayData;
    jsonAliPayData["payway_id"] = Json::Value("");
    jsonAliPayData["payway_type"] = Json::Value(tools::CStringTools::Int2String(ACCOUNT_ALIPAY));
    jsonAliPayData["account"] = Json::Value("");
    jsonBankPayData["payway_id"] = Json::Value("");
    jsonBankPayData["payway_type"] = Json::Value(tools::CStringTools::Int2String(ACCOUNT_ONLINEBANK));
    jsonBankPayData["name"] = Json::Value("");
    jsonBankPayData["bank"] = Json::Value("");
    jsonBankPayData["account"] = Json::Value("");
    map<string, string> mapAccountInfo;
    if (!logic::SelectAccountInfo(::atoi(strAccountID.c_str()), mapAccountInfo))
    {
        ERROR("Get account info [" + strAccountID + "] failed!");
        return false;
    }
    else
    {
        //读取账户信息成功
        DEBUG("Get account info [" + strAccountID + "] successed!");
    }
    unsigned int iPaywayType = ::atoi(mapAccountInfo["Faccount_type"].c_str());
    if (ACCOUNT_ONLINEBANK == iPaywayType)
    {
        //网银
        DEBUG("User using online bank.");
        jsonBankPayData["payway_id"] = Json::Value(strAccountID);
        jsonBankPayData["payway_type"] = Json::Value(mapAccountInfo["Faccount_type"]);
        jsonBankPayData["name"] = Json::Value(mapAccountInfo["Fwy_name"]);
        jsonBankPayData["bank"] = Json::Value( mapAccountInfo["Fwy_bank_name"]);
        jsonBankPayData["account"] = Json::Value(mapAccountInfo["Fwy_bank_cardid"]);
    }
    else if (ACCOUNT_ALIPAY == iPaywayType)
    {
        //支付宝
        DEBUG("User using alipay.");
        jsonAliPayData["payway_id"] = Json::Value(strAccountID);
        jsonAliPayData["payway_type"] = Json::Value(mapAccountInfo["Faccount_type"]);
        jsonAliPayData["account"] = Json::Value(mapAccountInfo["Fzfb_account"]);
    }
    else
    {
        ERROR("There is no such payway type.");
        return false;
    }
    jsonPaywayData["alipay"] = jsonAliPayData;
    jsonPaywayData["bank"] = jsonBankPayData;

    DEBUG("========== GetAccountInfo END ========");
    return true;
}
