#include "authority.h"
#include <vector>
#include <algorithm>
#include "configure.h"
#include "group_table.h"
#include "role_authority.h"
#include "string_tools.h"

using namespace std;

bool logic::HasAuthority(unsigned int iUserID, unsigned int iAuthorityID)
{
    vector<unsigned int> roleIDList;
    if (!logic::SelectGroupTable(iUserID, roleIDList))
    {
        ERROR("SelectGroupTable failed");
        return false;
    }

    vector<unsigned int> authorityList;
    if (!logic::SelectRoleAuthorityTable(roleIDList, authorityList))
    {
        ERROR("SelectRoleAuthorityTable failed");
        return false;
    }

    if (authorityList.end() != find(authorityList.begin(), authorityList.end(), iAuthorityID))
    {
        DEBUG("DO have authority for " + tools::CStringTools::Int2String(iAuthorityID));
        return true;
    }
    else
    {
        ERROR("do NOT have authority for " + tools::CStringTools::Int2String(iAuthorityID));
        return false;
    }

    ERROR("do NOT have authority for " + tools::CStringTools::Int2String(iAuthorityID));
    return false;
}

