#ifndef __GNET_TIMERMANAGER_HPP
#define __GNET_TIMERMANAGER_HPP

#include "thread.h"

namespace GNET
{

class TimerManager : public Thread::Runnable 
{
	static TimerManager s_instance;
	TimerManager();
public:
    void Run();
	static TimerManager *GetInstance(){return &s_instance;};
};

};

#endif

