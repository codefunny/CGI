//======================================================================
//
//        Copyright (C) 2014 回收宝
//        All rights reserved
//
//        filename : organization_table.h
//        description : 机构表
//
//        created by 叶飞 at  2014-08-01 10:00:00
//        modified by XX at  
//
//======================================================================

#ifndef _ORGANIZATION_TABLE_H_
#define _ORGANIZATION_TABLE_H_

#include <string>
#include <vector>
#include <map>

namespace logic
{

std::vector<unsigned int> SelectSubOrganizationID(unsigned int iOrganizationID);
bool SelectOrganizationTable(unsigned int iOrganizationID, std::map<std::string, std::string>& organizationInfo);

}

#endif


