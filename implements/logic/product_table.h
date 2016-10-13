//======================================================================
//
//        Copyright (C) 2014 回收宝
//        All rights reserved
//
//        filename : product_table.h
//        description : 产品表
//
//        created by 叶飞 at  2014-08-01 10:00:00
//        modified by XX at  
//
//======================================================================

#ifndef _PRODUCT_TABLE_H_
#define _PRODUCT_TABLE_H_

#include <string>
#include <vector>
#include <map>
#include "product.h"

namespace logic
{

class CProductTableData
{
public:
    CProductTableData() : iProductID(0), iCategoryID(0), iMaxPrice(0), iUpdater(0){}
    unsigned int iProductID;
    unsigned int iCategoryID;
    std::string strProductName;
    std::string strProductDesc;
    unsigned int iMaxPrice;
    std::string strShowWays;
    std::string strPriceList;
    std::string strPicID;
    unsigned int iUpdater;
};

bool SelectProductTable(unsigned int iProductID, std::map<std::string, std::string>& productInfo);
bool SelectProductTable(const logic::CGetProductFilter& oFilter, std::vector< std::map<std::string, std::string> >& vecProductList, unsigned int& iTotal);

bool UpdateProductTable(const CProductTableData& productTableData);

}

#endif



