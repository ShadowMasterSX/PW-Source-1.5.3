#include "string.h"
#include "world.h"
#include "pvplimit_filter.h"
#include "clstab.h"
#include "actobject.h"
#include "playertemplate.h"
#include <glog.h>

#include "world.h"
#include "player_imp.h"

DEFINE_SUBSTANCE(pvp_limit_filter,filter,CLS_FILTER_PVP_LIMIT)

void 
pvp_limit_filter::OnAttach()
{
	//发送进入安全区的消息
	_parent.GetImpl()->_runner->enter_sanctuary();

	gobject * pObj = _parent.GetImpl()->_parent;
	GLog::log(GLOG_INFO,"用户%d进入安全区(%f,%f)",pObj->ID.id,pObj->pos.x,pObj->pos.z);

    _parent.GetImpl()->SetHasPVPLimitFilter(true);
}

void 
pvp_limit_filter::OnRelease()
{
	//通知客户端退出了安全区
	_parent.GetImpl()->_runner->leave_sanctuary();

	gobject * pObj = _parent.GetImpl()->_parent;
	GLog::log(GLOG_INFO,"用户%d离开安全区(%f,%f)",pObj->ID.id,pObj->pos.x,pObj->pos.z);

    _parent.GetImpl()->SetHasPVPLimitFilter(false);
}


void 
pvp_limit_filter::TranslateRecvAttack(const XID & attacker,attack_msg & msg)
{
	msg.attacker_mode &= ~attack_msg::PVP_ENABLE;
	msg.force_attack = 0;
}       

void 
pvp_limit_filter::TranslateEnchant(const XID & attacker,enchant_msg & msg)
{
	msg.attacker_mode &= ~attack_msg::PVP_ENABLE;
	msg.force_attack = 0;
}

void 
pvp_limit_filter::TranslateSendAttack(const XID & target,attack_msg & msg)
{
	msg.attacker_mode &= ~attack_msg::PVP_ENABLE;
	msg.force_attack = 0;
}

void 
pvp_limit_filter::TranslateSendEnchant(const XID & target,enchant_msg & msg)
{
	msg.attacker_mode &= ~attack_msg::PVP_ENABLE;
	msg.force_attack = 0;
}


void 
pvp_limit_filter::Heartbeat(int tick)
{
	if((_counter += 1) < 7) return;
	//每隔7秒检查是否该脱离了安全区
	_counter = 0;
	if(!player_template::IsInSanctuary(_parent.GetPos()))
	{
		_is_deleted = true;
	}
}

