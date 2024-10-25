/*
	定时器模块， 提供了一个高效的定时器功能；
	作者：崔铭
	公司：完美时空
	时间：2002-4-1
*/


#ifndef __CMLIB_TIMER_H__
#define __CMLIB_TIMER_H__

#include <sys/time.h>
#include <stdint.h>
#include "ASSERT.h"

namespace abase
{
typedef void (*timer_callback)(int index,void *object,int remain);

class  timer
{
	void * __imp;
public:
	enum 
	{
		TICKTIME = 50000,
		MINTIME = 10000
	};
	/*
		初始化一个timer对象,一共有两个参数
		1:idx_tab_size 表示的所有timer对象里索引表的大小, 
			索引表提供了快速查询的能力，使得每次timer事件发生时无需再进行复杂查询
			这个索引表的大小表明了要缓冲多少个tick的timer事件，
			这个值越大，那么就可以对越大事件范围的定时器进行缓冲
		2:max_timer_count 表示最大能够同时容许多少个timer	
			每个额外的timer entry会多消耗44（32位系统）字节内存
	*/
	timer(int idx_tab_size,int max_timer_count);	
	~timer();


	uint64_t get_tick();
	long 	get_systime();				//同下
	void 	get_systime(timeval & tv);		//以timer的时间精度来取得系统时间，对于不精确的要求，可以使用这个函数，可以避免系统调用
	int 	get_free_timer_count();
	int64_t	get_timer_total_alloced();
	int 	set_timer(int interval,int start_time,int times,timer_callback routine, void * obj);
	/*
	 *	remove_timer
	 * 	删除一个timer，会在下一次tick时删除，即下一个tick此定时器才能被重新应用
	 *	本次tick仍然可能会被扫描到这个定时器，
	 *	但是其timer的callback并不会被调用,因此不会出问题 
	 *	注意：不要在callback或者OnTimer中调用此函数删除自身,如果要删除，调用另外一个函数 callback_remove_self
	 */
	int 		remove_timer(int index,void *object);
	//此函数只能在callback或者OnTimer里调用并用来删除自身
	int		callback_remove_self(int index);		
	int		get_next_interval(int index,void * obj, int * interval, int *rtimes);
	int		change_interval(int index ,void * obj, int interval,bool nolock/* in callback*/);
	int		change_interval_at_once(int index ,void * obj, int interval,bool nolock/* in callback*/);
	void 		timer_thread(int ticktime = TICKTIME,int mintime  = MINTIME);	//时间线程应该调用的函数 这个函数不会返回，除非其他线程调用了stop_thread
	void 		stop_thread();							//stop之后不要立刻重新开始,等待一段时间才能真正退出
	void		pause_thread();	
	void		resume_thread();
	void		prepare_for_single_thread(int ticktime = TICKTIME, int mintime = MINTIME);
	void 		single_turn();	//单线程模式下 应该调用的函数 这个调用会检查是否发生时间轮转，并且调用相应的触发函数
	void		reset();	// reset timer
	void		timer_tick();	// timer tick 

};

class timer_task
{
	virtual bool OnTimer2(int index,int rtimes) { OnTimer(index, rtimes); return true;}
	virtual void OnTimer(int index,int rtimes) = 0;		//返回值代表是否删除自身， 如果是true则未删除 
	inline void DoTimer(int index,int rtimes)
	{
		int tdx = _timer_index;
		timer * tm = _tm;
		if(OnTimer2(index,rtimes))
		{
			if(rtimes == 0)
			{
				//定时器被自动删除
				_timer_index = -1;
				_tm = NULL;
			}
		}
		else
		{
			if(rtimes != 0) tm->callback_remove_self(tdx);
		}
	}
	static void _callback(int index,void *object,int rtimes)
	{
		((timer_task*)object)->DoTimer(index,rtimes);
	}

protected:

	int _timer_index;
	timer * _tm;
	timer_task():_timer_index(-1),_tm(NULL){}
	virtual  ~timer_task()
	{
		ASSERT(_timer_index == -1 && _tm == NULL);
	}
	int RemoveSelf()
	{
		int rst = _tm->callback_remove_self(_timer_index);
		_timer_index = -1;
		_tm = NULL;
		return rst;
	}

	int ChangeInterval(int new_interval)
	{
		ASSERT(new_interval > 0);
		if(_timer_index < 0) return -1;
		int rst = _tm->change_interval(_timer_index,this,new_interval,false);
		ASSERT(rst == 0);
		return rst;
	}

	int ChangeIntervalInCallback(int new_interval)
	{
		ASSERT(new_interval > 0);
		int rst = _tm->change_interval(_timer_index,this,new_interval,true);
		ASSERT(rst == 0);
		return rst;
	}
	
public:

	int SetTimer(timer & tm, int interval,int times,int start_time = -1)
	{
		if(_timer_index != -1 &&  _tm != NULL )
		{
			ASSERT(false && "同样一个对象重复的加入到了定时器中");
			return -1;
		}
		_tm = & tm;
		_timer_index = tm.set_timer(interval,start_time,times,_callback,this);
		return _timer_index;
	}

	int RemoveTimer()
	{
		timer * __tm = _tm;
		int __index = _timer_index;
		if(!__tm || __index < 0) return -1;
		int tmp = __tm->remove_timer(__index,this);
		_timer_index = -1;
		_tm = NULL;
		return tmp;
	}

	inline int GetTimerIndex() { return _timer_index; }

};

}
#endif

