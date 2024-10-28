#include <stdio.h>
#include "string.h"
#include "world.h"
#include "clstab.h"
#include "skillwrapper.h"
#include <amemory.h>
#include "npcsession.h"
#include "aipolicy.h"
#include "obj_interface.h"
#include <common/message.h>
#include <arandomgen.h>
#include "patrol_agent.h"
#include "aitrigger.h"
#include "skillwrapper.h"

DEFINE_SUBSTANCE(ai_policy,substance, CLS_NPC_AI_POLICY_BASE)
DEFINE_SUBSTANCE(group_master_policy,ai_policy,CLS_NPC_AI_POLICY_MASTER)
DEFINE_SUBSTANCE(group_minor_policy, ai_policy,CLS_NPC_AI_POLICY_MINOR)
DEFINE_SUBSTANCE(group_boss_policy, ai_policy,CLS_NPC_AI_POLICY_BOSS)
DEFINE_SUBSTANCE(ai_task,substance, CLS_NPC_AI_TASK)
DEFINE_SUBSTANCE(ai_melee_task,ai_task,CLS_NPC_AI_MELEE_TASK)
DEFINE_SUBSTANCE(ai_range_task,ai_task,CLS_NPC_AI_RANGE_TASK)
DEFINE_SUBSTANCE(ai_magic_task,ai_task,CLS_NPC_AI_MAGIC_TASK)
DEFINE_SUBSTANCE(ai_magic_melee_task,ai_task,CLS_NPC_AI_MAGIC_MELEE_TASK)
DEFINE_SUBSTANCE(ai_follow_master,ai_task,CLS_NPC_AI_FOLLOW_MASTER_TASK)
DEFINE_SUBSTANCE(ai_pet_follow_master,ai_task,CLS_NPC_AI_PET_FOLLOW_MASTER_TASK)
DEFINE_SUBSTANCE(ai_follow_target,ai_task,CLS_NPC_AI_FOLLOW_TARGET_TASK)
DEFINE_SUBSTANCE(ai_runaway_task,ai_task,CLS_NPC_AI_RUNAWAY_TASK)
DEFINE_SUBSTANCE(ai_skill_task,ai_task, CLS_NPC_AI_SKILL_TASK)
DEFINE_SUBSTANCE(ai_skill_task_2,ai_task, CLS_NPC_AI_SKILL_TASK_2)
DEFINE_SUBSTANCE(ai_pet_skill_task,ai_task, CLS_NPC_AI_PET_SKILL_TASK)
DEFINE_SUBSTANCE(ai_silent_runaway_task,ai_runaway_task, CLS_NPC_AI_SILIENT_RUNAWAY_TASK)
DEFINE_SUBSTANCE(ai_rest_task,ai_task, CLS_NPC_AI_REST_TASK)
DEFINE_SUBSTANCE(ai_regeneration_task,ai_task, CLS_NPC_AI_REGENERATION_TASK)
DEFINE_SUBSTANCE(ai_patrol_task,ai_task, CLS_NPC_AI_PATROL_TASK)
DEFINE_SUBSTANCE(ai_fix_melee_task,ai_task,CLS_NPC_AI_FIX_MELEE_TASK)
DEFINE_SUBSTANCE(ai_silent_task,ai_task,CLS_NPC_AI_SILIENT_TASK)
DEFINE_SUBSTANCE(ai_returnhome_task,ai_task,CLS_NPC_AI_RETURNHOME_TASK)
DEFINE_SUBSTANCE(ai_fix_magic_task,ai_task,CLS_NPC_AI_FIX_MAGIC_TASK)
DEFINE_SUBSTANCE(ai_play_action_task,ai_task,CLS_NPC_AI_PLAY_ACTION_TASK)

DEFINE_SUBSTANCE_ABSTRACT(patrol_agent,substance,CLS_NPC_PATROL_AGENT)

ai_policy::~ai_policy()
{
	ClearTask();
	if(_self) delete _self;
	if(_path_agent) delete _path_agent;
	if(_at_policy) delete _at_policy;
}

void 
ai_policy::Init(const ai_object & self, const ai_param & aip)
{
	int primary_strategy = aip.primary_strategy;
	ASSERT(primary_strategy >=STRATEGY_MELEE && primary_strategy < STRATEGY_MAX);
	_primary_strategy = primary_strategy;
	_self = self.Clone();
	_quarter_hp = _self->GetMaxHP() >> 2;
	_cur_event_hp = _quarter_hp * 3;
	//ASSERT(_quarter_hp);
	if(_quarter_hp <=0) _quarter_hp = 1;
	_event_list[0] = aip.event[0];
	_event_list[1] = aip.event[1];
	_event_list[2] = aip.event[2];
	_event_level[0] = aip.event_level[0];
	_event_level[1] = aip.event_level[1];
	_event_level[2] = aip.event_level[2];

	_attack_skills.count = aip.skills.as_count;
	_bless_skills.count = aip.skills.bs_count;
	_curse_skills.count = aip.skills.cs_count;
	for(size_t i = 0; i < 8; i ++)
	{
		_attack_skills.skill[i]= aip.skills.attack_skills[i].id;
		_attack_skills.level[i]= aip.skills.attack_skills[i].level;

		_bless_skills.skill[i]= aip.skills.bless_skills[i].id;
		_bless_skills.level[i]= aip.skills.bless_skills[i].level;

		_curse_skills.skill[i]= aip.skills.curse_skills[i].id;
		_curse_skills.level[i]= aip.skills.curse_skills[i].level;
	}

	_patrol_mode = (bool)aip.patrol_mode;
	_no_auto_fight = aip.no_auto_fight;
	_max_move_range = aip.max_move_range;
	ASSERT(_cur_task == NULL);
	ASSERT(_task_list.empty());
	ChangePath(aip.path_id,aip.path_type,aip.speed_flag);
}

void 
ai_policy::ChangePath(int path_id,int path_type,char speed_flag)
{
	//如果当前存在session_npc_patrol, 则必须先停止它，否则session中的_path_agent指针会失效,导致程序宕机
	if(_self->HasSession())
		_self->AddSession(new session_npc_empty());
	if(_path_agent)
	{
		delete _path_agent;
		_path_agent = NULL;
	}
	if(path_id > 0)
	{
		_path_agent = new base_patrol_agent();
		if(!_path_agent->Init(path_id,path_type))
		{
			delete _path_agent;
			_path_agent = NULL;
		}
	}
	_speed_flag = speed_flag;	
}

bool 
ai_policy::DetermineTarget(XID & target)
{
	return _self->GetAggroEntry(0,target);
}

void ai_policy::OnAggro()
{
	//不可进入战斗
	if(_no_auto_fight) return;

	//设置有敌人的标记
	if(_idle_mode)
	{
		return ;
	}
	
	//如果是第一次出现aggro，那么就立刻进行反应
	XID target;
	if(!DetermineTarget(target))
	{
		//无法确定目标
		return ;
	}

	_self->ActiveCombatState(true);
	EnableCombat(true,false);
	_self->ActiveInvisible(false);
	
	if(_cur_task)
	{
		_cur_task->OnAggro();
		return ;
	}



	//如果仇恨列表中只有一个， 立刻发送一次HATE
	if(target.type != GM_TYPE_NPC) _self->HateTarget(target);

	if(!_at_policy)
	{
		int hp = _self->GetHP();
		if(hp < _cur_event_hp && TriggerEvent(hp,target))
		{
			return;
		}
	}

	//确定策略
	DeterminePolicy(target);
}

void 
ai_policy::KillTarget(const XID & target)
{
	if(_at_policy)
	{
		_at_policy->KillTarget(target);
	}
}


void ai_policy::DeterminePolicy(const XID & target)
{
	if(target.id != -1)
	{
		AddPrimaryTask(target, _primary_strategy);
		if(_cur_task ) return;
	}

//无目标 或者处理原目标失败
	int count = 3;
	XID old_target(-1,-1);
	while(_cur_task == NULL && _self->GetAggroCount() && count > 0)
	{
		XID new_target;
		if(!DetermineTarget(new_target))
		{
			break;
		}
		AddPrimaryTask(new_target, _primary_strategy);
		if(old_target == new_target) break;
		old_target = new_target;
	}
}

void
ai_policy::RollBack()
{
	//仇恨列表清空,做恢复操作
	_self->ActiveCombatState(false);
	EnableCombat(false,true);
	_self->ActiveInvisible(true);
	_self->ClearDamageList();
	_cur_event_hp = _self->GetHP();
	_policy_flag = 0;

	if(_path_agent)
	{
		A3DVECTOR pos;
		_path_agent->GetCurWayPoint(pos);
		//_self->ReturnHome(pos,0.0f);
		AddPosTask<ai_returnhome_task>(pos);
	}
	else
	{
		//判断是否回到出生地
		A3DVECTOR pos;
		if(_self->IsReturnHome(pos,GetReturnHomeRange()))
		{
			AddPosTask<ai_returnhome_task>(pos);
		}
	}
	//加入一个等待血值恢复的任务
	// if(_fast_regen && _cur_event_hp != _self->GetMaxHP()) AddTask<ai_regeneration_task>();
}

void TTTTT(gactive_imp *);

void 
ai_policy::HaveRest()
{
	//考虑是否休闲动作
	if(_patrol_mode && !_cur_task && _self->CanRest())
	{
		//加入休息策略
		AddTask<ai_rest_task>();
	}
}

void 
ai_policy::SetAITrigger(int policy_id)
{
	ASSERT(_at_policy == NULL);

	if(policy_id)
	{
		_at_policy = ai_object::CreatePolicy(policy_id);
		ASSERT(_at_policy);
		if(_at_policy)
		{
			_at_policy->SetParent(_self,this);
		}
	}

}

void ai_policy::OnHeartbeat()
{
	if(_self->GetState() & ai_object::STATE_ZOMBIE) return;
	//判断寿命
	if(_life > 0)
	{
		if(--_life <= 0)
		{
			//让自己强行死亡
			XID id;
			_self->GetID(id);
			_self->SendMessage(id,GM_MSG_DEATH);
			_life = 1;
			return ;
		}
	}

	if(_idle_mode) return;
	if(_at_policy)
	{
		if(InCombat())
		{
			_at_policy->OnHeartbeat();
			_at_policy->OnPeaceHeartbeatInCombat();
		}
		else
		{
			_at_policy->OnPeaceHeartbeat();
		}
	}
	if(_cur_task) 
	{
		//优先做当前的任务
		_cur_task->OnHeartbeat();
		if(InCombat())
		{
			if(!_path_agent && GetMaxMoveRange() > 1e-6)
			{
				A3DVECTOR selfpos;
				_self->GetPos(selfpos);
				if(selfpos.squared_distance(_self->GetBirthPlace()) > GetMaxMoveRange())
				{
					_self->ClearAggro();
				}
			}
			
			if(_self->GetAggroCount() == 0)
			{
				RollBack();
			}
		}
	}
	else
	{
		if(!InCombat())
		{
			//这里平时就会不停的调用，会否过于耗费?? 
			if(_path_agent)
			{
			//	由于巡逻的可能会比较远，所以可能需要不停的巡逻 以免玩家在路上等不到NPC
			//	if(_self->IsInIdleHeartbeat()) return ;

				A3DVECTOR pos;
				bool first_end = true;
				if(_path_agent->GetNextWayPoint(pos, first_end))
				{
					//加入一个追击的特殊task
					AddPosTask<ai_patrol_task>(pos);
				}
				else
				{
					if(first_end && _at_policy) _at_policy->PathEnd(_path_agent->GetPathID());
				}
				return;
			}
			HaveRest();
			return;
		}

		if(_self->GetAggroCount())
		{
			DeterminePolicy();
		}
		else
		{
			RollBack();
			return;
		}
	}

	if(!_at_policy)
	{
		int hp = _self->GetHP();
		if(hp < _cur_event_hp)
		{
			XID target(-1,-1);
			if(TriggerEvent(hp,target)) return;
		}
		else
		{
			hp -= _quarter_hp;
			while(hp > _cur_event_hp)
			{
				_cur_event_hp += _quarter_hp;
			}
		}
	}

	_tick ++;
}

void ai_policy::OnDeath(const XID & attacker)
{
	EnableCombat(false,false);
	
	if(_at_policy)
	{
		_at_policy->OnDeath();
		_at_policy->ResetAll();
	}
	ClearTask();
}

void ai_policy::OnDamage()
{
	if(_at_policy)
	{
		_at_policy->OnDamage();
	}
}

void ai_policy::EnableCombat(bool is_combat,bool is_trigger) 
{ 
	if(!is_combat)
	{
		if(InCombat() && _at_policy)
		{
			_at_policy->Reset();
			if(is_trigger)
			{
				_at_policy->EndCombat();
			}
		}
	}
	else
	{
		if(!InCombat() && _at_policy)
		{
			_at_policy->StartCombat();
		}
	}
	_in_combat_mode = is_combat;
}

/* --------------------------------------------------------------------- */


bool
ai_task::DetermineTarget(XID & target)
{
	return _self->GetAggroEntry(0,target);
}

bool 
ai_task::EndTask()
{
	_apolicy->TaskEnd();
	return true;
}

bool 
ai_target_task::StartTask()
{
	ASSERT(_target.id != -1);
	Execute();
	return true;
}

void
ai_target_task::OnSessionEnd(int session_id, int reason)
{
	//sessoin_id可能会不一致的，这是由于会有多个session
	if(session_id < _session_id) return;
	//当前sessoin结束,
	//重新回到执行界面
	switch(reason)
	{
		case NSRC_ERR_PATHFINDING:
		//考虑不直接清除，因为如果是组队的话，这样就会引发有人飞起逃跑，其他人都白打的情况，
		//考虑使用对此人加入20秒不许打得buff话，可能会更好 
		//不过WOW就是其他人打也会闪避的，所以先一律闪避吧。
		
		//但是aggro消息可能已经发出了，只是延迟收到 
		//先只处理error path finding
			if(_target.IsPlayerClass())
			{
			/*	_self->RemoveAggroEntry(_target);
				if(_self->GetAggroCount())
				{
					_self->AddInvincibleFilter(_target);
					XID t1;
					_self->GetAggroEntry(0,t1);
					ChangeTarget(t1);
				}
				else
				{*/
					_self->ClearAggro();
					_self->ClearDamageList();
					if(_apolicy->IsFastRegen())
					{
						session_npc_regeneration * pSession = new session_npc_regeneration(_self);
						pSession->SetAITask(-25);
						_self->AddSession(pSession);
					}
					EndTask();
				//}
				return ;
			}	
	}

	_session_id = -26;		//在重复执行时，这个session可能会保存上一次sessioin的值，因此现在应该清除

	if(_apolicy->HasNextTask())
	{
		EndTask();
	}
	else
	{
		Execute();
	}
}

bool 
ai_target_task::EndTask()
{
	return ai_task::EndTask();
}

bool 
ai_target_task::ChangeTarget(const XID & target)
{
	ASSERT(target.type != -1);
	if(_target == target) return true;
	_target = target;
    _self->SetTargetCache(target);
	_self->AddSession(new session_npc_empty());
	Execute();
	return true;
}

void 
ai_target_task::OnHeartbeat()
{
	if(_apolicy->HasNextTask())
	{
		if(!_self->HasSession())
		{
			EndTask();
		}
		else
		{
			_self->AddSession(new session_npc_empty());
		}
		return;
	}
	
	XID  target;
	if(_self->GetAggroEntry(0,target))
	{
		if(target != _target || !_self->HasSession())
		{
			_target = XID(-1,-1);
			_session_id = -27;
			ChangeTarget(target);
		}
	}
	else
	{
		//不论如何都加入一个空session
		_self->AddSession(new session_npc_empty());
		if(_session_id < 0)
		{
			//应该结束
			EndTask(); 
		}
		else
		{
			_target = XID(-1,-1);
		}
	}
	
}

/************************************************/

void
ai_melee_task::Execute()
{
	if(_session_id >= 0) return;
	if(_target.type == -1) 
	{
		EndTask();
		return ;
	}
	//如果在session中，则不处理
	//status A
	ai_object::target_info info;
	int target_state;
	float range;
	A3DVECTOR selfpos;

	_self->GetPos(selfpos);
	target_state = _self->QueryTarget(_target,info);
	float srange = _self->GetIgnoreRange();
	if(target_state != ai_object::TARGET_STATE_NORMAL 
		|| (range = info.pos.squared_distance(selfpos)) >= srange * srange
		|| _self->GetAntiInvisibleDegree() < info.invisible_degree)
	{
		//目标已经无法再被攻击,任务结束
		_self->RemoveAggroEntry(_target);
		EndTask();
		return;
	}

	//目标处于攻击范围内 
	//float attack_range = _self->GetAttackRange()  + info.body_size; 
	float body_size = _self->GetBodySize();
	float pure_attack_range = _self->GetAttackRange() - body_size;
	float attack_range = pure_attack_range*0.8f + body_size + info.body_size;
	float short_range = body_size + info.body_size;
	if(range > attack_range * attack_range)//如果与目标的距离超过了当前melee攻击的阈值，则先进入走路状态
	{
		session_npc_follow_target *pSession = new session_npc_follow_target(_self);
		pSession->SetTarget(_target,attack_range,_self->GetIgnoreRange(),pure_attack_range*0.6f+_self->GetBodySize() + info.body_size);
		pSession->SetChaseInfo(&_chase_info);
		//pImp->_cur_prop.attack_range,cmd->_ignore_range);
		pSession->SetAITask(_apolicy->GetTaskID());
		_self->AddSession(pSession);
		return ;
	}
	else
	{
		if (range < short_range*short_range)
		{
			//距离过近，要躲避一下
			session_npc_keep_out *pSession = new session_npc_keep_out(_self);
			pSession->SetTarget(_target,attack_range,4);
			pSession->SetAITask(_apolicy->GetTaskID());
			_self->AddSession(pSession);
			ClearChaseInfo();
			return ;
		}
	}

	//直接进入攻击状态
	session_npc_attack *attack = new session_npc_attack (_self);
	attack->SetTarget(_target,false);
	attack->SetShortRange(short_range);
	attack->SetAITask(_apolicy->GetTaskID());
	ClearChaseInfo();
	_self->AddSession(attack);
}

void
ai_fix_melee_task::Execute()
{
	if(_session_id >= 0) return;
	if(_target.type == -1) 
	{
		EndTask();
		return ;
	}
	//如果在session中，则不处理
	//status A
	ai_object::target_info info;
	int target_state;
	float range;
	A3DVECTOR selfpos;

	_self->GetPos(selfpos);
	target_state = _self->QueryTarget(_target,info);
	float srange = _self->GetIgnoreRange();
	if(target_state != ai_object::TARGET_STATE_NORMAL 
		|| (range = info.pos.squared_distance(selfpos)) >= srange * srange
		|| _self->GetAntiInvisibleDegree() < info.invisible_degree)
	{
		//目标已经无法再被攻击,任务结束
		_self->RemoveAggroEntry(_target);
		EndTask();
		return;
	}

	//目标处于攻击范围内 
	//float attack_range = _self->GetAttackRange()  + info.body_size; 
	float body_size = _self->GetBodySize();
	float pure_attack_range = _self->GetAttackRange() - body_size;
	float attack_range = pure_attack_range*0.8f + body_size + info.body_size;
	if(range < attack_range * attack_range)//如果与目标的距离超过了当前melee攻击的阈值，则先进入走路状态
	{
		//直接进入攻击状态
		session_npc_attack *attack = new session_npc_attack (_self);
		attack->SetTarget(_target,false);
		attack->SetShortRange(0);	//无short range
		attack->SetAITask(_apolicy->GetTaskID());
		_self->AddSession(attack);
	}
	else
	{
		//否则应该攻击其他人
		_self->FadeTarget(_target);
		EndTask();
		return;
	}
}

void
ai_fix_magic_task::Execute()
{
	if(_session_id >= 0) return;
	if(_target.type == -1) 
	{
		EndTask();
		return ;
	}
	//如果在session中，则不处理
	//status A
	ai_object::target_info info;
	int target_state;
	float range;
	A3DVECTOR selfpos;
	XID selfxid;

	_self->GetPos(selfpos);
	_self->GetID(selfxid);
	target_state = _self->QueryTarget(_target,info);
	float srange = _self->GetIgnoreRange();
	if(target_state != ai_object::TARGET_STATE_NORMAL 
		|| (range = info.pos.squared_distance(selfpos)) >= srange * srange
		|| _self->GetAntiInvisibleDegree() < info.invisible_degree
			&& _target.id != selfxid.id)
	{
		//目标已经无法再被攻击,任务结束
		_self->RemoveAggroEntry(_target);
		EndTask();
		return;
	}

	//目标处于攻击范围内 
	//float attack_range = _self->GetAttackRange()  + info.body_size; 
	int skill = 0;
	int skill_level;
	_apolicy->GetPrimarySkill(skill,skill_level);
	if(skill <=0)
	{
		_self->RemoveAggroEntry(_target);
		EndTask();
		return;
	}

	float body_size = _self->GetBodySize();
	float skill_range = _self->GetMagicRange(skill,skill_level) + body_size;

	if(range < skill_range * skill_range)
	{
		//直接进入攻击状态
		session_npc_skill *pSession = new session_npc_skill(_self);
		pSession->SetTarget(skill,skill_level,_target);
		pSession->SetAITask(_apolicy->GetTaskID());
		_self->AddSession(pSession);
	}
	else
	{
		//否则应该攻击其他人
		_self->FadeTarget(_target);
		EndTask();
		return;
	}
}

void
ai_range_task::Execute()
{
	if(_session_id >= 0) return;
	if(_target.type == -1)
	{
		EndTask();
		return ;
	}
	//如果在session中，则不处理
	//status A
	ai_object::target_info info;
	int target_state;
	float range;
	A3DVECTOR selfpos;

	_self->GetPos(selfpos);
	target_state = _self->QueryTarget(_target,info);
	float srange = _self->GetIgnoreRange();
	if(target_state != ai_object::TARGET_STATE_NORMAL 
		|| (range = info.pos.squared_distance(selfpos)) >= srange * srange
		|| _self->GetAntiInvisibleDegree() < info.invisible_degree)
	{
		//目标已经无法再被攻击,任务结束
		_self->RemoveAggroEntry(_target);
		EndTask();
		return;
	}

	//判断目标处于攻击范围内 
	float attack_range = _self->GetAttackRange()  + info.body_size; 
	float sa = attack_range * attack_range;
	if(range > sa)		//如果与目标的距离超过了当前range攻击的阈值，则先进入追击
	{
		session_npc_follow_target *pSession = new session_npc_follow_target(_self);
		pSession->SetTarget(_target,attack_range,_self->GetIgnoreRange(),attack_range * 0.8f);
		pSession->SetChaseInfo(&_chase_info);
		//pImp->_cur_prop.attack_range,cmd->_ignore_range);
		pSession->SetAITask(_apolicy->GetTaskID());
		_self->AddSession(pSession);
		return ;
	}

	//如果是第一次调用，则先进入攻击状态
	float short_range = sa * (0.3f*0.3f);
	if(short_range < 4.f) short_range = 4.f;
	float close_range = info.body_size + _self->GetBodySize() + 0.3f;
	if((range < close_range * close_range || (range < sa * (0.6f*0.6f)  && _ko_count > 0 ) )
			&&  _state != 1 )
	{
		_state = 1;
		_ko_count --;
		//在射程的0.6倍范围内，进入躲避状态
		session_npc_keep_out *pSession = new session_npc_keep_out(_self);
		pSession->SetTarget(_target,attack_range,5);
		pSession->SetAITask(_apolicy->GetTaskID());
		_self->AddSession(pSession);
		ClearChaseInfo();
		return;
	}
	//直接进入攻击状态
	_state = 2;	//当前是攻击状态
	session_npc_range_attack *attack = new session_npc_range_attack (_self);
	attack->SetTarget(_target,false);
	attack->SetInterrupt(range > short_range);
	attack->SetRange(attack_range, close_range);
	attack->SetAITask(_apolicy->GetTaskID());
	_self->AddSession(attack);
	ClearChaseInfo();
}
#define FLEE_RANGE 30.f

bool 
ai_runaway_task::StartTask()
{
	session_npc_flee *pSession = new session_npc_flee(_self);
	pSession->SetTarget(_target,FLEE_RANGE + _self->GetBodySize(),8);
	pSession->SetAITask(_apolicy->GetTaskID());
	_self->AddSession(pSession);
	return true;
}

void
ai_runaway_task::OnSessionEnd(int session_id, int reason)
{
	//sessoin_id可能会不一致的，这是由于会有多个session
	if(session_id < _session_id) return;
	_session_id = -36;
	EndTask();
}

bool 
ai_runaway_task::ChangeTarget(const XID & target)
{
	_target = target;
    _self->SetTargetCache(target);
	return true;
}

bool
ai_silent_runaway_task::StartTask()
{
	session_npc_silent_flee *pSession = new session_npc_silent_flee(_self);
	pSession->SetTarget(_target,FLEE_RANGE,8);
	pSession->SetAITask(_apolicy->GetTaskID());
	_self->AddSession(pSession);
	return true;
}

bool
ai_magic_task::StartTask()
{
	//获得技能攻击距离 
	_skill_type = _apolicy->GetPrimarySkill(_skill,_skill_level);
	if(_skill <=0) return false;
	_skill_range = _self->GetMagicRange(_skill,_skill_level) + _self->GetBodySize();
	return ai_target_task::StartTask();
}

void
ai_magic_task::Execute()
{
	if(_session_id >= 0) return;
	if(_target.type == -1)
	{
		EndTask();
		return ;
	}
	//如果在session中，则不处理
	//status A
	ai_object::target_info info;
	int target_state;
	float range;
	A3DVECTOR selfpos;
	XID selfxid;

	_self->GetPos(selfpos);
	_self->GetID(selfxid);
	target_state = _self->QueryTarget(_target,info);
	float srange = _self->GetIgnoreRange();
	if(target_state != ai_object::TARGET_STATE_NORMAL 
		|| (range = info.pos.squared_distance(selfpos)) >= srange * srange
		|| _self->GetAntiInvisibleDegree() < info.invisible_degree
			&& _target.id != selfxid.id)
	{
		//目标已经无法再被攻击,任务结束
		_self->RemoveAggroEntry(_target);
		EndTask();
		return;
	}

	if(!_skill_type)
	{
		//如果是祝福技能，则直接使用技能
		//使用法术
		_state = STATE_MAGIC;
		//use magic
		session_npc_skill *pSession = new session_npc_skill(_self);
		pSession->SetTarget(_skill,_skill_level,_target);
		pSession->SetAITask(_apolicy->GetTaskID());

		//随机取得下一次技能
		_skill_type = _apolicy->GetPrimarySkill(_skill,_skill_level);
		_skill_range = _self->GetMagicRange(_skill,_skill_level) + _self->GetBodySize();
		
		_self->AddSession(pSession); // 可能会delete掉当前的task 即this
		return ;
	}

	float magic_attack_range = _skill_range + info.body_size; 
	float sa = magic_attack_range * magic_attack_range;
	if(range > sa*0.81f)	//如果当前攻击距离超过了魔法的使用最大值，则进入追击
	{
		_state = STATE_TRACE;
		session_npc_follow_target *pSession = new session_npc_follow_target(_self);
		pSession->SetTarget(_target,magic_attack_range*0.9f,_self->GetIgnoreRange(),magic_attack_range * 0.8f);
		pSession->SetChaseInfo(&_chase_info);
		pSession->SetAITask(_apolicy->GetTaskID());
		_self->AddSession(pSession);
		return ;
	}

	bool keep_range = _apolicy->KeepMagicCastRange();
	
	if(range < sa * (0.5f * 0.5f) && _ko_count > 0 && _state != STATE_DODGE && keep_range)
	{
		_state = STATE_DODGE;
		_ko_count --;
		//这时应该躲避
		session_npc_keep_out *pSession = new session_npc_keep_out(_self);
		pSession->SetTarget(_target,magic_attack_range,4);	//假设都是2秒超时
		pSession->SetAITask(_apolicy->GetTaskID());
		_self->AddSession(pSession);
		ClearChaseInfo();
		return ;
	}
	/*
	else if(_state == STATE_MAGIC)
	{
		_state = STATE_DODGE;
		//这是应该等待
		session_npc_delay *pSession = new session_npc_delay(_self);
		pSession->SetTarget(2);	//假设都是2秒超时
		pSession->SetAITask(_apolicy->GetTaskID());
		_self->AddSession(pSession);
		return ;
	} 现在不进行两秒钟等待了
	*/
	else
	{
		//使用法术
		_state = STATE_MAGIC;
		//use magic
		session_npc_skill *pSession = new session_npc_skill(_self);
		pSession->SetTarget(_skill,_skill_level,_target);
		pSession->SetAITask(_apolicy->GetTaskID());
		
		ClearChaseInfo();
		//随机取得下一次技能
		_skill_type = _apolicy->GetPrimarySkill(_skill,_skill_level);
		_skill_range = _self->GetMagicRange(_skill,_skill_level) + _self->GetBodySize();
		_self->AddSession(pSession);
		return ;
	}
}

bool
ai_magic_melee_task::StartTask()
{
	//获得技能攻击距离 
	_skill_type = _apolicy->GetPrimarySkill(_skill,_skill_level);
	if(_skill <=0) return false;
	_skill_range = _self->GetMagicRange(_skill,_skill_level) + _self->GetBodySize();
	return ai_target_task::StartTask();
}

void
ai_magic_melee_task::Execute()
{
	if(_session_id >= 0) return;
	if(_target.type == -1) 
	{
		EndTask();
		return ;
	}
	//如果在session中，则不处理
	//status A
	ai_object::target_info info;
	int target_state;
	float range;
	A3DVECTOR selfpos;
	XID selfxid;

	_self->GetPos(selfpos);
	_self->GetID(selfxid);
	target_state = _self->QueryTarget(_target,info);
	float srange = _self->GetIgnoreRange();
	if(target_state != ai_object::TARGET_STATE_NORMAL 
		|| (range = info.pos.squared_distance(selfpos)) >= srange * srange
		|| _self->GetAntiInvisibleDegree() < info.invisible_degree
			&& _target.id != selfxid.id)
	{
		//目标已经无法再被攻击,任务结束
		_self->RemoveAggroEntry(_target);
		EndTask();
		return;
	}

	if(!_skill_type)
	{
		//如果是祝福技能，则直接使用技能
		//使用法术
		_state = STATE_MAGIC;
		//use magic
		session_npc_skill *pSession = new session_npc_skill(_self);
		pSession->SetTarget(_skill,_skill_level,_target);
		pSession->SetAITask(_apolicy->GetTaskID());

		//随机取得下一次技能
		_skill_type = _apolicy->GetPrimarySkill(_skill,_skill_level);
		_skill_range = _self->GetMagicRange(_skill,_skill_level) + _self->GetBodySize();
		_self->AddSession(pSession);
		return ;
	}


	float magic_attack_range = _skill_range + info.body_size; 
	float sa = magic_attack_range * magic_attack_range;
	
	if(range > sa*0.81f)	//如果当前攻击距离超过了魔法的使用最大值，则进入追击
	{
		_state = STATE_TRACE;
		session_npc_follow_target *pSession = new session_npc_follow_target(_self);
		pSession->SetTarget(_target,magic_attack_range*0.9f,_self->GetIgnoreRange(),magic_attack_range*0.8f);
		pSession->SetChaseInfo(&_chase_info);
		pSession->SetAITask(_apolicy->GetTaskID());
		_self->AddSession(pSession);
		return ;
	}

	float body_size = _self->GetBodySize();
	float pure_attack_range = _self->GetAttackRange() - body_size;
	float attack_range = pure_attack_range * 0.8f + body_size  + info.body_size; 
	sa = attack_range * attack_range;
	float short_range =  body_size + info.body_size + 0.0f;

	if(range > sa || (_state == STATE_PHYSC))
	{
		/*if(_state == STATE_MAGIC)
		{
			_state = STATE_DODGE;
			//这是应该等待
			session_npc_delay *pSession = new session_npc_delay(_self);
			pSession->SetTarget(2);	//假设都是2秒超时
			pSession->SetAITask(_apolicy->GetTaskID());
			_self->AddSession(pSession);
			return ;
		}
			现在不进行两秒钟等待了
		else*/
		{
			//使用法术
			_state = STATE_MAGIC;
			//use magic
			session_npc_skill *pSession = new session_npc_skill(_self);
			pSession->SetTarget(_skill,_skill_level,_target);
			pSession->SetAITask(_apolicy->GetTaskID());

			ClearChaseInfo();
			//随机取得下一次技能
			_skill_type = _apolicy->GetPrimarySkill(_skill,_skill_level);
			_skill_range = _self->GetMagicRange(_skill,_skill_level) + _self->GetBodySize();
			_self->AddSession(pSession);
			return ;
		}
	}
	else
	{
		if (range < short_range*short_range && _state != STATE_DODGE)
		{
			_state = STATE_DODGE;
			//距离过近，要躲避一下
			session_npc_keep_out *pSession = new session_npc_keep_out(_self);
			pSession->SetTarget(_target,attack_range,4);
			pSession->SetAITask(_apolicy->GetTaskID());
			_self->AddSession(pSession);
			ClearChaseInfo();
			return ;
		}
	}

	//进行物理攻击
	_state = STATE_PHYSC;
	//直接进入攻击状态
	session_npc_attack *attack = new session_npc_attack (_self);
	attack->SetTarget(_target,false);
	attack->SetAITask(_apolicy->GetTaskID());
	attack->SetShortRange(short_range);
	attack->SetAttackTimes( (195 + abase::Rand(10,20)) / (_self->GetAttackSpeed() + 1));
	ClearChaseInfo();
	_self->AddSession(attack);
}



void 
ai_rest_task::OnAggro()
{
	//确认重新进行查询操作
	ai_policy * policy = _apolicy;
	//中断当前的session 结束自己 ，重新搜寻任务
	if(_session_id >= 0)
		_self->ClearSession();
	else
		EndTask(); // 由于 ClearSession里可能EndTask所以不能再次调用了
	policy->DeterminePolicy(XID(-1,-1));
}

bool 
ai_rest_task::StartTask()
{
	Execute();
	return true;
}

bool 
ai_rest_task::EndTask()
{
	_apolicy->TaskEnd();
	return true;
}

void
ai_rest_task::OnHeartbeat()
{
	if(_apolicy->InCombat()) 
	{
		ai_policy * policy = _apolicy;
		EndTask();
		policy->DeterminePolicy(XID(-1,-1));
		return;
	}
	if(--_timeout < 0)
	{
		EndTask();
		return;
	}
}

void 
ai_rest_task::OnSessionEnd(int session_id, int reason)
{
	//sessoin_id可能会不一致的，这是由于会有多个session
	if(session_id < _session_id) return;
	//当前sessoin结束,
	//重新回到执行界面

	_session_id = -28;		//在重复执行时，这个session可能会保存上一次sessioin的值，因此现在应该清除

	if(_apolicy->HasNextTask())
	{
		EndTask();
	}
	else
	{
		if(abase::Rand(0.f,1.f) < 0.1f)
			Execute();
		else
			EndTask();
	}
}

void 
ai_rest_task::Execute()
{
	if(_session_id >= 0) return;
	//如果在session中，则不处理

	//休闲操作的执行，首先定位一个位置
	A3DVECTOR pos;
	_self->GetPatrolPos(pos);

	session_npc_cruise *pSession = new session_npc_cruise(_self);
	pSession->SetTarget(pos,8,10.0f);
	pSession->SetAITask(_apolicy->GetTaskID());
	_self->AddSession(pSession);
	return ;
}


bool 
ai_skill_task::StartTask()
{
	session_npc_skill *pSession = new session_npc_skill(_self);
	pSession->SetTarget(_skill,_level,_target);
	pSession->SetAITask(_apolicy->GetTaskID());
	_self->AddSession(pSession);
	return true;
}

void
ai_skill_task::OnSessionEnd(int session_id, int reason)
{
	//sessoin_id可能会不一致的，这是由于会有多个session
	if(session_id < _session_id) return;
	_session_id = -29;
	EndTask();
}

bool 
ai_skill_task::ChangeTarget(const XID & target)
{
	_target = target;
    _self->SetTargetCache(target);
	return true;
}

bool 
ai_silent_task::EndTask()
{
	_apolicy->TaskEnd();
	return true;
}

void 
ai_silent_task::OnHeartbeat()
{
	if(--_timeout <= 0 ) 
	{
		EndTask();
		return;
	}
}

void 
ai_regeneration_task::OnAggro()
{
	//不进入aggro状态，清除之
	_self->ClearAggro();
	_self->ActiveCombatState(false);
	_self->ClearDamageList();
}
bool 
ai_regeneration_task::StartTask()
{
	session_npc_regeneration * pSession = new session_npc_regeneration(_self);
	pSession->SetAITask(_apolicy->GetTaskID());
	_self->AddSession(pSession);
	return true;
}

bool 
ai_regeneration_task::EndTask()
{
	_apolicy->TaskEnd();
	return true;
}

void 
ai_regeneration_task::OnSessionEnd(int session_id, int reason)
{
	_session_id = -30;
	EndTask();
}

void 
ai_regeneration_task::OnHeartbeat()
{
	if(--_timer <= 0 ) 
	{
		EndTask();
		return;
	}
}

//-------------------
void 
ai_returnhome_task::OnAggro()
{
	//不进入aggro状态，清除之
	_apolicy->EnableCombat(false,false);
	_self->ActiveInvisible(true);	
	_self->ClearAggro();
	_self->ActiveCombatState(false);
	_self->ClearDamageList();
}

bool 
ai_returnhome_task::StartTask()
{
	int t = _apolicy->GetInvincibleTimeout();
	if(t > 0)
	{
		object_interface oi(_self->GetImpl());
		oi.SetInvincibleFilter(true,22,false);
	}
	session_npc_patrol *pSession = new session_npc_patrol(_self);
	pSession->SetTarget(_target,20,NULL,true);
	pSession->SetAITask(_apolicy->GetTaskID());
	_self->AddSession(pSession);
	return true;
}

bool 
ai_returnhome_task::EndTask()
{
	object_interface oi(_self->GetImpl());
	oi.SetInvincibleFilter(false,0,false);
	A3DVECTOR pos;
	//if(_self->IsReturnHome(pos, _apolicy->GetReturnHomeRange()))
	_self->GetPos(pos);
	if(_target.squared_distance(pos) > _apolicy->GetReturnHomeRange())
	{
		//如果仍然需要，则强行回家
		_self->ReturnHome(_target,0.f);
	}
	_apolicy->TaskEnd();
	return true;
}

void 
ai_returnhome_task::OnSessionEnd(int session_id, int reason)
{
	_session_id = -31;
	EndTask();
}

void 
ai_returnhome_task::OnHeartbeat()
{
	if(--_timeout <= 0 ) 
	{
		EndTask();
		return;
	}
}


void  
group_minor_policy::HaveRest()
{
	XID leader_id = _self->GetLeaderID();
	//考虑是否休闲动作
	//加入跟随master策略
	if(leader_id.IsActive())
	{
		if(!IsFixStrategy()) AddTargetTask<ai_follow_master>(leader_id);
	}
	else
	{
		ai_policy::HaveRest();
	}
	return;
	
}

float 
group_minor_policy::GetReturnHomeRange() 
{
	XID leader_id = _self->GetLeaderID();
	ai_object::target_info tinfo;
	if(leader_id.IsActive() && ai_object::TARGET_STATE_NORMAL == _self->QueryTarget(leader_id,tinfo)) 
		return 1e20;
	else
		return 10.f*10.f;
}

float 
group_minor_policy::GetMaxMoveRange()
{
	XID leader_id = _self->GetLeaderID();
	if(leader_id.IsActive())
		return 0.f;		//没有移动范围限制
	else
		return _max_move_range*_max_move_range;
}

void 
group_minor_policy::OnHeartbeat()
{
	//有target并且有leader表示是leader(boss)对指定target(玩家)召唤的怪物
	//有target并且没有leader表示是target(玩家)(通过任务等)召唤的怪物
	//在有target情况下要对下列条件进行判断并处理:
	//自身和leader脱离战斗、目标消失(下线或换场景)、距离出生地过远、目标死亡
	const XID& leader_id = _self->GetLeaderID();
	const XID& target_id = _self->GetTargetID();
	bool die_with_leader_checked = false;
	bool die_with_target_checked = false;
	if(target_id.IsActive())
	{
		//自身脱离战斗状态并且leader脱离战斗(用满血近似)，则自身消失
		if(!InCombat())
		{
			if(leader_id.IsActive())
			{
				ai_object::target_info info;
				if(_self->QueryTarget(leader_id,info) == ai_object::TARGET_STATE_NORMAL)
				{
					if(info.hp == info.max_hp)
					{
						XID id;
						_self->GetID(id);
						_self->SendMessage(id,GM_MSG_DEATH);
						_life = 1;
						return ;
					}
				}
				else if(_die_with_who & DIE_WITH_LEADER)	//顺便检查一下队长是不是死了
				{
					//队长已经死亡， 或不存在
					//让自己强行死亡
					XID id;
					_self->GetID(id);
					_self->SendMessage(id,GM_MSG_DEATH);
					_life = 1;
					return ;
				}
				die_with_leader_checked = true;
			}			
		}
		else
		{
			ai_object::target_info info1;
			int target_state = _self->QueryTarget(target_id,info1);
			if(target_state == ai_object::TARGET_STATE_NOT_FOUND)
			{
				//清除对目标的仇恨
				_self->RemoveAggroEntry(target_id);
			}
			else if(target_state == ai_object::TARGET_STATE_NORMAL)
			{
			}
			else if(_die_with_who & DIE_WITH_TARGET)//TARGET_STATE_ZOMBIE		//顺便检查以下目标是不是死了
			{
				//只有目标死亡时才让自己强行死亡
				XID id;
				_self->GetID(id);
				_self->SendMessage(id,GM_MSG_DEATH);
				_life = 1;
				return ;
			}
			die_with_target_checked = true;
				
			if(leader_id.IsActive())
			{
				A3DVECTOR selfpos;
				_self->GetPos(selfpos);
				ai_object::target_info info2;
				if(_self->QueryTarget(leader_id,info2) == ai_object::TARGET_STATE_NORMAL)
				{
					if(info2.pos.horizontal_distance(selfpos) > 1500.f*1500.f)	
					{
						//清仇恨列表
						_self->ClearAggro();
					}
				}
				else if(_die_with_who & DIE_WITH_LEADER)	//顺便检查一下队长是不是死了
				{
					//队长已经死亡， 或不存在
					//让自己强行死亡
					XID id;
					_self->GetID(id);
					_self->SendMessage(id,GM_MSG_DEATH);
					_life = 1;
					return ;
				}
				die_with_leader_checked = true;
			}
		}
	}

	//if(_die_with_leader)
	if(!die_with_leader_checked && _die_with_who & DIE_WITH_LEADER)
	{
		if(leader_id.IsActive())
		{
			ai_object::target_info info3;
			if(_self->QueryTarget(leader_id,info3) != ai_object::TARGET_STATE_NORMAL)
			{
				//队长已经死亡， 或不存在
				//让自己强行死亡
				XID id;
				_self->GetID(id);
				_self->SendMessage(id,GM_MSG_DEATH);
				_life = 1;
				return ;
			}
		}
	}
	
	if(!die_with_target_checked && _die_with_who & DIE_WITH_TARGET)
	{
		if(target_id.IsActive())
		{
			ai_object::target_info info4;
			if(_self->QueryTarget(target_id,info4) == ai_object::TARGET_STATE_ZOMBIE)
			{
				//只有目标死亡时才让自己强行死亡
				XID id;
				_self->GetID(id);
				_self->SendMessage(id,GM_MSG_DEATH);
				_life = 1;
				return ;
			}
		}
	}

	ai_policy::OnHeartbeat();
}

void
ai_follow_master::Execute()
{
}

void
ai_follow_master::OnHeartbeat()
{
	//增加后续task判断，使后续task尽快执行
	if(_apolicy->HasNextTask())
	{
		if(!_self->HasSession())
		{
			EndTask();
		}
		else
		{
			_self->AddSession(new session_npc_empty());
		}
		return;
	}
	//每个心跳都检查是否太耗费了 ?? 
	if(_session_id >= 0) return;
	if(_apolicy->InCombat()) 
	{
		ai_policy * policy = _apolicy;
		EndTask();
		policy->DeterminePolicy(XID(-1,-1));
		return;
	}

	ai_object::target_info info;
	int target_state;
	float range;
	A3DVECTOR selfpos;

	_self->GetPos(selfpos);
	target_state = _self->QueryTarget(_target,info);
	if(target_state != ai_object::TARGET_STATE_NORMAL)
	{
		//队长已经死亡， 或不存在
		//所以不进行什么处理
		//现在要结束task，防止后续task永远不会执行
		EndTask();
		return ;
	}

	range = info.pos.horizontal_distance(selfpos);
	if(range >= MAX_MASTER_MINOR_RANGE)
	{
		//执行回归策略
		_self->ReturnHome(info.pos,7.0f);

		return ;
	}
	if(range  >= 8.f*8.f)
	{
		//执行跟随策略
		session_npc_follow_target *pSession = new session_npc_follow_target(_self);
		pSession->SetTarget(_target,7.0f,20.f,7.0f);
		pSession->SetChaseInfo(&_chase_info);
		pSession->SetAITask(_apolicy->GetTaskID());
		_self->AddSession(pSession);
	}
	else if (_self->CanRest())
	{
		//试验着执行休闲策略
		session_npc_cruise *pSession = new session_npc_cruise(_self);
		pSession->SetTarget(info.pos,6,7.f);
		pSession->SetAITask(_apolicy->GetTaskID());
		_self->AddSession(pSession);
		ClearChaseInfo();
	}
}

void 
ai_follow_master::OnAggro()
{
	//确认重新进行查询操作
	ai_policy * policy = _apolicy;
	//中断当前的session 结束自己 ，重新搜寻任务
	if(_session_id >= 0)
		_self->ClearSession();
	else
		EndTask(); // 由于 ClearSession里可能EndTask所以不能再次调用了
	policy->DeterminePolicy(XID(-1,-1));
}

bool 
ai_follow_master::EndTask()
{
	_apolicy->TaskEnd();
	return true;
}


void 
ai_follow_master::OnSessionEnd(int session_id, int reason)
{
	//sessoin_id可能会不一致的，这是由于会有多个session
	if(session_id < _session_id) return;
	//当前sessoin结束,
	//重新回到执行界面

	_session_id = -32;		//在重复执行时，这个session可能会保存上一次sessioin的值，因此现在应该清除

	if(_apolicy->HasNextTask())
	{
		EndTask();
	}
	else
	{
		if(abase::Rand(0.f,1.f) < 0.1f)
			Execute();
		else
			EndTask();
	}
}

void 
group_boss_policy::OnHeartbeat()
{
	ai_policy::OnHeartbeat();
	TryForwardAggro();
}

void 
ai_patrol_task::OnAggro()
{
	//确认重新进行查询操作
	ai_policy * policy = _apolicy;
	//中断当前的session 结束自己 ，重新搜寻任务
	if(_session_id >= 0)
		_self->ClearSession();
	else
		EndTask(); // 由于 ClearSession里可能EndTask所以不能再次调用了
	policy->DeterminePolicy(XID(-1,-1));
}

bool 
ai_patrol_task::StartTask()
{
	_self->GetPos(_start);
	Execute();
	return true;
}

bool 
ai_patrol_task::EndTask()
{
	A3DVECTOR pos;
	_self->GetPos(pos);
	if(_start.squared_distance(pos) < 1e-3)
	{
		//无法移动，直接走过去 
		_self->ReturnHome(_target,0.0f);
	}
	_apolicy->TaskEnd();
	return true;
}

void
ai_patrol_task::OnHeartbeat()
{
	if(_apolicy->HasNextTask())
	{
		if(!_self->HasSession())
		{
			EndTask();
		}
		else
		{
			_self->AddSession(new session_npc_empty());
		}
		return;
	}
/*	if(padding1 || padding2)
	{
		GLog::log(GLOG_ERR,"ai_patrol_task padding not zero, padding1:%d, padding2:%d", padding1,padding2);
		padding1 = 0;
		padding2 = 0;
	}*/
	if(_apolicy->InCombat()) 
	{
		ai_policy * policy = _apolicy;
		EndTask();
		policy->DeterminePolicy(XID(-1,-1));
		return;
	}
}

void 
ai_patrol_task::OnSessionEnd(int session_id, int reason)
{
	//sessoin_id可能会不一致的，这是由于会有多个session
	if(session_id < _session_id) return;

	//考虑这时如果失败，应该强行移动npc到目标点
	_session_id = -2;
	EndTask();
}

void 
ai_patrol_task::Execute()
{
	if(_session_id >= 0) return;
	//如果在session中，则不处理

	//休闲操作的执行，首先定位一个位置
	session_npc_patrol *pSession = new session_npc_patrol(_self);
	pSession->SetTarget(_target,120,_apolicy->GetPathAgent(),_apolicy->GetSpeedFlag());
	pSession->SetAITask(_apolicy->GetTaskID());
	_self->AddSession(pSession);
	return ;
}

ai_follow_target::ai_follow_target()
{}

ai_follow_target::ai_follow_target(const XID & target):ai_target_task(target)
{}

ai_follow_target::~ai_follow_target()
{
}

bool 
ai_follow_target::StartTask()
{
	Execute();
	return true;
}

void
ai_follow_target::Execute()
{
	session_npc_follow_target *pSession = new session_npc_follow_target(_self);
	pSession->SetTarget(_target,5.0f,60.f,5.0f);
	pSession->SetChaseInfo(&_chase_info);
	pSession->SetAITask(_apolicy->GetTaskID());
	_self->AddSession(pSession);
}

void
ai_follow_target::OnHeartbeat()
{
	//每个心跳都检查是否太耗费了 ?? 
	if(_session_id >= 0) return;
	if(_apolicy->InCombat()) 
	{
		ai_policy * policy = _apolicy;
		EndTask();
		policy->DeterminePolicy(XID(-1,-1));
		return;
	}
}

void 
ai_follow_target::OnAggro()
{
	//确认重新进行查询操作
	ai_policy * policy = _apolicy;
	//中断当前的session 结束自己 ，重新搜寻任务
	if(_session_id >= 0)
		_self->ClearSession();
	else
		EndTask(); // 由于 ClearSession里可能EndTask所以不能再次调用了
	policy->DeterminePolicy(XID(-1,-1));
}

bool 
ai_follow_target::EndTask()
{
	_apolicy->TaskEnd();
	return true;
}

void 
ai_follow_target::OnSessionEnd(int session_id, int reason)
{
	//sessoin_id可能会不一致的，这是由于会有多个session
	if(session_id < _session_id) return;
	//当前sessoin结束,
	//重新回到执行界面

	_session_id = -33;		//在重复执行时，这个session可能会保存上一次sessioin的值，因此现在应该清除

	ai_object::target_info info;
	int target_state;
	float range;
	A3DVECTOR selfpos;

	_self->GetPos(selfpos);
	target_state = _self->QueryTarget(_target,info);
	range = info.pos.horizontal_distance(selfpos);
	if(target_state == ai_object::TARGET_STATE_NORMAL && range < 60.f*60.f)
	{
		if(range  >= 6.f*6.f)
		{
			//执行跟随策略
			Execute();
			return;
		}
	}
	EndTask();
}

bool 
ai_pet_follow_master::StartTask()
{
	Execute();
	return true;
}

void
ai_pet_follow_master::Execute()
{
	session_npc_follow_target *pSession = new session_npc_follow_target(_self);
	float offset = 0.f;
	if(_self->GetInhabitType() != 0) offset = 1.5f;
	pSession->SetTarget(_target,0.8f,62.f,1.0f,(int)(10/NPC_FOLLOW_TARGET_TIME), offset);
	pSession->SetChaseInfo(&_chase_info);
	pSession->SetAITask(_apolicy->GetTaskID());
	_self->AddSession(pSession);
}

void
ai_pet_follow_master::OnHeartbeat()
{
	if(_session_id >= 0) return;
	if(_apolicy->InCombat()) 
	{
		ai_policy * policy = _apolicy;
		EndTask();
		policy->DeterminePolicy(XID(-1,-1));
		return;
	}
	if(!_self->HasSession())
	{
		Execute();
	}
}

void 
ai_pet_follow_master::OnAggro()
{
	//确认重新进行查询操作
	ai_policy * policy = _apolicy;
	//中断当前的session 结束自己 ，重新搜寻任务
	if(_session_id >= 0)
		_self->ClearSession();
	else
		EndTask(); // 由于 ClearSession里可能EndTask所以不能再次调用了
	policy->DeterminePolicy(XID(-1,-1));
}

bool 
ai_pet_follow_master::EndTask()
{
	_apolicy->TaskEnd();
	return true;
}


void 
ai_pet_follow_master::OnSessionEnd(int session_id, int reason)
{
	//sessoin_id可能会不一致的，这是由于会有多个session
	if(session_id < _session_id) return;
	//当前sessoin结束,
	//重新回到执行界面

	_session_id = -34;		//在重复执行时，这个session可能会保存上一次sessioin的值，因此现在应该清除

	switch(reason)
	{
		case NSRC_OUT_OF_RANGE:
		case NSRC_TIMEOUT:
		case NSRC_ERR_PATHFINDING:
			_apolicy->UpdateChaseInfo(&_chase_info);
			_apolicy->FollowMasterResult(1);
			EndTask();
			return ;
	}

	ai_object::target_info info;
	int target_state;
	float range;
	A3DVECTOR selfpos;

	_self->GetPos(selfpos);
	target_state = _self->QueryTarget(_target,info);
	range = info.pos.horizontal_distance(selfpos);
	if(target_state == ai_object::TARGET_STATE_NORMAL && range < 60.f*60.f)
	{
		if(range  >= 6.f*6.f)
		{
			//执行跟随策略
			Execute();
			return;
		}
	}

	_apolicy->UpdateChaseInfo(&_chase_info);
	_apolicy->FollowMasterResult(0);
	EndTask();
}

bool 
ai_pet_skill_task::StartTask()
{
	_skill_range = _self->GetMagicRange(_skill,_level) + _self->GetBodySize();
	Execute();
	return true;
}

bool 
ai_skill_task_2::StartTask()
{
	_skill_range_type = GNET::SkillWrapper::RangeType(_skill);
	_skill_range = _self->GetMagicRange(_skill,_level) + _self->GetBodySize();
	Execute();
	return true;
}

void
ai_skill_task_2::OnSessionEnd(int session_id, int reason)
{
	//sessoin_id可能会不一致的，这是由于会有多个session
	if(session_id < _session_id) return;
	_session_id = -35;
	if(_state != STATE_MAGIC)
	{
		Execute();
	}
	else
	{
		EndTask();
	}
}

bool 
ai_skill_task_2::ChangeTarget(const XID & target)
{
	//_target = target;
	return true;
}

void
ai_skill_task_2::Execute()
{
	if(_session_id >= 0 || (_target.type == -1 && _skill_range_type != 2 && _skill_range_type != 5))
	{
		EndTask();
		return ;
	}

	//如果是自身和自身的球类型的技能，则直接使用 不检查目标状态和类型
	if(_skill_range_type == 2 || _skill_range_type == 5)
	{
		//如果法术是自身或者自身球，直接使用 如果 本身是固定类型的策略，也直接试图使用
		_state = STATE_MAGIC;
		//use magic
		session_npc_skill *pSession = new session_npc_skill(_self);
		pSession->SetTarget(_skill,_level,_target);
		pSession->SetAITask(_apolicy->GetTaskID());
		pSession->SetUseCoolDown(UseCoolDown());
		pSession->SetUseMp(UseMp());
		_self->AddSession(pSession);
		return ;
	}
	
	//如果在session中，则不处理
	//status A
	ai_object::target_info info;
	int target_state;
	float range;
	A3DVECTOR selfpos;
	XID selfxid;

	_self->GetPos(selfpos);
	_self->GetID(selfxid);
	target_state = _self->QueryTarget(_target,info);
	float srange = _self->GetIgnoreRange();
	if(target_state != ai_object::TARGET_STATE_NORMAL 
		|| (range = info.pos.squared_distance(selfpos)) >= srange * srange
		|| _self->GetAntiInvisibleDegree() < info.invisible_degree
			&& _target.id != selfxid.id)
	{
		//目标已经无法再被攻击,任务结束
		_self->RemoveAggroEntry(_target);
		EndTask();
		return;
	}

	if(_apolicy->IsFixStrategy() || _trace_count <= 0)
	{
		//本身是固定类型的策略，也直接试图使用  如果追击达到指定次数，则也直接使用技能
		_state = STATE_MAGIC;
		//use magic
		session_npc_skill *pSession = new session_npc_skill(_self);
		pSession->SetTarget(_skill,_level,_target);
		pSession->SetAITask(_apolicy->GetTaskID());
		pSession->SetUseCoolDown(UseCoolDown());
		pSession->SetUseMp(UseMp());
		_self->AddSession(pSession);
		return ;
	}

	_trace_count --;

	float magic_attack_range = _skill_range + info.body_size; 
	float sa = magic_attack_range * magic_attack_range;
	if(range > sa*0.81f)	//如果当前攻击距离超过了魔法的使用最大值，则进入追击
	{
		_state = STATE_TRACE;
		session_npc_follow_target *pSession = new session_npc_follow_target(_self);
		pSession->SetTarget(_target,magic_attack_range*0.9f,_self->GetIgnoreRange(),magic_attack_range * 0.8f);
		pSession->SetChaseInfo(&_chase_info);
		pSession->SetAITask(_apolicy->GetTaskID());
		_self->AddSession(pSession);
		return ;
	}

	//使用法术
	_state = STATE_MAGIC;
	//use magic
	session_npc_skill *pSession = new session_npc_skill(_self);
	pSession->SetTarget(_skill,_level,_target);
	pSession->SetAITask(_apolicy->GetTaskID());
	pSession->SetUseCoolDown(UseCoolDown());
	pSession->SetUseMp(UseMp());

	ClearChaseInfo();
	_self->AddSession(pSession);
	return ;
}

void 
ai_skill_task_2::OnHeartbeat()
{
	if(!_self->HasSession())
	{
		EndTask();
	}
	else if(_apolicy->HasNextTask())
	{
		_self->AddSession(new session_npc_empty());
		return;
	}
	//这里无视其他的任务
}

void 
ai_play_action_task::OnAggro()
{
	EndTask();
}

bool 
ai_play_action_task::StartTask()
{
	_self->StartPlayAction(_action_name,_play_times,_action_last_time,_interval_time);
	return true;
}

bool 
ai_play_action_task::EndTask()
{
	_self->StopPlayAction();
	return ai_task::EndTask();
}

void 
ai_play_action_task::OnHeartbeat()
{
	if(--_timeout <= 0 ) 
	{
		EndTask();
		return;
	}
}

void ai_policy::CheckNPCData()
{
	if(_cur_task) 
	{
		ASSERT(_cur_task->_apolicy == NULL || (void*)_cur_task->_apolicy != (void*) 0xFFFFFFFF);
	}
}

