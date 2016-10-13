//======================================================================
//
//        Copyright (C) 2014 回收宝
//        All rights reserved
//
//        filename : home_page.cpp
//        description : 主页的CGI入口
//
//        created by 叶飞 at  2014-08-01 10:00:00
//        modified by XX at  
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
#include "user_table.h"
#include "string_tools.h"
#include "group_table.h"

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
    string strCGIName = "/home_page";
    string strCsFile = "home_page.cs";

    if (strCGIName != oRequest.strScriptFileName)
    {
        ERROR("invalid request");
        return;
    }

    do
    {
        if (user::CheckLandingState(oRequest.strUserID, oRequest.strUserKey))
        {
            DEBUG("already login");

            map<string, string> userInfo;
            if (!logic::SelectUserTable(::atoi(oRequest.strUserID.c_str()), userInfo))
            {
                ERROR("SelectUserTable failed");
                break;
            }
            if (userInfo.end() == userInfo.find("Fuser_name"))
            {
                ERROR("invalid user");
                break;
            }
            if (userInfo.end() == userInfo.find("Fchannel_name"))
            {
                ERROR("invalid user");
                break;
            }

            string strPermission;
            vector< map<string, string> > authorityList;
            if (!logic::SelectGroupTableAuthorityList(::atoi(oRequest.strUserID.c_str()), authorityList))
            {
                ERROR("SelectGroupTable failed");
                break;
            }
            size_t maxSize = authorityList.size();
            for (size_t index = 0; index < maxSize; index++)
            {
                if (authorityList[index].end() == authorityList[index].find("Fauthority_id"))
                {
                    ERROR("Frole_id not found");
                    continue;
                }

                if (0 == index)
                {
                    strPermission += "[";
                    strPermission += authorityList[index]["Fauthority_id"];
                }
                else
                {
                    strPermission += ", ";
                    strPermission += authorityList[index]["Fauthority_id"];
                }

                if (maxSize - 1 == index)
                {
                    strPermission += "]";
                }
            }

            tools::CClearSilver oClearSilver;
            oClearSilver.SetValue("cdnbase", CDN_BASE);
            oClearSilver.SetValue("logininfo.islogined", "1");
            oClearSilver.SetValue("logininfo.uid", oRequest.strUserID);
            oClearSilver.SetValue("logininfo.username", userInfo["Fuser_name"]);
            oClearSilver.SetValue("logininfo.channel", userInfo["Fchannel_name"]);
            if (userInfo.end() != userInfo.find("Fchannel_logo"))
            {
                oClearSilver.SetValue("logininfo.channelpic", userInfo["Fchannel_logo"]);
            }
            oClearSilver.SetValue("logininfo.permission", strPermission);
            oClearSilver.SetDirectory(CDN_ROOT_DIRECTORY);
            string strResult = oClearSilver.Render(strCsFile);

            FCGI_printf("Content-type:text/html\r\n\r\n");
            DEBUG_AND_RETURN(strResult.c_str());
            return;
        }
    }while(0);

    DEBUG("unlogin");


    tools::CClearSilver oClearSilver;
    oClearSilver.SetValue("cdnbase", CDN_BASE);
    oClearSilver.SetValue("logininfo.islogined", "0");
    oClearSilver.SetValue("logininfo.uid", "");
    oClearSilver.SetValue("logininfo.username", "");
    oClearSilver.SetValue("logininfo.channel", "");
    oClearSilver.SetValue("logininfo.channelpic", "");
    oClearSilver.SetDirectory(CDN_ROOT_DIRECTORY);
    string strResult = oClearSilver.Render(strCsFile);

    //FCGI_printf("Location: http://huishoubao.com.cn/login_page\r\n");
    string strLocation = "Location: http://" + DOMAIN_NAME + "/login_page\r\n";
    FCGI_printf(strLocation.c_str());
    FCGI_printf("Content-type:text/html\r\n\r\n");
    DEBUG_AND_RETURN(strResult.c_str());
    return;
}

int main()
{
    while(FCGI_Accept() >= 0)
    {
        //FCGI_printf("Content-type:text/html\r\n\r\n");

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
                  << "channel           is : " << oRequest.strChannel << "";
        DEBUG(ssRequest.str());

        HandleCgi(oRequest);

        DEBUG("===========CGI END===========");
    }

    return 0;
}

