#include "string_tools.h"
#include <stdlib.h>
#include <sstream>
#include "configure.h"

using namespace std;

string tools::CStringTools::Int2String(int iInput)
{
    stringstream ssInput;
    ssInput << iInput;
    return ssInput.str();
}

string tools::CStringTools::Int2String(unsigned int iInput)
{
    stringstream ssInput;
    ssInput << iInput;
    return ssInput.str();
}

string tools::CStringTools::Int2String(unsigned long iInput)
{
    stringstream ssInput;
    ssInput << iInput;
    return ssInput.str();
}

//注意：当字符串为空时，也会返回一个空字符串
vector<string> tools::CStringTools::Split2String(const string& strInput, const string& strSeparator)
{
    vector<string> vecResult;

    size_t last = 0;
    size_t index = strInput.find_first_of(strSeparator, last);
    while (string::npos != index)
    {
        string strElement = strInput.substr(last, index - last);
        if (!strElement.empty())
        {
            vecResult.push_back(strElement);
        }
        last = index + 1;
        index = strInput.find_first_of(strSeparator, last);
    }
    string strElement = strInput.substr(last, index - last);
    if (!strElement.empty())
    {
        vecResult.push_back(strElement);
    }

    return vecResult;
}

vector<unsigned int> tools::CStringTools::Split2Uint(const string& strInput, const string& strSeparator)
{
    vector<unsigned int> vecResult;

    size_t last = 0;
    size_t index = strInput.find_first_of(strSeparator, last);
    while (string::npos != index)
    {
        string strElement = strInput.substr(last, index - last);
        if (!strElement.empty())
        {
            vecResult.push_back(::atoi(strElement.c_str()));
        }
        last = index + 1;
        index = strInput.find_first_of(strSeparator, last);
    }
    string strElement = strInput.substr(last, index - last);
    if (!strElement.empty())
    {
        vecResult.push_back(::atoi(strElement.c_str()));
    }

    return vecResult;
}

map<string, string> tools::CStringTools::SplitTwice2String(const string& strInput, const string& strSeparator, const string& strSeparator2)
{
    map<string, string> mapResult;

    vector<string> vec = tools::CStringTools::Split2String(strInput, strSeparator);
    size_t maxVec = vec.size();
    for (size_t iIndex = 0; iIndex < maxVec; iIndex++)
    {
        vector<string> vecTmp = tools::CStringTools::Split2String(vec[iIndex], strSeparator2);
        if (2 == vecTmp.size())
        {
            mapResult.insert(make_pair<string, string>(vecTmp[0], vecTmp[1]));
        }
        else if (1 == vecTmp.size())
        {
            mapResult.insert(make_pair<string, string>(vecTmp[0], ""));
        }
        else
        {
            continue;
        }
    }
    return mapResult;
}

map<unsigned int, unsigned int> tools::CStringTools::SplitTwice2Uint(const string& strInput, const string& strSeparator, const string& strSeparator2)
{
    map<unsigned int, unsigned int> mapResult;

    vector<string> vec = tools::CStringTools::Split2String(strInput, strSeparator);
    size_t maxVec = vec.size();
    for (size_t iIndex = 0; iIndex < maxVec; iIndex++)
    {
        vector<string> vecTmp = tools::CStringTools::Split2String(vec[iIndex], strSeparator2);
        if (2 == vecTmp.size())
        {
            mapResult.insert(make_pair<unsigned int, unsigned int>(::atoi(vecTmp[0].c_str()), ::atoi(vecTmp[1].c_str())));
        }
        else if (1 == vecTmp.size())
        {
            mapResult.insert(make_pair<unsigned int, unsigned int>(::atoi(vecTmp[0].c_str()), 0));
        }
        else
        {
            continue;
        }
    }
    return mapResult;
}

void tools::CStringTools::Show(const vector<unsigned int>& vecInput)
{
    size_t maxSize = vecInput.size();
    for (size_t index = 0; index < maxSize; index++)
    {
        DEBUG(tools::CStringTools::Int2String(vecInput[index]));
    }
}

void tools::CStringTools::Show(const map<string, string>& mapInput)
{
    map<string, string>::const_iterator iter = mapInput.begin();
    const map<string, string>::const_iterator iterEnd = mapInput.end();
    for (; iter != iterEnd; ++iter)
    {
        DEBUG(iter->first + " : " + iter->second);
    }
}

string tools::CStringTools::Trim(const string& strInput)
{
    string strTemp = strInput;
    size_t begin = 0;
    begin = strTemp.find(" ", begin);//查找空格在str中第一次出现的位置
    while(string::npos != begin)//表示字符串中存在空格
    {
        strTemp.replace(begin, 1, "");//用空串替换str中从begin开始的1个字符
        begin = strTemp.find(" ",begin);//查找空格在替换后的str中第一次出现的位置
    }
    return strTemp;
}

static inline char fromHex(const char &ch)
{
    return isdigit(ch) ? ch-'0' : ch-'A'+10;
}

std::string tools::CStringTools::UrlDecode(const std::string& strUrl)
{
    string strResult;
    for(size_t index = 0; index < strUrl.size(); index++)
    {
        char ch = 0;
        if(strUrl[index] == '%')
        {
            ch = (fromHex(strUrl[index+1])<<4);
            ch |= fromHex(strUrl[index+2]);
            index += 2;
        }
        else if(strUrl[index] == '+')
        {
            ch = ' ';
        }
        else
        {
            ch = strUrl[index];
        }
        strResult += (char)ch;
    }
    return strResult;
}

/**Created by Zhang Shibo at 2014-11-3 20:19:00
  *@brief: Whether the request coming from the host of us.
  *@param: strUrl: the request url.
  *@return: return true when it comes from host of us
  */
bool tools::CStringTools::IsHostName(const string& strUrl)
{
    const string strHostName = "huishoubao.com.cn";
    size_t last = 0;
    size_t uBeginPos = strUrl.find_first_of(".", last);
    if (string::npos == uBeginPos)
    {
        return false;
    }

    last = uBeginPos;
    size_t uEndPos = strUrl.find_first_of("/", last);
    if (string::npos == uEndPos)
    {
        return false;
    }

    string strResult = strUrl.substr(uBeginPos + 1, uEndPos - uBeginPos - 1);
    DEBUG("strResult = " + strResult);

    if (strHostName != strResult)
    {
        return false;
    }
    return true;
}

//added by shuting at 2014-11-14 11:25
string tools::CStringTools:: StrReplace(const string &strInput, const string &strSrc, const string &strDec)
{
    string strtmp = strInput;
    string::size_type fpos = 0; 
    fpos = strtmp.find(strSrc, fpos);
    while( strtmp.npos !=  fpos)
    {
        strtmp.replace( fpos, strSrc.size(), strDec );
        fpos = strtmp.find( strSrc, fpos+1 );
    }
    return strtmp;
}
//end modify by shuting

