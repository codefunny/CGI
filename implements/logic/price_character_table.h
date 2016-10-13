//======================================================================
//
//        Copyright (C) 2014 回收宝
//        All rights reserved
//
//        filename : price_character_table.h
//        description : 产品价格属性表
//
//        created by 叶飞 at  2014-08-01 10:00:00
//        modified by XX at
//
//======================================================================

#ifndef _PRICE_CHARACTER_TABLE_H_
#define _PRICE_CHARACTER_TABLE_H_

#include <string>
#include <vector>
#include <map>

namespace logic
{

bool SelectPriceCharacterTable(unsigned int iPriceID, std::map<std::string, std::string>& priceCharacterInfo);
bool GetPriceCharacterName(const std::vector<unsigned int>& vecPriceID, std::string& strCharacterNameList);
bool SelectPriceCharacterTableByFatherID(unsigned int iPriceFatherID, std::vector< std::map<std::string, std::string> >& priceCharacterList);
bool GetPriceCharacterType(const std::vector<unsigned int>& vecPriceID, std::map<unsigned int, unsigned int>& priceCharacterType);
bool CheckFatherIDIsRight(const std::vector<unsigned int>& vecProduct, const std::vector<unsigned int>& vecChannel, const std::vector<unsigned int>& vecCheckResult, std::vector<unsigned int>& vecNewCheckResult);
}

#endif


