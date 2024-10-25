
#ifndef __GNET_GLOG_H__
#define __GNET_GLOG_H__

#include <stdarg.h>
#include <syslog.h>
#include <stdint.h>
namespace GLog
{

	#define	GLOG_EMERG		LOG_EMERG
	#define	GLOG_ALERT		LOG_ALERT
	#define	GLOG_CRIT		CRIT
	#define	GLOG_ERR		LOG_ERR
	#define	GLOG_WARNING		LOG_WARNING
	#define GLOG_NOTICE		LOG_NOTICE
	#define GLOG_INFO		LOG_INFO
	#define GLOG_CHAT		8
	#define GLOG_CASH		9
	#define GLOG_ACTION		10

	void init(int lineid=0);

	void log( int __priority, const char * __fmt, ... )
#ifdef __GNUC__	
		__attribute__ ((format (printf, 2, 3)))
#endif
	;

	void logvital( int __priority, const char * __fmt, ... )
#ifdef __GNUC__	
		__attribute__ ((format (printf, 2, 3)))
#endif
	;

	void trace( const char * __fmt, ... );

	void tasklog( int roleid, int taskid, int type, const char * msg );
	void tasklog( int64_t roleid, int taskid, int type, const char * msg );

	void formatlog( const char * title, const char * msg );

	void task( int64_t roleid, int taskid, int type, const char * msg );
	void task( int roleid, int taskid, int type, const char * msg );
	//日志规范化修改
	void task(const char *tag, int userid, int roleid, int taskid, const char * szLog);

	void upgrade( int roleid, int level, int money );
	void upgrade( int64_t roleid, int level, int money );

	void die( int roleid, int type, int attacker = -1);
	void die( int64_t roleid, int type, int64_t attacker = -1);

	void keyobject( int id, int delta );

	void refine( int roleid, int item, int level, int result, int cost );
	void refine( int64_t roleid, int item, int level, int result, int cost );

	void cash( const char * __fmt, ... )
#ifdef __GNUC__	
		__attribute__ ((format (printf, 1, 2)))
#endif
	;
	
	void formatlog(const char * __fmt, ...)
#ifdef __GNUC__	
		__attribute__ ((format (printf, 1, 2)))
#endif
	;

	void action( const char * __fmt, ... )
#ifdef __GNUC__	
		__attribute__ ((format (printf, 1, 2)))
#endif
	;

}

#endif


