#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <common/protocol.h>
#include <arandomgen.h>
#include "string.h"
#include "world.h"
#include "servicenpc.h"
#include "clstab.h"
#include "npcgenerator.h"
#include "faction.h"
#include "towerbuild_filter.h"
#include <gsp_if.h>
#include "template/city_region.h"

DEFINE_SUBSTANCE(service_npc,gnpc_imp,CLS_SERVICE_NPC_IMP)

int 
service_npc::GetCurIDMafia()
{
	if(!_need_domain) return 0;

	int t = g_timer.get_systime();
	if(t > _domain_test_time + 60) 
	{
		_domain_test_time = t;
		int domain_id = city_region::GetDomainID(_parent->pos.x,_parent->pos.z);
		if(domain_id > 0)
		{
			_domain_mafia = GMSV::GetCityOwner(domain_id);
			if(_domain_mafia <= 0) _domain_mafia = -1;
		}
		else
		{
			_domain_mafia = -1;
			
		}
	}
	return _domain_mafia;
}

bool 
service_npc::CheckDistanceLimit(const A3DVECTOR & pos)
{
	if(_serve_distance_unlimited) return true;

	float s = 6 + _parent->body_size;
	return pos.squared_distance(_parent->pos) <= s * s;
}

int 
service_npc::MessageHandler(world * pPlane ,const MSG & msg)
{
	switch(msg.message)
	{
		case GM_MSG_SERVICE_REQUEST:
		{
			if(!CheckDistanceLimit(msg.pos))
			{
				SendTo<0>(GM_MSG_ERROR_MESSAGE,msg.source,S2C::ERR_SERVICE_UNAVILABLE);
				return 0;
			}
			//对服务的请求到来(要求服务)
			service_provider * provider = _service_list.GetProvider(msg.param);
			if(provider)
			{
				provider->PayService(msg.source,msg.content,msg.content_length);
			}
			else
			{
				//报告错误
				SendTo<0>(GM_MSG_ERROR_MESSAGE,msg.source,S2C::ERR_SERVICE_UNAVILABLE);
			}
		}
		return 0;
		
		case GM_MSG_SERVICE_HELLO:
		{
			if(CheckDistanceLimit(msg.pos))
			{
				int faction = msg.param & (~FACTION_PARIAH);
				if(!(faction & GetEnemyFaction()))
				{
					//检查帮派信息
					int id_mafia = GetCurIDMafia();
					SendTo<0>(GM_MSG_SERVICE_GREETING,msg.source,id_mafia);
				}
			}
		}
		return 0;


		case GM_MSG_SERVICE_QUIERY_CONTENT:
		{
			if(!CheckDistanceLimit(msg.pos))
			{
				SendTo<0>(GM_MSG_ERROR_MESSAGE,msg.source,S2C::ERR_SERVICE_UNAVILABLE);
			}
		}
		//return 0;

		//现在npc暂时没有服务数据回传，以后会有的
		if(msg.content_length == sizeof(int) * 2)
		{
			int cs_index = *(int*)msg.content;
			int sid = *((int*)msg.content + 1);
			service_provider * provider = _service_list.GetProvider(msg.param);
			if(provider)
			{
				provider->GetServiceContent(msg.source, cs_index, sid);
			}
		}
		else
		{
			ASSERT(false);
		}
		return 0;

		case GM_MSG_ATTACK:
		{
			ASSERT(msg.content_length >= sizeof(attack_msg));
			//不能攻击自己和队友
			bool IsInvader = false;
			attack_msg & amsg = *(attack_msg*)msg.content;
			if(amsg.ainfo.attacker.IsPlayerClass())
			{
				//if(!_pvp_enable_flag) return 0;
				/*
				if(!(amsg.attacker_mode & attack_msg::PVP_ENABLE))
				{
					return 0;
				}
				else
				{
					IsInvader = true;
				}
				if(!amsg.force_attack) return 0;
				*/
				IsInvader = !(GetFaction() & amsg.target_faction);
				if(IsInvader && !amsg.force_attack) return 0;
			}
			
			//这里由于都要进行复制操作，有一定的耗费存在
			attack_msg ack_msg = *(attack_msg*)msg.content;
			ack_msg.is_invader =  IsInvader;
			HandleAttackMsg(pPlane,msg,&ack_msg);
		}
		return 0;

		case GM_MSG_ENCHANT:
		{
			ASSERT(msg.content_length >= sizeof(enchant_msg));
			enchant_msg emsg = *(enchant_msg*)msg.content;
			if(!emsg.helpful)
			{
				//有害法术的攻击判定类似攻击
				bool IsInvader = false;
				if(emsg.ainfo.attacker.IsPlayerClass())
				{
					//if(!_pvp_enable_flag) return 0;
					/*
					if(!(emsg.attacker_mode & attack_msg::PVP_ENABLE))
					{
						return 0;
					}
					else
					{
						IsInvader = true;
					}
					if(!emsg.force_attack) return 0;
					*/

					IsInvader = !(GetFaction() & emsg.target_faction);
					if(IsInvader && !emsg.force_attack) return 0;
				}
				emsg.is_invader = IsInvader;
			}
			else
			{
				if(_no_accept_player_buff && msg.source.IsPlayerClass()) return 0;
			}
			HandleEnchantMsg(pPlane,msg,&emsg);
		}
		return 0;

		case GM_MSG_NPC_TRANSFORM:
		{
			ASSERT(msg.content_length == sizeof(msg_npc_transform));
			if(msg.content_length == sizeof(msg_npc_transform)) 
			{
				msg_npc_transform * data = (msg_npc_transform*)msg.content;
				
				gnpc_imp * __this = TransformMob(data->id_in_build);
				if(__this == NULL) 
				{
					GLog::log(GLOG_ERR,"NPC在转换的时候发生错误");
					return 0;
				}
				

				//加入定时控制filtero
				filter * pFilter = new towerbuild_filter(__this,FILTER_INDEX_TOWERBUILD,
									data->id_buildup,data->time_use);
				__this->_filters.AddFilter(pFilter);

				//发送更换状态数据
				__this->_runner->disappear();
				__this->_runner->enter_world();
				
			}
		}
		return 0;


	default:
		return gnpc_imp::MessageHandler(pPlane,msg);
	}
	return 0;
}

service_npc::~service_npc()
{
}

bool 
service_npc::Save(archive & ar)
{
	gnpc_imp::Save(ar);
	ar << _tax_rate;
	size_t size = _service_list.size();
	ar << size;

	for(LIST::iterator it = _service_list.begin();it != _service_list.end(); ++it)
	{
		ar << it->second->GetProviderType();
		it->second->Save(ar);
	}
	ar << _life_time << _src_monster;
	return true;
}

bool 
service_npc::Load(archive & ar)
{
	gnpc_imp::Load(ar);
	ar >> _tax_rate;
	size_t size;
	ar >> size;
	for(size_t i = 0;i < size; i ++)
	{
		int provider_id;
		ar >> provider_id;
		service_provider * provider = service_manager::CreateProviderInstance(provider_id);
		ASSERT(provider);
		provider->Load(ar);
		provider->ReInit(this);
		_service_list.AddProvider(provider);
	}
	ar >> _life_time >> _src_monster;
	return true;
}

void 
service_npc::OnNpcEnterWorld()
{
	if(_serve_distance_unlimited) _plane->AddSceneServiceNpc(GetNPCID(), _parent->ID.id);
}

void 
service_npc::OnNpcLeaveWorld()
{
	if(_serve_distance_unlimited) _plane->DelSceneServiceNpc(GetNPCID(), _parent->ID.id);
}

int
service_npc::DoAttack(const XID & target,char force_attack)
{
	attack_msg attack;
	MakeAttackMsg(attack,force_attack);
	FillAttackMsg(target,attack);

	//设置PVP强制攻击
	attack.attacker_mode = attack_msg::PVP_ENABLE;
	attack.force_attack = true;

	attack.speed = _damage_delay;

	MSG msg;
	BuildMessage(msg,GM_MSG_ATTACK,target,_parent->ID,_parent->pos,0,&attack,sizeof(attack));
	TranslateAttack(target,attack);
	_plane->PostLazyMessage(msg);

	return 0;
}

void    
service_npc::FillAttackMsg(const XID & target, attack_msg & attack)
{       
	gactive_imp::FillAttackMsg(target,attack);
	//设置PVP强制攻击
	attack.attacker_mode = attack_msg::PVP_ENABLE;
	attack.force_attack = true;
}

void    
service_npc::FillEnchantMsg(const XID & target, enchant_msg & enchant)
{       
	gactive_imp::FillEnchantMsg(target, enchant);
	//设置PVP强制攻击
	enchant.attacker_mode = attack_msg::PVP_ENABLE;
	enchant.force_attack = true;
}

void 
service_npc::OnHeartbeat(size_t tick)
{
	gnpc_imp::OnHeartbeat(tick);	
	if(_life_time > 0)
	{
		if((_life_time -= tick) <= 0)
		{
			SendTo<0>(GM_MSG_DEATH,_parent->ID,0);	
		}
	}
}

void 
service_npc::SetLifeTime(int life)
{
	if(((gnpc_controller*)_commander)->GetAI())
		((gnpc_controller*)_commander)->SetLifeTime(life);
	else
		_life_time = life;
}
