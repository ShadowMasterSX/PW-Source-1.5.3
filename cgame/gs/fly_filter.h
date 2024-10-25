#ifndef __ONLINEGAME_GS_FLY_FILTER_H__
#define __ONLINEGAME_GS_FLY_FILTER_H__

#include "filter.h"
#include "sfilterdef.h"

class fly_filter : public filter
{
private:
	virtual void Heartbeat(int tick){ASSERT(false);}			//在心跳时作处理,tick表示本次间隔几秒
public:
	fly_filter(object_interface parent,int filter_id)
		:filter(parent,FILTER_MASK_HEARTBEAT|FILTER_MASK_REMOVE_ON_DEATH|FILTER_MASK_UNIQUE)
	{
		_filter_id = filter_id;
	}

	virtual void OnAttach() 
	{
		//要让对象进入飞行状态
		_parent.TakeOff();
	}

	virtual void OnRelease()
	{
		//要让对象退出飞行状态 
		_parent.Landing();
	}

protected:
	fly_filter(){}
};

class flysword_fly_filter : public fly_filter
{
public:
	float _speed_enhance;
	bool _is_active;	//是否激活状态
	DECLARE_SUBSTANCE(flysword_fly_filter);
	flysword_fly_filter(object_interface parent,int filter_id,float speed_enhance)
		:fly_filter(parent,filter_id),_speed_enhance(speed_enhance),_is_active(false)
		{
		}

	virtual void OnAttach() 
	{
		//要让对象进入飞行状态
	//	_parent.SendClientMsgFlySwordTime(100);
		_parent.TakeOff();
	}

	virtual bool Save(archive & ar)
	{
		filter::Save(ar);
		ar << _speed_enhance << _is_active;
		return true;
	}

	virtual bool Load(archive & ar)
	{
		filter::Load(ar);
		ar >> _speed_enhance >> _is_active;
		return true;
	}

	virtual void OnRelease()
	{
		//将速度变回 
		Deactive();
		fly_filter::OnRelease();
	}
	virtual void  OnModify(int ctrlname,void * ctrlval,size_t ctrllen);
	void Deactive()
	{
		if(_is_active)
		{
			_parent.ImpairFlySpeed(_speed_enhance);
			_parent.UpdateSpeedData();
			_parent.SendClientCurSpeed();
			_is_active = false;
			_parent.SendClientRushMode(0);
		}
	}

	void Active()
	{
		if(!_is_active)
		{
			int time = _parent.GetFlyTime();
			if(time > 0)
			{
				_parent.EnhanceFlySpeed(_speed_enhance);
				_parent.UpdateSpeedData();
				_parent.SendClientCurSpeed();
				_is_active = true;
				_parent.SendClientRushMode(1);
				_parent.SendClientMsgFlySwordTime(time);
			}
		}
	}

protected:
	flysword_fly_filter(){}
	virtual void Heartbeat(int tick);
};

class angel_wing_fly_filter : public fly_filter
{
	int _mana_per_second;
public:
	DECLARE_SUBSTANCE(angel_wing_fly_filter);
	angel_wing_fly_filter(object_interface parent,int filter_id,int mana_per_second)
		:fly_filter(parent,filter_id),_mana_per_second(mana_per_second)
		{
		}
	virtual void Heartbeat(int tick);
	virtual bool Save(archive & ar);
	virtual bool Load(archive & ar);
	virtual void OnAttach() 
	{
		fly_filter::OnAttach();
		_parent.SendClientRushMode(1);
	}	

	virtual void OnRelease()
	{
		_parent.SendClientRushMode(0);
		fly_filter::OnRelease();
	}
protected:
	angel_wing_fly_filter(){}
};
#endif

