/*
	Thread Local Allocator
	类似栈的分配器，但拥有较高的容错性，对于临时使用的内存，可以达到极高的性能和并发能力

	作者： 崔铭  完美时空
	时间： 2010-9-1

	***说明：
	线程局部分配器，是用于临时的内存使用的一种分配器，在一次调用区间内，分配的内存只进行一次使用，不进行重用，在整个调用区间结束后，统一进行回收并确定是否有泄露存在。
	使用例子：
		BeginTLA();
		void *p Alloc(a);
		use p
		free(p, a);
		EndTLA();
	BeginTLA和EndTLA可以嵌套使用，但一个Begin/End对里都必须保证内存被正确的释放，另外，即使嵌套使用，这些所有的Begin/End对都使用一个内存池。只有在最外层的EndTLA调用后，内存池才会统一的回收复位。
	由于在一个BeginTLA/EndTLA区间内，不进行内存重用操作，所以在一次操作中分配的内存数量是有限制的，即累计分配数量不应超过内存池的大小，如果超过，TLA分配器会自动转入标准的CRT分配，正确性依然能够保证，但性能会降低。
		例如：
		BeginTLA();
		void * p = Alloc(a); Free(p,a):
		p = Alloc(a); Free(p,a);
		EndTLA();
	这样会占用双份a大小的内存，而不是一份，因为之前Free掉的内存在EndTLA之前是不会重用的，如果存在嵌套，这个Begin/End不是最外层的 BeingTLA/EndTLA对，在这个EndTLA后，这块内存也不会重用。
	虽然没有重用内存，但是Free依然是必要的，这个Free在 TLA内存池满了之后转入CRT内存分配后是有意义的，同时也进行了内存泄露的统计的工作。



	***性能表现：
		在实际测试中，TLA分配器在内存池不耗尽时，单线程性能为malloc/free的三倍以上。 当进行多线程内存分配时，TLA分配器的性能几乎是完全线性增长的，而标准的分配器和fast_allocator在多线程会面临着较多的锁冲突。
		极端情况下，TLA分配器可以达到malloc/free分配器10倍甚至50倍以上的分配性能。同时，由于没有锁冲突， TLA内存分配可以很平顺的进行分配和释放，从而使延迟表现也会较佳
		同时，由于集中释放和重用，TLA会减少内存碎片的产生，从而提高整体CRT内存管理的效率


	***使用：
		可以使用tla_manager或tla_alloc来进行操作， tla_alloc可以用于hash_map,vector等需要分配器的对象 


	***需要注意的地方：
		1. 自动初始化的功能会比较操作方便，但是在线程数目很多的时候会造成大量的内存耗费，因为每个线程都需要自己独立的内存池
		   1a，如果不想给某些线程分配TLA，而又不确定这些线程是否使用TLA，可以：对于要使用TLA的线程手动调用InitThread， 
		   								       对于不使用TLA线程(即其他线程)，给定一个很小的（比如1）数值，自动初始化TLA，这些线程的内存池就会很容易满而自动转入CRT模式
		   1b, 线程池模块中提供了一个入口函数，可以在每个线程池线程刚开始的时候执行一个函数，这个函数可以用于调用InitThread或其他类似的操作
		   1c，自动初始化时，不使用TLA的线程也就不会分配相应的内存池
		2. 循环分配和释放内存的会造成内存池内存耗尽，而转成较慢的malloc模式，因此应当控制消耗的内存总量，可以用GetOMSize来监控是否发生内存耗尽的情况 ,同时给定合理的内存池大小，建议每个线程2M以上。
		3. BeginTLA()和EndTLA必须配对，不然也会出现错误
		   3a，BeginTLA和EndTLA支持嵌套调用，但不应把嵌套调用成为一种通常的手法，注意只有最外层的EndTLA才会进行内存池的回收操作
		   3b，所有分配的内存依然需要调用释放函数，而且还需要提供分配时的内存大小，这是为了统计内存泄露而用的
		   3c，如下代码会出问题： BeginTLA();vector<int,tla_alloc> abc; use abc; EndTLA()，因为abc的析构调用会在EndTLA()之后
		   3d，所有错误会以assert的方式报错，因此请将测试覆盖到所有代码分支
		   3e，TLA分配的内存的生存期不能超过BeginTLA/EndTLA划定的区间，因此不要长期保存从TLA中分配的对象和内存地址
		   
	
	***例子
		初始化,每个线程内存池为1M字节，使用时自动初始化线程池
		abase::tla_alloc::Init(1024*1024,true);
		使用:
		abase::tla_alloc::BeginTLA();
		{
			abase::vector<int , abase::tla_alloc> list;
			list.push_back(1);

			void *p = abase::tla_alloc::allocate(1024);
			abase::tla_alloc::deallocate(p, 1024);
		}
		abase::tla_alloc::EndTLA();
 */

#ifndef __ABASE_TH_STACKLIKE_ALLOCATOR_H__
#define __ABASE_TH_STACKLIKE_ALLOCATOR_H__

#include <pthread.h>
#include <allocator.h>
namespace abase
{
class tla_mem;
class tla_manager
{	
	pthread_key_t _mem_key;		//for finalize
	size_t _pool_size;
	size_t _auto_pool_size;
	bool _auto_init;
	static void tla_mem_dtr(void * arg);
	
public:
	tla_manager();
	~tla_manager();
	tla_mem * GetAllocator();						//内部使用

	bool Init(size_t pool_size, bool AutoInit = false, size_t auto_pool_size = 1);	//初始化，设置必要的参数，包含每个内存池的大小，是否自动初始化，
											//自动初始化是在每个线程第一次调用BeginTLA时，若此线程没有显示调用过 InitThread则按照auto_pool_size的值来进行初始化工作
	tla_mem * InitThread(size_t init_size = 0);					//初始化线程本地内存池，每个线程只能调用一次，如果传入的参数为0 ，则使用init时传入的pool_size值，否则为特定的内存值 返回值可忽略
	int BeginTLA();									//开始进入TLA内存分配区 返回值是此TLA嵌套了几层了 返回1表示是第一层
	int EndTLA();									//结束TLA内存分配区 返回值是当前还有几层嵌套，返回0表示是没有嵌套了
	size_t GetOMSize();								//返回超出内存池后曾经分配外部内存的大小，可以用来判断是否需要增大内存池大小
	void * Alloc(size_t s);								//分配
	void Free(void * p , size_t size);						//释放
	void * ReAlloc(void *oldp, size_t new_size, size_t old_size);			//重新分配
};

class tla_alloc
{
	static tla_manager _tl_manager;
public:
	//用户需要自己完成这个_manager的初始化，并且自己完成BeginTLA和EndTLA的操作
	static tla_manager & GetManager() { return _tl_manager; }
	static void Init(size_t pool_size, bool auto_pool) { _tl_manager.Init(pool_size, auto_pool, pool_size); }
	static void BeginTLA() { _tl_manager.BeginTLA();}
	static void EndTLA() { _tl_manager.EndTLA();}
public:
	tla_alloc(){}
	tla_alloc(alloc_implement *) {}
	inline static void * allocate(size_t size) {
		return _tl_manager.Alloc(size);
	}
	inline static void  deallocate(void * ptr,size_t size)
	{
		_tl_manager.Free(ptr,size);
	}
	inline static void *reallocate(void *oldptr, size_t newsize, size_t oldsize)
	{
		return _tl_manager.ReAlloc(oldptr, newsize, oldsize);	
	}

	inline void * alloc(size_t size)
	{
		return _tl_manager.Alloc(size);
	}

	inline void free(void * ptr, size_t size)
	{
		_tl_manager.Free(ptr,size);
	}

};
class tla_keeper
{
public:
	tla_keeper(){abase::tla_alloc::BeginTLA();}
	~tla_keeper(){abase::tla_alloc::EndTLA();}
};

class tla_alloc_implement : public alloc_implement
{
public:
	virtual void * alloc(size_t size)
	{
		return tla_alloc::allocate(size);
	}
	virtual void  free(void * ptr, size_t size)
	{
		tla_alloc::deallocate(ptr, size);
	}
};

class default_alloc_implement : public alloc_implement
{
public:
	virtual void * alloc(size_t size)
	{
		return malloc(size);
	}
	virtual void  free(void * ptr, size_t size)
	{
		::free(ptr);
	}
};

}
#endif

