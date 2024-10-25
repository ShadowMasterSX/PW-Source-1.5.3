#include "glinkserver.hpp"
#include "gdeliveryclient.hpp"
#include "gproviderserver.hpp"
#include "gfactionclient.hpp"
#include "conf.h"
#include "log.h"
#include "thread.h"
#include <iostream>
#include <unistd.h>
#include <sys/resource.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include "xmlversion.h"

using namespace GNET;
void int_handler(int no)
{
	printf("Bye.\n");
	exit(0);
}

void usage(char* name)
{
	printf ("Usage: %s [-v] [-h] <configurefile> <section_num>\n", name);
}

int main(int argc, char *argv[])
{
	int opt;
	while((opt = getopt(argc, argv, "hv")) != EOF)
	{
		switch(opt)
		{
			case 'v':
				printf("Compiled By "__USER__ ", "__DATE__ " "__TIME__ "\n");
				printf("%s\n", XMLVERSION);
				exit(0);
			default:
				usage(argv[0]);
				exit(0);
		}
	}
	if (optind+1 >= argc || access(argv[optind], R_OK) == -1)
	{
		usage(argv[0]);
		exit(-1);
	}
	char* sect = argv[optind+1];
	Conf *conf = Conf::GetInstance(argv[optind]);
	signal(SIGINT, int_handler);
	char buf[256];
	snprintf(buf,256,"glinkd-%s",sect);
	Log::setprogname(buf);
	{
		GLinkServer *manager = GLinkServer::GetInstance();
		Timer::Attach(manager);	
		manager->SetSectionNum(sect);
		manager->SetAccumulate(atoi(conf->find(manager->Identification(), "accumulate").c_str()));
		IntervalTimer::StartTimer(100000);
		manager->SetCompress(atoi(conf->find(manager->Identification(), "compress").c_str()));		
		manager->SetTimerSenderSize(atoi(conf->find(manager->Identification(), "accumu_packets").c_str()));
		manager->RunTimerSender(atoi(conf->find(manager->Identification(), "sender_interval").c_str())/100000 );

		manager->SetUserCountLimit(atoi(conf->find(manager->Identification(), "max_users").c_str()));
		manager->SetHalfLoginLimit(atoi(conf->find(manager->Identification(), "halflogin_users").c_str()));
		manager->version=strtol(conf->find(manager->Identification(), "version").c_str(),(char**)NULL,16);
		manager->StartListen();
	}
	{
		GDeliveryClient *manager = GDeliveryClient::GetInstance();
		char link_type = (char)atoi(conf->find(GLinkServer::GetInstance()->Identification(), "link_type").c_str());
		if (LINK_TYPE_PC != link_type && LINK_TYPE_PHONE != link_type)
			link_type = LINK_TYPE_PC;
		manager->SetLinkType(link_type);
		manager->SetAccumulate(atoi(conf->find(manager->Identification(), "accumulate").c_str()));
		Protocol::Client(manager);
	}
	{
		GFactionClient *manager = GFactionClient::GetInstance();
		manager->SetAccumulate(atoi(conf->find(manager->Identification(), "accumulate").c_str()));
		Protocol::Client(manager);
	}
	{
		std::string value;
		GProviderServer *manager = GProviderServer::GetInstance();
		manager->SetSectionNum(sect);
		manager->SetAccumulate(atoi(conf->find(manager->Identification(), "accumulate").c_str()));
		manager->SetProviderServerID( atoi(sect) );
		Protocol::Server(manager);
	}
	Thread::Pool::AddTask(new CheckTimer(15));
	Thread::Pool::AddTask(PollIO::Task::GetInstance());
	Thread::Pool::Run();
	return 0;
}

