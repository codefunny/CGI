//======================================================================
//
//        Copyright (C) 2014 回收宝
//        All rights reserved
//
//        filename : wechat_template_msg.h
//        description : 微信公共平台发送模板信息
//
//        created by 张世柏 at  2014-10-20 10:00:00
//        modified by XX at
//
//======================================================================
#ifndef _WECHAT_TEMPLATE_MSG_H_
#define _WECHAT_TEMPLATE_MSG_H_


#include <string>
#include <map>
#include <json/json.h>
#include "http.h"

namespace wechat
{

class CTemplateMsg
{
public:
    bool Send(std::map<std::string, std::string>& mapMsgInfo, std::map<std::string, std::map<std::string, std::string> >& mapParamInfo);
private:
    std::string strAccessToken;
    tools::CCurlWrapper curlWrapper;

    bool GetAccessToken();
    std::string PackageJsonString(std::map<std::string, std::string>& mapMsgInfo, std::map<std::string, std::map<std::string, std::string> >& mapParamInfo);
    bool SendTemplateMsg(const std::string& strMsg);
};

}

#endif /*_WECHAT_TEMPLATE_MSG_H_*/

