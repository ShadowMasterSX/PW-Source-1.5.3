#include <stdio.h>

#include "statistic.h"
#include "log.h"
#include "octets.h"
#include "conf.h"
#include "thread.h"
#include "logclientclient.hpp"
#include "logclienttcpclient.hpp"
#include "statinfovital.hpp"
#include "statinfo.hpp"
#include "remotelogvital.hpp"
#include "remotelog.hpp"

namespace GNET
{
	std::string g_hostname;
	std::string g_progname;

	void Log::setprogname( const char * __name )
	{
		instance().m_progname = __name;
		instance().LogFacility = LOG_LOCAL0;

		openlog( __name, LOG_CONS | LOG_PID, LOG_LOCAL0 );

		char buffer[256];
		memset( buffer, 0, sizeof(buffer) );
		gethostname( buffer, sizeof(buffer)-1 );
		g_hostname = buffer;
		g_progname = __name;

		Conf *conf = Conf::GetInstance();
		{
			LogclientClient *manager = LogclientClient::GetInstance();
			manager->SetAccumulate(atoi(conf->find(manager->Identification(), "accumulate").c_str()));
			Protocol::Client(manager);
		}
		{
			LogclientTcpClient *manager = LogclientTcpClient::GetInstance();
			manager->SetAccumulate(atoi(conf->find(manager->Identification(), "accumulate").c_str()));
			Protocol::Client(manager);
		}
	}

	void Log::vlog( int __priority, const char * __fmt, va_list ap )
	{
		char buffer[1024];
		int len = vsnprintf( buffer, sizeof(buffer)-1, __fmt, ap );
		len = len >= (int)sizeof(buffer) ? sizeof(buffer) - 1 : len;
		if( len > 0 )
		{
			buffer[len] = 0;
			if( LogclientClient::GetInstance()->SendProtocol(
				RemoteLog( __priority, std::string(buffer,len), g_hostname, g_progname) ) )
			{
				if( Thread::Pool::Size() > 1 )
					PollIO::WakeUp();
			}
			else
			{
				Log::vsyslog( __priority, buffer, len );
			}
		}
	}

	void Log::vlogvital( int __priority, const char * __fmt, va_list ap )
	{
		char buffer[1024];
		int len = vsnprintf( buffer, sizeof(buffer)-1, __fmt, ap );
		len = len >= (int)sizeof(buffer) ? sizeof(buffer) - 1 : len;
		if( len > 0 )
		{
			buffer[len] = 0;
			if( LogclientTcpClient::GetInstance()->SendProtocol(
				RemoteLogVital( __priority, std::string(buffer,len), g_hostname, g_progname) ) )
			{
				if( Thread::Pool::Size() > 1 )
					PollIO::WakeUp();
			}
			else
			{
				Log::vsyslog( __priority, buffer, len );
			}
		}
	}

	void Log::vstatinfo( int __priority, const char * __fmt, va_list ap )
	{
		char buffer[1024];
		int len = vsnprintf( buffer, sizeof(buffer)-1, __fmt, ap );
		len = len >= (int)sizeof(buffer) ? sizeof(buffer) - 1 : len;
		if( len > 0 )
		{
			buffer[len] = 0;

			if( LogclientTcpClient::GetInstance()->SendProtocol(
				StatInfoVital( __priority, std::string(buffer,len), g_hostname, g_progname) ) )
			{
				if( Thread::Pool::Size() > 1 )
					PollIO::WakeUp();
			}
			else
			{
				Log::vsyslog( __priority, buffer, len );
			}
		}
	}

}


