
#include "timer.h"

namespace GNET
{
time_t Timer::now = time(NULL);
struct timeval Timer::now_tv;
};
