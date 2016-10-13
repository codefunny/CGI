//======================================================================
//
//        Copyright (C) 2014 回收宝
//        All rights reserved
//
//        filename : m-api-evaluate.cpp
//        description : 估价的CGI入口
//
//        created by 叶飞 at  2014-10-16 15:53:00
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

#include "price.h"

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
    string strPid;
    string strChannel;
    string strSelects;
    string strCreategood;
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
        if (mapRequest.end() == mapRequest.find("channel"))
        {
            ERROR("invalid request");
            return false;
        }
        oRequest.strUserID = mapRequest["uid"];
        oRequest.strUserKey = mapRequest["userkey"];
        oRequest.strChannel = mapRequest["channel"];
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
        if (mapRequest.end() == mapRequest.find("creategood"))
        {
            ERROR("invalid request");
            return false;
        }
        oRequest.strPid = mapRequest["pid"];
        oRequest.strSelects = mapRequest["selects"];
        oRequest.strCreategood = mapRequest["creategood"];
        DEBUG("pid is : " + oRequest.strPid + ", selects is : " + oRequest.strSelects + ", creategood is : " + oRequest.strCreategood);
    }
    else
    {
        ERROR("Hasn't post data!");
        return false;
    }

    return true;
}

static void HandleCgi(const CRequest& oRequest, Json::Value& jsonOutput)
{
    string strCGIName = "/api/evaluate";
    if (strCGIName != oRequest.strScriptFileName)
    {
        ERROR("invalid request");
        jsonOutput["ret"] = "1";
        jsonOutput["errstr"] = "请求错误";
        return;
    }

    unsigned int iGoodsID = 0;
    unsigned int iQuotation = 0;
    bool bIsCreateGoodsID = oRequest.strCreategood == "1" ? true : false;
    unsigned int iQuotationResult = logic::GetQuotation(::atoi(oRequest.strPid.c_str()), ::atoi(oRequest.strChannel.c_str()), oRequest.strUserID, oRequest.strUserKey, oRequest.strSelects, bIsCreateGoodsID, iGoodsID, iQuotation);
    if (error_num::SUCCEED == iQuotationResult)
    {
        Json::Value jsonGoodsinfo;
        if (bIsCreateGoodsID)
        {
            jsonGoodsinfo["gid"] = tools::CStringTools::Int2String(iGoodsID);
        }
        Json::Value jsonData;
        jsonData["quotation"] = iQuotation;
        jsonData["goodsinfo"] = jsonGoodsinfo;

        jsonOutput["ret"] = "0";
        jsonOutput["errcode"] = "0";
        jsonOutput["errstr"] = "";
        jsonOutput["data"] = jsonData;
    }
    else
    {
        jsonOutput["ret"] = "1";
        jsonOutput["errcode"] = tools::CStringTools::Int2String(iQuotationResult);
        jsonOutput["errstr"] = "获取报价失败";
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
                  << "pid               is : " << oRequest.strPid << "\n"
                  << "channel           is : " << oRequest.strChannel << "\n"
                  << "selects           is : " << oRequest.strSelects << "\n"
                  << "creategood        is : " << oRequest.strCreategood << "";
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


