#include <db.h>
#include "configure.h"
#include "string_tools.h"

using namespace std;
using namespace db;

struct Mysql::MysqlInfo
{
    MYSQL connection;

    string strHostName;
    string strUserName;
    string strPassword;
    string strDBName;
    string strCharacterSet;
    int iPort;
    bool bConnected;
    unsigned int iFields;
    unsigned int iRows;
    MYSQL_RES *result;
    MYSQL_ROW row;
    STRING2INT resultByName;
    bool bFieldIndexInitialized;
    string strErrMsg;
};

Mysql::Mysql()
{
    m_pMysqlInfo = new MysqlInfo();
    mysql_init(&m_pMysqlInfo->connection);

    /*
    m_pMysqlInfo->strHostName = "localhost";
    m_pMysqlInfo->strUserName = "root";
    m_pMysqlInfo->strPassword = "123456";
    m_pMysqlInfo->strDBName = "recycle";
    m_pMysqlInfo->strCharacterSet = "UTF8";
    */
    m_pMysqlInfo->strHostName = CConfigure::GetInstance()->GetHostName();
    m_pMysqlInfo->strUserName = CConfigure::GetInstance()->GetUserName();
    m_pMysqlInfo->strPassword = CConfigure::GetInstance()->GetPassword();
    m_pMysqlInfo->strDBName = CConfigure::GetInstance()->GetDBName();
    m_pMysqlInfo->strCharacterSet = CConfigure::GetInstance()->GetCharacterSet();

    m_pMysqlInfo->iPort = MYSQL_PORT;
    m_pMysqlInfo->bConnected = false;
    m_pMysqlInfo->iFields = 0;
    m_pMysqlInfo->iRows = 0;
    m_pMysqlInfo->result = NULL;
    m_pMysqlInfo->bFieldIndexInitialized = false;
}

Mysql::~Mysql()
{
    if (m_pMysqlInfo->bConnected)
    {
        FreeResult();
        mysql_close(&m_pMysqlInfo->connection);
    }
    delete m_pMysqlInfo;
}

int Mysql::SetOption(mysql_option option, const char *arg)
{
    return mysql_options(&m_pMysqlInfo->connection, option, arg);
}

int Mysql::Connect()
{
    if (!m_pMysqlInfo->bConnected)
    {
        if (NULL == mysql_real_connect(&m_pMysqlInfo->connection,
                                       m_pMysqlInfo->strHostName.c_str(),
                                       m_pMysqlInfo->strUserName.c_str(),
                                       m_pMysqlInfo->strPassword.c_str(),
                                       m_pMysqlInfo->strDBName.c_str(),
                                       m_pMysqlInfo->iPort, NULL, 0))
        {
            m_pMysqlInfo->strErrMsg = mysql_error(&m_pMysqlInfo->connection);
            ERROR(m_pMysqlInfo->strErrMsg);
            return -1;
        }
        mysql_set_character_set(&m_pMysqlInfo->connection, m_pMysqlInfo->strCharacterSet.c_str());
        m_pMysqlInfo->bConnected = true;
    }
    return 0;
}

int Mysql::Query(const string& strSql)
{
    Connect();

    if(0 != mysql_ping(&m_pMysqlInfo->connection))
    {
        FreeResult();
        mysql_close(&m_pMysqlInfo->connection);
        m_pMysqlInfo->bConnected = false;
        if(0 != Connect())
        {
            m_pMysqlInfo->bConnected = false;
            m_pMysqlInfo->strErrMsg = mysql_error(&m_pMysqlInfo->connection);
            WARN(m_pMysqlInfo->strErrMsg);
            return -1;
        }
    }

    if (0 != mysql_real_query(&m_pMysqlInfo->connection, strSql.c_str(), strSql.length()))
    {
        m_pMysqlInfo->strErrMsg = mysql_error(&m_pMysqlInfo->connection);
        ERROR(m_pMysqlInfo->strErrMsg);
        return -2;
    }

    if(0 != StoreResult())
    {
        return -3;
    }

    return 0;
}

int Mysql::Execute(const string& strSql)
{
    Connect();

    if (0 != mysql_ping(&m_pMysqlInfo->connection))
    {
        FreeResult();
        mysql_close(&m_pMysqlInfo->connection);
        m_pMysqlInfo->bConnected = false;
        if (0 != Connect())
        {
            m_pMysqlInfo->bConnected = false;
            m_pMysqlInfo->strErrMsg = mysql_error(&m_pMysqlInfo->connection);
            WARN(m_pMysqlInfo->strErrMsg);
            return -1;
        }
    }

    if (0 != mysql_real_query(&m_pMysqlInfo->connection, strSql.c_str(), strSql.length()))
    {
        m_pMysqlInfo->strErrMsg = mysql_error(&m_pMysqlInfo->connection);
        ERROR(m_pMysqlInfo->strErrMsg);
        return -2;
    }

    return 0;
}

void Mysql::FreeResult()
{
    if (NULL != m_pMysqlInfo->result)
    {
        mysql_free_result (m_pMysqlInfo->result);
        m_pMysqlInfo->result = NULL;
    }

    m_pMysqlInfo->iFields = 0;
    m_pMysqlInfo->iRows = 0;

    if (m_pMysqlInfo->bFieldIndexInitialized)
    {
        m_pMysqlInfo->resultByName.erase(m_pMysqlInfo->resultByName.begin(), m_pMysqlInfo->resultByName.end());
        m_pMysqlInfo->bFieldIndexInitialized = false;
    }

    return;
}

void Mysql::FreeMem()
{
    FreeResult();
}

int Mysql::StoreResult()
{
    FreeResult();

    m_pMysqlInfo->result = mysql_store_result(&m_pMysqlInfo->connection);
    if (NULL == m_pMysqlInfo->result)
    {
        m_pMysqlInfo->strErrMsg = mysql_error(&m_pMysqlInfo->connection);
        ERROR(m_pMysqlInfo->strErrMsg);
        return -1;
    }

    m_pMysqlInfo->iRows = mysql_num_rows(m_pMysqlInfo->result);
    m_pMysqlInfo->iFields = mysql_num_fields(m_pMysqlInfo->result);

    return 0;
}

void Mysql::InitFieldName()
{
    if ((!m_pMysqlInfo->bFieldIndexInitialized) && (NULL != m_pMysqlInfo->result))
    {
        MYSQL_FIELD *field;
        unsigned int iRowCount = mysql_num_fields(m_pMysqlInfo->result);
        for(unsigned int i = 0; i < iRowCount; i++)
        {
            field = mysql_fetch_field(m_pMysqlInfo->result);
            m_pMysqlInfo->resultByName[field->name] = i;
        }
        m_pMysqlInfo->bFieldIndexInitialized = true;
    }

    return;
}

const string Mysql::GetField(unsigned int iRow, unsigned int iField)
{
    //DEBUG(tools::CStringTools::Int2String(iRow) + " : " + tools::CStringTools::Int2String(iField) + " : " + tools::CStringTools::Int2String(m_pMysqlInfo->iFields));

    if (iRow >= m_pMysqlInfo->iRows || iField >= m_pMysqlInfo->iFields || NULL == m_pMysqlInfo->result)
    {
        return "";
    }
    else
    {
        mysql_data_seek(m_pMysqlInfo->result, iRow);
        m_pMysqlInfo->row = mysql_fetch_row(m_pMysqlInfo->result);
        if (NULL == m_pMysqlInfo->row)
        {
            return "";
        }
        else
        {
            return m_pMysqlInfo->row[iField] ? m_pMysqlInfo->row[iField] : "";
        }
    }
    return "";
}

const string Mysql::GetField(unsigned int iRow, const string& strFieldName)
{
    //DEBUG(tools::CStringTools::Int2String(iRow) + " : " + strFieldName);

    InitFieldName();

    if (strFieldName.empty())
    {
        return "";
    }

    if (m_pMysqlInfo->resultByName.find(strFieldName) == m_pMysqlInfo->resultByName.end())
    {
        return "";
    }

    return GetField(iRow, m_pMysqlInfo->resultByName[strFieldName]);
}

unsigned int Mysql::GetRecordCount() const
{
    return m_pMysqlInfo->iRows;
}

unsigned int Mysql::GetAffectedRows() const
{
    my_ulonglong iNumRows = mysql_affected_rows(&m_pMysqlInfo->connection);
    return (unsigned int)iNumRows;
}

const string Mysql::GetErrMsg() const
{
    return m_pMysqlInfo->strErrMsg;
}

const string Mysql::EscapeString(const string& strInput) const
{
    char *dest = new char[2 * strInput.length() + 1];

    mysql_real_escape_string(&m_pMysqlInfo->connection, dest, strInput.c_str(), strInput.length());

    string strOutput = dest;
    delete []dest;

    return strOutput;
}

