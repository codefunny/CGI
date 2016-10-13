#include "organization_table.h"
#include <sstream>
#include "configure.h"
#include "db.h"
#include "string_tools.h"

using namespace std;

static vector<unsigned int> GetSubOrganization(unsigned int iOrganizationID)
{
    vector<unsigned int> vecSubOrganization;

    stringstream ssSql;
    ssSql << "SELECT Fsub_organization FROM t_organization"
          << " WHERE Forganization_id = " << iOrganizationID;
    //DEBUG("sql is : " + ssSql.str());
    db::Mysql oMysql;
    if (0 != oMysql.Query(ssSql.str()))
    {
        ERROR("failed to query for sql : " + ssSql.str());
        return vecSubOrganization;
    }
    if (1 != oMysql.GetRecordCount())
    {
        ERROR("DB records have error for sql : " + ssSql.str());
        return vecSubOrganization;
    }

    string strOrgList = oMysql.GetField(0, "Fsub_organization");
    if (strOrgList.empty())
    {
        return vecSubOrganization;
    }
    DEBUG(tools::CStringTools::Int2String(iOrganizationID) + "'s sons are " + strOrgList);
    vecSubOrganization = tools::CStringTools::Split2Uint(strOrgList, "#");

    return vecSubOrganization;
}

static void GetSubOrganization(const vector<unsigned int>& vecOrganization, vector<unsigned int>& vecFinalResult)
{
    for (size_t iIndex = 0; iIndex < vecOrganization.size(); iIndex++)
    {
        //DEBUG("result : " + tools::CStringTools::Int2String(vecOrganization[iIndex]));
        vecFinalResult.push_back(vecOrganization[iIndex]);

        GetSubOrganization(GetSubOrganization(vecOrganization[iIndex]), vecFinalResult);
    }
}

vector<unsigned int> logic::SelectSubOrganizationID(unsigned int iOrganizationID)
{
    vector<unsigned int> vecFinalResult;

    //DEBUG("result has : " + tools::CStringTools::Int2String(iOrganizationID));
    //vecFinalResult.push_back(iOrganizationID);

    GetSubOrganization(GetSubOrganization(iOrganizationID), vecFinalResult);

    return vecFinalResult;
}

bool logic::SelectOrganizationTable(unsigned int iOrganizationID, std::map<std::string, std::string>& organizationInfo)
{
    DEBUG("========SelectOrganizationTable START========");

    stringstream ssSql;
    ssSql << "SELECT * FROM t_organization"
          << " WHERE Forganization_id = " << iOrganizationID;

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

    organizationInfo.insert(make_pair<string, string>("Forganization_id", oMysql.GetField(0, "Forganization_id")));
    organizationInfo.insert(make_pair<string, string>("Forganization_name", oMysql.GetField(0, "Forganization_name")));
    organizationInfo.insert(make_pair<string, string>("Forganization_desc", oMysql.GetField(0, "Forganization_desc")));
    organizationInfo.insert(make_pair<string, string>("Fsub_organization", oMysql.GetField(0, "Fsub_organization")));

    DEBUG("=========SelectOrganizationTable END=========");
    return true;
}


