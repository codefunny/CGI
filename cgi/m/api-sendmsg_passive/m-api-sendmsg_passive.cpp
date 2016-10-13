//======================================================================
//
//        Copyright (C) 2014 回收宝
//        All rights reserved
//
//        filename : m-api-sendmsg_passive.cpp
//        description : 验证Token及给用户发送被动消息的CGI
//
//        created by 张世柏 at  2014-10-13 10:00:00
//        modified by XX at
//
//======================================================================

#include <stdio.h>
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
#include "login.h"
#include "channel_table.h"
#include "user_table.h"
#include "wechat_login.h"
#include "time_tools.h"
#include "http.h"
#include "protocol.h"
#include "Markup.h"
#include "wechat_send_passive_msg.h"

using namespace std;

class CRequest
{
    public:
    CRequest() : iContentLength(0){}
    string strGetData;
    string strPostData;
    string strMethod;
    unsigned int iContentLength;
    string strScriptFileName;
    string strCookie;
};

static bool ParseCgi(CRequest& oRequest)
{
    DEBUG("======= Begin to parse cgi ======");
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
        DEBUG("CONTENT_LENGTH : " + string(getenv("CONTENT_LENGTH")));
    }

    if (getenv("SCRIPT_FILENAME"))
    {
        oRequest.strScriptFileName = getenv("SCRIPT_FILENAME");
    }

    //如果是POST请求，那么还需要读取XML数据包
    if (oRequest.strMethod == "POST")
    {
        char* data = (char*)::malloc(oRequest.iContentLength + 1);
        ::memset(data, 0, oRequest.iContentLength + 1);
        wechat::GetPostData(data, oRequest.iContentLength);
        oRequest.strPostData = string(data, oRequest.iContentLength + 1);
        DEBUG("strPostData : " + oRequest.strPostData);
        ::free(data);
    }
    DEBUG("======= End to parse cgi ======");
    return true;
}

static void HandleCgi(const CRequest& oRequest)
{
    string strCGIName = "/sendmsg_passive";
    if (strCGIName != oRequest.strScriptFileName)
    {
        ERROR("invalid request");
        return;
    }

    wechat::CSendPassiveMsg cSendPassiveMsg;
    //第一步；获取url后面的参数
    if (!cSendPassiveMsg.GetUrlParameter(oRequest.strGetData, oRequest.strMethod))
    {
        ERROR("Parse url parameter failed!");
        return;
    }
    else
    {
        DEBUG("Parse url parameter success.");
    }

    //第二步：判断是否是从微信服务器发过来的
    if (!cSendPassiveMsg.IsFromWechatServer("wechat"))
    {
        ERROR("Is not come from wechat server!");
        return;
    }
    else
    {
        DEBUG("Verify success, message is sended from wechat server.");
    }

    //第三步：如果是post请求则执行该步，解析XML包中的数据，否则跳到第四步
    if ("POST" == oRequest.strMethod)
    {
        if (!cSendPassiveMsg.GetXMLContent(oRequest.strPostData))
        {
            ERROR("Parse XML failed!");
            return;
        }
        else
        {
            DEBUG("Parse XML successed!");
        }
    }

    //第四步：给微信服务器回发XML数据包
    if ("GET" == oRequest.strMethod)
    {
        //回复微信服务器的get请求
        DEBUG_AND_RETURN(cSendPassiveMsg.GetEchostr().c_str());
    }
    else if ("POST" == oRequest.strMethod)
    {
        string strSendXML;
        map<string, string> mapMsgContent;
        vector<map<string, string> > vecMsgContent;
        if ("event" == cSendPassiveMsg.GetMsgType())
        {
            if ("subscribe" == cSendPassiveMsg.GetEvent())
            {
                string strContent = "Hi，感谢关注回收宝!\n我们是能吃苦的80后创业团队\n我们想用心做好手机回收事业\n我们承诺：\n>顺丰到付\n>收货后24小时内付款\n>只要您如实评估，我们绝不砍价";
                mapMsgContent.insert(make_pair<string, string>("Content", strContent));
                vecMsgContent.push_back(mapMsgContent);
                strSendXML= cSendPassiveMsg.GenerateXMLPackage(vecMsgContent);
            }
            else if ("CLICK" == cSendPassiveMsg.GetEvent())
            {
                if ("ad_old_for_new_click" == cSendPassiveMsg.GetEventKey())
                {
                    string strTitle = "【以旧换新】轻松入手iPhone6，你GET到了吗？";
                    string strDescription = "    近日，回收宝携手广东移动创新推出“以旧换新”活动。用户将旧手机让营业厅回收，即可获得折抵款用于购买新手机。";
                    string strPicUrl = "http://cdn.huishoubao.com.cn/m/img/wechat_operation/4.jpg";
                    string strUrl = "http://mp.weixin.qq.com/s?__biz=MzA3OTk5MTcxOQ==&mid=201476637&idx=1&sn=58e3e6d620656cd1e41b837533f4182d#rd";
                    mapMsgContent.insert(make_pair<string, string>("Title", strTitle));
                    mapMsgContent.insert(make_pair<string, string>("Description", strDescription));
                    mapMsgContent.insert(make_pair<string, string>("PicUrl", strPicUrl));
                    mapMsgContent.insert(make_pair<string, string>("Url", strUrl));
                    vecMsgContent.push_back(mapMsgContent);
                    strSendXML= cSendPassiveMsg.GenerateXMLPackage(vecMsgContent);
                }
                else
                {
                    //其他按键触发事件，待以后做
                    strSendXML= cSendPassiveMsg.GenerateXMLPackage(vecMsgContent);
                }
            }
            else if ("VIEW" == cSendPassiveMsg.GetEvent())
            {
                //
                strSendXML= cSendPassiveMsg.GenerateXMLPackage(vecMsgContent);
            }
            else
            {
                //其他的事件类型，待以后做
                strSendXML= cSendPassiveMsg.GenerateXMLPackage(vecMsgContent);
            }
        }
        else if ("text" == cSendPassiveMsg.GetMsgType())
        {
            //用户向公众号发送文本消息时的处理
            strSendXML= cSendPassiveMsg.GenerateXMLPackage(vecMsgContent);
        }
        else
        {
            //其他类型消息的处理，待以后做
            strSendXML= cSendPassiveMsg.GenerateXMLPackage(vecMsgContent);
        }

        //回复微信服务器的post请求
        DEBUG_AND_RETURN(strSendXML.c_str());
    }
}

int main()
{
    while(FCGI_Accept() >= 0)
    {
        FCGI_printf("Content-type:text/html\r\n\r\n");

        log::CLog::Initialize("");
        CRequest oRequest;
        bool parseResult = ParseCgi(oRequest);

        stringstream ssRequest;
        ssRequest << "==========CGI START==========\n"
        << "request method    is : " << oRequest.strMethod << "\n"
        << "content length    is : " << oRequest.iContentLength << "\n"
        << "request get data  is : " << oRequest.strGetData << "\n"
        << "request post data is : " << oRequest.strPostData << "\n"
        << "request post data length is : " << tools::CStringTools::Int2String(oRequest.strPostData.length()) << "\n"
        << "request url       is : " << oRequest.strScriptFileName << "\n";
        DEBUG(ssRequest.str());

        if (!parseResult)
        {
            ERROR("invalid request");
        }
        else
        {
            HandleCgi(oRequest);
        }
        DEBUG("===========CGI END===========");
    }

    return 0;
}

