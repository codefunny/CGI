#include "channel_table.h"
#include <stdlib.h>
#include <sstream>
#include "configure.h"
#include "db.h"

using namespace std;

bool logic::SelectChannelTable(unsigned int iChannelID, map<string, string>& channelInfo)
{
    DEBUG("=======SelectChannelTable START=======");

    stringstream ssSql;
    ssSql << "SELECT * FROM t_channel"
          << " INNER JOIN t_valuation ON t_channel.Fvaluation_id = t_valuation.Fvaluation_id"
          << " INNER JOIN t_quotation ON t_channel.Fquotation_id = t_quotation.Fquotation_id"
          << " WHERE t_channel.Fchannel_id = " << iChannelID;
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

    channelInfo.insert(make_pair<string, string>("Fchannel_id", oMysql.GetField(0, "Fchannel_id")));
    channelInfo.insert(make_pair<string, string>("Fchannel_name", oMysql.GetField(0, "Fchannel_name")));
    channelInfo.insert(make_pair<string, string>("Fchannel_desc", oMysql.GetField(0, "Fchannel_desc")));
    channelInfo.insert(make_pair<string, string>("Fchannel_logo", oMysql.GetField(0, "Fchannel_logo")));
    channelInfo.insert(make_pair<string, string>("Fvaluation_id", oMysql.GetField(0, "Fvaluation_id")));
    channelInfo.insert(make_pair<string, string>("Fquotation_id", oMysql.GetField(0, "Fquotation_id")));
    channelInfo.insert(make_pair<string, string>("Fapproximation_id", oMysql.GetField(0, "Fapproximation_id")));
    channelInfo.insert(make_pair<string, string>("Fis_need_login", oMysql.GetField(0, "Fis_need_login")));

    channelInfo.insert(make_pair<string, string>("Fvaluation_desc", oMysql.GetField(0, "Fvaluation_desc")));
    channelInfo.insert(make_pair<string, string>("Fshow_ways", oMysql.GetField(0, "Fshow_ways")));
    channelInfo.insert(make_pair<string, string>("Fcalc_ways", oMysql.GetField(0, "Fcalc_ways")));

    channelInfo.insert(make_pair<string, string>("Fquotation_desc", oMysql.GetField(0, "Fquotation_desc")));
    channelInfo.insert(make_pair<string, string>("Fquotation_ways", oMysql.GetField(0, "Fquotation_ways")));

    DEBUG("========SelectChannelTable END========");
    return true;
}

bool logic::ChannelTable_UserBegins2ChannelID(const string& strUserBegins, unsigned int& iChannelID)
{
    DEBUG("=======ChannelTable_UserBegins2ChannelID START=======");

    db::Mysql oMysql;
    stringstream ssSql;
    ssSql << "SELECT * FROM t_channel"
          << " WHERE Fuser_begin = '" << oMysql.EscapeString(strUserBegins) << "'";
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
    iChannelID = ::atoi(oMysql.GetField(0, "Fchannel_id").c_str());

    DEBUG("========ChannelTable_UserBegins2ChannelID END========");
    return true;
}


