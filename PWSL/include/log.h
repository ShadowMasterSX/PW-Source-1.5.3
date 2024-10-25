/*
 * 		基础日志功能的实现，缺省实现是记录到syslog中的，提供了vlog系列函数和LogTrait，以备后面自定义记录日志到其他地方
 *		公司：完美时空
 *		日期：2009-06-18
 *		修改：崔铭 加入了LogTrait
 */

#ifndef __GNET_LOG_H__
#define __GNET_LOG_H__

#include <stdio.h>
#include <syslog.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/un.h>
#include <errno.h>
#include <stdlib.h>

#include <ios>
#include <iostream>
#include <fstream>
#include <string>


//LOG_DEBUG=7
#define GLOG_ERR	LOG_ERR
#define GLOG_INFO	LOG_INFO
#define GLOG_WARNING	LOG_WARNING
#define LOG_CHAT	8
#define LOG_STAT	9
#define LOG_ACTION	10

namespace GNET
{
	class LogTrait
	{
	public:
		virtual ~LogTrait() {}
		virtual const char * GetPriorityName(int __priority) = 0;
		virtual void SetProgName(const char * __name) = 0;
		virtual void VLog(int __priority, const char * __fmt, va_list ap) = 0;
		virtual void VLogVital(int __priority, const char * __fmt, va_list ap) = 0;
		virtual void VStatInfo(int __priority, const char * __fmt, va_list ap) = 0;
		virtual void LogString(int __priotiry, const char * string) = 0;
	};

	class  Statistic;
	class Log
	{
	private:
		char * m_progname;
		bool	connected;
		int		LogFile;
		int		LogFacility;
		int 		console_mask;
		int		tracelevel;

		enum {
			DEFAULT_CONSOLE_MASK =  (1 << LOG_EMERG) | (1 << LOG_ALERT) | (1 << LOG_CRIT)
				| (1 << LOG_ERR) | (1 << LOG_WARNING) /*| (1 << LOG_NOTICE)*/
				| (1 <<LOG_INFO) | (1 << LOG_DEBUG) | (1 << LOG_CHAT)
				| (1 <<LOG_STAT) | (1 << LOG_ACTION) ,
		};

		Log() : m_progname(0),connected(false), LogFile(-1), LogFacility(LOG_USER), console_mask(DEFAULT_CONSOLE_MASK),tracelevel(1) { }

		static Log s_log;
		static LogTrait *_trait;

		~Log()
		{
			if( connected ) close( LogFile );
			closelog();
			if (m_progname) free(m_progname);
		}

		static Log & instance( ) { return s_log; }
		static bool LogStatistic( const std::string & __name, const Statistic * __pstat ); 	//记录一条统计信息 到系统日志中

		static void statinfo( int __priority, const char * __fmt, ... );
		static void open_internal( );
	public:
		static void vsyslog( int pri, const char * log, int len);
		static void vsyslog( int pri, const char * fmt, va_list ap );
		static void syslog_string( int pri, const char * str);

		static void setconsole(int __priority, bool enable);		//指定某个等级的日志是否在屏幕中打出
		static void vconsole(int __priority, const char * fmt, va_list ap); //向标准出错输出
	public:

		static void StatEnumerate( unsigned int __interval );			//记录指定时间的统计信息(min5, hour day) 会打开相应的日志并在写入后关闭

	//进行初始化和对优先级进行文字描述的函数
		static const char * prioritystring( int __priority );
		static void setprogname( const char * __name );
		
	//几个日志函数 使用 va_list作为参数的接口 ，这这些函数会根据trait的不同进行不同的转发工作
		static void vlog( int __priority, const char * __fmt, va_list ap );
		static void vlogvital( int __priority, const char * __fmt, va_list ap );
		static void vstatinfo( int __priority, const char * __fmt, va_list ap );
		static void logstr(int __priority, const char * str);
	
	//会写入syslog的日志函数 ，这些函数是肯定会写入到syslog之中的，正常情况下不建议调用这些函数
		static void syslog_setprogname( const char * __name );
		static void syslog_vlog( int __priority, const char * __fmt, va_list ap ) { Log::vsyslog( __priority, __fmt, ap );}
		static void syslog_vlogvital( int __priority, const char * __fmt, va_list ap ) { Log::vsyslog( __priority, __fmt, ap );}
		static void syslog_vstatinfo( int __priority, const char * __fmt, va_list ap ) { Log::vsyslog( __priority, __fmt, ap );}
		static void syslog_logstr(int __priority, const char * str) { Log::syslog_string(__priority, str);}

	//变参数封装的函数，这些函数在内部调用了使用va_list的函数实现
		static void log( int __priority, const char * __fmt, ... );
		static void logvital( int __priority, const char * __fmt, ... );
		static void trace( const char * __fmt, ... );	//肯定记入LOG_DEBUG日志
		static int trace_level() {return s_log.tracelevel;}
		static void set_trace_level(int level) {s_log.tracelevel = level;}
		
		static LogTrait *  SetNewTrait(LogTrait * trait) { LogTrait *old = _trait;_trait = trait; return old;}
	};

}

#ifdef _DEBUG
	#define LOG_TRACE	GNET::Log::trace
	#define LOG_TRACE1	if (GNET::Log::trace_level()>=1) GNET::Log::trace
	#define LOG_TRACE2	if (GNET::Log::trace_level()>=2) GNET::Log::trace
	#define LOG_TRACE3	if (GNET::Log::trace_level()>=3) GNET::Log::trace
	#define LOG_TRACE4	if (GNET::Log::trace_level()>=4) GNET::Log::trace
#else
	#define LOG_TRACE(...)
	#define LOG_TRACE1(...)
	#define LOG_TRACE2(...)
	#define LOG_TRACE3(...)
	#define LOG_TRACE4(...)
#endif


#endif


