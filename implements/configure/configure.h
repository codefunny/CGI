//======================================================================
//
//        Copyright (C) 2014 回收宝
//        All rights reserved
//
//        filename : configure.h
//        description : 配置中心、宏、全局变量
//
//        created by 叶飞 at  2014-08-01 10:00:00
//        modified by 张世柏 at 2014-10-24
//        modified by mickey at 2014-11-20 2:20:00
//
//======================================================================

#ifndef _CONFIGURE_H_
#define _CONFIGURE_H_

#include <string>
#include <map>
#include <fcgi_stdio.h>
#include "log.h"

static const std::string CDN_ROOT_DIRECTORY             = "/cdn/root/template/";
//modified by 张世柏 at 2014-10-16 15:00:00
static const std::string CDN_M_DIRECTORY                = "/cdn/root/m/tpl/";
//end modified
static const std::string CDN_BASE                       = "http://cdn.huishoubao.com.cn/";

//add by mickey at 2014-11-20 2:20:00
static const std::string TENCENT_CDN_BASE               = "http://1251010403.cdn.myqcloud.com/1251010403/";
//end modified

static const std::string CDN_BASE_RAW                   = "http://cdn.huishoubao.com.cn";
static const std::string DOMAIN_NAME                    = "sp.huishoubao.com.cn";
//modified by 张世柏 at 2014-10-20
static const std::string ROOT_DOMAIN_NAME               = "huishoubao.com.cn";
//end modify

#define DEBUG(input) log::DEBUG_LOG((input), (__FILE__), (__LINE__));
#define ERROR(input) log::ERROR_LOG((input), (__FILE__), (__LINE__));
#define WARN(input) log::WARN_LOG((input), (__FILE__), (__LINE__));

#define RETURN2CLIENT(input) FCGI_printf((input));
#define DEBUG_AND_RETURN(input) {DEBUG((input)); FCGI_printf((input));}
#define ERROR_AND_RETURN(input) {ERROR((input)); FCGI_printf((input));}
#define WARN_AND_RETURN(input) {WARN((input)); FCGI_printf((input));}

static const unsigned int PRODUCT_PROFIT_TYPE_PERCENT           = 1;//百分比
static const unsigned int PRODUCT_PROFIT_TYPE_ABSOLUTE          = 2;//值

static const unsigned int PRICE_CHARACTER_TYPE_SHOW             = 0;//展示
static const unsigned int PRICE_CHARACTER_TYPE_MULTIPLICATION   = 1;//乘法
static const unsigned int PRICE_CHARACTER_TYPE_SUBTRACTION      = 2;//减法
static const unsigned int PRICE_CHARACTER_TYPE_FUNCTION         = 3;//函数
static const unsigned int PRICE_CHARACTER_TYPE_FIXED            = 4;//固定值

//订单状态表 必须与数据库一致
static const unsigned int ORDER_STATUS_PLACED_ORDER             = 10;//已下单，待发货
static const unsigned int ORDER_STATUS_SENDING                  = 20;//已发货，物流中
static const unsigned int ORDER_STATUS_GOT_IN                   = 30;//已入库，待分检
static const unsigned int ORDER_STATUS_PRE_CHECK                = 40;//分检合格，待检测
static const unsigned int ORDER_STATUS_CHECKING                 = 50;//检测中
static const unsigned int ORDER_STATUS_PRE_PAY                  = 60;//检测一致，待付款
static const unsigned int ORDER_STATUS_PAYED_OUT                = 70;//已付款，待收款确认
static const unsigned int ORDER_STATUS_CANCELED                 = 80;//订单已取消
static const unsigned int ORDER_STATUS_PRE_SEND_BACK            = 90;//准备退货
static const unsigned int ORDER_STATUS_NEGOTIATING              = 100;//检测不一致，议价中
static const unsigned int ORDER_STATUS_SENDING_BACK             = 110;//已退货，物流中
static const unsigned int ORDER_STATUS_USER_GOT_BACK_GOOD       = 120;//已确认退货，待评价
static const unsigned int ORDER_STATUS_USER_GOT_MONEY           = 130;//已确认收款，待评价
static const unsigned int ORDER_STATUS_USER_ESTIMATED           = 140;//已评价
//static const unsigned int ORDER_STATUS_GOT_OUT                  = 150;//销售出库
//static const unsigned int ORDER_STATUS_GOT_MONEY                = 160;//销售实收到账

//权限表 必须与数据库一致
static const unsigned int AUTHORITY_GET_ORDER                   = 10;//订单查询
static const unsigned int AUTHORITY_CREATE_ORDER                = 20;//创建订单 0-10
static const unsigned int AUTHORITY_SEND_GOODS                  = 30;//订单发货 10-20
static const unsigned int AUTHORITY_CANCEL_BEFORE_SEND          = 40;//订单发货前取消 10-80
static const unsigned int AUTHORITY_MODIFY_BEFORE_GETIN         = 50;//订单入库前修改 before 30 暂无
static const unsigned int AUTHORITY_CONFIRM_MONEY               = 60;//确认收款 70-130
static const unsigned int AUTHORITY_CONFIRM_BACK_GOOD           = 70;//确认退货 110-120
static const unsigned int AUTHORITY_ESTIMATE                    = 80;//评价 130-140, 120-140
static const unsigned int AUTHORITY_PRE_GETIN                   = 1010;//物流签收 20-30
static const unsigned int AUTHORITY_CONFIRM_GETIN               = 1020;//确认收货 30-40
static const unsigned int AUTHORITY_START_CHECK                 = 1030;//开始检测 40-50
static const unsigned int AUTHORITY_JC_MODIFY_AFTER_GETIN       = 1040;//订单入库后检测员修改（订单内容） 50-60, 50-100
static const unsigned int AUTHORITY_PAY_OUT                     = 1050;//订单付款 60-70
static const unsigned int AUTHORITY_CANCEL_AFTER_GETIN          = 1060;//订单入库后取消 30-80
static const unsigned int AUTHORITY_PRE_SEND_BACK               = 1070;//订单入库后标记退货 30-90
static const unsigned int AUTHORITY_SEND_BACK                   = 1080;//订单入库后退货操作 90-110
static const unsigned int AUTHORITY_KF_MODIFY_AFTER_GETIN       = 1090;//订单入库后客服修改（订单内容） 100-60
static const unsigned int AUTHORITY_NEGOTIATE_FAILED            = 1100;//订单议价失败退货（订单状态）100-90

static const unsigned int LOGISTICS_STATUS_START                = 1;//物流开始
static const unsigned int LOGISTICS_STATUS_SENDING              = 2;//物流发货中
static const unsigned int LOGISTICS_STATUS_END                  = 3;//物流结束

static const unsigned int PRODUCT_VALID                         = 1;//有效产品
static const unsigned int PRODUCT_INVALID                       = 2;//无效产品

static const unsigned int CATEGORY_ROOT                         = 1;//类目树根结点

static const unsigned int TO_B_DEFAULT_CHANNEL                  = 2;//2B总渠道标识

//modified by 张世柏 at 2014-10-17 12:00:00
static const std::string WECHAT_USER_CHANNEL                    = "10000001";//微信公众号用户所属渠道
static const std::string WECHAT_USERID_BEGIN                    = "100000000";//微信公众号用户ID的最小编号
static const std::string WECHAT_USERID_END                      = "200000000";//微信公众号用户ID的最大编号
static const std::string WECHAT_GROUP_ID                        = "5";//微信公共号用户所在的用户组ID
static const std::string WECHAT_APPID                           = "wx3543d54ab3d1a24e";//微信公众号appid
static const std::string WECHAT_SECRET                          = "74f3a1f74142c898aa70f244a9af1ae5";//微信公众号secret
static const std::string WECHAT_LOCATION_FOR_CODE               = "https://open.weixin.qq.com/connect/oauth2/authorize"; //向微信服务器发起重定向请求code
static const std::string WECHAT_LOCATION_PARAM                  = "&response_type=code&scope=snsapi_base&state=STATE#wechat_redirect"; //向微信服务器发起重定向请求code的固定参数
static const std::string WECHAT_LOCATION_HOME_PAGE              = "http%%3A%%2F%%2Fm.huishoubao.com.cn%%2Fhome_page"; //微信首页链接

static const std::string TEMPLATEMSG_ORDER_STATE_UPDATE         = "Hi2PvSBagTqrPL7F6NTht8tDr-64IdZCGfdE79C0tEg";//订单状态更新模板ID

static const unsigned int ACCOUNT_ONLINEBANK                    = 1;//网银
static const unsigned int ACCOUNT_ALIPAY                        = 2;//支付宝
//end modified

class CConfigure
{
public:
    static CConfigure* GetInstance();
    std::string GetHostName() const {return m_strHostName;}
    std::string GetUserName() const {return m_strUserName;}
    std::string GetPassword() const {return m_strPassword;}
    std::string GetDBName() const {return m_strDBName;}
    std::string GetCharacterSet() const {return m_strCharacterSet;}
    void ShowConfig() const;

private:
    CConfigure();
    ~CConfigure();
    static CConfigure* pInstance;

    std::string m_strHostName;
    std::string m_strUserName;
    std::string m_strPassword;
    std::string m_strDBName;
    std::string m_strCharacterSet;
};

class CGlobalData
{
public:
    static CGlobalData* GetInstance();
    std::map<unsigned int, std::string> GetAllOrderStatus() const;
    std::string GetOrderName(unsigned int iOrderStatus);

private:
    CGlobalData();
    ~CGlobalData();
    static CGlobalData* pInstance;

    std::map<unsigned int, std::string> m_orderStatus2Name;
};

#endif


