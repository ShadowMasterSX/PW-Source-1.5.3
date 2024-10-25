
#include <unistd.h>
#include <mcheck.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <iostream>

#include "thread.h"
#include "conf.h"
#include "log.h"
#include "octets.h"
#include "itimer.h"
#include "cache.h"
#include "hardware.h"

using std::cout;
using std::endl;
using namespace GNET;

struct get_func
{
	bool operator() (const int &__key, int &__value) const
	{
		__value = __key * 1000;
		printf( "get_func called. key=%d, value=%d\n", __key, __value );
		return true;
	}
};

struct put_func
{
	bool operator() (const int &__key, const int &__value) const
	{
		printf( "put_func called. key=%d, value=%d\n", __key, __value );
		return true;
	}
};

struct remove_func
{
	bool operator() (const int &__key) const
	{
		printf( "remove_func called. key=%d\n", __key );
		return true;
	}
};

struct onobtained_func
{
	bool operator() (const std::pair<int,int>& __ktp, const cache_stamp& __stamp) const
	{
		printf( "onobtained_func called. key=%d, value=%d, stamp=%d\n", __ktp.first, __ktp.second, __stamp );
		return true;
	}
};

typedef Cache<int, int, get_func, put_func, remove_func, onobtained_func>	RoleCache;
static RoleCache g_role;

class TestDumpTask : public Thread::Runnable
{
	int value;
public:
	TestDumpTask( int __value ) : Thread::Runnable(1+__value%3), value(__value)
	{
	}

	virtual void Run( )
	{
		{
			printf( "g_role dump(value=%d):\n", value );
			g_role.dump();
			printf( "\n\n" );
		}

		delete this;
	}
};


class TestNormalTask : public Thread::Runnable
{
	int value;
public:
	TestNormalTask( int __value ) : Thread::Runnable(1+__value%3), value(__value)
	{
	}

	virtual void Run( )
	{
		{
			bool success = true;
			int i;
			int all = 100;

			for( i=1; i<=all; i++ )
				success = g_role.put( std::make_pair(rand()%all,i), 0 ) && success;

			for( i=1; i<=all; i++ )
				success = g_role.get( rand()%all ) && success;

			for( i=1; i<=all; i++ )
			{
				int key = rand()%all;
				int v;
				cache_stamp s;
				bool temp = g_role.get(key,v,s);
				printf( "get directly. key=%d,return=%d,value=%d,stamp=%d\n", key, temp, v, s );

				success = g_role.put( std::make_pair(key,i), s ) && success;
			}

			//for( i=1; i<=all; i++ )
			//	success = g_role.remove( rand()%all ) && success;

			printf( "success = %d\n\n", success );
			Thread::Pool::AddTask( new TestDumpTask(value) );
			Thread::HouseKeeper::AddTimerTask( new TestDumpTask(value), 20 );

		}

		delete this;
	}
};

class TestStatTask : public Thread::Runnable
{
	int value;
public:
	TestStatTask( int __value ) : Thread::Runnable(1+__value%3), value(__value)
	{
	}

	virtual void Run( )
	{
		STAT_MIN5("TestMin5",1);
		STAT_HOUR("TestHour",1);
		STAT_DAY("TestDay",1);

		STAT_HOUR("acc-login",1);
		STAT_HOUR("acc-online",1500);
		STAT_HOUR("acc-online-payed",500);

		STAT_DAY("acc-login-first",20000);
		STAT_DAY("acc-login-ever",3000000);

		char buffer[64];
		for( int m=1; m<=100; m++ )
		{
			memset( buffer, 0, sizeof(buffer) );
			sprintf( buffer, "monster-live-monster%.3d", m );
			STAT_DAY(buffer,3000+rand()%500);

			int killed = rand()%25;
			memset( buffer, 0, sizeof(buffer) );
			sprintf( buffer, "monster-killed-monster%.3d", m );
			STAT_DAY(buffer,killed);

			int money = killed * (100+rand()%100);
			memset( buffer, 0, sizeof(buffer) );
			sprintf( buffer, "monster-money-monster%.3d", m );
			STAT_DAY(buffer,money);
			STAT_DAY("monster-money-sum",money);

			int drop = killed * (rand()%2);
			memset( buffer, 0, sizeof(buffer) );
			sprintf( buffer, "monster-drop-monster%.3d", m );
			STAT_DAY(buffer,drop);
			STAT_DAY("monster-drop-sum",drop);
		}

		delete this;
	}
};

class TestAddtaskTask : public Thread::Runnable
{
	size_t m_task_count;

public:
	TestAddtaskTask( size_t task_count )
		 : Thread::Runnable(2), m_task_count(task_count)
	{ }

	virtual void Run( )
	{
		for( size_t i=1; i<=m_task_count; i++ )
		{
			Thread::Pool::AddTask( new TestStatTask(i) );

			sleep( 5 );
		}

		delete this;
	}
};

int main(int argc, char ** argv)
{
	setenv("MALLOC_TRACE", "./m.log", 1);
	mtrace();

	Log::formatlog( "title", "abcd" );
	Log::log( LOG_INFO, "abcd" );

	srand(time(NULL));
	Conf::GetInstance("io.conf");
	Log::setprogname( "thread" );
	IntervalTimer::StartTimer();

	const GNET::IFConfig *ifconfig = GNET::IFConfig::GetInstance();
	std::map< const char *, struct in_addr > map = ifconfig->if2ipv4();
	printf("if_count = %zd\n", map.size() );
	for ( std::map<const char *, struct in_addr>::iterator it = map.begin(), ie = map.end(); it != ie; ++it )
		printf("%s:\t%s\n", (*it).first, inet_ntoa((*it).second) );

	g_role.init( 200, 7, 10 );
	g_role.initfunc(get_func(), put_func(), remove_func(), onobtained_func());

	size_t task_count = 100;
	if( argc == 2 )
		task_count = atol(argv[1]);
	Thread::Pool::AddTask( new TestAddtaskTask(task_count) );
	Thread::Pool::Run( );

	muntrace();
	return EXIT_SUCCESS;
}



