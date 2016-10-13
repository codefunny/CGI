#include "hot_product_table.h"
#include <stdlib.h>
#include <sstream>
#include "configure.h"
#include "db.h"

using namespace std;

bool logic::SelectHotProductTable(vector<unsigned int>& productIDList)
{
    DEBUG("=======SelectHotProductTable START=======");

    stringstream ssSql;
    ssSql << "SELECT * FROM t_hot_product";
    DEBUG("sql is : " + ssSql.str());
    db::Mysql oMysql;
    if (0 != oMysql.Query(ssSql.str()))
    {
        ERROR("failed to query for sql : " + ssSql.str());
        return false;
    }

    for (unsigned int iIndex = 0; iIndex < oMysql.GetRecordCount(); iIndex++)
    {
        productIDList.push_back(::atoi(oMysql.GetField(iIndex, "Fproduct_id").c_str()));
    }

    DEBUG("========SelectHotProductTable END========");
    return true;
}

