//======================================================================
//
//        Copyright (C) 2014 回收宝
//        All rights reserved
//
//        filename : crypto.h
//        description : 加密算法
//
//        created by 叶飞 at  2014-08-01 10:00:00
//        modified by XX at  
//
//======================================================================

#ifndef _CRYPTO_H_
#define _CRYPTO_H_

#include <string>

namespace tools
{

class CMD5
{
public:
    CMD5(){}
    static const std::string raw_digest(const std::string& strInput);
    static const std::string hex_digest(const std::string& strInput);
};

class CTime33
{
public:
    static const std::string Encrypt(const std::string& strInput);
};

}

#endif
