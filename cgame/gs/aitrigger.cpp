#include "string.h"
#include "world.h"
#include "config.h"
#include "aitrigger.h"
#include "aipolicy.h"
#include <arandomgen.h>
#include "actobject.h"
#include "ai/policytype.h"
#include <common/chatdata.h>

namespace ai_trigger
{

class logic_val
{
	enum
	{
		INVALID_LOGIC_VAL = 0x7FFFFFFF, // 计算过程中产生的非法值
	};

	int _type;
	int _index;
	gactive_imp* _imp;
	mutable int _val;
public:	
	logic_val(int t, int i, gactive_imp* imp) : _type(t),_index(i),_imp(imp),_val(INVALID_LOGIC_VAL){}

	int getval() const
	{
		if(_val != INVALID_LOGIC_VAL) return _val;
		switch(_type)
		{
			case enumPVTGlobalVarID:
				if(_imp) _val = _imp->_plane->GetCommonValue(_index); 
				break;
			case enumPVTLocalVarID:
				if(_imp) _val = _imp->GetLocalVal(_index);
				break;
			case enumPVTConst:
				_val = _index;
				break;
			case enumPVTRandom:
				_val = abase::RandNormal(0,_index);
				break;
		}
		return _val;
	}
	void setval(int val)
	{
		if(val == INVALID_LOGIC_VAL) return;
		
		switch(_type)
		{
			case enumPVTGlobalVarID:
				if(_imp) _imp->_plane->SetCommonValue(_index, val); 
				break;
			case enumPVTLocalVarID:
				if(_imp) _imp->SetLocalVal(_index, val);
				break;
		}		
	}
	static int calc(const logic_val& lv1, int op, const logic_val& lv2)
	{
		int val1 = lv1.getval();
		int val2 = lv2.getval();
		if(val1 == INVALID_LOGIC_VAL || val2 == INVALID_LOGIC_VAL) return INVALID_LOGIC_VAL;

		switch(op)
		{
			case enumPOTAdd:
				return val1 + val2;
			case enumPOTSub:
				return val1 - val2;
			case enumPOTMul:
				return val1 * val2;
			case enumPOTDiv:
				if(val2 == 0) return INVALID_LOGIC_VAL;
				return val1 / val2;
			case enumPOTMod:
				if(val2 == 0) return INVALID_LOGIC_VAL;
				return val1 % val2;
		}

		return INVALID_LOGIC_VAL;
	}	

};

bool 
cond_hp_less::Check(policy * self, int)
{
	ai_object * ps = self->GetAIObject();
	return ps->GetHP() < ps->GetMaxHP() * _ratio ;
}

bool 
cond_timer::Check(policy * self, int)
{
	return self->CheckTimer(_timer_id);
}

bool 
cond_random::Check(policy * self, int)
{
	return  abase::RandUniform() < _rate;
}
//lgc
bool cond_time_point::Check(policy * self, int)
{
	time_t t1 = time(NULL);	
	struct tm tm1;
	localtime_r(&t1, &tm1);
	return tm1.tm_hour == _hour && tm1.tm_min == _minute;
}

bool cond_on_damage::Check(policy * self, int)
{
	int damage = self->GetAIObject()->GetLastDamage();
	return damage >= _min_damage && damage <= _max_damage;
}

bool cond_path_end::Check(policy * self, int path_id)
{
	int path = world_manager::GetPathMan().IdConvert(_global_path_id);
	return path_id == path;
}

bool cond_path_end_2::Check(policy * self, int path_id)
{
	logic_val lv_pathid(_type,_index,self->GetAIObject()->GetImpl());
	int global_path_id = lv_pathid.getval();
	int path = world_manager::GetPathMan().IdConvert(global_path_id);
	return path_id == path;
}

bool cond_at_history_stage::Check(policy * self, int)
{
	return world_manager::GetHistoryMan().GetStageVersion() == _history_stage_id;
}

bool cond_spec_filter::Check(policy * self, int)
{
	return self->GetAIObject()->GetImpl()->_filters.IsFilterExist(spec_filter_id);
}

int expr_common_data::GetValue(policy *self)
{
	return self->GetAIObject()->GetImpl()->_plane->GetCommonValue(_key);
}

int expr_history_value::GetValue(policy *self)
{
	UDOctets val(0);
	world_manager::GetUniqueDataMan().GetData(_key,val);
	return val;
}

int expr_local_value::GetValue(policy *self)
{
	return self->GetAIObject()->GetImpl()->GetLocalVal(_key);
}

int expr_room_index::GetValue(policy *self)
{
	return self->GetAIObject()->GetImpl()->GetMazeRoomIndex();
}
//---------后面是目标选择策略-------
void 
target_self::GetTarget(policy * self, XID & target)
{
	ai_object * ps = self->GetAIObject();
	ps->GetID(target);
}

void 
target_aggro_first::GetTarget(policy * self, XID & target)
{
	ai_object * ps = self->GetAIObject();
	if(ps->GetAggroCount() > 0)
	{
		ps->GetAggroEntry(0,target);
	}
	else 
	{
		target.type = -1;
		target.id  = -1;
	}
}

void 
target_aggro_second::GetTarget(policy * self, XID & target)
{
	ai_object * ps = self->GetAIObject();
	int count = ps->GetAggroCount();
	if(count > 1)
	{
		ps->GetAggroEntry(1,target);
	}
	else if(count > 0)
	{
		ps->GetAggroEntry(0,target);
	}
	else 
	{
		target.type = -1;
		target.id  = -1;
	}
}

void 
target_aggro_second_rand::GetTarget(policy * self, XID & target)
{
	ai_object * ps = self->GetAIObject();
	int count = ps->GetAggroCount();
	if(count > 1)
	{
		int index = abase::Rand(1, count -1);
		ps->GetAggroEntry(index,target);
	}
	else if(count > 0)
	{
		ps->GetAggroEntry(0,target);
	}
	else 
	{
		target.type = -1;
		target.id  = -1;
	}
}

void 
target_aggro_special::GetTarget(policy * self, XID & target)
{
	ai_object * ps = self->GetAIObject();
	int count = ps->GetAggroCount();
	
	target.type = -1;
	target.id  = -1;
	
	if(count > 0)
	{
		if(ATAS_RAND == _type)
			ps->GetAggroEntry(abase::Rand(0, count -1),target);
		else
		{
			float dis = -1.f;
			A3DVECTOR selfpos = ps->GetImpl()->_parent->pos;	
			
			for(int n = 0; n < count; ++n)
			{
				XID xid;
				world::object_info info;
				ps->GetAggroEntry(n,xid);
				if(ps->GetImpl()->_plane->QueryObject(xid,info))
				{
					float temp = selfpos.horizontal_distance(info.pos);
					if(dis < 0 || (_type == ATAS_NEAR && temp < dis) || (_type == ATAS_FAR && temp > dis) )
					{
						target = xid;
						dis = temp;
					}
				}
			}
		}
	}
}

void 
target_least_hp::GetTarget(policy * self, XID & target)
{
	//这里需要扫描所有的玩家，提出最小hp的玩家
	ai_object * ps = self->GetAIObject();
	int count = ps->GetAggroCount();
	target.type = -1;
	target.id = -1;
	if(count <= 0) return;

	int max_hp = 0x7FFFFFFF;
	for(int i = 0; i < count ; i ++)
	{
		ai_object::target_info info;
		XID id;
		ps->GetAggroEntry(i,id);
		if(ps->QueryTarget(id, info) == ai_object::TARGET_STATE_NORMAL)
		{
			if(info.hp < max_hp)
			{
				target = id;
				max_hp = info.hp;
			}
		}
	}
}

void 
target_most_hp::GetTarget(policy * self, XID & target)
{
	ai_object * ps = self->GetAIObject();
	int count = ps->GetAggroCount();
	target.type = -1;
	target.id = -1;
	if(count <= 0) return;

	int min_hp = -1;
	for(int i = 0; i < count ; i ++)
	{
		ai_object::target_info info;
		XID id;
		ps->GetAggroEntry(i,id);
		if(ps->QueryTarget(id, info) == ai_object::TARGET_STATE_NORMAL)
		{
			if(info.hp > min_hp)
			{
				target = id;
				min_hp = info.hp;
			}
		}
	}
}

void 
target_most_mp::GetTarget(policy * self, XID & target)
{
	ai_object * ps = self->GetAIObject();
	int count = ps->GetAggroCount();
	target.type = -1;
	target.id = -1;
	if(count <= 0) return;

	int min_mp = -1;
	for(int i = 0; i < count ; i ++)
	{
		XID id;
		ps->GetAggroEntry(i,id);
		ai_object::target_info info;
		if(ps->QueryTarget(id, info) == ai_object::TARGET_STATE_NORMAL)
		{
			if(info.mp > min_mp)
			{
				target = id;
				min_mp = info.mp;
			}
		}
	}
}

void 
target_class_combo::GetTarget(policy * self, XID & target)
{
	ai_object * ps = self->GetAIObject();
	int count = ps->GetAggroCount();
	target.type = -1;
	target.id = -1;
	if(count <= 0) return;
	
	XID target_list[16];
	int index = 0;

	for(int i = 0; i < count ; i ++)
	{
		XID id;
		ps->GetAggroEntry(i,id);
		if(!id.IsPlayer()) continue;
		ai_object::target_info info;
		if(ps->QueryTarget(id, info) == ai_object::TARGET_STATE_NORMAL)
		{
			int race = info.race & 0x7FFFFFFF;
			if((1 << race) & _combo_state)
			{
				//符合条件
				target_list[index] = id;
				index ++;
				if(index >= (int)(sizeof(target_list)/sizeof(XID)))
				{
					break;
				}
			}
		}
	}
	if(index > 0)
	{
		target = target_list[abase::Rand(0,index-1)];
	}
	else
	{
		ps->GetAggroEntry(0,target);
	}
}

void 
target_monster_killer::GetTarget(policy * self, XID & target)
{
	ai_object * ps = self->GetAIObject();
	if(ps) target = ps->GetChiefGainer();	
}

void
target_monster_birthplace_faction::GetTarget(policy * self, XID & target)
{
	ai_object * ps = self->GetAIObject();
	if(ps) target = ps->GetMafiaID();
}	

bool 
op_attack::DoSomething(policy * self)
{
	ai_policy * pAI = self->GetAIPolicy();
	XID target;
	_target->GetTarget(self,target);
	if(!target.IsActive())
	{
		if(!pAI->DetermineTarget(target))
		{
			target.type = -1;
		}
	}
	if(target.IsActive())
	{
		pAI->AddPrimaryTask(target,_attack_strategy);
	}
	return true;
}

bool 
op_skill::DoSomething(policy * self)
{
	ai_policy * pAI = self->GetAIPolicy();
	XID target;
	_target->GetTarget(self,target);
	if(!target.IsActive())
	{
		if(!pAI->DetermineTarget(target))
		{
			target.type = -1;
		}
	}
	if(target.IsActive())
	{
		 ai_task * pTask = new ai_skill_task_2(target, _skill_id, _skill_lvl);
		 pTask->Init(self->GetAIObject(),pAI);
		 pAI->AddTask(pTask);
	}
	return true;
}

bool 
op_skill_2::DoSomething(policy * self)
{
	ai_policy * pAI = self->GetAIPolicy();
	XID target;
	_target->GetTarget(self,target);
	if(!target.IsActive())
	{
		if(!pAI->DetermineTarget(target))
		{
			target.type = -1;
		}
	}
	if(target.IsActive())
	{
		logic_val lv_skillid(_skill_id_type,_skill_id,self->GetAIObject()->GetImpl()); 
		logic_val lv_skilllvl(_skill_lvl_type,_skill_lvl,self->GetAIObject()->GetImpl()); 
		ai_task * pTask = new ai_skill_task_2(target, lv_skillid.getval(), lv_skilllvl.getval());
		pTask->Init(self->GetAIObject(),pAI);
		pAI->AddTask(pTask);
	}
	return true;
}

bool 
op_flee::DoSomething(policy * self)
{
	ai_policy * pAI = self->GetAIPolicy();
	XID target;
	_target->GetTarget(self,target);
	if(!target.IsActive())
	{
		if(!pAI->DetermineTarget(target))
		{
			target.type = -1;
		}
	}
	if(target.IsActive())
	{
		 ai_task * pTask = new ai_runaway_task(target);
		 pTask->Init(self->GetAIObject(),pAI);
		 pAI->AddTask(pTask);
	}
	return true;
}

bool 
op_create_timer::DoSomething(policy * self)
{
	self->CreateTimer(_timerid,_interval, _count);
	return true;
}

bool 
op_remove_timer::DoSomething(policy * self)
{
	self->RemoveTimer(_timerid);
	return true;
}

bool 
op_enable_trigger::DoSomething(policy * self)
{
	self->EnableTrigger(_trigger_id,_is_enable);
	return true;
}

bool 
op_exec_trigger::DoSomething(policy * self)
{
	return _trigger->TestTrigger(self);
}

bool 
op_say::DoSomething(policy * self)
{	
	ai_object * ps = self->GetAIObject();
	XID target = XID(-1,-1);
	if(_target) _target->GetTarget(self,target);
	if(_size > 4)
	{
		//检测是否专用语言
		const short * p = (short*)_msg;
		size_t size = _size - sizeof(short) * 2;
		if(p[0] == '$' && p[1] == 'F')
		{
			//城战发言本派 三个均为广播 channel
			ps->BattleFactionSay(&p[2],size);
			return true;
		}
		else if(p[0] == '$' && p[1] == 'T')
		{
			//城战广播发言 三个均为广播 channel
			ps->BattleSay(&p[2],size);
			return true;
		}
		else if(p[0] == '$' && p[1] == 'B')
		{	
			//广播发言 三个均为广播 channel
			ps->BroadcastSay(&p[2],size,false);
			return true;
		}
		else if(p[0] == '$' && p[1] == 'S')
		{	
			//系统发言
			ps->BroadcastSay(&p[2],size,true);
			return true;
		}
		else if(p[0] == '$' && p[1] == 'A')
		{	
			ps->Say(target,&p[2],size,0,true);
			return true;
		}
		else if(p[0] == '$' && p[1] == 'I')
		{
			ps->InstanceSay(&p[2],size);
			return true;
		}
		else if(p[0] == '$' && p[1] == 'X')
		{
			//用来通知此副本world里的玩家在屏幕中间显示消息,只对副本有效
			ps->InstanceSay(&p[2],size,true);
			return true;
		}
	}
	ps->Say(target,_msg,_size,0);
	return true;
}

bool 
op_say_2::DoSomething(policy * self)
{	
	gactive_imp* imp = self->GetAIObject()->GetImpl(); 
	XID target = XID(-1,-1);
	if(_target) _target->GetTarget(self,target);
	
	packet_wrapper h1(64);
	h1 << (short) CHAT_S2C::CHAT_AIPOLICY_VALUE << _mask;
	if(_mask & 0x1) h1 << target.id;
	if(_mask & 0x2) h1 << imp->GetLocalVal(0);
	if(_mask & 0x4) h1 << imp->GetLocalVal(1);
	if(_mask & 0x8) h1 << imp->GetLocalVal(2);
	
	if(_size > 4)
	{
		//检测是否专用语言
		const short * p = (short*)_msg;
		size_t size = _size - sizeof(short) * 2;

		if(p[0] == '$' && p[1] == 'B')
		{	
			//广播发言 三个均为广播 channel
			broadcast_chat_msg(0,&p[2],size,GMSV::CHAT_CHANNEL_FARCRY,0,h1.data(),h1.size());
			return true;
		}
		else if(p[0] == '$' && p[1] == 'S')
		{	
			//系统发言
			broadcast_chat_msg(0,&p[2],size,GMSV::CHAT_CHANNEL_BROADCAST,0,h1.data(),h1.size());
			return true;
		}
		else if(p[0] == '$' && p[1] == 'A')
		{	
			AutoBroadcastChatMsg(imp->_plane,imp->_parent->pPiece,&p[2],size,GMSV::CHAT_CHANNEL_LOCAL,
					0,h1.data(),h1.size(),0,0,0);
			return true;
		}
		else if(p[0] == '$' && p[1] == 'I')
		{
			imp->_plane->InstanceSay(&p[2],size,false,h1.data(),h1.size());
			return true;
		}
		else if(p[0] == '$' && p[1] == 'X')
		{
			//用来通知此副本world里的玩家在屏幕中间显示消息,只对副本有效
			imp->_plane->InstanceSay(&p[2],size,true,h1.data(),h1.size());
			return true;
		}
	}
	AutoBroadcastChatMsg(imp->_plane,imp->_parent->pPiece,_msg,_size,GMSV::CHAT_CHANNEL_LOCAL,
					0,h1.data(),h1.size(),imp->_parent->ID.id,0,0);
	return true;
}

bool 
op_reset_aggro::DoSomething(policy * self)
{
	ai_object * ps = self->GetAIObject();
	ps->RegroupAggro();
	return true;
}

bool 
op_swap_aggro::DoSomething(policy * self)
{
	ai_object * ps = self->GetAIObject();
	ps->SwapAggro(_index1,_index2);
	return true;
}

bool 
op_be_taunted::DoSomething(policy * self)
{
	ai_object * ps = self->GetAIObject();
	XID target;
	_target->GetTarget(self,target);
	if(target.IsActive())
	{
		ps->BeTaunted(target);
	}
	return true;
}

bool 
op_fade_target::DoSomething(policy * self)
{
	ai_object * ps = self->GetAIObject();
	XID target;
	_target->GetTarget(self,target);
	if(target.IsActive())
	{
		ps->FadeTarget(target);
	}
	return true;
}

bool 
op_aggro_fade::DoSomething(policy * self)
{
	ai_object * ps = self->GetAIObject();
	ps->AggroFade();
	return true;
}

bool
op_active_spawner::DoSomething(policy * self)
{
	if(_ctrl_id <= 0) return true;

	int local_ctrl_id = _ctrl_id;
	self->GetAIObject()->GetImpl()->AdjustLocalControlID(local_ctrl_id);

	if(_is_active_spawner)
	{
		self->GetAIObject()->GetImpl()->_plane->TriggerSpawn(local_ctrl_id);
	}
	else
	{
		self->GetAIObject()->GetImpl()->_plane->ClearSpawn(local_ctrl_id);
	}
	return true;
}

bool
op_active_spawner_2::DoSomething(policy * self)
{
	logic_val lv_ctrlid(_ctrl_id_type,_ctrl_id,self->GetAIObject()->GetImpl());
	int local_ctrl_id = lv_ctrlid.getval();
	if(local_ctrl_id <= 0) return true;
	self->GetAIObject()->GetImpl()->AdjustLocalControlID(local_ctrl_id);

	if(_is_active_spawner)
	{
		self->GetAIObject()->GetImpl()->_plane->TriggerSpawn(local_ctrl_id);
	}
	else
	{
		self->GetAIObject()->GetImpl()->_plane->ClearSpawn(local_ctrl_id);
	}
	return true;
}

bool op_set_common_data::DoSomething (policy *self)
{   
	if(_is_value)
		self->GetAIObject()->GetImpl()->_plane->SetCommonValue(_key, _set_value);
	else
	{
		int value = self->GetAIObject()->GetImpl()->_plane->GetCommonValue(_set_value);
		self->GetAIObject()->GetImpl()->_plane->SetCommonValue(_key, value);
	}
	return true;
}

bool op_add_common_data::DoSomething (policy *self)
{
	__PRINTF("%d modify %d value %d\n", self->GetAIObject()->GetImpl()->_parent->ID.id,  _key, _add_value);
	self->GetAIObject()->GetImpl()->_plane->ModifyCommonValue(_key, _add_value);
	return true;
}

bool op_summon_monster::DoSomething(policy *self)
{
	ai_object * ps = self->GetAIObject();
	XID target;
	_target->GetTarget(self,target);
	if(!target.IsActive()) return false;
	
	if(_count > 10 || _count < 1) return false;
	if(_remain_time < 0) _remain_time = 0;
	int path_id = world_manager::GetPathMan().IdConvert(_path_id);
	ps->SummonMonster(_mob_id,_count,target,_target_distance,_remain_time,_die_with_who,path_id);
	return true;
}

bool op_summon_monster_2::DoSomething(policy *self)
{
	ai_object * ps = self->GetAIObject();
	XID target;
	_target->GetTarget(self,target);
	if(!target.IsActive()) return false;
	
	logic_val lv_count(_count_type,_count,ps->GetImpl());
	logic_val lv_path(_path_id_type,_path_id,ps->GetImpl());
	logic_val lv_mobid(_mob_id_type,_mob_id,ps->GetImpl());
	
	int count = lv_count.getval();

	if(count > 10 || count < 1) return false;
	if(_remain_time < 0) _remain_time = 0;
	int path_id = world_manager::GetPathMan().IdConvert(lv_path.getval());
	ps->SummonMonster(lv_mobid.getval(),count,target,_target_distance,_remain_time,_die_with_who,path_id);
	return true;
}

bool op_summon_npc::DoSomething(policy *self)
{
	ai_object * ps = self->GetAIObject();
	XID target;
	_target->GetTarget(self,target);
	if(!target.IsActive()) return false;
	
	logic_val lv_count(_count_type,_count,ps->GetImpl());
	logic_val lv_path(_path_id_type,_path_id,ps->GetImpl());
	logic_val lv_npcid(_npc_id_type,_npc_id,ps->GetImpl());
	logic_val lv_remain(_remain_time_type,_remain_time,ps->GetImpl());
	
	int count = lv_count.getval();
	int remain = lv_remain.getval();

	if(count > 10 || count < 1) return false;
	if(remain < 0) remain = 0;
	int path_id = world_manager::GetPathMan().IdConvert(lv_path.getval());

	object_interface obj(ps->GetImpl());
	object_interface::npc_param nparam = {lv_npcid.getval(), remain, path_id};

	for(int n = 0; n < count; ++n)
		obj.CreateNPC(nparam, target, _target_distance);

	return true;
}

bool op_summon_mine::DoSomething(policy *self)
{
	ai_object * ps = self->GetAIObject();
	XID target;
	_target->GetTarget(self,target);
	if(!target.IsActive()) return false;

	logic_val lv_count(_count_type,_count,ps->GetImpl());
	logic_val lv_remain(_remain_time_type,_remain_time,ps->GetImpl());
	logic_val lv_mineid(_mine_id_type,_mine_id,ps->GetImpl());
	
	int count = lv_count.getval();
	int remain = lv_remain.getval();

	if(count > 10 || count < 1) return false;
	if(remain < 0) remain = 0;

	object_interface obj(ps->GetImpl());
	object_interface::mine_param mparam = {lv_mineid.getval(), remain, true};

	for(int n = 0; n < count; ++n)
		obj.CreateMine(mparam, target, _target_distance);

	return true;
}

bool op_change_path::DoSomething(policy * self)
{
	if(world_manager::GetWorldTag() != _world_tag) return false;
	ai_policy * pAI = self->GetAIPolicy();
	int path_id = world_manager::GetPathMan().IdConvert(_global_path_id);
	pAI->ChangePath(path_id,_path_type,_speed_flag);
	return true;
}

bool op_change_path_2::DoSomething(policy * self)
{
	if(world_manager::GetWorldTag() != _world_tag) return false;
	ai_policy * pAI = self->GetAIPolicy();
	logic_val lv_pathid(_global_path_id_type,_global_path_id,self->GetAIObject()->GetImpl());
	int path_id = world_manager::GetPathMan().IdConvert(lv_pathid.getval());
	pAI->ChangePath(path_id,_path_type,_speed_flag);
	return true;
}

bool op_play_action::DoSomething(policy * self)
{
	ai_policy * pAI = self->GetAIPolicy();
	ai_task * pTask = new ai_play_action_task(_action_name,_play_times,_action_last_time,_interval_time);
	pTask->Init(self->GetAIObject(),pAI);
	pAI->AddTask(pTask);
	return true;
}

bool op_revise_history::DoSomething(policy * self)
{
	return world_manager::GetUniqueDataMan().ModifyData(_key,_value,false) >= 0;
}

bool op_set_history::DoSomething(policy * self)
{
	return world_manager::GetUniqueDataMan().ModifyData(_key,_value,true) >= 0;
}

bool op_deliver_faction_pvp_points::DoSomething(policy * self)
{
	ai_object * ps = self->GetAIObject();
	XID target;
	_target->GetTarget(self,target);
	if(target.IsErrorType()) 
	{
		GLog::log(GLOG_ERR,"NPC%d在发起帮派pvp奖励%d 失败",((gnpc*)ps->GetImpl()->_parent)->tid,_type+1);
		return false;
	}

	ps->FestiveAward(FAT_MAFIA_PVP,_type + 1,target); // 编辑器从0开始
	return true;
}

bool op_deliver_task::DoSomething(policy * self)
{
	XID target;
	_target->GetTarget(self,target);
	if(!target.IsPlayer())
	{		
		GLog::log(GLOG_ERR,"NPC%d发给非角色任务%d"	,((gnpc*)self->GetAIObject()->GetImpl()->_parent)->tid, _taskid);
		return false;
	}
	logic_val lv_taskid(_taskidtype,_taskid,self->GetAIObject()->GetImpl());
	self->GetAIObject()->GetImpl()->DeliverTaskToTarget(target,lv_taskid.getval());	
	return true;
}

bool op_calc_var::DoSomething(policy * self)
{
	logic_val lv_dest(_dtype,_dest,self->GetAIObject()->GetImpl());
	logic_val lv_src1(_stype1,_src1,self->GetAIObject()->GetImpl());
	logic_val lv_src2(_stype2,_src2,self->GetAIObject()->GetImpl());
		
	lv_dest.setval(logic_val::calc(lv_src1,_op,lv_src2));

	return true;	
}

bool op_deliver_random_task_in_region::DoSomething(policy * self)
{
	gactive_imp* pImp = self->GetAIObject()->GetImpl();
	MSG msg;
	BuildMessage(msg, GM_MSG_DELIVER_STORAGE_TASK, XID(GM_TYPE_PLAYER,0),
			pImp->_parent->ID,pImp->_parent->pos,_task_storageid);

	pImp->_plane->BroadcastLocalBoxMessage(msg,_rect);

	return true;
}

bool op_deliver_task_in_dmglist::DoSomething(policy* self)
{
	gactive_imp* pImp = self->GetAIObject()->GetImpl();
	logic_val lv_taskid(_ttype,_taskid,pImp);
	pImp->DispatchTaskToDmgList(lv_taskid.getval(),_count,_distance);
	return true;
}

}

