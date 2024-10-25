#ifndef __GNET_REQUESTLIMITER_H
#define __GNET_REQUESTLIMITER_H

#include <algorithm>
#include <map>
#include "timer.h"

namespace GNET
{

class LimitPolicy
{
public:
	virtual bool Update() { return false; }
	virtual ~LimitPolicy() { }
};
enum
{
	CHAT_POLICY      = 1,
	WHISPER_POLICY   = 2,
	FINDNAME_POLICY  = 3,
};

class CounterPolicy : public LimitPolicy
{
	int _period;
	int _threshold;
	int _term;
	int _counter;
	time_t _last;
public:
	CounterPolicy(int period, int threshold, int term):_period(period),_threshold(threshold),_term(term)
	{
		_last = Timer::GetTime();
		_counter = 0;
	}

	bool Update() 
	{ 
		time_t now = Timer::GetTime();
		if(_counter<0)
		{
			if(now-_last<_term)
				return false;
			_counter = 1;
			_last = now;
			return true;
		}
		if(now-_last>_period)
		{
			_counter = 1;
			_last = now;
			return true;
		}
		if(_counter>=_threshold)
		{
			_counter = -1;
			_last = now;
			return false;
		}
		else
		{
			_counter++;
		}
		return true;
	}
};

class RequestLimiter
{
	typedef std::map<int, LimitPolicy*> PolicyMap;
	PolicyMap limiters;
public:
	RequestLimiter() 
	{
	}
	~RequestLimiter()
	{
		for(PolicyMap::iterator it=limiters.begin();it!=limiters.end();++it)
			delete it->second;
		limiters.clear();
	}
	void Initialize()
	{
		limiters.insert(std::make_pair((int)CHAT_POLICY,new CounterPolicy(10,5,30)));
		limiters.insert(std::make_pair((int)WHISPER_POLICY,new CounterPolicy(10,8,30)));
		limiters.insert(std::make_pair((int)FINDNAME_POLICY,new CounterPolicy(10,3,15)));
	}
	bool Update(int requestid)
	{
		PolicyMap::iterator it = limiters.find(requestid);
		if(it==limiters.end())
			return false;
		return it->second->Update();
	}
};

}

#endif
