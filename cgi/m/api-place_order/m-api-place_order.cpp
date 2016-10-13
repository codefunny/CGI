//======================================================================
//
//        Copyright (C) 2014 回收宝
//        All rights reserved
//
//        filename : m-api-place_order.cpp
//        description : 微信用户下单的CGI入口
//
//        created by 张世柏 at  2014-10-21 17:00:00
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
#include "user_table.h"
#include "account_info_table.h"
#include "wechat_template_msg.h"
#include "wechat_send_sms_msg.h"
#include "order.h"
#include "login.h"

#define SMS_MSG_TEMP_ID 1038

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
    string strGid;
    string strChannel;
    string strPayways;
    string strTradeways;
    string strIMEI;
    string strRemarks;
    string strTel;
    string strPaywaysChangs;
    string strPaywayID;
    string strPaywayType;
    string strPaywayName;
    string strPaywayBank;
    string strPaywayBankAccount;
    string strPaywayAliAccount;
};

static bool ParseCgi(CRequest& oRequest)
{
    DEBUG("=========== ParseCgi START =========");
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
        oRequest.strUserID = mapRequest["uid"];
        oRequest.strUserKey = mapRequest["userkey"];
        //channel以post请求的为准
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
        if (mapRequest.end() == mapRequest.find("gid"))
        {
            ERROR("invalid request");
            return false;
        }
        if (mapRequest.end() == mapRequest.find("channel"))
        {
            ERROR("invalid request");
            return false;
        }
        if (mapRequest.end() == mapRequest.find("payways"))
        {
            ERROR("invalid request");
            return false;
        }
        if (mapRequest.end() == mapRequest.find("tradeways"))
        {
            ERROR("invalid request");
            return false;
        }
        if (mapRequest.end() == mapRequest.find("IMEI"))
        {
            ERROR("invalid request");
            return false;
        }
        if (mapRequest.end() == mapRequest.find("tel"))
        {
            ERROR("invalid request");
            return false;
        }
        if (mapRequest.end() == mapRequest.find("payway_change"))
        {
            ERROR("invalid request");
            return false;
        }
        if (mapRequest.end() == mapRequest.find("payway_type"))
        {
            ERROR("Cannot find payway_type value.");
            return false;
        }

        oRequest.strGid = mapRequest["gid"];
        oRequest.strChannel = mapRequest["channel"];
        oRequest.strPayways = mapRequest["payways"];
        oRequest.strTradeways = mapRequest["tradeways"];
        oRequest.strIMEI = mapRequest["IMEI"];
        oRequest.strTel = mapRequest["tel"];
        oRequest.strPaywaysChangs = mapRequest["payway_change"];
        oRequest.strPaywayType = mapRequest["payway_type"];

        DEBUG("gid is : " + oRequest.strGid +
              "\nchannel is : " + oRequest.strChannel +
              "\npayways is : " + oRequest.strPayways +
              "\ntradeways is : " + oRequest.strTradeways +
              "\nIMEI is : " + oRequest.strIMEI +
              "\npayway_change is: " + oRequest.strPaywaysChangs +
              "\nstrPaywayType: " + oRequest.strPaywayType);

        static const string strPayWayIsChanged = "1";
        if (strPayWayIsChanged == oRequest.strPaywaysChangs)
        {
            //用户使用了新的支付方式
            DEBUG("User use a new pay way.");
            unsigned int iPaywayType = ::atoi(oRequest.strPaywayType.c_str());
            if (ACCOUNT_ONLINEBANK == iPaywayType)
            {
                //网银支付
                DEBUG("User use bank as pay way.");
                if (mapRequest.end() == mapRequest.find("payway_name"))
                {
                    ERROR("Cannot find payway_name value.");
                    return false;
                }
                if (mapRequest.end() == mapRequest.find("payway_bank"))
                {
                    ERROR("Cannot find payway_bank value.");
                    return false;
                }
                if (mapRequest.end() == mapRequest.find("payway_bank_account"))
                {
                    ERROR("Cannot find payway_bank_account value.");
                    return false;
                }
                oRequest.strPaywayName = mapRequest["payway_name"];
                oRequest.strPaywayBank = mapRequest["payway_bank"];
                oRequest.strPaywayBankAccount = mapRequest["payway_bank_account"];
                DEBUG("payway_name: " + oRequest.strPaywayName +
                      "\npayway_bank: " + oRequest.strPaywayBank +
                      "\npayway_bank_account: " + oRequest.strPaywayBankAccount);
            }
            else if (ACCOUNT_ALIPAY == iPaywayType)
            {
                //支付宝支付
                DEBUG("User use alipay as pay way.");
                if (mapRequest.end() == mapRequest.find("payway_alipay_account"))
                {
                    ERROR("Cannot find payway_alipay_account value.");
                    return false;
                }
                oRequest.strPaywayAliAccount = mapRequest["payway_alipay_account"];
                DEBUG("payway_ali_account: " + oRequest.strPaywayAliAccount);
            }
            else
            {
                ERROR("This pay way doesn't exit!");
                return false;
            }
        }
        else
        {
            //支付方式未改变且用户之前有过购买记录
            DEBUG("User doesn't changed pay way.");
            if (mapRequest.end() == mapRequest.find("payway_id"))
            {
                ERROR("Cannot find payway_id value.");
                return false;
            }
            oRequest.strPaywayID = mapRequest["payway_id"];
            DEBUG("User has consumed before, and payway_id: " + oRequest.strPaywayID);
        }
        if (mapRequest.end() != mapRequest.find("remarks"))
        {
            oRequest.strRemarks = mapRequest["remarks"];
            DEBUG("remarks is : " + oRequest.strRemarks);
        }
    }
    else
    {
        ERROR("Hasn't post data!");
        return false;
    }
    DEBUG("=========== ParseCgi END =========");
    return true;
}

static void HandleCgi(CRequest& oRequest, Json::Value& jsonOutput)
{
    DEBUG("=========== HandleCgi START =========");
    string strCGIName = "/api/place_order";
    if (strCGIName != oRequest.strScriptFileName)
    {
        ERROR("invalid request");
        jsonOutput["ret"] = "1";
        jsonOutput["errstr"] = "请求错误";
        return;
    }
    unsigned int iOrderID = 0;
    unsigned int iPlaceOrderResult = logic::PlaceOrder(oRequest.strUserID, oRequest.strUserKey, ::atoi(oRequest.strGid.c_str()), ::atoi(oRequest.strChannel.c_str()), ::atoi(oRequest.strPayways.c_str()), ::atoi(oRequest.strTradeways.c_str()), oRequest.strIMEI, oRequest.strRemarks, iOrderID);
    if (error_num::SUCCEED == iPlaceOrderResult)
    {
        bool bIsAccountInfoModifySuccess = false;
        do
        {
            //判断用户的账户信息是否发生变化
            static const string strPayWayIsChanged = "1";
            if (strPayWayIsChanged == oRequest.strPaywaysChangs)
            {
                //用户的账户信息发生了变化
                //向用户账号表中新增一条数据
                DEBUG("User use a new pay way.");
                map<string, string> mapAccountInfo;
                mapAccountInfo.insert(make_pair<string, string>("Faccount_type", oRequest.strPaywayType));
                unsigned int iAccountInfoID;
                unsigned int iPaywayType = ::atoi(oRequest.strPaywayType.c_str());
                if (ACCOUNT_ONLINEBANK == iPaywayType)
                {
                    //网银
                    DEBUG("User use online_bank.");
                    mapAccountInfo.insert(make_pair<string, string>("Fwy_name", oRequest.strPaywayName));
                    mapAccountInfo.insert(make_pair<string, string>("Fwy_phone", oRequest.strTel));
                    mapAccountInfo.insert(make_pair<string, string>("Fwy_bank_name", oRequest.strPaywayBank));
                    mapAccountInfo.insert(make_pair<string, string>("Fwy_bank_cardid", oRequest.strPaywayBankAccount));
                }
                else if (ACCOUNT_ALIPAY == iPaywayType)
                {
                    //支付宝
                    DEBUG("User use alipay.");
                    mapAccountInfo.insert(make_pair<string, string>("Fzfb_phone", oRequest.strTel));
                    mapAccountInfo.insert(make_pair<string, string>("Fzfb_account", oRequest.strPaywayAliAccount));
                }
                else
                {
                    ERROR("This pay way [" + oRequest.strPaywayType + "] doesn't exit!");
                    break;
                }

                //执行插入操作
                if (!logic::InsertAccountInfoTable(mapAccountInfo, iAccountInfoID))
                {
                    ERROR("Insert account info failed.");
                    break;
                }
                else
                {
                    oRequest.strPaywayID = tools::CStringTools::Int2String(iAccountInfoID);
                    DEBUG("Insert account info successed! Account id is: " + oRequest.strPaywayID);
                }
            }
            else
            {
                //用户的账户信息未变化
                DEBUG("User use old pay way.");
            }

            //更新订单信息
            if (!logic::UpdateOrderTableAccountID(iOrderID, ::atoi(oRequest.strPaywayID.c_str())))
            {
                ERROR("Update order [" + tools::CStringTools::Int2String(iOrderID) + "] info failed!");
                break;
            }
            else
            {
                DEBUG("Update order [" + tools::CStringTools::Int2String(iOrderID) + "] successed!");
            }

            //更新用户表的Faccount_id 和 Fphone_num字段
            //第一步：取出用户表中的Faccount_id_list字段
            map<string, string> mapUserInfo;
            if (!logic::SelectUserTable(::atoi(oRequest.strUserID.c_str()), mapUserInfo))
            {
                ERROR("Select user info [user_id: " + oRequest.strUserID + "] failed!");
                break;
            }
            else
            {
                //获取用户信息成功
                DEBUG("Select user info [user_id: " + oRequest.strUserID + "] successed!");
            }
            if (mapUserInfo.end() == mapUserInfo.find("Faccount_id_list"))
            {
                ERROR("Cannot find Faccount_id_list value.");
                break;
            }
            string strOldAccountList = mapUserInfo["Faccount_id_list"];
            DEBUG("The old AccountList is: [" + strOldAccountList + "]");

            if (mapUserInfo.end() == mapUserInfo.find("Fphone_num"))
            {
                ERROR("Hasn't user phone value!");
                break;
            }
            string strOldPhoneNum = mapUserInfo["Fphone_num"];
            DEBUG("User's phone num is: " + strOldPhoneNum);

            //第二步：将用户最近使用的不同种类的支付账号依次读出
            vector<string> vecUserAccountList;
            vecUserAccountList = tools::CStringTools::Split2String(strOldAccountList, "#");
            size_t maxSize = vecUserAccountList.size();
            DEBUG("User has used [" + tools::CStringTools::Int2String(maxSize) + "] pay ways.");
            size_t index;
            for (index = 0; index != maxSize; ++index)
            {
                map<string, string> mapAccountInfo;
                if (!logic::SelectAccountInfo(::atoi(vecUserAccountList[index].c_str()), mapAccountInfo))
                {
                    WARN("There is one account_id [" + vecUserAccountList[index] + "] is unvaluable!");
                    continue;
                }
                DEBUG("mapAccountInfo[Faccount_type] : " + mapAccountInfo["Faccount_type"]);
                DEBUG("oRequest.strPaywayType : " + oRequest.strPaywayType);
                //注意：这里默认前台传来的支付类型编号和数据库的一致
                if (oRequest.strPaywayType == mapAccountInfo["Faccount_type"])
                {
                    //更新了用户以前用过的同一类支付方式
                    DEBUG("user's old AccountID is : " + vecUserAccountList[index]);
                    vecUserAccountList[index] = oRequest.strPaywayID;
                    DEBUG("user's new AccountID is : " + vecUserAccountList[index]);
                    DEBUG("Update one Payway which user has used before, and payway type is: " + oRequest.strPaywayType);
                    break;
                }
            }
            if (maxSize == index)
            {
                //用户第一次使用该支付方式
                vecUserAccountList.push_back(oRequest.strPaywayID);
                DEBUG("User has use this payway first, and AccountID is: " + oRequest.strPaywayID);
            }

            //第三步：更新用户最近一次的不同账单列表
            string strNewAccountList;
            maxSize = vecUserAccountList.size();
            for (index = 0; index != maxSize; ++index)
            {
                strNewAccountList += (0 == index ? vecUserAccountList[index] : ("#" + vecUserAccountList[index]));
            }
            DEBUG("The new AccountList is: [" + strNewAccountList + "]");

            //第四步：更新用户表的Faccount_id 和 Fphone_num字段
            if (strOldAccountList != strNewAccountList || strOldPhoneNum != oRequest.strTel)
            {
                if (!logic::UpdateUserTableAccountListAndPhoneNum(::atoi(oRequest.strUserID.c_str()), strNewAccountList, oRequest.strTel))
                {
                    ERROR("Update t_user [user_id: " + oRequest.strUserID + "] failed!");
                    break;
                }
                else
                {
                    DEBUG("Update t_user [user_id: " + oRequest.strUserID + "] successed! \
                        \nUser info changed! Old account list is : " + strOldAccountList + ", New account list is : " + strNewAccountList +
                       "\nOld phone num is : " + strOldPhoneNum + ", New phone num is : " + oRequest.strTel);
                }
            }
            else
            {
                DEBUG("User info doesn't changed! Old account list is : " + strOldAccountList + ", New account list is : " + strNewAccountList +
                       "\nOld phone num is : " + strOldPhoneNum + ", New phone num is : " + oRequest.strTel);
            }
            bIsAccountInfoModifySuccess = true;
        }while(0);

        if (bIsAccountInfoModifySuccess)
        {
            Json::Value jsonData;
            jsonData["orderid"] = tools::CStringTools::Int2String(iOrderID);

            jsonOutput["errcode"] = "0";
            jsonOutput["errstr"] = "";
            jsonOutput["data"] = jsonData;
        }
        else
        {
            jsonOutput["ret"] = "1";
            jsonOutput["errstr"] = "账户信息修改错误";
            return;
        }
        //给改用户发送一条下单成功的消息通知
        string strOpenID;
        string strGoodsName;
        string strRemarks;
        int iQuotation;
        map<string, string> mapMsgInfo;
        map<string, map<string, string> >mapParamInfo;
        if (!logic::SelectUserNameByUserID(::atoi(oRequest.strUserID.c_str()), strOpenID))
        {
            ERROR("Get user openid failed!");
            return;
        }
        else
        {
            DEBUG("Get user openid success! [" + strOpenID + "]");
        }

        if (!logic::GetGoodsNameAndQuotationByOrderID(iOrderID, strGoodsName, iQuotation))
        {
            ERROR("Get gooes name and quotation failed!");
            return;
        }
        else
        {
            DEBUG("Get goods name and quotation successed![" + strGoodsName + "], [" + tools::CStringTools::Int2String(iQuotation) + "].");
        }

        strRemarks = "\n您的手机为：" + strGoodsName +
                     "\n初步估价为：￥" + tools::CStringTools::Int2String(iQuotation/100) + " 元" +
                     "\n请尽快将手机快递给我们 \
                      \n寄送地址：深圳市南山区威新软件园5号楼3楼 郭艳波 0755-36604428 \
                      \n寄送方式：顺丰到付 400-811-1111";

        mapMsgInfo["touser"] = strOpenID;
        mapMsgInfo["template_id"] = TEMPLATEMSG_ORDER_STATE_UPDATE;
        mapMsgInfo["url"] = "https://open.weixin.qq.com/connect/oauth2/authorize?appid=wx3543d54ab3d1a24e&redirect_uri=http%3A%2F%2Fm.huishoubao.com.cn%2Fusercenter_page%3Froute%3Dorders&response_type=code&scope=snsapi_base&state=STATE#wechat_redirect";
        mapParamInfo["first"]["value"] = "尊敬的用户";
        mapParamInfo["first"]["color"] = "#173177";
        mapParamInfo["OrderSn"]["value"] = tools::CStringTools::Int2String(iOrderID);
        mapParamInfo["OrderSn"]["color"] = "#173177";
        mapParamInfo["OrderStatus"]["value"] = "下单成功";
        mapParamInfo["OrderStatus"]["color"] = "#173177";
        mapParamInfo["remark"]["value"] = strRemarks;
        mapParamInfo["remark"]["color"] = "#173177";

        wechat::CTemplateMsg templateMsg;
        if (templateMsg.Send(mapMsgInfo, mapParamInfo))
        {
            DEBUG("Template message send success.");
        }
        else
        {
            WARN("Template message send failed.");
        }

        //向用户发送一条短信
        string strOrderMsg = "您的手机是：" + strGoodsName + ", " +
                             "初步估价为：" + "￥" + tools::CStringTools::Int2String(iQuotation/100) + " 元。" +
                             "请尽快将手机快递给我们,寄送地址：深圳市南山区威新软件园5号楼3楼 郭艳波 0755-36604428 ,寄送方式：顺丰到付 400-811-1111.";
        if (!wechat::CSendMsg::SendMsg(SMS_MSG_TEMP_ID, oRequest.strTel, strOrderMsg))
        {
            WARN("Send message to user [" + oRequest.strTel + "] failed!");
        }
        else
        {
            DEBUG("Send one sms message to user [" + oRequest.strTel + "].!");
        }
    }
    else if (error_num::USER_UNLOGIN == iPlaceOrderResult)
    {
        ERROR("User has not login!");
        jsonOutput["ret"] = "1";
        jsonOutput["errcode"] = tools::CStringTools::Int2String(error_num::USER_UNLOGIN);
        jsonOutput["errstr"] = "用户未登陆";
    }
    else
    {
        jsonOutput["ret"] = "1";
        jsonOutput["errcode"] = tools::CStringTools::Int2String(iPlaceOrderResult);
        jsonOutput["errstr"] = "下单失败";
    }
    DEBUG("=========== HandleCgi END =========");
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
                  << "gid               is : " << oRequest.strGid << "\n"
                  << "channel           is : " << oRequest.strChannel << "\n"
                  << "payways           is : " << oRequest.strPayways << "\n"
                  << "tradeways         is : " << oRequest.strTradeways << "\n"
                  << "IMEI              is : " << oRequest.strIMEI << "\n"
                  << "remarks           is : " << oRequest.strRemarks << "";
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


