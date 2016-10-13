//======================================================================
//
//        Copyright (C) 2014 回收宝
//        All rights reserved
//
//        filename : http.h
//        description : http工具类
//
//        created by 叶飞 at  2014-10-10 15:33:00
//        modified by XX at  
//
//======================================================================

#ifndef _HTTP_H_
#define _HTTP_H_

#include <string>
#include <set>
#include <curl/curl.h>

namespace tools
{

class CCurlWrapper
{
public:
    CCurlWrapper ();
    ~CCurlWrapper ();

    int PostFile (const std::string& strUrl, const std::string& strRequest, std::string& strReply);
    int GetFile (const std::string& strUrl, std::string& strReply);
    void AddHttpHeader (const std::string& strHeader);

private:
    int Init ();
    static size_t WriteDataCallback (void *ptr, size_t size, size_t nCount, void *pData);

    CURL *m_pCurl;
    std::set<std::string> m_setHeaders;

};

}

#endif

