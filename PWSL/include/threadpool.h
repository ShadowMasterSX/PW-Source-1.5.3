/* 
	线程池模块， 这里实现了一个灵活可配置的线程池模块，提供了较好的性能，同时也加入了对非一致内存存取架构的支持。
	作者：崔铭
	修改：杨延昭
	公司：完美时空
	日期：2009-06-08
 */

#include <stdlib.h>
#include <stdio.h>
#ifndef __ONLINEGAME_COMMON_THREAD_H
#define __ONLINEGAME_COMMON_THREAD_H

namespace GNET
{

	class Runnable
	{
		int m_priority;
	public:
		Runnable( int priority = 0 ) : m_priority(priority) { }
		virtual ~Runnable() {}
		virtual void Run() = 0;

		int GetPriority() const { return m_priority; }
		void SetPriority( int priority ) { m_priority = priority; }
	};

	enum
	{
		TASK_TYPE_GLOBAL 	= 0x0001,
		TASK_TYPE_GROUP	 	= 0x0002,
		TASK_TYPE_SEQUENCE 	= 0x0004,
		TASK_TYPE_GROUP_SEQ 	= 0x0008,
	};

	class ThreadPool;
	class ThreadPolicy
	{
		public:
			typedef void *(*PoolEntrance)(ThreadPool *__this, int group_index, bool group, bool group_seq, bool global, bool global_seq);
			typedef void (*ThreadInit)();
		protected:
			PoolEntrance _thread_entrance;
			ThreadInit  _thread_init;
			
		public:

			ThreadPolicy():_thread_entrance(NULL),_thread_init(NULL) {}
			virtual ~ThreadPolicy() {}

			virtual void SetPoolEntrance(PoolEntrance  entrance) { _thread_entrance = entrance;}	//此函数由ThreadPool调用
			virtual void * GroupMalloc(int group_index, size_t size) { return malloc(size);}
			virtual void GroupFree(int group_index, void * buf,size_t size) { return free(buf);}
			virtual void AddTask(ThreadPool * pool ,int arg,  Runnable * task);	//标准实现是调用AddTask
			virtual void * PoolMalloc(size_t size)  {return malloc(size);}
			virtual void PoolFree(void * buf) {free(buf);}

			virtual void Release() = 0;
			virtual int GetGroupCount() = 0;
			virtual int GetSeqCount(int group_index) = 0;		//-1 for global sequence 
			virtual void Run(ThreadPool * pool) = 0;
	};

	class ThreadPool
	{
		void * _imp;
		ThreadPolicy* _policy;

		static void *WorkThread(ThreadPool * __this, int group_index, bool group, bool group_seq, bool global, bool global_seq);
	public:
		ThreadPool();
		~ThreadPool();
		bool Init(ThreadPolicy *p);
		bool Start();
		void Stop();		//Stop之后线程们不会立刻结束，如果需要等待结束，则调用下面的WaitStop
		void WaitStop();
		int TryProcessAllTask();	//试图一次性处理完所有的Task然后返回。（某个顺序任务被其它线程执行时，无法拿到该顺序的其它任务)
	public:
		void AddTask(Runnable * task);
		void AddGroupTask(size_t group_index, Runnable * task);
		void AddSeqTask(size_t seq_number, Runnable * task);
		void AddGroupSeqTask(size_t group_index, size_t seq_number, Runnable * task);		//由于初始化顺序的原因， 在没有Start前，这个调用会自动转移到 AddSeqTask或者AddTask中
		void CreateThread(void *(*function)(void*), void * param);	//创建一个单独的执行线程
		inline void PolicyAddTask(int arg, Runnable * task) { _policy->AddTask(this, arg, task); }

		bool IsIdle();
		void dump(FILE *file = stdout);
		inline bool IsInitialized() const {return _imp && _policy;}
	};

}

#endif

