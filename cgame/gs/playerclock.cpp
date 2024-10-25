#include <stdio.h>
#include <functional>
#include <timer.h>
#include "world.h"
#include "playerclock.h"
#include "actobject.h"
#include "playertemplate.h"

tm player_clock::dbgtime;
const int player_clock::GPC_INTERVAL[GPC_TYPE_MAX] = { 3600, GPC_DAY_SEC, GPC_DAY_SEC*7, GPC_DAY_SEC*31 , 3600, GPC_DAY_SEC, GPC_DAY_SEC*7, GPC_DAY_SEC*31 };

int player_clock::GetWeekNum(int nowtime)
{
	struct tm t0 ,t1;		
	localtime_r((time_t *)&nowtime, &t0); 
	t1 = t0;
	t0.tm_mday = 1;			
	time_t time0 = mktime(&t0);			
	localtime_r((time_t *)&time0, &t0); 
	int weeknum = (t1.tm_mday - 1) / 7; // 距离1号过了几周
	int daygap = (8 - t0.tm_wday) % 7;	// 1号距离周1有几天
	if((t1.tm_mday-1)%7 >= daygap) weeknum ++; // 当前时间过了几个周1

	return weeknum;
}

int player_clock::GetMonthDiff(int oldtime, int nowtime)
{
	if(oldtime > nowtime) 
	{
		int temp = nowtime;
		nowtime = oldtime;
		oldtime = temp;
	}
	
	struct tm t0 ,t1;
	localtime_r((time_t *)&oldtime, &t0);
	localtime_r((time_t *)&nowtime, &t1);
	return (t1.tm_year - t0.tm_year)*12 + (t1.tm_mon - t0.tm_mon);
}

int player_clock::GetMonthDayDiff(int year ,int mon)
{
	static int leapyear[12]   = {0, 2, 0, 1, 0, 1,  0, 0, 1, 0, 1, 0};
	static int normalyear[12] = {0, 3, 0, 1, 0, 1,  0, 0, 1, 0, 1, 0};

	return (year%4 == 0) && (year%100 != 0 || year%400 == 0) ? 
		leapyear[mon] : normalyear[mon];
}

int player_clock::GetNextUpdatetime(int type,int nowtime)
{
	if(type >= GPC_TYPE_MAX) return 0;

	struct tm tt;
	localtime_r((time_t *)&nowtime, &tt);
	if(type != GPC_PER_HOUR_GLOBAL && type != GPC_PER_HOUR_LOCAL) tt.tm_hour = 0;
	tt.tm_min = 0;
	tt.tm_sec = 0;
	int tstub = mktime(&tt);

	// set to origin
	switch(type)
	{
		case GPC_PER_WEEK_GLOBAL:
		case GPC_PER_WEEK_LOCAL:
			{
				tstub -= GPC_DAY_SEC * (tt.tm_wday ? tt.tm_wday-1 : 6);
			}
			break;
		case GPC_PER_MONTH_GLOBAL:
		case GPC_PER_MONTH_LOCAL:
			{
				tstub -= GPC_DAY_SEC * (tt.tm_mday - 1 + GetMonthDayDiff(tt.tm_year+1900,tt.tm_mon));
			}
			break;
	}

	return tstub + GPC_INTERVAL[type] ;
}

int player_clock::GetPassPeriod(int type, int oldtime, int nowtime)
{
	ASSERT(oldtime <= nowtime && "闹钟计算不存在老时间大于新时间");

	int passtime = nowtime - oldtime;
	int period = 0;

	if(type == GPC_PER_MONTH_LOCAL || type == GPC_PER_MONTH_GLOBAL) // 月的间隔不统一
	{
		int cost = GetNextUpdatetime(type, oldtime) - oldtime;
		while(passtime >= cost && period < 12) // 最多1年
		{
			++period;
			oldtime += cost;
			passtime -= cost;
			cost = GetNextUpdatetime(type, oldtime) - oldtime;
		}	
	}
	else
	{
		if(GPC_INTERVAL[type])	period = passtime / GPC_INTERVAL[type];	
	}	

	return period;
}

bool player_clock::CheckPrviateCond(int type, int cond, int stubtime, int nowtime, int passperiod, bool nopass)
{
	struct tm tt;
	localtime_r((time_t *)&nowtime, &tt);

	switch(type)
	{
		case GPC_PER_HOUR_GLOBAL:
		case GPC_PER_HOUR_LOCAL:
			{
				if(tt.tm_hour == cond) return true;
				if(nopass) return false;
				if(tt.tm_hour > cond && passperiod >= tt.tm_hour - cond) return true;
				if(tt.tm_hour < cond && passperiod >= tt.tm_hour + 24 - cond) return true;
			}
			break;
		case GPC_PER_DAY_GLOBAL:
		case GPC_PER_DAY_LOCAL:
			{
				if(tt.tm_wday == cond) return true;
				if(nopass) return false;
				if(tt.tm_wday > cond && passperiod >= tt.tm_wday - cond) return true;
				if(tt.tm_wday < cond && passperiod >= tt.tm_wday + 7 - cond) return true;
			}
			break;
		case GPC_PER_WEEK_GLOBAL:
		case GPC_PER_WEEK_LOCAL:
			{
				int weeknum = GetWeekNum(nowtime);
				if(weeknum == cond) return true;
				if(nopass) return false;
				if(GetMonthDiff(stubtime,nowtime) >= 2) return true;
				if(weeknum > cond && passperiod >= weeknum - cond) return true; 
				int stubweek = GetWeekNum(stubtime);
				if(stubweek <= cond && stubweek + passperiod >= cond)  return true; 
			}
			break;
		case GPC_PER_MONTH_GLOBAL:
		case GPC_PER_MONTH_LOCAL:
			{
				if(tt.tm_mon == cond) return true;
				if(nopass) return false;
				if(tt.tm_mon > cond && passperiod >= tt.tm_mon - cond) return true;
				if(tt.tm_mon < cond && passperiod >= tt.tm_mon + 12 - cond) return true;
			}
			break;
	}

	return false;
}

void player_clock::OnHeartbeat(gplayer_imp* player,int now,bool incentral)
{
	if(--_idle_time > 0) return;
	if(GPC_UNINIT == _state) return;
	if(player_template::GetDebugMode() && player_clock::dbgtime.tm_year) now = mktime(&player_clock::dbgtime);

	if(_state == GPC_INIT) 
	{
		_state = GPC_NORMAL;
		InitCheck(player,now,incentral);
	}
	else 	
		CheckTime(player,now,incentral); 
	
	_idle_time = GPC_HB_IDLE;
}

void player_clock::SaveToDB(archive & ar)
{
	ar << _update_time.size();
	for(size_t i = 0; i < _update_time.size(); ++i)
	{
		ar << _update_time[i].lasttime;
		ar << _update_time[i].nexttime;
	}
}

void player_clock::InitFromDB(archive & ar,int roleid)
{
	if(0 != ar.size())
	{		
		try
		{
			size_t upsize = 0;
			ar >> upsize;	
			if(upsize > _update_time.size()) 
			{
				GLog::log(GLOG_ERR,"用户%d时钟装载老数据过大",roleid);
				return;
			}

			for(size_t i = 0; i < upsize; ++i)
			{
				ar >> _update_time[i].lasttime;
				ar >> _update_time[i].nexttime;
			}
		}
		catch(...)
		{
			GLog::log(GLOG_ERR,"用户%d时钟装载出错",roleid);
			_update_time.clear();
			return;
		}
	}

	_state = GPC_INIT;
}

#define IS_LOCAL_CLOCK_TYPE(t) (t >= GPC_PER_HOUR_LOCAL && t <= GPC_PER_MONTH_LOCAL)

void player_clock::CheckTime(gplayer_imp* player,int now,bool incentral)
{
	for(size_t type = 0; type < _update_time.size(); ++type)
	{
		if(_update_time[type].nexttime <= now && !(incentral && IS_LOCAL_CLOCK_TYPE(type)))
		{
			NOTICE_NODE_LIST::iterator iter = _notice_list[type].begin();	
			NOTICE_NODE_LIST::iterator iend = _notice_list[type].end();	

			for(;iter != iend ; ++ iter)
			{
				iter->Run(player, type, now);		
			}

			_update_time[type].lasttime = now;
			_update_time[type].nexttime = GetNextUpdatetime(type,now);
		}
	}	
}

void player_clock::InitCheck(gplayer_imp* player,int now,bool incentral)
{
	for(size_t type = 0; type < _update_time.size(); ++type)
	{
		if(_update_time[type].nexttime <= now && !(incentral && IS_LOCAL_CLOCK_TYPE(type)))
		{
			NOTICE_NODE_LIST::iterator iter = _notice_list[type].begin();	
			NOTICE_NODE_LIST::iterator iend = _notice_list[type].end();	

			int passperiod = GetPassPeriod(type,_update_time[type].nexttime,now);
			
			for(;iter != iend ; ++ iter)
			{
				iter->Init(player, type, _update_time[type].lasttime, _update_time[type].nexttime, now, passperiod);		
			}
			
			_update_time[type].lasttime = now;
			_update_time[type].nexttime = GetNextUpdatetime(type,now);
		}
	}	
}

#undef IS_LOCAL_CLOCK_TYPE

void player_clock::Reset(gplayer_imp* player,int type)
{ 
	if(type >= 0 && type < (int)_update_time.size()) 
	{	
		_update_time[type].lasttime = 0; 
		_update_time[type].nexttime = 0;
	}
	else if(type == -1)
	{
		_update_time.clear();
		_update_time.insert(_update_time.begin(),GPC_TYPE_MAX,player_clock::clock_update());
	}
	else
	{
		InitCheck(player,mktime(&player_clock::dbgtime),false);
	}
}

