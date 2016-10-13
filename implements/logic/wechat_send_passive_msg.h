//======================================================================
//
//        Copyright (C) 2014 回收宝
//        All rights reserved
//
//        filename : wechat_send_passive_msg.h
//        description : 验证Token及给用户发送被动消息
//
//        created by 张世柏 at  2014-10-24 10:21:00
//        modified by XX at
//
//======================================================================
#ifndef WECHAT_SEND_PASSIVE_MSG_H
#define WECHAT_SEND_PASSIVE_MSG_H

#include <string>
#include <vector>
#include <map>

namespace wechat
{

class CSendPassiveMsg
{
public:
    bool GetUrlParameter(const std::string& strGetData, const std::string &strHttpMethod);
    bool GetXMLContent(const std::string& strXML);
    std::string GenerateXMLPackage(std::vector<std::map<std::string, std::string> >& vecMsgContent);
    bool IsFromWechatServer(const std::string& strToken);
    std::string& GetEchostr() { return strEchostr; }
    std::string& GetMsgType() { return strMsgType; }
    std::string& GetEvent() { return strEvent; }
    std::string& GetEventKey() { return strEventKey2; }
private:
    std::string strSignature; //微信加密签名
    std::string strTimeStamp; //时间戳
    std::string strNonce; //随机数
    std::string strEchostr; //随机字符串

    std::string strToUserName; //开发者微信号
    std::string strFromUserName; //发送方账号(OpenID)
    std::string strCreateTime; // 创建时间(整形)
    std::string strMsgType; // 消息类型
    std::string strContent; //文本消息内容
    std::string strMsgId; //消息id，64位整数字
    std::string strEvent; //事件类型
    std::string strEventKey; //自定义的事件的key值
    std::string strEventKey2; //自定义的事件的key值

    std::string GetSha1Str(const std::string& strOriginal);
};

void GetPostData(char* cData, unsigned int iLength);

}

#endif /*WECHAT_SEND_PASSIVE_MSG_H*/

