//======================================================================
//
//        Copyright (C) 2014 回收宝
//        All rights reserved
//
//        filename : product.h
//        description : 产品接口
//
//        created by 叶飞 at  2014-08-01 10:00:00
//        modified by XX at  
//
//======================================================================

#ifndef _PRODUCT_H_
#define _PRODUCT_H_

#include <string>
#include <json/json.h>

namespace logic
{

enum GET_PRODUCT_TYPE
{
    UNSUPPORT           = 0,
    BY_HOT_TYPE         = 1,//热门机型
    BY_CATEGORY_ID      = 2,//按类目ID
    BY_PRODUCT_DESC     = 3,//按产品描述
};

class CGetProductFilter
{
public:
    CGetProductFilter() : iType(0), iMid(0), iPagesize(0), iPageindex(0){}
    unsigned int iType;
    unsigned int iMid;
    std::string strSearch;
    unsigned int iPagesize;
    unsigned int iPageindex;
};

bool GetProductList(const std::string& strUserID, const std::string& strUserKey, bool bRawProucts, unsigned int iChannelID, const CGetProductFilter& oFilter, Json::Value& jsonOutput);
bool GetProductParameter(unsigned int iProductID, unsigned int iChannelID, const std::string& strUserID, const std::string& strUserKey, Json::Value& jsonOutput);

}

#endif


