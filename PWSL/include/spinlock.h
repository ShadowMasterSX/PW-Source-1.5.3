/*
	spinlock.h
	作者： 崔铭   袁宏俊
	功能： 自旋锁实现

        注释：禁止在一个线程内Lock两次。没有实现此功能。
*/

#ifndef __SPIN_LOCK_H__
#define __SPIN_LOCK_H__

#include "ASSERT.h"

void mutex_spinlock(int *__spinlock);			// lock
void mutex_spinlock2(int *__spinlock);			// m不会因为卡住而报告ASSERT的版本
void mutex_spinunlock(int *__spinlock);			// unlock
int  mutex_spinset(int *__spinlock);		// test and set , faster than mutex_spinwait(spinlock,0)

void mutex_futex_lock(int * __futex);
void mutex_futex_unlock(int * __futex);

void cond_futex_wait(int * __futex, int val, int timeout);		//等待 *__futex不等于 val 信号或者其他情况也有可能使得此函数返回 timeout 为微秒数 为0则无穷等待
void cond_futex_notify(int * __futex, int number);			//触发__futex等待 number 表示触发等待的数量

class spin_autolock{
	int * _spinlock;
	spin_autolock & operator =(spin_autolock & rhs);
public:
	spin_autolock(spin_autolock & rhs)
	{
		_spinlock = rhs._spinlock;
		rhs._spinlock = 0;
	}
	
	inline explicit spin_autolock(int & spinlock):_spinlock(&spinlock) 
	{ 
		mutex_spinlock(_spinlock); 
	} 
	inline spin_autolock(int * spinlock):_spinlock(spinlock)
	{ 
		if(spinlock) mutex_spinlock(_spinlock); 
	}
	inline void detach(bool unlock = true) 
	{
		if(unlock) mutex_spinunlock(_spinlock);
		_spinlock = 0;
	}

	inline bool is_attached()
	{
		return _spinlock; 
	}
	
	inline void attach(int *spinlock ,bool lock = false)
	{
		ASSERT(_spinlock == 0);
		_spinlock = spinlock;
		if(lock) mutex_spinlock(_spinlock); 
	}
	inline ~spin_autolock(){ if(_spinlock) mutex_spinunlock(_spinlock); } 
};

class spin_doublelock
{
	int * _spinlock1;
	int * _spinlock2;
public:
	spin_doublelock(int &spinlock1,int &spinlock2):_spinlock1(&spinlock1),_spinlock2(&spinlock2)
	{
		if(&spinlock1 < &spinlock2)
		{
			mutex_spinlock(&spinlock1);
			mutex_spinlock(&spinlock2);
		}
		else
		{
			ASSERT(&spinlock1 != &spinlock2);
			mutex_spinlock(&spinlock2);
			mutex_spinlock(&spinlock1);
		}
	}

	spin_doublelock(int *spinlock1,int * spinlock2):_spinlock1(spinlock1),_spinlock2(spinlock2)
	{
		if(spinlock1 < spinlock2)
		{
			mutex_spinlock(spinlock1);
			mutex_spinlock(spinlock2);
		}
		else
		{
			ASSERT(spinlock1 != spinlock2);
			mutex_spinlock(spinlock2);
			mutex_spinlock(spinlock1);
		}
	}

	~spin_doublelock()
	{

		mutex_spinunlock(_spinlock2);
		mutex_spinunlock(_spinlock1);
	}

};

#endif

