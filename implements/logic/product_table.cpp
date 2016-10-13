#include "product_table.h"
#include <stdlib.h>
#include <sstream>
#include "configure.h"
#include "db.h"
#include "string_tools.h"
#include "category_table.h"
#include "hot_product_table.h"

using namespace std;

bool logic::SelectProductTable(unsigned int iProductID, map<string, string>& productInfo)
{
    DEBUG("=======SelectProductTable START=======");

    stringstream ssSql;
    ssSql << "SELECT * FROM t_product"
          << " INNER JOIN t_category ON t_product.Fcategory_id = t_category.Fcategory_id"
          << " INNER JOIN t_product_profit_plan ON t_product.Fprofit_plan_id = t_product_profit_plan.Fprofit_plan_id"
          << " WHERE t_product.Fproduct_id = " << iProductID
          << " AND t_product.Fvalid = " << PRODUCT_VALID;
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

    productInfo.insert(make_pair<string, string>("Fproduct_id", oMysql.GetField(0, "Fproduct_id")));
    productInfo.insert(make_pair<string, string>("Fproduct_name", oMysql.GetField(0, "Fproduct_name")));
    productInfo.insert(make_pair<string, string>("Fproduct_desc", oMysql.GetField(0, "Fproduct_desc")));
    productInfo.insert(make_pair<string, string>("Fmax_price", oMysql.GetField(0, "Fmax_price")));
    productInfo.insert(make_pair<string, string>("Fshow_ways", oMysql.GetField(0, "Fshow_ways")));
    productInfo.insert(make_pair<string, string>("Fcalc_ways", oMysql.GetField(0, "Fcalc_ways")));
    productInfo.insert(make_pair<string, string>("Fprofit_plan_method", oMysql.GetField(0, "Fprofit_plan_method")));
    productInfo.insert(make_pair<string, string>("Fprice_list", oMysql.GetField(0, "Fprice_list")));
    productInfo.insert(make_pair<string, string>("Fpic_id", oMysql.GetField(0, "Fpic_id")));

    productInfo.insert(make_pair<string, string>("Fcategory_id", oMysql.GetField(0, "Fcategory_id")));
    productInfo.insert(make_pair<string, string>("Fcategory_name", oMysql.GetField(0, "Fcategory_name")));
    productInfo.insert(make_pair<string, string>("Fsub_desc", oMysql.GetField(0, "Fsub_desc")));
    productInfo.insert(make_pair<string, string>("Fsub_category", oMysql.GetField(0, "Fsub_category")));

    DEBUG("========SelectProductTable END========");
    return true;
}

static bool GetCondition(const logic::CGetProductFilter& oFilter, string& strConditionWithLimit, string& strConditionForTotal)
{
    stringstream ssCondition;
    switch (oFilter.iType)
    {
        case logic::BY_HOT_TYPE:
        {
            vector<unsigned int> vecProductIDList;
            if (!logic::SelectHotProductTable(vecProductIDList))
            {
                ERROR("SelectHotProductTable failed");
                return false;
            }

            size_t maxSize = vecProductIDList.size();
            for (size_t index = 0; index < maxSize; index++)
            {
                if (0 == index)
                {
                    ssCondition << " AND (Fproduct_id = " << vecProductIDList[index];
                }
                else
                {
                    ssCondition << " OR Fproduct_id = " << vecProductIDList[index];
                }
            }
            if (!vecProductIDList.empty())
            {
                ssCondition << ")";
            }
            break;
        }
        case logic::BY_CATEGORY_ID:
        {
            vector<unsigned int> vecCategoryIDList = logic::SelectSubCategoryID(oFilter.iMid);
            size_t maxSize = vecCategoryIDList.size();
            for (size_t index = 0; index < maxSize; index++)
            {
                if (0 == index)
                {
                    ssCondition << " AND (Fcategory_id = " << vecCategoryIDList[index];
                }
                else
                {
                    ssCondition << " OR Fcategory_id = " << vecCategoryIDList[index];
                }
            }
            if (!vecCategoryIDList.empty())
            {
                ssCondition << ")";
            }
            break;
        }
        case logic::BY_PRODUCT_DESC:
        {
            db::Mysql oMysql;
            vector<string> vecSearchCondition = tools::CStringTools::Split2String(oFilter.strSearch, " ");
            size_t maxSize = vecSearchCondition.size();
            for (size_t index = 0; index < maxSize; index++)
            {
                ssCondition << " AND Fproduct_desc LIKE '%" << oMysql.EscapeString(vecSearchCondition[index]) << "%'";
            }
            break;
        }
        default:
        {
            ERROR("invalid request");
            return false;
        }
    }

    strConditionForTotal = ssCondition.str();
    ssCondition << " LIMIT " << oFilter.iPagesize * oFilter.iPageindex << ", " << oFilter.iPagesize;
    strConditionWithLimit = ssCondition.str();

    return true;
}

bool logic::SelectProductTable(const logic::CGetProductFilter& oFilter, vector< map<string, string> >& vecProductList, unsigned int& iTotal)
{
    DEBUG("=======SelectProductTable START=======");

    string strConditionWithLimit, strConditionForTotal;
    if (!GetCondition(oFilter, strConditionWithLimit, strConditionForTotal))
    {
        ERROR("GetCondition failed");
        return false;
    }

    stringstream ssSql;
    ssSql << "SELECT * FROM t_product WHERE Fvalid = " << PRODUCT_VALID;
    ssSql << strConditionWithLimit;
    DEBUG("sql is : " + ssSql.str());
    db::Mysql oMysql;
    if (0 != oMysql.Query(ssSql.str()))
    {
        ERROR("failed to query for sql : " + ssSql.str());
        return false;
    }

    for (unsigned int iIndex = 0; iIndex < oMysql.GetRecordCount(); iIndex++)
    {
        map<string, string> productItem;
        productItem.insert(make_pair<string, string>("Fproduct_id", oMysql.GetField(iIndex, "Fproduct_id")));
        productItem.insert(make_pair<string, string>("Fproduct_name", oMysql.GetField(iIndex, "Fproduct_name")));
        productItem.insert(make_pair<string, string>("Fproduct_desc", oMysql.GetField(iIndex, "Fproduct_desc")));
        productItem.insert(make_pair<string, string>("Fmax_price", oMysql.GetField(iIndex, "Fmax_price")));
        productItem.insert(make_pair<string, string>("Fshow_ways", oMysql.GetField(iIndex, "Fshow_ways")));
        productItem.insert(make_pair<string, string>("Fprice_list", oMysql.GetField(iIndex, "Fprice_list")));
        productItem.insert(make_pair<string, string>("Fpic_id", oMysql.GetField(iIndex, "Fpic_id")));

        vecProductList.push_back(productItem);
    }

    ssSql.str("");
    ssSql << "SELECT COUNT(*) FROM t_product WHERE Fvalid = " << PRODUCT_VALID;
    ssSql << strConditionForTotal;
    DEBUG("sql is : " + ssSql.str());
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
    iTotal = ::atoi(oMysql.GetField(0, "COUNT(*)").c_str());

    DEBUG("========SelectProductTable END========");
    return true;
}

bool logic::UpdateProductTable(const CProductTableData& productTableData)
{
    /*
    DEBUG("=======UpdateProductTable START=======");

    stringstream ssSql;
    ssSql << "INSERT INTO t_product_history SELECT * FROM t_product";

    ssSql << "SELECT * FROM t_product"
          << " INNER JOIN t_category ON t_product.Fcategory_id = t_category.Fcategory_id"
          << " WHERE t_product.Fproduct_id = " << iProductID
          << " AND t_product.Fvalid = " << PRODUCT_VALID;
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

    productInfo.insert(make_pair<string, string>("Fproduct_id", oMysql.GetField(0, "Fproduct_id")));
    productInfo.insert(make_pair<string, string>("Fproduct_name", oMysql.GetField(0, "Fproduct_name")));
    productInfo.insert(make_pair<string, string>("Fproduct_desc", oMysql.GetField(0, "Fproduct_desc")));
    productInfo.insert(make_pair<string, string>("Fmax_price", oMysql.GetField(0, "Fmax_price")));
    productInfo.insert(make_pair<string, string>("Fshow_ways", oMysql.GetField(0, "Fshow_ways")));
    productInfo.insert(make_pair<string, string>("Fprice_list", oMysql.GetField(0, "Fprice_list")));
    productInfo.insert(make_pair<string, string>("Fpic_id", oMysql.GetField(0, "Fpic_id")));

    productInfo.insert(make_pair<string, string>("Fcategory_id", oMysql.GetField(0, "Fcategory_id")));
    productInfo.insert(make_pair<string, string>("Fcategory_name", oMysql.GetField(0, "Fcategory_name")));
    productInfo.insert(make_pair<string, string>("Fsub_desc", oMysql.GetField(0, "Fsub_desc")));
    productInfo.insert(make_pair<string, string>("Fsub_category", oMysql.GetField(0, "Fsub_category")));
*/
    DEBUG("========UpdateProductTable END========");
    return true;
}


