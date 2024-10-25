#include "glinkclient.hpp"
#include "conf.h"
#include "log.h"
#include "thread.h"
#include <iostream>
#include <unistd.h>
#include "itimer.h"
#include "timersender.h"
#include "vclient_if.h"
using namespace GNET;

class VcPolicy : public Thread::Pool::Policy
{
public:
	virtual void OnQuit( )
	{
		VCLIENT::Quit();
	}
};
static VcPolicy	s_policy;

int main(int argc, char *argv[])
{
	Conf *conf = NULL;

	if(argc != 2 || access(argv[1], R_OK) == -1)
	{
	 	conf = Conf::GetInstance("gclient.conf"); // try defalut
	}
	else
	{
		conf = Conf::GetInstance(argv[1]);
	}

	if(NULL == conf)
	{
		std::cerr << "Usage: " << argv[0] << " configurefile " << std::endl;
		exit(-1);
	}

	srand(time(NULL));
	Log::setprogname("gclient");

	std::string prefix = conf->find("Account","prefix");
	int start = atoi(conf->find("Account","start").c_str());
	int end = atoi(conf->find("Account","end").c_str());
	std::string password = conf->find("Account","password");
	int role = atoi(conf->find("Account","role").c_str());

	for(int i=start; i<end; i++)
	{
		GLinkClient * manager = new GLinkClient();
		char buf[20];
		memset(buf,0,sizeof(buf));
		sprintf(buf,"%d",i);
		std::string account = prefix + std::string(buf);
	
		manager->identity=Octets(account.c_str(),account.length());
		manager->origin_password=Octets(password.c_str(),password.length());
		manager->password=manager->origin_password;
		manager->role = role;
//		manager->SetAccumulate(atoi(conf->find(manager->Identification(), "accumulate").c_str()));
		Protocol::Client(manager);
//		manager->RunTimerSender();
		Thread::HouseKeeper::AddTimerTask(new KeepAliveTask(manager,15),15);
		sleep(0);
	}

	Thread::HouseKeeper::AddTimerTask(new ReportAliveTask(30),30);

	IntervalTimer::StartTimer();

	VCLIENT::Init();
	Thread::Pool::AddTask(PollIO::Task::GetInstance());
	Thread::Pool::Run(&s_policy);
	return 0;
}

