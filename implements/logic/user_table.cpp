#include "user_table.h"
#include <stdlib.h>
#include <sstream>
#include "error_num.h"
#include "configure.h"
#include "string_tools.h"
#include "db.h"

using namespace std;

bool logic::SelectUserTable(unsigned int iUserID, map<string, string>& mapUserInfo)
{
    DEBUG("========SelectUserTable START========");

    stringstream ssSql;
    ssSql << "SELECT * FROM t_user"
          << " INNER JOIN t_channel ON t_user.Fuser_channel = t_channel.Fchannel_id"
          << " INNER JOIN t_organization ON t_user.Forganization_id = t_organization.Forganization_id"
          << " WHERE Fuser_id = " << iUserID;

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

    mapUserInfo.insert(make_pair<string, string>("Fuser_id", oMysql.GetField(0, "Fuser_id")));
    mapUserInfo.insert(make_pair<string, string>("Fuser_channel", oMysql.GetField(0, "Fuser_channel")));
    mapUserInfo.insert(make_pair<string, string>("Fchannel_name", oMysql.GetField(0, "Fchannel_name")));
    mapUserInfo.insert(make_pair<string, string>("Fchannel_desc", oMysql.GetField(0, "Fchannel_desc")));
    mapUserInfo.insert(make_pair<string, string>("Fchannel_logo", oMysql.GetField(0, "Fchannel_logo")));
    mapUserInfo.insert(make_pair<string, string>("Fuser_name", oMysql.GetField(0, "Fuser_name")));
    mapUserInfo.insert(make_pair<string, string>("Fuser_password", oMysql.GetField(0, "Fuser_password")));
    mapUserInfo.insert(make_pair<string, string>("Fphone_num", oMysql.GetField(0, "Fphone_num")));
    mapUserInfo.insert(make_pair<string, string>("Fsalt", oMysql.GetField(0, "Fsalt")));
    mapUserInfo.insert(make_pair<string, string>("Forganization_id", oMysql.GetField(0, "Forganization_id")));
    mapUserInfo.insert(make_pair<string, string>("Forganization_name", oMysql.GetField(0, "Forganization_name")));
    mapUserInfo.insert(make_pair<string, string>("Forganization_desc", oMysql.GetField(0, "Forganization_desc")));
    mapUserInfo.insert(make_pair<string, string>("Faccount_id_list", oMysql.GetField(0, "Faccount_id_list")));

    DEBUG("=========SelectUserTable END=========");
    return true;
}

//modify by 张世柏 at 2014-10-16
/**
  *@brief: 根据用户名检查用户信息在数据库中是否存在
  *@param1: 参数1：strUserName:传入参数，用户名
  *@param2: mapUserInfo:传出参数：用户ID和用户渠道ID
  *@param3: isOldUser:传出参数: 是否是老用户
  *@return: sql执行成功返回true, 否则返回false
  */
bool logic::CheckAndSelectUserByUserName(const string& strUserName, map<string, string>& mapUserInfo, bool& isOldUser)
{
    DEBUG("========CheckAndSelectUserByUserName START========");
    db::Mysql oMysql;
     //第一步:先获取到用户的渠道ID;
    stringstream ssSqlSelectChannel;
    ssSqlSelectChannel << "SELECT Fuser_channel FROM t_user "
                       << "WHERE Fuser_id >= " << oMysql.EscapeString(WECHAT_USERID_BEGIN) << " AND Fuser_id < " << oMysql.EscapeString(WECHAT_USERID_END)
                       << " ORDER BY Fuser_id DESC LIMIT 1";
     DEBUG("sql is : " + ssSqlSelectChannel.str());

    //sql语句执行错误
    if (0 != oMysql.Query(ssSqlSelectChannel.str()))
    {
        //sql语句执行错误
        ERROR("failed to query for sql : " + ssSqlSelectChannel.str());
        return false;//执行sql语句时发生错误
    }
    else
    {
        //sql语句执行成功
    }

    //判断结果集，即判断用户是否存在
    if (0 != oMysql.GetRecordCount())
    {
        //找到该用户的信息
        mapUserInfo.insert(make_pair<string, string>("Fuser_channel", oMysql.GetField(0, "Fuser_channel")));
        DEBUG("Fuser_channel = " + mapUserInfo["Fuser_channel"]);
    }
    else
    {
        //数据库中不存在该用户
        ERROR("Get user Fuser_channel info error.");
        return false;
    }
    //第二步:再通过Fuser_name + Fchannel_name获取用户的Fuser_id
    stringstream ssSql;
    ssSql << "SELECT * FROM t_user "
          << "WHERE Fuser_name = '" << oMysql.EscapeString(strUserName)
          << "' and Fuser_channel = '" << oMysql.EscapeString(mapUserInfo["Fuser_channel"]) << "'";
    DEBUG("sql is : " + ssSql.str());
    if (0 != oMysql.Query(ssSql.str()))
    {
        //sql语句执行错误
        ERROR("failed to query for sql : " + ssSql.str());
        return false;
    }
    else
    {
        //sql语句执行成功
    }
    if (0 != oMysql.GetRecordCount())
    {
        //数据库中有该用户信息
        mapUserInfo.insert(make_pair<string, string>("Fuser_id", oMysql.GetField(0, "Fuser_id")));
        DEBUG("Fuser_id = " + mapUserInfo["Fuser_id"]);
        isOldUser = true;
    }
    else
    {
        //数据库中不存在该用户
        isOldUser = false;
    }
    DEBUG("=========CheckAndSelectUserByUserName END=========");
    return true;
}

//modify by 张世柏 at 2014-10-16
/**
  *@brief:向用户表用插入一行新的数据, 返回新数据的userid和channelid
  *@param1：strOpenID:传入参数：用户名
  *@param2: mapUserInfo:传出参数: 新插入的行对应的userid和channelid
  *@return: sql执行成功返回true, 否则返回false
  */
unsigned int logic::InsertUserTable(const string& strOpenID, map<string, string>& mapUserInfo)
{
    DEBUG("========InsertUserTable START========");
    db::Mysql oMysql;

    //执行插入
    stringstream ssSqlInsert;
    ssSqlInsert << "INSERT INTO t_user (Fuser_id, Fuser_channel, Forganization_id, Fuser_name, Fuser_desc, Fcreate_time, Fvalid_start_time, Fvalid_end_time)"
                << " SELECT Fuser_id+1 AS Fuser_id, Fuser_channel, Forganization_id, '" << oMysql.EscapeString(strOpenID) << "' AS Fuser_name, '"
                << oMysql.EscapeString(strOpenID) << "' AS Fuser_desc, NOW() AS Fcreate_time, NOW() AS Fvalid_start_time, '2099-12-30 00:00:00' AS Fvalid_end_time FROM t_user "
                << "WHERE Fuser_id >= " << oMysql.EscapeString(WECHAT_USERID_BEGIN) << " AND Fuser_id < " << oMysql.EscapeString(WECHAT_USERID_END)
                << " ORDER BY Fuser_id DESC LIMIT 1";
    DEBUG("sql is : " + ssSqlInsert.str());
    if (0 != oMysql.Execute(ssSqlInsert.str()))
    {
        //sql语句执行错误
        ERROR("failed to execute for sql : " + ssSqlInsert.str());
        return error_num::USER_ADD_FAILED;
    }
    else
    {
        //sql语句执行成功
    }

    if (1 != oMysql.GetAffectedRows())
    {
        //插入失败
        ERROR("DB has error for sql : " + ssSqlInsert.str());
        return error_num::USER_ADD_FAILED;
    }
    else
    {
        //插入成功
    }
    DEBUG("Insert into t_user a new user[ " + strOpenID + " ] success");

    //通过微信用户的OpenID名获取用户的Fuser_id和Fuser_channel
    //第一步:先获取到用户的渠道ID;
    stringstream ssSqlSelectChannel;
    ssSqlSelectChannel << "SELECT Fuser_channel FROM t_user "
                       << "WHERE Fuser_id >= " << oMysql.EscapeString(WECHAT_USERID_BEGIN) << " AND Fuser_id < " << oMysql.EscapeString(WECHAT_USERID_END)
                       << " ORDER BY Fuser_id DESC LIMIT 1";
     DEBUG("sql is : " + ssSqlSelectChannel.str());

    //sql语句执行错误
    if (0 != oMysql.Query(ssSqlSelectChannel.str()))
    {
        //sql语句执行错误
        ERROR("failed to query for sql : " + ssSqlSelectChannel.str());
        return error_num::USER_QUERY_FAILED;//执行sql语句时发生错误
    }
    else
    {
        //sql语句执行成功
    }

    //判断结果集，即判断用户是否存在
    if (0 != oMysql.GetRecordCount())
    {
        //找到该用户的信息
        mapUserInfo.insert(make_pair<string, string>("Fuser_channel", oMysql.GetField(0, "Fuser_channel")));
        DEBUG("Fuser_channel = " + mapUserInfo["Fuser_channel"]);
    }
    else
    {
        //数据库中不存在该用户
        ERROR("Get user Fuser_channel info error.");
        return error_num::USER_QUERY_FAILED;
    }

    //第二步:再通过Fuser_name + Fchannel_name获取用户的Fuser_id
    stringstream ssSqlSelectUserID;
    ssSqlSelectUserID << "SELECT Fuser_id FROM t_user "
                << "WHERE Fuser_name = '" << oMysql.EscapeString(strOpenID)
                << "' and Fuser_channel = '" << oMysql.EscapeString(mapUserInfo["Fuser_channel"]) << "'";
    DEBUG("sql is : " + ssSqlSelectUserID.str());

    //sql语句执行错误
    if (0 != oMysql.Query(ssSqlSelectUserID.str()))
    {
        ERROR("failed to query for sql : " + ssSqlSelectUserID.str());
        return error_num::USER_QUERY_FAILED;//执行sql语句时发生错误
    }
    else
    {
        //sql语句执行成功
    }

    //判断结果集，即判断用户是否存在
    if (0 != oMysql.GetRecordCount())
    {
        //找到该用户的信息
        mapUserInfo.insert(make_pair<string, string>("Fuser_id", oMysql.GetField(0, "Fuser_id")));
        DEBUG("Fuser_id = " + mapUserInfo["Fuser_id"]);
    }
    else
    {
        //数据库中不存在该用户
        ERROR("Get user Fuser_id info error.");
        return error_num::USER_QUERY_FAILED;
    }
    DEBUG("=========InsertUserTable END=========");
    return error_num::SUCCEED;
}

/**
  *modified by 张世柏 at 2014-10-24 15:50
  *@brief: select user name (openid) by userid
  *@param1: iUserID: userid
  *@param2: strUserName: user openid
  *return: return true when select success, otherwish return false
  */
bool logic::SelectUserNameByUserID(unsigned int iUserID, string& strUserName)
{
    DEBUG("========SelectUserNameByUserID START========");
    db::Mysql oMysql;
    stringstream ssSql;
    ssSql << "SELECT Fuser_name FROM t_user "
          << "WHERE Fuser_id = " << iUserID;
    if(0 != oMysql.Query(ssSql.str()))
    {
        ERROR("Sql [" + ssSql.str() + "] is query wrong!");
        return false;
    }
    if(1 != oMysql.GetRecordCount())
    {
        ERROR("There's not found user with id = [" + tools::CStringTools::Int2String(iUserID) + "]");
        return false;
    }
    else
    {
        strUserName = oMysql.GetField(0, "Fuser_name");
        DEBUG("Success to find user [" + tools::CStringTools::Int2String(iUserID) + "], Openid is [" + strUserName + "]");
    }
    DEBUG("========SelectUserNameByUserID ENG========");
    return true;
}

/**
  *created by Zhang Shibo at 2014-10-29 16:58:00
  *@brief:update AccountList value in user table
  *@param:iUserID: t_user key
  *@param:strAccountList: AccountList value
  *return:return true when success, else, return false
  */
bool logic::UpdateUserTableAccountListAndPhoneNum(unsigned int iUserID, const string& strAccountList, const string& strPhoneNum)
{
    DEBUG("======== UpdateUserTableAccountList START ========");
    db::Mysql oMysql;
    stringstream ssSql;
    ssSql << "UPDATE t_user"
          << " SET Faccount_id_list = '" << oMysql.EscapeString(strAccountList)
          << "', Fphone_num = '" << oMysql.EscapeString(strPhoneNum)
          << "' WHERE Fuser_id = " << iUserID;
    DEBUG("Sql is: [" + ssSql.str() + "]");
    if (0 != oMysql.Execute(ssSql.str()))
    {
        ERROR("Sql is execute wrong, sql is: [" + ssSql.str() + "]");
        return false;
    }
    if (1 != oMysql.GetAffectedRows())
    {
        ERROR("The numbers of affected rows isn't 1, and it should be 1.\nSql is: [" + ssSql.str() + "]");
        return false;
    }
    DEBUG("======== UpdateUserTableAccountList ENG ========");
    return true;
}

bool logic::OrderID2OrganizationID(const vector<unsigned int>& vecOrderIDList, vector<unsigned int>& vecOrganizationID)
{
    DEBUG("========OrderID2OrganizationID START========");

    if (vecOrderIDList.empty())
    {
        ERROR("order id is empty");
        return false;
    }

    db::Mysql oMysql;
    stringstream ssSql;
    ssSql << "SELECT Forganization_id FROM t_user"
          << " INNER JOIN t_order ON t_order.Fuser_id = t_user.Fuser_id"
          << " WHERE t_order.Forder_id = ";
    size_t maxSize = vecOrderIDList.size();
    for (size_t index = 0; index < maxSize; index++)
    {
        if (0 == index)
        {
            ssSql << vecOrderIDList[index];
        }
        else
        {
            ssSql << " OR t_order.Forder_id = " << vecOrderIDList[index];
        }
    }
    DEBUG("sql is : " + ssSql.str());
    if (0 != oMysql.Query(ssSql.str()))
    {
        ERROR("failed to query for sql : " + ssSql.str());
        return false;
    }

    for (unsigned int iIndex = 0; iIndex < oMysql.GetRecordCount(); iIndex++)
    {
        vecOrganizationID.push_back(::atoi(oMysql.GetField(iIndex, "Forganization_id").c_str()));
    }

    DEBUG("=========OrderID2OrganizationID END=========");

    return true;
}

