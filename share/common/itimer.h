#ifndef __ITIMER_H
#define __ITIMER_H

#include <signal.h>
#include <pthread.h>
#include <sys/time.h>
#include <map>

#include "thread.h"

namespace GNET
{
#if !defined _REENTRANT_
#define SuspendTimer IntervalTimer::Suspend
#define ResumeTimer  IntervalTimer::Resume
#define UpdateItimer IntervalTimer::UpdateTimer
#else
#define SuspendTimer() 
#define ResumeTimer() 
#define UpdateItimer()
#endif

#define DEFAULT_INTERVAL  100000

class IntervalTimer : public Thread::Runnable{
public:
	class Observer
	{
		friend class IntervalTimer;
		size_t interval;
	public:
		virtual ~Observer() {}
		virtual bool Update() = 0;
		virtual size_t GetInterval() { return interval;}
	};

private:
	typedef std::multimap<int64_t, Observer*> Observers;
	class TimerTask : public Observer
	{
		Thread::Runnable* runnable;
	public: 
		TimerTask(Thread::Runnable* r) : runnable(r){}
		bool Update()
		{
			Thread::Pool::AddTask(runnable, true);
			delete this;
			return false;
		}
	};

	static bool stop;
	static bool triggered; 
	static size_t interval;
	static Thread::Mutex locker;
	static Observers observers;
	static struct timeval now;
	static struct timeval base;
	static int64_t tick_now;
	static IntervalTimer instance;
	static sigset_t mask;

	struct timeval start;

	void Run()
	{
#if defined _REENTRANT_
		sigset_t sigs;

		sigfillset(&sigs);
		pthread_sigmask(SIG_BLOCK, &sigs, NULL);

		struct itimerval value;
		value.it_interval.tv_sec = interval/1000000;
		value.it_interval.tv_usec = interval%1000000;
		value.it_value.tv_sec = interval/1000000;
		value.it_value.tv_usec = interval%1000000;
		setitimer(ITIMER_REAL, &value, NULL);

		sigfillset(&sigs);
		sigdelset(&sigs, SIGALRM);
		while(!stop){
			sigsuspend(&sigs);
			Update();
		}
		setitimer(ITIMER_REAL, NULL, NULL);
#endif
	}
public:
	static void Update()
	{
		Observers::iterator it;
		bool add;
		{
			Thread::Mutex::Scoped lock(locker);
			gettimeofday(&now, NULL);
			tick_now = ((int64_t)(now.tv_sec - base.tv_sec)*1000000LL + now.tv_usec - base.tv_usec)/interval;
			it = observers.begin();
		}

		while(it!=observers.end()){
			if(it->first > tick_now)
				break;
			add = it->second->Update();

			{
				Thread::Mutex::Scoped lock(locker);
				if(add)
					AttachObserver(it->second);
				observers.erase(it);
				it = observers.begin();
			}
		}
	}
private:
	static void AttachObserver(Observer* o)
	{
		if(!stop){
			observers.insert(Observers::value_type(tick_now + o->interval, o));
		}
	}

public:
	IntervalTimer() : start(now){}
	static int Resolution() { return interval; }

	static void Attach(Observer* o, size_t delay)  // delay的单位是tick
	{ 
		delay = delay>0?delay:1;
		o->interval = delay;
		Thread::Mutex::Scoped lock(locker);
		AttachObserver(o);
	}
	static void AddTimer(Observer* o, int sec)  // 加一个sec秒后触发的定时器
	{ 
		o->interval = sec*1000000/interval;
		Thread::Mutex::Scoped lock(locker);
		AttachObserver(o);
	}

	static void Attach(Observer* o)  // 1 秒
	{ 
		o->interval = 1000000/interval;
		Thread::Mutex::Scoped lock(locker);
		AttachObserver(o);
	}

	static void Schedule(Runnable *task, size_t delay)
	{
		Attach(new TimerTask(task), delay);
	}

	static bool StartTimer(size_t usec=0) 
	{ 
		if(usec==0)
			usec = DEFAULT_INTERVAL;
		if(!stop)
			return false;
		stop = false; 
		interval = usec;

#if !defined _REENTRANT_
		sigemptyset(&mask);
		sigaddset(&mask, SIGALRM);
		sigprocmask(SIG_BLOCK, &mask, NULL);
		signal(SIGALRM, Handler);

		struct itimerval value;
		value.it_interval.tv_sec = interval/1000000;
		value.it_interval.tv_usec = interval%1000000;
		value.it_value.tv_sec = interval/1000000;
		value.it_value.tv_usec = interval%1000000;
		setitimer(ITIMER_REAL, &value, NULL);
//#else
	//	Thread::Pool::AddTask(&instance, true);
#endif
		tick_now = 0;
		gettimeofday(&base, NULL);
		return true;
	}

	static void StopTimer() 
	{ 
		stop = true; 
	}

	static void GetTime(struct timeval* val)
	{
		Thread::Mutex::Scoped lock(locker);
		if(val)
			*val = now;
	}

	static int64_t GetTick() { return tick_now;}

	int64_t UElapse() const 
	{
		Thread::Mutex::Scoped lock(locker);
		return (int64_t)(now.tv_sec - start.tv_sec)*1000000LL + now.tv_usec - start.tv_usec;
	}

	int Elapse() const 
	{
		return now.tv_sec - start.tv_sec;
	}

	void Reset() { start = now; }

	static void Handler(int signum)
	{
		triggered = true;
	}

	static void Suspend()
	{
		sigprocmask(SIG_BLOCK, &mask, NULL);
		if(triggered)
		{
			Update();
			triggered = false;
		}
	}

	static void Resume()
	{
		sigprocmask(SIG_UNBLOCK, &mask, NULL);
	}

	static void UpdateTimer()
	{
		if(triggered)
		{
			Update();
			triggered = false;
		}
	}

};

};

#endif
