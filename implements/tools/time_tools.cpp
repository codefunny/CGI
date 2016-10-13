#include "time_tools.h"
#include <string>
#include <sstream>
#include <time.h>

using namespace std;

string tools::CTimeTools::GetTime()
{
    stringstream ssTime;
    ssTime << ::time(NULL);
    return ssTime.str();
}
