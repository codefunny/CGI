//======================================================================
//
//        Copyright (C) 2014 回收宝
//        All rights reserved
//
//        filename : api-shipments.cpp
//        description : 发货的CGI入口
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
    string strPostData;
    string strCookie;
    string strUserID;
    string strUserKey;

    unsigned int iLogisticsType;
    std::string strLogisticsID;
    std::string strOrderIDList;
    logic::CSendGoodsInfo oSendGoodsInfo;
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
        if (mapRequest.end() == mapRequest.find("ltype"))
        {
            ERROR("invalid request");
            return false;
        }
        if (mapRequest.end() == mapRequest.find("lid"))
        {
            ERROR("invalid request");
            return false;
        }
        if (mapRequest.end() == mapRequest.find("ltel"))
        {
            ERROR("invalid request");
            return false;
        }
        if (mapRequest.end() == mapRequest.find("orders"))
        {
            ERROR("invalid request");
            return false;
        }
        oRequest.iLogisticsType = ::atoi(mapRequest["ltype"].c_str());
        oRequest.strLogisticsID = mapRequest["lid"];
        oRequest.strOrderIDList = mapRequest["orders"];
        oRequest.oSendGoodsInfo.strSenderPhone = mapRequest["ltel"];
    }

    return true;
}

static void HandleCgi(const CRequest& oRequest, Json::Value& jsonOutput)
{
    string strCGIName = "/api/shipments";
    if (strCGIName != oRequest.strScriptFileName)
    {
        ERROR("invalid request");
        jsonOutput["ret"] = "1";
        return;
    }

    std::vector<unsigned int> vecOrderSentIDList;
    unsigned int iSendGoodsResult = logic::SendGoods(oRequest.strUserID, oRequest.strUserKey, oRequest.iLogisticsType, oRequest.strLogisticsID, tools::CStringTools::Split2Uint(oRequest.strOrderIDList, "#"), oRequest.oSendGoodsInfo, vecOrderSentIDList);
    if (error_num::SUCCEED == iSendGoodsResult)
    {
        std::string strData;
        size_t maxSize = vecOrderSentIDList.size();
        for (size_t index = 0; index < maxSize; index++)
        {
            if (index == maxSize - 1)
            {
                strData = strData + tools::CStringTools::Int2String(vecOrderSentIDList[index]);
            }
            else
            {
                strData = strData + tools::CStringTools::Int2String(vecOrderSentIDList[index]) + "#";
            }
        }

        jsonOutput["errcode"] = "0";
        jsonOutput["errstr"] = "";
        jsonOutput["data"] = strData;
    }
    else
    {
        jsonOutput["ret"] = "1";
        jsonOutput["errcode"] = tools::CStringTools::Int2String(iSendGoodsResult);
        jsonOutput["errstr"] = "发货失败";
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
                  << "request url       is : " << oRequest.strScriptFileName << "\n"
                  << "request cookie    is : " << oRequest.strCookie << "\n"
                  << "uid               is : " << oRequest.strUserID << "\n"
                  << "userkey           is : " << oRequest.strUserKey << "\n"
                  << "ltype             is : " << oRequest.iLogisticsType << "\n"
                  << "lid               is : " << oRequest.strLogisticsID << "\n"
                  << "orders            is : " << oRequest.strOrderIDList << "\n"
                  << "ltel              is : " << oRequest.oSendGoodsInfo.strSenderPhone<< "";
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

