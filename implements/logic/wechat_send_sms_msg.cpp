#include "wechat_send_sms_msg.h"
#include <iostream>
#include <sstream>
#include <json/json.h>
#include "http.h"
#include "string_tools.h"
#include "configure.h"

using namespace std;

unsigned char ToHex(unsigned char x);
string UrlEncode(const string& str);

bool wechat::CSendMsg::SendMsg(unsigned int iMsgID, const string& strPhoneNum, const string& strMsgInfo)
{
    string strUrl = string("http://v.juhe.cn/sms/send?dtype=json") +
                    "&mobile=" + strPhoneNum +
                    "&tpl_id=" + tools::CStringTools::Int2String(iMsgID) +
                    "&tpl_value=" + UrlEncode("#ordermsg#=" + UrlEncode(strMsgInfo)) +
                    "&key=ae31592297a4ebd5bc2d79bd88e0b01a";
    string strReply;
    tools::CCurlWrapper curlWrapper;
    DEBUG("strUrl : " + strUrl);
    int iRet = curlWrapper.GetFile(strUrl, strReply);
    if (0 != iRet)
    {
        ERROR("Curl send a get request, and the result value is : " + tools::CStringTools::Int2String(iRet));
        return false;
    }
    else
    {
        DEBUG("Curl send get request successed!");
    }
    DEBUG("reply : " + strReply);
    DEBUG("strReply : " + strReply);

    //解析Json字符串
    Json::Value jsonOutput;
    Json::Reader reader;
    if (!reader.parse(strReply, jsonOutput))
    {
        WARN("Protocol unmatch [" + strReply + "]");
        return false;
    }
    if (jsonOutput["error_code"].isNull())
    {
        WARN("Hasn't reslut code!");
        return false;
    }
    string strErrorCode = tools::CStringTools::Int2String(jsonOutput["error_code"].asInt());
    DEBUG("error_code is : " + strErrorCode);

    if (jsonOutput["reason"].isNull())
    {
        WARN("Hasn't result data!");
        return false;
    }
    string strResult = jsonOutput["reason"].asString();
    DEBUG("Msg send result is : " + strResult);

    if (0 == jsonOutput["error_code"].asInt())
    {
        DEBUG("Success send a message to user [" + strPhoneNum + "]!");
    }
    else
    {
        WARN("Send message to user [" + strPhoneNum + "] failed! Reason is " + strResult);
        return false;
    }
    return true;
}

unsigned char ToHex(unsigned char x)
{
    return  x > 9 ? x + 55 : x + 48;
}

std::string UrlEncode(const std::string& str)
{
    std::string strTemp = "";
    size_t length = str.length();
    for (size_t i = 0; i < length; i++)
    {
        if (isalnum((unsigned char)str[i]) ||
            (str[i] == '-') ||
            (str[i] == '_') ||
            (str[i] == '.') ||
            (str[i] == '~'))
            strTemp += str[i];
        else if (str[i] == ' ')
            strTemp += "+";
        else
        {
            strTemp += '%';
            strTemp += ToHex((unsigned char)str[i] >> 4);
            strTemp += ToHex((unsigned char)str[i] % 16);
        }
    }
    return strTemp;
}


