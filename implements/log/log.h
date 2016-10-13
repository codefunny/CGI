//======================================================================
//
//        Copyright (C) 2014 回收宝
//        All rights reserved
//
//        filename : log.h
//        description : 日志类
//
//        created by 叶飞 at  2014-08-01 10:00:00
//        modified by XX at  
//
//======================================================================

#ifndef _LOG_H_
#define _LOG_H_

#include <string>

namespace log
{

void DEBUG_LOG(const std::string& strInfo, const std::string& strFile, int iLine);
void ERROR_LOG(const std::string& strInfo, const std::string& strFile, int iLine);
void WARN_LOG(const std::string& strInfo, const std::string& strFile, int iLine);

class CLog
{
public:
    static CLog* Initialize(const std::string& strLogFile);

private:
    CLog(const std::string& strLogFile);
    static CLog* pInstance;
};

}
#endif

