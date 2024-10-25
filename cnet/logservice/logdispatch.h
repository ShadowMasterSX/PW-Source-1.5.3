
#ifndef __GNET_LOGDISPATCH_H__
#define __GNET_LOGDISPATCH_H__

#include <stdio.h>
#include <syslog.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/un.h>

#include <ios>
#include <iostream>
#include <fstream>
#include <string>

#include "conf.h"
#include "timer.h"
#include "log.h"

using std::string;

namespace GNET
{

	class LogDispatch
	{
		enum argument {
			flags = O_WRONLY|O_CREAT|O_APPEND|O_NONBLOCK,//|O_LARGEFILE,
			mode  = S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH
		};

		static int fd_err;
		static int fd_log;
		static int fd_formatlog;
		static int fd_trace;
		static int fd_chat;
		static int fd_cash;

		static int fd_statinfom;
		static int fd_statinfoh;
		static int fd_statinfod;

		static int threshhold;

		static int writefile( int fd, int priority,
					std::string& msg, std::string& hostname, std::string& servicename )
		{
			char buffer[1024];
    
			struct tm now_tm;
			time_t now = Timer::GetTime();
			size_t pos = strftime(buffer, sizeof(buffer)-1, "%Y-%m-%d %H:%M:%S ", localtime_r(&now, &now_tm));
								// syslog format "%h %e %T "
    
			if( pos + hostname.length() + 5 < sizeof(buffer) )
			{
				strncpy( buffer+pos, hostname.c_str(), hostname.length() );
				pos += hostname.length();
			}
			strncpy( buffer+pos, " ", 1 );
			pos ++;
    
			if( pos + servicename.length() + 5 < sizeof(buffer) )
			{
				strncpy( buffer+pos, servicename.c_str(), servicename.length() );
				pos += servicename.length();
			}
			strncpy( buffer+pos, ": ", 2 );
			pos ++;
			pos ++;
    
			if( pos + 20 < sizeof(buffer) )
				pos += sprintf( buffer+pos, "%s : ", Log::prioritystring(priority) );
    
			strncpy( buffer+pos, msg.c_str(), std::min(msg.length(),sizeof(buffer)-pos-1) );
			pos += std::min(msg.length(),sizeof(buffer)-pos-1);
    
			if( buffer[pos-1] != '\n' )
				buffer[pos++] = '\n';
			return write( fd, buffer, pos );
		}

	public:
		static void openfiles( )
		{
			Conf * conf = Conf::GetInstance();

			std::string err = conf->find("logservice","fd_err");
			std::string log = conf->find("logservice","fd_log");
			std::string formatlog = conf->find("logservice","fd_formatlog");
			std::string trace = conf->find("logservice","fd_trace");
			std::string chat = conf->find("logservice","fd_chat");
			std::string cash = conf->find("logservice","fd_cash");
			std::string statinfom = conf->find("logservice","fd_statinfom");
			std::string statinfoh = conf->find("logservice","fd_statinfoh");
			std::string statinfod = conf->find("logservice","fd_statinfod");

			if( err.length() <= 0 )
				err			=	"/var/log/world2.err";
			if( log.length() <= 0 )
				log			=	"/var/log/world2.log";
			if( formatlog.length() <= 0 )
				formatlog	=	"/var/log/world2.formatlog";
			if( trace.length() <= 0 )
				trace		=	"/var/log/world2.trace";
			if( chat.length() <= 0 )
				chat		=	"/var/log/world2.chat";
			if( cash.length() <= 0 )
				cash		=	"/var/log/world2.cash";
			if( statinfom.length() <= 0 )
				statinfom	=	"/var/log/statinfom";
			if( statinfoh.length() <= 0 )
				statinfoh	=	"/var/log/statinfoh";
			if( statinfod.length() <= 0 )
				statinfod	=	"/var/log/statinfod";

			fd_err = open( err.c_str(), flags, mode );
			fd_log = open( log.c_str(), flags, mode );
			fd_formatlog = open( formatlog.c_str(), flags, mode );
			fd_trace = open( trace.c_str(), flags, mode );
			fd_chat = open( chat.c_str(), flags, mode );
			fd_cash = open( cash.c_str(), flags, mode );
			fd_statinfom = open( statinfom.c_str(), flags, mode );
			fd_statinfoh = open( statinfoh.c_str(), flags, mode );
			fd_statinfod = open( statinfod.c_str(), flags, mode );
		}

		static void closefiles( )
		{
			close( fd_err );
			close( fd_log );
			close( fd_formatlog );
			close( fd_trace );
			close( fd_chat );
			close( fd_cash );
			close( fd_statinfom );
			close( fd_statinfoh );
			close( fd_statinfod );
			fd_err = fd_log = fd_formatlog = fd_trace = fd_chat = fd_cash =
					fd_statinfom = fd_statinfoh = fd_statinfod = -1;
		}

		static void set_threshhold( int __threshhold )
		{
			threshhold = __threshhold;
		}

		static void log( int priority, string& msg, string& hostname, string& servicename )
		{
			if( LOG_CHAT == priority )
			{
				writefile( fd_chat, priority, msg, hostname, servicename );
				return;
			}
			if( LOG_CASH == priority )
			{
				writefile( fd_cash, priority, msg, hostname, servicename );
				return;
			}

			if( LOG_WARNING >= priority )
			{
				writefile( fd_err, priority, msg, hostname, servicename );
			}
			else if( LOG_NOTICE == priority )
			{
				writefile( fd_formatlog, priority, msg, hostname, servicename );
			}

			if( priority > threshhold )
				return;

			if( LOG_INFO == priority )
			{
				writefile( fd_log, priority, msg, hostname, servicename );
			}
			else if( LOG_DEBUG == priority )
			{
				writefile( fd_trace, priority, msg, hostname, servicename );
			}
		}

		static void stat( int priority, string& msg, string& hostname, string& servicename )
		{
			if( LOG_DEBUG == priority )
			{
				writefile( fd_statinfom, priority, msg, hostname, servicename );
			}
			else if( LOG_INFO == priority )
			{
				writefile( fd_statinfoh, priority, msg, hostname, servicename );
			}
			else if( LOG_NOTICE == priority )
			{
				writefile( fd_statinfod, priority, msg, hostname, servicename );
			}
		}
	};
}

#endif


