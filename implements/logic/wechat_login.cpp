#include "wechat_login.h"
#include <string>
#include <json/json.h>
#include "configure.h"
#include "http.h"
#include "protocol.h"
#include "string_tools.h"
#include "login.h"

using namespace std;

/**
  *brife 获取微信用户的OpenID和AccessToken
  *@param1:strCode:传入参数：微信服务器发来的code
  *@param2:strOpenid：传出参数：用户的OpenID
  *@param3:strAccessToken：传出参数：网页授权AccessToken
  *return:json解析成功时返回true,否则返回false
  */
bool wechat::GetOpenIDAccessToken(const string &strCode, string &strOpenid, string &strAccessToken)
{
    DEBUG("=======GetOpenIDAccessToken START=======");
    string strReply;
    string strUrl = string("https://api.weixin.qq.com/sns/oauth2/access_token?") +
                    "appid=" + WECHAT_APPID +
                    "&secret=" + WECHAT_SECRET +
                    "&code=" + strCode +
                    "&grant_type=authorization_code";

    //modified by Zhang Shibo at 2014-11-14 22:32:00
    //modify brief: Add the judge with return value.
    //发送get请求类型的url
    DEBUG("url : " + strUrl);
    tools::CCurlWrapper curlWrapper;
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
    //end modify

    //parseJson
    if(!protocol::UnPackingByJson(strReply, "openid", strOpenid))
    {
        ERROR("Parse Openid in Json failed!");
        return false;
    }
    else
    {
        //Parse Openid success
    }

    if(!protocol::UnPackingByJson(strReply, "access_token", strAccessToken))
    {
        ERROR("Parse access_token in Json failed");
        return false;
    }
    else
    {
        //Parse access_token success
    }
    DEBUG("=======GetOpenIDAccessToken START=======");
    return true;
}

/**
  *brife:判断用户是否刷新页面
  *param1：从客户端发过来的cookie
  *return: 页面刷新返回true，否则返回false
  */
bool wechat::IsRefreshenedPage(const string &strCookie)
{
    DEBUG("============== Begin to judge IsRefreshenedPage ================");

    string strUserID, strUserkey, strChannel;
    if (!strCookie.empty())
    {
        map<string, string> mapRequest = tools::CStringTools::SplitTwice2String(strCookie, ";", "=");
        if (mapRequest.end() == mapRequest.find("uid"))
        {
            ERROR("invalid request : There is no key named uid in cookie." );
            return false;
        }
        if (mapRequest.end() == mapRequest.find("userkey"))
        {
            ERROR("invalid request : There is no key named userkey in cookie." );
            return false;
        }
        strUserID = mapRequest["uid"];
        strUserkey = mapRequest["userkey"];

        if (user::CheckLandingState(strUserID, strUserkey))
        {
            DEBUG("already login");
            return true;
        }
        else
        {
            ERROR("This user[" + strUserID + "] doesn't has land state");
            return false;
        }
    }
    else
    {
        ERROR("Doesn't get cookie in environment valuable.");
        return false;
    }

    DEBUG("============== End to judge IsRefreshenedPage ================");
    return false;
}

string wechat::WriteInCookie(const string& strParamName, const string& strParamValue)
{
    string strCookie = "Set-Cookie: " + strParamName + "=" + strParamValue + "; PATH=/; DOMAIN=" + ROOT_DOMAIN_NAME + "\r\n";
    return strCookie;
}
