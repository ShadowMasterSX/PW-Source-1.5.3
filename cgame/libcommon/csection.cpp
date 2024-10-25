#include <string.h>
#include <stdlib.h>

#include "csection.h"
#include "spinlock.h"


#if ( !defined(WIN32) && !defined(LINUX) ) || defined(USE_PTHREAD)
#include "../pthread.h"
#elif defined(WIN32)
#include <windows.h>
#elif defined(LINUX)
#include <unistd.h>
#endif


typedef struct critical_section_t 
{
#if ( !defined(WIN32) && !defined(LINUX) ) || defined(USE_PTHREAD)
	pthread_t thread_id;
#elif defined(WIN32)
	unsigned int thread_id;
#elif defined(LINUX)
	unsigned int thread_id;
#endif
	int	spin_lock;
	int	spin_count;
}critical_section;

critical_section * initialize_critical_section()
{
	critical_section * cs;
	cs = (critical_section *) malloc(sizeof(*cs));
	memset(cs,0,sizeof(*cs));
	return cs;
}

void delete_critical_section(critical_section * __cs)
{
	free(__cs);
}

void  enter_critical_section(critical_section *__cs)
{
	int rst;
#if ( !defined(WIN32) && !defined(LINUX) ) || defined(USE_PTHREAD)
	pthread_t thread_id;    // use POSIX standard get thread function;
	thread_id = thread_self();	
#elif defined( WIN32 )		// Windows thread has a unique ID 
	unsigned int thread_id;
	thread_id = GetCurrentThreadId();
#elif defined(LINUX)
	unsigned int thread_id;
	thread_id = getpid();	// linux use process implement thread
#endif

	rst = mutex_spinset(&(__cs->spin_lock));
	if(!rst)	//successfully lock ^_^
	{
		__cs->thread_id = thread_id;
		__cs->spin_count = 1;
		return ;
	}
	else
	{

/*
	In WINOWS, thread id is an simple unsigned int.
	In LINUX , we use process ID instead of threa id.
	we use simple compare to determine difference of two thread_id.
	In POSIX standard use function pthread_equal to compare two thread_t
*/
#if ( !defined(WIN32) && !defined(LINUX) ) || defined(USE_PTHREAD)
		if(pthread_equal(thread_id,__sc->thread_id))
#else
		if(__cs->thread_id == thread_id)	//same thread, no need lock
#endif
		{
			__cs->spin_count ++;
			return;
		}

		//other thread, need lock again :(
		mutex_spinlock(&(__cs->spin_lock));
		__cs->spin_count = 1;
		__cs->thread_id	 = thread_id;
	}
	return;
}

void  leave_critical_section(critical_section *__cs)
{
	__cs->spin_count --;
	if(!__cs->spin_count)
	{
		__cs->thread_id = 0;
		__cs->spin_lock = 0;
	}
}

