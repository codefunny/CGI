#include "price_character_table.h"
#include <stdlib.h>
#include <sstream>
#include <algorithm>
#include "configure.h"
#include "db.h"
#include "string_tools.h"

using namespace std;

bool logic::SelectPriceCharacterTable(unsigned int iPriceID, map<string, string>& priceCharacterInfo)
{
    DEBUG("===SelectPriceCharacterTable START===");

    stringstream ssSql;
    ssSql << "SELECT * FROM t_price_character"
          << " WHERE Fprice_id = " << iPriceID;
    DEBUG("sql is : " + ssSql.str());
    db::Mysql oMysql;
    if (0 != oMysql.Query(ssSql.str()))
    {
        ERROR("failed to query for sql : " + ssSql.str());
        return false;
    }
    if (1 != oMysql.GetRecordCount())
    {
        ERROR("DB has error for sql : " + ssSql.str());
        return false;
    }

    priceCharacterInfo.insert(make_pair<string, string>("Fprice_id", oMysql.GetField(0, "Fprice_id")));
    priceCharacterInfo.insert(make_pair<string, string>("Fprice_father", oMysql.GetField(0, "Fprice_father")));
    priceCharacterInfo.insert(make_pair<string, string>("Fprice_name", oMysql.GetField(0, "Fprice_name")));
    priceCharacterInfo.insert(make_pair<string, string>("Fprice_desc", oMysql.GetField(0, "Fprice_desc")));
    priceCharacterInfo.insert(make_pair<string, string>("Fprice_type", oMysql.GetField(0, "Fprice_type")));

    DEBUG("====SelectPriceCharacterTable END====");
    return true;
}

bool logic::GetPriceCharacterName(const vector<unsigned int>& vecPriceID, string& strCharacterNameList)
{
    DEBUG("===GetPriceCharacterName START===");
    if (vecPriceID.empty())
    {
        ERROR("price id is empty");
        return false;
    }

    stringstream ssSql;
    ssSql << "SELECT * FROM t_price_character";
    for (size_t index = 0; index < vecPriceID.size(); index++)
    {
        if (0 == index)
        {
            ssSql << " WHERE (Fprice_id = " << vecPriceID[index];
        }
        else
        {
            ssSql << " OR Fprice_id = " << vecPriceID[index];
        }
    }
    ssSql << ")";
    DEBUG("sql is : " + ssSql.str());
    db::Mysql oMysql;
    if (0 != oMysql.Query(ssSql.str()))
    {
        ERROR("failed to query for sql : " + ssSql.str());
        return false;
    }

    for (unsigned int iIndex = 0; iIndex < oMysql.GetRecordCount(); iIndex++)
    {
        strCharacterNameList += oMysql.GetField(iIndex, "Fprice_name");
        strCharacterNameList += " | ";
    }

    DEBUG("====GetPriceCharacterName END====");
    return true;
}

bool logic::SelectPriceCharacterTableByFatherID(unsigned int iPriceFatherID, vector< map<string, string> >& priceCharacterList)
{
    DEBUG("===SelectPriceCharacterTableByFatherID START===");

    stringstream ssSql;
    ssSql << "SELECT * FROM t_price_character"
          << " WHERE Fprice_father = " << iPriceFatherID;
    DEBUG("sql is : " + ssSql.str());
    db::Mysql oMysql;
    if (0 != oMysql.Query(ssSql.str()))
    {
        ERROR("failed to query for sql : " +  ssSql.str());
        return false;
    }

    for (unsigned int iIndex = 0; iIndex < oMysql.GetRecordCount(); iIndex++)
    {
        map<string, string> priceCharacterItem;
        priceCharacterItem.insert(make_pair<string, string>("Fprice_id", oMysql.GetField(iIndex, "Fprice_id")));
        priceCharacterItem.insert(make_pair<string, string>("Fprice_father", oMysql.GetField(iIndex, "Fprice_father")));
        priceCharacterItem.insert(make_pair<string, string>("Fprice_name", oMysql.GetField(iIndex, "Fprice_name")));
        priceCharacterItem.insert(make_pair<string, string>("Fprice_desc", oMysql.GetField(iIndex, "Fprice_desc")));
        priceCharacterItem.insert(make_pair<string, string>("Fprice_type", oMysql.GetField(iIndex, "Fprice_type")));
        priceCharacterList.push_back(priceCharacterItem);
    }

    DEBUG("====SelectPriceCharacterTableByFatherID END====");
    return true;
}

bool logic::GetPriceCharacterType(const vector<unsigned int>& vecPriceID, map<unsigned int, unsigned int>& priceCharacterType)
{
    DEBUG("===GetPriceCharacterType START===");

    if (vecPriceID.empty())
    {
        ERROR("empty input");
        return false;
    }

    stringstream ssSql;
    ssSql << "SELECT * FROM t_price_character"
          << " WHERE Fprice_id =";
    for (size_t iIndex = 0; iIndex < vecPriceID.size(); iIndex++)
    {
        if (0 == iIndex)
        {
            ssSql << " " << vecPriceID[iIndex];
        }
        else
        {
            ssSql << " OR Fprice_id = " << vecPriceID[iIndex];
        }
    }
    DEBUG("sql is : " + ssSql.str());
    db::Mysql oMysql;
    if (0 != oMysql.Query(ssSql.str()))
    {
        ERROR("failed to query for sql : " +  ssSql.str());
        return false;
    }

    for (unsigned int iIndex = 0; iIndex < oMysql.GetRecordCount(); iIndex++)
    {
        priceCharacterType.insert(make_pair<unsigned int, unsigned int>(::atoi(oMysql.GetField(iIndex, "Fprice_id").c_str()), ::atoi(oMysql.GetField(iIndex, "Fprice_type").c_str())));
    }

    DEBUG("====GetPriceCharacterType END====");
    return true;
}

bool logic::CheckFatherIDIsRight(const vector<unsigned int>& vecProduct, const vector<unsigned int>& vecChannel, const vector<unsigned int>& vecCheckResult, vector<unsigned int>& vecNewCheckResult)
{
    DEBUG("vecCheckResult :");
    tools::CStringTools::Show(vecCheckResult);

    stringstream ssSql;
    ssSql << "SELECT * FROM t_price_character"
          << " WHERE Fprice_id =";
    for (size_t iIndex = 0; iIndex < vecCheckResult.size(); iIndex++)
    {
        if (0 == iIndex)
        {
            ssSql << " " << vecCheckResult[iIndex];
        }
        else
        {
            ssSql << " OR Fprice_id = " << vecCheckResult[iIndex];
        }
    }
    DEBUG("sql is : " + ssSql.str());
    db::Mysql oMysql;
    if (0 != oMysql.Query(ssSql.str()))
    {
        ERROR("failed to query for sql : " +  ssSql.str());
        return false;
    }

    vector<unsigned int> vecFatherCheckResult;
    for (unsigned int iIndex = 0; iIndex < oMysql.GetRecordCount(); iIndex++)
    {
        vecFatherCheckResult.push_back(::atoi(oMysql.GetField(iIndex, "Fprice_father").c_str()));
    }

    vector<unsigned int> vecNeedToCheck = vecProduct;
    vecNeedToCheck.insert(vecNeedToCheck.end(), vecChannel.begin(), vecChannel.end());

    sort(vecNeedToCheck.begin(), vecNeedToCheck.end());
    sort(vecFatherCheckResult.begin(), vecFatherCheckResult.end());

    if (vecNeedToCheck == vecFatherCheckResult)
    {
        //验证检查项是否齐全，与，估价计算方式整理，似乎可以用同一sql语句解决。但为了解耦还是预留了两套算法
        ssSql.str("");
        ssSql << "SELECT * FROM t_price_character"
              << " WHERE Fprice_father =";
        for (size_t iIndex = 0; iIndex < vecChannel.size(); iIndex++)
        {
            if (0 == iIndex)
            {
                ssSql << " " << vecChannel[iIndex];
            }
            else
            {
                ssSql << " OR Fprice_father = " << vecChannel[iIndex];
            }
        }
        for (size_t iIndex = 0; iIndex < vecProduct.size(); iIndex++)
        {
            ssSql << " OR Fprice_father = " << vecProduct[iIndex];
        }
        DEBUG("sql is : " + ssSql.str());
        if (0 != oMysql.Query(ssSql.str()))
        {
            ERROR("failed to query for sql : " +  ssSql.str());
            return false;
        }
        map<unsigned int, unsigned int> mapAllSubItemNeedToCheck;//所有待检查可能项
        for (unsigned int iIndex = 0; iIndex < oMysql.GetRecordCount(); iIndex++)
        {
            mapAllSubItemNeedToCheck.insert(make_pair<unsigned int, unsigned int>(::atoi(oMysql.GetField(iIndex, "Fprice_id").c_str()), ::atoi(oMysql.GetField(iIndex, "Fprice_father").c_str())));
        }
        map<unsigned int, unsigned int> mapCheckResult;
        for (size_t iIndex = 0; iIndex < vecCheckResult.size(); iIndex++)
        {
            if (mapAllSubItemNeedToCheck.end() == mapAllSubItemNeedToCheck.find(vecCheckResult[iIndex]))
            {
                //必须得找到，不然前面验证检查项是否齐全的算法就有错误
                WARN("logic error for item " + tools::CStringTools::Int2String(vecCheckResult[iIndex]));
                return false;
            }
            //mapAllSubItemNeedToCheck为 Fprice_id : Fprice_father
            //mapCheckResult为           Fprice_father : Fprice_id
            mapCheckResult.insert(make_pair<unsigned int, unsigned int>(mapAllSubItemNeedToCheck[vecCheckResult[iIndex]], vecCheckResult[iIndex]));//实际检查结果
        }

        //开始排序
        for (size_t iIndex = 0; iIndex < vecChannel.size(); iIndex++)
        {
            if (mapCheckResult.end() == mapCheckResult.find(vecChannel[iIndex]))
            {
                //必须得找到，不然前面验证检查项是否齐全的算法就有错误
                WARN("logic error for item " + tools::CStringTools::Int2String(vecChannel[iIndex]));
                return false;
            }
            vecNewCheckResult.push_back(mapCheckResult[vecChannel[iIndex]]);
        }
        for (size_t iIndex = 0; iIndex < vecProduct.size(); iIndex++)
        {
            if (mapCheckResult.end() == mapCheckResult.find(vecProduct[iIndex]))
            {
                //必须得找到，不然前面验证检查项是否齐全的算法就有错误
                WARN("logic error for item " + tools::CStringTools::Int2String(vecProduct[iIndex]));
                return false;
            }
            vecNewCheckResult.push_back(mapCheckResult[vecProduct[iIndex]]);
        }

        DEBUG("==================================================");
        tools::CStringTools::Show(vecCheckResult);
        DEBUG("==================================================");
        tools::CStringTools::Show(vecNewCheckResult);
        DEBUG("==================================================");

        return true;
    }
    else
    {
        ERROR("vecNeedToCheck :");
        tools::CStringTools::Show(vecNeedToCheck);
        ERROR("vecFatherCheckResult :");
        tools::CStringTools::Show(vecFatherCheckResult);
        return false;
    }
}

