#include "wechat_send_passive_msg.h"
#include <vector>
#include <algorithm>
#include <inttypes.h>
#include <openssl/sha.h>
#include <json/json.h>
#include "protocol.h"
#include "configure.h"
#include "error_num.h"
#include "Markup.h"
#include "string_tools.h"
#include "http.h"


using namespace std;

/**
  *@brief:get parameter in url
  *@param1:strGetData: a string after ? in url.
  *@param2:strHttpMethod: http request method
  *@return: return true when parse success, otherwish return false
  */
bool wechat::CSendPassiveMsg::GetUrlParameter(const string& strGetData, const string& strHttpMethod)
{
    DEBUG("========== GetUrlParameter BEGIN ============");
    map<string, string> mapRequest = tools::CStringTools::SplitTwice2String(strGetData, "&", "=");
    //判断signature参数是否存在
    if (mapRequest.end() == mapRequest.find("signature"))
    {
        ERROR("invalid request");
        return false;
    }
    else
    {
        DEBUG("signature=" + mapRequest["signature"]);
        strSignature = mapRequest["signature"];
    }

    //判断timestamp参数是否存在
    if (mapRequest.end() == mapRequest.find("timestamp"))
    {
        ERROR("invalid request");
        return false;
    }
    else
    {
        DEBUG("timestamp=" + mapRequest["timestamp"]);
        strTimeStamp = mapRequest["timestamp"];
    }

    //判断nonce参数是否存在
    if (mapRequest.end() == mapRequest.find("nonce"))
    {
        ERROR("invalid request");
        return false;
    }
    else
    {
        DEBUG("nonce=" + mapRequest["nonce"]);
        strNonce = mapRequest["nonce"];
    }

    //判断echostr参数是否存在，只在GET请求中才有
    if (strHttpMethod == "GET")
    {
        if (mapRequest.end() == mapRequest.find("echostr"))
        {
            ERROR("invalid request");
            return false;
        }
        else
        {
            DEBUG("echostr=" + mapRequest["echostr"]);
            strEchostr = mapRequest["echostr"];
        }
    }
    else
    {
        //微信服务器的POST请求中没有echostr参数
    }
    DEBUG("========== GetUrlParameter END ============");
    return true;
}

/**
  *@brief:get message in XML
  *@param:strXML:post request's data
  *@return:return true when parse success, otherwish return false
  */
bool wechat::CSendPassiveMsg::GetXMLContent(const string& strXML)
{
    DEBUG("========== GetXMLContent BEGIN ============");
    CMarkup xml;
    xml.SetDoc(strXML);

    if (!xml.FindChildElem("ToUserName"))
    {
        ERROR("protocol unmatch, request[" + strXML + "], key[ToUserName]");
        return false;
    }
    strToUserName = xml.GetChildData();

    if (!xml.FindChildElem("FromUserName"))
    {
        ERROR("protocol unmatch, request[" + strXML + "], key[FromUserName]");
        return false;
    }
    strFromUserName = xml.GetChildData();

    if (!xml.FindChildElem("CreateTime"))
    {
        ERROR("protocol unmatch, request[" + strXML + "], key[CreateTime]");
        return false;
    }
    strCreateTime = xml.GetChildData();

    if (!xml.FindChildElem("MsgType"))
    {
        ERROR("protocol unmatch, request[" + strXML + "], key[MsgType]");
        return false;
    }
    strMsgType = xml.GetChildData();
    if (strMsgType == "text")
    {
        //用户发送文本消息事件
        if (!xml.FindChildElem("Content"))
        {
            ERROR("protocol unmatch, request[" + strXML + "], key[Content]");
            return false;
        }
        strContent = xml.GetChildData();

        if (!xml.FindChildElem("MsgId"))
        {
            ERROR("protocol unmatch, request[" + strXML + "], key[MsgId]");
            return false;
        }
        strMsgId = xml.GetChildData();
    }
    else if (strMsgType == "event")
    {
        //用户关注事件Event
        if (!xml.FindChildElem("Event"))
        {
            ERROR("protocol unmatch, request[" + strXML + "], key[Event]");
            return false;
        }
        strEvent = xml.GetChildData();
        if (xml.FindChildElem("EventKey"))
        {
            DEBUG(xml.GetChildData());
            string strTmp = xml.GetChildData();
            strEventKey2 = strTmp;
            DEBUG(strEventKey2);
        }
    }
    else
    {
        WARN("event is illegall");
    }
    DEBUG("========== GetXMLContent END ============");
    return true;
}

/**
  *@brief:packaging the parameter in mapMsgContent into a XML package
  *@param:mapMsgContent: parameters used to send to wechat server
  *@return:return a formated XML package
  */
string wechat::CSendPassiveMsg::GenerateXMLPackage(vector<map<string, string> >& vecMsgContent)
{
    DEBUG("========== GenerateXMLPackage BEGIN ============");
    string strSendXML;
    CMarkup xmlOutput;
    xmlOutput.AddElem("xml");
    xmlOutput.IntoElem();
    xmlOutput.AddElem("ToUserName", strFromUserName);
    xmlOutput.AddElem("FromUserName", strToUserName);
    xmlOutput.AddElem("CreateTime", strCreateTime);
    if ("event" == strMsgType)
    {
        if ("subscribe" == strEvent)
        {
            xmlOutput.AddElem("MsgType", "text");
            if (1 != vecMsgContent.size())
            {
                //一定要回复一个空字符串，以防微信回复"该公众号暂不能提供服务"
                ERROR("There is no message!");
                return "";
            }
            if (vecMsgContent[0].end() == vecMsgContent[0].find("Content"))
            {
                //一定要回复一个空字符串，以防微信回复"该公众号暂不能提供服务"
                ERROR("There is no content!");
                return "";
            }
            xmlOutput.AddElem("Content", vecMsgContent[0]["Content"]);
        }
        else if ("CLICK" == strEvent)
        {
            if ("ad_old_for_new_click" == strEventKey2)
            {
                xmlOutput.AddElem("MsgType", "news");
                xmlOutput.AddElem("ArticleCount", tools::CStringTools::Int2String(vecMsgContent.size()));
                xmlOutput.AddElem("Articles");
                unsigned int iIndex, iMaxSize = vecMsgContent.size();
                if (0 == iMaxSize)
                {
                    //一定要回复一个空字符串，以防微信回复"该公众号暂不能提供服务"
                    ERROR("There isn't message content.");
                    return "";
                }
                for (iIndex = 0; iIndex != iMaxSize; ++iIndex)
                {
                    xmlOutput.AddChildElem("item");
                    xmlOutput.IntoElem();
                    if (vecMsgContent[iIndex].end() != vecMsgContent[iIndex].find("Title"))
                    {
                        xmlOutput.AddChildElem("Title", vecMsgContent[iIndex]["Title"]);
                    }
                    if (vecMsgContent[iIndex].end() != vecMsgContent[iIndex].find("Description"))
                    {
                        xmlOutput.AddChildElem("Description", vecMsgContent[iIndex]["Description"]);
                    }
                    if (vecMsgContent[iIndex].end() != vecMsgContent[iIndex].find("PicUrl"))
                    {
                        xmlOutput.AddChildElem("PicUrl", vecMsgContent[iIndex]["PicUrl"]);
                    }
                    if (vecMsgContent[iIndex].end() != vecMsgContent[iIndex].find("Url"))
                    {
                        xmlOutput.AddChildElem("Url", vecMsgContent[iIndex]["Url"]);
                    }
                    xmlOutput.OutOfElem();
                }

                xmlOutput.OutOfElem();
                strSendXML = xmlOutput.GetDoc();
            }
            else
            {
                //其他按键触发事件，待以后做，一定要回复一个空字符串，以防微信回复"该公众号暂不能提供服务"
                return "";
            }
        }
        else if ("VIEW" == strEvent)
        {
            DEBUG("User [" + strFromUserName + "] is click the url, do nothing.");
            return "";
        }
        else
        {
            //其他的事件类型，待以后做，一定要回复一个空字符串，以防微信回复"该公众号暂不能提供服务"
            return "";
        }
    }
    else if ("text" == strMsgType)
    {
        //用户向公众号发送文本消息时的处理，一定要回复一个空字符串，以防微信回复"该公众号暂不能提供服务"
        DEBUG("User [" + strFromUserName + "] is send text msg : " + strContent);
        return "";
    }
    else
    {
        //其他类型消息的处理，待以后做，现在一定要回复一个空字符串，以防微信回复"该公众号暂不能提供服务"
        return "";
    }
    xmlOutput.OutOfElem();
    strSendXML = xmlOutput.GetDoc();
    DEBUG("XML Content:" + strSendXML);
    DEBUG("========== GenerateXMLPackage END ============");
    return strSendXML;
}

/**
  *@brief:Thansform a string into a sha1 string.
  *@param: Original string.
  *@return:return the thansformed string
  */
string wechat::CSendPassiveMsg::GetSha1Str(const std::string& strOriginal)
{
    DEBUG("========== GetSha1Str BEGIN ============");
    unsigned char digest[20] = {0};
    ::SHA1((const unsigned char *)strOriginal.c_str(), strOriginal.length(), digest);
    string strResult;
    char sHexStr[32];
    for ( uint32_t i = 0; i < sizeof(digest); i++ )
    {
        snprintf( sHexStr, sizeof(sHexStr), "%x%x", ((int)digest[i] & 0xf0) >> 4, ((int)digest[i] & 0x0f) );
        strResult.append(sHexStr);
    }
    DEBUG("========== GetSha1Str END ============");
    return strResult;
}

/**
  *@brief:Judge whether the request is comes from wechat server
  *@param: a token
  *@return: return true when verify success, otherwish return false
  */
bool wechat::CSendPassiveMsg::IsFromWechatServer(const std::string& strToken)
{
    DEBUG("========== IsFromWechatServer BEGIN ============");
    //验证是否是微信服务器发来的
    //第一步：将参数按字典序排序
    vector<string> svec;
    svec.push_back(strToken);
    svec.push_back(strTimeStamp);
    svec.push_back(strNonce);
    vector<string>::iterator iBegin = svec.begin();
    vector<string>::iterator iEnd = svec.end();
    sort(iBegin, iEnd);

    //第二步：将排好序的字符串进行拼接
    string strVerify;
    for (iBegin = svec.begin(); iBegin != svec.end(); ++iBegin)
    {
        strVerify += *iBegin;
    }

    //第三步：对拼接后的字符串进行sha1加密
    string strSha1 = GetSha1Str(strVerify);

    //第四步:判断是否和signature一样
    if (strSignature != strSha1)
    {
        ERROR("This request isn't send from wechat server!");
        DEBUG("strOriginal=" + strVerify);
        DEBUG("strSha1=" + strSha1);
        DEBUG("strSignature=" + strSignature);
        return false;
    }
    else
    {
        //verification success
        DEBUG("string=" + strVerify);
        DEBUG("signature=" + strSignature);
        DEBUG("strSha1=" + strSha1);
        return true;
    }
    DEBUG("========== IsFromWechatServer END ============");
    return false;
}

/**
  *@brief:read data from post request data
  *@param:cData: a memory to store data
  *@param:iLength: data's length
  *@return:void
  */
void wechat::GetPostData(char *cData, unsigned int iLength)
{
    unsigned int iReadCharactorNum = 0;
    while (1)
    {
        cData[iReadCharactorNum] = (char)fgetc(stdin);
        if (iLength - 1 == iReadCharactorNum)
        {
            cData[iReadCharactorNum + 1] = '\0';
            break;
        }
        iReadCharactorNum++;
    }
}

