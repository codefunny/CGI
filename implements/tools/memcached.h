//======================================================================
//
//        Copyright (C) 2014 回收宝
//        All rights reserved
//
//        filename : memcached.h
//        description : 缓存类
//
//        created by 叶飞 at  2014-08-01 10:00:00
//        modified by XX at  
//
//======================================================================

#ifndef _MEMCACHED_H_
#define _MEMCACHED_H_

#include <string>

namespace tools
{

class CMemcached
{
public:
    CMemcached();
    ~CMemcached();
    bool Get(const std::string& strKey, std::string& strValue);
    bool Set(const std::string& strKey, const std::string& strValue);
    bool Delete(const std::string& strKey);

private:
    void *mem;
};

}

#endif

