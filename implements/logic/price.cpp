#include "price.h"
#include <stdlib.h>
#include <vector>
#include <map>
#include "configure.h"
#include "error_num.h"
#include "string_tools.h"
#include "login.h"
#include "user_table.h"
#include "channel_table.h"
#include "price_character_table.h"
#include "product_table.h"
#include "goods_table.h"
#include "allowance_table.h"

using namespace std;

//估价
//iProductID 产品ID
//vecValuationWays 估价算法
//mapPriceInputResult 检测结果
static bool GetValuation(unsigned int iProductID, const vector<unsigned int>& vecValuationWays, const vector<unsigned int>& vecCheckResults, unsigned int& iValuation)
{
    DEBUG("=======Valuation START=======");

    //1、根据产品ID查询出最高价格 和 产品价格属性ID列表（系数或值）
    //1.1、最高价格
    map<string, string> mapProductTable;
    if (!logic::SelectProductTable(iProductID, mapProductTable))
    {
        ERROR("SelectProductTable failed");
        return false;
    }
    if (mapProductTable.end() == mapProductTable.find("Fmax_price"))
    {
        ERROR("Fmax_price is not found");
        return false;
    }
    DEBUG("Fmax_price is : " + mapProductTable["Fmax_price"]);
    unsigned int iMaxPrice = ::atoi(mapProductTable["Fmax_price"].c_str());
    iValuation = iMaxPrice;

    //1.2、确定实际产品最高价
    if (mapProductTable.end() == mapProductTable.find("Fcalc_ways"))
    {
        ERROR("Fcalc_ways is not found");
        return false;
    }
    DEBUG("Fcalc_ways is : " + mapProductTable["Fcalc_ways"]);//1#3#4#16#17#
    vector<unsigned int> vecProductCalcWays = tools::CStringTools::Split2Uint(mapProductTable["Fcalc_ways"], "#");

    //1.3、产品价格属性ID列表（系数或值）
    if (mapProductTable.end() == mapProductTable.find("Fprice_list"))
    {
        ERROR("Fprice_list is not found");
        return false;
    }
    DEBUG("Fprice_list is : " + mapProductTable["Fprice_list"]);//1101:100#1102:20#1201:100#1202:40#1301:100#1302:20#1401:0#1402:10#1403:20#1404:30#1501:0#1502:10#1701:100#1702:90#1703:20#1801:0#1802:50#
    map<unsigned int, unsigned int> mapPriceValue = tools::CStringTools::SplitTwice2Uint(mapProductTable["Fprice_list"], "#", ":");

    //1.4、进一步查询出价格类型列表
    vector<unsigned int> vecPriceID;
    map<unsigned int, unsigned int>::iterator iter = mapPriceValue.begin();
    const map<unsigned int, unsigned int>::const_iterator iterEnd = mapPriceValue.end();
    for (; iter != iterEnd; ++iter)
    {
        vecPriceID.push_back(iter->first);
    }
    map<unsigned int, unsigned int> mapPriceType;
    if (!logic::GetPriceCharacterType(vecPriceID, mapPriceType))
    {
        ERROR("GetPriceCharacterType failed");
        return false;
    }

    //1.5、校验vecProductShowWays + vecValuationWays 是否等于 vecCheckResults
    //1#3#4#16#17#
    //14#13#11#
    //101#1601#1701#1101#1301#1401#
    vector<unsigned int> vecNewCheckResult;
    if (!logic::CheckFatherIDIsRight(vecProductCalcWays, vecValuationWays, vecCheckResults, vecNewCheckResult))
    {
        ERROR("check list is imcomplete");
        return false;
    }



    //2.0、根据最高价格 + 产品价格属性值 + 估价算法 + 检测结果，计算出估价
    if (mapPriceValue.size() != mapPriceType.size())
    {
        ERROR("DB error");
        return false;
    }

    unsigned int iMaxCheckResults = vecNewCheckResult.size();
    for (unsigned int iIndex = 0; iIndex < iMaxCheckResults; iIndex++)
    {
        if (mapPriceType.end() == mapPriceType.find(vecNewCheckResult[iIndex]))
        {
            ERROR("DB error for character[" + tools::CStringTools::Int2String(vecNewCheckResult[iIndex]) + "]");
            return false;
        }
        if (mapPriceValue.end() == mapPriceValue.find(vecNewCheckResult[iIndex]))
        {
            ERROR("DB error for character[" + tools::CStringTools::Int2String(vecNewCheckResult[iIndex]) + "]");
            return false;
        }

        unsigned int iType = mapPriceType[vecNewCheckResult[iIndex]];
        unsigned int iValue = mapPriceValue[vecNewCheckResult[iIndex]];
        DEBUG("check for character[" + tools::CStringTools::Int2String(vecNewCheckResult[iIndex]) + "]"
              + ", type[" + tools::CStringTools::Int2String(iType) + "]"
              + ", value[" + tools::CStringTools::Int2String(iValue) + "]");

        if (PRICE_CHARACTER_TYPE_MULTIPLICATION == iType)//乘法
        {
            if (iValue > 100)
            {
                ERROR("DB error");
                return false;
            }
            iValuation = iValuation * iValue / 100;
        }
        else if (PRICE_CHARACTER_TYPE_SUBTRACTION == iType)//减法
        {
            iValuation = iValuation > iValue ? iValuation - iValue : 0;
        }
        else if (PRICE_CHARACTER_TYPE_FUNCTION == iType)//函数
        {
            //do nothing
        }
        else if (PRICE_CHARACTER_TYPE_SHOW == iType)
        {
            //do nothing
        }
        else if (PRICE_CHARACTER_TYPE_FIXED == iType)
        {
            iValuation = iValue;
        }
        else
        {
            ERROR("Input error");
            return false;
        }

        DEBUG("current valuation is " + tools::CStringTools::Int2String(iValuation));

    }

    DEBUG("========Valuation END========");
    return true;
}

//报价
unsigned int logic::GetQuotation(unsigned int iProductID, unsigned int iChannelID, const string& strUserID, const string& strUserKey, const string& strCheckResults, bool bIsCreateGoodsID, unsigned int& iGoodsID, unsigned int& iQuotation)
{
    DEBUG("=======Quotation START=======");

    //特定渠道需要校验登录态
    bool bNeedLogin = false;
    unsigned int iTrueChannelID = 0;
    unsigned int iLoginState = user::CheckLandingStateWithChannel(::atoi(strUserID.c_str()), strUserKey, iChannelID, iTrueChannelID, bNeedLogin);
    if (error_num::SUCCEED != iLoginState)
    {
        ERROR("CheckLandingStateWithChannel failed");
        return iLoginState;
    }
    iChannelID = iTrueChannelID;

    //1.0、根据渠道号查询出估价算法、报价ID
    map<string, string> mapChannelTable;
    if (!logic::SelectChannelTable(iChannelID, mapChannelTable))
    {
        ERROR("SelectChannelTable failed");
        return error_num::DB_CHANNEL;
    }
    if (mapChannelTable.end() == mapChannelTable.find("Fcalc_ways"))
    {
        ERROR("Fcalc_ways is not found");
        return error_num::DB_VALUATION;
    }
    //1.1、估价算法 14#13#11#
    DEBUG("Fcalc_ways is : " + mapChannelTable["Fcalc_ways"]);
    vector<unsigned int> vecCalcWays = tools::CStringTools::Split2Uint(mapChannelTable["Fcalc_ways"], "#");

    //1.2、报价ID
    /*
    if (mapChannelTable.end() == mapChannelTable.find("Fquotation_id"))
    {
        ERROR("Fquotation_id is not found");
        return error_num::DB_QUOTATION;
    }
    DEBUG("Fquotation_id is : " + mapChannelTable["Fquotation_id"]);
    unsigned int iQuotationID = ::atoi(mapChannelTable["Fquotation_id"].c_str());
    */



    //2.0、开始估价
    unsigned int iValuation = 0;
    vector<unsigned int> vecCheckResults = tools::CStringTools::Split2Uint(strCheckResults, "#");
    if (!GetValuation(iProductID, vecCalcWays, vecCheckResults, iValuation))
    {
        ERROR("GetValuation failed");
        return error_num::VALUATION_FAILED;
    }
    DEBUG("Valuation result is : " + tools::CStringTools::Int2String(iValuation));



    //3.0、开始报价
    int iSpProfit = 0;
    int iSelfProfit = 0;
    if (error_num::SUCCEED != GetQuotation(iProductID, iChannelID, iValuation, iQuotation, iSpProfit, iSelfProfit))
    {
        ERROR("GetQuotation failed");
        return error_num::QUOTATION_FAILED;
    }

    if (bIsCreateGoodsID)
    {
        if (bNeedLogin)
        {
            if (!logic::InsertGoodsTableForPrice(iProductID, iChannelID, ::atoi(strUserID.c_str()), strCheckResults, iValuation, iQuotation, iSpProfit, iSelfProfit, iGoodsID))
            {
                ERROR("save goods info failed");
                return error_num::GOODS_FAILED;
            }
        }
        else
        {
            if (!logic::InsertGoodsTableForPrice(iProductID, iChannelID, 0, strCheckResults, iValuation, iQuotation, iSpProfit, iSelfProfit, iGoodsID))
            {
                ERROR("save goods info failed");
                return error_num::GOODS_FAILED;
            }
        }
    }

    DEBUG("========Quotation END========");
    return error_num::SUCCEED;
}

//中山联通/杭州联通按台给店奖，500元以下提成0元，500-1000提成30元/台，1000以上提成50元/台。
//回收宝按台分成，150元以下提成0元，150-300提成20元，300-450提40元，450-700，提成60元，700-1000提成100元，1000以上提成150元
bool logic::GetQuotationMethod_1(unsigned int iValuation, unsigned int& iQuotation, int& iSpProfit, int& iSelfProfit)
{
    iSelfProfit = iValuation * 10 / 100;
    if (iValuation < 15000)
    {
        iSelfProfit = 0;
    }
    else if (15000 <= iValuation && iValuation < 30000)
    {
        iSelfProfit = 2000;
    }
    else if (30000 <= iValuation && iValuation < 45000)
    {
        iSelfProfit = 4000;
    }
    else if (45000 <= iValuation && iValuation < 70000)
    {
        iSelfProfit = 6000;
    }
    else if (70000 <= iValuation && iValuation < 100000)
    {
        iSelfProfit = 10000;
    }
    else if (100000 <= iValuation)
    {
        iSelfProfit = 15000;
    }
    else
    {
        return false;
    }

    if (iValuation < 50000)
    {
        iSpProfit = 0;
    }
    else if (50000 <= iValuation && iValuation < 100000)
    {
        iSpProfit = 3000;
    }
    else if (100000 <= iValuation)
    {
        iSpProfit = 5000;
    }
    else
    {
        return false;
    }

    iQuotation = iValuation - iSelfProfit - iSpProfit;

    return true;
}

//广州广信取实际报价的6%作为渠道分成
//回收宝按台分成，150元以下提成0元，150-300提成20元，300-450提40元，450-700，提成60元，700-1000提成100元，1000以上提成150元
bool logic::GetQuotationMethod_2(unsigned int iValuation, unsigned int& iQuotation, int& iSpProfit, int& iSelfProfit)
{
    if (iValuation < 15000)
    {
        iSelfProfit = 0;
    }
    else if (15000 <= iValuation && iValuation < 30000)
    {
        iSelfProfit = 2000;
    }
    else if (30000 <= iValuation && iValuation < 45000)
    {
        iSelfProfit = 4000;
    }
    else if (45000 <= iValuation && iValuation < 70000)
    {
        iSelfProfit = 6000;
    }
    else if (70000 <= iValuation && iValuation < 100000)
    {
        iSelfProfit = 10000;
    }
    else if (100000 <= iValuation)
    {
        iSelfProfit = 15000;
    }
    else
    {
        return false;
    }

    iSpProfit = iValuation * 6 / 100;

    iQuotation = iValuation - iSelfProfit - iSpProfit;

    return true;
}

//国信灵通取实际报价的5%作为渠道分成
//回收宝按台分成，150元以下提成0元，150-300提成20元，300-450提40元，450-700，提成60元，700-1000提成100元，1000以上提成150元
bool logic::GetQuotationMethod_3(unsigned int iValuation, unsigned int& iQuotation, int& iSpProfit, int& iSelfProfit)
{
    if (iValuation < 15000)
    {
        iSelfProfit = 0;
    }
    else if (15000 <= iValuation && iValuation < 30000)
    {
        iSelfProfit = 2000;
    }
    else if (30000 <= iValuation && iValuation < 45000)
    {
        iSelfProfit = 4000;
    }
    else if (45000 <= iValuation && iValuation < 70000)
    {
        iSelfProfit = 6000;
    }
    else if (70000 <= iValuation && iValuation < 100000)
    {
        iSelfProfit = 10000;
    }
    else if (100000 <= iValuation)
    {
        iSelfProfit = 15000;
    }
    else
    {
        return false;
    }

    iSpProfit = iValuation * 5 / 100;

    iQuotation = iValuation - iSelfProfit - iSpProfit;

    return true;
}

//全部0分成
bool logic::GetQuotationMethod_4(unsigned int iValuation, unsigned int& iQuotation, int& iSpProfit, int& iSelfProfit)
{
    return true;
}

bool logic::Allowance(unsigned int iProductID, unsigned int iChannelID, unsigned int iValuation, unsigned int& iQuotation, int& iSpProfit, int& iSelfProfit)
{
    int iSpAllowance = 0;
    int iSelfAllowance = 0;
    if (!logic::SelectAllowanceTable(iProductID, iChannelID, iSpAllowance, iSelfAllowance))
    {
        ERROR("SelectAllowanceTable failed");
        return false;
    }
    iSpProfit += iSpAllowance;
    iSelfProfit += iSelfAllowance;

    if (iValuation - iSpProfit - iSelfProfit < 0)//有问题
    {
        WARN("Quotation would be less than 0");
        return false;
    }
    else
    {
        iQuotation = iValuation - iSpProfit - iSelfProfit;
        return true;
    }

    return true;
}

unsigned int logic::BankersRoundingMethod(unsigned int iPriceInput)
{
    if ((iPriceInput / 10) % 10 <= 4)//当舍去位的数值小于5时，直接舍去该位
    {
        return (iPriceInput / 100) * 100;
    }
    else if ((iPriceInput / 10) % 10 >= 6)//当舍去位的数值大于等于6时，在舍去该位的同时向前位进一
    {
        return (iPriceInput / 100 + 1) * 100;
    }
    else//当舍去位的数值等于5时，如果前位数值为奇，则在舍去该位的同时向前位进一，如果前位数值为偶，则直接舍去该位
    {
        unsigned int aheader = iPriceInput / 100 % 10;
        if (0 != aheader % 2)
        {
            return (iPriceInput / 100 + 1) * 100;
        }
        else
        {
            return (iPriceInput / 100) * 100;
        }
    }

    return 0;
}

int logic::BankersRoundingMethod(int iPriceInput)
{
    if ((iPriceInput / 10) % 10 <= 4)//当舍去位的数值小于5时，直接舍去该位
    {
        return (iPriceInput / 100) * 100;
    }
    else if ((iPriceInput / 10) % 10 >= 6)//当舍去位的数值大于等于6时，在舍去该位的同时向前位进一
    {
        return (iPriceInput / 100 + 1) * 100;
    }
    else//当舍去位的数值等于5时，如果前位数值为奇，则在舍去该位的同时向前位进一，如果前位数值为偶，则直接舍去该位
    {
        int aheader = iPriceInput / 100 % 10;
        if (0 != aheader % 2)
        {
            return (iPriceInput / 100 + 1) * 100;
        }
        else
        {
            return (iPriceInput / 100) * 100;
        }
    }

    return 0;
}

//对用户报价进行十位数的银行家舎入
bool logic::GetApproximationMethod_1000000(unsigned int& iQuotation)
{
    iQuotation = (logic::BankersRoundingMethod(iQuotation / 10)) * 10;

    return true;
}

bool logic::ProductProfit(const string& strProfitMethod, unsigned int iValuation, int& iSelfProfit)
{
    DEBUG("profit method is " + strProfitMethod + ", valuation is " + tools::CStringTools::Int2String(iValuation));

    vector<string> vecMethodList = tools::CStringTools::Split2String(strProfitMethod, "#");
    size_t maxMethodSize = vecMethodList.size();
    for (size_t index = 0; index < maxMethodSize; index++)
    {
        vector<unsigned int> vecMethodItemList = tools::CStringTools::Split2Uint(vecMethodList[index], ":");
        if (4 != vecMethodItemList.size())
        {
            ERROR("profit method is illegal");
            return false;
        }
        unsigned int iProfitType = vecMethodItemList[0];
        unsigned int iProfitValue = vecMethodItemList[1];
        unsigned int iProfitStart = vecMethodItemList[2];
        unsigned int iProfitEnd = vecMethodItemList[3];
        DEBUG("profit type  is " + tools::CStringTools::Int2String(iProfitType));
        DEBUG("profit value is " + tools::CStringTools::Int2String(iProfitValue));
        DEBUG("profit start is " + tools::CStringTools::Int2String(iProfitStart));
        DEBUG("profit end   is " + tools::CStringTools::Int2String(iProfitEnd));
        if (iProfitStart <= iValuation && iValuation <= iProfitEnd)
        {
            if (PRODUCT_PROFIT_TYPE_PERCENT == iProfitType)
            {
                int iSelfProfitBefore = iSelfProfit;
                iSelfProfit += iValuation * iProfitValue / 100;
                DEBUG("self profit was modified from " + tools::CStringTools::Int2String(iSelfProfitBefore) + " to " + tools::CStringTools::Int2String(iSelfProfit));
                return true;
            }
            else if (PRODUCT_PROFIT_TYPE_ABSOLUTE == iProfitType)
            {
                int iSelfProfitBefore = iSelfProfit;
                iSelfProfit += iProfitValue;
                DEBUG("self profit was modified from " + tools::CStringTools::Int2String(iSelfProfitBefore) + " to " + tools::CStringTools::Int2String(iSelfProfit));
                return true;
            }
            else
            {
                WARN("profit type is illegal");
            }
        }
    }
    DEBUG("self profit was not modified");
    return true;
}

unsigned int logic::GetQuotation(unsigned int iProductID, unsigned int iChannelID, unsigned int iValuation, unsigned int& iQuotation, int& iSpProfit, int& iSelfProfit)
{
    DEBUG("=======Quotation START=======");

    map<string, string> mapChannelTable;
    if (!logic::SelectChannelTable(iChannelID, mapChannelTable))
    {
        ERROR("SelectChannelTable failed");
        return error_num::DB_CHANNEL;
    }
    if (mapChannelTable.end() == mapChannelTable.find("Fquotation_id"))
    {
        ERROR("Fquotation_id is not found");
        return error_num::DB_QUOTATION;
    }
    DEBUG("Fquotation_id is : " + mapChannelTable["Fquotation_id"]);
    unsigned int iQuotationID = ::atoi(mapChannelTable["Fquotation_id"].c_str());
    if (mapChannelTable.end() == mapChannelTable.find("Fapproximation_id"))
    {
        ERROR("Fapproximation_id is not found");
        return error_num::DB_APPROXIMATION;
    }
    DEBUG("Fapproximation_id is : " + mapChannelTable["Fapproximation_id"]);
    unsigned int iApproximationID = ::atoi(mapChannelTable["Fapproximation_id"].c_str());

    //报价，仅和渠道有关系
    switch (iQuotationID)
    {
        case 1:
        {
            if (!GetQuotationMethod_1(iValuation, iQuotation, iSpProfit, iSelfProfit))
            {
                ERROR("GetQuotationMethod_1 failed");
                return error_num::QUOTATION_FAILED;
            }
            break;
        }
        case 2:
        {
            if (!GetQuotationMethod_2(iValuation, iQuotation, iSpProfit, iSelfProfit))
            {
                ERROR("GetQuotationMethod_2 failed");
                return error_num::QUOTATION_FAILED;
            }
            break;
        }
        case 3:
        {
            if (!GetQuotationMethod_3(iValuation, iQuotation, iSpProfit, iSelfProfit))
            {
                ERROR("GetQuotationMethod_3 failed");
                return error_num::QUOTATION_FAILED;
            }
            break;
        }
        case 4:
        {
            if (!GetQuotationMethod_4(iValuation, iQuotation, iSpProfit, iSelfProfit))
            {
                ERROR("GetQuotationMethod_3 failed");
                return error_num::QUOTATION_FAILED;
            }
            break;
        }
        default:
        {
            ERROR("QuotationID is invalid");
            return error_num::QUOTATION_UNSUPPORT;
        }
    }
    DEBUG("Valuation is : " + tools::CStringTools::Int2String(iValuation)
      + ", Quotation is : " + tools::CStringTools::Int2String(iQuotation)
      + ", SpProfit is : " + tools::CStringTools::Int2String(iSpProfit)
      + ", SelfProfit is : " + tools::CStringTools::Int2String(iSelfProfit));

    //利润，仅和产品有关系
    map<string, string> mapProductTable;
    if (!logic::SelectProductTable(iProductID, mapProductTable))
    {
        ERROR("SelectProductTable failed");
        return error_num::PRODUCT_UNKNOWN;
    }
    if (mapProductTable.end() == mapProductTable.find("Fprofit_plan_method"))
    {
        ERROR("Fprofit_plan_method is not found");
        return error_num::PRODUCT_PROFIT_METHOD;
    }
    DEBUG("Fprofit_plan_method is : " + mapProductTable["Fprofit_plan_method"]);
    string strProfitMethod = mapProductTable["Fprofit_plan_method"];
    do
    {
        if (ProductProfit(strProfitMethod, iValuation, iSelfProfit))
        {
            if (iSpProfit > 0 && iValuation < (unsigned int)iSpProfit)
            {
                iSpProfit = iValuation;
                iSelfProfit = 0;
                iQuotation = 0;
                break;
            }
            
            if (iSelfProfit > 0 && (unsigned int)(iValuation - iSpProfit) < (unsigned int)iSelfProfit)
            {
                iSelfProfit = iValuation - iSpProfit;
                iQuotation = 0;
                break;
            }
            
            iQuotation = iValuation - iSpProfit - iSelfProfit;
        }
    }while(0);
    DEBUG("Valuation is : " + tools::CStringTools::Int2String(iValuation)
      + ", Quotation is : " + tools::CStringTools::Int2String(iQuotation)
      + ", SpProfit is : " + tools::CStringTools::Int2String(iSpProfit)
      + ", SelfProfit is : " + tools::CStringTools::Int2String(iSelfProfit));

    //补贴，和产品渠道都有关系
    if (!Allowance(iProductID, iChannelID, iValuation, iQuotation, iSpProfit, iSelfProfit))
    {
        ERROR("Allowance failed");
        return error_num::QUOTATION_FAILED;
    }
    DEBUG("Valuation is : " + tools::CStringTools::Int2String(iValuation)
      + ", Quotation is : " + tools::CStringTools::Int2String(iQuotation)
      + ", SpProfit is : " + tools::CStringTools::Int2String(iSpProfit)
      + ", SelfProfit is : " + tools::CStringTools::Int2String(iSelfProfit));

    //近似算法，仅和渠道有关系
    switch (iApproximationID)
    {
        case 1000000:
        {
            if (!GetApproximationMethod_1000000(iQuotation))
            {
                ERROR("GetApproximationMethod2 failed");
                return error_num::APPROXIMATION_FAILED;
            }
            break;
        }
        case 10000000:
        {
            //do nothing
            DEBUG("do nothing for ApproximationMethod 10000000");
            break;
        }
        default:
        {
            ERROR("ApproximationID is invalid");
            return error_num::APPROXIMATION_UNSUPPORT;
        }
    }
    iSelfProfit = iValuation - iQuotation - iSpProfit;
    DEBUG("Valuation is : " + tools::CStringTools::Int2String(iValuation)
      + ", Quotation is : " + tools::CStringTools::Int2String(iQuotation)
      + ", SpProfit is : " + tools::CStringTools::Int2String(iSpProfit)
      + ", SelfProfit is : " + tools::CStringTools::Int2String(iSelfProfit));

    //取整
    iSpProfit = logic::BankersRoundingMethod(iSpProfit);
    iSelfProfit = iValuation - iQuotation - iSpProfit;
    DEBUG("Valuation is : " + tools::CStringTools::Int2String(iValuation)
      + ", Quotation is : " + tools::CStringTools::Int2String(iQuotation)
      + ", SpProfit is : " + tools::CStringTools::Int2String(iSpProfit)
      + ", SelfProfit is : " + tools::CStringTools::Int2String(iSelfProfit));

    //底价，仅和产品有关系
    /////////////////////////////////////////////////////////////////////////////////////
    DEBUG("Valuation is : " + tools::CStringTools::Int2String(iValuation)
      + ", Quotation is : " + tools::CStringTools::Int2String(iQuotation)
      + ", SpProfit is : " + tools::CStringTools::Int2String(iSpProfit)
      + ", SelfProfit is : " + tools::CStringTools::Int2String(iSelfProfit));

    DEBUG("========Quotation END========");
    return error_num::SUCCEED;
}

