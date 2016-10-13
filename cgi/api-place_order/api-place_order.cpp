//======================================================================
//
//        Copyright (C) 2014 回收宝
//        All rights reserved
//
//        filename : api-place_order.cpp
//        description : 下单的CGI入口
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
    string strGid;
    string strChannel;
    string strPayways;
    string strTradeways;
    string strIMEI;
    string strRemarks;
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
        //channel以post请求的为准
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
        if (mapRequest.end() == mapRequest.find("gid"))
        {
            ERROR("invalid request");
            return false;
        }
        if (mapRequest.end() == mapRequest.find("channel"))
        {
            ERROR("invalid request");
            return false;
        }
        if (mapRequest.end() == mapRequest.find("payways"))
        {
            ERROR("invalid request");
            return false;
        }
        if (mapRequest.end() == mapRequest.find("tradeways"))
        {
            ERROR("invalid request");
            return false;
        }
        if (mapRequest.end() == mapRequest.find("IMEI"))
        {
            ERROR("invalid request");
            return false;
        }
        oRequest.strGid = mapRequest["gid"];
        oRequest.strChannel = mapRequest["channel"];
        oRequest.strPayways = mapRequest["payways"];
        oRequest.strTradeways = mapRequest["tradeways"];
        oRequest.strIMEI = mapRequest["IMEI"];
        if (mapRequest.end() != mapRequest.find("remarks"))
        {
            oRequest.strRemarks = mapRequest["remarks"];
        }
        DEBUG("gid is : " + oRequest.strGid + ", channel is : " + oRequest.strChannel + ", payways is : " + oRequest.strPayways + ", tradeways is : " + oRequest.strTradeways +
            ", IMEI is : " + oRequest.strIMEI + ", remarks is : " + oRequest.strRemarks);
    }

    return true;
}

static void HandleCgi(const CRequest& oRequest, Json::Value& jsonOutput)
{
    string strCGIName = "/api/place_order";
    if (strCGIName != oRequest.strScriptFileName)
    {
        ERROR("invalid request");
        jsonOutput["ret"] = "1";
        return;
    }

    unsigned int iOrderID = 0;
    unsigned int iPlaceOrderResult = logic::PlaceOrder(oRequest.strUserID, oRequest.strUserKey, ::atoi(oRequest.strGid.c_str()), ::atoi(oRequest.strChannel.c_str()), ::atoi(oRequest.strPayways.c_str()), ::atoi(oRequest.strTradeways.c_str()), oRequest.strIMEI, oRequest.strRemarks, iOrderID);
    if (error_num::SUCCEED == iPlaceOrderResult)
    {
        Json::Value jsonData;
        jsonData["orderid"] = tools::CStringTools::Int2String(iOrderID);

        jsonOutput["errcode"] = "0";
        jsonOutput["errstr"] = "";
        jsonOutput["data"] = jsonData;
    }
    else
    {
        jsonOutput["ret"] = "1";
        jsonOutput["errcode"] = tools::CStringTools::Int2String(iPlaceOrderResult);
        jsonOutput["errstr"] = "下单失败";
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
                  << "uid               is : " << oRequest.strUserID << "\n"
                  << "userkey           is : " << oRequest.strUserKey << "\n"
                  << "request url       is : " << oRequest.strScriptFileName << "\n"
                  << "gid               is : " << oRequest.strGid << "\n"
                  << "channel           is : " << oRequest.strChannel << "\n"
                  << "payways           is : " << oRequest.strPayways << "\n"
                  << "tradeways         is : " << oRequest.strTradeways << "\n"
                  << "IMEI              is : " << oRequest.strIMEI << "\n"
                  << "remarks           is : " << oRequest.strRemarks << "";
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


