#include "threadpool.h"

namespace ONET
{
int __thread_lock_count = 0;
int __thread_task_count = 0;
namespace Thread{


size_t 	Thread::Pool::s_task_size = 0;
size_t	Thread::Pool::s_thread_count = 0;
size_t	Thread::Pool::s_task_count = 0;

#ifdef __INDEPENDENT_TASK_QUEUE__
Thread::Pool::TaskQueue *Thread::Pool::s_tasks = NULL;
int	*Thread::Pool::s_mutex_tasks = NULL;
size_t	*Thread::Pool::s_task_size2 = NULL;
bool	Thread::Pool::s_pool_pause = false;
#else
Thread::Pool::TaskQueue	Thread::Pool::s_tasks;
int	Thread::Pool::s_mutex_tasks;
#endif

extern "C" int mutex_spinlock2(int *__spinlock);
void Thread::Pool::Dump()
{
#ifdef __INDEPENDENT_TASK_QUEUE__
	for(size_t i = 0; i < s_thread_count; i ++)
	{
		printf("%d: task %d\n",i, s_task_size2[i]);
	}
#endif
}

void * Thread::Pool::RunThread( void * arg )
{
	pthread_detach( pthread_self() );

	size_t thread_index = (size_t)arg;
	ASSERT(thread_index < s_thread_count);

	// thread block all signal
	//sigset_t sigs;
	//sigfillset(&sigs);
	//pthread_sigmask(SIG_BLOCK, &sigs, NULL);
	
#ifdef __INDEPENDENT_TASK_QUEUE__
	TaskQueue & queue = s_tasks[thread_index];
	int	  * lock = s_mutex_tasks + thread_index;
	size_t	  & task_size = s_task_size2[thread_index];
#else
	TaskQueue & queue = s_tasks;
	int 	  * lock = &s_mutex_tasks;
	size_t	  & task_size = s_task_size;
#endif

#ifdef __INDEPENDENT_TASK_QUEUE__

	TaskQueue exec_queue;
	while (true)
	{
		if(!task_size || s_pool_pause)
		{
			while(!task_size || s_pool_pause)
			{
				usleep(1000);	//并非所有的内核都能到到1000us的精度
			}
		}

		int rst = mutex_spinlock2(lock);
#ifndef NDEBUG
		__thread_lock_count += rst?1:0;
#endif
		size_t c_count = queue.size();
		if(!c_count)
		{
			mutex_spinunlock(lock);
			continue;
		}
		exec_queue.swap(queue);
		task_size -= c_count;
		mutex_spinunlock(lock);
		TaskQueue::iterator it = exec_queue.begin();
		for(; it != exec_queue.end(); ++it)
		//for(size_t i = 0; i < c_count; i ++)
		//while (!exec_queue.empty())
		{
			Runnable * pTask = *it;
			if(pTask) pTask->Run();
		}
		/*
		for(size_t i = 0; i < c_count; i ++)
		{
			for(size_t j= 0; j <500;j ++)
			{
			}
		}*/
		
		exec_queue.clear();
		interlocked_sub((int*)&s_task_size,(int)c_count);
		interlocked_add((int*)&__thread_task_count,(int)c_count);
	}
#else
	while (true)
	{
		try {

			Runnable * pTask = NULL;
			{
				if(!task_size)
				{
					while(!task_size)
					{
						usleep(1000);	//并非所有的内核都能到到1000us的精度
					}
				}

				int rst = mutex_spinlock2(lock);
#ifndef NDEBUG
				__thread_lock_count += rst?1:0;
#endif
				if (!queue.empty())
				{
					pTask = queue.front();
					queue.pop_front();
					task_size --;
					
				}
				__thread_task_count ++;
				mutex_spinunlock(lock);
				
			}
			if(pTask)
			{
				pTask->Run();
			}

		} catch ( ... ) { }
	}
#endif
	return NULL;
}
}
}
