#include "string.h"
#include "moving_action.h"
#include "world.h"
#include "actobject.h"
#include "skill_filter.h"

moving_timer_action::moving_timer_action(gactive_imp * imp) : moving_action(imp), _plane(NULL)
{
	if(_imp) 
	{
		_plane = _imp->_plane;
		_self_id = _imp->_parent->ID;
	}
}

void moving_timer_action::SendRepeatMsg()
{
	MSG msg;
	BuildMessage(msg,GM_MSG_OBJ_ACTION_REPEAT,_self_id,_self_id,A3DVECTOR(0.f,0.f,0.f),_action_id);
	_plane->PostLazyMessage(msg);
}

void moving_timer_action::SendEndMsg()
{
	MSG msg;
	BuildMessage(msg,GM_MSG_OBJ_ACTION_END,_self_id,_self_id,A3DVECTOR(0.f,0.f,0.f),_action_id);
	_plane->PostLazyMessage(msg);
}

void moving_skill::SetTarget(int skill_id, char force_attack,int target_num,int * targets)
{
	_data.id = skill_id;
	_data.forceattack = force_attack;
	_force_attack = force_attack;
	if(target_num > 0)
	{
		XID id;
		_target_list.reserve(target_num);
		for(int i = 0; i < target_num; i ++,targets ++)
		{
			MAKE_ID(id,*targets);
			_target_list.push_back(id);
		}
	}
}

bool moving_skill::StartAction()
{
	__PRINTF("ACTION: moving skill................. %d\n", _data.id);

	//悬空状态不能释放技能
	if(_imp->CheckLevitate()) return false;	
	
	int first_interval = _imp->StartSkill(_data,_target_list.begin(),_target_list.size(),_next_interval);
	if(first_interval < 0)
	{
		_imp->_runner->error_message(S2C::ERR_SKILL_NOT_AVAILABLE);
		return false;
	}

	if(_target_list.size())
	{
		//检查是否负面法术
		int type = GNET::SkillWrapper::GetType(_data.id);
		if(type == 1 || type == 3)
		{
			_imp->Notify_StartAttack(_target_list[0], _force_attack);
		}
	}
	
	if(first_interval < 50)
	{
	/*
		__PRINTF("玩家瞬发技能\n");
		//瞬发技能
		int next_interval;
		_imp->RunSkill(_data,_target_list.begin(),_target_list.size(),next_interval);
		return false;
		*/
		first_interval = 50;
	}

	//将时间转换成tick
	__PRINTF("ACTION: moving skill start: first_interval %d, next_interval %d\n",first_interval, _next_interval);
	first_interval /= 50;
	_next_interval /= 50;
	ASSERT(first_interval > 0);

	if(_data.skippable) 
	{
		_skill_skip_time = g_timer.get_tick();
	}

	
	SetTimer(g_timer,20,0,first_interval);

	//注册一个filter
	_imp->_filters.AddFilter(new moving_skill_interrupt_filter(_imp,_action_id,FILTER_INDEX_MOVING_SKILL));
	return true;
}

bool moving_skill::RestartAction()
{
	if(!_data.skippable) return true;
	ASSERT(_action_id >= 0);
	//重新分配action id
	_action_id = _imp->_moving_action_env.NextActionID();

	//蓄力技能重新开始
	int tick = g_timer.get_tick() - _skill_skip_time;
	if(tick <= 0) tick = 0;
	
	//停止当前定时器
	RemoveTimer();

	int next_interval;
	int interval = _imp->ContinueSkill(_data,_target_list.begin(),_target_list.size(),next_interval,tick*50);
	//将时间转换成tick
	interval /= 50;
	if(interval <= 0) return false;
	_next_interval = next_interval / 50;

	if(_data.skippable) 
	{
		_skill_skip_time = g_timer.get_tick();
	}
	
	SetTimer(g_timer,20,0,interval);
	return true;
}

bool moving_skill::RepeatAction()
{
	if(_end_flag) return false;	//结束

	int new_interval = -1;
	int rst = _imp->RunSkill(_data,_target_list.begin(),_target_list.size(),new_interval);
	if(rst <= 0 || _next_interval <= 0) return false;

	__PRINTF("ACTION: moving skill repeat , next interval %d\n",new_interval);
	if(_data.skippable) 
	{
		_skill_skip_time = g_timer.get_tick();
	}
	if(new_interval <= 0)
	{
		_next_interval  = new_interval;
		return true;
	}
	_next_interval  = new_interval / 50;
	return true;
}

bool moving_skill::EndAction()
{
	if(_action_id >= 0)
	{
		//并非强制终止技能，试图终止之
		__PRINTF("ACTION: moving skill session end \n");
		_action_id = -1;
		RemoveTimer();
		_imp->_filters.RemoveFilter(FILTER_INDEX_MOVING_SKILL);
		_imp->_runner->stop_skill();
		timeval tv;
		gettimeofday(&tv,NULL);
		__PRINTF("ACTION: player %6d stop moving skill at %ld.%06ld\n",_imp->_parent->ID.id,tv.tv_sec,tv.tv_usec);
	}
	return true;
}

bool moving_skill::TerminateAction(bool force)
{
	if(force) 
	{
		return EndAction();
	}

	if(_action_id >= 0)
	{
		if(_imp->CancelSkill(_data))
		{
			return EndAction();
		}
		return false;
	}
	return true;
}

void moving_skill::OnTimer(int index,int rtimes)
{
	int interval = _next_interval;
	__PRINTF("ACTION: %d moving skill change timer %d %d\n",_self_id.id,interval,_end_flag);
	if(interval <= 0 || _end_flag)
	{
		//结束自己 
		SendRepeatMsg();
		if(_timer_index != -1) //试图结束自己的定时器
		{
			RemoveSelf();
		}
	}
	else
	{
		ChangeIntervalInCallback(interval);
		_next_interval = 20;		//这个操作理论上可能由于时序引发错误$$$$
		SendRepeatMsg();
	}
}

bool moving_skill::OnAttacked()
{
	if(_imp->SkillOnAttacked(_data))
	{
		_end_flag = true;
		RemoveTimer();
		SendEndMsg();
		return true;
	}
	return false;
}

void moving_instant_skill::SetTarget(int skill_id, char force_attack,int target_num,int * targets)
{
	_data.id = skill_id;
	_data.forceattack = force_attack;
	if(target_num > 0)
	{
		XID id;
		_target_list.reserve(target_num);
		for(int i = 0; i < target_num; i ++,targets ++)
		{
			MAKE_ID(id,*targets);
			_target_list.push_back(id);
		}
	}
}

bool moving_instant_skill::StartAction()
{
	__PRINTF("ACTION: moving instant skill................. %d\n", _data.id);
	//悬空状态不能释放技能
	if(_imp->CheckLevitate()) return false;	
	
	int rst = _imp->CastInstantSkill(_data,_target_list.begin(),_target_list.size());
	if(rst < 0)
	{
		_imp->_runner->error_message(S2C::ERR_SKILL_NOT_AVAILABLE);
		return false;
	}
	return false;
}

