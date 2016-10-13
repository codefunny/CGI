#include "goods_table.h"
#include <stdlib.h>
#include <sstream>
#include "configure.h"
#include "db.h"

using namespace std;

bool logic::InsertGoodsTableForPrice(unsigned int iProductID, unsigned int iChannelID,
                                     unsigned int iSeller, const string& strPriceInputResult, unsigned int iValuation, unsigned int iQuotation, unsigned int iSpProfit, unsigned int iSelfProfit,
                                     unsigned int& iGoodsID)
{
    DEBUG("=======InsertGoodsTableForPrice START=======");

    db::Mysql oMysql;
    stringstream ssSql;
    ssSql << "INSERT INTO t_goods"
          << " SET Fproduct_id = " << iProductID
          << ", Fchannel_id = " << iChannelID;
    if (0 != iSeller)//免登录时，没有用户信息，填数据库默认值
    {
        ssSql << ", Fseller = " << iSeller;
    }
    ssSql << ", Fprice_input_result = '" << oMysql.EscapeString(strPriceInputResult) << "'"
          << ", Fvaluation = " << iValuation
          << ", Fquotation = " << iQuotation
          << ", Fsp_profit = " << iSpProfit
          << ", Fself_profit = " << iSelfProfit;
    DEBUG("sql is : " + ssSql.str());
    if (0 != oMysql.Execute(ssSql.str()))
    {
        ERROR("failed to execute for sql : " + ssSql.str());
        return false;
    }
    if (1 != oMysql.GetAffectedRows())
    {
        ERROR("failed to execute for sql : " + ssSql.str());
        return false;
    }

    stringstream ssLastInsertID;
    ssLastInsertID << "SELECT LAST_INSERT_ID()";
    DEBUG("sql is : " + ssLastInsertID.str());
    if (0 != oMysql.Query(ssLastInsertID.str()))
    {
        ERROR("failed to query for sql : " + ssLastInsertID.str());
        return false;
    }
    if (1 != oMysql.GetRecordCount())
    {
        ERROR("DB has error for sql : " + ssLastInsertID.str());
        return false;
    }
    DEBUG("Fgoods_id is : " + oMysql.GetField(0, 0));
    iGoodsID = ::atoi(oMysql.GetField(0, 0).c_str());

    DEBUG("========InsertGoodsTableForPrice END========");
    return true;
}

bool logic::SelectGoodsTable(unsigned int iGoodsID, map<string, string>& goodsInfo)
{
    DEBUG("===SelectGoodsTable START===");

    stringstream ssSql;
    ssSql << "SELECT * FROM t_goods"
          << " INNER JOIN t_product ON t_goods.Fproduct_id = t_product.Fproduct_id"
          << " INNER JOIN t_channel on t_goods.Fchannel_id = t_channel.Fchannel_id"
          << " WHERE Fgoods_id = " << iGoodsID;
    DEBUG("sql is : " + ssSql.str());
    db::Mysql oMysql;
    if (0 != oMysql.Query(ssSql.str()))
    {
        ERROR("failed to query for sql : " + ssSql.str());
        return false;
    }
    if (1 != oMysql.GetRecordCount())
    {
        ERROR("DB records have error for sql : " + ssSql.str());
        return false;
    }

    goodsInfo.insert(make_pair<string, string>("Fgoods_id", oMysql.GetField(0, "Fgoods_id")));
    goodsInfo.insert(make_pair<string, string>("Fproduct_id", oMysql.GetField(0, "Fproduct_id")));
    goodsInfo.insert(make_pair<string, string>("Fchannel_id", oMysql.GetField(0, "Fchannel_id")));
    goodsInfo.insert(make_pair<string, string>("Fseller", oMysql.GetField(0, "Fseller")));
    goodsInfo.insert(make_pair<string, string>("Fprice_input_result", oMysql.GetField(0, "Fprice_input_result")));
    goodsInfo.insert(make_pair<string, string>("Fvaluation", oMysql.GetField(0, "Fvaluation")));
    goodsInfo.insert(make_pair<string, string>("Fquotation", oMysql.GetField(0, "Fquotation")));
    goodsInfo.insert(make_pair<string, string>("Fsp_profit", oMysql.GetField(0, "Fsp_profit")));
    goodsInfo.insert(make_pair<string, string>("Fself_profit", oMysql.GetField(0, "Fself_profit")));
    goodsInfo.insert(make_pair<string, string>("Fupdate_time", oMysql.GetField(0, "Fupdate_time")));
    goodsInfo.insert(make_pair<string, string>("Fupdate_count", oMysql.GetField(0, "Fupdate_count")));

    //产品表
    goodsInfo.insert(make_pair<string, string>("Fproduct_name", oMysql.GetField(0, "Fproduct_name")));

    //渠道表
    goodsInfo.insert(make_pair<string, string>("Fchannel_name", oMysql.GetField(0, "Fchannel_name")));

    //modified by 张世柏 at 2014-10-20
    goodsInfo.insert(make_pair<string, string>("Fpic_id", oMysql.GetField(0, "Fpic_id")));
    //end modify

    DEBUG("====SelectGoodsTable END====");
    return true;
}

bool logic::UpdateGoodsTableForFseller(unsigned int iGoodID, unsigned int iSeller)
{
    DEBUG("=======UpdateGoodsTableForFseller START=======");

    db::Mysql oMysql;
    stringstream ssSql;
    ssSql << "UPDATE t_goods"
          << " SET Fseller = " << iSeller
          << " WHERE Fgoods_id = " << iGoodID;
    DEBUG("sql is : " + ssSql.str());
    if (0 != oMysql.Execute(ssSql.str()))
    {
        ERROR("failed to execute for sql : " + ssSql.str());
        return false;
    }
    if (1 != oMysql.GetAffectedRows())
    {
        ERROR("failed to execute for sql : " + ssSql.str());
        return false;
    }

    DEBUG("========UpdateGoodsTableForFseller END========");
    return true;
}


