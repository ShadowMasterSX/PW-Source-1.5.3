#include <unistd.h>
#include <iostream>
#include <signal.h>

#include "conf.h"
#include "thread.h"
#include "itimer.h"

using namespace GNET;

void showmask()
{
        sigset_t mask, old_mask;
        sigprocmask(SIG_SETMASK, &mask, &old_mask);
        sigprocmask(SIG_SETMASK, &old_mask, NULL);

        printf(" %08x : ", old_mask);

        if (sigismember(&old_mask, SIGALRM))
                printf ("SIGALRM blocked\n");
        else
                printf ("SIGALRM not blocked\n");
}

class TickCounter : public IntervalTimer::Observer
{
	IntervalTimer* t;
public:
	bool Update()
	{
		if(!t)
			t = new IntervalTimer;
		printf("Tick: %lld,  %lld usec elapsed\n", IntervalTimer::GetTick(), t->UElapse());
		return true;
	}
	TickCounter():t(NULL) {}
};
	
class TestObserver : public IntervalTimer::Observer
{
public:
	bool Update()
	{
		static int i=0;
		printf("In TestObserver:");
		showmask();
		i++;
		if(i==5)
		{
			printf("Delay update for 3 seconds\n");
			sleep(3);
		}
		i %= 5;
		printf("    Observer %p updated\n", this);
		return true;
	}
};

class TestTask : public Thread::Runnable
{
	int64_t start;
	int delay;
public:
	void Run( )
	{
		int64_t now = IntervalTimer::GetTick();
		printf("    Task run in thread: %d delay(%d %ld)\n", pthread_self(), delay, now - start);
		delete this;
	}

	TestTask(int t):delay(t)
	{
		start = IntervalTimer::GetTick();
	}
};

class Single : public Thread::Runnable
{
public:
	void Run( )
	{
		printf("run\n");
		while(1)
		{
			//ResumeTimer();
			printf("%d sec left \n", sleep(5));
			UpdateItimer();
		}
	}
};

class TestTask2 : public Thread::Runnable
{
public:
	void Run( )
	{
		struct timeval tv;
		gettimeofday(&tv, NULL);
		IntervalTimer::Schedule(new TestTask(1), 1);
		IntervalTimer::Schedule(new TestTask(2), 2);
		IntervalTimer::Schedule(new TestTask(4), 4);
		IntervalTimer::Schedule(new TestTask(8), 8);
		IntervalTimer::Schedule(new TestTask(16), 16);
		IntervalTimer::Schedule(new TestTask(32), 32);
		IntervalTimer::Schedule(new TestTask(100), 100);
		delete this;
	}
};

void handler(int signum)
{
	IntervalTimer::StopTimer();
	struct timeval tm;
	IntervalTimer::GetTime(&tm);
	printf("Current time: %d:%d\n", tm.tv_sec, tm.tv_usec);
}


class StressTask : public IntervalTimer::Observer
{
public:
	bool Update()
	{
//	    double f = 1.0e20;
//		for(long long i=0;i<50;i++)
//			f/=i;
		return true;
	}
};

void StressTest(int num)
{
	for(int i=0;i<num;i++)
		IntervalTimer::Attach(new StressTask(), 1);

}

Thread::Mutex locker("Interval::locker");
int TestLocker()
{
	Thread::Mutex::Scoped lock(locker);
	static int n = 0;
	return n++; 
}

void Test()
{
	struct timeval start,end;
	gettimeofday(&start, NULL);
	for(int i=0;i<100000;i++)
	{
		TestLocker();
	}
	gettimeofday(&end, NULL);
	printf("%lld usec used\n", (int64_t)(end.tv_sec-start.tv_sec)*1000000LL + end.tv_usec - start.tv_usec);
}

int main(int argc, char ** argv)
{
	signal(SIGHUP, handler);
	Conf::GetInstance("io.conf");
	IntervalTimer::StartTimer(50000);
	IntervalTimer::Schedule(new TestTask2(), 1);
	IntervalTimer::Attach(new TestObserver(), 2);
	IntervalTimer::Attach(new TickCounter(), 1);
	IntervalTimer::Attach(new TestObserver(), 8);
	StressTest(10000);
#if !defined _REENTRANT_
	printf("Single thread version\n");
	Thread::Pool::AddTask(new Single());
#else
	printf("Multi thread version\n");
#endif
	Thread::Pool::Run( );
	return 0;
}
