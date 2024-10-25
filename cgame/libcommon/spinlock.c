
#include "spinlock.h"
#include "mtime.h"
#include <stdio.h>

#define MUTEX_SPIN_COUNT                50
#define MUTEX_SPINSLEEP_DURATION        1001				//针对2.6以上的内核，1ms的精度可以让它反映更加快


#if  defined( WIN32 ) || defined(WIN64)
__inline static  int   test_and_set(int *spinlock)
{
	__asm{
		MOV EBX,[spinlock]
		MOV EAX,0x01
		XCHG EAX,[EBX]
	}
}
#else
#include <sched.h>
static inline int test_and_set (int *spinlock)
{
	int ret;

	__asm__ __volatile__(
		"xchgl %0, %1"
		: "=r"(ret), "=m"(*spinlock)
		: "0"(1), "m"(*spinlock)
		: "memory");

	return ret;
}
#endif


int  mutex_spinset(int *__spinlock)
{
	return test_and_set(__spinlock);
}

int mutex_spinwait(int *__spinlock,int __timeout)
{
	int rst;
	int cnt   = 0;
	int first = 1;
	unsigned int tv,tv0 = 0;
	while( (rst=test_and_set(__spinlock)) && __timeout ) 
	{
		////////////////////////////
		// check if timeout
		////////////////////////////
		if(first){
			first = 0;
			tv0 = gettickcount();
			tv0 += __timeout;
		}
		tv = gettickcount();		
		if( ((int)tv -(int)tv0) > 0)
		{
			break;
		}

		////////////////////////////
		// sleep delay
		////////////////////////////
		if(cnt < MUTEX_SPIN_COUNT) 
		{
#if  defined( WIN32 )
			//SwitchToThread();  faint win98 not support
			usleep(0);		// for win98, use sleep to swtich
#elif defined( LINUX )
			sched_yield();
#else
#error not for non-linux yet
#endif
			cnt++;
		} 
		else 
		{
			usleep(MUTEX_SPINSLEEP_DURATION);
			cnt = 0;
		}
	}
	return rst;
}


void mutex_spinlock(int *__spinlock)
{
	int cnt = 0;
#ifdef _DEBUG
int  icount = 0;
#endif
	while( test_and_set(__spinlock)) 
	{
		////////////////////////////
		// sleep delay
		////////////////////////////
		if(cnt < MUTEX_SPIN_COUNT + 1) 
		{
#if  defined( WIN32 )
			//SwitchToThread();  faint win98 not support
			usleep(0);		// for win98, use sleep to swtich
#elif defined( LINUX )
			sched_yield();
#else
#error not for non-linux yet
#endif
			cnt++;
		} 
		else 
		{
			usleep(MUTEX_SPINSLEEP_DURATION);
			cnt = 1;
#ifdef _DEBUG
			icount ++;
			if(icount > 4096)
			{
				ASSERT(0 && "加锁的时间过长");
			}
#endif
		}
	}

#ifdef _DEBUG
	if(cnt)
	{
		//printf("####***************************************************************###%08x:%d:%d\n",__spinlock,icount,cnt);
	}
#endif
	return;
}

int mutex_spinlock2(int *__spinlock)
{
	int cnt = 0;
#ifdef _DEBUG
int  icount = 0;
#endif
	while( test_and_set(__spinlock)) 
	{
		////////////////////////////
		// sleep delay
		////////////////////////////
		if(cnt < MUTEX_SPIN_COUNT + 1) 
		{
#if  defined( WIN32 )
			//SwitchToThread();  faint win98 not support
			usleep(0);		// for win98, use sleep to swtich
#elif defined( LINUX )
			sched_yield();
#else
#error not for non-linux yet
#endif
			cnt++;
		} 
		else 
		{
			usleep(MUTEX_SPINSLEEP_DURATION);
			cnt = 1;
#ifdef _DEBUG
			icount ++;
			if(icount > 4096)
			{
				ASSERT(0 && "加锁的时间过长");
			}
#endif
		}
	}

	return cnt;
}


void mutex_spinunlock(int *spinlock)
{
	*spinlock = 0;
}

