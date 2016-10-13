//======================================================================
//
//        Copyright (C) 2014 回收宝
//        All rights reserved
//
//        filename : api-get_channel.cpp
//        description : 获取渠道信息的CGI入口
//
//        created by 叶飞 at  2014-08-01 10:00:00
//        modified by XX at  
//
//======================================================================

#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <fcgi_stdio.h>
#include <json/json.h>
#include "configure.h"
#include "error_num.h"
#include "string_tools.h"

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
    string strChannelID;
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
        if (mapRequest.end() == mapRequest.find("cid"))
        {
            ERROR("invalid request");
            return false;
        }
        oRequest.strChannelID = mapRequest["cid"];
    }

    return true;
}

static void HandleCgi(const CRequest& oRequest, Json::Value& jsonOutput)
{
    string strCGIName = "/api/get_channel";
    if (strCGIName != oRequest.strScriptFileName)
    {
        ERROR("invalid request");
        jsonOutput["ret"] = "1";
        return;
    }

    do
    {
        map<string, string> channelInfo;
        if (logic::SelectChannelTable(::atoi(oRequest.strChannelID.c_str()), channelInfo))
        {
            if (channelInfo.end() == channelInfo.find("Fchannel_name"))
            {
                ERROR("Fchannel_name not found");
                break;
            }
            if (channelInfo.end() == channelInfo.find("Fchannel_logo"))
            {
                ERROR("Fchannel_logo not found");
                break;
            }

            jsonOutput["ret"] = "0";
            jsonOutput["errcode"] = "0";
            jsonOutput["errstr"] = "";

            Json::Value jsonData;
            jsonData["name"] = channelInfo["Fchannel_name"];
            jsonData["logo"] = channelInfo["Fchannel_logo"];
            jsonOutput["data"] = jsonData;

            string strAccessControl = "Access-Control-Allow-Origin: " + CDN_BASE_RAW + "\r\n";
            FCGI_printf(strAccessControl.c_str());
            FCGI_printf("Content-type:text/html\r\n\r\n");
            return;
        }
    }while(0);

    jsonOutput["ret"] = "1";
    jsonOutput["errcode"] = tools::CStringTools::Int2String(error_num::DB_CHANNEL);
    jsonOutput["errstr"] = "获取渠道信息失败";
    FCGI_printf("Content-type:text/html\r\n\r\n");

    return;
}

int main()
{
    while(FCGI_Accept() >= 0)
    {
        //FCGI_printf("Content-type:text/html\r\n\r\n");

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
                  << "request cid       is : " << oRequest.strChannelID << "";
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

