#ifndef __GNET_PROTOCOL_EXECUTOR_H
#define __GNET_PROTOCOL_EXECUTOR_H
#include "thread.h"
#include "protocol.h"
namespace GNET
{
	
class ProtocolExecutor : public Thread::Runnable
{
	Protocol::Manager*	manager;
	unsigned int 		sid;
	Protocol*			p;
public:
	ProtocolExecutor(Protocol::Manager* m,unsigned int _sid,Protocol* _p,int prior=1) : Runnable(prior),manager(m),sid(_sid),p(_p) { }
	void Run()
	{
		p->Process(manager,sid);
		p->Destroy();
		delete this;
	}
};

}
#endif
