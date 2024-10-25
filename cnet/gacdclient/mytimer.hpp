#ifndef __GNET_MYTIMER_HPP
#define __GNET_MYTIMER_HPP

#include "thread.h"

namespace GNET
{

class MyTimer : public Thread::Runnable {
	static MyTimer instance;
	MyTimer();
public:
    void Run();
};

};

#endif

