/*
 *
 * 	本文件是对IO库初始化过程的一些包装，使用了默认推荐的参数和初始化过程。
 * 	具体使用中，可根据情况选择分步初始化，或对其中某一步的参数进行调整。
 */
#ifndef _GNET_INIT_H_
#define _GNET_INIT_H_
#include "thread.h"
#include "threadpolicy.h"
#include "glog.h"
#include "gnet_timer.h"
#include "statistic.h"
#include "conf.h"

namespace GNET
{
	//初始化静态变量。库中使用了许多静态变量，它们的初始化顺序有较严格的要求，需调用这个函数以确保按顺序进行。
	//该函数必须调用，并且尽可能提前。没有调用的话，Timer将不能正常工作。
	void InitStaticObjects();

#ifdef __OLD_IOLIB_COMPATIBLE__ 

	inline void InitIoMan(bool single_thread)		//IOMan初始化
	{
		GNET::PollIO::Init (single_thread);
	}

	inline void InitThreadPolicy(bool single_thread)	//推荐的线程池初始化
	{
		if (single_thread)
		{
			Thread::Pool::_pool.Init( new ThreadPolicySingle());
			return;
		}
		ThreadPolicyBase * policy = new ThreadPolicyBase();
		policy->AddGroup(-1, 0); 
		policy->AddGroup(-1, 0);
		policy->SetSequenceCount(2);
		policy->AddThread(0, GNET::TASK_TYPE_GLOBAL);
		policy->AddThread(1, GNET::TASK_TYPE_GLOBAL);
		policy->AddThread(0, GNET::TASK_TYPE_GLOBAL | GNET::TASK_TYPE_SEQUENCE);
		policy->AddThread(1, GNET::TASK_TYPE_GLOBAL | GNET::TASK_TYPE_SEQUENCE);
		Thread::Pool::_pool.Init(policy);
		Thread::Pool::_pool.Start();
	}

	inline void InitLog(const char *name)			//日志库初始化，需在Conf, IOMan初始化完成后进行
	{
		GLog::Init(name);
	}

	inline void InitTimer(bool single_thread, abase::timer *gtimer = NULL)	//推荐的itimer初始化
	{
		if (single_thread)
		{
			GNET::IntervalTimer::StartTimer(50000, true);
		}
		else
		{
			IntervalTimer::PrepareTimer(gtimer, 50000);
			IntervalTimer::StartTimerThread();
		}
	}

	//推荐的初始化方式, 其初始化顺序是经过仔细考虑的。
	//在多线程模式下，gtimer必须由外部创建并传入。单线程模式下，gtimer为空。
	inline void AppInit(const char *conf_file, const char *servername, bool single_thread = true, abase::timer *gtimer = NULL)
	{
		InitStaticObjects();
		Conf::GetInstance(conf_file /* , servername */);  //配置文件的初始化. 第二个参数是conf的group名称。
		InitIoMan(single_thread);
		InitThreadPolicy(single_thread);
		InitLog(servername);
		InitTimer(single_thread, gtimer);
	}

	//推荐的主循环
	inline void AppRun(bool single_thread)
	{
		UpdateStatToLog  stat_observer;	//声明一个定期刷新到日志中的类。会自动Attach
		if (single_thread)
		{
			while(1)
			{
				PollIO::Poll(10);
				Timer::Update();
				IntervalTimer::UpdateTimer();
				Thread::Pool::_pool.TryProcessAllTask();
			}
		}
		else
		{
			while(1)
			{
				PollIO::Poll(10);
				Timer::Update();
			}

		}
	}

	//推荐的关闭方式
	inline void AppStop()
	{
		GNET::IntervalTimer::StopTimer();
		GNET::Thread::Pool::_pool.Stop();
		sleep(2);
		GNET::Thread::Pool::_pool.WaitStop();
	}
#endif

};
#endif
