#include <stdio.h>

#include "glog.h"
#include "statistic.h"
#include "log.h"
#include "octets.h"
#include "conf.h"
#include "thread.h"
#include "logclientclient.hpp"
#include "remotelog.hpp"

using namespace GNET;
namespace GLog
{
	std::string g_hostname;
	std::string g_progname;

	void init(int lineid)
	{
		Log::setprogname( "gamed" );
	}

	void log( int __priority, const char * __fmt, ... )
	{
		va_list ap;
		va_start(ap,__fmt);

		if( __priority <= LOG_NOTICE )
			Log::vlogvital( __priority, __fmt, ap );
		else
			Log::vlog( __priority, __fmt, ap );

		va_end(ap);
	}

	void logvital( int __priority, const char * __fmt, ... )
	{
		va_list ap;
		va_start(ap,__fmt);

		Log::vlogvital( __priority, __fmt, ap );

		va_end(ap);
	}

	void trace( const char * __fmt, ... )
	{
		va_list ap;
		va_start(ap,__fmt);

		Log::vlog( LOG_DEBUG, __fmt, ap );

		va_end(ap);
	}

	void tasklog( int roleid, int taskid, int type, const char * msg )
	{
		GLog::log( LOG_INFO, "task:roleid=%d:taskid=%d:type=%d:msg=%s",
						roleid, taskid, type, msg );
	}

	void formatlog( const char * title, const char * msg )
	{
		Log::formatlog( title, msg );
	}

	void task( int roleid, int taskid, int type, const char * msg )
	{
		GLog::log( LOG_NOTICE, "formatlog:task:roleid=%d:taskid=%d:type=%d:msg=%s",
							roleid, taskid, type, msg );
	}

	void upgrade( int roleid, int level, int money )
	{
		Log::upgrade( roleid, level, money );
	}

	void die( int roleid, int type, int attacker )
	{
		Log::die( roleid, type, attacker );
	}

	void keyobject( int id, int delta )
	{
		Log::keyobject( id, delta );
	}

	void refine( int roleid, int item, int level, int result, int cost )
	{
		GLog::log( LOG_NOTICE, "formatlog:refine:roleid=%d:item=%d:level=%d:result=%d:cost=%d",
				roleid, item, level, result, cost);
	}

	void cash( const char * __fmt, ... )
	{
		va_list ap;
		va_start(ap,__fmt);

		Log::vlog( LOG_CASH, __fmt, ap );

		va_end(ap);
	}

	void formatlog(const char * __fmt, ...)
	{
		va_list ap;
		va_start(ap,__fmt);
		Log::vlogvital( LOG_NOTICE, __fmt, ap );
		va_end(ap);
	}

}



