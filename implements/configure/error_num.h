//======================================================================
//
//        Copyright (C) 2014 回收宝
//        All rights reserved
//
//        filename : error_num.h
//        description : 错误码文件
//
//        created by 叶飞 at  2014-08-01 10:00:00
//        modified by 张世柏 at 2014-10-22 10:15:00
//
//======================================================================

#ifndef _ERROR_NUM_H_
#define _ERROR_NUM_H_

namespace error_num
{

static const unsigned int SUCCEED                       = 0;

static const unsigned int ILLEGAL_REQUEST               = 1001;//非法请求
static const unsigned int HOSTILE_ATTACK                = 1002;//恶意攻击
static const unsigned int SYSTEM_BUG                    = 1003;//系统bug

//数据库2001~2999
static const unsigned int DB_PRICE_CHARACTER            = 2001;//数据库配置错误之产品价格属性表
static const unsigned int DB_CHANNEL                    = 2002;//数据库配置错误之渠道表
static const unsigned int DB_USER                       = 2003;//数据库配置错误之用户表
static const unsigned int DB_VALUATION                  = 2004;//数据库配置错误之估价算法
static const unsigned int DB_QUOTATION                  = 2005;//数据库配置错误之报价算法
static const unsigned int DB_GOODS                      = 2006;//数据库配置错误之商品表
static const unsigned int DB_PRODUCT                    = 2007;//数据库配置错误之产品表
static const unsigned int DB_ORDER                      = 2008;//数据库配置错误之订单表
static const unsigned int DB_ORGANIZATION               = 2009;//数据库配置错误之机构表
static const unsigned int DB_LOGISTICS_CHANNEL          = 2010;//数据库配置错误之物流类型表
static const unsigned int DB_GROUP                      = 2011;//数据库配置错误之用户组
static const unsigned int DB_APPROXIMATION              = 2012;//数据库配置错误之近似算法

//用户3001~3999
static const unsigned int USER_UNKNOWN                  = 3001;//未知用户
static const unsigned int USER_UNLOGIN                  = 3002;//用户未登录
static const unsigned int USER_MODIFY_FAILED            = 3003;//用户修改密码失败
//modified by 张世柏 at 2014-10-22
static const unsigned int USER_QUERY_FAILED             = 3004;//查询信息失败
static const unsigned int USER_ADD_FAILED               = 3005;//向用户表中插入信息失败
//end modify

//渠道4001~4999
static const unsigned int CHANNEL_UNKNOWN               = 4001;//未知渠道

//估价5001~5999
static const unsigned int VALUATION_UNKNOWN             = 5001;//估价算法不存在
static const unsigned int VALUATION_FAILED              = 5002;//估价失败
static const unsigned int VALUATION_SHOWWAYS_UNKNOWN    = 5003;//估价展示条目不存在

//报价6001~6999
static const unsigned int QUOTATION_UNKNOWN             = 6001;//报价算法不存在
static const unsigned int QUOTATION_FAILED              = 6002;//报价失败
static const unsigned int QUOTATION_UNSUPPORT           = 6003;//报价算法暂不支持

//类目7001~7999
static const unsigned int CATEGORY_UNKNOWN              = 7001;//类目错误

//产品8001~8999
static const unsigned int PRODUCT_UNKNOWN               = 8001;//产品不存在
static const unsigned int PRODUCT_PIC_UNKNOWN           = 8002;//产品图片不存在
static const unsigned int PRODUCT_MAXPRICE_UNKNOWN      = 8003;//产品最高价格不存在
static const unsigned int PRODUCT_SHOWWAYS_UNKNOWN      = 8004;//产品展示条目不存在
static const unsigned int PRODUCT_PROFIT_METHOD         = 8005;//利润方案

//商品9001~9999
static const unsigned int GOODS_UNKNOWN                 = 9001;//商品不存在
static const unsigned int GOODS_FAILED                  = 9002;//更新商品表失败

//订单10001~10999
static const unsigned int ORDER_UNKNOWN                 = 10001;//订单不存在
static const unsigned int ORDER_FAILED                  = 10002;//更新订单表失败

//物流11001~11999
static const unsigned int LOGISTICS_UNKNOWN             = 11001;//物流单不存在
static const unsigned int LOGISTICS_FAILED              = 11002;//更新物流表失败

//权限12001~12999
static const unsigned int AUTHORITY_NO                  = 12001;//没有权限

//近似算法13001~13999
static const unsigned int APPROXIMATION_UNKNOWN         = 13001;//近似算法未知
static const unsigned int APPROXIMATION_FAILED          = 13002;//近似算法失败
static const unsigned int APPROXIMATION_UNSUPPORT       = 13003;//近似算法暂不支持

//账户
//modified by Zhang Shibo at 2014-10-30 18:02:00
static const unsigned int ACCOUNT_GET_FAILED            = 14001;//获取账户信息失败
static const unsigned int ACCOUNT_MODIFY_FAILED         = 14002;//修改账户信息失败
//end modify

}

#endif

