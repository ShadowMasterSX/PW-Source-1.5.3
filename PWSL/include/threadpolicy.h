/* 	
	线程池创建和执行策略定义文件
	作者：杨延昭
	修改：崔铭
	公司：完美时空
	日期：2009-06-08
 */

#ifndef __ONLINEGMAE_COMMON_THREAD_POLICY_H__
#define __ONLINEGMAE_COMMON_THREAD_POLICY_H__

#include "numalib.h"
#include "threadpool.h"
#include <vector>
#include <string.h>
#include <pthread.h>

namespace GNET
{
	class ThreadPolicySingle : public ThreadPolicy
	{
		public:
			virtual void Release() { delete this;}
			virtual int GetGroupCount()  { return 1;}
			virtual int GetSeqCount(int group_index) { if(group_index < 0) return 1; else return 0; }
			virtual void Run(ThreadPool * pool) { _thread_entrance(pool, 0, true, false, true, false);}
			virtual void AddTask(ThreadPool * pool, int arg, Runnable * task) { pool->AddSeqTask(0, task);}
			void RunTick(ThreadPool *pool) {pool->TryProcessAllTask();}
	};


	class ThreadPolicyBase : public ThreadPolicy
	{
		protected:
		struct thread_t 
		{
			bool global;
			bool group;
			bool sequence;
			bool group_sequence;
		};
		struct  group_t
		{
			int cpu_node;
			int seq_count;
			std::vector<thread_t> threads;
		};
		
		std::vector<group_t> _groups;
		int _seq_count;
		public:
			ThreadPolicyBase():_seq_count(0) {}
		
			//建立线程信息的函数
			//加入一个分组， cpu_node是指这个group希望在哪个cpu上跑。cpu_node<0时，参数无效，表示由系统随机分配，没有期望
			//seq_count是指这个分组中，序列通道的数目（有几条跑道， 同一个跑道上的人要按顺序)
			//注意，至少要有一个分组，否则线程池无法运行
			int AddGroup(int cpu_node, int seq_count)
			{
				group_t gt;
				gt.cpu_node = cpu_node;
				gt.seq_count = seq_count;
				_groups.push_back(gt);
				return _groups.size() - 1;
			}

			//加入一个线程。group_index是指这个线程负责哪个group
			//task_type_mask是指这个线程负责哪些任务
			void AddThread(int group_index, unsigned int task_type_mask) /*TASK_TYPE_GLOBAL, TASK_TYPE_GROUP,TASK_TYPE_SEQUENCE, TASK_TYPE_GROUP_SEQ*/
			{
				group_t & gt = _groups[group_index];
				thread_t tt;
				memset(&tt, 0, sizeof(tt));
				if(task_type_mask & TASK_TYPE_GLOBAL) tt.global = true;
				if(task_type_mask & TASK_TYPE_GROUP) tt.group = true;
				if(task_type_mask & TASK_TYPE_SEQUENCE) tt.sequence = true;
				if(task_type_mask & TASK_TYPE_GROUP_SEQ) tt.group_sequence = true;
				gt.threads.push_back(tt);
			}

			//设置实际上的、物理上共有几个“序列通道”。   (相当于体育场上的跑道，实实在在就8个跑道)
			//而seqence number是应用程序产生的“逻辑上的序列号” (相当于运动员手里拿的接力棒号牌，数字可能很大，比如100,101, 2000等等, 没上场的其它组的队伍也算在内）
			//同一逻辑序列号上的Task是顺序执行的。  (接力棒要从一个人手里传到另一个手里后才有效)
			//同一个物理序列通道上的Task也是顺序执行的， (第一小组的第2道， 跟第二小组的第2道， 虽然他们序列号不同接力棒号码不同， 但也要第一小组的跑完，第二组才能上)
			void SetSequenceCount(int count) { _seq_count = count; }
		public:
			//继承的虚函数
			virtual void Release() { delete this;}
			virtual int GetGroupCount() 
			{ 
				return _groups.size();
			}
			virtual int GetSeqCount(int group_index)
			{
				if(group_index < 0) return _seq_count;
				return _groups[group_index].seq_count;
			}

			virtual void Run(ThreadPool * pool)
			{
				for(size_t i =0; i < _groups.size(); i ++)
				{
					for(size_t j = 0;j < _groups[i].threads.size(); j ++)
					{
						thread_node_t * node = new thread_node_t(this, pool, i, j);
						pthread_t th;
						pthread_create(&th,NULL, GetThreadEntrance(),node);
					}
				}
			}
		protected:
			typedef void *(*THREAD_ENTRANCE)(void*);
			virtual THREAD_ENTRANCE GetThreadEntrance() { return ThreadEntrance;}
			struct thread_node_t
			{
				ThreadPolicyBase * __this;
				ThreadPool * pool;
				int group;
				int thread;
				thread_node_t(ThreadPolicyBase * tp, ThreadPool * p, int g, int t):__this(tp),pool(p), group(g),thread(t) {}
			};
			static void * ThreadEntrance(void * param)
			{
				pthread_detach(pthread_self());
				thread_node_t * node = (thread_node_t*)param;
				ThreadPolicyBase * __this = node->__this;
				ThreadPool * pool = node->pool;
				int group_index = node->group;
				thread_t & th = __this->_groups[node->group].threads[node->thread];
				delete node;
				
				if(__this->_thread_init) __this->_thread_init();	//进行线程入口初始化
				return __this->_thread_entrance(pool,  group_index, th.group, th. group_sequence, th.global, th.sequence);
			}
			#ifdef __OLD_IOLIB_COMPATIBLE__
			//Policy的AddTask的标准实现是pool->AddTask, 但在兼容模式下，Manager收到了“优先级不为0”的协议，会调用此接口添加处理协议，
			//如果仍然是向全局的Task队列中加，则处理协议的顺序可能是乱的，与“优先级为100,101的协议需顺序处理”的要求不符。
			//为了让旧的项目不必修改rpcalls.xml，也不必写自己的threadpoolpolicy，需重写这个AddTask. 
			virtual void AddTask(ThreadPool * pool ,int arg,  Runnable * task)
			{
				int prior = arg;
				if (prior == 100 || prior == 101 || prior ==0) //旧项目中使用100,101表示顺序处理，0代表立即被pollio线程处理.
					pool->AddSeqTask(prior, task);
				else
					pool->AddTask(task);
			}
			#endif
	};

	class ThreadPolicyNuma : public ThreadPolicyBase
	{
		int _node_num;

		virtual void * GroupMalloc(int group_index, size_t size) {
			return Numa::Memory::Alloc(group_index % _node_num, size);
		}
		virtual void GroupFree(int group_index, void * buf, size_t size) 
		{
			Numa::Memory::Free(buf, size);
		}
		static void NewEntrance(PoolEntrance ent, ThreadPool * pool, int group_index, thread_t th, void * tmp)
		{
			ent(pool, group_index, th.group, th. group_sequence, th.global, th.sequence);
		}
		static void * ThreadEntranceNuma(void * param)
		{
			pthread_detach(pthread_self());
			thread_node_t * node = (thread_node_t*)param;
			ThreadPolicyNuma * __this = (ThreadPolicyNuma*)node->__this;
			ThreadPool * pool = node->pool;
			group_t & group = __this->_groups[node->group];
			thread_t & th = group.threads[node->thread];

			if(group.cpu_node >= 0)
			{
				int node_index = group.cpu_node % __this->_node_num;
				Numa::Thread::SetAffinityByNode(node_index);
				Numa::Thread::SetPolicy(Numa::POLICY_PREFERRED, node_index);
				char * tmp = (char*)alloca(4096);
				memset (tmp, 0, 4096);
				if(__this->_thread_init) __this->_thread_init();	//进行线程入口初始化
				NewEntrance(__this->_thread_entrance, pool, node->group, th, tmp);
			}
			else
			{
				if(__this->_thread_init) __this->_thread_init();	//进行线程入口初始化
				__this->_thread_entrance(pool,  node->group , th.group, th. group_sequence, th.global, th.sequence);
			}
			delete node;
			return NULL;
		}

		virtual THREAD_ENTRANCE GetThreadEntrance() { return ThreadEntranceNuma;}
	public:
		ThreadPolicyNuma()
		{
			_node_num = Numa::Thread::GetNodeNum();
		}
	};

}
 
#endif

