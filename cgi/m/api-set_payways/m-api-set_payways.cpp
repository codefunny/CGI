//======================================================================
//
//        Copyright (C) 2014 回收宝
//        All rights reserved
//
//        filename : m-api-set_payways.cpp
//        description : 微信用户写支付信息的CGI入口
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
    string strPostData;
    string strPaywayType;
    string strPaywayName;
    string strPaywayBank;
    string strPaywayBankAccount;
    string strPaywayAliAccount;
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

    char* data = (char*)::malloc(oRequest.iContentLength + 1);
    ::memset(data, 0, oRequest.iContentLength + 1);
    FCGI_fgets(data, oRequest.iContentLength + 1, FCGI_stdin);
    oRequest.strPostData = data;
    oRequest.strPostData = tools::CStringTools::UrlDecode(oRequest.strPostData);
    ::free(data);

    if (!oRequest.strPostData.empty())
    {
        map<string, string> mapRequest = tools::CStringTools::SplitTwice2String(oRequest.strPostData, "&", "=");
        if (mapRequest.end() == mapRequest.find("payway_type"))
        {
            ERROR("Cannot find payway_type value.");
            return false;
        }
        oRequest.strPaywayType = mapRequest["payway_type"];
        DEBUG("strPaywayType: " + oRequest.strPaywayType);
        unsigned int iPaywayType = ::atoi(oRequest.strPaywayType.c_str());
        if (ACCOUNT_ONLINEBANK == iPaywayType)
        {
            //网银支付
            DEBUG("User use bank as pay way.");
            if (mapRequest.end() == mapRequest.find("payway_name"))
            {
                ERROR("Cannot find payway_name value.");
                return false;
            }
            if (mapRequest.end() == mapRequest.find("payway_bank"))
            {
                ERROR("Cannot find payway_bank value.");
                return false;
            }
            if (mapRequest.end() == mapRequest.find("payway_bank_account"))
            {
                ERROR("Cannot find payway_bank_account value.");
                return false;
            }
            oRequest.strPaywayName = mapRequest["payway_name"];
            oRequest.strPaywayBank = mapRequest["payway_bank"];
            oRequest.strPaywayBankAccount = mapRequest["payway_bank_account"];
            DEBUG("payway_name: " + oRequest.strPaywayName +
                  "\npayway_bank: " + oRequest.strPaywayBank +
                  "\npayway_bank_account: " + oRequest.strPaywayBankAccount);
        }
        else if (ACCOUNT_ALIPAY == iPaywayType)
        {
            //支付宝支付
            DEBUG("User use alipay as pay way.");
            if (mapRequest.end() == mapRequest.find("payway_alipay_account"))
            {
                ERROR("Cannot find payway_alipay_account value.");
                return false;
            }
            oRequest.strPaywayAliAccount = mapRequest["payway_alipay_account"];
            DEBUG("payway_ali_account: " + oRequest.strPaywayAliAccount);
        }
        else
        {
            ERROR("This pay way [" + oRequest.strPaywayType + "] doesn't exit!");
            return false;
        }
    }
    else
    {
        ERROR("Hasn't post data!");
        return false;
    }
    DEBUG("=========== ParseCgi END =========");
    return true;
}

static void HandleCgi(const CRequest& oRequest, Json::Value& jsonOutput)
{
    DEBUG("=========== HandleCgi START =========");
    string strCGIName = "/api/set_payways";
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

    bool bModifyIsSuccess = false;
    do
    {
        //第一步：修改账户信息
        map<string, string> mapAccountInfo;
        string strNewAccountID;
        mapAccountInfo.insert(make_pair<string, string>("Faccount_type", oRequest.strPaywayType));
        unsigned int iAccountInfoID;
        unsigned int iPaywayType = ::atoi(oRequest.strPaywayType.c_str());
        if (ACCOUNT_ONLINEBANK == iPaywayType)
        {
            //网银
            DEBUG("User use online_bank.");
            mapAccountInfo.insert(make_pair<string, string>("Fwy_name", oRequest.strPaywayName));
            mapAccountInfo.insert(make_pair<string, string>("Fwy_phone", ""));
            mapAccountInfo.insert(make_pair<string, string>("Fwy_bank_name", oRequest.strPaywayBank));
            mapAccountInfo.insert(make_pair<string, string>("Fwy_bank_cardid", oRequest.strPaywayBankAccount));
        }
        else if (ACCOUNT_ALIPAY == iPaywayType)
        {
            //支付宝
            DEBUG("User use alipay.");
            mapAccountInfo.insert(make_pair<string, string>("Fzfb_phone", ""));
            mapAccountInfo.insert(make_pair<string, string>("Fzfb_account", oRequest.strPaywayAliAccount));
        }
        else
        {
            ERROR("This pay way [" + oRequest.strPaywayType + "] doesn't exit!");
            break;
        }

        //执行插入操作
        if (!logic::InsertAccountInfoTable(mapAccountInfo, iAccountInfoID))
        {
            ERROR("Insert account info failed.");
            break;
        }
        else
        {
            strNewAccountID = tools::CStringTools::Int2String(iAccountInfoID);
            DEBUG("Insert account info successed! Account id is: " + strNewAccountID);
        }

        //第二步：修改用户信息
        //用户每次下单都要更新用户表的Faccount_id 和 Fphone_num字段
        //第一步：取出用户表中的Faccount_id_list字段
        map<string, string> mapUserInfo;
        if (!logic::SelectUserTable(::atoi(oRequest.strUserID.c_str()), mapUserInfo))
        {
            ERROR("Select user info [user_id: " + oRequest.strUserID + "] failed!");
            break;
        }
        else
        {
            //获取用户信息成功
            DEBUG("Get user [" + mapUserInfo["Fuser_id"] + "] info successed!");
        }
        if (mapUserInfo.end() == mapUserInfo.find("Faccount_id_list"))
        {
            ERROR("Cannot find Faccount_id_list value.");
            break;
        }
        string strAccountList = mapUserInfo["Faccount_id_list"];
        string strPhoneNum = mapUserInfo["Fphone_num"];
        DEBUG("The old AccountList is: [" + strAccountList + "]");

        //第二步：将用户最近使用的不同种类的支付账号依次读出
        vector<string> vecUserAccountList;
        vecUserAccountList = tools::CStringTools::Split2String(strAccountList, "#");
        size_t maxSize = vecUserAccountList.size();
        DEBUG("User has used [" + tools::CStringTools::Int2String(maxSize) + "] pay ways.");
        size_t index;
        for (index = 0; index != maxSize; ++index)
        {
            map<string, string> mapAccountInfo;
            if (!logic::SelectAccountInfo(::atoi(vecUserAccountList[index].c_str()), mapAccountInfo))
            {
                WARN("There is one account_id [" + vecUserAccountList[index] + "] is unvaluable!");
                continue;
            }

            //注意：这里默认前台传来的支付类型编号和数据库的一致
            if (oRequest.strPaywayType == mapAccountInfo["Faccount_type"])
            {
                //更新了用户以前用过的同一类支付方式
                DEBUG("The old AccountID is : " + vecUserAccountList[index]);
                vecUserAccountList[index] = strNewAccountID;
                DEBUG("The new AccountID is : " + vecUserAccountList[index]);
                DEBUG("Update one Payway which user has used before, and payway type is: " + oRequest.strPaywayType);
                break;
            }
        }

        if (index == maxSize)
        {
            DEBUG("User is first used account with this payway type [" + oRequest.strPaywayType + "], New AccountID is : " + strNewAccountID);
            vecUserAccountList.push_back(strNewAccountID);
        }

        //第三步：更新用户最近一次的不同账单列表
        //有问题，需加事务???????????????????????????????????
        strAccountList = "";
        maxSize = vecUserAccountList.size();
        for (index = 0; index != maxSize; ++index)
        {
            strAccountList += (0 == index ? vecUserAccountList[index] : ("#" + vecUserAccountList[index]));
        }
        DEBUG("The new AccountList is: [" + strAccountList + "]");

        //第四步：更新用户表的Faccount_id 和 Fphone_num字段
        if (!logic::UpdateUserTableAccountListAndPhoneNum(::atoi(oRequest.strUserID.c_str()), strAccountList, strPhoneNum))
        {
            ERROR("Update t_user [user_id: " + oRequest.strUserID + "] failed!");
            break;
        }
        else
        {
            DEBUG("Update t_user [user_id: " + oRequest.strUserID + "] successed!");
        }
        bModifyIsSuccess = true;
    }while(0);

    if (bModifyIsSuccess)
    {
        jsonOutput["errcode"] = "0";
        jsonOutput["errstr"] = "";
    }
    else
    {
        jsonOutput["ret"] = "1";
        jsonOutput["errcode"] = tools::CStringTools::Int2String(error_num::ACCOUNT_MODIFY_FAILED);
        jsonOutput["errstr"] = "修改失败";
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
                  << "request post data is : " << oRequest.strPostData << "\n"
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


