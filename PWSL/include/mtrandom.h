#include "spinlock.h"
#include <pthread.h>

#ifndef __ABASE_SF_MT_RANDOM_H__
#define __ABASE_SF_MT_RANDOM_H__

namespace abase {
class mt_rand
{
	void * _imp;
public:
	mt_rand():_imp(0)
	{}
	
	~mt_rand();
	
	bool Init();
	int IRandom(int min, int max);
	double Random();
	float FRandom();
};


class lock_mt_rand : private mt_rand
{
	int _lock;
public:
	lock_mt_rand():_lock(0)
	{}
	
	int IRandom(int min, int max)
	{
		spin_autolock keeper(_lock);
		return  mt_rand::IRandom(min, max);
	}

	double Random()
	{
		spin_autolock keeper(_lock);
		return  mt_rand::Random();
	}

	float FRandom()
	{
		spin_autolock keeper(_lock);
		return  mt_rand::FRandom();
	}

};

class ThRandMan
{
	mt_rand _common_rand;
	int _common_rand_lock;
	pthread_key_t _rand_key;

	static ThRandMan _instance;

	int LockIRandom(int min, int max)
	{
		spin_autolock keeper(_common_rand_lock);
		return _common_rand.IRandom(min,max);
	}

	double LockRandom()
	{
		spin_autolock keeper(_common_rand_lock);
		return _common_rand.Random();
	}

	float LockFRandom()
	{
		spin_autolock keeper(_common_rand_lock);
		return _common_rand.FRandom();
	}

public:
	ThRandMan():_common_rand_lock(0),_rand_key(-1)
	{}

	inline ThRandMan & Instance() { return _instance;}

	bool Init();
	void AttachThread();
	
	int IRandom(int min , int max);
	double Random();
	float FRandom();


};
}
#endif
 
