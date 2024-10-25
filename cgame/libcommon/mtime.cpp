#include "mtime.h"

#ifdef WIN32
#include "Winsock2.h"
#include "windows.h"
#endif

#ifdef WIN32

int  gettimeofday(struct timeval *tv,struct timezone *tz)
{
	int tick;
	tick = GetTickCount();
	tv->tv_sec  = tick /1000;
	tv->tv_usec = (tick % 1000) * 1000;
	return 0;
}
unsigned int gettickcount()
{
	return GetTickCount();
}

unsigned long timegettime()
{
	static int first_init = 0;
	static LARGE_INTEGER frequency;
	static LARGE_INTEGER factor;
	static LARGE_INTEGER factor2;
	LARGE_INTEGER ccounter;
	int rst;
	unsigned long tmp;
	if(!first_init){
		rst = QueryPerformanceFrequency(&frequency);
		if(!rst) return 0;
		first_init = 1;
		factor.QuadPart = frequency.QuadPart * 0xffffffff / 1000000;
		factor2.QuadPart = frequency.QuadPart % 1000000;
	}
	rst = QueryPerformanceCounter(&ccounter);
	if(!rst) return 0;
	tmp = (unsigned long)(((ccounter.QuadPart % factor.QuadPart) * 1000000 + factor.QuadPart)/ frequency.QuadPart);
	return tmp;
}

int msleep(struct timeval *tv)
{
	long t;
	t = tv->tv_sec * 1000000 + tv->tv_usec;
	unsigned long time;
	unsigned long time2;
	time = timegettime();
	time2= time + t - 3000;
	while(((long)time2 - (long)time) > 0)
	{
		Sleep(1);
		time = timegettime();
	}
	return 0;
}

void usleep(unsigned long usec)
{
	int t;
	t = usec /1000;
	Sleep(t);
}

//#elif LINUX
#else

#include <sys/time.h>

//在LINUX下的时间函数，呵呵
unsigned int gettickcount()
{
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return tv.tv_sec*1000 + tv.tv_usec/1000;
}

unsigned long timegettime()
{
	unsigned long long t1;
	struct timeval tv;
	gettimeofday(&tv,NULL);
	t1 = tv.tv_sec;
	t1 *= 1000000;
	t1 += tv.tv_usec;
	return (unsigned long )(t1 & 0xffffffff);
}

int msleep(struct timeval *tv)
{
	unsigned long t;
	t = (unsigned long)tv->tv_sec * 1000000 + tv->tv_usec;
	usleep(t);
	return 0;
}

#endif

