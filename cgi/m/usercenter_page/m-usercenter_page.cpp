//======================================================================
//
//        Copyright (C) 2014 回收宝
//        All rights reserved
//
//        filename : m-usercenter_page.cpp
//        description : 微信用户个人中心的CGI入口
//
//        created by 张世柏 at  2014-10-20 21:09:00
//        modified by mickey at 2014-11-20 2:20:00
//
//======================================================================

#include <stdlib.h>
#include <string.h>
#include <string>
#include <map>
#include <sstream>
#include <fcgi_stdio.h>
#include "configure.h"
#include "login.h"
#include "clearsilver.h"
#include "string_tools.h"
#include "user_table.h"
#include "wechat_login.h"

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
    string strChannel;
    string strReferer;
    bool bFromTmplateMsg;
};

bool WriteCookieSuccess(const string& strCode, CRequest& oRequest)
{
    DEBUG("============ WriteCookieSuccess START ============");
    DEBUG("Begin to get openid and access_token");
    string strOpenID, strAccessToken;
    //获取用户的openid和access_token
    if (!wechat::GetOpenIDAccessToken(strCode, strOpenID, strAccessToken))
    {
        ERROR("Fail to get openid and access_token!");
        return false;
    }
    else
    {
        DEBUG("Success to get openid and access_token!");
    }
    map<string, string> mapUserInfo;
    bool isOldUser;
    //判断是否在用户表中,如果存在就通过mapUserInfo返回，返回值是数据库操作是否正确
    bool bIsSqlQuerySuccess= logic::CheckAndSelectUserByUserName(strOpenID, mapUserInfo, isOldUser);
    if (bIsSqlQuerySuccess)
    {
        if (isOldUser)
        {
            //写入缓存
            if(mapUserInfo.end() == mapUserInfo.find("Fuser_id"))
            {
                ERROR("mapUserInfo doesn't has item Fuser_id.");
                return false;
            }
            user::SaveLandingState(mapUserInfo["Fuser_id"], strAccessToken);

            //写入cookie
            DEBUG("Writing into cookie!");
            DEBUG("Fuser_id = " + mapUserInfo["Fuser_id"]);
            DEBUG("Fuser_channel = " + mapUserInfo["Fuser_channel"]);
            DEBUG_AND_RETURN(wechat::WriteInCookie("uid", mapUserInfo["Fuser_id"]).c_str());
            DEBUG_AND_RETURN(wechat::WriteInCookie("userkey", strAccessToken).c_str());
            DEBUG_AND_RETURN(wechat::WriteInCookie("channel", mapUserInfo["Fuser_channel"]).c_str());
            oRequest.strUserID = mapUserInfo["Fuser_id"];
            oRequest.strUserKey = strAccessToken;
        }
        else
        {
            ERROR("This user [ " + strOpenID + " ] is a new user");
            return false;
        }
    }
    else
    {
        ERROR("Check and get user by user name [" + strOpenID + "]falsed!");
        return false;
    }
    DEBUG("============ WriteCookieSuccess END ============");
    return true;
}

void CheckIsComeFromTemplateMsg(CRequest& oRequest, bool bCookieValuable)
{
    DEBUG("============ CheckIsComeFromTemplateMsg START ============");
    oRequest.bFromTmplateMsg = false;
    if (!oRequest.strGetData.empty())
    {
        map<string, string> mapRequest = tools::CStringTools::SplitTwice2String(oRequest.strGetData, "&", "=");
        if (mapRequest.end() != mapRequest.find("code"))
        {
            string strOpenID, strAccessToken;
            string strCode = mapRequest["code"];

            if (!bCookieValuable)
            {
                if (WriteCookieSuccess(strCode, oRequest))
                {
                    oRequest.bFromTmplateMsg = true;
                }
                else
                {
                    ERROR("Write in cookie falsed!");
                }
            }
            else
            {
                //cookie有效
                oRequest.bFromTmplateMsg = true;
            }
        }
        else
        {
            DEBUG("Haven't code parameter.");
        }
    }
    else
    {
        DEBUG("Havent't Get parameter.");
    }
    DEBUG("============ CheckIsComeFromTemplateMsg END ============");
}

static CRequest ParseCgi()
{
    CRequest oRequest;
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
    bool bCookieValuable = false;
    if (!oRequest.strCookie.empty())
    {
        map<string, string> mapRequest = tools::CStringTools::SplitTwice2String(oRequest.strCookie, ";", "=");
        if (mapRequest.end() == mapRequest.find("uid"))
        {
            ERROR("invalid request");
            return oRequest;
        }
        if (mapRequest.end() == mapRequest.find("userkey"))
        {
            ERROR("invalid request");
            return oRequest;
        }
        oRequest.strUserID = mapRequest["uid"];
        oRequest.strUserKey = mapRequest["userkey"];
        bCookieValuable = true;
        if (mapRequest.end() != mapRequest.find("channel"))
        {
            oRequest.strChannel = mapRequest["channel"];
        }
    }

    CheckIsComeFromTemplateMsg(oRequest, bCookieValuable);
    return oRequest;
}

static void HandleCgi(const CRequest& oRequest)
{
    string strCGIName = "/usercenter_page";
    string strCsFile = "usercenter_page.cs";
    string strErrorCsFile = "error_page.cs";
    string strLocationUrl = "Location:" + WECHAT_LOCATION_FOR_CODE + "?appid=" + WECHAT_APPID + "&redirect_uri=" + WECHAT_LOCATION_HOME_PAGE + WECHAT_LOCATION_PARAM + "\r\n";

    if (strCGIName != oRequest.strScriptFileName)
    {
       ERROR("invalid request");
       return;
    }

    if (!user::CheckLandingState(oRequest.strUserID, oRequest.strUserKey))
    {
        ERROR("User [" + oRequest.strUserID + "] hasn't logined!");
        DEBUG_AND_RETURN(strLocationUrl.c_str());
        FCGI_printf("Content-type:text/html\r\n\r\n");
        return;
    }

    do
    {
        if (oRequest.bFromTmplateMsg)
        {
            DEBUG("From template message.");
            break;
        }
        else
        {
            DEBUG("Not from Template message.");
        }
        if (oRequest.strReferer.empty())
        {
            ERROR("Http referer is NULL.");
            DEBUG_AND_RETURN(strLocationUrl.c_str());
            FCGI_printf("Content-type:text/html\r\n\r\n");
            return;
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
                return;
            }
            else
            {
                //match
                DEBUG("Lawful request. ");
            }
        }
    }while(0);

    string strResult;
    tools::CClearSilver oClearSilver;
    /// start modified by mickey at 2014-11-20 2:20:00
    //oClearSilver.SetValue("cdnbase", CDN_BASE);
    oClearSilver.SetValue("cdnbase", TENCENT_CDN_BASE);
    /// end modified
    oClearSilver.SetDirectory(CDN_M_DIRECTORY);
    strResult = oClearSilver.Render(strCsFile);
    FCGI_printf("Content-type:text/html\r\n\r\n");
    DEBUG_AND_RETURN(strResult.c_str());
}

int main()
{
    while(FCGI_Accept() >= 0)
    {
        log::CLog::Initialize("");

        CRequest oRequest = ParseCgi();

        stringstream ssRequest;
        ssRequest << "==========CGI START==========\n"
                  << "request method    is : " << oRequest.strMethod << "\n"
                  << "content length    is : " << oRequest.iContentLength << "\n"
                  << "request get data  is : " << oRequest.strGetData << "\n"
                  << "request url       is : " << oRequest.strScriptFileName << "\n"
                  << "request cookie    is : " << oRequest.strCookie << "\n"
                  << "userID            is : " << oRequest.strUserID << "\n"
                  << "userKey           is : " << oRequest.strUserKey << "\n"
                  << "channel           is : " << oRequest.strChannel << "";
        DEBUG(ssRequest.str());

        HandleCgi(oRequest);

        DEBUG("===========CGI END===========");
    }

    return 0;
}

