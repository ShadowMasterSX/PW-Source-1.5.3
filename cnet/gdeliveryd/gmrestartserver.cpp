#include "gmrestartserver.hpp"
namespace GNET
{
	Thread::Mutex	GMRestartServer::GMRestartTask::locker("GMRestartServer::GMRestartTask::locker");
	GMRestartServer::GMRestartTask GMRestartServer::GMRestartTask::instance;
}
