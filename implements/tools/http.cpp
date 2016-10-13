#include "http.h"
#include <sstream>

using namespace std;
using namespace tools;

CCurlWrapper::CCurlWrapper () :
    m_pCurl(NULL)
{
    //curl_global_init(CURL_GLOBAL_ALL);
    Init();
}

CCurlWrapper::~CCurlWrapper ()
{
    curl_easy_cleanup(m_pCurl);
    m_pCurl = NULL;
}

int CCurlWrapper::Init ()
{
    if (m_pCurl)
    {
        curl_easy_cleanup(m_pCurl);
        m_pCurl = NULL;
    }

    m_pCurl = curl_easy_init();
    if (m_pCurl == NULL)
    {
        return -1;
    }

    // set connection timeout to 10's
    curl_easy_setopt(m_pCurl, CURLOPT_CONNECTTIMEOUT, 10);

    // set timeout to 30's
    curl_easy_setopt(m_pCurl, CURLOPT_TIMEOUT, 30);

    // accept identiy, deflate, and gzip encoding. (Accept-Encoding: )
    curl_easy_setopt(m_pCurl, CURLOPT_ENCODING, "gzip, deflate");

    // set user-agent to that of MSIE6
    curl_easy_setopt(m_pCurl, CURLOPT_USERAGENT, "Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1; .NET CLR 1.1.4322; .NET CLR 2.0.50727)");

    // let curl to follow location (auto handle HTTP 301, 302)
    curl_easy_setopt(m_pCurl, CURLOPT_FOLLOWLOCATION, 1);

    m_setHeaders.clear();
    m_setHeaders.insert("Accept-Language: zh-cn");

    // output debug info (for debug only)
    //curl_easy_setopt(m_pCurl, CURLOPT_VERBOSE, true);
    curl_easy_setopt(m_pCurl, CURLOPT_VERBOSE, false);

    return 0;
}

void CCurlWrapper::AddHttpHeader (const string& strHeader)
{
    m_setHeaders.insert(strHeader);
}

//modified by Zhang Shibo at 2014-11-14 22:32:00
//modify brief: modify the return value with different num.
int CCurlWrapper::PostFile (const string& strUrl, const string& strRequest, string& strReply)
{
    if (m_pCurl == NULL)
        return -1;

    int iRet;
    if (strUrl.empty())
    {
        return -2;
    }

    curl_easy_setopt(m_pCurl, CURLOPT_URL, strUrl.c_str());
    stringstream ssReply;
    curl_easy_setopt(m_pCurl, CURLOPT_WRITEDATA, &ssReply);
    curl_easy_setopt(m_pCurl, CURLOPT_WRITEFUNCTION, WriteDataCallback);

    // set post data
    if (strRequest.empty())
    {
        return -3;
    }
    curl_easy_setopt(m_pCurl, CURLOPT_POST, true);
    curl_easy_setopt(m_pCurl, CURLOPT_POSTFIELDS, strRequest.c_str());
    curl_easy_setopt(m_pCurl, CURLOPT_POSTFIELDSIZE, strRequest.length());

    // autosave cookie with the handle
    curl_easy_setopt(m_pCurl, CURLOPT_COOKIEFILE, "/dev/null");

    // set header
    struct curl_slist *headers = NULL;
    if (!m_setHeaders.empty())
    {
        for (set<string>::iterator it = m_setHeaders.begin(); it != m_setHeaders.end(); ++it)
            headers = curl_slist_append(headers, it->c_str());
    }
    curl_easy_setopt(m_pCurl, CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(m_pCurl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(m_pCurl, CURLOPT_SSL_VERIFYHOST, 0L);

    //curl_easy_setopt(m_pCurl, CURLOPT_WRITEDATA, &ssReply);
    //curl_easy_setopt(m_pCurl, CURLOPT_WRITEFUNCTION, WriteDataCallback);

    // perform
    iRet = curl_easy_perform(m_pCurl);
    if (iRet == CURLE_OK)
    {
        strReply = ssReply.str();
    }

    if (headers)
        curl_slist_free_all(headers);

    return (iRet == CURLE_OK) ? 0 : iRet;
}


//modified by Zhang Shibo at 2014-11-14 22:32:00
//modify brief: modify the return value with different num.
int CCurlWrapper::GetFile (const string& strUrl, string& strReply)
{
    if (m_pCurl == NULL)
        return -1;

    int iRet;
    if (strUrl.empty())
    {
        return -2;
    }

    curl_easy_setopt(m_pCurl, CURLOPT_URL, strUrl.c_str());
    stringstream ssReply;
    curl_easy_setopt(m_pCurl, CURLOPT_WRITEDATA, &ssReply);
    curl_easy_setopt(m_pCurl, CURLOPT_WRITEFUNCTION, WriteDataCallback);

    curl_easy_setopt(m_pCurl, CURLOPT_HTTPGET, true);

    // autosave cookie with the handle
    curl_easy_setopt(m_pCurl, CURLOPT_COOKIEFILE, "/dev/null");

    // set header
    struct curl_slist *headers = NULL;
    if (!m_setHeaders.empty())
    {
        for (set<string>::iterator it = m_setHeaders.begin(); it != m_setHeaders.end(); ++it)
            headers = curl_slist_append(headers, it->c_str());
    }
    curl_easy_setopt(m_pCurl, CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(m_pCurl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(m_pCurl, CURLOPT_SSL_VERIFYHOST, 0L);

    //curl_easy_setopt(m_pCurl, CURLOPT_WRITEDATA, &ssReply);
    //curl_easy_setopt(m_pCurl, CURLOPT_WRITEFUNCTION, WriteDataCallback);

    // perform
    iRet = curl_easy_perform(m_pCurl);
    if (iRet == CURLE_OK)
    {
        strReply = ssReply.str();
    }

    if (headers)
        curl_slist_free_all(headers);

    return (iRet == CURLE_OK) ? 0 : iRet;
}

size_t CCurlWrapper::WriteDataCallback (void *ptr, size_t size, size_t nCount, void *pData)
{
    stringstream* pTmp = (stringstream*) pData;
    pTmp->write((char*) ptr, size * nCount);
    return size * nCount;
}
