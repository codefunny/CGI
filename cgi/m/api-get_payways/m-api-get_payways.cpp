//======================================================================
//
//        Copyright (C) 2014 回收宝
//        All rights reserved
//
//        filename : m-api-get_payways.cpp
//        description : 微信用户查询支付信息的CGI入口
//
//        created by 张世柏 at  2014-10-30 15:21:00
//        modified by XX at
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
#include "user_table.h"
#include "login.h"
#include "account_info_table.h"
#include "wechat_template_msg.h"
#include "order.h"

using namespace std;

class CRequest
{
public:
    CRequest() : iContentLength(0){}
    string strGetData;
    string strMethod;
    unsigned int iContentLength;
    string strScriptFileName;
    string strCookie;
    string strUserID;
    string strUserKey;
};

static bool ParseCgi(CRequest& oRequest)
{
    DEBUG("=========== ParseCgi START =========");
    if (getenv("QUERY_STRING"))
    {
        oRequest.strGetData = getenv("QUERY_STRING");
    }

    if (getenv("REQUEST_METHOD"))
    {
        oRequest.strMethod = getenv("REQUEST_METHOD");
    }

    if (getenv("CONTENT_LENGTH"))
    {
        oRequest.iContentLength = ::atoi(getenv("CONTENT_LENGTH"));
    }

    if (getenv("SCRIPT_FILENAME"))
    {
        oRequest.strScriptFileName = getenv("SCRIPT_FILENAME");
    }

    if (getenv("HTTP_COOKIE"))
    {
        oRequest.strCookie = getenv("HTTP_COOKIE");
        oRequest.strCookie = tools::CStringTools::Trim(oRequest.strCookie);
    }

    if (!oRequest.strCookie.empty())
    {
        map<string, string> mapRequest = tools::CStringTools::SplitTwice2String(oRequest.strCookie, ";", "=");
        if (mapRequest.end() == mapRequest.find("uid"))
        {
            ERROR("invalid request");
            return false;
        }
        if (mapRequest.end() == mapRequest.find("userkey"))
        {
            ERROR("invalid request");
            return false;
        }
        oRequest.strUserID = mapRequest["uid"];
        oRequest.strUserKey = mapRequest["userkey"];
    }
    else
    {
        ERROR("Hasn't cookies!");
        return false;
    }
    DEBUG("=========== ParseCgi END =========");
    return true;
}

static void HandleCgi(CRequest& oRequest, Json::Value& jsonOutput)
{
    DEBUG("=========== HandleCgi START =========");
    string strCGIName = "/api/get_payways";
    if (strCGIName != oRequest.strScriptFileName)
    {
        ERROR("invalid request");
        jsonOutput["ret"] = "1";
        jsonOutput["errstr"] = "请求错误";
        return;
    }

    //校验登陆态
    if (!user::CheckLandingState(oRequest.strUserID, oRequest.strUserKey))
    {
        ERROR("User has not login!");
        jsonOutput["ret"] = "1";
        jsonOutput["errcode"] = tools::CStringTools::Int2String(error_num::USER_UNLOGIN);
        jsonOutput["errstr"] = "用户未登陆";
        return;
    }

    Json::Value jsonPaywayData, jsonAliPayData, jsonBankPayData;
    bool bSearchIsSuccess = false;
    do
    {
        //第一步：获取用户账户ID列表
        map<string, string> mapUserInfo;
        string strAccountList;
        if (!logic::SelectUserTable(::atoi(oRequest.strUserID.c_str()), mapUserInfo))
        {
            ERROR("Get user [" + oRequest.strUserID + "] info failed!");
            break;
        }
        strAccountList = mapUserInfo["Faccount_id_list"];
        DEBUG("user account list is: " + strAccountList);

        //第二步：用账户ID获取用户账户详情
        Json::Value jsonAliPayData, jsonBankPayData;
        vector<string> vecAccountIdList;
        vecAccountIdList = tools::CStringTools::Split2String(strAccountList, "#");
        size_t maxSize = vecAccountIdList.size();
        DEBUG("User's account numbers are: " + tools::CStringTools::Int2String(maxSize));
        jsonAliPayData["payway_id"] = Json::Value("");
        jsonAliPayData["payway_type"] = Json::Value(tools::CStringTools::Int2String(ACCOUNT_ALIPAY));
        jsonAliPayData["account"] = Json::Value("");
        jsonBankPayData["payway_id"] = Json::Value("");
        jsonBankPayData["payway_type"] = Json::Value(tools::CStringTools::Int2String(ACCOUNT_ONLINEBANK));
        jsonBankPayData["name"] = Json::Value("");
        jsonBankPayData["bank"] = Json::Value("");
        jsonBankPayData["account"] = Json::Value("");
        size_t index;
        for (index = 0; index != maxSize; ++ index)
        {
            map<string, string> mapAccountInfo;
            if (!logic::SelectAccountInfo(::atoi(vecAccountIdList[index].c_str()), mapAccountInfo))
            {
                WARN("Get account info [" + vecAccountIdList[index] + "] failed!");
                continue;
            }
            else
            {
                //读取账户信息成功
                DEBUG("Get account info [" + vecAccountIdList[index] + "] successed!");
            }
            unsigned int iPaywayType = ::atoi(mapAccountInfo["Faccount_type"].c_str());
            if (ACCOUNT_ONLINEBANK == iPaywayType)
            {
                //网银
                DEBUG("User using online bank.");
                jsonBankPayData["payway_id"] = Json::Value(vecAccountIdList[index]);
                jsonBankPayData["payway_type"] = Json::Value(mapAccountInfo["Faccount_type"]);
                jsonBankPayData["name"] = Json::Value(mapAccountInfo["Fwy_name"]);
                jsonBankPayData["bank"] = Json::Value( mapAccountInfo["Fwy_bank_name"]);
                jsonBankPayData["account"] = Json::Value(mapAccountInfo["Fwy_bank_cardid"]);
            }
            else if (ACCOUNT_ALIPAY == iPaywayType)
            {
                //支付宝
                DEBUG("User using alipay.");
                jsonAliPayData["payway_id"] = Json::Value(vecAccountIdList[index]);
                jsonAliPayData["payway_type"] = Json::Value(mapAccountInfo["Faccount_type"]);
                jsonAliPayData["account"] = Json::Value(mapAccountInfo["Fzfb_account"]);
            }
            else
            {
                WARN("There is no such payway type[" + mapAccountInfo["Faccount_type"] + "]");
                continue;
            }
        }
        jsonPaywayData["alipay"] = jsonAliPayData;
        jsonPaywayData["bank"] = jsonBankPayData;
        bSearchIsSuccess = true;
    }while(0);

    if (bSearchIsSuccess)
    {
        Json::Value jsonPayway;
        jsonPayway["payway"] = jsonPaywayData;

        jsonOutput["errcode"] = "0";
        jsonOutput["errstr"] = "";
        jsonOutput["data"] = jsonPayway;
    }
    else
    {
        jsonOutput["ret"] = "1";
        jsonOutput["errcode"] = tools::CStringTools::Int2String(error_num::ACCOUNT_GET_FAILED);
        jsonOutput["errstr"] = "查询失败";
    }
    DEBUG("=========== HandleCgi END =========");
    return;
}

int main()
{
    while(FCGI_Accept() >= 0)
    {
        FCGI_printf("Content-type:text/html\r\n\r\n");

        log::CLog::Initialize("");
        Json::Value jsonOutput;
        CRequest oRequest;
        bool parseResult = ParseCgi(oRequest);

        stringstream ssRequest;
        ssRequest << "==========CGI START==========\n"
                  << "request method    is : " << oRequest.strMethod << "\n"
                  << "content length    is : " << oRequest.iContentLength << "\n"
                  << "request get data  is : " << oRequest.strGetData << "\n"
                  << "request cookie    is : " << oRequest.strCookie << "\n"
                  << "uid               is : " << oRequest.strUserID << "\n"
                  << "userkey           is : " << oRequest.strUserKey << "\n"
                  << "request url       is : " << oRequest.strScriptFileName << "\n";
        DEBUG(ssRequest.str());

        if (!parseResult)
        {
            ERROR("invalid request");
            jsonOutput["ret"] = "1";
            jsonOutput["errstr"] = "参数错误";
        }
        else
        {
            jsonOutput["ret"] = "0";
            HandleCgi(oRequest, jsonOutput);
        }

        Json::FastWriter fast_writer;
        stringstream ssResult;
        ssResult << fast_writer.write(jsonOutput);
        string strResult = ssResult.str();
        DEBUG_AND_RETURN(strResult.c_str());

        DEBUG("===========CGI END===========");
    }

    return 0;
}


