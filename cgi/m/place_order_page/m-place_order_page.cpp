//======================================================================
//
//        Copyright (C) 2014 回收宝
//        All rights reserved
//
//        filename : m-place_order_page.cpp
//        description : 微信用户下单的CGI入口
//
//        created by 张世柏 at  2014-10-17 18:00:00
//        modified by mickey at 2014-11-20 2:20:00
//
//======================================================================

#include <stdlib.h>
#include <string.h>
#include <string>
#include <map>
#include <sstream>
#include <fcgi_stdio.h>
#include <json/json.h>
#include "configure.h"
#include "error_num.h"
#include "string_tools.h"
#include "price.h"
#include "login.h"
#include "user_table.h"
#include "account_info_table.h"
#include "channel_table.h"
#include "price_character_table.h"
#include "product_table.h"
#include "goods_table.h"
#include "allowance_table.h"
#include "order.h"
#include "clearsilver.h"
#include "category_table.h"
#include "goods_table.h"

using namespace std;

class CRequest
{
public:
    string strGetData;
    string strMethod;
    string strScriptFileName;
    string strCookie;
    string strUserID;
    string strUserKey;
    string strChannel;
    string strPid;
    string strSelects;
    string strReferer;
    bool bFromSharedPage;
};

static bool ParseCgi(CRequest& oRequest)
{
    DEBUG("============== Begin to parse cgi ================");
    oRequest.bFromSharedPage = false;
    string strLocationUrl = "Location:" + WECHAT_LOCATION_FOR_CODE + "?appid=" + WECHAT_APPID + "&redirect_uri=" + WECHAT_LOCATION_HOME_PAGE + WECHAT_LOCATION_PARAM + "\r\n";
    if (getenv("QUERY_STRING"))
    {
        oRequest.strGetData = getenv("QUERY_STRING");
        oRequest.strGetData = tools::CStringTools::UrlDecode(oRequest.strGetData);
    }

    if (getenv("REQUEST_METHOD"))
    {
        oRequest.strMethod = getenv("REQUEST_METHOD");
    }

    if (getenv("SCRIPT_FILENAME"))
    {
        oRequest.strScriptFileName = getenv("SCRIPT_FILENAME");
    }

    if (getenv("HTTP_REFERER"))
    {
        oRequest.strReferer = getenv("HTTP_REFERER");
        DEBUG(oRequest.strReferer);
    }

    if (getenv("HTTP_COOKIE"))
    {
        oRequest.strCookie = getenv("HTTP_COOKIE");
        oRequest.strCookie = tools::CStringTools::Trim(oRequest.strCookie);
    }

    if (!oRequest.strCookie.empty())
    {
        map<string, string> mapRequest = tools::CStringTools::SplitTwice2String(oRequest.strCookie, ";", "=");
        if (mapRequest.end() != mapRequest.find("uid"))
        {
            oRequest.strUserID = mapRequest["uid"];
            DEBUG("uid : " + oRequest.strUserID);
        }
        if (mapRequest.end() != mapRequest.find("userkey"))
        {
            oRequest.strUserKey = mapRequest["userkey"];
            DEBUG("userkey : " + oRequest.strUserKey);
        }
        if (mapRequest.end() != mapRequest.find("channel"))
        {
            oRequest.strChannel = mapRequest["channel"];
            DEBUG("channel : " + oRequest.strChannel);
        }
    }

    if (!oRequest.strGetData.empty())
    {
        map<string, string> mapRequest = tools::CStringTools::SplitTwice2String(oRequest.strGetData, "&", "=");
        if (mapRequest.end() == mapRequest.find("pid"))
        {
            ERROR("invalid request");
            return false;
        }
        if (mapRequest.end() == mapRequest.find("selects"))
        {
            ERROR("invalid request");
            return false;
        }

        oRequest.strPid = mapRequest["pid"];
        oRequest.strSelects = mapRequest["selects"];
        if(("" == oRequest.strPid) || ("" == oRequest.strSelects))
        {
            ERROR("Product info is NULL");
            return false;
        }
        DEBUG("pid is : " + oRequest.strPid + ", selects is : " + oRequest.strSelects);
    }
    else
    {
        ERROR("There is no parameters after url.");
        return false;
    }


    //校验登陆态
    if (!user::CheckLandingState(oRequest.strUserID, oRequest.strUserKey))
    {
        ERROR("User has not login!");
        DEBUG_AND_RETURN(strLocationUrl.c_str());
        FCGI_printf("Content-type:text/html\r\n\r\n");
        oRequest.bFromSharedPage = true;
        return false;
    }

    if (oRequest.strReferer.empty())
    {
        ERROR("Http referer is NULL.");
        DEBUG_AND_RETURN(strLocationUrl.c_str());
        FCGI_printf("Content-type:text/html\r\n\r\n");
        oRequest.bFromSharedPage = true;
        return false;
    }
    else
    {
        //Judge whether from XXX.huishoubao.com.cn
        if (!tools::CStringTools::IsHostName(oRequest.strReferer))
        {
            //not match
            ERROR("Http referer is: " + oRequest.strReferer);
            DEBUG_AND_RETURN(strLocationUrl.c_str());
            FCGI_printf("Content-type:text/html\r\n\r\n");
            oRequest.bFromSharedPage = true;
            return false;
        }
        else
        {
            //match
            DEBUG("Lawful request. ");
        }
    }

    DEBUG("============== End to parse cgi ================");
    return true;
}

static void HandleCgi(const CRequest& oRequest)
{
    DEBUG("============== Begin to handle cgi ================");
    string strCGIName = "/placeorder_page";
    string strCsFile = "placeorder_page.cs";
    string strErrorCsFile = "error_page.cs";

    if (strCGIName != oRequest.strScriptFileName)
    {
        ERROR("invalid request");
        return;
    }

    string strResult;
    tools::CClearSilver oClearSilver;
    /// start modified by mickey at 2014-11-20 2:20:00
    //oClearSilver.SetValue("cdnbase", CDN_BASE);
    oClearSilver.SetValue("cdnbase", TENCENT_CDN_BASE);
    /// end modified
    oClearSilver.SetDirectory(CDN_M_DIRECTORY);
    do
    {
        //渲染模板
        DEBUG("Begin to render the template");

        //价格估算
        unsigned int iGoodsID = 0;
        unsigned int iQuotation = 0;
        bool bIsCreateGoodsID = true;
        unsigned int iQuotationResult = logic::GetQuotation(::atoi(oRequest.strPid.c_str()),::atoi(oRequest.strChannel.c_str()),oRequest.strUserID,
                                        oRequest.strUserKey,oRequest.strSelects,bIsCreateGoodsID,iGoodsID,iQuotation);
        if (error_num::SUCCEED == iQuotationResult)
        {
            //产品信息
            map<string, string> mapGoodsInfo;
            if (logic::SelectGoodsTable(iGoodsID, mapGoodsInfo))
            {
                if (mapGoodsInfo.end() == mapGoodsInfo.find("Fproduct_name"))
                {
                    ERROR("Fproduct_name is not found");
                    break;
                }
                if (mapGoodsInfo.end() == mapGoodsInfo.find("Fpic_id"))
                {
                    ERROR("Fpic_id is not found");
                    break;
                }
                oClearSilver.SetValue("quotation", iQuotation);
                oClearSilver.SetValue("goodsinfo.gid", iGoodsID);
                oClearSilver.SetValue("goodsinfo.productname", mapGoodsInfo["Fproduct_name"]);
                oClearSilver.SetValue("goodsinfo.picurl", mapGoodsInfo["Fpic_id"]);

                do
                {
                    //第一步：获取用户账户ID列表
                    map<string, string> mapUserInfo;
                    string strAccountList;
                    string strPhoneNum;
                    if (!logic::SelectUserTable(::atoi(oRequest.strUserID.c_str()), mapUserInfo))
                    {
                        ERROR("Get user [" + oRequest.strUserID + "] info failed!");
                        break;
                    }
                    if (mapUserInfo.end() == mapUserInfo.find("Fphone_num"))
                    {
                        ERROR("Hasn't user phone value!");
                        break;
                    }
                    strPhoneNum = mapUserInfo["Fphone_num"];
                    DEBUG("User's phone num is: " + strPhoneNum);
                    oClearSilver.SetValue("tel", strPhoneNum);
                    if (mapUserInfo.end() == mapUserInfo.find("Faccount_id_list"))
                    {
                        ERROR("Hasn't user account id list value!");
                        break;
                    }
                    strAccountList = mapUserInfo["Faccount_id_list"];
                    DEBUG("user account list is: " + strAccountList);

                    //第二步：用账户ID获取用户账户详情
                    vector<string> vecAccountIdList;
                    vecAccountIdList = tools::CStringTools::Split2String(strAccountList, "#");
                    size_t maxSize = vecAccountIdList.size();
                    DEBUG("User's account numbers are: " + tools::CStringTools::Int2String(maxSize));
                    oClearSilver.SetValue("payway.bank.payway_id", NULL);
                    oClearSilver.SetValue("payway.bank.payway_type", tools::CStringTools::Int2String(ACCOUNT_ONLINEBANK));
                    oClearSilver.SetValue("payway.bank.name", NULL);
                    oClearSilver.SetValue("payway.bank.bank", NULL);
                    oClearSilver.SetValue("payway.bank.account", NULL);
                    oClearSilver.SetValue("payway.alipay.payway_id", NULL);
                    oClearSilver.SetValue("payway.alipay.payway_type", tools::CStringTools::Int2String(ACCOUNT_ALIPAY));
                    oClearSilver.SetValue("payway.alipay.account", NULL);
                    size_t index;
                    for (index = 0; index != maxSize; ++ index)
                    {
                        map<string, string> mapAccountInfo;
                        if (!logic::SelectAccountInfo(::atoi(vecAccountIdList[index].c_str()), mapAccountInfo))
                        {
                            WARN("There is one account_id [" + vecAccountIdList[index] + "] is unvaluable!");
                            continue;
                        }
                        else
                        {
                            //读取账户信息成功
                            DEBUG("Faccount_type: " + mapAccountInfo["Faccount_type"] +
                                  "\nFwy_name: " + mapAccountInfo["Fwy_name"] +
                                  "\nFwy_phone: " + mapAccountInfo["Fwy_phone"] +
                                  "\nFwy_bank_name: " + mapAccountInfo["Fwy_bank_name"] +
                                  "\nFwy_bank_cardid: " + mapAccountInfo["Fwy_bank_cardid"] +
                                  "\nFzfb_phone: " + mapAccountInfo["Fzfb_phone"] +
                                  "\nFzfb_account: " + mapAccountInfo["Fzfb_account"]);
                        }
                        unsigned int iPaywayType = ::atoi(mapAccountInfo["Faccount_type"].c_str());
                        if (ACCOUNT_ONLINEBANK == iPaywayType)
                        {
                            //网银
                            DEBUG("User using online bank.");
                            oClearSilver.SetValue("payway.bank.payway_id", vecAccountIdList[index]);
                            oClearSilver.SetValue("payway.bank.payway_type", mapAccountInfo["Faccount_type"]);
                            oClearSilver.SetValue("payway.bank.name", mapAccountInfo["Fwy_name"]);
                            oClearSilver.SetValue("payway.bank.bank", mapAccountInfo["Fwy_bank_name"]);
                            oClearSilver.SetValue("payway.bank.account", mapAccountInfo["Fwy_bank_cardid"]);
                        }
                        else if (ACCOUNT_ALIPAY == iPaywayType)
                        {
                            //支付宝
                            DEBUG("User using alipay.");
                            oClearSilver.SetValue("payway.alipay.payway_id", vecAccountIdList[index]);
                            oClearSilver.SetValue("payway.alipay.payway_type", mapAccountInfo["Faccount_type"]);
                            oClearSilver.SetValue("payway.alipay.account", mapAccountInfo["Fzfb_account"]);
                        }
                        else
                        {
                            WARN("There is no such payway type[" + mapAccountInfo["Faccount_type"] + "]");
                            continue;
                        }
                    }
                }while(0);
                strResult = oClearSilver.Render(strCsFile);
                FCGI_printf("Content-type:text/html\r\n\r\n");
                DEBUG_AND_RETURN(strResult.c_str());
                return;
            }
            else
            {
                ERROR("Select goods [" + tools::CStringTools::Int2String(iGoodsID) + "] failed.");
                break;
            }
        }
        else
        {
            ERROR("Quotation result failed!");
            break;
        }
    }while(0);

    //错误处理
    strResult = oClearSilver.Render(strErrorCsFile);
    DEBUG_AND_RETURN("Content-type:text/html\r\n\r\n");
    DEBUG_AND_RETURN(strResult.c_str());
    FCGI_printf("Calculate price failed!");
    ERROR("Calculate price failed!");
    DEBUG("============== End to handle cgi ================");
}

int main()
{
    while(FCGI_Accept() >= 0)
    {
        log::CLog::Initialize("");
        CRequest oRequest;
        bool parseResult = ParseCgi(oRequest);

        stringstream ssRequest;
        ssRequest << "==========CGI START==========\n"
                  << "request method    is : " << oRequest.strMethod << "\n"
                  << "request get data  is : " << oRequest.strGetData << "\n"
                  << "request cookie    is : " << oRequest.strCookie << "\n"
                  << "uid               is : " << oRequest.strUserID << "\n"
                  << "userkey           is : " << oRequest.strUserKey << "\n"
                  << "request url       is : " << oRequest.strScriptFileName << "\n"
                  << "pid               is : " << oRequest.strPid << "\n"
                  << "selects           is : " << oRequest.strSelects << "\n";
        DEBUG(ssRequest.str());

        if (!parseResult)
        {
            if (oRequest.bFromSharedPage)
            {
                DEBUG("From shared page, and is already location to home_page!");
            }
            else
            {
                ERROR("invalid request");
                static const string strErrorCsFile = "error_page.cs";
                tools::CClearSilver oClearSilver;
                /// start modified by mickey at 2014-11-20 2:20:00
                //oClearSilver.SetValue("cdnbase", CDN_BASE);
                oClearSilver.SetValue("cdnbase", TENCENT_CDN_BASE);
                /// end modified
                oClearSilver.SetDirectory(CDN_M_DIRECTORY);
                string strResult = oClearSilver.Render(strErrorCsFile);
                DEBUG_AND_RETURN("Content-type:text/html\r\n\r\n");
                DEBUG_AND_RETURN(strResult.c_str());
            }
        }
        else
        {
            HandleCgi(oRequest);
        }
        DEBUG("===========CGI END===========");
    }

    return 0;
}


