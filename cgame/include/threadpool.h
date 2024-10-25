/* 
 	线程池函数头文件，文件原名thread.h，为了避免冲突，更改了名称
	原作者：sunzhenyu
	修改：崔明
	ChangeLog:去掉了一些复杂的功能,修改了一些接口,使用末拌的方式去掉了cpp文件
 */


#ifndef __ONLINEGAME_COMMON_THREAD_H
#define __ONLINEGAME_COMMON_THREAD_H

#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>

#include <vector>
#include <string>
#include <queue>
#include "spinlock.h"
#include "interlocked.h"

//#define __INDEPENDENT_TASK_QUEUE__

namespace ONET
{

namespace Thread
{
	class Condition;

	class Runnable
	{
		int m_priority;
	public:
		struct Compare
		{
			bool operator() (const Runnable * p1, const Runnable * p2 ) const
			{
				return p1->GetPriority() < p2->GetPriority();
			}
		};

		Runnable( int priority = 0 ) : m_priority(priority) { }
		virtual ~Runnable() { }

		virtual void Run() = 0;

		int GetPriority() const { return m_priority; }
		void SetPriority( int priority ) { m_priority = priority; }
	};
	class Mutex
	{
		friend class Condition;
		int mutex;
	public:
		~Mutex () 
		{
		}
		Mutex():mutex(0)
		{}
		void Lock()   { mutex_spinlock(&mutex); }
		void UNLock() { mutex_spinunlock(&mutex); }
		class Scoped
		{
			Mutex *mx;
		public:
			~Scoped () { if(mx) mx->UNLock(); }
			explicit Scoped(Mutex& m) : mx(&m) { mx->Lock(); }
			void Detach() { mx = NULL; }
			void Unlock() { mx->UNLock();}
			void Lock() {mx->Lock();}
		};
	};

	class RWLock : public Mutex
	{
	public:
		~RWLock()
		{
		}
		RWLock() {}
		void WRLock() { Mutex::Lock(); } 
		void RDLock() { Mutex::Lock(); }
		void UNLock() { Mutex::UNLock(); }
		class WRScoped
		{
			RWLock *rw;
		public:
			~WRScoped () { rw->UNLock(); }
			explicit WRScoped(RWLock &l) : rw(&l) { rw->WRLock(); }
		};
		class RDScoped
		{
			RWLock *rw;
		public:
			~RDScoped () { rw->UNLock(); }
			explicit RDScoped(RWLock &l) : rw(&l) { rw->RDLock(); }
		};
	};

	class Pool
	{
	public:
		enum PoolState
		{
			stateNormal = 1,
			stateQuit = 2,
			stateQuitAtOnce = 3
		};

		//typedef std::priority_queue<Runnable*, std::vector<Runnable*>, Runnable::Compare> TaskQueue;
		typedef std::deque<Runnable*> TaskQueue;

	private:

#ifdef __INDEPENDENT_TASK_QUEUE__
		static TaskQueue *s_tasks;
		static int  *s_mutex_tasks;
		static size_t *s_task_size2;
		static bool s_pool_pause;
#else
		static TaskQueue s_tasks;
		static int s_mutex_tasks;
#endif

		static size_t s_task_size;
		static size_t s_thread_count;
		static size_t s_task_count;

	public:

		static void CreatePool( int pool_count)
		{
			{
				spin_autolock keeper(s_mutex_tasks);
				ASSERT(s_thread_count == 0 && pool_count > 0);
				s_thread_count = pool_count;
#ifdef __INDEPENDENT_TASK_QUEUE__
				s_tasks = new TaskQueue[s_thread_count];
				s_mutex_tasks = new int[s_thread_count];
				s_task_size2 = new size_t[s_thread_count];
				memset(s_mutex_tasks,0,sizeof(int)*s_thread_count);
				memset(s_task_size2,0,sizeof(size_t)*s_thread_count);
#endif

				pthread_t th;
				for( size_t i=0; i<s_thread_count; i++ )
					pthread_create( &th, NULL, &Pool::RunThread, (void*)i);
			}
		}

		static void CreateThread(Runnable * task)
		{
			pthread_t ph;
			pthread_create( &ph, NULL, &Pool::RunSingleThread, task);
		}

		static void CreateThread(void (*thread_routine)())
		{
			pthread_t ph;
			pthread_create( &ph, NULL, &Pool::RunSingleThread2, (void*)thread_routine);
		}

#ifdef __INDEPENDENT_TASK_QUEUE__
private:	
		static inline void AddSpecTask(size_t index, Runnable * task)
		{
			mutex_spinlock2(s_mutex_tasks + index);
			s_tasks[index].push_back( task );
			s_task_size2[index] ++;
			mutex_spinunlock(s_mutex_tasks + index);
		}
public:
		static void AddTask(Runnable * task, size_t index)
		{
			size_t pool_index = index % s_thread_count;
			AddSpecTask(pool_index, task);
			interlocked_increment((int*)&s_task_size);
		}

		static void AddTask( Runnable * task )
		{
			AddSpecTask(rand() % s_thread_count, task);
			interlocked_increment((int*)&s_task_size);
		}

		static void Pause(bool is_pause)
		{
			s_pool_pause = is_pause;
		}
		
#else
		static void AddTask( Runnable * task )
		{
			mutex_spinlock2(&s_mutex_tasks);
			s_tasks.push_back( task );
			s_task_size ++;
			mutex_spinunlock(&s_mutex_tasks);
		}
#endif

		static size_t QueueSize()
		{
			//spin_autolock keeper(s_mutex_tasks);
			//return s_tasks.size();
			return s_task_size;
		}
		
		static size_t GetTotalTaskCount()
		{
			return s_task_count;
		}

		static size_t Size()
		{
			return s_thread_count;
		}

		static void Dump();


	private:
		virtual ~Pool()
		{
		}

		static void * RunSingleThread(void * arg)
		{
			pthread_detach( pthread_self() );
			((Runnable*)arg)->Run();
			return NULL;
		}

		static void * RunSingleThread2(void * arg)
		{
			pthread_detach( pthread_self() );
			void (*routine)() = ((void (*)())arg);
			(*routine)();
			return NULL;
		}

		static void * RunThread( void * );
	};

}
}

class rdlock_scoped
{
	pthread_rwlock_t &_lock;
public:
	rdlock_scoped(pthread_rwlock_t & lock):_lock(lock)
	{
		pthread_rwlock_rdlock(&_lock);
	}
	~rdlock_scoped()
	{
		pthread_rwlock_unlock(&_lock);
	}
};

class wrlock_scoped
{
	pthread_rwlock_t & _lock;
public:
	wrlock_scoped(pthread_rwlock_t & lock):_lock(lock)
	{
		pthread_rwlock_wrlock(&_lock);
	}
	~wrlock_scoped()
	{
		pthread_rwlock_unlock(&_lock);
	}
};
#endif
