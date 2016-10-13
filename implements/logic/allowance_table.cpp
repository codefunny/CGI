#include "allowance_table.h"
#include <stdlib.h>
#include <sstream>
#include "configure.h"
#include "db.h"

using namespace std;

bool logic::SelectAllowanceTable(unsigned int iProductID, unsigned int iChannelID, int& iSpAllowance, int& iSelfAllowance)
{
    DEBUG("========SelectAllowanceTable START========");

    stringstream ssSql;
    ssSql << "SELECT * FROM t_allowance"
          << " INNER JOIN t_product ON t_allowance.Fproduct_id = t_product.Fproduct_id"
          << " INNER JOIN t_channel ON t_allowance.Fchannel_id = t_channel.Fchannel_id"
          << " WHERE t_product.Fproduct_id = " << iProductID
          << " AND t_channel.Fchannel_id = " << iChannelID;
    DEBUG("sql is : " + ssSql.str());
    db::Mysql oMysql;
    if (0 != oMysql.Query(ssSql.str()))
    {
        ERROR("failed to query for sql : " + ssSql.str());
        return false;
    }
    if (1 != oMysql.GetRecordCount())
    {
        DEBUG("NO Allowance");
        iSpAllowance = 0;
        iSelfAllowance = 0;
        return true;
    }

    iSpAllowance = ::atoi(oMysql.GetField(0, "Fsp_allowance").c_str());
    iSelfAllowance = ::atoi(oMysql.GetField(0, "Fself_allowance").c_str());

    DEBUG("=========SelectAllowanceTable END=========");
    return true;
}

