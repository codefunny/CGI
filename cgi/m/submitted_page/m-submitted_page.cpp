//======================================================================
//
//        Copyright (C) 2014 回收宝
//        All rights reserved
//
//        filename : m-submitted_page.cpp
//        description : 微信用户订单提交成功的CGI入口
//
//        created by 张世柏 at  2014-10-20 19:09:00
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

using namespace std;

class CRequest
{
public:
    CRequest() : iContentLength(0){}
    string strGetData;
    string strMethod;
    unsigned int iContentLength;
    string strPostData;
    string strScriptFileName;
    string strCookie;
    string strUserID;
    string strUserKey;
    string strChannel;
    string strReferer;
};

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
        if (mapRequest.end() != mapRequest.find("channel"))
        {
            oRequest.strChannel = mapRequest["channel"];
        }
    }

    char* data = (char*)::malloc(oRequest.iContentLength + 1);
    ::memset(data, 0, oRequest.iContentLength + 1);
    FCGI_fgets(data, oRequest.iContentLength + 1, FCGI_stdin);
    oRequest.strPostData = data;
    ::free(data);

    return oRequest;
}

static void HandleCgi(const CRequest& oRequest)
{
    string strCGIName = "/submitted_page";
    string strCsFile = "submitted_page.cs";
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
                  << "request post data is : " << oRequest.strPostData << "\n"
                  << "request url       is : " << oRequest.strScriptFileName << "\n"
                  << "request cookie    is : " << oRequest.strCookie << "\n"
                  << "userID            is : " << oRequest.strUserID << "\n"
                  << "userKey           is : " << oRequest.strUserKey << "\n"
                  << "channel           is : " << oRequest.strChannel << "\n"
                  << "referer           is : " << oRequest.strReferer << "\n";
        DEBUG(ssRequest.str());

        HandleCgi(oRequest);

        DEBUG("===========CGI END===========");
    }

    return 0;
}

