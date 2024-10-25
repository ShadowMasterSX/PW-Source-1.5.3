#include "logclientclient.hpp"
#include "conf.h"
#include "log.h"
#include "thread.h"
#include <iostream>
#include <unistd.h>

#include "remotelog.hpp"

using namespace GNET;

class TestLogTask : public Thread::Runnable
{
public:
	TestLogTask( ) : Thread::Runnable()
	{
	}

	virtual void Run( )
	{
		sleep( 2 );

		printf( "TestLogTask Run\n" );

		Benchmark bm;

		for( int i=0; i<1; i++ )
		{
			bm.tickbegin();
			Log::log(LOG_DEBUG, "DEBUG   hello asdfasdfdhello asdfasdfdhello asdfasdfd");
			Log::log(LOG_INFO, "INFO   hello asdfasdfdhello asdfasdfdhello asdfasdfd");
			Log::log(LOG_NOTICE, "NOTICE    hello asdfasdfdhello asdfasdfdhello asdfasdfdhello asdfasdfd");
			Log::log(LOG_WARNING, "WARNING   hello asdfasdfdhello asdfasdfdhello asdfasdfd");
			Log::log(LOG_ERR, "ERR hello   hello asdfasdfdhello asdfasdfdasdfasdfd");
			bm.tickend();
		}

		bm.outputtickstat( std::cout );
		bm.outputusage( std::cout );

		delete this;
	}
};

int main(int argc, char *argv[])
{
	if (argc != 2 || access(argv[1], R_OK) == -1)
	{
		std::cerr << "Usage: " << argv[0] << " configurefile" << std::endl;
		exit(-1);
	}

	Conf::GetInstance(argv[1]);
	Log::setprogname("logclient");

	Thread::Pool::AddTask(new TestLogTask());

	Thread::Pool::AddTask(PollIO::Task::GetInstance());
	Thread::Pool::Run();
	return 0;
}

