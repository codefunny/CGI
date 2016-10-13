//======================================================================
//
//        Copyright (C) 2014 回收宝
//        All rights reserved
//
//        filename : price.h
//        description : 估价、报价
//
//        created by 叶飞 at  2014-08-01 10:00:00
//        modified by XX at
//
//======================================================================

#ifndef _PRICE_H_
#define _PRICE_H_

#include <string>

namespace logic
{

unsigned int GetQuotation(unsigned int iProductID, unsigned int iChannelID, const std::string& strUserID, const std::string& strUserKey, const std::string& strCheckResults, bool bIsCreateGoodsID, unsigned int& iGoodsID, unsigned int& iQuotation);
unsigned int GetQuotation(unsigned int iProductID, unsigned int iChannelID, unsigned int iValuation, unsigned int& iQuotation, int& iSpProfit, int& iSelfProfit);

bool GetQuotationMethod_1(unsigned int iValuation, unsigned int& iQuotation, int& iSpProfit, int& iSelfProfit);
bool GetQuotationMethod_2(unsigned int iValuation, unsigned int& iQuotation, int& iSpProfit, int& iSelfProfit);
bool GetQuotationMethod_3(unsigned int iValuation, unsigned int& iQuotation, int& iSpProfit, int& iSelfProfit);
bool GetQuotationMethod_4(unsigned int iValuation, unsigned int& iQuotation, int& iSpProfit, int& iSelfProfit);

bool ProductProfit(const std::string& strProfitMethod, unsigned int iValuation, int& iSelfProfit);

bool Allowance(unsigned int iProductID, unsigned int iChannelID, unsigned int iValuation, unsigned int& iQuotation, int& iSpProfit, int& iSelfProfit);

unsigned int BankersRoundingMethod(unsigned int iPriceInput);
int BankersRoundingMethod(int iPriceInput);

bool GetApproximationMethod_1000000(unsigned int& iQuotation);

}

#endif


