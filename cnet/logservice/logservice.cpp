#include <iostream>
#include <unistd.h>
#include <strings.h>

#include "conf.h"
#include "log.h"
#include "thread.h"
#include "logdispatch.h"
#include "logserviceserver.hpp"
#include "logservicetcpserver.hpp"

using namespace GNET;

static std::string conf_filename;

void set_logthreshhold()
{
	int threshhold = LOG_INFO;

	Conf * conf = Conf::GetInstance();
	std::string hold = conf->find("logservice","threshhold");
	if( 0 == strcasecmp(hold.c_str(),"LOG_DEBUG") )
		threshhold = LOG_DEBUG;
	else if( 0 == strcasecmp(hold.c_str(),"LOG_INFO") )
		threshhold = LOG_INFO;
	else if( 0 == strcasecmp(hold.c_str(),"LOG_NOTICE") )
		threshhold = LOG_NOTICE;
	else if( 0 == strcasecmp(hold.c_str(),"LOG_WARNING") )
		threshhold = LOG_WARNING;
	else if( 0 == strcasecmp(hold.c_str(),"LOG_WARN") )
		threshhold = LOG_WARNING;
	else if( 0 == strcasecmp(hold.c_str(),"LOG_ERR") )
		threshhold = LOG_ERR;
	else if( 0 == strcasecmp(hold.c_str(),"LOG_ERROR") )
		threshhold = LOG_ERR;
	else if( 0 == strcasecmp(hold.c_str(),"LOG_CRIT") )
		threshhold = LOG_CRIT;
	else if( 0 == strcasecmp(hold.c_str(),"LOG_ALERT") )
		threshhold = LOG_ALERT;
	else if( 0 == strcasecmp(hold.c_str(),"LOG_EMERG") )
		threshhold = LOG_EMERG;

	LogDispatch::set_threshhold( threshhold );
}

class LogPolicy : public Thread::Pool::Policy
{
public:
	virtual void OnSIGHUP( )
	{
		Log::log( LOG_INFO, "SIGHUP received, reopenfiles." );
		Conf::GetInstance(conf_filename.c_str());
		set_logthreshhold();
		LogDispatch::closefiles();
		LogDispatch::openfiles();
	}

};

static LogPolicy	s_policy;

int main(int argc, char *argv[])
{
	if (argc != 2 || access(argv[1], R_OK) == -1)
	{
		std::cerr << "Usage: " << argv[0] << " configurefile" << std::endl;
		exit(-1);
	}

	Log::setprogname( "logservice" );

	conf_filename = argv[1];
	Conf *conf = Conf::GetInstance(argv[1]);
	set_logthreshhold();
	LogDispatch::openfiles();
	{
		LogserviceServer *manager = LogserviceServer::GetInstance();
		manager->SetAccumulate(atoi(conf->find(manager->Identification(), "accumulate").c_str()));
		Protocol::Server(manager);
	}
	{
		LogserviceTcpServer *manager = LogserviceTcpServer::GetInstance();
		manager->SetAccumulate(atoi(conf->find(manager->Identification(), "accumulate").c_str()));
		Protocol::Server(manager);
	}


	Thread::Pool::AddTask(PollIO::Task::GetInstance());
	Thread::Pool::Run(&s_policy);
	return 0;
}

