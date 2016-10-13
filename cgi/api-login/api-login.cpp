//======================================================================
//
//        Copyright (C) 2014 回收宝
//        All rights reserved
//
//        filename : api-login.cpp
//        description : 登录的CGI入口
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
#include <json/json.h>
#include "configure.h"
#include "error_num.h"
#include "string_tools.h"

#include "login.h"
#include "channel_table.h"

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
    string strPostData;
    string strChannel;
    string strName;
    string strPsw;
};

static bool ParseCgi(CRequest& oRequest)
{
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
        if (mapRequest.end() == mapRequest.find("channel"))
        {
            ERROR("invalid request");
            return false;
        }
        if (mapRequest.end() == mapRequest.find("name"))
        {
            ERROR("invalid request");
            return false;
        }
        if (mapRequest.end() == mapRequest.find("psw"))
        {
            ERROR("invalid request");
            return false;
        }
        oRequest.strChannel = mapRequest["channel"];
        oRequest.strName = mapRequest["name"];
        oRequest.strPsw = mapRequest["psw"];
    }

    return true;
}

static void HandleCgi(const CRequest& oRequest, Json::Value& jsonOutput)
{
    string strCGIName = "/api/login";
    if (strCGIName != oRequest.strScriptFileName)
    {
        ERROR("invalid request");
        jsonOutput["ret"] = "1";
        return;
    }

    unsigned int iChannelIDAfterConvert = user::ChannelFuzzy(::atoi(oRequest.strChannel.c_str()), oRequest.strName);

    string strUserID;
    string strUserKey;
    if (user::Login(iChannelIDAfterConvert, oRequest.strName, oRequest.strPsw, strUserID, strUserKey))
    {
        DEBUG("login succeed");

        map<string, string> channelInfo;
        if (!logic::SelectChannelTable(iChannelIDAfterConvert, channelInfo))
        {
            ERROR("SelectChannelTable failed");
            jsonOutput["ret"] = "1";
            jsonOutput["errcode"] = tools::CStringTools::Int2String(error_num::CHANNEL_UNKNOWN);
            jsonOutput["errstr"] = "渠道不存在";
            return;
        }
        if (channelInfo.end() == channelInfo.find("Fchannel_id"))
        {
            ERROR("Fchannel_id not found");
            jsonOutput["ret"] = "1";
            jsonOutput["errcode"] = tools::CStringTools::Int2String(error_num::CHANNEL_UNKNOWN);
            jsonOutput["errstr"] = "渠道不存在";
            return;
        }
        if (channelInfo.end() == channelInfo.find("Fchannel_name"))
        {
            ERROR("Fchannel_name not found");
            jsonOutput["ret"] = "1";
            jsonOutput["errcode"] = tools::CStringTools::Int2String(error_num::CHANNEL_UNKNOWN);
            jsonOutput["errstr"] = "渠道不存在";
            return;
        }

        Json::Value jsonData;
        jsonData["uid"] = strUserID;
        jsonData["userkey"] = strUserKey;
        jsonData["cid"] = channelInfo["Fchannel_id"];
        jsonData["cname"] = channelInfo["Fchannel_name"];

        jsonOutput["errcode"] = "0";
        jsonOutput["errstr"] = "";
        jsonOutput["data"] = jsonData;
        return;
    }
    else
    {
        ERROR("unlogin");
        jsonOutput["ret"] = "1";
        jsonOutput["errcode"] = tools::CStringTools::Int2String(error_num::USER_UNLOGIN);
        jsonOutput["errstr"] = "未登录";
        return;
    }

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
                  << "request url       is : " << oRequest.strScriptFileName << "\n"
                  << "channel           is : " << oRequest.strChannel << "\n"
                  << "name              is : " << oRequest.strName << "\n"
                  << "psw               is : " << oRequest.strPsw << "";
        DEBUG(ssRequest.str());

        if (!parseResult)
        {
            ERROR("invalid request");
            jsonOutput["ret"] = "1";
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

