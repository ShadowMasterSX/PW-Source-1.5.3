#ifndef __THREAD_H
#define __THREAD_H

#include <signal.h>

#include <vector>
#include <string>
#include <queue>
#include <list>

#include "mutex.h"

#include "timer.h"
#include "statistic.h"
#include "benchmark.h"
#include "log.h"

namespace GNET
{

namespace Thread
{
	class Runnable
	{
	protected:
		int m_priority;

	public:
		Runnable( int priority = 1 )
			: m_priority(priority)
		{
	   	}
		virtual ~Runnable() { }

		virtual void Run() = 0;

		void SetPriority( int priority ) { m_priority = priority; }
		int GetPriority() const { return m_priority; }
	};

};

};

#if defined _REENTRANT_
#include <pthread.h>
#include <errno.h>
#include <unistd.h>

extern int __gnfailed;
extern int __gnRDfailed;
extern int __gnWRfailed;
namespace GNET
{

namespace Thread
{
	class Pool
	{
	public:
		enum PoolState
		{
			stateNormal = 1,
			stateQuitAtOnce = 2
		};

		typedef std::map< int, size_t > CountMap;
		typedef std::multimap< int, Runnable*, std::less<int> > TaskQueue;

		class Policy
		{
			friend class Pool;
		protected:
			size_t m_max_queuesize;
			CountMap m_thread_counts;

			int    m_state;
			size_t m_thread_count;

			void LoadConfig();

		public:
			static Policy s_policy;

			Policy() : m_max_queuesize(1048576), m_state(stateNormal)
			{  }

			virtual ~Policy() { }

			void SetState( int state )
			{
				m_state = state;

				if( stateQuitAtOnce == state )
					Pool::s_cond_threads.NotifyAll();
			}
			void SetMaxQueueSize( size_t max_queuesize )
			{
				m_max_queuesize = max_queuesize;
			}
			CountMap & GetThreadCountMap() { return m_thread_counts; }

			virtual std::string Identification() { return "ThreadPool"; }

			virtual size_t OnGetThreadCount(int prior)
			{
				if( stateQuitAtOnce == m_state )
					return 0;
				return std::max( 1, (int)m_thread_counts[prior] );
			}

			virtual size_t OnGetThreadCount() const
			{
				if( stateQuitAtOnce == m_state )
					return 0;
				return m_thread_count;
			}

			virtual bool OnAddTask(Thread::Runnable * pTask, size_t queuesize, bool bForced)
			{
				STAT_MIN5("TaskQueueSize",queuesize);
				if( stateNormal == m_state )
				{
					if( pTask && (queuesize<m_max_queuesize || bForced) )
						return true;
					Log::log( LOG_WARNING, "OnAddTask failed.(pTask=%u,QueueSize=%u,TaskPriority=%u)",
								pTask, queuesize, pTask->GetPriority() );
				}
				delete pTask;
				return false;
			}

			virtual void OnQuit( ) { }
			virtual void OnSIGUSR2( ) { }
			virtual void OnSIGHUP( ) { }
		};
	private:
		static TaskQueue s_tasks;
		static Mutex s_mutex_tasks;

		static Mutex s_mutex_threads;
		static Condition s_cond_threads;
		static size_t s_thread_count;
		static CountMap s_thread_counts;
		static bool s_prior_strict;

		static Policy * s_ppolicy;

		static void sigusr1_handler( int signum );

	public:
		static void setupdaemon( )
		{
			switch(fork())
			{
			case    0:
				break;
			case    -1:
				exit(-1);
			default:
				exit(0);
			}

			setsid();
		}

		static void SetPolicy( Policy * policy )
		{
			s_ppolicy = policy;
		}

		static void Run( Policy * policy = &(Policy::s_policy), bool wantreturn = false );
		static void AddTask( Runnable * task, bool bForced = false )
		{
			if( !s_ppolicy || s_ppolicy->OnAddTask( task, QueueSize(), bForced ) )
			{
				{
					Mutex::Scoped lock(s_mutex_tasks);
					s_tasks.insert( std::make_pair(task->GetPriority(),task) );
				}
				s_cond_threads.NotifyAll();
			}
		}

		static size_t QueueSize()
		{
			Mutex::Scoped lock(s_mutex_tasks);
			return s_tasks.size();
		}

		static size_t Size()
		{
			return s_thread_count;
		}

	private:
		virtual ~Pool()
		{
		}

		static Runnable * FetchTask( int nThreadPrior )
		{
			Mutex::Scoped lock(s_mutex_tasks);
			if( s_prior_strict )
			{
				Runnable * pTask = NULL;
				if (!s_tasks.empty())
				{
					TaskQueue::iterator it = s_tasks.find( nThreadPrior );
					if( it != s_tasks.end() )
					{
						pTask = (*it).second;
						s_tasks.erase( it );
					}
				}
				return pTask;
			}
			else
			{
				Runnable * pTask = NULL;
				if (!s_tasks.empty())
				{
					TaskQueue::iterator it = s_tasks.lower_bound( nThreadPrior );
					if( it == s_tasks.end() )
					{
						it = s_tasks.begin();
					}
					pTask = (*it).second;
					s_tasks.erase( it );
				}
				return pTask;
			}
		}

		static void * RunThread( void * pParam )
		{

			#ifdef __i386__
			int nThreadPrior = (int)pParam;
			#elif defined __x86_64__
			int nThreadPrior = (long)pParam;
			#endif

			pthread_detach( pthread_self() );

			sigset_t sigs;
			sigemptyset(&sigs);
			sigaddset(&sigs, SIGUSR1);
			sigaddset(&sigs, SIGUSR2);
			sigaddset(&sigs, SIGHUP);
			pthread_sigmask(SIG_BLOCK, &sigs, NULL);

			while (true)
			{
				try {
				while (true)
				{

					Runnable * pTask = FetchTask( nThreadPrior );
					if( pTask )
					{
						STAT_MIN5("DealedTask",1);
						try {
							pTask->Run();
						} catch ( ... ) { delete pTask; }
					}

					{
						Mutex::Scoped lock(s_mutex_threads);
						size_t thcount_want = s_ppolicy->OnGetThreadCount(nThreadPrior);
						size_t thcount_now = s_thread_counts[nThreadPrior];
						if( thcount_now < thcount_want )
						{
							s_thread_counts[nThreadPrior] ++;
							s_thread_count ++;

							pthread_t th;
							pthread_create( &th, NULL, &Pool::RunThread, NULL );
						}
						else if( thcount_now > thcount_want )
						{
							s_thread_counts[nThreadPrior] --;
							s_thread_count --;
							break;
						}

						if ( NULL == pTask )
							s_cond_threads.Wait(s_mutex_threads);
					}

				}
				} catch ( ... ) { continue; }
				break;
			}

			return NULL;
		}
	};




};

};

#else

extern int __gnfailed;
extern int __gnRDfailed;
extern int __gnWRfailed;
namespace GNET
{

namespace Thread
{
	class Pool
	{
	public:
		enum PoolState
		{
			stateNormal = 1,
			stateQuitAtOnce = 2
		};

		typedef std::multimap< int, Runnable*, std::less<int> > TaskQueue;
		// typedef std::queue<Runnable*> TaskQueue;

		class Policy
		{
			friend class Pool;
		protected:
			size_t m_max_queuesize;
			int    m_state;

			void LoadConfig();

		public:
			static Policy s_policy;

			Policy() : m_max_queuesize(1048576), m_state(stateNormal)
			{  }
			virtual ~Policy() { }

			void SetState( int state )
			{
				m_state = state;
			}
			void SetMaxQueueSize( size_t max_queuesize )
			{
				m_max_queuesize = max_queuesize;
			}

			virtual std::string Identification() { return "ThreadPool"; }
			virtual size_t OnGetThreadCount() const
			{
				if( stateQuitAtOnce == m_state )
					return 0;
				return 1;
			}
			virtual bool OnAddTask(Thread::Runnable * pTask, size_t queuesize, bool bForced)
			{
				STAT_MIN5("TaskQueueSize",queuesize);
				if(stateNormal == m_state)
				{
					if( pTask && (queuesize<m_max_queuesize || bForced) )
						return true;
					Log::log( LOG_WARNING, "OnAddTask failed.(pTask=%u,QueueSize=%u)",
							pTask, queuesize );
				}
				delete pTask;
				return false;
			}
			virtual void OnQuit( )
			{
			}
		};

	private:
		static TaskQueue s_tasks;

		static Policy * s_ppolicy;

		static void sigusr1_handler( int signum );

	public:
		static void setupdaemon( )
		{
			switch(fork())
			{
			case    0:
				break;
			case    -1:
				exit(-1);
			default:
				exit(0);
			}

			setsid();
		}

		static void SetPolicy( Policy * policy )
		{
			s_ppolicy = policy;
		}

		static void Run( Policy * policy = &(Policy::s_policy) )
		{
			s_ppolicy = policy;
			s_ppolicy->LoadConfig();

			//Log::log( LOG_INFO, "program started with 1 threads." );

			signal( SIGUSR1, sigusr1_handler );

			Pool::RunThread( NULL );
		}

		static void AddTask( Runnable * task, bool bForced = false )
		{
			if( !s_ppolicy || s_ppolicy->OnAddTask( task, QueueSize(), bForced ) )
			{
				s_tasks.insert( std::make_pair(task->GetPriority(),task) );
			}
		}

		static size_t QueueSize() { return s_tasks.size(); }

		static size_t Size() { return 1; }

	private:
		virtual ~Pool() { }

		static Runnable * FetchTask( )
		{
			Runnable * pTask = NULL;
			if (!s_tasks.empty())
			{
				TaskQueue::iterator it = s_tasks.lower_bound( 1 );
				if( it == s_tasks.end() )
				{
					it = s_tasks.begin();
				}
				pTask = (*it).second;
				s_tasks.erase( it );
			}
			return pTask;
		}

		static void * RunThread( void * )
		{
			while (true)
			{
				try {
					while (true)
					{
						Runnable * pTask = FetchTask( );
						if( pTask )
						{
							STAT_MIN5("DealedTask",1);
							try {
								pTask->Run();
							} catch( ... ) { delete pTask; }
						}

						if( s_ppolicy && 0 == s_ppolicy->OnGetThreadCount() )
						{
							break;
						}
					}
				} catch ( ... ) { continue; }
				break;
			}
			return NULL;
		}
	};

};

};

#endif

namespace GNET
{
namespace Thread
{
	class HouseKeeper : public Timer::Observer
	{
	private:
		typedef std::multimap<int, Runnable*>	TimerTaskQueue;
		TimerTaskQueue	tasks;
		Thread::Mutex	locker_tasks;
		Timer	timer;

		HouseKeeper() : locker_tasks("Thread::HouseKeeper::locker_tasks") { Timer::Attach(this); }
		static HouseKeeper & GetInstance() { static HouseKeeper s_instance; return s_instance; }

		void AddTask( Runnable * pTask, int waitsecs )
		{
			Thread::Mutex::Scoped lock(locker_tasks);
			tasks.insert( std::make_pair(timer.Elapse() + waitsecs, pTask) );
		}

		void Update()
		{
			int nElapse = timer.Elapse();
			Thread::Mutex::Scoped lock(locker_tasks);

			TimerTaskQueue::iterator it = tasks.begin();
			while(it!=tasks.end())
			{
				if(it->first > nElapse)
					break;

				Thread::Pool::AddTask( (*it).second, true );
				tasks.erase( it );
				it = tasks.begin();
			}
		}

	public:
		static void AddTimerTask( Runnable * pTask, int waitsecs )
		{
			GetInstance().AddTask( pTask, waitsecs );
			// IntervalTimer::Schedule( pTask,  waitsecs * 1000000 / IntervalTimer::Resolution() );
		}
	};

};

};


#endif
