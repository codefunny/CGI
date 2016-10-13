#include "memcached.h"
#include <stdlib.h>
#include <string>
#include <time.h>
#include <libmemcached/memcached.h>

using namespace std;
using namespace tools;

//  /usr/local/bin/memcached -d -m 10 -u root -l 127.0.0.1 -p 12000 -c 1024 -P /tmp/memcached.pid

CMemcached::CMemcached()
{
    memcached_st *memc = memcached_create(NULL);
    mem = memc;

    memcached_return rc;
    memcached_server_st* servers = memcached_server_list_append(NULL, "10.66.116.155", 9101, &rc);
    memcached_server_push((memcached_st *)mem, servers);
    memcached_server_list_free(servers);

    //memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_BINARY_PROTOCOL, 0);

    //MEMCACHED_BEHAVIOR_DISTRIBUTION
    //Using this you can enable different means of distributing values to servers.
    //The default method is MEMCACHED_DISTRIBUTION_MODULA. You can enable consistent hashing by setting MEMCACHED_DISTRIBUTION_CONSISTENT.
    //Consistent hashing delivers better distribution and allows servers to be added to the cluster with minimal cache losses.
    //Currently MEMCACHED_DISTRIBUTION_CONSISTENT is an alias for the value MEMCACHED_DISTRIBUTION_CONSISTENT_KETAMA.
    //memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_DISTRIBUTION, MEMCACHED_DISTRIBUTION_CONSISTENT);

    //MEMCACHED_BEHAVIOR_RETRY_TIMEOUT
    //When enabled a host which is problematic will only be checked for usage based on the amount of time set by this behavior. The value is in seconds.
    //memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_RETRY_TIMEOUT, 20);

    //MEMCACHED_BEHAVIOR_REMOVE_FAILED_SERVERS
    //If enabled any hosts which have been flagged as disabled will be removed from the list of servers in the memcached_st structure.
    //memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_REMOVE_FAILED_SERVERS, 1);

    //MEMCACHED_BEHAVIOR_SERVER_FAILURE_LIMIT
    //Set this value to enable the server be removed after continuous MEMCACHED_BEHAVIOR_SERVER_FAILURE_LIMIT times connection failure.
    //memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_SERVER_FAILURE_LIMIT, 5);

    //If enabled any hosts which have been flagged as disabled will be removed from the list of servers in the memcached_st structure.
    //This must be used in combination with MEMCACHED_BEHAVIOR_SERVER_FAILURE_LIMIT.
    //memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_AUTO_EJECT_HOSTS, 1);
}

CMemcached::~CMemcached()
{
    memcached_free((memcached_st *)mem);
}

bool CMemcached::Get(const string& strKey, string& strValue)
{
    size_t value_length;
    memcached_return rc;
    uint32_t flags;
    char *result = memcached_get((memcached_st *)mem, strKey.c_str(), strKey.length(), &value_length, &flags, &rc);
    if (MEMCACHED_SUCCESS == rc)
    {
        strValue = string(result);
        free(result);//Any object returned by memcached_get() must be released by the caller application.
        //printf("CMEM Get OK\n");
        return true;
    }

    strValue = "";
    //printf("CMEM Get failed\n");
    return false;
}

bool CMemcached::Set(const string& strKey, const string& strValue)
{
    const static time_t expiration = 3600 * 24;
    memcached_return rc;
    uint32_t flags;
    rc = memcached_set((memcached_st *)mem, strKey.c_str(), strKey.length(), strValue.c_str(), strValue.length(), expiration, flags);
    if (MEMCACHED_SUCCESS == rc)
    {
        //printf("CMEM Set OK\n");
        return true;
    }

    //printf("CMEM Set failed\n");
    return false;
}

bool CMemcached::Delete(const string& strKey)
{
    time_t expiration = 0;
    memcached_return rc;
    rc = memcached_delete((memcached_st *)mem, strKey.c_str(), strKey.length(), expiration);
    if (MEMCACHED_SUCCESS == rc)
    {
        //printf("CMEM Delete OK\n");
        return true;
    }

    //printf("CMEM Delete failed\n");
    return false;
}

