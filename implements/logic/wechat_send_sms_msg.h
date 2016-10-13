//======================================================================
//
//        Copyright (C) 2014 回收宝
//        All rights reserved
//
//        filename : wechat_send_sms_msg.h
//        description : 微信公共平台发送短消息
//
//        created by 张世柏 at  2014-11-20 14:00:00
//        modified by XX at
//
//======================================================================
#ifndef WECHAT_SEND_SMS_MSG_H
#define WECHAT_SEND_SMS_MSG_H 

#include <string>
#include <map>

namespace wechat
{

class CSendMsg
{
public:
    static bool SendMsg(unsigned int iMsgID, const std::string& strPhoneNum, const std::string& strMsgInfo);
};

}

#endif /*WECHAT_SEND_SMS_MSG_H*/