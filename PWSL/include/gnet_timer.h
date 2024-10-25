/*
		保持了原有IO库的Timer  保持了原有IO库 IntervalTimer的部分逻辑和接口，ItervalTimer的实现使用了abase::timer来完成
		作者：未知
		修改：2009-6-17 崔铭 内容见标题
		公司：完美时空
*/


#ifndef __TIMER_H
#define __TIMER_H

#include <vector>
#include <functional>
#include <algorithm>
#include <time.h>
#include <sys/time.h> 
#include "timer.h"
#include "ASSERT.h"
#include "threadpool.h"

namespace GNET
{

class Timer
{
public:
	class Observer
	{
	public:
		virtual ~Observer() { }
		virtual void Update() = 0;
	};
private:
	static time_t now;
	typedef std::vector<Observer*> Observers;
	static Observers& observers();
	time_t t;
	static struct timeval now_tv;
	timeval tv;
public:
	Timer() : t(now) 
	{ 
		if(!now) now = t = time(NULL); 
		if ( !timerisset(&now_tv) )
		{
			gettimeofday(&now_tv,NULL);
			tv.tv_sec=now_tv.tv_sec;
			tv.tv_usec=now_tv.tv_usec;
		}
	}

	static void Attach(Observer *o) { observers().push_back(o); }
	static void Detach(Observer *o) { observers().erase( std::remove(observers().begin(), observers().end(), o), observers().end()); }
	static void Update() 
	{
		time_t tmp = time(NULL);
		if (tmp > now)
		{
			now = tmp;
			gettimeofday( &now_tv,NULL );
			std::for_each(observers().begin(), observers().end(), std::mem_fun(&Observer::Update));	
		}
	}
	static time_t GetTime() { return now; }
	static timeval GetTime_tv() { return now_tv; }
	int Elapse() const { return now - t; }
	int Elapse_ms() const { return (now_tv.tv_sec - tv.tv_sec)*1000 + (now_tv.tv_usec - tv.tv_usec)/1000; }

	struct timeval Elapse_tv() const
	{
		timeval tmp;
		tmp.tv_sec = now_tv.tv_sec- tv.tv_sec -  ( now_tv.tv_sec >= tv.tv_sec ? 0 : 1 );
		tmp.tv_usec= now_tv.tv_usec-tv.tv_usec + ( now_tv.tv_usec>= tv.tv_usec ? 0 : 1000000 );
		return tmp;
	}
	void Reset() 
	{ 
		t = now; 
		tv.tv_sec=now_tv.tv_sec;
		tv.tv_usec=now_tv.tv_usec;
	}
};

class IntervalTimer
{
public: 
	enum {DEFAULT_INTERVAL = 100000};
	class Observer : private abase::timer_task
	{
		size_t _interval;
		bool _in_timer;
		bool _wait_delete;
		virtual bool OnTimer2(int index, int rtimes)
		{
			
			_in_timer = true;
			bool add = Update();
			if(!add && !_wait_delete) RemoveSelf();	//Update返回false并且没有调用ReleaseInUpdate时, 移除自己
			_in_timer = false;
			if(_wait_delete) 
			{
				RemoveSelf();	//当Update中调用了Release或ReleaseInUpdate时，无论返回是true还是false,移除自己
				delete this;
				return true;
			}
			return true;
		}
		virtual void OnTimer(int index,int rtimes){}
		friend class IntervalTimer;
	protected:
		void ReleaseInUpdate()
		{
			ASSERT(_in_timer && "ReleaseInUpdate函数只能在Update函数中调用");
			_wait_delete = true;
		}

		virtual ~Observer() { ASSERT(!_in_timer && "Update中删除自身请调用ReleaseInUpdate函数");}
	public:
		Observer():_interval(0), _in_timer(false),_wait_delete(false)
		{}

		virtual bool Update() = 0;
		virtual size_t GetInterval() { return _interval;}

		void ReleaseTimer()
		{
			if (_timer_index != -1 && _tm != NULL)
			{
				RemoveTimer();
			}
		}

		void Release()
		{
			if (_in_timer)
			{
				ReleaseInUpdate();
			}
			else
			{
				ReleaseTimer();
				delete this;
			}
		}
	};
public:
	class TimerTask : public Observer
	{
		Runnable * runnable;
	public: 
		TimerTask(Runnable* r) : runnable(r){}
		bool Update()
		{
			_pool->PolicyAddTask(-1, runnable);
			ReleaseInUpdate();
			return false;
		}

	};
	
private:
	static void AttachObserver(Observer* o)
	{
		o->SetTimer(*_tm, o->_interval,0);
	}

	struct timeval start;
	static unsigned int _interval;
	static abase::timer * _tm;
	static ThreadPool * _pool;

	static void * timer_thread( void *);
public:
	IntervalTimer() { gettimeofday(&start, NULL); }
	
	static int Resolution() { return _interval;}
	static void Attach( Observer * o, size_t delay)	//delay是指delay多少个tick
	{
		o->_interval = delay>0?delay:1;
		AttachObserver(o);
	}

	static void AddTimer(Observer* o, int sec)  // 加一个sec秒后触发的定时器
	{ 
		o->_interval = sec*1000000/_interval;
		AttachObserver(o);
	}

	static void Attach(Observer* o)  // 1 秒
	{ 
		o->_interval = 1000000/_interval;
		AttachObserver(o);
	}

	static void Schedule(Runnable *task, size_t delay)
	{
		Attach(new TimerTask(task), delay);
	}

	static bool PrepareTimer(abase::timer * tm, size_t usec=0, ThreadPool *pool = NULL);
	static void UpdateTimer()
	{	
		//仅用于单线程模式
		//多线程模式请自己在外面创建一个线程调用 _tm的 timer_thread
		//建议在每次Poll之后调用本Update 
		_tm->single_turn();
	}

#ifdef __OLD_IOLIB_COMPATIBLE__
	static void StartTimer(int interval_time, bool single_thread_mode, int idx_tab_size = 3000, int max_timer_count = 300000)
	{
		ASSERT(single_thread_mode && "多线程下请在外面初始化gtimer并调用PrepareTimer和StartTimerThread");	//多线程下不应使用这种方式初始化	
		PrepareTimer(new abase::timer(idx_tab_size, max_timer_count), interval_time);
	}

#endif
	static void StartTimerThread()
	{
		ASSERT(_tm);
		//这里负责启动 timer的线程， 这也是为了兼容性考虑的处理
		_pool->CreateThread( timer_thread, _tm);
	}

	static void StopTimer() 
	{
		//仅仅为多线程准备的函数
		_tm->stop_thread();
	}

	static void GetTime(struct timeval* val)
	{
		_tm->get_systime(*val);
	}

	static int64_t GetTick() { return _tm->get_tick();}

public:
	int64_t UElapse() const 
	{
		struct timeval now;
		gettimeofday(&now, NULL);
		return (int64_t)(now.tv_sec - start.tv_sec)*1000000LL + now.tv_usec - start.tv_usec;
	}

	int Elapse() const 
	{
		struct timeval now;
		gettimeofday(&now, NULL);
		return now.tv_sec - start.tv_sec;
	}

	void Reset() { gettimeofday(&start, NULL); }
};

/*
	itimer使用方式；
	初始化abase::timer tm;
	ItervalTimer::PrepareTimer(&tm ,ticktime);

	然后：单线程
		隔一段时间就调用 ItervalTimer::UpdateTimer ，可以在每次Poll之后调用

	      多线程
	      	创建一个线程调用 tm.timer_thread(ticktime, minticktime)

*/


}

#endif

