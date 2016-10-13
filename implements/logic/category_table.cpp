#include "category_table.h"
#include <sstream>
#include "configure.h"
#include "db.h"
#include "string_tools.h"

using namespace std;

static vector<unsigned int> GetSubCategory(unsigned int iCategoryID)
{
    vector<unsigned int> vecSubCategory;

    stringstream ssSql;
    ssSql << "SELECT Fsub_category FROM t_category"
          << " WHERE Fcategory_id = " << iCategoryID;
    //DEBUG("sql is : " + ssSql.str());
    db::Mysql oMysql;
    if (0 != oMysql.Query(ssSql.str()))
    {
        ERROR("failed to query for sql : " + ssSql.str());
        return vecSubCategory;
    }
    if (1 != oMysql.GetRecordCount())
    {
        ERROR("DB records have error for sql : " + ssSql.str());
        return vecSubCategory;
    }

    string strOrgList = oMysql.GetField(0, "Fsub_category");
    if (strOrgList.empty())
    {
        return vecSubCategory;
    }
    DEBUG(tools::CStringTools::Int2String(iCategoryID) + "'s sons are " + strOrgList);
    vecSubCategory = tools::CStringTools::Split2Uint(strOrgList, "#");

    return vecSubCategory;
}

static void GetSubCategory(const vector<unsigned int>& vecCategory, vector<unsigned int>& vecFinalResult)//递归下去
{
    for (size_t iIndex = 0; iIndex < vecCategory.size(); iIndex++)
    {
        //DEBUG("result : " + tools::CStringTools::Int2String(vecCategory[iIndex]));
        vecFinalResult.push_back(vecCategory[iIndex]);

        GetSubCategory(GetSubCategory(vecCategory[iIndex]), vecFinalResult);
    }
}

vector<unsigned int> logic::SelectSubCategoryID(unsigned int iCategoryID)
{
    vector<unsigned int> vecFinalResult;

    //DEBUG("result has : " + tools::CStringTools::Int2String(iCategoryID));
    vecFinalResult.push_back(iCategoryID);

    GetSubCategory(GetSubCategory(iCategoryID), vecFinalResult);

    return vecFinalResult;
}

bool logic::SelectSubCategoryDepthForOne(unsigned int iCategoryID, vector< map<string, string> >& vecCategoryList)//仅递归一层
{
    DEBUG("=======SelectSubCategoryDepthForOne START=======");

    stringstream ssSql;
    ssSql << "SELECT Fsub_category FROM t_category"
          << " WHERE Fcategory_id = " << iCategoryID;
    DEBUG("sql is : " + ssSql.str());
    db::Mysql oMysql;
    if (0 != oMysql.Query(ssSql.str()))
    {
        ERROR("failed to query for sql : " + ssSql.str());
        return false;
    }
    if (1 != oMysql.GetRecordCount())
    {
        ERROR("DB records have error for sql : " + ssSql.str());
        return false;
    }
    string strOrgList = oMysql.GetField(0, "Fsub_category");
    DEBUG(tools::CStringTools::Int2String(iCategoryID) + "'s sons are " + strOrgList);

    vector<unsigned int> vecSubCategory = tools::CStringTools::Split2Uint(strOrgList, "#");
    if (vecSubCategory.empty())
    {
        return true;
    }
    ssSql.str("");
    ssSql << "SELECT * FROM t_category";
    size_t maxSize = vecSubCategory.size();
    for (size_t index = 0; index < maxSize; index++)
    {
        if (0 == index)
        {
            ssSql << " WHERE Fcategory_id = " << vecSubCategory[index];
        }
        else
        {
            ssSql << " OR Fcategory_id = " << vecSubCategory[index];;
        }
    }
    DEBUG("sql is : " + ssSql.str());
    if (0 != oMysql.Query(ssSql.str()))
    {
        ERROR("failed to query for sql : " + ssSql.str());
        return false;
    }
    for (unsigned int iIndex = 0; iIndex < oMysql.GetRecordCount(); iIndex++)
    {
        map<string, string> categoryItem;
        categoryItem.insert(make_pair<string, string>("Fcategory_id", oMysql.GetField(iIndex, "Fcategory_id")));
        categoryItem.insert(make_pair<string, string>("Fcategory_name", oMysql.GetField(iIndex, "Fcategory_name")));
        categoryItem.insert(make_pair<string, string>("Fsub_desc", oMysql.GetField(iIndex, "Fsub_desc")));
        categoryItem.insert(make_pair<string, string>("Fsub_category", oMysql.GetField(iIndex, "Fsub_category")));
        categoryItem.insert(make_pair<string, string>("Fbrand_logo", oMysql.GetField(iIndex, "Fbrand_logo")));

        vecCategoryList.push_back(categoryItem);
    }

    DEBUG("========SelectSubCategoryDepthForOne END========");
    return true;
}

bool logic::SelectCategory(unsigned int iCategoryID, map<string, string>& categoryInfo)
{
    DEBUG("=======SelectSubCategoryDepthForOne START=======");

    stringstream ssSql;
    ssSql << "SELECT * FROM t_category"
          << " WHERE Fcategory_id = " << iCategoryID;
    DEBUG("sql is : " + ssSql.str());
    db::Mysql oMysql;
    if (0 != oMysql.Query(ssSql.str()))
    {
        ERROR("failed to query for sql : " + ssSql.str());
        return false;
    }
    if (1 != oMysql.GetRecordCount())
    {
        ERROR("DB records have error for sql : " + ssSql.str());
        return false;
    }

    categoryInfo.insert(make_pair<string, string>("Fcategory_id", oMysql.GetField(0, "Fcategory_id")));
    categoryInfo.insert(make_pair<string, string>("Fcategory_name", oMysql.GetField(0, "Fcategory_name")));
    categoryInfo.insert(make_pair<string, string>("Fsub_desc", oMysql.GetField(0, "Fsub_desc")));
    categoryInfo.insert(make_pair<string, string>("Fsub_category", oMysql.GetField(0, "Fsub_category")));
    categoryInfo.insert(make_pair<string, string>("Fbrand_logo", oMysql.GetField(0, "Fbrand_logo")));

    DEBUG("========SelectSubCategoryDepthForOne END========");
    return true;
}

