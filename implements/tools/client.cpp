//======================================================================
//
//        Copyright (C) 2014 回收宝
//        All rights reserved
//
//        filename : client.cpp
//        description : 测试工具
//
//        created by 叶飞 at  2014-08-01 10:00:00
//        modified by XX at  
//
//======================================================================

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <json/json.h>
#include "crypto.h"
#include "memcached.h"
#include "Markup.h"
#include "http.h"

using namespace std;

void SendPost()
{
    tools::CCurlWrapper curlWrapper;
    string strUrl = "http://203.195.154.249/api/get_logistics";
    string strData = "haha";
    string strReply;
    curlWrapper.AddHttpHeader("HOST:huishoubao.com.cn");
    curlWrapper.PostFile(strUrl, strData, strReply);
    cout << "post result : " << strReply << "\n";
}

void SendGet()
{
    tools::CCurlWrapper curlWrapper;
    string strUrl = "http://203.195.154.249/index.html";
    string strReply;
    curlWrapper.GetFile(strUrl, strReply);
    cout << "get result : " << strReply << "\n";
}

void XML()
{
    CMarkup write;

    write.SetDoc("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n");
    write.AddElem("UserInfo");
    write.IntoElem();
    write.AddElem("UserID","123");
    write.AddElem("UserID","456");
    write.OutOfElem();

    string strWrite = write.GetDoc();
    cout << strWrite << endl;

    CMarkup read;
    read.SetDoc(strWrite);
    string strRead;
    //while (read.FindChildElem("UserInfo"))
    {
        //read.IntoElem();
        read.FindChildElem("UserID");
        strRead = read.GetChildData();
        //read.OutOfElem();
    }
    cout << strRead << endl;
}

string EncryptPassWord(const string& strPassWord, const string& strSalt)
{
    string strTime33 = tools::CTime33::Encrypt(strPassWord);
    string strDBPassWord = tools::CMD5::hex_digest(strTime33 + strSalt);
    cout << "============================================\n";
    cout << "original password : " << strPassWord << endl;
    cout << "crypt password    : " << strTime33 << endl;
    cout << "salt              : " << strSalt << endl;
    cout << "db password       : " << strDBPassWord << endl;
    cout << "============================================\n";
    return strDBPassWord;
}

string Time33(const string& strInput)
{
    string strResult = tools::CTime33::Encrypt(strInput);
    cout << strInput << " : " << strResult << endl;
    return strResult;
}

string ConvertMD5(const string& strInput)
{
    string strResult = tools::CMD5::hex_digest(strInput);
    cout << strInput << " : " << strResult << endl;
    return strResult;
}

string GetLandingState(const string& strUserId)
{
    string strValue;
    tools::CMemcached oMemcached;
    if (!oMemcached.Get("login|" + strUserId, strValue))
    {
        cout << strUserId << " did not login\n";
        return "";
    }
    cout << strUserId << " : " << strValue << endl;
    return strValue;
}

Json::Value GetMyTeam()
{
    //我的队伍阵容
    Json::Value jsonMyTeam;
    Json::Value jsonMyTeamItem;
    jsonMyTeamItem["id"] = "1001";
    jsonMyTeamItem["pos"] = "19";
    jsonMyTeamItem["index"] = "9";
    jsonMyTeamItem["att"] = "1";
    jsonMyTeam.append(jsonMyTeamItem);

    jsonMyTeamItem["id"] = "1001";
    jsonMyTeamItem["pos"] = "20";
    jsonMyTeamItem["index"] = "10";
    jsonMyTeamItem["att"] = "2";
    jsonMyTeam.append(jsonMyTeamItem);

    jsonMyTeamItem["id"] = "1001";
    jsonMyTeamItem["pos"] = "21";
    jsonMyTeamItem["index"] = "11";
    jsonMyTeamItem["att"] = "3";
    jsonMyTeam.append(jsonMyTeamItem);

    jsonMyTeamItem["id"] = "1001";
    jsonMyTeamItem["pos"] = "22";
    jsonMyTeamItem["index"] = "12";
    jsonMyTeamItem["att"] = "4";
    jsonMyTeam.append(jsonMyTeamItem);

    jsonMyTeamItem["id"] = "1001";
    jsonMyTeamItem["pos"] = "23";
    jsonMyTeamItem["index"] = "13";
    jsonMyTeamItem["att"] = "5";
    jsonMyTeam.append(jsonMyTeamItem);

    jsonMyTeamItem["id"] = "1001";
    jsonMyTeamItem["pos"] = "24";
    jsonMyTeamItem["index"] = "14";
    jsonMyTeamItem["att"] = "6";
    jsonMyTeam.append(jsonMyTeamItem);

    Json::FastWriter fast_writer;
    stringstream ssResult;
    ssResult << fast_writer.write(jsonMyTeam);
    cout << "GetMyTeam : " << ssResult.str() << endl;

    return jsonMyTeam;
}

Json::Value GetEnemyWaves()
{
    //怪物的队伍阵容
    Json::Value jsonEnemyWave1;
    Json::Value jsonEnemyWaveItem;
    jsonEnemyWaveItem["id"] = "1001";
    jsonEnemyWaveItem["pos"] = "7";
    jsonEnemyWaveItem["index"] = "1";
    jsonEnemyWaveItem["att"] = "1";
    jsonEnemyWave1.append(jsonEnemyWaveItem);

    jsonEnemyWaveItem["id"] = "1002";
    jsonEnemyWaveItem["pos"] = "9";
    jsonEnemyWaveItem["index"] = "2";
    jsonEnemyWaveItem["att"] = "2";
    jsonEnemyWave1.append(jsonEnemyWaveItem);

    jsonEnemyWaveItem["id"] = "1001";
    jsonEnemyWaveItem["pos"] = "10";
    jsonEnemyWaveItem["index"] = "3";
    jsonEnemyWaveItem["att"] = "3";
    jsonEnemyWave1.append(jsonEnemyWaveItem);

    jsonEnemyWaveItem["id"] = "1002";
    jsonEnemyWaveItem["pos"] = "11";
    jsonEnemyWaveItem["index"] = "4";
    jsonEnemyWaveItem["att"] = "4";
    jsonEnemyWave1.append(jsonEnemyWaveItem);

    jsonEnemyWaveItem["id"] = "1003";
    jsonEnemyWaveItem["pos"] = "12";
    jsonEnemyWaveItem["index"] = "5";
    jsonEnemyWaveItem["att"] = "5";
    jsonEnemyWave1.append(jsonEnemyWaveItem);

    Json::Value jsonEnemyWave2;
    jsonEnemyWaveItem["id"] = "1001";
    jsonEnemyWaveItem["pos"] = "15";
    jsonEnemyWaveItem["index"] = "6";
    jsonEnemyWaveItem["att"] = "1";
    jsonEnemyWave2.append(jsonEnemyWaveItem);

    jsonEnemyWaveItem["id"] = "1002";
    jsonEnemyWaveItem["pos"] = "17";
    jsonEnemyWaveItem["index"] = "7";
    jsonEnemyWaveItem["att"] = "2";
    jsonEnemyWave2.append(jsonEnemyWaveItem);

    jsonEnemyWaveItem["id"] = "1003";
    jsonEnemyWaveItem["pos"] = "18";
    jsonEnemyWaveItem["index"] = "8";
    jsonEnemyWaveItem["att"] = "3";
    jsonEnemyWave2.append(jsonEnemyWaveItem);

    Json::Value jsonEnemyWave3;
    jsonEnemyWaveItem["id"] = "1001";
    jsonEnemyWaveItem["pos"] = "19";
    jsonEnemyWaveItem["index"] = "9";
    jsonEnemyWaveItem["att"] = "1";
    jsonEnemyWave3.append(jsonEnemyWaveItem);

    jsonEnemyWaveItem["id"] = "1001";
    jsonEnemyWaveItem["pos"] = "20";
    jsonEnemyWaveItem["index"] = "10";
    jsonEnemyWaveItem["att"] = "2";
    jsonEnemyWave3.append(jsonEnemyWaveItem);

    jsonEnemyWaveItem["id"] = "1001";
    jsonEnemyWaveItem["pos"] = "21";
    jsonEnemyWaveItem["index"] = "11";
    jsonEnemyWaveItem["att"] = "3";
    jsonEnemyWave3.append(jsonEnemyWaveItem);

    jsonEnemyWaveItem["id"] = "1001";
    jsonEnemyWaveItem["pos"] = "22";
    jsonEnemyWaveItem["index"] = "12";
    jsonEnemyWaveItem["att"] = "4";
    jsonEnemyWave3.append(jsonEnemyWaveItem);

    jsonEnemyWaveItem["id"] = "1001";
    jsonEnemyWaveItem["pos"] = "23";
    jsonEnemyWaveItem["index"] = "13";
    jsonEnemyWaveItem["att"] = "5";
    jsonEnemyWave3.append(jsonEnemyWaveItem);

    jsonEnemyWaveItem["id"] = "1001";
    jsonEnemyWaveItem["pos"] = "24";
    jsonEnemyWaveItem["index"] = "14";
    jsonEnemyWaveItem["att"] = "6";
    jsonEnemyWave3.append(jsonEnemyWaveItem);

    Json::Value jsonEnemyWaves;
    jsonEnemyWaves.append(jsonEnemyWave1);
    jsonEnemyWaves.append(jsonEnemyWave2);
    jsonEnemyWaves.append(jsonEnemyWave3);

    Json::FastWriter fast_writer;
    stringstream ssResult;
    ssResult << fast_writer.write(jsonEnemyWaves);
    cout << "GetEnemyWaves : " << ssResult.str() << endl;

    return jsonEnemyWaves;
}

Json::Value GetMapInfo()
{
    Json::Value jsonOutput;
    Json::FastWriter fast_writer;

    jsonOutput["myTeam"] = GetMyTeam();
    jsonOutput["enemyWaves"] = GetEnemyWaves();

    stringstream ssResult;
    ssResult << fast_writer.write(jsonOutput);
    cout << "GetMapInfo : " << ssResult.str() << endl;

    return jsonOutput;
}

int main(int argc, char *argv[])
{
    //SendPost();
    //SendGet();
    
    //XML();

    //Time33("GM1");
    //ConvertMD5("GM1");
    //ConvertMD5("1a5551f9164bdaf561071d26fe1af3c81234567890");

    EncryptPassWord("GM1", "1234567890");
    EncryptPassWord("GM2", "1234567890");
    EncryptPassWord("GM3", "1234567890");
    EncryptPassWord("GM4", "1234567890");
    EncryptPassWord("GM5", "1234567890");

    EncryptPassWord("yefei", "1234567890");
    EncryptPassWord("lixiaoya", "1234567890");
    EncryptPassWord("zhouxiaolei", "1234567890");
    EncryptPassWord("liujianyi", "1234567890");
    EncryptPassWord("linzhuosheng", "1234567890");
    EncryptPassWord("heyizhou", "1234567890");
    EncryptPassWord("hefan", "1234567890");
    EncryptPassWord("leiwenting", "1234567890");
    EncryptPassWord("yanghuajun", "1234567890");
    EncryptPassWord("123456", "1234567890");
    EncryptPassWord("houxueqing", "1234567890");

    //GetLandingState("1");

    //GetMapInfo();

    return 0;
}
