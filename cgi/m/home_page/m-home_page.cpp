//======================================================================
//
//        Copyright (C) 2014 回收宝
//        All rights reserved
//
//        filename : m-home_page.cpp
//        description : 微信用户登陆首页的CGI入口
//
//        created by 张世柏 at  2014-10-22 10:00:00
//        modified by mickey at 2014-11-20 2:20:00
//
//======================================================================

#include <stdlib.h>
#include <string.h>
#include <string>
#include <map>
#include <sstream>
#include <fcgi_stdio.h>
#include "configure.h"
#include "error_num.h"
#include "string_tools.h"
#include "login.h"
#include "user_table.h"
#include "group_table.h"
#include "wechat_login.h"
#include "category_table.h"
#include "clearsilver.h"

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
    string strPostData;
    string strCode;
    bool bFromSharedPage;
};

static bool ParseCgi(CRequest& oRequest)
{
    DEBUG("============== Begin to parse cgi ================");
    oRequest.bFromSharedPage = false;
    string strLocationUrl = "Location:" + WECHAT_LOCATION_FOR_CODE + "?appid=" + WECHAT_APPID + "&redirect_uri=" + WECHAT_LOCATION_HOME_PAGE + WECHAT_LOCATION_PARAM + "\r\n";
    if (getenv("QUERY_STRING"))
    {
        oRequest.strGetData = getenv("QUERY_STRING");
    }

    if (getenv("REQUEST_METHOD"))
    {
        oRequest.strMethod = getenv("REQUEST_METHOD");
    }

    if (getenv("SCRIPT_FILENAME"))
    {
        oRequest.strScriptFileName = getenv("SCRIPT_FILENAME");
    }

    if(getenv("HTTP_COOKIE"))
    {
        oRequest.strCookie = getenv("HTTP_COOKIE");
        oRequest.strCookie = tools::CStringTools::Trim(oRequest.strCookie);
    }

    if (!oRequest.strGetData.empty())
    {
        map<string, string> mapRequest = tools::CStringTools::SplitTwice2String(oRequest.strGetData, "&", "=");
        if (mapRequest.end() == mapRequest.find("code"))
        {
            ERROR("invalid request");
            DEBUG_AND_RETURN(strLocationUrl.c_str());
            FCGI_printf("Content-type:text/html\r\n\r\n");
            oRequest.bFromSharedPage = true;
            return false;
        }
        oRequest.strCode = mapRequest["code"];
    }
    else
    {
        //code must be not empty;
        DEBUG_AND_RETURN(strLocationUrl.c_str());
        FCGI_printf("Content-type:text/html\r\n\r\n");
        oRequest.bFromSharedPage = true;
        return false;
    }
    DEBUG("============== End to parse cgi ================");
    return true;
}

static void HandleCgi(const CRequest& oRequest)
{
    DEBUG("============== Begin to handle cgi ================");
    static const string strCGIName = "/home_page";
    static const string strCsFile = "home_page.cs";
    static const string strErrorCsFile = "error_page.cs";
    string strOpenID, strAccessToken;
    map<string, string> mapUserInfo;
    bool isOldUser;
    if (strCGIName != oRequest.strScriptFileName)
    {
        ERROR("invalid request");
        return;
    }

    do
    {
        //开发者模式
        if ("12345" == oRequest.strCode)
        {
            DEBUG("########################## TEST MODEL ##############################");
            strOpenID = "op_GUt80vhU1ejCuMhbVUuOCQQOc";
            strAccessToken = "OezXcEiiBSKSxW0eoylIeC3JAzEzfsNDHZUKja9LGMBPybc7cVmVUNOasVBi04igl9HodsTgv69fXb7cG7ff6FnAJTtjwQASAX0PeSVLkCrcnjcnNKaTawKMHf7WgWv62R1AG1Yxum2Sy8mNllmeyw";
            mapUserInfo.insert(make_pair<string, string>("Fuser_id", "100000001"));
            mapUserInfo.insert(make_pair<string, string>("Fuser_channel", "10000001"));
            user::SaveLandingState(mapUserInfo["Fuser_id"], strAccessToken);
            isOldUser = true;
            break;
        }

        //判断页面是否刷新
        bool bIsRefreshenedPage = wechat::IsRefreshenedPage(oRequest.strCookie);
        if (!bIsRefreshenedPage)
        {
            DEBUG("Begin to get openid and access_token");

            //获取用户的openid和access_token
            if (!wechat::GetOpenIDAccessToken(oRequest.strCode, strOpenID, strAccessToken))
            {
                ERROR("Fail to get openid and access_token!");
                break;
            }
            else
            {
                DEBUG("Success to get openid and access_token!");
            }

            //判断是否在用户表中,如果存在就通过mapUserInfo返回，返回值是数据库操作是否正确
            bool bIsSqlQuerySuccess= logic::CheckAndSelectUserByUserName(strOpenID, mapUserInfo, isOldUser);
            if (bIsSqlQuerySuccess)
            {
                //判断是否是已注册用户
                if (!isOldUser)
                {
                    //用户表中没有该用户记录，则向数据库t_user,t_group表中插入新用户信息
                    //用户的id是数据库中现有用户id号中最大的那个加1
                    //用户的渠道id是固定的：10000001
                    //用户的机构id是固定的：10000001

                    DEBUG("The user isn't a registered user");
                    unsigned int iInsertResult = logic::InsertUserTable(strOpenID, mapUserInfo);
                    if (error_num::USER_ADD_FAILED == iInsertResult)
                    {
                        //向用户表中插入失败
                        ERROR("Fail to insert into t_user!");
                        break;
                    }
                    else if(error_num::USER_QUERY_FAILED == iInsertResult)
                    {
                        //插入数据成功，但读取刚插入的用户ID出错
                        ERROR("Insert user [" + strOpenID + "] success, but fail to select for Fuser_id or Fuser_channel!");
                        break;
                    }
                    else
                    {
                        //成功向用户表中插入一条数据
                        DEBUG("Add a new user [" + mapUserInfo["Fuser_id"] + "] into t_user success!");
                    }

                    unsigned int iTryTimes;
                    static const unsigned int MAX_TRY_TIMES = 3;
                    //这里之所以要插入3次是因为要防止用户表中新增一条数据，用户组表中没有增加，造成数据不同步
                    for (iTryTimes = 0; iTryTimes < MAX_TRY_TIMES; ++iTryTimes)
                    {
                        if (!logic::InsertGroupTable(mapUserInfo["Fuser_id"], WECHAT_GROUP_ID))
                        {
                            continue;
                        }
                        else
                        {
                            //成功向用户组表中插入一条数据
                            DEBUG("Insert user [" + mapUserInfo["Fuser_id"] + "] into t_group success.");
                            break;
                        }
                    }
                    if (MAX_TRY_TIMES == iTryTimes)
                    {
                        //3次仍未插入成功，需要报出警告信息
                        WARN("Fail to insert userid [" + mapUserInfo["Fuser_id"] + "] into t_group");
                        break;
                    }
                }
                else
                {
                    //数据库中已存在该用户信息
                    DEBUG("The user is a registered user");
                }

            }
            else
            {
                //执行数据库查询操作失败
                ERROR("Select Table t_user failed!");
                break;
            }

            //写入缓存
            if(mapUserInfo.end() == mapUserInfo.find("Fuser_id"))
            {
                ERROR("mapUserInfo doesn't has item Fuser_id.");
                break;
            }
            user::SaveLandingState(mapUserInfo["Fuser_id"], strAccessToken);

            //写入cookie
            DEBUG("Writing into cookie!");
            DEBUG("Fuser_id = " + mapUserInfo["Fuser_id"]);
            DEBUG("Fuser_channel = " + mapUserInfo["Fuser_channel"]);
            DEBUG_AND_RETURN(wechat::WriteInCookie("uid", mapUserInfo["Fuser_id"]).c_str());
            DEBUG_AND_RETURN(wechat::WriteInCookie("userkey", strAccessToken).c_str());
            DEBUG_AND_RETURN(wechat::WriteInCookie("channel", mapUserInfo["Fuser_channel"]).c_str());
        }
        else
        {
            DEBUG("User has refreshened the page!");
            isOldUser = true;
        }
    }while(0);

    //渲染模板
    DEBUG("Begin to render the template");
    tools::CClearSilver oClearSilver;

    /// start modified by mickey at 2014-11-20 2:20:00
    //oClearSilver.SetValue("cdnbase", CDN_BASE);
    oClearSilver.SetValue("cdnbase", TENCENT_CDN_BASE);
    /// end modified
    oClearSilver.SetDirectory(CDN_M_DIRECTORY);
    oClearSilver.SetValue("banner.0.image", "banner_2014.11.10_4.jpg");
    oClearSilver.SetValue("banner.0.href", "");
    oClearSilver.SetValue("banner.1.image", "banner_2014.11.10_2.jpg");
    oClearSilver.SetValue("banner.1.href", "");
    oClearSilver.SetValue("banner.2.image", "banner_2014.11.10_3.jpg");
    oClearSilver.SetValue("banner.2.href", "");

    //从产品表中获取根节点的产品信息
    vector< map<string, string> > vecCategoryList;
    if (!logic::SelectSubCategoryDepthForOne(CATEGORY_ROOT, vecCategoryList))
    {
        ERROR("Select t_category table failed");
        string strResult = oClearSilver.Render(strErrorCsFile);
        FCGI_printf("Content-type:text/html\r\n\r\n");
        FCGI_printf(strResult.c_str());
        return;
    }

    //加入hdf数组
    size_t maxSize = vecCategoryList.size();
    for (size_t index = 0; index < maxSize; index++)
    {
        string strMidKey, strMidValue;
        string strNameKey, strNameValue;
        string strLogoKey, strLogoValue;

        strMidKey += "menu." + tools::CStringTools::Int2String(index) + ".mid";
        if (vecCategoryList[index].end() == vecCategoryList[index].find("Fcategory_id"))
        {
            ERROR("Fcategory_id is NULL.");
            strMidValue = "";
        }
        else
        {
            strMidValue = vecCategoryList[index]["Fcategory_id"];
            DEBUG("Fcategory_id = " + strMidValue);
        }

        strNameKey += "menu." + tools::CStringTools::Int2String(index) + ".name";
        if (vecCategoryList[index].end() == vecCategoryList[index].find("Fcategory_name"))
        {
            ERROR("Fcategory_name is NULL.");
            strNameValue = "";
        }
        else
        {
            strNameValue = vecCategoryList[index]["Fcategory_name"];
            DEBUG("Fcategory_name = " + strNameValue);
        }

        strLogoKey += "menu." + tools::CStringTools::Int2String(index) + ".logo";
        if (vecCategoryList[index].end() == vecCategoryList[index].find("Fbrand_logo"))
        {
            ERROR("Fbrand_logo is NULL.");
            strLogoValue = "";
        }
        else
        {
            strLogoValue = vecCategoryList[index]["Fbrand_logo"];
            DEBUG("Fbrand_logo = " + strLogoValue);
        }

        oClearSilver.SetValue(strMidKey, strMidValue);
        oClearSilver.SetValue(strNameKey, strNameValue);
        oClearSilver.SetValue(strLogoKey, strLogoValue);
    }

    if (isOldUser)
    {
        oClearSilver.SetValue("firstvisit", "0");
    }
    else
    {
        oClearSilver.SetValue("firstvisit", "1");
    }
    string strResult = oClearSilver.Render(strCsFile);

    //测试模式下写cookie
    if ("12345" == oRequest.strCode)
    {
        DEBUG("Writing into cookie!");
        DEBUG("Fuser_id = " + mapUserInfo["Fuser_id"]);
        DEBUG("Fuser_channel = " + mapUserInfo["Fuser_channel"]);
        DEBUG_AND_RETURN(wechat::WriteInCookie("uid", mapUserInfo["Fuser_id"]).c_str());
        DEBUG_AND_RETURN(wechat::WriteInCookie("userkey", strAccessToken).c_str());
        DEBUG_AND_RETURN(wechat::WriteInCookie("channel", mapUserInfo["Fuser_channel"]).c_str());
    }

    FCGI_printf("Content-type:text/html\r\n\r\n");
    FCGI_printf(strResult.c_str());
    DEBUG("============== End to handle cgi ================");
}

int main()
{
    while(FCGI_Accept() >= 0)
    {
        log::CLog::Initialize("");
        CRequest oRequest;
        bool parseResult = ParseCgi(oRequest);

        stringstream ssRequest;
        ssRequest << "==========CGI START==========\n"
                  << "request method    is : " << oRequest.strMethod << "\n"
                  << "content length    is : " << oRequest.iContentLength << "\n"
                  << "request get data  is : " << oRequest.strGetData << "\n"
                  << "request post data is : " << oRequest.strPostData << "\n"
                  << "request cookie    is : " << oRequest.strCookie << "\n"
                  << "request url       is : " << oRequest.strScriptFileName << "\n"
                  << "code              is : " << oRequest.strCode;

        DEBUG(ssRequest.str());

        if (!parseResult)
        {
            if (oRequest.bFromSharedPage)
            {
                DEBUG("From shared page, and is already location to home_page!");
            }
            else
            {
                ERROR("invalid request");
                static const string strErrorCsFile = "error_page.cs";
                tools::CClearSilver oClearSilver;
                /// start modified by mickey at 2014-11-20 2:20:00
                //oClearSilver.SetValue("cdnbase", CDN_BASE);
                oClearSilver.SetValue("cdnbase", TENCENT_CDN_BASE);
                /// end modified
                oClearSilver.SetDirectory(CDN_M_DIRECTORY);
                string strResult = oClearSilver.Render(strErrorCsFile);
                DEBUG_AND_RETURN("Content-type:text/html\r\n\r\n");
                DEBUG_AND_RETURN(strResult.c_str());
            }
        }
        else
        {
            HandleCgi(oRequest);
        }

        DEBUG("===========CGI END===========");
    }

    return 0;
}

