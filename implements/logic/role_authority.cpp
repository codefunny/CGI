#include "role_authority.h"
#include <stdlib.h>
#include <sstream>
#include "configure.h"
#include "db.h"

using namespace std;

bool logic::SelectRoleAuthorityTable(const vector<unsigned int>& roleIDList, vector<unsigned int>& authorityList)
{
    DEBUG("========SelectRoleAuthorityTable START========");

    if (roleIDList.empty())
    {
        ERROR("role list is empty");
        return false;
    }

    stringstream ssSql;
    ssSql << "SELECT * FROM t_role_authority"
          << " INNER JOIN t_role ON t_role_authority.Frole_id = t_role.Frole_id"
          << " INNER JOIN t_authority ON t_role_authority.Fauthority_id = t_authority.Fauthority_id";
    size_t maxSize = roleIDList.size();
    for (size_t index = 0; index < maxSize; index++)
    {
        if (0 == index)
        {
            ssSql << " WHERE (t_role.Frole_id = " << roleIDList[index];
        }
        else
        {
            ssSql << " OR t_role.Frole_id = " << roleIDList[index];
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
        authorityList.push_back(::atoi(oMysql.GetField(iIndex, "Fauthority_id").c_str()));
    }

    DEBUG("=========SelectRoleAuthorityTable END=========");
    return true;
}


