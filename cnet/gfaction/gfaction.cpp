#include "gfactionserver.hpp"
#include "gproviderserver.hpp"
#include "gfactiondbclient.hpp"
#include "conf.h"
#include "log.h"
#include "thread.h"
#include <iostream>
#include <unistd.h>
#include "matcher.h"
#include "itimer.h"
#include "uniquenameclient.hpp"
#include "factiondb.h"

using namespace GNET;
int main(int argc, char *argv[])
{
	if (argc != 2 || access(argv[1], R_OK) == -1)
	{
		printf("Compiled By "__USER__ ", "__DATE__ " "__TIME__ "\n");
		std::cerr << "Usage: " << argv[0] << " configurefile" << std::endl;
		exit(-1);
	}

	Conf *conf = Conf::GetInstance(argv[1]);
	IntervalTimer::StartTimer(500000);

	bool is_central_faction = false;
	Log::setprogname("gfaction");
	{
		GFactionServer *manager = GFactionServer::GetInstance();
		manager->SetAccumulate(atoi(conf->find(manager->Identification(), "accumulate").c_str()));
		Protocol::Server(manager);
		std::string central_faction = conf->find(manager->Identification(), "is_central_faction");
		if(central_faction == "true") manager->SetCentralFaction(true);
		is_central_faction = manager->IsCentralFaction();

		if(Matcher::GetInstance()->Load(conf->find(manager->Identification(),"table_name").c_str(),
			"UCS2", conf->find(manager->Identification(),"name_charset").c_str(),
			conf->find(manager->Identification(), "table_charset").c_str()))
		{
			std::cerr<<"Cannot load table of sensitive words. check file ./filters."<<std::endl;
			Log::log(LOG_ERR,"Cannot load sensitive words, check %s", 
				conf->find(manager->Identification(),"table_name").c_str());
			exit(-1);
		}
	}

	{
		GProviderServer *manager = GProviderServer::GetInstance();
		//faction server的ID必须为101-200之间
		manager->SetProviderID(atoi(conf->find(manager->Identification(), "id").c_str()));
		if (manager->GetProviderID()<101 || manager->GetProviderID()>200) 
		{
			Log::log(LOG_ERR,"FactionProvider's id must be in range (101,200). Check .conf file.\n");
			exit(EXIT_FAILURE);
		}
		manager->SetAccumulate(atoi(conf->find(manager->Identification(), "accumulate").c_str()));
		Protocol::Server(manager);
	}
	{
		GFactionDBClient *manager = GFactionDBClient::GetInstance();
		manager->SetAccumulate(atoi(conf->find(manager->Identification(), "accumulate").c_str()));
		Protocol::Client(manager);
	}
	if(!is_central_faction)
	{
		UniqueNameClient* manager=UniqueNameClient::GetInstance();
		Protocol::Client(manager);
		Thread::HouseKeeper::AddTimerTask(new KeepAliveTask(30),30); 
	}

	Factiondb::GetInstance()->InitFactiondb();

	Thread::Pool::AddTask(PollIO::Task::GetInstance());
	Thread::Pool::Run();
	return 0;
}

