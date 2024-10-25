#include <glog.h>
#include "playerfatering.h"

#include "world.h"
#include "player_imp.h"

bool fate_ring_enum::Enhance(gplayer_imp * imp, int type) const
{
	return world_manager::GetFateRingMan().EnhanceFateRing(imp,type,level);
}

bool fate_ring_enum::Impair(gplayer_imp * imp, int type) const
{
	return world_manager::GetFateRingMan().ImpairFateRing(imp,type,level);
}

void player_fatering::Swap(player_fatering & rhs)
{
	abase::swap(next_update_time, rhs.next_update_time);
	abase::swap(gain_times, rhs.gain_times);
	fate_rings.swap(rhs.fate_rings);
}

bool player_fatering::Save(archive & ar)
{
	ar << next_update_time;
	ar << gain_times;
	ar << fate_rings.size();
	if (!fate_rings.empty())
	{
		size_t size = fate_rings.size();
		for (size_t i = 0; i < size; i++)
		{
			ar << fate_rings[i].level;
			ar << fate_rings[i].power;
		}
	}
	return true;
}

bool player_fatering::Load(archive & ar)
{
	try
	{
		ar >> next_update_time;
		ar >> gain_times;
		size_t frsize = 0;
		ar >> frsize;
		if (frsize && frsize <= PLAYER_FATE_RING_TOTAL)
		{
			for (size_t i = 0; i < frsize; i++)
			{
				ar >> fate_rings[i].level;
				ar >> fate_rings[i].power;
			}
		}
		else
		{
			GLog::log(LOG_ERR,"roleid:%d player_fatering Load failed, size=%d.",_owner->_parent->ID.id,frsize);
			return false;
		}
	}
	catch(...)
	{
		GLog::log(LOG_ERR,"roleid:%d player_fatering Load failed.",_owner->_parent->ID.id);
		return false;
	}
	return true;
}

bool player_fatering::InitFromDB(archive & ar)
{
	if (0 == ar.size())
	{
		//设置一下每日次数
		CheckRefresh(g_timer.get_systime());
		return true;
	}

	try
	{
		ar >> next_update_time;
		ar >> gain_times;
		size_t frsize = 0;
		ar >> frsize;
		if (frsize && frsize <= PLAYER_FATE_RING_TOTAL)
		{
			for (size_t i = 0; i < frsize; i++)
			{
				ar >> fate_rings[i].level;
				ar >> fate_rings[i].power;
			}
		}
		else
		{
			GLog::log(LOG_ERR,"roleid:%d player_fatering InitFromDB failed, size=%d.",_owner->_parent->ID.id,frsize);
			return false;
		}
	}
	catch(...)
	{
		GLog::log(LOG_ERR,"roleid:%d player_fatering InitFromDB failed.",_owner->_parent->ID.id);
		return false;
	}

	__PRINTF("player_fatering InitFromDB roleid=%d next_update_time=%d gain_times=%d.\n",_owner->_parent->ID.id,next_update_time,gain_times);

	//检查是否更新每日次数
	CheckRefresh(g_timer.get_systime());
	return true;
}

bool player_fatering::SaveToDB(archive & ar)
{
	ar << next_update_time;
	ar << gain_times;
	ar << fate_rings.size();
	if (!fate_rings.empty())
	{
		size_t size = fate_rings.size();
		for (size_t i = 0; i < size; i++)
		{
			ar << fate_rings[i].level;
			ar << fate_rings[i].power;
		}
	}
	return true;
}

void player_fatering::NotifyClientRefresh() const
{
	packet_wrapper h1(64);
	using namespace S2C;
	size_t size = fate_rings.size();
	CMD::Make<CMD::refresh_fatering>::From(h1, gain_times,size);
	for(size_t i=0; i<size; i++)
	{
		const fate_ring_enum & fre = fate_rings[i];
		CMD::Make<CMD::refresh_fatering>::Add(h1, fre.level, fre.power);
	}
	send_ls_msg(_owner->GetParent(), h1);
}

void player_fatering::OnHeartbeat(int curtime)
{
	if (CheckRefresh(curtime))
		NotifyClientRefresh();
}

bool player_fatering::CheckRefresh(int curtime)
{
	if (curtime >= next_update_time)
	{
		struct tm tt;
		localtime_r((time_t *)&curtime, &tt);
		tt.tm_hour = 0;
		tt.tm_min = 0;
		tt.tm_sec = 0;

		int days = PLAYER_FATE_RING_UPDATE_WEEKDAY - tt.tm_wday;
		if (days <= 0) days += 7;
	
		next_update_time = mktime(&tt)+86400*days;
		gain_times = 0;

		return true;
	}
	return false;
}

bool player_fatering::OnUseSoul(int type, int level, int power)
{
	__PRINTF("player_fatering OnUseSoul roleid=%d type=%d level=%d power=%d.\n",_owner->_parent->ID.id,type,level,power);

	if (type < 0 || type >= PLAYER_FATE_RING_TOTAL ||
		level < 1 || level > PLAYER_FATE_RING_MAX_LEVEL ||
		power <= 0)
		return false;
	
	fate_ring_enum & fre = fate_rings[type];
	if (fre.level >= PLAYER_FATE_RING_TOP_LEVEL_OPEN)
	{
		_owner->_runner->error_message(S2C::ERR_USE_SOUL_TOP_LEVEL);
		return false;
	}

	int add_power = (int)(power * player_template::GetUseSoulExpAdjust(fre.level, level));
	
	if (!AddFateRingPower(type, add_power))
	{
		_owner->_runner->error_message(S2C::ERR_USE_SOUL_EXP_FULL);
		return false;
	}
	
	return true;
}

bool player_fatering::AddFateRingPower(int type, int power)
{
	if (type < 0 || type >= PLAYER_FATE_RING_TOTAL || power <= 0)
		return false;

	fate_ring_enum & fre = fate_rings[type];
	if (fre.level >= PLAYER_FATE_RING_TOP_LEVEL_OPEN)
		return false;

	int require_level=0,require_power=0;
	if (!world_manager::GetFateRingMan().GetFateRingLevelUpRequire(type,fre.level+1,require_level,require_power))
		return false;

	if (_owner->GetHistoricalMaxLevel() < require_level && require_power - fre.power <= 1)
		return false;

	fre.power += power;

	bool lp = false;
	do
	{
		if (fre.power < require_power) break;
		if (_owner->GetHistoricalMaxLevel() < require_level)	//未达到命轮升级所需要的人物等级
		{
			fre.power = require_power - 1;
			break;
		}

		if (!lp)
		{
			lp = true;
			//先从玩家身上把属性扣除
			fre.Impair(_owner,type);
		}

		fre.level ++;
		fre.power -= require_power;
		
		if (fre.level >= PLAYER_FATE_RING_TOP_LEVEL_OPEN)
		{
			fre.power = 0;
			break;
		}
		if (!world_manager::GetFateRingMan().GetFateRingLevelUpRequire(type,fre.level+1,require_level,require_power))
		{
			fre.power = 0;
			break;
		}
	}while(1);

	if (lp)
	{
		fre.Enhance(_owner,type);

		//刷新玩家属性
		property_policy::UpdatePlayer(_owner->GetPlayerClass(),_owner);	
	}

	__PRINTF("player_fatering AddFateRingPower roleid=%d type=%d level=%d power=%d.\n",_owner->_parent->ID.id,type,fre.level,fre.power);
	
	//通知客户端刷新命轮数据
	NotifyClientRefresh();
	return true;
}

void player_fatering::EnhanceAll()
{
	size_t size = fate_rings.size();
	for (size_t i = 0; i < size; i++)
	{
		fate_rings[i].Enhance(_owner,i);
	}
}

