#ifndef __ONLINE_GAME_GS_WALLOW_OBJ_H__
#define __ONLINE_GAME_GS_WALLOW_OBJ_H__

#include "addiction.h"

class wallow_object
{
	Addiction::IAddictionData * _imp;
	public:
	wallow_object()
	{ 
		_imp = Addiction::CreateAddictionObj(0);
	}
	~wallow_object()
	{
		delete _imp;
	}

	inline void GetTimeLeft(time_t * ltime, time_t * htime, int * play_time)
	{
		return _imp->GetTimeLeft(ltime, htime, play_time);
	}

	inline int Tick(time_t now)
	{
		Addiction::OnlineState ol;
		_imp->Tick(now, &ol);
		return (int)ol;
	}

	inline bool Init(time_t now, const void * buf, size_t len)
	{
		_imp->Init(now, buf, len);
		return true;
	}

	inline size_t GetData(time_t now, void ** ptr)
	{
		int len;
		_imp->GetData(now, ptr, &len);
		return len;
	}

	inline void Swap(wallow_object & rhs)
	{
		Addiction::IAddictionData * tmp = _imp;
		_imp = rhs._imp;
		rhs._imp = tmp;
	}

	static void SetParam(int l, int h, int c, int mode)
	{

		Addiction::SetAddictionParam(l,h,c,mode);
	}
};

template <typename WRAPPER>
WRAPPER & operator >> (WRAPPER & wrapper, wallow_object & rhs)
{
	size_t len;
	wrapper >> len;
	rhs.Init(time(NULL),wrapper.cur_data(), len);
	wrapper.shift(len);
	return wrapper;
}

template <typename WRAPPER>
WRAPPER & operator << (WRAPPER & wrapper, wallow_object & rhs)
{
	void * ptr;
	size_t len = rhs.GetData(time(NULL),&ptr);
	wrapper << len;
	wrapper.push_back(ptr, len);
	return wrapper;
}

#endif

