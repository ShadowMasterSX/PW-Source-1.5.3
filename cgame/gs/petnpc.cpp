#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <common/protocol.h>
#include <arandomgen.h>
#include "string.h"
#include "world.h"
#include "npc.h"
#include "usermsg.h"
#include "clstab.h"
#include "ainpc.h"
#include "actsession.h"
#include "npcgenerator.h"
#include "item.h"
#include "playertemplate.h"
#include "npc_filter.h"
#include "pathfinding/pathfinding.h"
#include "guardnpc.h"
#include "petnpc.h"
#include "player_imp.h"
#include "petdataman.h"
#include "pet_filter.h"
#include "sfilterdef.h"
#include "npcsession.h"
#include "pvplimit_filter.h"

DEFINE_SUBSTANCE(gpet_imp,gnpc_imp,CLS_PET_IMP)
DEFINE_SUBSTANCE(gpet_imp_2,gpet_imp,CLS_PET_IMP_2)
DEFINE_SUBSTANCE(gpet_plant_imp,gpet_imp,CLS_PLANT_PET_IMP)
DEFINE_SUBSTANCE(gpet_dispatcher,gnpc_dispatcher,CLS_PET_DISPATCHER)
DEFINE_SUBSTANCE(gpet_policy,ai_policy,CLS_NPC_AI_POLICY_PET)
DEFINE_SUBSTANCE(gpet_plant_policy,gpet_policy,CLS_NPC_AI_POLICY_PLANT_PET)

void 
gpet_dispatcher::enter_slice(slice *pPiece ,const A3DVECTOR &pos)
{
	packet_wrapper  h1(64);
	using namespace S2C;
	gnpc* pNPC = (gnpc*)_imp->_parent;
	CMD::Make<CMD::npc_enter_slice>::From(h1,pNPC,pos);

	cs_user_map map; 
	pPiece->Lock();
	if(pNPC->IsInvisible())		//如主人在这个slice，则会向主人发
		gather_slice_cs_user_in_invisible(pPiece,map,pNPC->invisible_degree,0,((gpet_imp *)_imp)->_leader_id.id);
	else
		gather_slice_cs_user(pPiece,map);
	pPiece->Unlock();
	if(map.size()) multi_send_ls_msg(map,h1);
}

void 
gpet_dispatcher::leave_slice(slice *pPiece ,const A3DVECTOR &pos)
{
	packet_wrapper  h1(64);
	using namespace S2C;
	gnpc* pNPC = (gnpc*)_imp->_parent;
	CMD::Make<CMD::leave_slice>::From(h1,pNPC);
	
	cs_user_map map; 
	pPiece->Lock();
	if(pNPC->IsInvisible())		//如主人在这个slice，则会向主人发
		gather_slice_cs_user_in_invisible(pPiece,map,pNPC->invisible_degree,0,((gpet_imp *)_imp)->_leader_id.id);
	else
		gather_slice_cs_user(pPiece,map);
	pPiece->Unlock();
	if(map.size()) multi_send_ls_msg(map,h1);
}

void 
gpet_dispatcher::on_inc_invisible(int prev_invi_degree, int cur_invi_degree)
{
	ASSERT(cur_invi_degree > prev_invi_degree);
	packet_wrapper  h1(64);
	using namespace S2C;
	gnpc * pNPC = (gnpc *)_imp->_parent;
	CMD::Make<CMD::object_disappear>::From(h1,pNPC);
	slice * pPiece = pNPC->pPiece;
	AutoBroadcastCSMsgToSpec(_imp->_plane,pPiece,h1,cur_invi_degree,prev_invi_degree,0,((gpet_imp *)_imp)->_leader_id.id);
}

void 
gpet_dispatcher::on_dec_invisible(int prev_invi_degree, int cur_invi_degree)
{
	ASSERT(cur_invi_degree < prev_invi_degree);
	packet_wrapper  h1(64);
	using namespace S2C;
	gnpc* pNPC = (gnpc*)_imp->_parent;
	CMD::Make<CMD::npc_enter_world>::From(h1,pNPC);
	slice * pPiece = pNPC->pPiece;
	AutoBroadcastCSMsgToSpec(_imp->_plane,pPiece,h1,prev_invi_degree,cur_invi_degree,0,((gpet_imp *)_imp)->_leader_id.id);
}

void
gpet_dispatcher::move(const A3DVECTOR & target, int cost_time,int speed,unsigned char move_mode)
{
	packet_wrapper  h1(64);
	using namespace S2C;
	gnpc * pNPC = (gnpc*)_imp->_parent;
	CMD::Make<CMD::object_move>::From(h1,pNPC,target,cost_time,speed,move_mode);
//	__PRINTF("npc move:(%f %f %f)\n",target.x,target.y,target.z);

	slice * pPiece = pNPC->pPiece;
	if(pNPC->IsInvisible())
	{
		gpet_imp * pImp = (gpet_imp *)_imp;
		AutoBroadcastCSMsgInInvisible(_imp->_plane,pPiece,h1,pNPC->invisible_degree,0,pImp->_leader_id.id);
		//主人一定能看到宠物,给主人发移动消息
		send_ls_msg(pImp->_leader_data.cs_index, pImp->_leader_id.id, pImp->_leader_data.cs_sid, h1);
	}
	else
		AutoBroadcastCSMsg(_imp->_plane,pPiece,h1);
}

void
gpet_dispatcher::stop_move(const A3DVECTOR & target, unsigned short speed,unsigned char dir,unsigned char move_mode)
{
	if(_imp->_parent->IsZombie()) 
	{
		//如果死亡了就不必发送了
		return ;
	}

	((gactive_imp*)_imp)->RecalcDirection();
	if(((gpet_imp*)_imp)->_direction.squared_magnitude() < 1e-3)
	{
		__PRINTF("最后一次移动的比例过小\n");
	}

	packet_wrapper  h1(64);
	using namespace S2C;
	gnpc* pNPC = (gnpc*)_imp->_parent;
//	CMD::Make<CMD::npc_stop_move>::From(h1,pObj,speed,pObj->dir,move_mode);
	CMD::Make<CMD::object_stop_move>::From(h1,pNPC,target,speed,pNPC->dir,move_mode);

	slice * pPiece = pNPC->pPiece;
	if(pNPC->IsInvisible())
	{
		gpet_imp * pImp = (gpet_imp *)_imp;
		AutoBroadcastCSMsgInInvisible(_imp->_plane,pPiece,h1,pNPC->invisible_degree,0,pImp->_leader_id.id);
		//主人一定能看到宠物,给主人发移动消息
		send_ls_msg(pImp->_leader_data.cs_index, pImp->_leader_id.id, pImp->_leader_data.cs_sid, h1);
	}
	else
		AutoBroadcastCSMsg(_imp->_plane,pPiece,h1);
}

void
gpet_dispatcher::enter_world()
{
	packet_wrapper  h1(64);
	using namespace S2C;
	gnpc* pNPC = (gnpc*)_imp->_parent;
	slice *pPiece = pNPC->pPiece;
	CMD::Make<CMD::npc_enter_world>::From(h1,pNPC);
	if(pNPC->IsInvisible())
	{
		gpet_imp * pImp = (gpet_imp *)_imp;
		AutoBroadcastCSMsgInInvisible(_imp->_plane,pPiece,h1,pNPC->invisible_degree,0,pImp->_leader_id.id);
		//主人一定能看到宠物,给主人发
		send_ls_msg(pImp->_leader_data.cs_index, pImp->_leader_id.id, pImp->_leader_data.cs_sid, h1);
	}
	else
		AutoBroadcastCSMsg(_imp->_plane,pPiece,h1);

	//暂时先不考虑在边界隐身的情况
	if(pPiece->IsBorder())
	{
		extern_object_manager::SendAppearMsg<0>(_imp->_plane,pNPC,pPiece);
	}
}

void 
gpet_dispatcher::disappear()
{
	packet_wrapper  h1(64);
	using namespace S2C;
	gnpc * pNPC = (gnpc*)_imp->_parent;
	CMD::Make<CMD::object_disappear>::From(h1,pNPC);
	slice * pPiece = pNPC->pPiece;
	if(pNPC->IsInvisible())
	{
		gpet_imp * pImp = (gpet_imp *)_imp;
		AutoBroadcastCSMsgInInvisible(_imp->_plane,pPiece,h1,pNPC->invisible_degree,0,pImp->_leader_id.id);
		//主人一定能看到宠物,给主人发
		send_ls_msg(pImp->_leader_data.cs_index, pImp->_leader_id.id, pImp->_leader_data.cs_sid, h1);
	}
	else
		AutoBroadcastCSMsg(_imp->_plane,pPiece,h1);

	//暂时先不考虑在边界隐身的情况
	if(pPiece->IsBorder())
	{
		extern_object_manager::SendDisappearMsg<0>(_imp->_plane,pNPC,pPiece);
	}
}

void 
gpet_dispatcher::enter_sanctuary()
{
	packet_wrapper  h1(16);
	using namespace S2C;
	gpet_imp * pImp = (gpet_imp *)_imp;
	CMD::Make<CMD::object_enter_sanctuary>::From(h1, pImp->_parent->ID.id);
	send_ls_msg(pImp->_leader_data.cs_index, pImp->_leader_id.id, pImp->_leader_data.cs_sid, h1);
}

void 
gpet_dispatcher::leave_sanctuary()
{
	packet_wrapper  h1(16);
	using namespace S2C;
	gpet_imp * pImp = (gpet_imp *)_imp;
	CMD::Make<CMD::object_leave_sanctuary>::From(h1, pImp->_parent->ID.id);
	send_ls_msg(pImp->_leader_data.cs_index, pImp->_leader_id.id, pImp->_leader_data.cs_sid, h1);
}

gpet_imp::gpet_imp()
{
	_pet_stamp = -1;
	_attack_hook = NULL;
	_enchant_hook = NULL;
	_attack_fill = NULL;
	_enchant_fill = NULL;
	memset(&_leader_data, 0, sizeof(_leader_data));
	_leader_force_attack = 0;
	_aggro_state = PET_AGGRO_DEFENSE;
	_stay_mode = PET_MOVE_FOLLOW;
	_notify_master_counter = 0;
	_hp_notified= -1;
	_mp_notified = -1;
	_pet_tid = 0;
	_honor_level = 0;
	_master_attack_target = 0;
	_old_combat_state = -1;
	_old_attack_monster = false;
	memset(&_skills,0,sizeof(_skills));
}

void 
gpet_imp::Init(world * pPlane,gobject*parent)
{
	gnpc_imp::Init(pPlane,parent);
	_filters.AddFilter(new pet_damage_filter(this, 10));
}

int
gpet_imp::DoAttack(const XID & target,char force_attack)
{
	if(!IsAttackMonster() && target.type == GM_TYPE_NPC)
    {
		SetAttackMonster(true);
	}

	return gnpc_imp::DoAttack(target, force_attack);
}

bool 
gpet_imp::CheckCoolDown(int idx) 
{ 
	return _cooldown.TestCoolDown(idx);
}

void 
gpet_imp::SetCoolDown(int idx, int msec) 
{ 
	_cooldown.SetCoolDown(idx,msec);
	NotifySetCoolDownToMaster(idx,msec);
}

void 
gpet_imp::NotifySetCoolDownToMaster(int idx, int msec)
{
	SendTo<0>(GM_MSG_PET_SET_COOLDOWN,_leader_id,idx, &msec, sizeof(msec));
}

void 
gpet_imp::NotifySkillStillCoolDown(int cd_id)
{
	packet_wrapper  h1(64);
	using namespace S2C;
	CMD::Make<CMD::error_msg>::From(h1, S2C::ERR_PET_SKILL_IN_COOLDOWN);
	//发送离开消息
	send_ls_msg(_leader_data.cs_index, _leader_id.id, _leader_data.cs_sid, h1);
}

bool 
gpet_imp::DrainMana(int mana)
{
	if(_basic.mp >= mana)
	{
		_basic.mp -= mana;
		return true;
	}
	else
	{
		_basic.mp = 0;
	}
	return false;
}

void 
gpet_imp::PetRelocatePos(bool is_disappear)
{
	if(_leader_id.IsValid())
	{
		int dis = is_disappear?1:0;
		SendTo<0>(GM_MSG_PET_RELOCATE_POS,_leader_id,_pet_stamp,&dis, sizeof(dis));
	}
}

void 
gpet_imp::TryChangeInhabitMode(char leader_layer, const A3DVECTOR & leader_pos)
{
	//根据主人的所处的layer改变inhabit_mode
	//宠物的能否继续存在仍由主人检查
	switch(_inhabit_type)
	{
		default:
		case 0: //陆地
		case 1:	//水下
		case 2: //空中
			return;	//由主人负责处理
		break;
		case 3:	//地面加水下
			if(leader_layer == LAYER_AIR) return; //由主人负责召回
		break;
		
		case 4:	//地面加空中
			if(leader_layer == LAYER_WATER) return; //由主人负责召回
			//if(leader_layer == LAYER_GROUND) return; //万家在地面 宠物在空中 可行
		break;
		case 5:	//水下加空中
			if(leader_layer == LAYER_GROUND && _layer_ctrl.GetLayer() == LAYER_AIR) return;
		break;
		case 6:	//海陆空
			//if(leader_layer == LAYER_GROUND && _layer_ctrl.GetLayer() == LAYER_AIR) return;
		break;
	}
	//重新定位
	A3DVECTOR pos = _parent->pos;
	char inhabit_mode;
	A3DVECTOR offset = leader_pos;
	offset -= _parent->pos;
	if(offset.squared_magnitude() > 1.f*1.f) offset.normalize();	
	if(pet_gen_pos::FindValidPos(pos,inhabit_mode,leader_layer,_inhabit_type,_plane,0.f,offset) == 0)
	{
		SetInhabitMode(inhabit_mode);
		if(AddSession(new session_npc_empty())) StartSession();
		_runner->stop_move(pos, (unsigned short)(_cur_prop.run_speed*256.0f+0.5f) ,1,GetMoveModeByInhabitType<0>()|C2S::MOVE_MODE_RUN);
		pos -= _parent->pos;
		StepMove(pos);
	}
	else if(_parent->pos.squared_distance(leader_pos) > 10.f*10.f)
	{
		PetRelocatePos(false);	
	}
}

bool 
gpet_imp::PetGetNearestTeammate(float range, XID & target)
{
	target = XID(-1,-1);
	if(_leader_data.team_count)
	{
		//在组队中	
		float min_dis = range*range;
		for(int i=0; i<_leader_data.team_count; i++)
		{
			world::object_info info;
			if(!_plane->QueryObject(_leader_data.teamlist[i], info)
					|| !(info.state & world::QUERY_OBJECT_STATE_ACTIVE))
				continue;
			float dis = info.pos.squared_distance(_parent->pos);
			if(dis < min_dis)
			{
				min_dis = dis;
				target = _leader_data.teamlist[i];
			}
		}
	}
	else
	{
		//不在组队
		world::object_info info;
		if(_plane->QueryObject(_leader_id, info)
				&& (info.state & world::QUERY_OBJECT_STATE_ACTIVE)
				&& info.pos.squared_distance(_parent->pos) < range*range)
		{
			target = _leader_id;
		}
	}
	return target.IsValid();
}

void 
gpet_imp::SetHonorLevel(int honor_level)
{
	_honor_level = honor_level;
	_filters.ModifyFilter(FILTER_INDEX_PET_DAMAGE,FMID_PET_HONOR,&_honor_level,sizeof(_honor_level));
}

void 
gpet_imp::SetAttackHook(attack_judge hook1,enchant_judge hook2,attack_fill fill1, enchant_fill fill2)
{
	_attack_hook = hook1;
	_enchant_hook = hook2;
	_attack_fill = fill1;
	_enchant_fill = fill2;
}

int 
gpet_imp::MessageHandler(world * pPlane, const MSG & msg)
{
//	__PRINTF("pet handle message %d\n",msg.message);
	switch(msg.message)
	{
		case GM_MSG_PET_CTRL_CMD:
		if(msg.source == _leader_id)
		{
			if(msg.content_length > sizeof(short))
			{
				DispatchPlayerCommand(msg.param,msg.content,msg.content_length);
			}
			else
			{
				ASSERT(false);
			}
		}
		return 0;

		case GM_MSG_PET_CHANGE_POS:
		{
			if(msg.content_length == sizeof(msg_pet_pos_t) && msg.source == _leader_id) 
			{
				msg_pet_pos_t & pet_pos = *(msg_pet_pos_t*)msg.content;
				A3DVECTOR pos = pet_pos.pos;
				SetInhabitMode(pet_pos.inhabit_mode);
				if(AddSession(new session_npc_empty())) StartSession();
				_runner->stop_move(pos,0x500,1,GetMoveModeByInhabitType<0>()|C2S::MOVE_MODE_RETURN);
				pos -= _parent->pos;
				StepMove(pos);
			}
		}
		return 0;

		case GM_MSG_PET_DISAPPEAR:
		{ 
			if(msg.source == _leader_id || msg.source == _parent->ID)  
			{
				OnNpcLeaveWorld();
				_runner->disappear();
				_commander->Release();
			}
		}
		return 0;

		case GM_MSG_ATTACK:
		{
			ASSERT(msg.content_length >= sizeof(attack_msg));

			attack_msg ack_msg = *(attack_msg*)msg.content;
			//处理一下到来的攻击消息
			_filters.EF_TransRecvAttack(msg.source, ack_msg);

			//进行决斗的检测
			if(_leader_data.duel_target && _leader_data.duel_target == ack_msg.ainfo.attacker.id)
			{
				//如何处理宠物？
				ack_msg.attacker_mode |= attack_msg::PVP_DUEL;
			}

			//宠物永远不会受到来自主人的伤害,也不会受到主人召出的其他宠物的伤害
			if(ack_msg.ainfo.attacker == _leader_id) return 0;

			if(_attack_hook)
			{
				//进行特殊的检测
				if(!(*_attack_hook)(this,msg,ack_msg)) return 0;
			}
			else
			{
				//使用通用怪物方式的检测
				if(!ack_msg.force_attack && !(GetFaction() & ack_msg.target_faction))
				{
					return 0;
				}
			}

			HandleAttackMsg(pPlane,msg,&ack_msg);
		}
		return 0;
		
		case GM_MSG_ENCHANT:
		{
			ASSERT(msg.content_length >= sizeof(enchant_msg));
			enchant_msg ech_msg = *(enchant_msg*)msg.content;
			_filters.EF_TransRecvEnchant(msg.source, ech_msg);
			if(!ech_msg.helpful)
			{
				//宠物永远不会受到来自主人的伤害,也不会受到主人召出的其他宠物的伤害
				if(ech_msg.ainfo.attacker == _leader_id) return 0;
				
				//进行决斗的检测
				if(_leader_data.duel_target && _leader_data.duel_target == ech_msg.ainfo.attacker.id)
				{
					//如何处理宠物？
					ech_msg.attacker_mode |= attack_msg::PVP_DUEL;
				}
			}
			else
			{
				//进行决斗的检测
				if(_leader_data.duel_target && _leader_data.duel_target == ech_msg.ainfo.attacker.id)
				{
					//如何处理宠物？
					ech_msg.target_faction = 0xFFFFFFFF;
				}
			}

			if(msg.source == _parent->ID)
			{
				//这种情况是宠物自己使用祝福技能
				if(!ech_msg.helpful) return 0;
			}
			else if(_enchant_hook)
			{
				//进行特殊的检测
				if(!(*_enchant_hook)(this,msg,ech_msg)) return 0;
			}
			else
			{
				//使用通用怪物方式的检测
				if(!ech_msg.helpful)
				{
					if(!ech_msg.force_attack && !(GetFaction() & ech_msg.target_faction))
					{
						return 0;
					}
				}
				else
				{
					if(!(GetFaction() & ech_msg.attacker_faction))
					{
						return 0;
					}
				}
			}
			HandleEnchantMsg(pPlane,msg,&ech_msg);
		}
		return 0;

		case GM_MSG_DUEL_HURT:
		{
			if(!_parent->IsZombie()) 
			{
				if(_leader_data.duel_target && _leader_data.duel_target == msg.source.id)
				{
					DoDamage(msg.param);
					if(_basic.hp == 0)
					{
						Die(msg.source,false,0,0);
					}
				}
			}
		}
		return 0;
		
		case GM_MSG_PET_MASTER_INFO:
		{
			if(msg.content_length == sizeof(pet_leader_prop) && msg.source == _leader_id)
			{
				int old_duel_target = _leader_data.duel_target;
				_leader_data = *(pet_leader_prop*)msg.content;
				_invader_state = _leader_data.invader_state;
				if(old_duel_target != _leader_data.duel_target)
				{
					if(old_duel_target == 0)
					{
						//进入战斗
						//做啥处理？ 好像不需要
						
					}
					else if(_leader_data.duel_target == 0)
					{
						//解除决斗
						//清除所有的仇恨
						gnpc_ai * pAI = ((gnpc_controller*)_commander)->GetAI();
						aggro_policy * pAggro = pAI->GetAggroCtrl();
						pAggro->Clear();
						_leader_force_attack = 0; 
						_master_attack_target = 0;
					}
				}
			}
			else
			{
				//modify by liuguichen 宠物消失后Object对象可能被其他宠物重用，发生重用时新宠物的XID与旧宠物一致，新宠物有可能接收到旧宠物的MSG
				//ASSERT(false);
			}
		}
		return 0;

		case GM_MSG_PET_LEVEL_UP:
		{
			if(msg.content_length == sizeof(int) && msg.source == _leader_id)
			{
				int level = *(int*)msg.content;
				extend_prop prop;
				if(pet_dataman::GenerateBaseProp(_pet_tid, level,prop))
				{
					_base_prop = prop;
					property_policy::UpdateNPC(this);
					_basic.hp = _cur_prop.max_hp;
					_runner->level_up();
				}
			}
		}
		return 0;

		case GM_MSG_PET_HONOR_MODIFY:
		{
			if(msg.content_length == sizeof(int) && msg.source == _leader_id)
			{
				size_t level = *(size_t*)msg.content;
				if(level >= pet_data::HONOR_LEVEL_COUNT)
				{
					level = pet_data::HONER_LEVEL_3;
				}
				SetHonorLevel(level);
			}
		}
		return 0;

		case GM_MSG_PET_SKILL_LIST:
		{
			if(msg.source == _leader_id)
			{
				size_t n = sizeof(pet_data::__skills) * pet_data::MAX_PET_SKILL_COUNT;
				if(msg.content_length == n)
				{
					ClearSkill();
					int * p = (int*)msg.content;
					for(size_t i = 0; i < pet_data::MAX_PET_SKILL_COUNT; i ++, p +=2)
					{
						if(*p)
						{
							AddSkill(*p, *(p+1));
						}
					}
					gnpc_ai * pAI = ((gnpc_controller*)_commander)->GetAI();
					if(pAI)
					{
						ai_policy * pPolicy = pAI->GetAICtrl();
						pPolicy->SetPetAutoSkill(0,0,0);
					}
				}
				else
				{
					ASSERT(false);
				}
			}
		}
		return 0;

		case GM_MSG_MASTER_ASK_HELP:
		{
			if(msg.content_length == sizeof(XID))
			{
				XID who = *(XID*)msg.content;
				if(who.IsValid() && msg.source == _leader_id)
				{
					if(_aggro_state == PET_AGGRO_DEFENSE)
					{
						//是防御姿态
						gnpc_ai * pAI = ((gnpc_controller*)_commander)->GetAI();
						aggro_policy * pAggro = pAI->GetAggroCtrl();
						ai_policy * pPolicy = pAI->GetAICtrl();
						if(pAggro->AddAggro(who,2,2) == 0)
						{
							pPolicy->OnAggro();
						}
					}
				}
			}
		}
		return 0;

		case GM_MSG_PET_AUTO_ATTACK:
		{
			if(msg.content_length == sizeof(XID))
			{
				if(_aggro_state == PET_AGGRO_AUTO)
				{
					gnpc_ai * pAI = ((gnpc_controller*)_commander)->GetAI();
					aggro_policy * pAggro = pAI->GetAggroCtrl();
					if(pAggro->Size() == 0)
					{
						XID target = *(XID*)msg.content;
						char force_attack = msg.param & 0xFF;
						pAggro->SetAggroState(aggro_policy::STATE_NORMAL);
						pAI->AddAggro(target, _cur_prop.max_hp + 10);
						pAggro->SetAggroState(aggro_policy::STATE_FREEZE);

						_leader_force_attack = force_attack; 
						_master_attack_target = target.id;
					}
				}
			}
		}
		return 0;

		case GM_MSG_HATE_YOU:
		if(_enemy_list.size() < MAX_PLAYER_ENEMY_COUNT)
		{
			_enemy_list[msg.source.id] ++;
		}
		else    
		{       
			ENEMY_LIST::iterator it = _enemy_list.find(msg.source.id);
			if(it != _enemy_list.end())
			{
				it->second ++;
			}
		}
		return 0;

		case GM_MSG_NOTIFY_INVISIBLE_DATA:
		{
			//收到leader的隐身数据
			ASSERT(msg.content_length == sizeof(msg_invisible_data));
			if(msg.source == _leader_id)
			{
				msg_invisible_data & data = *(msg_invisible_data*)msg.content;	
				//宠物隐身等级反隐等级与主人相同,但当前是否隐身是受ai_policy控制
				//先处理pet的隐身
				gnpc* pNPC = (gnpc*)_parent;
				int inc_invi = data.invisible_degree - _invisible_active;
				if(inc_invi > 0)
				{
					_invisible_active = data.invisible_degree;
					if(!pNPC->IsInvisible())
					{
						//清仇恨进入隐身
						gnpc_ai * pAI = ((gnpc_controller*)_commander)->GetAI();
						aggro_policy * pAggro = pAI->GetAggroCtrl();
						pAggro->Clear();
						SetInvisible(1);
					}
					else
					{
						//原来就在隐身则隐身隐身等级提高	
						pNPC->invisible_degree = _invisible_active;
						_runner->on_inc_invisible(pNPC->invisible_degree-inc_invi,pNPC->invisible_degree);
						__PRINTF("进入%d级隐身了\n",pNPC->invisible_degree);
					}
							
				}
				else if(inc_invi < 0)
				{
					if(pNPC->IsInvisible())
					{
						if(data.invisible_degree == 0)
						{
							//脱离隐身，不再需要通知主人，所以调用gnpc_imp::ClearInvisible
							gnpc_imp::ClearInvisible();	
						}
						else
						{
							//隐身等级下降,这种情况暂时不会出现
							pNPC->invisible_degree += inc_invi;
							_runner->on_dec_invisible(pNPC->invisible_degree-inc_invi,pNPC->invisible_degree);
							__PRINTF("进入%d级隐身了\n",pNPC->invisible_degree);
						}
					}
					else
					{
						;//当前不在隐身不用处理了	
					}
					_invisible_active = data.invisible_degree;
				}
				//再处理pet的反隐
				int inc_anti = data.anti_invisible_degree - _anti_invisible_active;
				if(inc_anti > 0)
					IncAntiInvisibleActive(inc_anti);
				else if(inc_anti < 0)
					DecAntiInvisibleActive(-inc_anti);
			}
		}
		return 0;

		case GM_MSG_MASTER_NOTIFY_LAYER:
		{
			if(msg.content_length == sizeof(char) && msg.source == _leader_id)
			{
				char layer = *(char *)msg.content;
				if(layer != _layer_ctrl.GetLayer())
				{
					TryChangeInhabitMode(layer, msg.pos);
				}
			}
		}
		return 0;

		case GM_MSG_PET_TEST_SANCTUARY:
		{
			if(msg.source == _leader_id)
			{
				TestSanctuary();
			}		
		}
		return 0;

	default:
		return gnpc_imp::MessageHandler(pPlane,msg);
	}
}

void 
gpet_imp::OnDeath(const XID & attacker,bool is_invader,char attacker_mode, int)
{
	if(_parent->IsZombie())
	{
		//已经是zombie了
		return ;
	}
	//死亡的操作，是，进入zombie状态，
	_parent->b_zombie = true;

	_team_visible_state_flag = false;
	_visible_team_state.clear();
	_visible_team_state_param.clear();
	_enemy_list.clear();

	//执行策略的OnDeath
	((gnpc_controller*)_commander) -> OnDeath(attacker);
	//清除当前Session 注意必须在commander的OnDeath之后调用，否则可能由于策略中排队的任务会引发新的session和任务的产生
	ClearSession();
	
	if(attacker.type == GM_TYPE_PLAYER && is_invader && _leader_data.invader_state == INVADER_LVL_0 && !_leader_data.free_pvp_mode)
	{
		//是红名，发送红名消息
		SendTo<0>(GM_MSG_PLAYER_BECOME_PARIAH,attacker,0);
	}

	//发送死亡消息
	_runner->on_death(attacker,_corpse_delay);

	{
		//发送消失的消息，延时发送，
		//因为现在消失的话，可能在心跳中死亡 会有很多额外的处理
		MSG msg;
		BuildMessage(msg,GM_MSG_OBJ_ZOMBIE_END,_parent->ID,_parent->ID,_parent->pos);
		_plane->PostLazyMessage(msg);
	}

	//还需要发送一个消息给master
	NotifyDeathToMaster();
}

void 
gpet_imp::NotifyDeathToMaster()
{
	SendTo<0>(GM_MSG_PET_NOTIFY_DEATH, _leader_id, _pet_stamp);
}

void 
gpet_imp::SetAggroState(int state)
{
	gnpc_ai * pAI = ((gnpc_controller*)_commander)->GetAI();
	aggro_policy * pAggro = pAI->GetAggroCtrl();
	switch (state)
	{
		default:
		case PET_AGGRO_DEFENSE:
			//被动
			pAggro->SetAggroState(aggro_policy::STATE_NORMAL);
			break;
		case PET_AGGRO_AUTO:
			//主动
			pAggro->SetAggroState(aggro_policy::STATE_NORMAL);
			break;
		case PET_AGGRO_PASSIVE:
			//发呆
			pAggro->Clear();
			pAggro->SetAggroState(aggro_policy::STATE_FREEZE);
			break;

	}
	_aggro_state = state;

	//通知policy对象当前的状态
	ai_policy * pPolicy = pAI->GetAICtrl();
	pPolicy->ChangeAggroState(state);
}

void 
gpet_imp::SendPetAIState()
{
	packet_wrapper  h1(64);
	using namespace S2C;
	CMD::Make<CMD::pet_ai_state>::From(h1,_aggro_state,_stay_mode);
	send_ls_msg(_leader_data.cs_index, _leader_id.id, _leader_data.cs_sid, h1);
	__PRINTF("发送 ai state\n");
}

void 
gpet_imp::SetStayState(int state)
{
	gnpc_ai * pAI = ((gnpc_controller*)_commander)->GetAI();
	aggro_policy * pAggro = pAI->GetAggroCtrl();
	switch (state)
	{
		case PET_MOVE_FOLLOW:
			//跟随
			pAggro->Clear();
			break;
		case PET_STAY_STAY:
		default:
			//停留
			pAggro->Clear();
			break;
	}

	//清除当前的session
	ClearSession();

	ai_policy * pPolicy = pAI->GetAICtrl();
	_stay_mode = state;
	pPolicy->ChangeStayMode(state);

}

bool 
gpet_imp::TestSanctuary()
{
	if(player_template::IsInSanctuary(_parent->pos))
	{
		_filters.AddFilter(new pvp_limit_filter(this,FILTER_INDEX_PVPLIMIT));
		return true;
	}
	else
	{
		GLog::log(GLOG_INFO,"用户%d未进入安全区(%f,%f)",_parent->ID.id, _parent->pos.x,_parent->pos.z);
	}
	return false;
}

void 
gpet_imp::DispatchPlayerCommand(int target, const void * buf, size_t size)
{
	int pet_cmd = *(int*)buf;
	gnpc_ai * pAI = ((gnpc_controller*)_commander)->GetAI();
	if(!pAI) return;
	switch(pet_cmd)
	{
		case 1:
		//attack...
		if(size == sizeof(char) + sizeof(int))
		{
			if(target == 0 || target == -1) return;
			char force_attack = *((char*)buf + sizeof(int));
			XID id;
			MAKE_ID(id, target);
			aggro_policy * pAggro = pAI->GetAggroCtrl();
			pAggro->Clear();
			int st1 = pAggro->GetAggroState();
			pAggro->SetAggroState(aggro_policy::STATE_NORMAL);
			pAI->AddAggro(id, _cur_prop.max_hp + 10);
			pAggro->SetAggroState(st1);
			
			_leader_force_attack = force_attack; 
			_master_attack_target = target;
		}
		__PRINTF("发出攻击命令\n");
		break;

		case 2:
		//change follow mode
		if(size == sizeof(int) * 2)
		{
			aggro_policy * pAggro = pAI->GetAggroCtrl();
			int state = ((int*)(buf))[1];
			switch (state)
			{
				case PET_MOVE_FOLLOW:
				//跟随
					pAggro->Clear();
					break;
				case PET_STAY_STAY:
				//停留
					pAggro->Clear();
					break;

				default:
					return;
			}

			//清除当前的session
			ClearSession();

			ai_policy * pPolicy = pAI->GetAICtrl();
			if(_stay_mode != state)
			{
				pPolicy->ClearTask();
				//如果发生变化，将状态发送给客户端
				_stay_mode = state;
				SendPetAIState();
			}
			pPolicy->ChangeStayMode(state);

			//清空内容
			_leader_force_attack = 0; 
			_master_attack_target = 0;
		}
		break;

		case 3:
		//change aggro state
		if(size != sizeof(int)*2) return;
		{
			int state = ((int*)(buf))[1];
			aggro_policy * pAggro = pAI->GetAggroCtrl();
			switch (state)
			{
				case PET_AGGRO_DEFENSE:
				//被动
					if(_aggro_state != state) pAggro->Clear();
					pAggro->SetAggroState(aggro_policy::STATE_NORMAL);
					break;

				case PET_AGGRO_AUTO:
				//主动
					//pAggro->SetAggroState(aggro_policy::STATE_NORMAL);
				//目前的主动同发呆	
					pAggro->Clear();
					pAggro->SetAggroState(aggro_policy::STATE_FREEZE);
					break;

				case PET_AGGRO_PASSIVE:
				//发呆
					pAggro->Clear();
					pAggro->SetAggroState(aggro_policy::STATE_FREEZE);
					break;

				default:
					return;
			}
			if(_aggro_state != state)
			{
				//如果发生变化，将状态发送给客户端
				_aggro_state = state;
				SendPetAIState();
			}

			//通知policy对象当前的状态
			ai_policy * pPolicy = pAI->GetAICtrl();
			pPolicy->ChangeAggroState(state);


			//清空内容
			_leader_force_attack = 0; 
			_master_attack_target = 0;
		}
		break;

		case 4:
		if(size != sizeof(int)*2 + sizeof(char)) return;
		{
			int skill_id = ((int*)(buf))[1];
			char force_attack = *((char*)buf + sizeof(int) + sizeof(int));
			int skill_level = GetSkillLevel(skill_id);
			if(skill_level <= 0) break;	//此宠物无此技能

			//根据技能的类别进行调整
			//判断是否需要攻击目标
			XID id(-1,-1);
			int rangetype = GNET::SkillWrapper::RangeType(skill_id);
			if(rangetype != 2 && rangetype != 5)
			{
				if(target == 0 || target == -1) return;
				MAKE_ID(id, target);
				char type = GNET::SkillWrapper::GetType(skill_id);
				if(type != 2)	//攻击或诅咒
				{
					if(id == _parent->ID) return ;
					if(id == _leader_id) return;

					//如果是合法目标则加入一些仇恨
					aggro_policy * pAggro = pAI->GetAggroCtrl();
					pAggro->Clear();
					int st1 = pAggro->GetAggroState();
					pAggro->SetAggroState(aggro_policy::STATE_NORMAL);
					pAI->RawAddAggro(id, _cur_prop.max_hp + 10);
					pAggro->SetAggroState(st1);
				}
			}
			ai_policy * pPolicy = pAI->GetAICtrl();
			pPolicy->ClearNextTask();
			pPolicy->AddPetSkillTask(skill_id,skill_level, id, rangetype);

			
			_leader_force_attack = force_attack; 
			_master_attack_target = target;
			_leader_force_attack = force_attack; 
			_master_attack_target = target;

		//	aggro_policy * pAggro = pAI->GetAggroCtrl();
		//	pAggro->Clear();
		//	pAggro->SetAggroState(aggro_policy::STATE_NORMAL);
		//	pAI->AddAggro(id, _cur_prop.max_hp + 10);
			
		}
		break;

		case 5:
		{
			int skill_id = ((int*)(buf))[1];
			ai_policy * pPolicy = pAI->GetAICtrl();
			if(skill_id <= 0)
			{
				pPolicy->SetPetAutoSkill(0,0,0);
			}

			int skill_level = GetSkillLevel(skill_id);
			if(skill_level <= 0) break;	//此宠物无此技能

			int range_type = GNET::SkillWrapper::RangeType(skill_id);
			if(range_type == 5) break;	//以自身为目标的技能不能自动使用

			pPolicy->SetPetAutoSkill(skill_id, skill_level,range_type);
		}
		break;

		default:
		__PRINTF("宠物收到命令%d\n",pet_cmd);
		break;
	}
}

bool 
gpet_imp::OI_IsPVPEnable()
{
	return _leader_data.is_pvp_enable;
}

bool 
gpet_imp::OI_IsInPVPCombatState()
{
	return _leader_data.pvp_combat_timer;
}

bool 
gpet_imp::OI_IsInTeam()
{
	return _leader_data.team_count > 0;
}

bool 
gpet_imp::OI_IsMember(const XID & id)
{
	for(int i = 0; i < _leader_data.team_count; i ++)
	{
		if(_leader_data.teamlist[i] == id) return true;
	}
	return false;
}

int  
gpet_imp::OI_GetMafiaID()
{
	return _leader_data.mafia_id;
}

bool 
gpet_imp::OI_IsFactionAlliance(int fid)
{
	return _leader_faction_alliance.find(fid) != _leader_faction_alliance.end();
}

bool 
gpet_imp::OI_IsFactionHostile(int fid)
{
	return _leader_faction_hostile.find(fid) != _leader_faction_hostile.end();
}

void 
gpet_imp::SendMsgToTeam(const MSG & msg,float range,bool exclude_self)
{
	if(!_leader_data.team_count) return;
	size_t index = 0;
	XID  list[TEAM_MEMBER_CAPACITY+1];
	ASSERT(msg.source == _parent->ID);
	range *= range;
	if(!exclude_self) list[index++] = _parent->ID;
	for(int i=0; i<_leader_data.team_count; i++)
	{
		world::object_info info;
		if(!_plane->QueryObject(_leader_data.teamlist[i], info)
				|| info.pos.squared_distance(_parent->pos) >= range)
			continue;
		list[index++] = _leader_data.teamlist[i];
	}
	if(index) _plane->SendMessage(list, list+index, msg);	
}

void 
gpet_imp::FillAttackMsg(const XID & target, attack_msg & attack,int dec_arrow)
{
	gactive_imp::FillAttackMsg(target,attack);
	attack.ainfo.attacker = _leader_id;
	if(target.id == _master_attack_target)
	{
		attack.force_attack = _leader_force_attack;
	}
	else
	{
		
		attack.force_attack = 0;
	}

	attack.ainfo.sid = _leader_data.cs_sid;
	attack.ainfo.cs_index = _leader_data.cs_index;
	attack.ainfo.team_id = _leader_data.team_id;
	attack.ainfo.team_seq = _leader_data.team_seq;
	int eff_level =_leader_data.team_efflevel;
	ASSERT(eff_level > 0);
	//if(_leader_data.team_count) eff_level = _leader_data.team_efflevel;  现在始终使用主人的级别了
	attack.ainfo.eff_level = eff_level;
	attack.ainfo.wallow_level = _leader_data.wallow_level;
	attack.ainfo.profit_level = _leader_data.profit_level;

	//调用master的填充接口
	if(_attack_fill)
	{
		(*_attack_fill)(this, attack);
	}
}


void 
gpet_imp::FillEnchantMsg(const XID & target,enchant_msg & enchant)
{
	gactive_imp::FillEnchantMsg(target,enchant);
	enchant.ainfo.attacker = _leader_id;
	if(target.id == _master_attack_target)
	{
		enchant.force_attack = _leader_force_attack;
	}
	else
	{
		enchant.force_attack = 0;
	}

	enchant.ainfo.sid = _leader_data.cs_sid;
	enchant.ainfo.cs_index = _leader_data.cs_index;
	enchant.ainfo.team_id = _leader_data.team_id;
	enchant.ainfo.team_seq = _leader_data.team_seq;
	int eff_level =_leader_data.team_efflevel;
	ASSERT(eff_level > 0);
	//if(_leader_data.team_count) eff_level = _leader_data.team_efflevel;  现在始终使用主人的级别了
	enchant.ainfo.eff_level = eff_level;
	enchant.ainfo.wallow_level = _leader_data.wallow_level;
	enchant.ainfo.profit_level = _leader_data.profit_level;

	//调用master的填充接口
	if(_enchant_fill)
	{
		(*_enchant_fill)(this, enchant);
	}
}

void 
gpet_imp::InitFromMaster(gplayer_imp * pImp)
{
	pImp->SetPetLeaderData(_leader_data);
	_invader_state = _leader_data.invader_state;
	
	_leader_faction_alliance.clear();
	for(abase::hash_map<int,int>::iterator it=pImp->_faction_alliance.begin(); it!=pImp->_faction_alliance.end(); ++it)
		_leader_faction_alliance[it->first] = 1;
	_leader_faction_hostile.clear();
	for(abase::hash_map<int,int>::iterator it=pImp->_faction_hostile.begin(); it!=pImp->_faction_hostile.end(); ++it)
		_leader_faction_hostile[it->first] = 1;
}

void 
gpet_imp::OnHeartbeat(size_t tick)
{
	gnpc_imp::OnHeartbeat(tick);
	
	if(!_parent->IsZombie() && (g_timer.get_systime() & 0x03) == 0)
	{
		//每4秒钟清除一次仇恨
		ENEMY_LIST::iterator it = _enemy_list.end();
		for(;it > _enemy_list.begin(); )
		{
			--it;
			if(it->second <=0)
			{
				it = _enemy_list.erase(it);
			}
			else
			{
				it->second = 0;
			}
		}
	}

	PeepEnemy();
	//后面是通知主人逻辑，放在最后
	NotifyMasterInHeartbeat();
}

void 
gpet_imp::NotifyMasterInHeartbeat()
{
	_notify_master_counter  ++;
	gnpc_ai * pAI = ((gnpc_controller*)_commander)->GetAI();
	aggro_policy * pAggro = pAI->GetAggroCtrl();
	int combat_state = pAggro->Size()?1:0;
	char attack_monster = IsAttackMonster();
	if(_notify_master_counter >= 5)
	{
		//每5秒向master通报一下自己的数据
		//现在需要传递的数据有血量比例filter数据，但暂时先传递空闲数据
		//现在先只通知血量比例
		msg_pet_hp_notify info = {_basic.hp / (float)_cur_prop.max_hp , _basic.hp,_aggro_state,_stay_mode,0, attack_monster, _basic.mp/(float)_cur_prop.max_mp, _basic.mp};
		info.combat_state = combat_state;

		SendTo<0>(GM_MSG_PET_NOTIFY_HP, _leader_id, _pet_stamp,&info,sizeof(info));
		_hp_notified = _basic.hp;
		_mp_notified = _basic.mp;
		_old_combat_state = combat_state;
		_old_attack_monster = attack_monster;
		_notify_master_counter = 0;
		return ;
	}
	
	if(_hp_notified != _basic.hp || _mp_notified != _basic.mp || combat_state != _old_combat_state || _old_attack_monster != attack_monster)
	{
		//血量发生了变化，直接通知master
		msg_pet_hp_notify info = {_basic.hp / (float)_cur_prop.max_hp , _basic.hp,_aggro_state,_stay_mode,0, attack_monster, _basic.mp/(float)_cur_prop.max_mp, _basic.mp};
		info.combat_state = combat_state;

		SendTo<0>(GM_MSG_PET_NOTIFY_HP, _leader_id, _pet_stamp,&info,sizeof(info));
		_hp_notified = _basic.hp;
		_mp_notified = _basic.mp;
		_old_combat_state = combat_state;
		_old_attack_monster = attack_monster;
		_notify_master_counter = 0;
		return ;
	}
}

void 
gpet_imp::PeepEnemy()
{
	_peep_counter ++;
	if(_peep_counter >= 3)
	{
		//每三秒发一次广播
		_peep_counter = 0;
		gnpc * pNPC = (gnpc*)_parent;
		MSG msg;
		msg_watching_t mwt= {_basic.level,GetFaction(),pNPC->invisible_degree};
		BuildMessage(msg,GM_MSG_WATCHING_YOU,XID(GM_TYPE_NPC,-1),pNPC->ID,pNPC->pos,0,&mwt,sizeof(mwt));
		float tmp = world_manager::GetMaxMobSightRange();
		_plane->BroadcastMessage(msg,tmp,gobject::MSG_MASK_PLAYER_MOVE);
	}
}

void
gpet_imp::AddAggroToEnemy(const XID & who,int rage)
{                       
	size_t count = _enemy_list.size();
	if(!count || rage <= 0) return;
	XID list[MAX_PLAYER_ENEMY_COUNT];
	ENEMY_LIST::iterator it = _enemy_list.begin();
	for(size_t i = 0;it != _enemy_list.end();i ++, ++it )
	{                       
		MAKE_ID(list[i],it->first);
	}               

	msg_aggro_info_t info;
	info.source = who;
	info.aggro = rage;
	info.aggro_type = 0;
	info.faction = 0xFFFFFFFF;
	info.level = 0;
	MSG msg;
	BuildMessage(msg,GM_MSG_GEN_AGGRO,XID(-1,-1),who,_parent->pos,0,&info,sizeof(info));

	_plane->SendMessage(list, list + count, msg);
}

void 
gpet_imp::AdjustDamage(const MSG & msg, attack_msg * attack,damage_entry & dmg, float & damage_adjust)
{
	if(IS_HUMANSIDE(attack->ainfo.attacker))
	{
		int pp = (((attack->attacker_layer) & 0x03) << 2) | _layer_ctrl.GetLayer();
		ASSERT((_layer_ctrl.GetLayer() & ~0x03) == 0);
		switch(pp)
		{
			case ((LAYER_GROUND << 2) | LAYER_GROUND):
			case ((LAYER_AIR << 2) | LAYER_AIR):
			case ((LAYER_WATER << 2) | LAYER_WATER):
			case ((LAYER_GROUND << 2) | LAYER_AIR):
			case ((LAYER_GROUND << 2) | LAYER_WATER):
			case ((LAYER_WATER << 2) | LAYER_GROUND):
			case ((LAYER_WATER << 2) | LAYER_AIR):
				damage_adjust *= PVP_DAMAGE_REDUCE;
				break;

			case ((LAYER_AIR << 2) | LAYER_GROUND):
			case ((LAYER_AIR << 2) | LAYER_WATER):
				damage_adjust *= PVP_DAMAGE_REDUCE * 0.5f;
				break;

			case ((LAYER_INVALID<< 2) | LAYER_GROUND):
			case ((LAYER_INVALID<< 2) | LAYER_AIR):
			case ((LAYER_INVALID<< 2) | LAYER_WATER):
			case ((LAYER_INVALID<< 2) | LAYER_INVALID):
			case ((LAYER_GROUND << 2) | LAYER_INVALID):
			case ((LAYER_AIR << 2) | LAYER_INVALID):
			case ((LAYER_WATER << 2) | LAYER_INVALID):
				ASSERT(false);
				break;
			default:
				ASSERT(false);
		}
	}
	
	if(attack->skill_id && attack->skill_enhance2)
		damage_adjust *= (0.01f * (100 + attack->skill_enhance2));
}

void 
gpet_imp::ClearInvisible()
{
	if(_invisible_active <= 0) return;
	gnpc* pNPC = (gnpc*)_parent;
	if(pNPC->invisible_degree <= 0) return;
	_runner->appear_to_spec(pNPC->invisible_degree);
	pNPC->invisible_degree = 0;
	pNPC->object_state &= ~gactive_object::STATE_INVISIBLE;
	_runner->toggle_invisible(0);
	__PRINTF("pet_npc脱离隐身拉\n");
	//通知主人
	NotifyClearInvisibleToMaster();
}

void 
gpet_imp::NotifyClearInvisibleToMaster()
{
	SendTo<0>(GM_MSG_NOTIFY_CLEAR_INVISIBLE, _leader_id, 0);
}

void 
gpet_imp::DrainLeaderHPMP(const XID & attacker, int hp, int mp)
{
	if(_leader_id.IsValid())
	{
		msg_hp_mp_t data;
		data.hp = hp;
		data.mp = mp;
		SendTo<0>(GM_MSG_DRAIN_HP_MP, _leader_id, attacker.id, &data, sizeof(data));
	}
}

/* ---------------------------policy pet-------------------------------------- */
gpet_policy::gpet_policy()
{
	memset(&_chase_info,0,sizeof(_chase_info));
	_pathfind_result = 0;
	_aggro_state = gpet_imp::PET_AGGRO_DEFENSE;
	_stay_mode = gpet_imp::PET_MOVE_FOLLOW;
	_auto_skill_id = 0;
	_auto_skill_level = 0;
	_auto_skill_type = 0;
}

void 
gpet_policy::DeterminePolicy(const XID & target)
{
	if(target.id != -1)
	{
		//考虑是否自动使用技能
		if(_auto_skill_id && CheckCoolDown(GNET::SkillWrapper::GetCooldownID(_auto_skill_id)) && CheckMp(_auto_skill_id,_auto_skill_level)) 
		{
			AddAutoCastSkill(target);
		}
		else
		{
			AddPetPrimaryTask(target, _primary_strategy);
		}
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
		if(_auto_skill_id && CheckCoolDown(GNET::SkillWrapper::GetCooldownID(_auto_skill_id)) && CheckMp(_auto_skill_id,_auto_skill_level))
		{
			AddAutoCastSkill(new_target);
		}
		else
		{
			AddPetPrimaryTask(new_target, _primary_strategy);
		}
		if(old_target == new_target) break;
		old_target = new_target;
	}                               
}                                       

void 
gpet_policy::AddPetPrimaryTask(const XID & target, int strategy)
{
	int seal_flag = _self->GetSealMode();
	if(seal_flag & ai_object::SEAL_MODE_SILENT)
	{
		ai_task * pTask = NULL;
		switch(strategy)
		{
			case STRATEGY_MELEE:
			case STRATEGY_RANGE:
			case STRATEGY_MAGIC:
			case STRATEGY_MELEE_MAGIC:
				pTask = new ai_silent_runaway_task(target);
				break;
			case STRATEGY_CRITTER:
				pTask = new ai_runaway_task(target);
				break;
			case STRATEGY_FIX:
			case STRATEGY_FIX_MAGIC:
				pTask = new ai_silent_task();
				break;
			case STRATEGY_STUB:
				//do nothing
				break;
			default:
				ASSERT(false);
		}
		if(pTask){
			pTask->Init(_self,this);
			AddTask(pTask);
		}
	}
	else
	{
		switch(strategy)
		{
			case STRATEGY_MELEE:
				AddTargetTask<ai_melee_task>(target);
				break;
			case STRATEGY_RANGE:
				AddTargetTask<ai_range_task>(target);
				break;
			case STRATEGY_MAGIC:
				AddTargetTask<ai_magic_task>(target);
				break;
			case STRATEGY_MELEE_MAGIC:
				AddTargetTask<ai_magic_melee_task>(target);
				break;
			case STRATEGY_CRITTER:
				AddTargetTask<ai_runaway_task>(target);
				break;
			case STRATEGY_FIX:
				AddTargetTask<ai_fix_melee_task>(target);
				break;
			case STRATEGY_STUB:
				//do nothing
				break;
			case STRATEGY_FIX_MAGIC:
				AddTargetTask<ai_fix_magic_task>(target);
				break;
			default:
				ASSERT(false);
		}               
	}               
}

void 
gpet_policy::UpdateChaseInfo(const CChaseInfo * pInfo)
{
	_chase_info = *pInfo;
}

void 
gpet_policy::FollowMasterResult(int reason) 
{
	if(reason)
	{
		// 寻路失败
		_pathfind_result ++;
	}
	else
	{
		_pathfind_result = 0;
	}
}

bool
gpet_policy::GatherTarget()
{
	A3DVECTOR pos;
	_self->GetPos(pos);
	guard_agent::search_target<slice> worker(_self,_self->GetSightRange(),_self->GetEnemyFaction(),0);
	_self->GetImpl()->_plane->ForEachSlice(pos,_self->GetSightRange(),worker);
	XID target;
	if(_self->GetAggroEntry(0,target))
	{
		OnAggro();
		return true;
	}
	return false;
}

void 
gpet_policy::OnHeartbeat()
{
	if(InCombat() && _cur_task && _cur_task->GetTaskType() != ai_task::AI_TASK_TYPE_PET_SKILL && 
			_self->GetImpl()->_session_state != gactive_imp::STATE_SESSION_USE_SKILL &&
			!HasNextTask())
	{
		//如果在战斗状态，则判断是否可以自动使用技能
		if(_auto_skill_id && CheckCoolDown(GNET::SkillWrapper::GetCooldownID(_auto_skill_id)) && CheckMp(_auto_skill_id,_auto_skill_level))
		{
			XID target;
			if(_self->GetAggroEntry(0,target))
			{
				AddAutoCastSkill(target);
			}
		}
	}
	
	ai_policy::OnHeartbeat();
	
	XID leader = _self->GetLeaderID();
	A3DVECTOR selfpos;
	_self->GetPos(selfpos);

	//如果没有任务 则试图跟随主人
	if(!InCombat())
	{
		if(!_cur_task)
		{
			// 如果是主动寻敌的状态，那么就进行敌人的收集 
/*			if(_aggro_state == gpet_imp::PET_AGGRO_AUTO)
			{
				if(GatherTarget())
				{
					//发现了敌人，则跳过后面的内容
					return ;
				}
			}*/
		
			ai_object::target_info info;
			int target_state;
			float range;

			target_state = _self->QueryTarget(leader,info);
			if(target_state != ai_object::TARGET_STATE_NORMAL)
			{
				//目标不存在 让自己消失
				XID id;
				_self->GetID(id);
				_self->SendMessage(id,GM_MSG_PET_DISAPPEAR);
				return;
			}

			range = info.pos.horizontal_distance(selfpos);
			if(_stay_mode != 0  )
			{
				//停留模式
				float h = fabs(selfpos.y - info.pos.y);
				if(h > 60.f || range >= 60.f*60.f || (range < 6.f*6.f && h > 30.f))
				{
					//需要让自己消失，通知master
					RelocatePetPos(true);
					return ;
				}
			}
			else if(_stay_mode == 0)
			{
				//跟随模式
				if(range > 150.f * 150.F)
				{
					//需要瞬移
					RelocatePetPos();
					return;
				}

				float h = fabs(selfpos.y - info.pos.y);
				if(h > 60.f || range >= 60.f*60.f || (range < 6.f*6.f && h > 30.f))
				{
					//距离过远或者无法到达
					//需要瞬移
					RelocatePetPos();
					return ;
				}

				if(range > 1.5f*1.5f || h > 10.f)
				{
					//跟随
					AddTargetTask<ai_pet_follow_master>(leader,&_chase_info);
				}
			}

		}

		if(_pathfind_result >= 5)
		{
			RelocatePetPos();
		}
	}
	else
	{
		//also need check master states
		ai_object::target_info info;
		int target_state = _self->QueryTarget(leader,info);
		if(target_state != ai_object::TARGET_STATE_NORMAL)
		{
			//can not find master
			XID id;
			_self->GetID(id);
			_self->SendMessage(id,GM_MSG_PET_DISAPPEAR);
			return;
		}


		//如果超出距离就直接消失
		float range = info.pos.horizontal_distance(selfpos);
		float h = fabs(selfpos.y - info.pos.y);
		if(h > 60.f || range >= 60.f*60.f || (range < 6.f*6.f && h > 40.f))
		{
			//回到主人身边
			RelocatePetPos(false);
			return ;
		}

	}
}

void 
gpet_policy::RelocatePetPos(bool disappear)
{
	_self->PetRelocatePos(disappear);
	_pathfind_result = 0;
}

void 
gpet_policy::ChangeAggroState(int state)
{
	_aggro_state = state;
}

void 
gpet_policy::ChangeStayMode(int state)
{
	_stay_mode = state;
	if(_stay_mode == 1)
	{
		//需要记录当前的坐标
		_self->GetPos(_stay_pos);
	}
}

void 
gpet_policy::RollBack()
{
	_self->ActiveCombatState(false);
	EnableCombat(false,false);
	_self->ActiveInvisible(true);
	_self->ClearDamageList();
	_cur_event_hp = _self->GetHP();
	_policy_flag = 0;
	if(_stay_mode == 1)
	{
		AddPosTask<ai_returnhome_task>(_stay_pos);
	}
}

int 
gpet_policy::GetInvincibleTimeout()
{
	return 0;
}

void 
gpet_policy::SetPetAutoSkill(int skill_id, int skill_level, int range_type)
{
	_auto_skill_id = skill_id;
	_auto_skill_level = skill_level;
	_auto_skill_type = range_type;
}

bool 
gpet_policy::CheckCoolDown(size_t idx)
{
	return _self->GetImpl()->CheckCoolDown(idx);
}

bool
gpet_policy::CheckMp(int skill_id, int skill_level)
{
	return  _self->GetMP() >= GNET::SkillWrapper::GetMpCost(skill_id, object_interface(_self->GetImpl()), skill_level);
}

int 
gpet_imp_2::MessageHandler(world * pPlane, const MSG & msg)
{
	switch(msg.message)
	{
		case GM_MSG_PET_CTRL_CMD:
		return 0;

		case GM_MSG_ATTACK:
		return 0;
		
		case GM_MSG_ENCHANT:
		return 0;

		case GM_MSG_DUEL_HURT:
		return 0;

		case GM_MSG_PET_LEVEL_UP:
		return 0;

		case GM_MSG_PET_HONOR_MODIFY:
		return 0;

		case GM_MSG_MASTER_ASK_HELP:
		return 0;

		case GM_MSG_PET_AUTO_ATTACK:
		return 0;

		case GM_MSG_HATE_YOU:
		return 0;
		
		case GM_MSG_PET_TEST_SANCTUARY:
		return 0;

	default:
		return gpet_imp::MessageHandler(pPlane,msg);
	}
}

void 
gpet_imp::Notify_StartAttack(const XID & target,char force_attack)
{
	if(target != _last_attack_target)
	{
		SendTo<0>(GM_MSG_PET_ANTI_CHEAT,_leader_id,0);
		_last_attack_target = target;
	}
}

void 
gpet_plant_imp::InitSkill()
{
	//植物的第一个技能设置为自动技能
	if(_skills[0].skill)
	{
		gnpc_ai * pAI = ((gnpc_controller*)_commander)->GetAI();
		if(pAI)
		{
			ai_policy * pPolicy = pAI->GetAICtrl();	
			int range_type = GNET::SkillWrapper::RangeType(_skills[0].skill);
			pPolicy->SetPetAutoSkill(_skills[0].skill, _skills[0].level, range_type);
		}
	}
	//第二个技能设置为自爆技能
	if(_skills[1].skill)
	{
		_suicide_skill_id = _skills[1].skill;
		_suicide_skill_level = _skills[1].level;
	}
}

void 
gpet_plant_imp::NotifyDeathToMaster()
{
	SendTo<0>(GM_MSG_PLANT_PET_NOTIFY_DEATH, _leader_id, _pet_stamp);
}

int 
gpet_plant_imp::MessageHandler(world * pPlane, const MSG & msg)
{
	switch(msg.message)
	{
		case GM_MSG_PET_CTRL_CMD:
		return 0;

		case GM_MSG_PET_CHANGE_POS:
		return 0;
		
		case GM_MSG_PET_LEVEL_UP:
		return 0;

		case GM_MSG_PET_HONOR_MODIFY:
		return 0;

		case GM_MSG_PET_SKILL_LIST:
		return 0;
		
		case GM_MSG_MASTER_ASK_HELP:
		{
			if(msg.content_length == sizeof(XID))
			{
				XID who = *(XID*)msg.content;
				if(who.IsValid() && msg.source == _leader_id)
				{
					if(_aggro_state == PET_AGGRO_AUTO)
					{
						//是攻击性植物
						gnpc_ai * pAI = ((gnpc_controller*)_commander)->GetAI();
						aggro_policy * pAggro = pAI->GetAggroCtrl();
						ai_policy * pPolicy = pAI->GetAICtrl();
						if(pAggro->AddAggro(who,2,2) == 0)
						{
							pPolicy->OnAggro();
						}
					}
				}
			}
		}
		return 0;
		
		case GM_MSG_PET_AUTO_ATTACK:
		{
			if(msg.content_length == sizeof(XID))
			{
				XID who = *(XID*)msg.content;
				char force_attack = msg.param & 0xFF;
				if(who.IsValid() && msg.source == _leader_id)
				{
					if(_aggro_state == PET_AGGRO_AUTO)
					{
						//是攻击性植物
						//必须先设置强制攻击标志，否则后面的OnAggro可能无法攻击目标
						_leader_force_attack = force_attack; 
						_master_attack_target = who.id;
						gnpc_ai * pAI = ((gnpc_controller*)_commander)->GetAI();
						aggro_policy * pAggro = pAI->GetAggroCtrl();
						ai_policy * pPolicy = pAI->GetAICtrl();
						if(pAggro->AddToFirst(who,2) == 0)
						{
							pPolicy->OnAggro();
						}
					}
				}
			}
		}
		return 0;
		
		case GM_MSG_PLANT_PET_SUICIDE:
		{
			if(msg.source == _leader_id && msg.content_length == sizeof(XID) && _suicide_skill_id > 0) 
			{
				XID who = *(XID*)msg.content;
				//必须先设置强制攻击标志
				_leader_force_attack = msg.param & 0xFF; 
				_master_attack_target = who.id;
				int next;
				if(NPCStartSkill(_suicide_skill_id,_suicide_skill_level,who,next) >= 0)
				{
					NPCEndSkill(_suicide_skill_id,_suicide_skill_level,who);
				}
				//延迟0.5s消失，使客户端有时间播放技能施放起施放落攻击特效击中特效
				LazySendTo<0>(GM_MSG_PET_DISAPPEAR,_parent->ID,0,TICK_PER_SEC/2);
			}				
		}
		return 0;
		
		default:
		return gpet_imp::MessageHandler(pPlane,msg);
	}
}

void 
gpet_plant_imp::NotifyMasterInHeartbeat()
{
	_notify_master_counter  ++;
	if(_notify_master_counter >= 5 || _hp_notified != _basic.hp || _mp_notified != _basic.mp)
	{
		msg_plant_pet_hp_notify info = {_basic.hp / (float)_cur_prop.max_hp , _basic.hp, _basic.mp / (float)_cur_prop.max_mp , _basic.mp};

		SendTo<0>(GM_MSG_PLANT_PET_NOTIFY_HP, _leader_id, _pet_stamp,&info,sizeof(info));
		_hp_notified = _basic.hp;
		_mp_notified = _basic.mp;
		_notify_master_counter = 0;
	}
}

void 
gpet_plant_imp::PetRelocatePos(bool is_disappear)
{
	//植物宠只能消失
	ASSERT(is_disappear);
	if(_leader_id.IsValid())
	{
		SendTo<0>(GM_MSG_PLANT_PET_NOTIFY_DISAPPEAR,_leader_id,_pet_stamp);
	}
}

void 
gpet_plant_policy::OnHeartbeat()
{
	ai_policy::OnHeartbeat();
	
	if(_aggro_state == gpet_imp::PET_AGGRO_AUTO)
	{
		//攻击类植物如果不在战斗状态，那么就进行敌人的收集 
		if(!InCombat() && !_cur_task)
		{
			GatherTarget();
		}
	}
	else if(_aggro_state == gpet_imp::PET_AGGRO_PASSIVE)
	{
		//辅助类植物查询视野内最近的主人或队友，自动使用技能
		if(!_cur_task && _auto_skill_id && CheckCoolDown(GNET::SkillWrapper::GetCooldownID(_auto_skill_id)) && CheckMp(_auto_skill_id,_auto_skill_level))
		{
			XID target(-1,-1);
			if(_self->PetGetNearestTeammate(_self->GetSightRange(),target))
			{
				AddAutoCastSkill(target);
			}
		}
	}
	
	//检查自己与主人的距离
	XID leader = _self->GetLeaderID();
	ai_object::target_info info;
	int target_state = _self->QueryTarget(leader,info);
	if(target_state != ai_object::TARGET_STATE_NORMAL)
	{
		//主人不存在，让自己消失
		XID id;
		_self->GetID(id);
		_self->SendMessage(id,GM_MSG_PET_DISAPPEAR);
		return;
	}

	//如果超出距离就直接消失
	A3DVECTOR selfpos;
	_self->GetPos(selfpos);
	float range = info.pos.horizontal_distance(selfpos);
	float h = fabs(selfpos.y - info.pos.y);
	if(h > 60.f || range >= 60.f*60.f || (range < 6.f*6.f && h > 40.f))
	{
		//需要让自己消失，通知master
		_self->PetRelocatePos(true);
		return ;
	}
}

void gpet_plant_policy::RollBack()
{
	_self->ActiveCombatState(false);
	EnableCombat(false,false);
	_self->ActiveInvisible(true);
	_self->ClearDamageList();
	_cur_event_hp = _self->GetHP();
	_policy_flag = 0;
}

