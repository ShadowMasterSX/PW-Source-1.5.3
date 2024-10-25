#ifndef __IOLIB_GLOG_H__
#define __IOLIB_GLOG_H__

#include "pollio.h"
namespace GLog
{
	void Init(const char * name, GNET::IOMan * man = NULL);
	void log( int __priority, const char * __fmt, ... ) 			__attribute__ ((format (printf, 2, 3)));
	void logvital( int __priority, const char * __fmt, ... ) 		__attribute__ ((format (printf, 2, 3)));
	void trace( const char * __fmt, ... ) 					__attribute__ ((format (printf, 1, 2)));
	void formatlog( const char * title, const char * __fmt, ... ) 		__attribute__ ((format (printf, 2, 3)));
	void formatlog(const char * __fmt, ... ) 				__attribute__ ((format (printf, 1, 2)));
}

#endif

