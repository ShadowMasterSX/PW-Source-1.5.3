#include <stdio.h>
#include <sys/resource.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <string>
#include <map>
#include <sstream>

#define FOR_GS
#include <ASSERT.h>
/* GS和Delivery用的不是一套线程池 */
#ifdef FOR_GS
#include <threadpool.h>
#include <timer.h>
#include <arandomgen.h>
#include <gsp_if.h>
typedef ONET::Thread::Runnable RunnableBase;
#else
#include <thread.h>
typedef GNET::Thread::Runnable RunnableBase;
#endif
#include "threadusage.h"

#ifndef RUSAGE_THREAD
#define RUSAGE_THREAD 1
#endif


class usage_stat
{
    struct timeval _begin_time;
    struct rusage _begin_usage;

    float _user;
    float _sys;
    bool _thread;
public:
    usage_stat(bool is_thread=true):_user(0.f),_sys(0.f), _thread(is_thread)
    {
    	Reset();
    }
    float GetUser() const {return _user;}
    float GetSys() const {return _sys;}

    static float GetDiff(const timeval& end, const timeval & begin)
    {
    	return (float)(end.tv_sec - begin.tv_sec) + (end.tv_usec - begin.tv_usec)/1000000.f;
    }
    bool DoStat(float min_interval, float e)
    {
	    struct timeval now;
	    gettimeofday(&now, NULL);
	    float elapse= GetDiff( now, _begin_time);
	    if (elapse -min_interval < e) return false;

	    struct rusage r_usage;
	    getrusage(_thread?RUSAGE_THREAD:RUSAGE_SELF,&r_usage );
	    float ut = GetDiff(r_usage.ru_utime, _begin_usage.ru_utime);
	    float st = GetDiff(r_usage.ru_stime, _begin_usage.ru_stime);
	    _user = ut/elapse;
	    _sys = st/elapse;
	    return true;
    }
    void Reset()
    {
    	gettimeofday(&_begin_time, NULL);
	getrusage(_thread?RUSAGE_THREAD:RUSAGE_SELF,&_begin_usage);
    }
};

struct thread_usage	//一个线程所相关的统计信息
{
	int lock;
private:
	usage_stat sec;	//每一秒的统计
	usage_stat sec5;//每五秒的统计
	usage_stat min; //每分钟的统计
	usage_stat min5;//每五分钟的统计
public:
	std::string ident;
	thread_usage():lock(0){}
	thread_usage(bool thread):lock(0),sec(thread),sec5(thread),min(thread),min5(thread){}
	void TryDoStat() { //这个函数会被一秒调用一次
		ASSERT(lock);
		if (sec.DoStat(1, -0.3)) sec.Reset();
		if (sec5.DoStat(5,-0.3)) sec5.Reset();
		if (min.DoStat(60, -1))  min.Reset();
		if (min5.DoStat(300, -5)) min5.Reset();
	}
	std::stringstream& dump(int idx, std::stringstream& os)
	{
		ASSERT(lock);
		char buf[256] = {0};
		char name[256] = {0};
		if (!ident.empty())
			strcpy(name ,ident.c_str());
		else
			strcpy(name, "unknown");
		sprintf (buf, "%d:%-10s cpu:1(%02.1f%%,%02.1f%%) 5(%02.1f%%,%02.1f%%) 60(%02.1f%%,%02.1f%%) 300(%02.1f%%,%02.1f%%)\n",
				idx, name, 
				sec.GetUser()*100 , sec.GetSys()*100,
				sec5.GetUser()*100, sec5.GetSys()*100,
				min.GetUser()*100, min.GetSys()*100,
				min5.GetUser()*100, min5.GetSys()*100);
		os << buf;
		return os;
	}

	std::stringstream& get_log_string(std::stringstream& os)
	{
		ASSERT(lock);
		char buf[256] = {0};
		char name[256] = {0};
		if (!ident.empty())
			strcpy(name ,ident.c_str());
		else
			strcpy(name, "unknown");
		sprintf (buf, "(%s,us%02.1f%%,sy%02.1f%%)", name, min5.GetUser()*100, min5.GetSys()*100);
		os << buf;
		return os;
	}
};

class cpu_usage_man
{
	int lock;
	std::map<pthread_t,thread_usage> _thread_list;
	thread_usage _process;		//进程的CPU使用率
	int _delay;			//从收到启动指令到真正开始采集要延迟多长时间(秒)
	bool _start;			//是否已经启动，防止启动多个采集线程
	bool _stop_flag;		//是否要关闭
	FILE *_fpout;	
	int _dump_interval;
public:
	cpu_usage_man():lock(0), _process(false),_delay(10), _start(false),_stop_flag(false), _fpout(0),_dump_interval(8){_process.ident="Total";}
	~cpu_usage_man(){
		if (_fpout) fclose(_fpout);
	}
	thread_usage & GetUsage(pthread_t id)
	{
		spin_autolock keeper(lock);
		return _thread_list[id];
	}
	thread_usage& GetProcessUsage() {return _process;}
	void SetDelay(int sec) {_delay = sec;}
	int GetDelay() const {return _delay;}
	void SetDumpInterval(int i) {_dump_interval = i;}
	void dump(FILE *fp)
	{
		std::stringstream os;
		int i=0;
		{
		
			spin_autolock keeper(_process.lock);
			_process.dump(i++, os);
		}
		{
			spin_autolock keeper(lock);
			std::map<pthread_t,thread_usage>::iterator it = _thread_list.begin();
			for(;it != _thread_list.end(); ++it, ++i)
			{
				thread_usage & u = it->second;
				spin_autolock keeper(u.lock);
				u.dump(i, os);
			}
		}
		fprintf (fp, os.str().c_str());
		fflush(fp);
	}
	void get_log_string(std::stringstream& os)
	{
		{
		
			spin_autolock keeper(_process.lock);
			_process.get_log_string(os);
		}
		{
			spin_autolock keeper(lock);
			std::map<pthread_t,thread_usage>::iterator it = _thread_list.begin();
			for(;it != _thread_list.end(); ++it)
			{
				thread_usage & u = it->second;
				spin_autolock keeper(u.lock);
				u.get_log_string(os);
			}
		}
	}
	class ProbeTask: public RunnableBase	//检测线程池中用的任务
	{
		void Run()
		{
			_man->StatSelf("Pool");
			delete this;
		}
		cpu_usage_man *_man;
	public:
		ProbeTask(cpu_usage_man *man):_man(man){}
	};

	#ifdef FOR_GS
	class TimerProbeTask: public abase::timer_task	 //检测定时器线程用的任务
	{
		void OnTimer(int index,int rtimes)
		{
			_man->StatSelf("Timer");
			if(_man->IsWantStop())
			{
				RemoveSelf();
			}
		}
		cpu_usage_man *_man;
	public:
		TimerProbeTask(cpu_usage_man *man):_man(man){}
	};
	#endif

	static void *WorkThread(void *arg)
	{
		cpu_usage_man * man = (cpu_usage_man*)arg;
		sleep(man->GetDelay());

		#ifdef FOR_GS
		TimerProbeTask *pTask = new TimerProbeTask(man);
		extern abase::timer g_timer;
		pTask->SetTimer(g_timer, 20, 0);
		#endif

		while(!man->IsWantStop())
		{
			sleep(1);
			for (int i=0; i<10; i++)
			{
				RunnableBase * pTask = new ProbeTask(man);
			#ifdef FOR_GS
				ONET::Thread::Pool::AddTask(pTask);
				GMSV::ThreadUsageStat();
			#else
				const int pri[4]={0,1,100,101};
				pTask->SetPriority(pri[rand()%4]);
				GNET::Thread::Pool::AddTask(pTask);
			#endif
			}
			man->StatSelf("Stat");

			thread_usage & u = man->GetProcessUsage();
			{
				spin_autolock keeper(u.lock);
				u.TryDoStat();
			}
			man->Heartbeat();
		}
		return NULL;
	}
	bool IsStart() const { return _start;}
	void Start()
	{
		if (_start) return ;
		_start = true;
		char *fname = getenv("CPU_USAGE_FILE");
		if (fname) _fpout = fopen(fname, "ab");
		fname = getenv("CPU_USAGE_INTERVAL");
		if (fname) SetDumpInterval(atoi(fname));
		fname = getenv("CPU_USAGE_DELAY");
		if (fname) SetDelay(atoi(fname));

		pthread_t tid;
		pthread_create(&tid, NULL, WorkThread, this);
	}
	void Heartbeat()
	{
		if ( _dump_interval && !(time(0)%_dump_interval)){
			if (_fpout) dump(_fpout);
			else dump(stdout);
		}
	}
	void Stop() {_stop_flag = true;}
	bool IsWantStop() const {return _stop_flag;}

	void StatSelf(const char *ident)
	{
		thread_usage & u = GetUsage(pthread_self());
		spin_autolock keeper(u.lock);
		u.TryDoStat();
		if (u.ident.empty() && ident)
		{
			u.ident = ident;
		}
	}
};
namespace ThreadUsage {
	static cpu_usage_man _ins;
	void Start(int delaysecond, int dump_interval)
	{
		_ins.SetDelay(delaysecond);
		_ins.SetDumpInterval(dump_interval);
		_ins.Start();
	}
	void ChangeDumpInterval(int dump_interval)
	{
		_ins.SetDumpInterval(dump_interval);
	}
	void Dump(FILE *fp)
	{
		_ins.dump(fp);
	}
	void Stop()
	{
		_ins.Stop();
	}
	void StatSelf(const char *thread_ident)
	{
		_ins.StatSelf(thread_ident);
	}
	void GetLogString(std::stringstream& os)
	{
		_ins.get_log_string(os);
	}
}
