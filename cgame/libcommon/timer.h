#ifndef __CMLIB_TIMER_H__
#define __CMLIB_TIMER_H__

#include "spinlock.h"
#include <sys/time.h>

extern int g_use_old_move_policy;

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
			每个额外的timer entry会多消耗44（暂时）字节内存
	*/
	timer(int idx_tab_size,int max_timer_count);	
	~timer();


	unsigned int 	get_tick();
	long 	get_systime();
	void 	get_systime(timeval & tv);
	int 	get_free_timer_count();
	int 	get_timer_total_alloced();
	int	get_timer_left(int index,int & interval);
	int 	set_timer(int interval,int start_time,int times,timer_callback routine, void * obj);
	/*
	 * 	删除一个timer，会在下一次tick时删除，即下一个tick此定时器才能被重新应用
	 *	本次tick仍然可能会被扫描到这个定时器，
	 *	但是其timer的callback并不会被调用,因此不会出问题 
	 *	注意：不要在callback或者OnTimer中调用此函数删除自身,如果要删除，调用另外一个函数
	 */
	int 		remove_timer(int index,void *object);
	//此函数只能在callback或者OnTimer里调用并用来删除自身
	int		callback_remove_self(int index);		
	int		get_next_interval(int index,void * obj, int * interval, int *rtimes);
	int		change_interval(int index ,void * obj, int interval,bool nolock/* in callback*/);
	int		change_interval_at_once(int index ,void * obj, int interval,bool nolock/* in callback*/);
	void 		timer_thread(int ticktime = TICKTIME,int mintime  = MINTIME);
	void 		stop_thread();	//stop之后不要立刻重新开始,等待一段时间才能真正退出
	void		pause_thread();	
	void		resume_thread();	
	void		reset();	// reset timer
	void		timer_tick();	// timer tick 

};

class timer_task
{
	virtual void OnTimer(int index,int rtimes) = 0;
	inline void DoTimer(int index,int rtimes)
	{
	/*
		if(g_use_old_move_policy)
		{
			if(rtimes == 0)
			{
				//定时器被自动删除
				extern void _t_RecordTimer(int , timer_task *,int,int);
				_t_RecordTimer(_timer_index, this, 2, 0);
				_timer_index = -1;
				_tm = NULL;
			}
		}
		OnTimer(index,rtimes);

		if(!g_use_old_move_policy)
		{
			//为新线路使用新的逻辑，测试一下有否副作用 $$$$$$$$$$$
			if(rtimes == 0)
			{
				//定时器被自动删除
				extern void _t_RecordTimer(int , timer_task *,int,int);
				_t_RecordTimer(_timer_index, this, 2, 0);
				_timer_index = -1;
				_tm = NULL;
			}
		}
		*/
		OnTimer(index,rtimes);
		if(rtimes == 0)
		{
			//定时器被自动删除
			_timer_index = -1;
			_tm = NULL;
		}
	}
protected:

protected:

	int _timer_index;
	timer * _tm;
	timer_task():_timer_index(-1),_tm(NULL){}
	virtual  ~timer_task()
	{
		//ASSERT(_timer_index == -1 && _tm == NULL);
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
	
	int GetTaskData(int &interval, int & rtimes)
	{
		return _tm->get_next_interval(_timer_index, this,&interval,&rtimes);
	}

public:

	int SetTimer(timer & tm, int interval,int times,int start_time = -1)
	{
		//$$$$$$$$ 这里没有限制 interval 不能小于等于 0
		if(_timer_index != -1 &&  _tm != NULL )
		{
			ASSERT(false && "同样一个对象重复的加入到了定时器中，或者tag没有赋值");
			return -1;
	
		}
		_tm = & tm;
		_timer_index = tm.set_timer(interval,start_time,times,_callback,this);
		//extern void _t_RecordTimer(int , timer_task *,int,int);
		//_t_RecordTimer(_timer_index, this, 0,_timer_index);
		return _timer_index;
	}

	int RemoveTimer()
	{
		timer * __tm = _tm;
		int __index = _timer_index;
		if(!__tm || __index < 0) return -1;
		int tmp = __tm->remove_timer(__index,this);
		//extern void _t_RecordTimer(int , timer_task *,int,int);
		//_t_RecordTimer(__index, this, 1, tmp);
		_timer_index = -1;
		_tm = NULL;
		return tmp;
	}
	inline int GetTimerIndex()
	{
		return _timer_index;
	}


	static void _callback(int index,void *object,int rtimes)
	{
		((timer_task*)object)->DoTimer(index,rtimes);
	}
};

extern int g_delta_time;

}
#endif

