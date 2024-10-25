#include "string.h"
#include "addiction.h"
#include <map>
#include <algorithm>

namespace Addiction
{

int LIGHT_PANISH_TIME = 100;
int WEIGHT_PANISH_TIME = 200;
int CLEAR_TIME = 300;
int ADDICTION_MODE = 0;

enum 
{
	VERSION_1 = 1,
};

IAddictionData::~IAddictionData() {}

struct AddictionInfo
{
	AddictionInfo() : version(VERSION_1), online_sum(0), offline_sum(0), logout_time(0) {}

	void Reset(int now)
	{
		version = VERSION_1;
		online_sum = offline_sum = 0;
		logout_time = now;	
	}
	unsigned int version;
	int online_sum;
	int offline_sum;
	int logout_time;
};

struct AddictionData: public IAddictionData
{
	AddictionData(int now) 
		: time_light(now + LIGHT_PANISH_TIME), time_weight(now + WEIGHT_PANISH_TIME) 
	{ }

	virtual ~AddictionData() {}

	virtual void GetTimeLeft(time_t * _time_light, time_t * _time_weight, int * play_time)
	{
		*_time_light = time_light;
		*_time_weight = time_weight; 
		*play_time = info.online_sum;
	}

	virtual void Tick(time_t now, OnlineState * _state)
	{
		if (now >= time_weight)
			*_state = S_WEIGHT_PANISH; 
		else if (now >= time_light)
			*_state = S_LIGHT_PANISH;
		else
			*_state = S_NORMAL;	
	}

	virtual void GetData(time_t now, void **ptr, int *size)
	{
		info.online_sum = now - time_light + LIGHT_PANISH_TIME;
		info.logout_time = now;
		*ptr = &info;
		*size = sizeof(AddictionInfo);
	}

	virtual bool Init(time_t now, const void * buf, int len)
	{
		if (NULL == buf || len == 0)
		{
			Reset(now);	
			return true;
		}

		if (len < 4) return false;

		unsigned int version = *(unsigned int*)buf;
		if (version  == VERSION_1)	
		{
			if (len != sizeof(AddictionInfo)) 
				return false;

			memcpy(&info, buf, len);
			int delta = now - info.logout_time;
			info.offline_sum += delta;
			if (info.offline_sum > CLEAR_TIME)
			{
				Reset(now);
			}
			else
			{
				time_light = now - info.online_sum + LIGHT_PANISH_TIME;
				time_weight = now - info.online_sum + WEIGHT_PANISH_TIME;
			}
			return true;
		}
		return false;
	}
	
	void Reset(int now)
	{
		time_light = now + LIGHT_PANISH_TIME;
		time_weight = now + WEIGHT_PANISH_TIME;		
		info.Reset(now);
	}

	time_t time_light;
	time_t time_weight;	
	AddictionInfo info;
};

struct AddictionData_XZero: public AddictionData
{
	time_t _next_clear_time;

	time_t GetNextMidnight(time_t t)
	{
		struct tm tt;
		localtime_r(&t, &tt);
		tt.tm_hour = 23;
		tt.tm_min = 59;
		tt.tm_sec = 59; 

		return mktime(&tt) + 1;
	}

	AddictionData_XZero(int now):AddictionData(now)
	{
		_next_clear_time = 0;
	}

	virtual bool Init(time_t now, const void * buf, int len)
	{
		bool bRst = AddictionData::Init(now, buf, len);
		if(bRst)
		{
			//根据logout计算是否应当清空时间
			time_t t = GetNextMidnight(info.logout_time);
			if(now > t)
			{
				//超过了清空时间
				Reset(now);
				_next_clear_time = GetNextMidnight(info.logout_time);
			}
			else
			{
				//没超过清空时间 设置下次清空时间
				_next_clear_time = t;
			}
		}
		return bRst;
	}
	virtual void Tick(time_t now, OnlineState * _state)
	{

		if(now > _next_clear_time)
		{
			Reset(now);
			*_state = S_NORMAL;
			return;
		}

		if (now >= time_weight)
			*_state = S_WEIGHT_PANISH; 
		else if (now >= time_light)
			*_state = S_LIGHT_PANISH;
		else
			*_state = S_NORMAL;	
	}
};

void SetAddictionParam(int _time_light, int _time_weight, int _clear_time,int calcmode)
{
	LIGHT_PANISH_TIME = _time_light * 60;
	WEIGHT_PANISH_TIME = _time_weight * 60;
	CLEAR_TIME = _clear_time * 60;
	ADDICTION_MODE = calcmode;
	if(calcmode == 1)
	{
		CLEAR_TIME = 24*3600;
	}
}

IAddictionData * CreateAddictionObj(int now) 
{	
	if(ADDICTION_MODE == 0)
		return new AddictionData(now); 
	else
		return new AddictionData_XZero(now); 
} 

}; // namespace
	
