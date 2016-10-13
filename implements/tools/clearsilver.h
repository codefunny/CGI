//======================================================================
//
//        Copyright (C) 2014 回收宝
//        All rights reserved
//
//        filename : clearsilver.h
//        description : ClearSilver封装类
//
//        created by 叶飞 at  2014-08-01 10:00:00
//        modified by XX at  
//
//======================================================================

#ifndef _CLEARSILVER_H_
#define _CLEARSILVER_H_

#include <ClearSilver.h>
#include <string>

namespace tools
{

class CClearSilver
{
public:
    CClearSilver();
    ~CClearSilver();
    void SetValue(const std::string& strKey, const std::string& strValue);
    void SetValue(const std::string& strKey, int iValue);
    void SetDirectory(const std::string& strDirectory);
    std::string Render(const std::string& strTemplateFile);

private:
    void PrintLog(NEOERR* err);

    HDF* m_pHDF;
    CSPARSE* m_pCS;
    CGI* m_pCGI;
};

}

#endif

