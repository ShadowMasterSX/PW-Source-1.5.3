#include "string.h"
#include "world.h"
#include "clstab.h"
#include "npc.h"
#include "ainpc.h"
#include "aiaggro.h"
#include <common/protocol.h>
#include "invincible_filter.h"
#include "usermsg.h"
#include "aitrigger.h"

DEFINE_SUBSTANCE(aggro_policy,substance, CLS_NPC_AGGRO_POLICY)
DEFINE_SUBSTANCE(aggro_minor_policy,aggro_policy, CLS_NPC_MINOR_AGGRO_POLICY)
DEFINE_SUBSTANCE(pet_aggro_policy,aggro_policy, CLS_PET_AGGRO_POLICY)
DEFINE_SUBSTANCE(turret_aggro_policy,aggro_policy, CLS_TURRET_AGGRO_POLICY)

ai_trigger::policy * 
ai_object::CreatePolicy(int id)
{
	const ai_trigger::policy * pTmp = world_manager::GetTriggerMan().GetPolicy(id);
	if(pTmp)
	{
		return new ai_trigger::policy(*pTmp);
	}
	return NULL;
}

bool
ai_npcobject::IsReturnHome(A3DVECTOR & pos, float offset_range)
{
	pos = ((gnpc_imp*)_imp)->_birth_place;
	if(pos.squared_distance(_imp->_parent->pos) > offset_range)
	{
		return true;
	}
	return false;
}

void
ai_npcobject::BeHurt(int hp)
{
	attacker_info_t info ={XID(-1,-1),0,0,0,0,0};
	_imp->BeHurt(XID(-1,-1),info,hp,false,0);
}

void 
ai_npcobject::Say(const XID & target, const void * msg, size_t size, int level, bool anonymous)
{
	gnpc_imp * pImp = (gnpc_imp*)_imp;
	gnpc * pNPC = (gnpc*)pImp->_parent;
	if(pNPC->pPiece)
	{
		SaySomething(pImp->_plane,pNPC->pPiece,msg,size,GMSV::CHAT_CHANNEL_LOCAL,anonymous?0:pNPC->ID.id);
	}
}

int 
ai_npcobject::GetInhabitType()
{
	return ((gnpc_imp*)_imp)->_inhabit_type;
}


void ai_npcobject::BattleFactionSay(const void * msg, size_t size)
{
	gnpc_imp * pImp = (gnpc_imp*)_imp;
	pImp->_plane->BattleFactionSay(pImp->GetFaction(), msg,size);
}

void ai_npcobject::BattleSay(const void * msg, size_t size)
{
	gnpc_imp * pImp = (gnpc_imp*)_imp;
	pImp->_plane->BattleSay(msg,size);
}

void ai_npcobject::BroadcastSay(const void * msg, size_t size, bool is_system)
{
	if(is_system)
		SystemChatMsg(msg, size, GMSV::CHAT_CHANNEL_BROADCAST);
	else
		broadcast_chat_msg(0,msg, size,GMSV::CHAT_CHANNEL_FARCRY,0,0,0);
}

void ai_npcobject::InstanceSay(const void * msg, size_t size, bool middle)
{
	gnpc_imp * pImp = (gnpc_imp*)_imp;
	pImp->_plane->InstanceSay(msg,size,middle);
}

void 
ai_npcobject::AddInvincibleFilter(const XID & who)
{
	filter * f = new invincible_filter_to_spec_id(_imp,1000,20,who);
	_imp->_filters.AddFilter(f);
}

void 
ai_npcobject::ReturnHome(const A3DVECTOR & target,float range)
{
	A3DVECTOR pos(target);
	pos.x += abase::Rand(-range,range);
	pos.z += abase::Rand(-range,range);
	//超过距离10米的时候即回到出生位置
	_imp->_runner->stop_move(pos,0x500,1,C2S::MOVE_MODE_RETURN);
	pos -= _imp->_parent->pos;
	_imp->StepMove(pos);
}

const XID & 
ai_npcobject::GetLeaderID()
{
	return ((gnpc_imp*)_imp)->_leader_id;
}

void 
ai_npcobject::PetRelocatePos(bool is_disappear)
{
	((gnpc_imp*)_imp)->PetRelocatePos(is_disappear);
}

bool 
ai_npcobject::PetGetNearestTeammate(float range, XID & target)
{
	return ((gnpc_imp*)_imp)->PetGetNearestTeammate(range,target);
}

int
ai_npcobject::GetLastDamage()
{
	return ((gnpc_imp*)_imp)->GetLastDamage();
}

XID
ai_npcobject::GetChiefGainer()
{
	return ((gnpc_imp*)_imp)->GetChiefGainer();
}	

XID
ai_npcobject::GetMafiaID()
{
	int mafia = ((gnpc_imp*)_imp)->OI_GetMafiaID();
	return mafia ? XID(GM_TYPE_MAFIA, mafia) : XID(-1,0);
}

void 
ai_npcobject::FestiveAward(int fa_type,int type,const XID & target)
{
	return ((gnpc_imp*)_imp)->FestiveAward(fa_type,type,target);
}

void
ai_npcobject::SetLeaderID(const XID & leader)
{
	((gnpc_imp*)_imp)->_leader_id  = leader;
}

const XID & 
ai_npcobject::GetTargetID()
{
	return ((gnpc_imp*)_imp)->_target_id;
}

void
ai_npcobject::SetTargetID(const XID & target)
{
	((gnpc_imp*)_imp)->_target_id  = target;
}

void 
ai_npcobject::ForwardFirstAggro(const XID & id,int rage)
{
	gnpc_imp * pImp = (gnpc_imp*)_imp;
	pImp->ForwardFirstAggro(id,rage);
	return ;
}

bool
ai_npcobject::CheckWorld()
{
	gnpc_imp * pImp = (gnpc_imp*)_imp;
	return pImp->_plane->w_activestate == 1;
	
}

float 
ai_npcobject::GetSightRange()
{
	return	_ctrl->GetAI()->GetSightRange();
}

int ai_npcobject::GetPetMaster(const XID& target)
{
	world::object_info info;
	if(_imp->_plane->QueryObject(target,info))
	{
		return info.master_id;
	}
	
	return 0;

}

bool 
gnpc_ai::Init(gnpc_controller * pControl,const aggro_param & aggp, const ai_param & aip)
{	
	_commander= pControl;
	ASSERT(_aggro == NULL);
	switch(aggp.aggro_policy)
	{
		case AGGRO_POLICY_NORMAL:
			_aggro = new aggro_policy;
			break;
		case AGGRO_POLICY_1:
			_aggro = new aggro_policy;
			break;
		case AGGRO_POLICY_2:
			_aggro = new aggro_policy;
			break;
		case AGGRO_POLICY_3:
			_aggro = new aggro_policy;
			break;
		case AGGRO_POLICY_BOSS:
			_aggro = new aggro_policy;
			break;
		case AGGRO_POLICY_BOSS_MINOR:
			_aggro = new aggro_minor_policy;
			break;
		case AGGRO_POLICY_PET:
			_aggro = new pet_aggro_policy;
			break;
		case AGGRO_POLICY_TURRET:
			_aggro = new turret_aggro_policy;
			break;

		default:
			ASSERT(false);
	}
	_aggro->Init(aggp);
	substance * pSub = substance::CreateInstance(aip.policy_class);
	ASSERT(pSub->GetRunTimeClass()->IsDerivedFrom(CLASSINFO(ai_policy)));
	_core = (ai_policy*)pSub;
	ai_npcobject self((gactive_imp*)pControl->_imp,pControl,_aggro);
	_core->Init(self,aip);

	_core->SetAITrigger(aip.trigger_policy);

	float body_size = self.GetBodySize();

	//初始化自己的参数
	_sight_range = aggp.sight_range;
	_squared_sight_range = (_sight_range + body_size)* (_sight_range + body_size);
	_faction_ask_help = aggp.faction_ask_help;
	_faction_accept_help = aggp.faction_accept_help;
	return true;
}


void 
ai_npcobject::SendClientTurretNotify(int id)
{
	_imp->_runner->send_turrent_leader(id);
}

void 
ai_npcobject::SummonMonster(int mod_id, int count, const XID& target, int target_distance, int remain_time, char die_with_who, int path_id)
{
	gnpc_imp * pImp = (gnpc_imp*)_imp;
	pImp->SummonMonster(mod_id,count,target,target_distance,remain_time,die_with_who,path_id);	
}

void 
ai_npcobject::StartPlayAction(char action_name[128], int play_times, int action_last_time, int interval_time)
{
	_imp->_runner->start_play_action(action_name,play_times,action_last_time,interval_time);
}

void 
ai_npcobject::StopPlayAction()
{
	_imp->_runner->stop_play_action();
}

void 
ai_npcobject::SetTargetCache(const XID& target)
{
	_imp->SetTargetCache(target);
}

int
ai_npcobject::GetState()
{
	//还有移出世界的状态
	if(_imp->_parent->IsZombie())
	{	
		return STATE_ZOMBIE;
	}
	return STATE_NORMAL;
}


bool 
ai_npcobject::CanRest()
{
	gnpc * pNPC = (gnpc*)_imp->_parent;
	if(pNPC->idle_timer <= 0 ) return false;
	pNPC->cruise_timer = (pNPC->cruise_timer - 1) & (32 - 1);
	return (pNPC->cruise_timer == 0);
}

bool 
ai_npcobject::IsInIdleHeartbeat()
{
	gnpc * pNPC = (gnpc*)_imp->_parent;
	return (pNPC->idle_timer <= 0 );
}

void 
ai_npcobject::GetPatrolPos(A3DVECTOR & pos)
{
	pos = ((gnpc_imp*)_imp)->_birth_place;

}

void 
ai_npcobject::GetPatrolPos(A3DVECTOR & pos,float range)
{
	pos.x += abase::Rand(-range,range);
	pos.z += abase::Rand(-range,range);
	pos.y = _imp->_plane->GetHeightAt(pos.x, pos.z);
}

bool 
gnpc_ai::Save(archive & ar)
{
	ar << _sight_range << _squared_sight_range << _faction_ask_help << _faction_accept_help;
	if(_aggro) 
		_aggro->SaveInstance(ar);
	else 
		ar << -1;

	if(_core)
		_core->SaveInstance(ar);
	else
		ar << -1;
	return true;
}

bool 
gnpc_ai::Load(archive & ar)
{
	ar >> _sight_range >> _squared_sight_range >> _faction_ask_help >> _faction_accept_help;
	int guid;
	ar >> guid;
	ASSERT(_aggro == NULL && _core == NULL);
	if(guid >= 0)
	{
		_aggro = substance::LoadSpecInstance<aggro_policy>(guid,ar);
	}
	ar >> guid;
	if(guid >= 0)
	{
		_core = substance::LoadSpecInstance<ai_policy>(guid,ar);
	}
	_commander = NULL;
	ASSERT(_aggro && _core);
	return _aggro && _core;
}

void 
gnpc_ai::Heartbeat()
{
	
#ifdef __TEST_PERFORMANCE__
	int flag = 3;
#endif
	_aggro->OnHeartbeat();
	if(_aggro->Size() && g_timer.get_systime() % 2 == 0) //每两秒才会发一次
	{
		XID target;
		_aggro->GetFirst(target);
		//if(target.type != GM_TYPE_NPC) $$$$$$  宠物也能接受此消息，所以不再判断是否NPC了
		{
			ai_npcobject obj((gactive_imp*)_commander->_imp,_commander,_aggro);
			obj.HateTarget(target);
		}

		/*
		//给所有敌人发送HATE消息
		abase::vector<XID> list;
		list.reserve(_aggro->Size());
		_aggro->GetAll(list);
		gactive_imp * imp = (gactive_imp*)_commander->_imp;
		MSG msg;
		BuildMessage(msg,GM_MSG_HATE_YOU,XID(-1,-1),imp->_parent->ID,imp->_parent->pos);
		imp->_plane->SendMessage(list.begin(),list.end(),msg);
		*/
	}
	else
	{
#ifdef __TEST_PERFORMANCE__
		int flag = 15;
#endif
	}
	_core->OnHeartbeat();
#ifdef __TEST_PERFORMANCE__
	if(abase::Rand(0,flag) == 0)
	{
		gactive_imp * imp = (gactive_imp*)_commander->_imp;
		gnpc * pNPC = (gnpc*)imp->_parent;
		MSG msg;
		msg_watching_t mwt= {1,imp->GetFaction(),pNPC->invisible_degree};
		BuildMessage(msg,GM_MSG_WATCHING_YOU,XID(GM_TYPE_NPC,-1),pNPC->ID,pNPC->pos,0,&mwt,sizeof(mwt));
		float tmp = world_manager::GetMaxMobSightRange();
		imp->_plane->BroadcastMessage(msg,tmp,gobject::MSG_MASK_PLAYER_MOVE);
	}
#endif
}


