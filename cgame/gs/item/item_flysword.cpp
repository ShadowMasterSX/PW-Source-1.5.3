#include "../clstab.h"
#include "../world.h"
#include "../actobject.h"
#include "../item_list.h"
#include "item_flysword.h"
#include "../filter.h"
#include "../fly_filter.h"
#include "../clstab.h"
#include "../playertemplate.h"
#include "../world.h"
#include "../worldmanager.h"
#include "../playerlimit.h"

DEFINE_SUBSTANCE(flysword_item,item_body,CLS_ITEM_FLY_SWORD)
DEFINE_SUBSTANCE(angel_wing_item,item_body,CLS_ITEM_ANGEL_WING)
DEFINE_SUBSTANCE(cls_flysword_item,item_body, CLS_ITEM_CLS_FLY_SWORD)

//飞剑的实现
bool 
flysword_item::VerifyRequirement(item_list & list,gactive_imp* obj)
{ 
	return _ess.ess.require_level <= obj->GetHistoricalMaxLevel();
}

int
flysword_item::OnFlying(int tick)
{
	if(!IsActive()) return -1;
	_ess.ess.cur_time -= tick;
	if(_ess.ess.cur_time < 0) _ess.ess.cur_time = 0;
	return  _ess.ess.cur_time;
}

int
flysword_item::OnCharge(int element_level, size_t count,int & cur_time)
{
	ASSERT(count);
	ASSERT(_ess.ess.time_per_element);
	if(_ess.ess.cur_time >= _ess.ess.max_time) return 0;
	size_t offset = _ess.ess.max_time - _ess.ess.cur_time;
	size_t use_count  = offset / _ess.ess.time_per_element;

	if(offset > use_count * _ess.ess.time_per_element) use_count ++;
	if(use_count > count) 
	{
		use_count = count;
		_ess.ess.cur_time += count * _ess.ess.time_per_element;
	}
	else
	{
		_ess.ess.cur_time = _ess.ess.max_time;
	}
	cur_time = _ess.ess.cur_time;
	return (int)use_count;
}

int 
flysword_item::OnUse(item::LOCATION l,gactive_imp * obj,size_t count)
{
	if(l != item::BODY || !IsActive()) 
	{
		obj->_runner->error_message(S2C::ERR_CANNOT_USE_ITEM);
		return -1;
	}

	if(world_manager::GetWorldLimit().nofly  || obj->GetPlayerLimit(PLAYER_LIMIT_NOFLY))
	{
		obj->_runner->error_message(S2C::ERR_CANNOT_FLY);
		return 0;
	}

	if(obj->_filters.IsFilterExist(FILTER_FLY_EFFECT))
	{
		//取消飞行效果
		obj->_filters.RemoveFilter(FILTER_FLY_EFFECT);
	}
	else
	{
		if(obj->_bind_to_ground)
		{
			obj->_runner->error_message(S2C::ERR_CANNOT_FLY);
			return 0;
		}

		//加入飞行效果
		obj->_filters.AddFilter(new flysword_fly_filter(obj,FILTER_FLY_EFFECT,0.f));
	}
	return 0;
}

//  羽人专用翅膀
bool 
angel_wing_item::IsItemCanUse(item::LOCATION l)
{
	return l == item::BODY;
}

bool 
angel_wing_item::VerifyRequirement(item_list & list,gactive_imp* imp)
{
	gactive_object *obj = (gactive_object*)imp->_parent;
	int cls =  (obj->base_info.race & 0x7FFF);	 //计算出所属的职业
	return _ess.ess.require_level <= imp->GetHistoricalMaxLevel() &&  
		(cls == USER_CLASS_ARCHER || cls == USER_CLASS_ANGEL);
}
void 
angel_wing_item::OnActivate(item::LOCATION ,size_t pos, size_t count, gactive_imp* obj)
{
	obj->_skill.EventWield(obj,65535);
}

void 
angel_wing_item::OnDeactivate(item::LOCATION ,size_t pos, size_t count,gactive_imp* obj)
{
	obj->_skill.EventUnwield(obj,65535);
}


int 
angel_wing_item::OnUse(item::LOCATION l,gactive_imp * imp,size_t count)
{
	if(l != item::BODY || !IsActive()) 
	{
		imp->_runner->error_message(S2C::ERR_CANNOT_USE_ITEM);
		return -1;
	}
	if(world_manager::GetWorldLimit().nofly || imp->GetPlayerLimit(PLAYER_LIMIT_NOFLY))
	{
		imp->_runner->error_message(S2C::ERR_CANNOT_FLY);
		return 0;
	}
	if(imp->_filters.IsFilterExist(FILTER_FLY_EFFECT))
	{
		//取消飞行效果
		imp->_filters.RemoveFilter(FILTER_FLY_EFFECT);
	}
	else
	{
		//加入飞行效果
		if(imp->_bind_to_ground)
		{
			imp->_runner->error_message(S2C::ERR_CANNOT_FLY);
			return 0;
		}
		if(!imp->DrainMana(_ess.ess.mp_launch)) return -3;
		imp->_filters.AddFilter(new angel_wing_fly_filter(imp,FILTER_FLY_EFFECT,_ess.ess.mp_per_second));
	}
	return 0;
}

//飞剑的实现
bool 
cls_flysword_item::VerifyRequirement(item_list & list,gactive_imp* obj)
{ 
	return _ess.ess.require_level <= obj->GetHistoricalMaxLevel() && 
	( (1 << (obj->GetObjectClass() & 0x1F)) & _ess.ess.require_class); 
}

int cls_flysword_item::OnGetFlyTime()
{
	return _ess.ess.cur_time;
}

int
cls_flysword_item::OnFlying(int tick)
{
	if(!IsActive()) return -1;
	_ess.ess.cur_time -= tick;
	if(_ess.ess.cur_time < 0) _ess.ess.cur_time = 0;
	return  _ess.ess.cur_time;
}

int
cls_flysword_item::OnCharge(int element_level, size_t count,int & cur_time)
{
	ASSERT(count);
	ASSERT(_ess.ess.time_per_element);
	if(_ess.ess.cur_time >= _ess.ess.max_time) return 0;
	if(_ess.ess.level <= 0 || element_level <= 0) return 0;
	/*
	int level_offset = element_level - _ess.ess.level;
	float xo = _ess.ess.time_per_element;
	if(level_offset < 0)
	{
		xo /= (float)(1 <<(-level_offset));
	}
	else
	{
		xo *= (float)(1 << level_offset);
	}
	*/
	float xo = element_level;
	xo /= _ess.ess.level;

	size_t offset = _ess.ess.max_time - _ess.ess.cur_time;
	size_t use_count  = (int)(offset / xo);

	if(offset > (size_t)(use_count * xo)) use_count ++;
	if(use_count > count) 
	{
		use_count = count;
		_ess.ess.cur_time += (int)(count * xo);
	}
	else
	{
		_ess.ess.cur_time = _ess.ess.max_time;
	}
	cur_time = _ess.ess.cur_time;
	return (int)use_count;
}

int 
cls_flysword_item::OnUse(item::LOCATION l,gactive_imp * obj,size_t count)
{
	if(l != item::BODY || !IsActive()) 
	{
		obj->_runner->error_message(S2C::ERR_CANNOT_USE_ITEM);
		return -1;
	}
	if(world_manager::GetWorldLimit().nofly  || obj->GetPlayerLimit(PLAYER_LIMIT_NOFLY))
	{
		obj->_runner->error_message(S2C::ERR_CANNOT_FLY);
		return 0;
	}
	if(obj->_filters.IsFilterExist(FILTER_FLY_EFFECT))
	{
		//取消飞行效果
		obj->_filters.RemoveFilter(FILTER_FLY_EFFECT);
	}
	else
	{
		if(obj->IsBindGound())
		{
			obj->_runner->error_message(S2C::ERR_CANNOT_FLY);
			return 0;
		}
		//加入飞行效果
		float speed_offset = _ess.ess.speed_increase2 - _ess.ess.speed_increase;
		obj->_filters.AddFilter(new flysword_fly_filter(obj,FILTER_FLY_EFFECT,speed_offset));
	}
	return 0;
}

int 
cls_flysword_item::OnGetLevel()
{
	return _ess.ess.level;
}

int 
cls_flysword_item::GetImproveLevel()
{
	return _ess.ess.improve_level;
}

bool 
cls_flysword_item::FlyswordImprove(float speed_inc, float speed_inc2)
{
	_ess.ess.improve_level ++;
	_ess.ess.speed_increase += speed_inc;
	_ess.ess.speed_increase2 += speed_inc2;
	CalcCRC();
	return true;
}

bool
cls_flysword_item::Sign(unsigned short color, const char * signature, unsigned int signature_len)
{
	if(signature_len == 0) //清除签名
	{
		if(_ess.name_type == element_data::IMT_NULL || _ess.name_size == 0) return false;
		_ess.name_type = element_data::IMT_NULL;
		memset(_ess.name,0,MAX_USERNAME_LENGTH);
		_ess.name_size = 0;
	}
	else
	{
		_ess.name_type = element_data::IMT_SIGN;
		memset(_ess.name,0,MAX_USERNAME_LENGTH);
		memcpy(_ess.name,(char *)&color,sizeof(unsigned short));
		memcpy(_ess.name+sizeof(unsigned short),signature,signature_len);
		_ess.name_size = sizeof(unsigned short) + signature_len;
	}
	CalcCRC();
	return true;
}

