#include "clearsilver.h"
#include "configure.h"

using namespace std;

tools::CClearSilver::CClearSilver() : m_pHDF(NULL), m_pCS(NULL), m_pCGI(NULL)
{
    NEOERR* err = hdf_init(&m_pHDF);
    if (STATUS_OK != err)
    {
        m_pHDF = NULL;
        return;
    }

    err = cs_init(&m_pCS, m_pHDF);
    if (STATUS_OK != err)
    {
        m_pCS = NULL;
        return;
    }
/*
    err = cgi_init(&m_pCGI, NULL);
    if (STATUS_OK != err)
    {
        m_pCGI = NULL;
        return;
    }
*/
    return;
}

tools::CClearSilver::~CClearSilver()
{
    if (m_pCS)
    {
        cs_destroy(&m_pCS);
    }

    if (m_pHDF)
    {
        hdf_destroy(&m_pHDF);
    }

    if (m_pCGI)
    {
        cgi_destroy(&m_pCGI);
    }
}

void tools::CClearSilver::SetValue(const string& strKey, const string& strValue)
{
    if (m_pHDF)
    {
        hdf_set_value(m_pHDF, strKey.c_str(), strValue.c_str());
    }

    if (m_pCGI)
    {
        hdf_set_value(m_pCGI->hdf, strKey.c_str(), strValue.c_str());
    }

    return;
}

void tools::CClearSilver::SetValue(const string& strKey, int iValue)
{
    if (m_pHDF)
    {
        hdf_set_int_value(m_pHDF, strKey.c_str(), iValue);
    }

    if (m_pCGI)
    {
        hdf_set_int_value(m_pCGI->hdf, strKey.c_str(), iValue);
    }

    return;
}

void tools::CClearSilver::SetDirectory(const string& strDirectory)
{
    ::chdir(strDirectory.c_str());
}

static NEOERR* cs_output(void *ctx, char *s)
{
    string* x = (string*)ctx;
    x->append(s);

    return STATUS_OK;
}

void tools::CClearSilver::PrintLog(NEOERR* err)
{
    STRING str;
    string_init(&str);
    nerr_error_traceback(err, &str);
    ERROR(string(str.buf, str.len));
}

string tools::CClearSilver::Render(const string& strTemplateFile)
{
    NEOERR* err = cs_parse_file(m_pCS, strTemplateFile.c_str());
    if (STATUS_OK != err)
    {
        tools::CClearSilver::PrintLog(err);
        return "";
    }

    string strResult;
    err = cs_render(m_pCS, &strResult, cs_output);
    if (STATUS_OK != err)
    {
        tools::CClearSilver::PrintLog(err);
        return "";
    }
    else
    {
        return strResult;
    }


/*
    err = cgi_display(m_pCGI, strTemplateFile.c_str());
    if (STATUS_OK != err)
    {
        cgi_neo_error(m_pCGI, err);
        return "";
    }
*/
    return "";
}


