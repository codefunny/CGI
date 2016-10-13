//======================================================================
//
//        Copyright (C) 2014 回收宝
//        All rights reserved
//
//        filename : db.h
//        description : 数据库api的封装
//
//        created by 叶飞 at  2014-08-01 10:00:00
//        modified by XX at  
//
//======================================================================

#ifndef _DB_H_
#define _DB_H_

#include <string>
#include <map>
#include <vector>
#include <mysql.h>

namespace db
{

class Mysql
{
public:
    Mysql();
    ~Mysql();

    /**
     * @brief SetOption(MYSQL_OPT_CONNECT_TIMEOUT, (const char *)&iMySqlTimeout)
     */
    int SetOption(mysql_option option, const char *arg);

    /**
     * @brief Used to establist db connection.
     * @return 0 on success, -1 on failure.
     */
    int Connect();

    /**
     * @brief Used to execute db select sql query.
     * @param szSqlString indicates sql string which should include binary data.
     * @return 0 on scuesss <0 fail.
     */
    int Query(const std::string& strSql);

    /**
     * @brief Used to get query affected rows.
     * @return affected row count.
     */
    unsigned int GetAffectedRows() const;

    /**
     * @brief Used to execute db insert/update/delete sql query.
     * @param szSqlString indicates sql string which should include binary data.
     * @return 0 on scuesss <0 fail.
     */
    int Execute(const std::string& strSql);

    /**
     * @brief Used to get the count of sql query result.
     * @return result count.
     */
    unsigned int GetRecordCount() const;

    /**
     * @brief Used to get a field of a record index by iRow iField.
     * @return filed value.
     */
    const std::string GetField(unsigned int iRow, unsigned int iField);

    /**
     * @brief Used to get a field of a record index by iRow szFieldName.
     * @return filed value.
     */
    const std::string GetField(unsigned int iRow, const std::string& strFieldName);

    /**
     * @brief Used to get free memory allocated by Query.
     * @return
     */
    void FreeMem();

    const std::string GetErrMsg(void) const;
    const std::string EscapeString(const std::string& strInput) const;

private:
    /**
    * Used to store query result.
    * @return 0 onsuccess -1 on failure.
    */
    int StoreResult();
    void FreeResult();
    void InitFieldName();

private:
    typedef std::map<std::string, int> STRING2INT;

    struct MysqlInfo;
    MysqlInfo* m_pMysqlInfo;
};

}

#endif

