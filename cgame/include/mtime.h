/*
	mtime.h
	作者： 崔铭
	功能： 与时间相关的函数
        注释：
*/

#ifndef __MTIME_H__
#define __MTIME_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32

struct timezone;
int  gettimeofday(struct timeval *tv,struct timezone *tz);
void usleep(unsigned long usec);

#else
#include <sys/time.h>
#include <unistd.h>
#endif

unsigned int 	gettickcount();		/*return millisecond*/
unsigned long 	timegettime();		/*return microsecond*/
int 		msleep(struct timeval *tv);

#ifdef __cplusplus
};
#endif
#endif

