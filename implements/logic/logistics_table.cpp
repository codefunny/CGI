#include "logistics_table.h"
#include <stdlib.h>
#include <sstream>
#include "configure.h"
#include "db.h"

using namespace std;

bool logic::InsertLogisticsTable(unsigned int iChannelType, const string& strChannelID, unsigned int& iLogisticsID)
{
    DEBUG("=======InsertLogisticsTable START=======");

    db::Mysql oMysql;
    stringstream ssSql;
    ssSql << "INSERT INTO t_logistics SET"
          << " Fchannel_type = " << iChannelType
          << ", Fchannel_id = '" << oMysql.EscapeString(strChannelID) << "'"
          << ", Flogistics_status = " << LOGISTICS_STATUS_START;
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
        ERROR("failed to query for sql : " +  ssLastInsertID.str());
        return false;
    }
    if (1 != oMysql.GetRecordCount())
    {
        ERROR("DB has error for sql : " + ssLastInsertID.str());
        return false;
    }
    DEBUG("iLogisticsID is : " + oMysql.GetField(0, 0));
    iLogisticsID = ::atoi(oMysql.GetField(0, 0).c_str());

    DEBUG("========InsertLogisticsTable END========");

    return true;
}


