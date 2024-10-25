#ifndef  __GNET_TIMERTASK
#define  __GNET_TIMERTASK

#include "protocol.h"
namespace GNET
{
class ReconnectTask: public Thread::Runnable
{
public:
    Protocol::Manager* manager;

    ReconnectTask(Protocol::Manager* m,int priority): Runnable(priority),manager(m) {}
    void Run()
    {
        Protocol::Client(manager);  //reconnect
        delete this;
    }
};
}
#endif
