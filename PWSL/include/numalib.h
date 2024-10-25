#ifndef __NUMA_LIB_H__
#define __NUMA_LIB_H__

namespace Numa
{
	enum {
		POLICY_LOCAL,		//本地分配(总在当前CPU所对应的NODE上分配)
		POLICY_PREFERRED,	//优先使用某个NODE, 如果分配失败，选择其它的NODE
		POLICY_INTERLEAVE,	//交叉、轮流分配
		POLICY_BIND,		//严格绑定到NODE上分配，如果分配失败，则调用失败
	};

	enum{
		MAX_CPU = 128,		//这个库中假定的最大CPU数
		MAX_NODE = 32,		//这个库假定的最大NODE数
	};

	/**********和系统物理相关的函数 *********/
	namespace System {
		/**
		 * @brief 获得系统中的CPU数
		 *
		 * @return 获取CPU个数
		 * @note   CPU的索引为0,1,...N-1
		 */
		int GetCpuNum();

		/**
		 * @brief 获取系统中的NODE数
		 *
		 * @return node数
		 * @note 对于非NUMA结构的系统，Node返回1.
		 *	 当返回多于1个时，node的索引就是0,1,..N-1
		 */
		int GetNodeNum();

		/**
		 * @brief 获取某个NODE所关联的CPU
		 *
		 * @param node   Numa内存结点索引
		 * @param cpu	用于接受cpu号的起始地址
		 * @param maxcpu 可以接受多少个cpu
		 *
		 * @return 	cpu数目, 不超过maxcpu
		 */
		int GetCpuOnNode(int node, int *cpu, int maxcpu); //使用同一个node的cpu列表

		/**
		 * @brief 获得某NUMA结点的信息
		 *
		 * @param node  Numa结点索引
		 * @param total 用于接收结点总共大小的指针
		 * @param free  用于接收结点空闲空间的指针
		 */
		void GetNodeSize(int node, size_t *total, size_t *free); //node的大小
		/**
		 * @brief 获得某NUMA结点的信息64位
		 *
		 * @param node  Numa结点索引
		 * @param total 用于接收结点总共大小的指针
		 * @param free  用于接收结点空闲空间的指针
		 */
		void GetNodeSize64(int node, long long *total, long long *free); //node的大小
	};

	/*********当前线程相关的函数 ********/
	namespace Thread
	{
		//获得当前线程正在哪个CPU上跑, 未实现
		int GetCurrentCpu();

		/**
		 * @brief 获得当前线程在多少个CPU上跑
		 * 	  SetAffinity()会影响这个数目
		 * @return cpu个数
		 */
		int GetCpuNum();

		/**
		 * @brief 当前线程期望在哪些CPU上跑
		 *
		 * @param cpu 	  用于接受cpu的缓存地址
		 * @param maxcpu  最多可接受CPU的个数
		 *
		 * @return 	cpu数目, 不超过maxcpu
		 */
		int GetAffinity(int *cpu, int maxcpu);

		/**
		 * @brief  设置期望在哪些CPU上跑
		 *
		 * @param cpu	   cpu号的缓存起始地址
		 * @param cpu_num  cpu个数
		 *
		 * @return  0 -- success  <0 -- failed
		 * @note  可能会引起调度,线程迁移到另一个CPU上
		 */
		int SetAffinity(int *cpu, int cpu_num);	//sched_setaffinity

		/**
		 * @brief   设置期望在哪个NODE关联的CPU上跑
		 *
		 * @param node	  Numa的内存结点索引
		 * @return  0 -- success <0 --failed
		 * @note  当系统中只有一个node时，会绑定到所有CPU上。
		 */
		int SetAffinityByNode(int node);

		/**
		 * @brief 当前线程使用的NODE个数
		 *
		 * @return node个数, 这个数大于等于1
		 * @note 对于使用默认policy local的，返回系统中的Node总数
		 */
		int GetNodeNum();

		/**
		 * @brief 	获取该线程使用的NODE列表
		 *
		 * @param nodes		用于存放node的缓冲区地址
		 * @param maxnode	缓冲区中最大存放的node个数
		 *
		 * @return 	Node个数.   nodes中将被填入node索引.
		 * @note	如果当前线程POLICY是 local,则返回系统中所有结点
		 */
		int GetNode(int *nodes, int maxnode);
		/** 
		 * @brief 获取当前线程的NUMA策略
		 * 
		 * @param policy  用以存放NUMA内存分配策略, 可以为POLICY_LOCAL, POLICY_PREFERRED,
				  POLICY_INTERLEAVE, POLICY_BIND
		 * @param nodes		用于存放node的缓冲区地址
		 * @param maxnode	缓冲区中最大存放的node个数
		 * 
		 * @return 	node个数。　<=0时表示调用失败
				policy中将被填入策略。nodes中被填入node索引
		 */
		int GetPolicy(int *policy, int *nodes, int maxnode) ; //get_mempolicy

		/** 
		 * @brief 设置当前线程的NUMA策略
		 * 
		 * @param policy  NUMA内存分配策略, 可以为POLICY_LOCAL, POLICY_PREFERRED,
				  POLICY_INTERLEAVE, POLICY_BIND
		 * @param nodes		所期望绑定的nodes
		 * @param node_num	所期望绑定的node个数。当policy为local时，nodes,nodes_num被忽略
		 * 
		 * @return  0 -- success <0 --failed
		 */
		int SetPolicy(int policy, int *nodes, int node_num); //set_mempolicy

		/** 
		 * @brief 设置当前线程的NUMA策略
		 * 
		 * @param policy  NUMA内存分配策略, 可以为POLICY_LOCAL, POLICY_PREFERRED,
				  POLICY_INTERLEAVE, POLICY_BIND
		 * @param node		所期望绑定的node
		 * 
		 * @return  0 -- success <0 --failed
		 */
		int SetPolicy(int policy, int node);
	};

	/**********内存区域相关的函数 *********/
	namespace Memory
	{
		/** 
		 * @brief 	获取当前进程中，某个内存地址所使用的NUMA策略
		 * 
		 * @param policy   用于存放policy的指针
		 * @param node		用于存放node索引的地址
		 * @param maxnode	node索引的最大容量
		 * @param mem		所要检查的内存地址
		 * 
		 * @return 	node个数。　<=0表示失败
		 */
		int GetPolicy(int *policy, int *node, int maxnode, const void *mem);

		/** 
		 * @brief 设置某块内存区域的NUMA策略
		 * 
		 * @param policy  NUMA内存分配策略, 可以为POLICY_LOCAL, POLICY_PREFERRED,
				  POLICY_INTERLEAVE, POLICY_BIND
		 * @param nodes		所期望绑定的nodes
		 * @param node_num	所期望绑定的node个数。当policy为local时，nodes,nodes_num被忽略
		 * @param mem		内存起始地址
		 * @param size		内存长度
		 * 
		 * @return  0 -- success <0 --failed
		 */
		int SetPolicy(int policy, const int *node, int nodenum, void *mem, size_t size);

		/** 
		 * @brief 	在某个特定NODE上分配一定大小的内存
		 * 
		 * @param node	Numa节点
		 * @param size  所申请的内存大小
		 * 
		 * @return 	所分配的内存。NULL时分配失败
		 * @note	一般情况下，不需要使用这个接口，这里的分配事实上是mmap了一段内存然后将
				它绑定到该node上.
				分配的内存必须由Free(void *mem, size_t size)来释放
		 */
		void *Alloc(int node, size_t size);

		/** 
		 * @brief 释放由Alloc分配的内存
		 * 
		 * @param mem	由Alloc分配的内存地址
		 * @param size	长度
		 */
		void Free(void *mem, size_t size);
	};
};

#endif
