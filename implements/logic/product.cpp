#include "product.h"
#include <stdlib.h>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include "configure.h"
#include "error_num.h"
#include "string_tools.h"
#include "login.h"
#include "product_table.h"
#include "channel_table.h"
#include "price_character_table.h"
#include "category_table.h"
#include "price.h"
#include "memcached.h"

using namespace std;

static bool GetjsonMenu(const logic::CGetProductFilter& oFilter, Json::Value& jsonMenu)
{
    DEBUG("=======GetjsonMenu START=======");

    unsigned int iCategoryID = 0;
    switch (oFilter.iType)
    {
        case logic::BY_HOT_TYPE:
        {
            iCategoryID = 1;//根节点
            break;
        }
        case logic::BY_CATEGORY_ID:
        {
            iCategoryID = oFilter.iMid;
            break;
        }
        case logic::BY_PRODUCT_DESC:
        {
            iCategoryID = 1;//根节点
            break;
        }
        default:
        {
            ERROR("invalid request");
            return false;
        }
    }

    //自己
    map<string, string> itselfCategoryInfo;
    if (logic::SelectCategory(iCategoryID, itselfCategoryInfo))
    {
        if (itselfCategoryInfo.end() == itselfCategoryInfo.find("Fcategory_id"))
        {
            ERROR("Fcategory_id not found");
            return false;
        }
        jsonMenu["mid"] = itselfCategoryInfo["Fcategory_id"];

        if (itselfCategoryInfo.end() == itselfCategoryInfo.find("Fcategory_name"))
        {
            ERROR("Fcategory_name not found");
            return false;
        }
        jsonMenu["name"] = itselfCategoryInfo["Fcategory_name"];

        if (itselfCategoryInfo.end() == itselfCategoryInfo.find("Fsub_desc"))
        {
            ERROR("Fsub_desc not found");
            return false;
        }
        jsonMenu["subdesc"] = itselfCategoryInfo["Fsub_desc"];
    }
    else
    {
        ERROR("SelectCategory failed");
        return false;
    }

    //子导航
    Json::Value jsonSubtree;
    vector< map<string, string> > subCategoryList;
    if (logic::SelectSubCategoryDepthForOne(iCategoryID, subCategoryList))
    {
        Json::Value jsonSubtreeItem;

        size_t maxSize = subCategoryList.size();
        for (size_t index = 0; index < maxSize; index++)
        {
            if (subCategoryList[index].end() == subCategoryList[index].find("Fcategory_id"))
            {
                ERROR("Fcategory_id not found");
                return false;
            }
            jsonSubtreeItem["mid"] = subCategoryList[index]["Fcategory_id"];

            if (subCategoryList[index].end() == subCategoryList[index].find("Fcategory_name"))
            {
                ERROR("Fcategory_name not found");
                return false;
            }
            jsonSubtreeItem["name"] = subCategoryList[index]["Fcategory_name"];

            jsonSubtree.append(jsonSubtreeItem);
        }
    }
    else
    {
        ERROR("SelectSubCategoryDepthForOne failed");
        return false;
    }
    jsonMenu["subtree"] = jsonSubtree;

    DEBUG("========GetjsonMenu END========");
    return true;
}

bool logic::GetProductList(const string& strUserID, const string& strUserKey, bool bRawProucts, unsigned int iChannelID, const CGetProductFilter& oFilter, Json::Value& jsonOutput)
{
    jsonOutput["ret"] = "0";
    jsonOutput["errcode"] = "0";
    jsonOutput["errstr"] = "";

    if (!bRawProucts)
    {
        //特定渠道需要校验登录态
        bool bNeedLogin = false;
        unsigned int iTrueChannelID = 0;
        unsigned int iLoginState = user::CheckLandingStateWithChannel(::atoi(strUserID.c_str()), strUserKey, iChannelID, iTrueChannelID, bNeedLogin);
        if (error_num::SUCCEED != iLoginState)
        {
            ERROR("CheckLandingStateWithChannel failed");
            jsonOutput["ret"] = "1";
            jsonOutput["errcode"] = tools::CStringTools::Int2String(iLoginState);
            jsonOutput["errstr"] = "用户未登录";
            return false;
        }
        iChannelID = iTrueChannelID;
    }



    Json::Value jsonData;

    //产品列表
    Json::Value jsonProductlist;
    vector< map<string, string> > vecProductList;
    unsigned int iTotal = 0;
    if (!logic::SelectProductTable(oFilter, vecProductList, iTotal))
    {
        ERROR("get product list failed");
        jsonOutput["ret"] = "1";
        jsonOutput["errcode"] = tools::CStringTools::Int2String(error_num::PRODUCT_UNKNOWN);
        jsonOutput["errstr"] = "产品不存在";
        return false;
    }
    size_t maxSize = vecProductList.size();
    for (size_t index = 0; index < maxSize; index++)
    {
        Json::Value jsonProduct;
        map<string, string> productInfo = vecProductList[index];
        if (productInfo.end() == productInfo.find("Fproduct_id"))
        {
            ERROR("Fproduct_id not found");
            continue;
        }
        jsonProduct["pid"] = productInfo["Fproduct_id"];

        if (productInfo.end() == productInfo.find("Fproduct_name"))
        {
            ERROR("Fproduct_name not found");
            continue;
        }
        jsonProduct["name"] = productInfo["Fproduct_name"];

        if (productInfo.end() == productInfo.find("Fpic_id"))
        {
            ERROR("Fpic_id not found");
            continue;
        }
        jsonProduct["picurl"] = productInfo["Fpic_id"];

        if (!bRawProucts)
        {
            if (productInfo.end() == productInfo.find("Fmax_price"))
            {
                ERROR("Fmax_price not found");
                continue;
            }
            //jsonProduct["maxprice"] = productInfo["Fmax_price"];
            //最高价也需要按报价算法计算一次
            static int useNum = 1;
            string strValue;
            tools::CMemcached oMemcached;

            bool istrue = oMemcached.Get("maxPrice|" + productInfo["Fproduct_id"] , strValue);
            if(istrue)
            {
                ++useNum;
            }
            if ((0 == useNum%20) || !istrue )
            {
                ERROR("failed to get maxprice from memcached key: maxPrice|" + productInfo["Fproduct_id"]);

                unsigned int iMaxprice = 0;
                int iSpProfit = 0;
                int iSelfProfit = 0;
                if (error_num::SUCCEED != logic::GetQuotation(::atoi(productInfo["Fproduct_id"].c_str()), iChannelID, ::atoi(productInfo["Fmax_price"].c_str()), iMaxprice, iSpProfit, iSelfProfit))
                {
                    ERROR("GetQuotation failed ,Productid:" + productInfo["Fproduct_id"]);
                    continue;
                }
                strValue = tools::CStringTools::Int2String(iMaxprice);
                if(!oMemcached.Set("maxPrice|" + productInfo["Fproduct_id"], strValue))
                {
                    ERROR("failed to set maxprice to memcached key: maxPrice|" + productInfo["Fproduct_id"]);
                }
                else
                {
                    DEBUG("set maxprice to memcached key:maxPrice|" + productInfo["Fproduct_id"] + " prodname: " + productInfo["Fproduct_name"] + " price: " + strValue);
                }
            }
            jsonProduct["maxprice"] = strValue;
        }

        jsonProductlist.append(jsonProduct);
    }
    jsonData["productlist"] = jsonProductlist;



    //导航列表
    Json::Value jsonMenu;
    if (!GetjsonMenu(oFilter, jsonMenu))
    {
        jsonOutput["ret"] = "1";
        jsonOutput["errcode"] = "0";
        jsonOutput["errstr"] = "获取导航失败";
        return false;
    }

    jsonData["menu"] = jsonMenu;

    Json::Value jsonPageinfo;
    jsonPageinfo["pagesize"] = tools::CStringTools::Int2String(oFilter.iPagesize);
    jsonPageinfo["pageindex"] = tools::CStringTools::Int2String(oFilter.iPageindex);
    jsonPageinfo["total"] = tools::CStringTools::Int2String(iTotal);
    jsonData["pageinfo"] = jsonPageinfo;

    jsonOutput["ret"] = "0";
    jsonOutput["errcode"] = "0";
    jsonOutput["errstr"] = "";
    jsonOutput["data"] = jsonData;
    return true;
}

static bool GetPriceCharacter(unsigned int iPriceID, Json::Value& jsonItemListItem)
{
    jsonItemListItem["id"] = tools::CStringTools::Int2String(iPriceID);

    map<string, string> mapPriceCharacterTable;
    if (!logic::SelectPriceCharacterTable(iPriceID, mapPriceCharacterTable))
    {
        ERROR("SelectPriceCharacterTable failed");
        return false;
    }
    if (mapPriceCharacterTable.end() == mapPriceCharacterTable.find("Fprice_name"))
    {
        ERROR("Fprice_name is not found");
        return false;
    }
    DEBUG("Fprice_name is : " + mapPriceCharacterTable["Fprice_name"]);
    jsonItemListItem["name"] = mapPriceCharacterTable["Fprice_name"];



    Json::Value jsonQuestion;
    vector< map<string, string> > vecSubPriceCharacter;
    if (!logic::SelectPriceCharacterTableByFatherID(iPriceID, vecSubPriceCharacter))
    {
        ERROR("SelectPriceCharacterTableByFatherID failed");
        return false;
    }
    size_t maxVec = vecSubPriceCharacter.size();
    for (size_t iIndex = 0; iIndex < maxVec; iIndex++)
    {
        map<string, string> SubPriceCharacterItem = vecSubPriceCharacter[iIndex];
        if (SubPriceCharacterItem.end() == SubPriceCharacterItem.find("Fprice_id"))
        {
            ERROR("Fprice_id is not found");
            return false;
        }
        if (SubPriceCharacterItem.end() == SubPriceCharacterItem.find("Fprice_name"))
        {
            ERROR("Fprice_name is not found");
            return false;
        }
        DEBUG("Fprice_id is : " + SubPriceCharacterItem["Fprice_id"] + ", Fprice_name is : " + SubPriceCharacterItem["Fprice_name"]);

        Json::Value jsonQuestionItem;
        jsonQuestionItem["id"] = SubPriceCharacterItem["Fprice_id"];
        jsonQuestionItem["name"] = SubPriceCharacterItem["Fprice_name"];
        jsonQuestion.append(jsonQuestionItem);
    }

    jsonItemListItem["question"] = jsonQuestion;
    return true;
}

bool logic::GetProductParameter(unsigned int iProductID, unsigned int iChannelID, const string& strUserID, const string& strUserKey, Json::Value& jsonOutput)
{
    jsonOutput["ret"] = "0";
    jsonOutput["errcode"] = "0";
    jsonOutput["errstr"] = "";

    //特定渠道需要校验登录态
    bool bNeedLogin = false;
    unsigned int iTrueChannelID = 0;
    unsigned int iLoginState = user::CheckLandingStateWithChannel(::atoi(strUserID.c_str()), strUserKey, iChannelID, iTrueChannelID, bNeedLogin);
    if (error_num::SUCCEED != iLoginState)
    {
        ERROR("CheckLandingStateWithChannel failed");
        jsonOutput["ret"] = "1";
        jsonOutput["errcode"] = tools::CStringTools::Int2String(iLoginState);
        jsonOutput["errstr"] = "用户未登录";
        return false;
    }
    iChannelID = iTrueChannelID;



    Json::Value jsonData;
    jsonData["pid"] = iProductID;

    //产品表
    map<string, string> mapProductTable;
    if (!logic::SelectProductTable(iProductID, mapProductTable))
    {
        ERROR("SelectProductTable failed");
        jsonOutput["ret"] = "1";
        jsonOutput["errcode"] = tools::CStringTools::Int2String(error_num::PRODUCT_UNKNOWN);
        jsonOutput["errstr"] = "产品不存在";
        return false;
    }
    if (mapProductTable.end() == mapProductTable.find("Fproduct_name"))
    {
        ERROR("Fproduct_name is not found");
        jsonOutput["ret"] = "1";
        jsonOutput["errcode"] = tools::CStringTools::Int2String(error_num::PRODUCT_UNKNOWN);
        jsonOutput["errstr"] = "产品不存在";
        return false;
    }
    DEBUG("Fproduct_name is : " + mapProductTable["Fproduct_name"]);
    jsonData["productname"] = mapProductTable["Fproduct_name"];

    if (mapProductTable.end() == mapProductTable.find("Fpic_id"))
    {
        ERROR("Fpic_id is not found");
        jsonOutput["ret"] = "1";
        jsonOutput["errcode"] = tools::CStringTools::Int2String(error_num::PRODUCT_PIC_UNKNOWN);
        jsonOutput["errstr"] = "产品图片不存在";
        return false;
    }
    DEBUG("Fpic_id is : " + mapProductTable["Fpic_id"]);
    jsonData["picurl"] = mapProductTable["Fpic_id"];

    if (mapProductTable.end() == mapProductTable.find("Fmax_price"))
    {
        ERROR("Fmax_price is not found");
        jsonOutput["ret"] = "1";
        jsonOutput["errcode"] = tools::CStringTools::Int2String(error_num::PRODUCT_MAXPRICE_UNKNOWN);
        jsonOutput["errstr"] = "产品最高价格不存在";
        return false;
    }
    DEBUG("Fmax_price is : " + mapProductTable["Fmax_price"]);
    //jsonData["maxprice"] = mapProductTable["Fmax_price"];
    //最高价也需要按报价算法计算一次
    unsigned int iMaxprice = 0;
    int iSpProfit = 0;
    int iSelfProfit = 0;
    if (error_num::SUCCEED != logic::GetQuotation(iProductID, iChannelID, ::atoi(mapProductTable["Fmax_price"].c_str()), iMaxprice, iSpProfit, iSelfProfit))
    {
        ERROR("GetQuotation failed");
        jsonOutput["ret"] = "1";
        jsonOutput["errcode"] = tools::CStringTools::Int2String(error_num::DB_QUOTATION);
        jsonOutput["errstr"] = "系统错误";
        return false;

    }
    jsonData["maxprice"] = tools::CStringTools::Int2String(iMaxprice);

    Json::Value jsonItemList;
    if (mapProductTable.end() == mapProductTable.find("Fshow_ways"))
    {
        ERROR("Fshow_ways is not found");
        jsonOutput["ret"] = "1";
        jsonOutput["errcode"] = tools::CStringTools::Int2String(error_num::PRODUCT_SHOWWAYS_UNKNOWN);
        jsonOutput["errstr"] = "产品最高价格不存在";
        return false;
    }
    DEBUG("Fshow_ways is : " + mapProductTable["Fshow_ways"]);//1#3#4#16#17#
    vector<unsigned int> vecProductPriceCharacter = tools::CStringTools::Split2Uint(mapProductTable["Fshow_ways"], "#");
    size_t maxVec = vecProductPriceCharacter.size();
    for (size_t iIndex = 0; iIndex < maxVec; iIndex++)
    {
        Json::Value jsonItemListItem;
        if (!GetPriceCharacter(vecProductPriceCharacter[iIndex], jsonItemListItem))
        {
            ERROR("GetPriceCharacter failed");
            jsonOutput["ret"] = "1";
            jsonOutput["errcode"] = tools::CStringTools::Int2String(error_num::DB_PRICE_CHARACTER);
            jsonOutput["errstr"] = "系统错误";
            return false;
        }
        jsonItemList.append(jsonItemListItem);
    }



    //渠道表
    map<string, string> mapChannelTable;
    if (!logic::SelectChannelTable(iChannelID, mapChannelTable))
    {
        ERROR("SelectChannelTable failed");
        jsonOutput["ret"] = "1";
        jsonOutput["errcode"] = tools::CStringTools::Int2String(error_num::CHANNEL_UNKNOWN);
        jsonOutput["errstr"] = "渠道不存在";
        return false;
    }
    if (mapChannelTable.end() == mapChannelTable.find("Fshow_ways"))
    {
        ERROR("Fshow_ways is not found");
        jsonOutput["ret"] = "1";
        jsonOutput["errcode"] = tools::CStringTools::Int2String(error_num::VALUATION_SHOWWAYS_UNKNOWN);
        jsonOutput["errstr"] = "估价展示条目不存在";
        return false;
    }
    DEBUG("Fshow_ways is : " + mapChannelTable["Fshow_ways"]);//'11#13#14'
    vector<unsigned int> vecChannelPriceCharacter = tools::CStringTools::Split2Uint(mapChannelTable["Fshow_ways"], "#");
    maxVec = vecChannelPriceCharacter.size();
    for (size_t iIndex = 0; iIndex < maxVec; iIndex++)
    {
        Json::Value jsonItemListItem;
        if (!GetPriceCharacter(vecChannelPriceCharacter[iIndex], jsonItemListItem))
        {
            ERROR("GetPriceCharacter failed");
            jsonOutput["ret"] = "1";
            jsonOutput["errcode"] = tools::CStringTools::Int2String(error_num::DB_PRICE_CHARACTER);
            jsonOutput["errstr"] = "系统错误";
            return false;
        }
        jsonItemList.append(jsonItemListItem);
    }

    jsonData["itemList"] = jsonItemList;
    jsonOutput["data"] = jsonData;
    jsonOutput["errcode"] = tools::CStringTools::Int2String(error_num::SUCCEED);
    jsonOutput["errstr"] = "";
    return true;
}


