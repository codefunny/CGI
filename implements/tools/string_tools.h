//======================================================================
//
//        Copyright (C) 2014 回收宝
//        All rights reserved
//
//        filename : string_tools.h
//        description : string工具类
//
//        created by 叶飞 at  2014-08-01 10:00:00
//        modified by Zhang Shibo at 2014-11-3 20:19:00
//
//======================================================================

#ifndef _STRING_TOOLS_H_
#define _STRING_TOOLS_H_

#include <string>
#include <vector>
#include <map>

namespace tools
{

class CStringTools
{
public:
    static std::string Int2String(int iInput);
    static std::string Int2String(unsigned int iInput);
    static std::string Int2String(unsigned long iInput);
    static std::vector<std::string> Split2String(const std::string& strInput, const std::string& strSeparator);
    static std::vector<unsigned int> Split2Uint(const std::string& strInput, const std::string& strSeparator);
    static std::map<std::string, std::string> SplitTwice2String(const std::string& strInput, const std::string& strSeparator, const std::string& strSeparator2);
    static std::map<unsigned int, unsigned int> SplitTwice2Uint(const std::string& strInput, const std::string& strSeparator, const std::string& strSeparator2);

    static void Show(const std::vector<unsigned int>& vecInput);
    static void Show(const std::map<std::string, std::string>& mapInput);

    static std::string Trim(const std::string& strInput);

    static std::string UrlDecode(const std::string& strUrl);
    //modified by Zhang Shibo at 2014-11-3 20:19:00
    static bool IsHostName(const std::string & strUrl);
    //end modify

    //modified by shuting at 2014-11-14 11:04:00
    static std::string StrReplace(const std::string &strInput, const std::string &strSrc, const std::string &strDec);
    //end modify by shuting

    template <class Type>
    static std::map<Type, unsigned int> Vector2Map(std::vector<Type> vecInput)
    {
        std::map<Type, unsigned int> mapResult;
        size_t maxSize = vecInput.size();
        for (size_t index = 0; index < maxSize; index++)
        {
            mapResult.insert(std::make_pair<Type, unsigned int>(vecInput[index], 1));
        }
        return mapResult;
    }
};

}

#endif
