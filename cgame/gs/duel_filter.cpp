#include "string.h"
#include "world.h"
#include "duel_filter.h"
#include "clstab.h"
#include "actobject.h"
#include "playertemplate.h"
#include <glog.h>
#include "sfilterdef.h"

DEFINE_SUBSTANCE(pvp_duel_filter,filter,CLS_FILTER_PVP_DUEL)

void 
pvp_duel_filter::OnAttach()
{
	_parent.SetNoAmulet(true);
	_parent.DuelStart(_target);
	_parent.SendClientDuelStart(_target);
}

void 
pvp_duel_filter::OnRelease()
{
	if(!_is_release)
	{
		_is_release = true;
		_parent.SetNoAmulet(false);
		_parent.DuelStop();
		_parent.SendClientDuelStop(_target);
		_parent.ClearSpecFilter(filter::FILTER_MASK_DEBUFF);
	}
}

void 
pvp_duel_filter::TranslateRecvAttack(const XID & attacker,attack_msg & msg)
{
	if(msg.ainfo.attacker == _target)
	{
		if(_mode)
		{
			//现在则是无敌
			msg.target_faction = 0;
			msg.force_attack  = 0;
		}
		else
		{
			//如何处理宠物？ 考虑在ainfo 中加入是否宠物的内容
			msg.attacker_mode |= attack_msg::PVP_DUEL;
		}
	}
}       

void 
pvp_duel_filter::TranslateEnchant(const XID & attacker,enchant_msg & msg)
{
	if(msg.ainfo.attacker == _target)
	{
		if(_mode)
		{
			if(!msg.helpful)
			{
				//现在则是无敌
				msg.target_faction = 0;
				msg.force_attack  = 0;
			}
		}
		else
		{
			if(msg.helpful)
			{
				msg.target_faction = 0xFFFFFFFF;
			}
			else
			{
				//如何处理宠物？
				msg.attacker_mode |= attack_msg::PVP_DUEL;
			}
		}
	}
}

void
pvp_duel_filter::Heartbeat(int tick)
{
	if(!_mode) return;
	_mode --;
	if(_mode <= 0)
	{
	 	_is_deleted = true;
	}
}


void 
pvp_duel_filter::OnModify(int ctrlname, void * ctrlval, size_t ctrlen)
{
	if(ctrlname == FMID_DUEL_END)
	{
		_mode = 5;
		OnRelease();
	}
}

