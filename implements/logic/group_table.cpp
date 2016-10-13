#include "group_table.h"
#include <stdlib.h>
#include <sstream>
#include "configure.h"
#include "db.h"

using namespace std;

bool logic::SelectGroupTable(unsigned int iUserID, vector<unsigned int>& roleIDList)
{
    DEBUG("========SelectGroupTable START========");

    stringstream ssSql;
    ssSql << "SELECT * FROM t_group"
          << " INNER JOIN t_user ON t_group.Fuser_id = t_user.Fuser_id"
          << " INNER JOIN t_role ON t_group.Frole_id = t_role.Frole_id"
          << " WHERE t_user.Fuser_id = " << iUserID;
    DEBUG("sql is : " + ssSql.str());
    db::Mysql oMysql;
    if (0 != oMysql.Query(ssSql.str()))
    {
        ERROR("failed to query for sql : " + ssSql.str());
        return false;
    }

    for (unsigned int iIndex = 0; iIndex < oMysql.GetRecordCount(); iIndex++)
    {
        roleIDList.push_back(::atoi(oMysql.GetField(iIndex, "Frole_id").c_str()));
    }

    DEBUG("=========SelectGroupTable END=========");
    return true;
}

bool logic::SelectGroupTable(unsigned int iUserID, vector< map<string, string> >& roleInfoList)
{
    DEBUG("========SelectGroupTable START========");

    stringstream ssSql;
    ssSql << "SELECT * FROM t_group"
          << " INNER JOIN t_user ON t_group.Fuser_id = t_user.Fuser_id"
          << " INNER JOIN t_role ON t_group.Frole_id = t_role.Frole_id"
          << " WHERE t_user.Fuser_id = " << iUserID;
    DEBUG("sql is : " + ssSql.str());
    db::Mysql oMysql;
    if (0 != oMysql.Query(ssSql.str()))
    {
        ERROR("failed to query for sql : " + ssSql.str());
        return false;
    }

    for (unsigned int iIndex = 0; iIndex < oMysql.GetRecordCount(); iIndex++)
    {
        map<string, string> roleInfo;
        roleInfo.insert(make_pair<string, string>("Frole_id", oMysql.GetField(iIndex, "Frole_id")));
        roleInfo.insert(make_pair<string, string>("Frole_name", oMysql.GetField(iIndex, "Frole_name")));
        roleInfoList.push_back(roleInfo);
    }

    DEBUG("=========SelectGroupTable END=========");
    return true;
}

bool logic::SelectGroupTableAuthorityList(unsigned int iUserID, vector< map<string, string> >& authorityList)
{
    DEBUG("========SelectGroupTable START========");

    stringstream ssSql;
    ssSql << "SELECT * FROM t_group"
          << " INNER JOIN t_user ON t_group.Fuser_id = t_user.Fuser_id"
          << " INNER JOIN t_role ON t_group.Frole_id = t_role.Frole_id"
          << " WHERE t_user.Fuser_id = " << iUserID;
    DEBUG("sql is : " + ssSql.str());
    db::Mysql oMysql;
    if (0 != oMysql.Query(ssSql.str()))
    {
        ERROR("failed to query for sql : " + ssSql.str());
        return false;
    }
    if (0 == oMysql.GetRecordCount())
    {
        ERROR("role list is empty");
        return false;
    }

    ssSql.str("");
    ssSql << "SELECT * FROM t_role_authority"
          << " INNER JOIN t_role ON t_role_authority.Frole_id = t_role.Frole_id"
          << " INNER JOIN t_authority ON t_role_authority.Fauthority_id = t_authority.Fauthority_id";
    for (unsigned int iIndex = 0; iIndex < oMysql.GetRecordCount(); iIndex++)
    {
        if (0 == iIndex)
        {
            ssSql << " WHERE (t_role.Frole_id = " << oMysql.GetField(iIndex, "Frole_id");
        }
        else
        {
            ssSql << " OR t_role.Frole_id = " << oMysql.GetField(iIndex, "Frole_id");
        }
    }
    ssSql << ")";
    DEBUG("sql is : " + ssSql.str());
    if (0 != oMysql.Query(ssSql.str()))
    {
        ERROR("failed to query for sql : " + ssSql.str());
        return false;
    }

    for (unsigned int iIndex = 0; iIndex < oMysql.GetRecordCount(); iIndex++)
    {
        map<string, string> authorityInfo;
        authorityInfo.insert(make_pair<string, string>("Fauthority_id", oMysql.GetField(iIndex, "Fauthority_id")));
        authorityInfo.insert(make_pair<string, string>("Fauthority_name", oMysql.GetField(iIndex, "Fauthority_name")));
        authorityList.push_back(authorityInfo);
    }

    DEBUG("=========SelectGroupTable END=========");
    return true;
}

//modify by 张世柏 at 2014-10-20
//向用户组表用插入一行新的数据
//参数1：strUserID:传入参数：用户ID
//参数2：strGroupID：传入参数：用户组ID
//返回值true：表示sql执行成功
//返回值false：表示执行数据库操作失败
bool logic::InsertGroupTable(const string& strUserID, const string& strGroupID)
{
    DEBUG("========InsertGroupTable START========");
    db::Mysql oMysql;
    stringstream ssSql;
    ssSql << "INSERT INTO t_group "
          << "SET Fuser_id = " << oMysql.EscapeString(strUserID)
          << ", Frole_id = "   << oMysql.EscapeString(strGroupID);
    DEBUG("sql is : " + ssSql.str());
    if (0 != oMysql.Execute(ssSql.str()))
    {
        ERROR("failed to execute for sql : " + ssSql.str());
        return false;
    }
    if (1 != oMysql.GetAffectedRows())
    {
        ERROR("DB has error for sql : " + ssSql.str());
        return false;
    }
    DEBUG("=========InsertGroupTable END=========");
    return true;
}
//end modify

