#ifndef __ONLINE_GAME_IOLIB_THREAD_H__
#define __ONLINE_GAME_IOLIB_THREAD_H__
#include "threadpool.h"
#include "mutex.h"
#include "gnet_timer.h"
#include "protocol.h"
#include <map>

namespace GNET
{       
	namespace Thread
	{       
		typedef GNET::Runnable Runnable;
		typedef GNET::Mutex Mutex;
		typedef GNET::RWLock RWLock;
		typedef GNET::Mutex SpinLock;


		class Pool
		{
			public:
			static ThreadPool _pool;
			static int Run()
			{
				return -1;
			}

			static inline void AddTask(Runnable * task, bool)
			{
				_pool.AddTask(task);
			}

			static inline void AddTask(Runnable * task)
			{
				_pool.AddTask(task);
			}
		};

#ifndef ACCURATE_THREAD_CONDITION
		//Condition简化版实现，每次NotifyAll只能唤醒一个
		class Condition
		{
			int _cond;	//条件。 不等于0时表示事件发生
			Condition(const Condition& rhs){ }
			public:
			~Condition ()
			{
				NotifyAll();
				NotifyAll();
				NotifyAll();
			}
			explicit Condition( ): _cond (0){}
			int Wait( Mutex & mutex ); 
			int TimedWait( Mutex & mutex, int nseconds );
			int NotifyOne( );
			int NotifyAll( ) 
			{
				NotifyOne();	//只唤醒一个, 有多少个线程在Wait没有统计。希望上层能够统计一下，然后多调几次NotifyAll
				return 0;
			}
		};
#else
		//复杂版，可以正确对当前等待的线程进行计数
		class Condition
		{
			int _cond;	//条件。 不等于0时表示事件发生
			int _wait_num;	//有多少线程在等, 主要用来实现NotifyAll.
			Mutex _lock;
			Condition(const Condition& rhs){ }
			public:
			~Condition ();
			explicit Condition(): _cond (0),_wait_num(0){}
			int Wait( Mutex & mutex ); 
			int TimedWait( Mutex & mutex, int nseconds);
			int NotifyOne();
			int NotifyAll(); 
		};
#endif
	}

	class ReconnectTask: public Thread::Runnable
	{
		public:
			PManager* manager;

			ReconnectTask(PManager* m,int priority): Runnable(priority),manager(m) {}
			void Run()
			{
				manager->InitClient(manager->GetIOMan());
				delete this;
			}
	};
	class StaticReconnectTask: public Thread::Runnable	//不释放的版本
	{
		public:
			PManager* manager;

			StaticReconnectTask(PManager* m,int priority): Runnable(priority),manager(m) {}
			void Run()
			{
				manager->InitClient(manager->GetIOMan());
			}
	};
}

namespace GNET
{
	namespace Thread
	{
		class HouseKeeper : public Timer::Observer
		{
			private:
				typedef std::multimap<int, Runnable*>   TimerTaskQueue;
				TimerTaskQueue  tasks;
				Thread::Mutex   locker_tasks;
				Timer   timer;
				ThreadPool  *th_pool;

				void Update();
			public:
				//构造函数变为public，是期望可以有多个HouseKeeper,每个threadpool不同，而不再是单件
				HouseKeeper(ThreadPool *pool = &Pool::_pool) : locker_tasks("Thread::HouseKeeper::locker_tasks"), th_pool(pool) { }
				void AddTask( Runnable * pTask, int waitsecs )
				{
					Thread::Mutex::Scoped lock(locker_tasks);
					tasks.insert( std::make_pair(timer.Elapse() + waitsecs, pTask) );
				}
				static HouseKeeper& GetInstance(ThreadPool* p = &Pool::_pool); //根据 threadpool来取Instance，没有的话，会创建一个HouseKeeper对象
				static void AddTimerTask( Runnable * pTask, int waitsecs ) //注意：：这是向默认的ThreadPool所对应的HouseKeeper中添加Task
				{
					GetInstance().AddTask( pTask, waitsecs );
					// IntervalTimer::Schedule( pTask,  waitsecs * 1000000 / IntervalTimer::Resolution() );
				}
		};

	};

};

#endif
