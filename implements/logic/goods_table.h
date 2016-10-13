//======================================================================
//
//        Copyright (C) 2014 回收宝
//        All rights reserved
//
//        filename : goods_table.h
//        description : 商品表
//
//        created by 叶飞 at  2014-08-01 10:00:00
//        modified by XX at  
//
//======================================================================

#ifndef _GOODS_TABLE_H_
#define _GOODS_TABLE_H_

#include <string>
#include <map>

namespace logic
{

bool InsertGoodsTableForPrice(unsigned int iProductID, unsigned int iChannelID,
                              unsigned int iSeller, const std::string& strPriceInputResult, unsigned int iValuation, unsigned int iQuotation, unsigned int iSpProfit, unsigned int iSelfProfit,
                              unsigned int& iGoodsID);
bool SelectGoodsTable(unsigned int iGoodsID, std::map<std::string, std::string>& goodsInfo);
bool UpdateGoodsTableForFseller(unsigned int iGoodID, unsigned int iSeller);

}

#endif

