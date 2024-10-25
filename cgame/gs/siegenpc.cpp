#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arandomgen.h>
#include "string.h"
#include "world.h"

#include "skillwrapper.h"
#include <amemory.h>
#include "npcsession.h"
#include "aipolicy.h"
#include "obj_interface.h"
#include <common/message.h>
#include <arandomgen.h>
#include "patrol_agent.h"
#include "aitrigger.h"
#include "clstab.h"
#include "faction.h"

#include "siegenpc.h"

DEFINE_SUBSTANCE(turret_policy,guard_policy,CLS_NPC_AI_POLICY_TURRET)

void 
turret_policy::OnHeartbeat()
{
	ai_policy::OnHeartbeat();

	XID leader = _self->GetLeaderID();

	A3DVECTOR selfpos;
	_self->GetPos(selfpos);
	//检查是否有目标对象
	if(!leader.IsValid())
	{
		_send_notify_counter ++;
		if(_send_notify_counter  >= SEND_NOTIFY_COUNTER_NO_LEADER)
		{
			SendNotify(0);
		}

		//如果处于攻击状态,或者有任何任务，此时停止当前攻击
		if(InCombat() || _cur_task)
		{
			_self->ActiveCombatState(false);
			EnableCombat(false,false);
			_self->ActiveInvisible(true);
			ClearTask();
			_self->ClearDamageList();
			_self->ClearAggro();
			_policy_flag = 0;
		}

		//如果没有目标对象，则定期减少血量
		A3DVECTOR pos2;
		pos2 = _self->GetBirthPlace();
		if(selfpos.squared_distance(pos2) > 100.f)	//超过出生地10米则少血
		{
			//定期减少血值
			_self->BeHurt((int)(_self->GetMaxHP() * 0.033333333f));
		}
		else
		{
			//考虑回归出生地
		}
		
	}
	else
	{
		//如果存在控制者，则每秒钟发送一次消息通知
		_self->SendMessage(leader, GM_MSG_TURRET_NOTIFY_LEADER);
		_send_notify_counter ++;
		if(_send_notify_counter  >= SEND_NOTIFY_COUNTER)
		{
			SendNotify(leader.id);
		}
		
		if(!_self->GetAggroCount() && !_cur_task) 
		{
			_agent->GatherTarget(_self,this,FACTION_HUMAN|FACTION_ORC|FACTION_YUMAO|FACTION_GHOST|FACTION_SPIRIT|FACTION_OBORO);
		}

		//试图跟随对象，如果对象不存在，或者超过距离， 则清除当前对象
		ai_object::target_info info;
		int target_state;
		float range;

		target_state = _self->QueryTarget(leader,info);
		if(target_state != ai_object::TARGET_STATE_NORMAL || info.invisible_degree > 0)
		{
			//目标不存在
			ClearLeader();
			return;
		}

		range = info.pos.horizontal_distance(selfpos);
		if(range > 150.f * 150.F)
		{
			ClearLeader();
			return;
		}

		float h = fabs(selfpos.y - info.pos.y);
		if(h > 60.f || range >= 60.f*60.f || (range < 6.f*6.f && h > 6.f))
		{
			//距离过远或者无法到达
			_leader_counter ++;
			if(_leader_counter > 5)
			{
				ClearLeader();
			}
			return ;
		}

		_leader_counter  = 0;

		if(range > 6.5f*6.5f)
		{
			if(InCombat())
			{
				_self->ActiveCombatState(false);
				EnableCombat(false,true);
				_self->ActiveInvisible(true);
				ClearTask();
				_self->ClearDamageList();
				_self->ClearAggro();
				_policy_flag = 0;
				AddTargetTask<ai_follow_target>(leader);
			}
			else
			{
				if(!_cur_task) 
				{
					AddTargetTask<ai_follow_target>(leader);
				}
			}

		}
/*
		if(!_cur_task)
		{
			if(range > 6.5f*6.5f)
			{
				AddTargetTask<ai_follow_target>(leader);
			}
		}
		*/
	}
}

void 
turret_policy::ClearLeader()
{
	_self->SetLeaderID(XID(-1,-1));
	SendNotify(0);
}

void 
turret_policy::Init(const ai_object & self, const ai_param & aip)
{
	ai_policy::Init(self,aip);
	_patrol_mode = false;
}

bool 
turret_policy::ChangeTurretMaster(const XID & target)
{
	XID leader = _self->GetLeaderID();
	if(leader.IsValid()) return false;

	_self->SetLeaderID(target);
	SendNotify(target.id);
	return true;
}

void 
turret_policy::ClearTurretMaster()
{
	const XID & leader = _self->GetLeaderID();
	if(leader.IsValid())
	{
		ClearLeader();
	}
}

void 
turret_policy::OnAggro()
{
	XID leader = _self->GetLeaderID();
	if(!leader.IsValid()) return;
	ai_policy::OnAggro();
}

void 
turret_policy::SendNotify(int id)
{
	_self->SendClientTurretNotify(id);
	_send_notify_counter = 0;
}

