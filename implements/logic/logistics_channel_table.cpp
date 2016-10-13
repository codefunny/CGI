#include "logistics_channel_table.h"
#include <stdlib.h>
#include <sstream>
#include "configure.h"
#include "db.h"

#include "string_tools.h"

using namespace std;

bool logic::SelectLogisticsChannelTable(vector< map<string, string> >& logisticsChannelList)
{
    DEBUG("=======SelectLogisticsChannelTable START=======");

    db::Mysql oMysql;
    stringstream ssSql;
    ssSql << "SELECT * FROM t_logistics_channel";
    DEBUG("sql is : " + ssSql.str());
    if (0 != oMysql.Query(ssSql.str()))
    {
        ERROR("failed to execute for sql : " + ssSql.str());
        return false;
    }

    for (unsigned int iIndex = 0; iIndex < oMysql.GetRecordCount(); iIndex++)
    {
        map<string, string> logisticsChannelItem;
        logisticsChannelItem.insert(make_pair<string, string>("Flogistics_type", oMysql.GetField(iIndex, "Flogistics_type")));
        logisticsChannelItem.insert(make_pair<string, string>("Flogistics_name", oMysql.GetField(iIndex, "Flogistics_name")));
        logisticsChannelList.push_back(logisticsChannelItem);
    }

    DEBUG("========SelectLogisticsChannelTable END========");
    return true;
}


