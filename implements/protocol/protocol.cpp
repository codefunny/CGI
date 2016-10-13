#include "protocol.h"
#include <sstream>
#include <json/json.h>
#include "configure.h"
#include "Markup.h"

using namespace std;
using namespace protocol;

static const unsigned int JSON_PROTOCOL = 1;
static const unsigned int XML_PROTOCOL = 2;
static const unsigned int PROTOCOL_TYPE = JSON_PROTOCOL;

bool protocol::UnPacking(const string& strRequest, const string& strKey, string& strValue)
{
    switch (PROTOCOL_TYPE)
    {
        case JSON_PROTOCOL:
        {
            return UnPackingByJson(strRequest, strKey, strValue);
        }
        case XML_PROTOCOL:
        {
            return UnPackingByXML(strRequest, strKey, strValue);
        }
        default:
        {
            ERROR("invalid protocol");
            return false;
        }
    }
    ERROR("invalid protocol");
    return false;
}

bool protocol::UnPackingByJson(const string& strRequest, const string& strKey, string& strValue)
{
    Json::Reader reader;
    Json::Value jsonInput;
    if (!reader.parse(strRequest, jsonInput))
    {
        ERROR("protocol unmatch[" + strRequest + "]");
        return false;
    }
    if (!jsonInput[strKey].isString())
    {
        ERROR("protocol unmatch, request[" + strRequest + "], key[" + strKey + "]");
        return false;
    }

    strValue = jsonInput[strKey].asString();
    return true;
}

/**
  *Create by Zhang Shibo at 2014-11-13 10:36:00
  *brief: parse int value from Json string.
  */
bool protocol::UnPackingByJson(const string& strRequest, const string& strKey, int& iValue)
{
    Json::Reader reader;
    Json::Value jsonInput;
    if (!reader.parse(strRequest, jsonInput))
    {
        ERROR("protocol unmatch[" + strRequest + "]");
        return false;
    }
    if (!jsonInput[strKey].isInt())
    {
        ERROR("protocol unmatch, request[" + strRequest + "], key[" + strKey + "]");
        return false;
    }

    iValue = jsonInput[strKey].asInt();
    return true;
}


bool protocol::UnPackingByXML(const string& strRequest, const string& strKey, string& strValue)
{
    CMarkup xml;
    xml.SetDoc(strRequest);

    if (!xml.FindChildElem(strKey))
    {
        ERROR("protocol unmatch, request[" + strRequest + "], key[" + strKey + "]");
        return false;
    }
    strValue = xml.GetChildData();
    return true;
}

string protocol::Packing(const map<string, string>& mapRequest)
{
    switch (PROTOCOL_TYPE)
    {
        case JSON_PROTOCOL:
        {
            return PackingByJson(mapRequest);
        }
        case XML_PROTOCOL:
        {
            return PackingByXML(mapRequest);
        }
        default:
        {
            ERROR("invalid protocol");
            return "";
        }
    }
    ERROR("invalid protocol");
    return "";
}

string protocol::PackingByJson(const map<string, string>& mapRequest)
{
    map<string, string>::const_iterator iter = mapRequest.begin();
    const map<string, string>::const_iterator iterEnd = mapRequest.end();
    Json::Value jsonOutput;
    for (; iter != iterEnd; ++iter)
    {
        jsonOutput[iter->first] = iter->second;
    }
    Json::FastWriter fast_writer;
    stringstream ssResult;
    ssResult << fast_writer.write(jsonOutput);
    return ssResult.str();
}

string protocol::PackingByXML(const map<string, string>& mapRequest)
{
    CMarkup xmlOutput;

    xmlOutput.AddElem("xml");
    xmlOutput.IntoElem();

    map<string, string>::const_iterator iter = mapRequest.begin();
    const map<string, string>::const_iterator iterEnd = mapRequest.end();
    for (; iter != iterEnd; ++iter)
    {
        xmlOutput.AddElem(iter->first, iter->second);
    }

    xmlOutput.OutOfElem();

    return xmlOutput.GetDoc();
}

