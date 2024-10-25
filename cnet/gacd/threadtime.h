#ifndef __GNET_THREADTIME
#define __GNET_THREADTIME

namespace GNET
{

struct ThreadTime
{
	int tid;
	int all_secs;
	int kernel_secs;
	int user_secs;

	ThreadTime() : tid(0), all_secs(0), kernel_secs(0), user_secs(0) { }
	ThreadTime(int _tid, int _all_secs, int _kernel_secs, int _user_secs )
		: tid(_tid), all_secs(_all_secs), kernel_secs(_kernel_secs), user_secs(_user_secs) { }
};

};

#endif
