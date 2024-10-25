#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arandomgen.h>
#include <common/protocol.h>
#include "world.h"
#include "player_imp.h"
#include "usermsg.h"

#include "playerdailysign.h"
#include <bitset>

tm player_dailysign::dbgtime;

int GetMonthDiff(int year ,int mon)
{
	static int leapyear[MAX_SIGN_MONTH] = {0,-2,0,-1,0,-1,0,0,-1,0,-1,0};
	static int normalyear[MAX_SIGN_MONTH] = {0,-3,0,-1,0,-1,0,0,-1,0,-1,0};

	return (year%4 == 0) && (year%100 != 0 || year%400 == 0) ? 
		leapyear[mon] : normalyear[mon];
}

void player_dailysign::SaveToDB(int& uptime, int& monthcal, int& curryear, int& lastyear, char& awardedtimes, char& latesignintimes)
{
	uptime = _update_time;
	monthcal = _month_calendar;
	curryear = _curr_year;
	lastyear = _last_year;
    awardedtimes = _awarded_times;
    latesignintimes = _late_signin_times;
}

void player_dailysign::InitFromDB(int uptime, int monthcal, int curryear, int lastyear, char awardedtimes, char latesignintimes)
{
	_update_time = uptime;
	_month_calendar = monthcal;
	_curr_year = curryear;
	_last_year = lastyear;
    _awarded_times = awardedtimes;
    _late_signin_times = latesignintimes;

	CheckPoint();
}

void player_dailysign::CheckPoint()
{
	if(_update_time == 0)
	{
		_update_time = MK_LOCAL_TIME();
	}
	else
	{
		struct tm tm1,tm2;
		time_t upt = _update_time;
		time_t now = MK_LOCAL_TIME();
		localtime_r(&upt, &tm1);
		localtime_r(&now, &tm2);

		if(tm1.tm_year != tm2.tm_year)
		{
			AggregateYearSign();	
		}
		else if(tm1.tm_mon != tm2.tm_mon)
		{
			AggregateMonthSign();
		}

	}
}

void player_dailysign::AggregateMonthSign()
{
	struct tm tm1;
	time_t upt = _update_time;
	localtime_r(&upt, &tm1);

	if( _curr_year.GetState(tm1.tm_mon) ) // 可能是修改服务器系统时间所致
	{
		GLog::log(LOG_ERR,"roleid:%d aggregate month signin fail[updatetime%d-%d-%d] \n",
				_owner->_parent->ID.id, 1900+tm1.tm_year,1+tm1.tm_mon,tm1.tm_mday);
		_update_time = MK_LOCAL_TIME();
	}
	else
	{
		std::bitset<31> mo(_month_calendar);
		
		mo.flip();

		int missday = mo.count() + GetMonthDiff(tm1.tm_year+1900,tm1.tm_mon);
		
		if(missday  == 0)
		{
			_curr_year.SetState(tm1.tm_mon,NO_TIME_MISS );	
		}
		else if (missday <= 3)
		{
			_curr_year.SetState(tm1.tm_mon,UNDER_3_TIMES_MISS );	
		}
		else
		{
			_curr_year.SetState(tm1.tm_mon,OVER_3_TIMES_MISS );
		}
	}

	_month_calendar = 0;
	_update_time = MK_LOCAL_TIME();
    _awarded_times = 0;
    _late_signin_times = 0;
}

void player_dailysign::AggregateYearSign()
{
	AggregateMonthSign();
	_last_year = _curr_year;
	_curr_year.clear();
}
	
bool player_dailysign::DaySignIn(int dsttime)
{
	CheckPoint();
	
	struct tm tm1,tm2;
	time_t dst = dsttime;
	time_t now = MK_LOCAL_TIME();
	localtime_r(&dst, &tm1);
	localtime_r(&now, &tm2);
	
	if(tm1.tm_year != tm2.tm_year || tm1.tm_mon != tm2.tm_mon || tm1.tm_mday > tm2.tm_mday || tm1.tm_mday < 1)
	{
		GLog::log(LOG_ERR,"roleid:%d sign in a wrong day[d%d-%d-%d][n%d-%d-%d] \n",
				_owner->_parent->ID.id, 1900+tm1.tm_year,1+tm1.tm_mon,tm1.tm_mday, 
				1900+tm2.tm_year, 1+tm2.tm_mon, tm2.tm_mday);
		ClientSync(SYNC4ERR);
		return false;
	}

    bool is_late_signin = (tm1.tm_mday != tm2.tm_mday);
    if (is_late_signin && (_late_signin_times >= MAX_LATE_SIGNIN_TIMES))
    {
        GLog::log(LOG_ERR,"roleid:%d late signin times(%d) over max times(%d)\n",
                _owner->_parent->ID.id, _late_signin_times, MAX_LATE_SIGNIN_TIMES);
        return false;
    }

	if(_month_calendar & (1 << (tm1.tm_mday-1)))
	{
		return false;
	}

	_month_calendar |= (1 << (tm1.tm_mday-1));

    if (is_late_signin) ++_late_signin_times;

	ClientSync(tm1.tm_mday == tm2.tm_mday ? SYNC4DAILY : SYNC4LATE);

	__PRINTF("role%d signin at [%d-%d-%d]", _owner->_parent->ID.id, 1900+tm1.tm_year, 1+tm1.tm_mon, tm1.tm_mday);

	
	// 签到补签均奖励统一的经验及元神
	int exp = (_owner->_basic.level + 1)*_owner->_basic.level*10;
	int sp  = (int)(exp * 0.226f); 
	_owner->ReceiveCommonExp(exp, sp);

	return true;
}

bool player_dailysign::MonthSignIn(int dsttime)
{
	CheckPoint();
	
	struct tm tm1,tm2;
	time_t dst = dsttime;
	time_t now = MK_LOCAL_TIME();
	localtime_r(&dst, &tm1);
	localtime_r(&now, &tm2);

	if(tm1.tm_year != tm2.tm_year || tm1.tm_mon >= tm2.tm_mon)
	{
		GLog::log(LOG_ERR,"roleid:%d month late sign in a wrong day[d%d-%d-%d][n%d-%d-%d] \n",
				_owner->_parent->ID.id, 1900+tm1.tm_year,1+tm1.tm_mon,tm1.tm_mday, 
				1900+tm2.tm_year, 1+tm2.tm_mon, tm2.tm_mday);
		return false;
	}

	if(_curr_year.GetState(tm1.tm_mon) >= NO_TIME_MISS)
	{
		GLog::log(LOG_ERR,"roleid:%d month late signin no need[d%d-%d-%d][monthstate%d] \n",
				_owner->_parent->ID.id, 1900+tm1.tm_year,1+tm1.tm_mon,tm1.tm_mday, _curr_year.GetState(tm1.tm_mon)); 
		return false;
	}

	_curr_year.SetState(tm1.tm_mon, NO_TIME_MISS );	

	return true;
}

bool player_dailysign::YearSignIn(int dsttime)
{
	CheckPoint();
	
	struct tm tm1,tm2;
	time_t dst = dsttime;
	time_t now = MK_LOCAL_TIME();
	localtime_r(&dst, &tm1);
	localtime_r(&now, &tm2);

	if(tm1.tm_year != tm2.tm_year )
	{
		GLog::log(LOG_ERR,"roleid:%d year late sign in a wrong day[d%d-%d-%d][n%d-%d-%d] \n",
				_owner->_parent->ID.id, 1900+tm1.tm_year,1+tm1.tm_mon,tm1.tm_mday, 
				1900+tm2.tm_year, 1+tm2.tm_mon, tm2.tm_mday);
		return false;
	}

	for(int n = 0; n <= tm1.tm_mon; ++n)
	{
		if(_curr_year.GetState(n) < NO_TIME_MISS)
			_curr_year.SetState(n, NO_TIME_MISS );	
    }

	return true;
}

bool player_dailysign::RequestAwards(int type,int mon)
{
	CheckPoint(); 

    struct tm tm1;
    time_t now = MK_LOCAL_TIME();
    localtime_r(&now, &tm1);

    unsigned int award_id = 0;
    int award_num = 0;

    if (type & REQ_DAILY_AWARD)
    {
        std::bitset<31> mo(_month_calendar);
        if (_awarded_times >= (int)mo.count()) return false;

        DATA_TYPE dt;
        const SIGN_AWARD_CONFIG* signin = (const SIGN_AWARD_CONFIG*)world_manager::GetDataMan().get_data_ptr(AWARD_TEMPLATE_ID[tm1.tm_mon], ID_SPACE_CONFIG, dt);
        if ((signin == NULL) || (dt != DT_SIGN_AWARD_CONFIG)) return false;

        int size = sizeof(signin->list) / sizeof(signin->list[0]);
        if ((_awarded_times < 0) || (_awarded_times >= size)) return false;

        award_id = signin->list[(int)_awarded_times].id;
        award_num = signin->list[(int)_awarded_times].num;
    }

	int needslots = 0;
    if ((type & REQ_YEAR_AWARD) || (type & REQ_PERFECT_AWARD)) ++needslots;
    if (type & REQ_DAILY_AWARD) ++needslots;

	if(!_owner->InventoryHasSlot(needslots))
	{
		GLog::log(LOG_ERR,"roleid:%d request award has not enough slot[%d] \n", 
				_owner->_parent->ID.id, needslots);
		return false;
	}

	if((type & REQ_YEAR_AWARD) && 
		(_last_year.CheckYearAward() != YEAR_AWARD || _last_year.IsYearAwardClose() || tm1.tm_mon != REQ_YEAR_AWARD_MONTH) )
	{	
		GLog::log(LOG_ERR,"roleid:%d request YR award fail[%d][%d][%d]\n", 
				_owner->_parent->ID.id, _last_year.CheckYearAward(), _last_year.IsYearAwardClose(), tm1.tm_mon);
		return false;
	}

    if ((type & REQ_PERFECT_AWARD) &&
        ((_last_year.CheckYearAward() != PERFECT_AWARD) || _last_year.IsPerfectAwardClose() || (tm1.tm_mon != REQ_YEAR_AWARD_MONTH)))
    {
        GLog::log(LOG_ERR, "roleid:%d request YR Perfect award fail[%d][%d][%d]\n", _owner->_parent->ID.id, _last_year.CheckYearAward(), _last_year.IsPerfectAwardClose(), tm1.tm_mon);
        return false;
    }

    if (type & REQ_PERFECT_AWARD)
    {
        PurchaseItem(SIGNIN_PERFECT_AWARD, 1);
        _last_year.SetPerfectAwardClose();
    }
	else if(type & REQ_YEAR_AWARD)
	{
		PurchaseItem(SIGNIN_YEAR_AWARD, 1);
		_last_year.SetYearAwardClose();
	}

    if (type & REQ_DAILY_AWARD)
    {
        PurchaseItem(award_id, award_num);
        ++_awarded_times;
    }

	ClientSync(SYNC4AWARD);

	return true;
}

bool player_dailysign::PreSyncTime(int clientlocaltime)
{
	struct tm tm1,tm2;
	time_t clt = clientlocaltime;
	time_t now = MK_LOCAL_TIME();
	localtime_r(&clt, &tm1);
	localtime_r(&now, &tm2);

	if(tm1.tm_year != tm2.tm_year || 
	   tm1.tm_mon != tm2.tm_mon || 
	   tm1.tm_mday != tm2.tm_mday)
	{
		CheckPoint();
		ClientSync(SYNC4ERR);	
		return true;	
	}

	return false;
}

void player_dailysign::ClientSync(char type)
{
	_owner->_runner->refresh_signin(type,_month_calendar,_curr_year,_last_year,_update_time,MK_LOCAL_TIME(), _awarded_times, _late_signin_times);
}

void player_dailysign::PurchaseItem(int type, int num, int expire)
{
	const item_data * pItem = (const item_data*)world_manager::GetDataMan().get_item_for_sell(type);
	if(!pItem) return;

	item_data * pItem2 = DupeItem(*pItem);
	
	int guid1 = 0;
	int guid2 = 0;   
	if(pItem2->guid.guid1 != 0) 
	{ 
		 get_item_guid(type, guid1,guid2); 
		 pItem2->guid.guid1 = guid1;
		 pItem2->guid.guid2 = guid2; 
	} 

//	pItem2->proc_type |= item::ITEM_PROC_TYPE_DAILY_SIGNIN; 
	item_list & inventory = _owner->GetInventory();

	int expire_date = expire ? g_timer.get_systime() + expire : 0; 

	int ocount = num;
	pItem2->count = ocount;
	pItem2->expire_date = expire_date;
	int rst = inventory.Push(*pItem2);
	ASSERT(rst >= 0 && pItem2->count == 0);
    inventory[rst].InitFromShop();

	_owner->_runner->obtain_item(pItem2->type,expire_date,ocount,inventory[rst].count, 0,rst);

	GLog::log(GLOG_INFO,"用户%d从签到系统获得了%d个item%d",_owner->_parent->ID.id, num, type);
	
	FreeItem(pItem2);
}

