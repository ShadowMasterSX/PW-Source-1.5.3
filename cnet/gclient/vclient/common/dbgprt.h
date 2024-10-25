/*
*filename:		dbgprt.h 
*description:	copy from cgame/gs/dbgprt.h 
*/
#ifndef __ONLINEGAME_GS_DBGPRT_H__
#define __ONLINEGAME_GS_DBGPRT_H__

#include <stdio.h>
#include <stdarg.h>

extern bool __PRINT_FLAG;
extern bool __PRINT_INFO_FLAG;

inline int __PRINTF(const char * fmt, ...)
{
	if(!__PRINT_FLAG) return 0;
	va_list ap;
	va_start(ap, fmt);
	int rst = vprintf(fmt, ap);
	va_end(ap);
	return rst;
}

inline int __PRINTINFO(const char * fmt, ...)
{
	if(!__PRINT_INFO_FLAG) return 0;
	va_list ap;
	va_start(ap, fmt);
	int rst = vprintf(fmt, ap);
	va_end(ap);
	return rst;
}

#endif

