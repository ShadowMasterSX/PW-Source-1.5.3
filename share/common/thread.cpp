
#include <string.h>
#include <unistd.h>

#include "thread.h"
#include "conf.h"
#include "../io/pollio.h"
#include "itimer.h"
#include "conv_charset.h"

using namespace std;
using namespace GNET;

int __gnfailed=0;
int __gnRDfailed=0;
int __gnWRfailed=0;
namespace GNET
{

#if defined _REENTRANT_

Thread::Mutex			Thread::Pool::s_mutex_tasks("Thread::Pool::s_mutex_tasks");

Thread::Mutex			Thread::Pool::s_mutex_threads("Thread::Pool::s_mutex_threads");
Thread::Condition		Thread::Pool::s_cond_threads;
size_t					Thread::Pool::s_thread_count = 0;
Thread::Pool::CountMap	Thread::Pool::s_thread_counts;
bool					Thread::Pool::s_prior_strict = false;
#endif

Thread::Pool::Policy	Thread::Pool::Policy::s_policy;

Thread::Pool::TaskQueue	Thread::Pool::s_tasks;
Thread::Pool::Policy*	Thread::Pool::s_ppolicy = &(Thread::Pool::Policy::s_policy);

Thread::Mutex			CharsetConverter::locker_iconv;

void Thread::Pool::Policy::LoadConfig( )
{
	Conf *conf = Conf::GetInstance( );
	Conf::section_type section = Identification();
	size_t size;

#if defined _REENTRANT_
	m_thread_count = 0;
	m_thread_counts.clear();

	std::string threads = conf->find(section, "threads");
	if( threads.length() > 1023 )
	{
		Log::log( LOG_ERR, "threads is too long in the configuration file. use default." );
		threads = "(0,1)(1,2)(2,1)(3,1)";
	}
	else if( threads.length() == 0 )
	{
		Log::log( LOG_ERR, "threads is not gived in the configuration file. use default." );
		threads = "(0,1)(1,2)(2,1)(3,1)";
	}

	char buffer[1024];
	strncpy( buffer, threads.c_str(), std::min(sizeof(buffer)-1,threads.length()) );
	buffer[sizeof(buffer)-1] = 0;

	char * cur = buffer;
	char * token = strchr( cur, '(' );
	while( NULL != token )
	{
		cur = token+1;
		token = strchr( cur, ',' );
		if( NULL == token )	break;
		*token = 0;
		int prior = atol(cur);

		cur = token+1;
		token = strchr( cur, ')' );
		if( NULL == token )	break;
		*token = 0;
		size_t size = atol(cur);

		m_thread_counts[prior] = size;
		m_thread_count += size;

		cur = token+1;
		token = strchr( cur, '(' );
	}

	if( m_thread_counts[1] < 2 )
	{
		Log::log( LOG_WARNING, "threads of prior 1 is less than 2, not enough." );
	}

	s_prior_strict = atoi(conf->find(section, "prior_strict").c_str());
	if( s_prior_strict )
	{
		if( m_thread_counts[100] < 1 )
		{
			Log::log( LOG_WARNING, "no thread of prior 100." );
		}
	}
#endif

	m_max_queuesize = 1048576;
	if ((size = atoi(conf->find(section, "max_queuesize").c_str())))
		m_max_queuesize = size;

}

void Thread::Pool::sigusr1_handler( int signum )
{
	if( SIGUSR1 == signum )
	{
		Log::log( LOG_INFO, "SIGUSR1 received, program will quit." );
		
		s_ppolicy->OnQuit();
		s_ppolicy->SetState( stateQuitAtOnce );
		
		PollIO::WakeUp();
	}
}

#if defined _REENTRANT_
void sighandler_null( int signum )
{
}

void Thread::Pool::Run( Policy * policy ,bool wantreturn)
{
	{
		Mutex::Scoped lock(s_mutex_threads);

		s_ppolicy = policy;
		s_thread_count = 0;
		s_ppolicy->LoadConfig();

		pthread_t th;
		CountMap & counts = s_ppolicy->GetThreadCountMap();
		for( CountMap::const_iterator it = counts.begin(), ie = counts.end(); it != ie; ++it )
		{
			int prior = (*it).first;
			size_t size = (*it).second;
			for( size_t k=0; k<size; k++ )
				pthread_create( &th, NULL, &Pool::RunThread, (void*)prior );

			if( size > 0 )
			{
				s_thread_counts[prior] = size;
				s_thread_count += size;
			}
		}
	}

	//Log::log( LOG_INFO, "program started with %u threads.", s_thread_count );
	if( wantreturn )
		return;

	struct sigaction act;
	memset( &act, 0, sizeof(act) );
	act.sa_handler = sighandler_null;
	sigemptyset(&act.sa_mask);
	sigaddset(&act.sa_mask, SIGALRM);
	sigaddset(&act.sa_mask, SIGUSR1);
	sigaddset(&act.sa_mask, SIGUSR2);
	sigaddset(&act.sa_mask, SIGHUP);
	sigaction( SIGALRM, &act, NULL );
	sigaction( SIGUSR1, &act, NULL );
	sigaction( SIGUSR2, &act, NULL );
	sigaction( SIGHUP, &act, NULL );

	int val;
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGALRM);
	sigaddset(&set, SIGUSR1);
	sigaddset(&set, SIGUSR2);
	sigaddset(&set, SIGHUP);
	pthread_sigmask(SIG_BLOCK, &set, NULL );

	int update_time = time(NULL);
	int resolution = IntervalTimer::Resolution();
	struct itimerval value;
	value.it_interval.tv_sec = resolution/1000000;
	value.it_interval.tv_usec = resolution%1000000;
	value.it_value.tv_sec = resolution/1000000;
	value.it_value.tv_usec = resolution%1000000;
	setitimer(ITIMER_REAL, &value, NULL);

	for( ;; )
	{
		sigwait(&set, &val);
		if( SIGALRM == val )
		{
			IntervalTimer::Update();
			int now = time(NULL);
			if (now > update_time)
			{
				Timer::Update();
				update_time = now;
			}
			if( 0 == Size() )
			{
				usleep(5000);
				exit(0);
			}
		}
		else if( SIGUSR1 == val )
			sigusr1_handler( SIGUSR1 );
		else if( SIGUSR2 == val )
			s_ppolicy->OnSIGUSR2( );
		else if( SIGHUP == val )
			s_ppolicy->OnSIGHUP( );
	}
}
#endif
}


