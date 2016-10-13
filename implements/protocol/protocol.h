//======================================================================
//
//        Copyright (C) 2014 回收宝
//        All rights reserved
//
//        filename : protocol.h
//        description : 协议接口
//
//        created by 叶飞 at  2014-08-01 10:00:00
//        modified by 张世柏 at 2014-11-13 10:36:00
//
//======================================================================

#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

#include <string>
#include <map>

namespace protocol
{

bool UnPacking(const std::string& strRequest, const std::string& strKey, std::string& strValue);//协议与实现耦合小，但解析效率低
bool UnPackingByJson(const std::string& strRequest, const std::string& strKey, std::string& strValue);
//modified by Zhang Shibo at 2014-11-13 10:36:00
//brief:解析出整型的参数
bool UnPackingByJson(const std::string& strRequest, const std::string& strKey, int& iValue);
//end modify

bool UnPackingByXML(const std::string& strRequest, const std::string& strKey, std::string& strValue);

std::string Packing(const std::map<std::string, std::string>& mapRequest);
std::string PackingByJson(const std::map<std::string, std::string>& mapRequest);
std::string PackingByXML(const std::map<std::string, std::string>& mapRequest);

}

#endif

