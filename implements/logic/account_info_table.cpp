#include "account_info_table.h"
#include <stdlib.h>
#include <sstream>
#include "configure.h"
#include "string_tools.h"
#include "db.h"

using namespace std;

/**
  *created by Zhang Shibo at 2014-10-29 21:06:00
  *@brief: Insert into t_account_info
  *@param: mapAccountInfo: account info
  *@param: iAccountInfo: Faccount_id
  *@return: return true when execute success, else, will return false
  */
bool logic::InsertAccountInfoTable(map<string, string>& mapAccountInfo, unsigned int& iAccountID)
{
    DEBUG("======== InsertAccountInfoTable START ========");
    if (mapAccountInfo.end() == mapAccountInfo.find("Faccount_type"))
    {
        ERROR("Cannot find Faccount_type value!");
        return false;
    }
    unsigned int iAccountType = ::atoi(mapAccountInfo["Faccount_type"].c_str());
    DEBUG("Account type is: " + mapAccountInfo["Faccount_type"]);
    db::Mysql oMysql;
    stringstream ssSql;
    ssSql << "INSERT INTO t_account_info "
          << "SET Faccount_type = " << iAccountType;
    switch(iAccountType)
    {
        case ACCOUNT_ONLINEBANK:
        {
                if(mapAccountInfo.end() == mapAccountInfo.find("Fwy_name"))
                {
                    ERROR("Cannot find Fwy_name value!");
                    return false;
                }
                if(mapAccountInfo.end() == mapAccountInfo.find("Fwy_phone"))
                {
                    ERROR("Cannot find Fwy_phone value!");
                    return false;
                }
                if(mapAccountInfo.end() == mapAccountInfo.find("Fwy_bank_name"))
                {
                    ERROR("Cannot find Fwy_bank_name value!");
                    return false;
                }
                if(mapAccountInfo.end() == mapAccountInfo.find("Fwy_bank_cardid"))
                {
                    ERROR("Cannot find Fwy_bank_cardid value!");
                    return false;
                }
                ssSql << ", Fwy_name = '" << oMysql.EscapeString(mapAccountInfo["Fwy_name"])
                      << "', Fwy_phone = '" << oMysql.EscapeString(mapAccountInfo["Fwy_phone"])
                      << "', Fwy_bank_name = '" << oMysql.EscapeString(mapAccountInfo["Fwy_bank_name"])
                      << "', Fwy_bank_cardid = '" << oMysql.EscapeString(mapAccountInfo["Fwy_bank_cardid"]) << "'";
                break;
        }
        case ACCOUNT_ALIPAY:
        {
                if(mapAccountInfo.end() == mapAccountInfo.find("Fzfb_phone"))
                {
                    ERROR("Cannot find Fzfb_phone value!");
                    return false;
                }
                if(mapAccountInfo.end() == mapAccountInfo.find("Fzfb_account"))
                {
                    ERROR("Cannot find Fzfb_account value!");
                    return false;
                }
                ssSql << ", Fzfb_phone = '" << oMysql.EscapeString(mapAccountInfo["Fzfb_phone"])
                      << "', Fzfb_account = '" << oMysql.EscapeString(mapAccountInfo["Fzfb_account"]) << "'";
                break;
        }
        default :
        {
            return false;
        }
    }
    DEBUG("Sql is: [" + ssSql.str() + "].");
    if (0 != oMysql.Execute(ssSql.str()))
    {
        ERROR("Execute sql wrong![sql is: " + ssSql.str() + "].");
        return false;
    }

    if (1 != oMysql.GetAffectedRows())
    {
        ERROR("The numbers of affected rows isn't 1, and it should be 1.\n[sql is: " + ssSql.str() + "].");
        return false;
    }

    //获取新插入行的主键值
    string strSelectLastInsertID = "SELECT LAST_INSERT_ID()";
    if (0!= oMysql.Query(strSelectLastInsertID))
    {
        ERROR("Sql is query wrong!, sql is: [" + strSelectLastInsertID + "]");
        return false;
    }
    if (1 != oMysql.GetRecordCount())
    {
        ERROR("There is no result with sql: [" + strSelectLastInsertID + "]/");
        return false;
    }
    iAccountID = ::atoi(oMysql.GetField(0, 0).c_str());
    DEBUG("Account_id: " + oMysql.GetField(0, 0));
    DEBUG("======== InsertAccountInfoTable END ========");
    return true;
}

/**
  *created by Zhang Shibo at 2014-10-29 21:06:00
  *@brief: select account info from t_account_info by Faccount_id
  *@param: iAccountID: Faccount_id
  *@param: mapAccountInfo: account info
  *@return: return true when query success, else, will return false
  */
bool logic::SelectAccountInfo(unsigned int iAccountID, std::map<std::string, std::string>& mapAccountInfo)
{
    DEBUG("======== SelectAccountInfo START ========");
    db::Mysql oMysql;
    stringstream ssSql;
    ssSql << "SELECT * FROM t_account_info "
          << "WHERE Faccount_id = " << iAccountID;
    DEBUG("Sql is: [" + ssSql.str() + "]");
    if (0 != oMysql.Query(ssSql.str()))
    {
        ERROR("Sql is query wrong!, sql is: [" + ssSql.str() + "]");
        return false;
    }
    if (1 != oMysql.GetRecordCount())
    {
        ERROR("The numbers of record count isn't 1, and it should be 1.\n[sql is: " + ssSql.str() + "].");
        return false;
    }

    //查询成功
    mapAccountInfo.insert(make_pair<string, string>("Faccount_type", oMysql.GetField(0, "Faccount_type")));
    mapAccountInfo.insert(make_pair<string, string>("Fwy_name", oMysql.GetField(0, "Fwy_name")));
    mapAccountInfo.insert(make_pair<string, string>("Fwy_phone", oMysql.GetField(0, "Fwy_phone")));
    mapAccountInfo.insert(make_pair<string, string>("Fwy_bank_name", oMysql.GetField(0, "Fwy_bank_name")));
    mapAccountInfo.insert(make_pair<string, string>("Fwy_bank_cardid", oMysql.GetField(0, "Fwy_bank_cardid")));
    mapAccountInfo.insert(make_pair<string, string>("Fzfb_phone", oMysql.GetField(0, "Fzfb_phone")));
    mapAccountInfo.insert(make_pair<string, string>("Fzfb_account", oMysql.GetField(0, "Fzfb_account")));
    DEBUG("Faccount_type: " + mapAccountInfo["Faccount_type"] +
          "\nFwy_name: " + mapAccountInfo["Fwy_name"] +
          "\nFwy_phone: " + mapAccountInfo["Fwy_phone"] +
          "\nFwy_bank_name: " + mapAccountInfo["Fwy_bank_name"] +
          "\nFwy_bank_cardid: " + mapAccountInfo["Fwy_bank_cardid"] +
          "\nFzfb_phone: " + mapAccountInfo["Fzfb_phone"] +
          "\nFzfb_account: " + mapAccountInfo["Fzfb_account"]);
    DEBUG("======== SelectAccountInfo END ========");
    return true;
}