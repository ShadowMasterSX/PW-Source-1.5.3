#ifndef __THREADPOOL_H
#define __THREADPOOL_H

#include <pthread.h>
#include <queue>
#include <set>
#include "mutex.h"
#include "thread.h"
#include "reference.h"
#include "pool.h"

namespace GNET
{

class ReuseableThread
{
	pthread_t tid;
	bool active;
	Thread::Mutex qlock("ReuseableThread::qlock");
	Thread::Condition qcond;
	std::queue<Thread::Runnable *> rq;
	void *thread_loop()
	{
		while (true)
		{
			Thread::Runnable *r;
			{
			Thread::Mutex::Scoped l(qlock);
			if ( rq.empty() )
			{
				active = false;
				qcond.Wait(qlock);
			}
			r = rq.front();
			rq.pop();
			}
			if (r) try { r->Run(); } catch (...) { delete r; }
			else break;
		}
		return NULL;
	}
	
	static void * thread_run(void * p)
	{
		return static_cast<ReuseableThread *>(p)->thread_loop();
	}

	ReuseableThread(const ReuseableThread&) { }
public:
	void RunTask(Thread::Runnable *r)
	{
		Thread::Mutex::Scoped l(qlock);
		qcond.NotifyOne();
		rq.push(r);
	}
	bool SetInactive() { return true; }
	bool SetActive()
	{
		Thread::Mutex::Scoped l(qlock);
		bool r = !active; active = true; return r;
	}

	~ReuseableThread()
	{
		{
		Thread::Mutex::Scoped l(qlock);
		qcond.NotifyOne();
		rq.push(NULL);
		active = true;
		}
		pthread_join(tid, NULL);
		printf("Destroy [%08x]\n", tid);
	}

	ReuseableThread() : active(true)
	{
		pthread_create(&tid, NULL, thread_run, this );
		printf("Create [%08x]\n", tid);
	}

	pthread_t Key() const { return tid; }
};

typedef ReuseableObjectRef<ReuseableThread, pthread_t> ReuseableThreadRef;
typedef std::set<ReuseableThreadRef> ReuseableThreadRefContainer;
typedef Pool<ReuseableThreadRefContainer> ThreadPool;

class StrictChooseThread
{
public:
	typedef ReuseableThreadRefContainer::iterator iterator;
	iterator operator() (ReuseableThreadRefContainer& oc)
	{
		iterator it = oc.begin(), ie = oc.end();
		for( ;it != ie && ! (*it)->SetActive(); ++it );
		return it;
	}
};

class RandomChooseThread
{
public:
	typedef ReuseableThreadRefContainer::iterator iterator;
	iterator operator() (ReuseableThreadRefContainer& oc)
	{
		iterator it = StrictChooseThread()(oc);
		if ( it != oc.end() )
			return it;
		it = oc.begin();
		std::advance( it, rand() % oc.size() );
		return it;
	}
};

template<typename ChooseMode>
class AffinityChooseThread
{
	bool init;
	pthread_t tid;
	ChooseMode chooser;
public:
	typedef ReuseableThreadRefContainer::iterator iterator;
	AffinityChooseThread() : init(false) { }
	AffinityChooseThread(pthread_t id) : init(true), tid(id) { }
	iterator operator() (ReuseableThreadRefContainer& oc)
	{
		iterator it, ie = oc.end();
		if ( init && (it = oc.find(tid)) != ie )
			return it;
		if ( init = ((it = chooser(oc)) != ie) )
			tid = (*it).Key();
		return it;
	}
};

template<typename ChooseMode>
class ThreadChooser : public ObjectChooser<ReuseableThreadRefContainer>
{
	ChooseMode chooser;
public:
	typedef ReuseableThreadRefContainer::iterator iterator;
	bool Choose(ReuseableThreadRefContainer& oc)
	{
		iterator it = chooser(oc);
		bool r = (it != oc.end());
		if ( r ) AppendObject(it);
		return r;
	}
};

};

#endif
