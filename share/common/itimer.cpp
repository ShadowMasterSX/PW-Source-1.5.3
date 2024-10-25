#include "itimer.h"

namespace GNET
{
	bool   IntervalTimer::stop = true;
	bool   IntervalTimer::triggered = true;
	size_t IntervalTimer::interval = DEFAULT_INTERVAL;
	Thread::Mutex IntervalTimer::locker("IntervalTimer::locker",true);
	struct timeval IntervalTimer::now;
	struct timeval IntervalTimer::base;
	int64_t IntervalTimer::tick_now = 0;
	sigset_t IntervalTimer::mask;
	IntervalTimer IntervalTimer::instance;
	IntervalTimer::Observers IntervalTimer::observers;
};
