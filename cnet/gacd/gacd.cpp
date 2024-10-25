#include <unistd.h>
#include <iostream>

#include "conf.h"
#include "log.h"
#include "thread.h"
#include "ganticheaterserver.hpp"
#include "gaccontrolserver.hpp"
#include "configmanager.hpp"
#include "timermanager.hpp"

using namespace GNET;

int main(int argc, char *argv[])
{
	if (argc != 2 || access(argv[1], R_OK) == -1)
	{
		std::cerr << "Usage: " << argv[0] << " configurefile" << std::endl;
		exit(-1);
	}

	//TimerManager::GetInstance()->Run();

	Conf *conf = Conf::GetInstance(argv[1]);
	Log::setprogname("gacd");
	{
		GAntiCheaterServer *manager = GAntiCheaterServer::GetInstance();
		manager->SetAccumulate(atoi(conf->find(manager->Identification(), "accumulate").c_str()));
		Protocol::Server(manager);
	}
	{
		GACControlServer *manager = GACControlServer::GetInstance();
		manager->SetAccumulate(atoi(conf->find(manager->Identification(), "accumulate").c_str()));
		Protocol::Server(manager);
	}

	ConfigManager::GetInstance()->Init("gacd.xml");

	Thread::Pool::AddTask(PollIO::Task::GetInstance());
	Thread::Pool::Run();
	return 0;
}

