#include "crypto.h"
#include <stdio.h>
#include <openssl/md5.h>
#include <sstream>

using namespace std;

const string tools::CMD5::raw_digest(const string& strInput)
{
    return string(reinterpret_cast<const char*>(MD5((const unsigned char *)strInput.c_str(), strInput.length(), NULL)));
}

const string tools::CMD5::hex_digest(const string& strInput)
{
    unsigned char *pOutput = MD5((const unsigned char *)(strInput.c_str()), strInput.length(), NULL);

    char cResult[33] = {'\0'};
    for (unsigned i = 0; i < 16; i++)
        sprintf(cResult + i * 2, "%02x", pOutput[i]);
    cResult[32] = '\0';
    return string(cResult);
}

const string tools::CTime33::Encrypt(const string& strInput)
{
    /*
    unsigned long hash = 5381;
    size_t size = strInput.size();
    for (size_t index = 0; index < size; index++)
    {
        hash = hash * 33 + (unsigned long)strInput[index];
    }
    return tools::CStringTools::Int2String(hash);
    */

    //unsigned long hash = 5381;
    unsigned int hash = 5381;
    size_t size = strInput.size();
    for (size_t index = 0; index < size; index++)
    {
        hash += (hash << 5) + strInput[index];
    }

    hash = hash & 0x7fffffff;

    stringstream sshash;
    sshash << hash;
    return sshash.str();
}



