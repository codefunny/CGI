#include "configure.h"
#include <stdlib.h>
#include <sstream>
#include "Markup.h"
#include "db.h"

using namespace std;

CConfigure* CConfigure::pInstance = NULL;

CConfigure* CConfigure::GetInstance()
{
    if (NULL == pInstance)
    {
        pInstance = new CConfigure();
    }
    return pInstance;
}

CConfigure::CConfigure()
{
    static const string strXMLFile = "/huishoubao/config/config.xml";

    CMarkup xml;
    xml.Load(strXMLFile);

    if (xml.FindChildElem("DB"))
    {
        xml.IntoElem();
        if (xml.FindChildElem("HostName"))
        {
            m_strHostName = xml.GetChildData();
        }

        if (xml.FindChildElem("UserName"))
        {
            m_strUserName = xml.GetChildData();
        }

        if (xml.FindChildElem("Password"))
        {
            m_strPassword = xml.GetChildData();
        }

        if (xml.FindChildElem("DBName"))
        {
            m_strDBName = xml.GetChildData();
        }

        if (xml.FindChildElem("CharacterSet"))
        {
            m_strCharacterSet = xml.GetChildData();
        }
    }
}

CConfigure::~CConfigure()
{
}

void CConfigure::ShowConfig() const
{
    DEBUG("HostName     : " + CConfigure::GetInstance()->GetHostName());
    DEBUG("UserName     : " + CConfigure::GetInstance()->GetUserName());
    DEBUG("Password     : " + CConfigure::GetInstance()->GetPassword());
    DEBUG("DBName       : " + CConfigure::GetInstance()->GetDBName());
    DEBUG("CharacterSet : " + CConfigure::GetInstance()->GetCharacterSet());
}

CGlobalData* CGlobalData::pInstance = NULL;

CGlobalData* CGlobalData::GetInstance()
{
    if (NULL == pInstance)
    {
        pInstance = new CGlobalData();
    }
    return pInstance;
}

CGlobalData::CGlobalData()
{
    stringstream ssSql;
    ssSql << "SELECT * FROM t_order_status";
    DEBUG("sql is : " + ssSql.str());
    db::Mysql oMysql;
    if (0 != oMysql.Query(ssSql.str()))
    {
        ERROR("failed to query for sql : " + ssSql.str());
        return;
    }

    for (unsigned int iIndex = 0; iIndex < oMysql.GetRecordCount(); iIndex++)
    {
        m_orderStatus2Name.insert(make_pair<unsigned int, string>(::atoi(oMysql.GetField(iIndex, "Forder_status_id").c_str()), oMysql.GetField(iIndex, "Forder_status_name")));
    }
}

CGlobalData::~CGlobalData()
{
}

string CGlobalData::GetOrderName(unsigned int iOrderStatus)
{
    if (m_orderStatus2Name.end() != m_orderStatus2Name.find(iOrderStatus))
    {
        return m_orderStatus2Name[iOrderStatus];
    }
    return "";
}

map<unsigned int, string> CGlobalData::GetAllOrderStatus() const
{
    return m_orderStatus2Name;
}


