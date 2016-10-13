//======================================================================
//
//        Copyright (C) 2014 回收宝
//        All rights reserved
//
//        filename : category_table.h
//        description : 类目表
//
//        created by 叶飞 at  2014-08-01 10:00:00
//        modified by XX at  
//
//======================================================================

#ifndef _CATEGORY_TABLE_H_
#define _CATEGORY_TABLE_H_

#include <string>
#include <vector>
#include <map>

namespace logic
{

std::vector<unsigned int> SelectSubCategoryID(unsigned int iCategoryID);
bool SelectSubCategoryDepthForOne(unsigned int iCategoryID, std::vector< std::map<std::string, std::string> >& vecCategoryList);
bool SelectCategory(unsigned int iCategoryID, std::map<std::string, std::string>& categoryInfo);

}

#endif

