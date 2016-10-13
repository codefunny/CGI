#include "login.h"
#include <stdlib.h>
#include <string>
#include <map>
#include <sstream>
#include <time.h>
#include "configure.h"
#include "error_num.h"
#include "crypto.h"
#include "db.h"
#include "memcached.h"
#include "time_tools.h"
#include "string_tools.h"
#include "channel_table.h"
#include "user_table.h"

using namespace std;

//modified by 张世柏 at 2014-10-18
//add "user::" namespace before SaveLandingState
void user::SaveLandingState(const string& strUserID, const string& strUserKey)
{
    tools::CMemcached oMemcached;
    if (oMemcached.Set("login|" + strUserID, strUserKey))
    {
        DEBUG("uid[" + strUserID + "], key[" + strUserKey + "] save successfully");
    }
    else
    {
        ERROR("uid[" + strUserID + "], key[" + strUserKey + "] save faield");
    }
}

bool user::CheckLandingState(const string& strUserID, const string& strUserKey)
{
    string strValue;
    tools::CMemcached oMemcached;
    if (!oMemcached.Get("login|" + strUserID, strValue))
    {
        ERROR("failed to get login state for uid : " + strUserID);
        return false;
    }
    if (strUserKey != strValue)
    {
        ERROR("uid[" + strUserID + "], request key[" + strUserKey + "], presave key[" + strValue + "]");
        return false;
    }
    DEBUG("uid[" + strUserID + "], key[" + strUserKey + "] has logined already");
    return true;
}

unsigned int user::ChannelFuzzy(unsigned int iUserChannel, const string& strUserName)
{
    unsigned int iUserChannelAfter = iUserChannel;
    if (TO_B_DEFAULT_CHANNEL == iUserChannel)
    {
        if (strUserName.size() >= 6)
        {
            if (logic::ChannelTable_UserBegins2ChannelID(strUserName.substr(0, 6), iUserChannelAfter))
            {
                DEBUG("channel convert from " + tools::CStringTools::Int2String(iUserChannel) + " to " + tools::CStringTools::Int2String(iUserChannelAfter));
                return iUserChannelAfter;
            }
        }
        if (strUserName.size() >= 8)
        {
            if (logic::ChannelTable_UserBegins2ChannelID(strUserName.substr(0, 8), iUserChannelAfter))
            {
                DEBUG("channel convert from " + tools::CStringTools::Int2String(iUserChannel) + " to " + tools::CStringTools::Int2String(iUserChannelAfter));
                return iUserChannelAfter;
            }
        }
    }
    return iUserChannel;
}

bool user::Login(unsigned int iUserChannel, const string& strUserName, const string& strUserPassword, string& strUserID, string& strUserKey)
{
    DEBUG("==========login start==========");

    db::Mysql oMysql;
    if (oMysql.EscapeString(strUserName) != strUserName || oMysql.EscapeString(strUserPassword) != strUserPassword)
    {
        ERROR("illegal userName[" + strUserName + "], or illegal userPassword[" + strUserPassword + "]");
        return false;
    }

    stringstream ssSql;
    ssSql << "SELECT * FROM t_user"
          << " WHERE Fuser_channel = '" << iUserChannel << "'"
          << " AND Fuser_name = '" << strUserName << "'";
    DEBUG("sql is : " + ssSql.str());
    if (0 != oMysql.Query(ssSql.str()))
    {
        ERROR("failed to query for sql : " + ssSql.str());
        return false;
    }
    if (1 != oMysql.GetRecordCount())
    {
        ERROR("DB records have error for sql : " + ssSql.str());
        return false;
    }

    strUserID = oMysql.GetField(0, "Fuser_id");
    string strDBUserPassword = oMysql.GetField(0, "Fuser_password");
    string strDBSalt = oMysql.GetField(0, "Fsalt");
    if (tools::CMD5::hex_digest(strUserPassword + strDBSalt) != strDBUserPassword)
    {
        ERROR("error password for user : " + strUserName);
        return false;
    }

    string strTime = tools::CTimeTools::GetTime();
    stringstream ssKey;
    ssKey << strUserName << strTime << ::rand();
    strUserKey = strTime + "-" + tools::CMD5::hex_digest(ssKey.str());

    DEBUG("userID[" + strUserID + "], userKey[" + strUserKey + "]");
    SaveLandingState(strUserID, strUserKey);

    DEBUG("===========login end===========");
    return true;
}

bool user::ModifyPassword(unsigned int iUserChannel, const string& strUserName, const string& strOldPassword, const string& strNewPassword, string& strUserID, string& strUserKey)
{
    DEBUG("==========ModifyPassword start==========");

    db::Mysql oMysql;
    if (oMysql.EscapeString(strUserName) != strUserName || oMysql.EscapeString(strOldPassword) != strOldPassword || oMysql.EscapeString(strNewPassword) != strNewPassword)
    {
        ERROR("illegal userName[" + strUserName + "], or illegal oldPassword[" + strOldPassword + "], or illegal newPassword[" + strNewPassword + "]");
        return false;
    }

    stringstream ssSql;
    ssSql << "SELECT * FROM t_user"
          << " WHERE Fuser_channel = '" << iUserChannel << "'"
          << " AND Fuser_name = '" << strUserName << "'";
    DEBUG("sql is : " + ssSql.str());
    if (0 != oMysql.Query(ssSql.str()))
    {
        ERROR("failed to query for sql : " + ssSql.str());
        return false;
    }
    if (1 != oMysql.GetRecordCount())
    {
        ERROR("DB records have error for sql : " + ssSql.str());
        return false;
    }

    strUserID = oMysql.GetField(0, "Fuser_id");
    string strDBUserPassword = oMysql.GetField(0, "Fuser_password");
    string strDBSalt = oMysql.GetField(0, "Fsalt");
    if (tools::CMD5::hex_digest(strOldPassword + strDBSalt) != strDBUserPassword)
    {
        ERROR("error password for user : " + strUserName);
        return false;
    }

    ssSql.str("");
    ssSql << "UPDATE t_user SET Fuser_password = '" << tools::CMD5::hex_digest(strNewPassword + strDBSalt) << "'"
          << " WHERE Fuser_id = " << strUserID;
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
    DEBUG("modify password for " + strUserName + " successfully !");

    string strTime = tools::CTimeTools::GetTime();
    stringstream ssKey;
    ssKey << strUserName << strTime << ::rand();
    strUserKey = strTime + "-" + tools::CMD5::hex_digest(ssKey.str());

    DEBUG("userID[" + strUserID + "], userKey[" + strUserKey + "]");
    SaveLandingState(strUserID, strUserKey);

    DEBUG("===========ModifyPassword end===========");
    return true;
}

unsigned int user::CheckLandingStateWithChannel(unsigned int iUserID, const string& strUserKey, unsigned int iChannelID, unsigned int& iTrueChannelID, bool& bNeedLogin)
{
    if (user::CheckLandingState(tools::CStringTools::Int2String(iUserID), strUserKey))//已登录
    {
        map<string, string> mapUserTable;
        if (!logic::SelectUserTable(iUserID, mapUserTable))
        {
            ERROR("SelectUserTable failed");
            iTrueChannelID = 0;
            return error_num::USER_UNKNOWN;
        }
        if (mapUserTable.end() == mapUserTable.find("Fuser_channel"))
        {
            ERROR("invalid user");
            iTrueChannelID = 0;
            return error_num::DB_USER;
        }
        DEBUG("Fuser_channel is : " + mapUserTable["Fuser_channel"]);

        if (tools::CStringTools::Int2String(iChannelID) == mapUserTable["Fuser_channel"])
        {
            DEBUG("userID " + tools::CStringTools::Int2String(iUserID) + " is logined, and channelID is " + mapUserTable["Fuser_channel"]);
            bNeedLogin = true;
            iTrueChannelID = iChannelID;
            return error_num::SUCCEED;
        }
        else
        {
            ERROR("userID and channelID are not matching");
            iTrueChannelID = 0;
            return error_num::USER_UNKNOWN;
        }
    }
    else//未登录
    {
        map<string, string> mapChannelTable;
        if (!logic::SelectChannelTable(iChannelID, mapChannelTable))
        {
            ERROR("SelectChannelTable failed");
            iTrueChannelID = 0;
            return error_num::DB_CHANNEL;
        }
        if (mapChannelTable.end() == mapChannelTable.find("Fis_need_login"))
        {
            ERROR("Fis_need_login is not found");
            iTrueChannelID = 0;
            return error_num::DB_CHANNEL;
        }
        DEBUG("Fis_need_login is : " + mapChannelTable["Fis_need_login"]);

        if (0 == ::atoi(mapChannelTable["Fis_need_login"].c_str()))//不需要用户登录
        {
            DEBUG("userID " + tools::CStringTools::Int2String(iUserID) + " is not need to login, and channelID is " + tools::CStringTools::Int2String(iTrueChannelID));
            bNeedLogin = false;
            iTrueChannelID = iChannelID;
            return error_num::SUCCEED;
        }
        else
        {
            ERROR("userID " + tools::CStringTools::Int2String(iUserID) + " needs to login for channelID is " + tools::CStringTools::Int2String(iTrueChannelID));
            bNeedLogin = true;
            iTrueChannelID = 0;
            return error_num::USER_UNLOGIN;
        }
    }

    return error_num::SYSTEM_BUG;
}


