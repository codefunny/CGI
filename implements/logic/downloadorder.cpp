#include "downloadorder.h"
#include <stdlib.h>
#include <fstream>
#include "configure.h"
#include "string_tools.h"
#include "time_tools.h"
#include "login.h"
#include "user_table.h"
#include "order_table.h"
#include "organization_table.h"
#include "account_info_table.h"

using namespace std;

bool logic::DownloadOrderList(const string& strUserID, const string& strUserKey, string& strFileName)
{
    DEBUG("========DownloadOrderList START========");

    //1.0、校验登录态
    if (!user::CheckLandingState(strUserID, strUserKey))
    {
        ERROR("need to login first");
        return false;
    }

    map<string, string> mapUserTable;
    if (!logic::SelectUserTable(::atoi(strUserID.c_str()), mapUserTable))//用户ID转到机构ID
    {
        ERROR("SelectUserTable failed");
        return false;
    }
    if (mapUserTable.end() == mapUserTable.find("Forganization_id"))
    {
        ERROR("Forganization_id not found");
        return false;
    }
    vector<unsigned int> vecOrganizationID = logic::SelectSubOrganizationID(::atoi(mapUserTable["Forganization_id"].c_str()));//旗下的全量机构ID
    if (mapUserTable.end() == mapUserTable.find("Fuser_name"))
    {
        ERROR("Fuser_name not found");
        return false;
    }

    vector< map<string, string> > vecOrderList;
    unsigned int iTotal;
    CGetOrderFilter oFilter;
    if (!logic::SelectOrderTable(::atoi(strUserID.c_str()), vecOrganizationID, oFilter, vecOrderList, iTotal))
    {
        ERROR("SelectOrderTableByOrganizationID failed");
        return false;
    }

    strFileName = "/download/" + mapUserTable["Fuser_name"] + "_" + tools::CTimeTools::GetTime() + ".csv";
    ofstream excelFile(("/cdn/root" + strFileName).c_str());
    if (!excelFile)
    {
        ERROR("create file failed");
        return false;
    }

    const static string strMsBom = "\xEF\xBB\xBF";
    excelFile << strMsBom << "商品名称,回收价格,IMEI码,回收人,回收网点,回收时间,状态,备注信息,评估结果,手机,支付账号,物流单号\r\n";

    size_t maxSize = vecOrderList.size();
    for (size_t index = 0; index < maxSize; index++)
    {
        map<string, string> orderItem = vecOrderList[index];
        //modify by shuting at 2014-11-13 19:37
        if (orderItem.end() == orderItem.find("Fproduct_name"))
        {
            ERROR("Fproduct_name not found");
            //continue;
        }
        else
        {
            excelFile << orderItem["Fproduct_name"] ;
        }

        excelFile << ",";
        if (orderItem.end() == orderItem.find("Fquotation"))
        {
            ERROR("Fquotation not found");
            //continue;
        }
        else
        {
            excelFile << ::atoi(orderItem["Fquotation"].c_str()) / 100 ;
        }

        excelFile << ",";
        if (orderItem.end() == orderItem.find("FIMEI"))
        {
            ERROR("FIMEI not found");
            //continue;
        }
        else
        {
            excelFile << orderItem["FIMEI"] ;
        } 

        excelFile << ",";
        if (orderItem.end() == orderItem.find("Fuser_name"))
        {
            ERROR("Fuser_name not found");
            //continue;
        }
        else
        {
            excelFile << orderItem["Fuser_name"] ;
        }

        excelFile << ",";
        if (orderItem.end() == orderItem.find("Forganization_name"))
        {
            ERROR("Forganization_name not found");
            //continue;
        }
        else
        {
            excelFile << orderItem["Forganization_name"] ;
        }

        excelFile << ",";
        if (orderItem.end() == orderItem.find("Forder_time"))
        {
            ERROR("Forder_time not found");
            //continue;
        }
        else
        {
            excelFile << orderItem["Forder_time"] ;
        }

        excelFile << ",";
        if (orderItem.end() == orderItem.find("Forder_status"))
        {
            ERROR("Forder_status not found");
            //continue;
        }
        else
        {
            excelFile << CGlobalData::GetInstance()->GetOrderName(::atoi(orderItem["Forder_status"].c_str())) ;
        }

        excelFile << ",";
        if (orderItem.end() == orderItem.find("t_order_Fbackup"))
        {
            ERROR("t_order_Fbackup not found");
            //continue;
        }
        else
        {
            excelFile << tools::CStringTools::StrReplace(orderItem["t_order_Fbackup"], ",", "，");
        }

        excelFile << ",";
        if (orderItem.end() == orderItem.find("Fprice_input_result_for_name"))
        {
            ERROR("Fprice_input_result_for_name not found");
            //continue;
        }
        else
        {
            excelFile << orderItem["Fprice_input_result_for_name"] ;
        }

        //modified by Zhang Shibo at 2014-11-5 17:52:00
        map<string, string> mapUserInfo;
        unsigned int iUserID = ::atoi(orderItem["Fuser_id"].c_str());
        DEBUG("User id : " + orderItem["Fuser_id"]);
        if (!logic::SelectUserTable(iUserID, mapUserInfo))
        {
            ERROR("Get user [" + orderItem["Fuser_id"] + "] info failed!");
            //continue;
        }
        else
        {
            DEBUG("Get user [" + orderItem["Fuser_id"] + "] info successed!");
        }

        excelFile << ",";
        if (mapUserInfo.end() == mapUserInfo.find("Fphone_num"))
        {
            ERROR("Fphone_num not found");
            //continue;
        }
        else
        {
            excelFile << mapUserInfo["Fphone_num"] ;
            DEBUG("Fphone_num : " + mapUserInfo["Fphone_num"]);
        }

        excelFile << "," ;
        if (orderItem.end() == orderItem.find("Faccount_id"))
        {
            ERROR("Faccount_id not found");
            //continue;
        }

        if ("" == orderItem["Faccount_id"])
        {
            DEBUG("This user [" + orderItem["Fuser_id"] + "] hasn't account info in this order[" + orderItem["Forder_id"] + "]!");
        }
        else
        {
            map<string, string> mapAccountInfo;
            if (!logic::SelectAccountInfo(::atoi(orderItem["Faccount_id"].c_str()), mapAccountInfo))
            {
                ERROR("Get account [" + orderItem["Faccount_id"] + "] info failed!");
                //continue;
            }
            else
            {
                DEBUG("Get account [" + orderItem["Faccount_id"] + "] info successed!");
            }

            if (mapAccountInfo.end() == mapAccountInfo.find("Faccount_type"))
            {
                ERROR("Faccount_type not found");
                //continue;
            }
            string strAccount;
            unsigned int iAccountType = ::atoi(mapAccountInfo["Faccount_type"].c_str());
            if (ACCOUNT_ONLINEBANK == iAccountType)
            {
                DEBUG("online bank");
                if (mapAccountInfo.end() == mapAccountInfo.find("Fwy_name"))
                {
                    ERROR("Fwy_name not found");
                    //continue;
                }
                else if (mapAccountInfo.end() == mapAccountInfo.find("Fwy_bank_name"))
                {
                    ERROR("Fwy_bank_name not found");
                    //continue;
                }
                else if (mapAccountInfo.end() == mapAccountInfo.find("Fwy_bank_cardid"))
                {
                    ERROR("Fwy_bank_cardid not found");
                   //continue;
                }
                else
                {
                    strAccount = "网银: " + mapAccountInfo["Fwy_bank_name"] + " " + mapAccountInfo["Fwy_bank_cardid"];
                }

            }
            else if (ACCOUNT_ALIPAY == iAccountType)
            {
                DEBUG("Alipay");
                if (mapAccountInfo.end() == mapAccountInfo.find("Fzfb_account"))
                {
                    ERROR("Fzfb_account not found");
                    //continue;
                }
                else
                {
                    strAccount = "支付宝: " + mapAccountInfo["Fzfb_account"];
                }
            }
            DEBUG("Account info: " + strAccount);

            excelFile << strAccount ;

        }

        excelFile << "," << orderItem["Fchannel_id"];
        excelFile << "\r\n" ;
        //end modify by shuting
        //end modify
    }

    DEBUG("=========DownloadOrderList END=========");
    return true;
}

