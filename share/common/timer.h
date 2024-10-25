#ifndef __TIMER_H
#define __TIMER_H

#include <vector>
#include <functional>
#include <algorithm>
#include <time.h>
#include <sys/time.h> //for gettimeofday
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
	static Observers& observers() { static Observers tmp; return tmp; }
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
	int Elapse_ms() const
    {
        return (now_tv.tv_sec - tv.tv_sec)*1000 + (now_tv.tv_usec - tv.tv_usec)/1000;
    }
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

};

#endif
