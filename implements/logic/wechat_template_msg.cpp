#include "wechat_template_msg.h"
#include <stdio.h>
#include <sstream>
#include <json/json.h>
#include "configure.h"
#include "protocol.h"
#include "string_tools.h"

using namespace std;

/**
  *brief: send template message
  *@param1 mapMsgInfo:包含发送模板消息必须的信息，比如：touser,template_id,可选的有：url,topcolor
  *@param2 mapParamInfo:map的第一项是模板消息中的参数名，第二项是对应第一项的value,color等信息
  *return: when send success, return true; otherwish, return false;
  */
bool wechat::CTemplateMsg::Send(std::map<std::string, std::string>& mapMsgInfo, std::map<std::string, std::map<std::string, std::string> >& mapParamInfo)
{
    DEBUG("============ Begin to send message ===============");
    if (!wechat::CTemplateMsg::GetAccessToken())
    {
        ERROR("Get access_token failed.");
        return false;
    }
    else
    {
        DEBUG("Get access_token successed!");
    }
    string strResult = wechat::CTemplateMsg::PackageJsonString(mapMsgInfo, mapParamInfo);
    DEBUG("Message content : [" + strResult + "]");

    if ("" == strResult)
    {
        ERROR("Make Json string failed.");
        return false;
    }

    if (!wechat::CTemplateMsg::SendTemplateMsg(strResult))
    {
        ERROR("Send Template message failed!\n");
        return false;
    }
    else
    {
        DEBUG("Send Template message successed!");
    }
    DEBUG("============ Send message over ===============");
    return true;
}

/**
  *@brief get access_token from wechat server
  *@return success return true else return false
  */
bool wechat::CTemplateMsg::GetAccessToken()
{
    DEBUG("============ Begin to GetAccessToken ===============");
    string strReply;
    string strUrl = string("https://api.weixin.qq.com/cgi-bin/token?grant_type=client_credential") +
                    "&appid=" + WECHAT_APPID +
                    "&secret=" + WECHAT_SECRET;

    //向微信服务器发送Get请求
    DEBUG("strUrl : " + strUrl);
    curlWrapper.GetFile(strUrl, strReply);
    DEBUG("strReply : " + strReply);

    //解析微信服务器的回包
    Json::Value jsonOutput;
    Json::Reader reader;
    if (!reader.parse(strReply, jsonOutput))
    {
        ERROR("protocol unmatch[" + strReply + "]");
        return false;
    }
    if (!jsonOutput["access_token"].isString())
    {
        ERROR("protocol unmatch, request[" + strReply + "], key[\"access_token\"].");
        return false;
    }
    strAccessToken = jsonOutput["access_token"].asString();
    DEBUG("============= End to GetAccessToken ===============");
    return true;
}

/**
  *@brief packaging data as a Json string
  *@param1 mapMsgInfo:包含发送模板消息必须的信息，比如：touser,template_id,可选的有：url,topcolor
  *@param2 mapParamInfo:map的第一项是模板消息中的参数名，第二项是对应第一项的value,color等信息
  *@return a Json string
  */
string wechat::CTemplateMsg::PackageJsonString(map<string, string> &mapMsgInfo, map<string, map<string, string> > &mapParamInfo)
{
    DEBUG("========= Begin to PackageJsonString ========");
    Json::Value jsonOutput;
    string strJsonString;
    map<string, string>::iterator mapMsgInfoIt;
    if (mapMsgInfo.end() == mapMsgInfo.find("touser") || mapMsgInfo.end() == mapMsgInfo.find("template_id"))
    {
        ERROR("There must be have touser and template_id as parameter!");
        return strJsonString;
    }
    jsonOutput["touser"] = Json::Value(mapMsgInfo["touser"]);
    jsonOutput["template_id"] = Json::Value(mapMsgInfo["template_id"]);
    if (mapMsgInfo.end() != mapMsgInfo.find("url"))
    {
        //微信用户点击模板信息时所跳转的页面
        jsonOutput["url"] = Json::Value(mapMsgInfo["url"]);
    }
    if (mapMsgInfo.end() != mapMsgInfo.find("topcolor"))
    {
        jsonOutput["topcolor"] = Json::Value(mapMsgInfo["topcolor"]);
    }

    Json::Value jsonData;

    //将模板信息的参数内容加入Json中
    map<string, map<string, string> >::iterator itMapParamInfo;
    for (itMapParamInfo = mapParamInfo.begin(); itMapParamInfo != mapParamInfo.end(); ++itMapParamInfo)
    {
        //将每一个参数都生成Json对象
        Json::Value jsonDataItem;
        map<string, string>::iterator itMapItemInfo;
        for (itMapItemInfo = itMapParamInfo->second.begin(); itMapItemInfo != itMapParamInfo->second.end(); ++itMapItemInfo)
        {
            //为参数对应的Json对象赋值
            jsonDataItem[itMapItemInfo->first] = Json::Value(itMapItemInfo->second);
        }
        //将参数的Json对象加入jsonData对象中
        jsonData[itMapParamInfo->first] = jsonDataItem;
    }

    //将jsonData对象加入jsonOutput对象中
    jsonOutput["data"] = jsonData;

    Json::StyledWriter  jsonStyledWriter;
    stringstream ssResult;
    ssResult << jsonStyledWriter.write(jsonOutput);
    strJsonString = ssResult.str();

    DEBUG("========= End to PackageJsonString ========");
    return strJsonString;
}

/**
  *@brief send template message
  *@param1 strMsg:a Json string
  *@return success return true else return false
  */
bool wechat::CTemplateMsg::SendTemplateMsg(const string& strMsg)
{
    DEBUG("========== Begin to SendTemplateMsg ===============");
    string strReply;
    string strUrl = string("https://api.weixin.qq.com/cgi-bin/message/template/send?") +
                    "access_token=" + strAccessToken;
    DEBUG("strUrl is : " + strUrl);

    //modified by Zhang Shibo at 2014-11-14 22:32:00
    //modify brief: Add the judge with return value.
    //向微信服务器发送post请求，数据内容为模板消息的Json字符串
    int iRet = curlWrapper.PostFile(strUrl, strMsg, strReply);
    if (0 != iRet)
    {
        ERROR("Curl send a post request, and the result value is : " + tools::CStringTools::Int2String(iRet));
        return false;
    }
    else
    {
        DEBUG("Curl send post request successed!");
    }
    DEBUG("strReply is :" + strReply);
    //end modify

    //解析微信服务器的回包
    int iErrCode;
    if (!protocol::UnPackingByJson(strReply, "errcode", iErrCode))
    {
        ERROR("Parse errcode wrong!");
        return false;
    }

    //errcode这一项为0表示消息发送成功
    if (0 != iErrCode)
    {
        ERROR("Template message send failed, ErrCode is : [" + tools::CStringTools::Int2String(iErrCode)+ "]");
        return false;
    }
    else
    {
        DEBUG("Template message send successed!");
    }
    DEBUG("=========== End to SendTemplateMsg ===============");
    return true;
}

