#ifndef __ONLINEGAME_GS_PLAYER_DAILY_SIGN_H__
#define __ONLINEGAME_GS_PLAYER_DAILY_SIGN_H__
#include "common/packetwrapper.h"
#include "playertemplate.h"

#define REQ_YEAR_AWARD_MONTH	0
#define MAX_SIGN_MONTH 			12
#define REQUEST_YEAR_AWARD 		MAX_SIGN_MONTH
#define REQUEST_PERFECT_AWARD 	(REQUEST_YEAR_AWARD + 1)

#define LATE_DAY_SIGNIN_ITEM	39960	
#define SIGNIN_PERFECT_AWARD	39959
#define SIGNIN_YEAR_AWARD		39958
#define SIGNIN_MONTH_AWARD		39957

#define MAX_LATE_SIGNIN_TIMES   4

const unsigned int AWARD_TEMPLATE_ID[MAX_SIGN_MONTH] =
{
    1800, 1801, 1802, 1803, 1804, 1805, 1806, 1807, 1808, 1809, 1810, 1811,
};


class gplayer_imp;
class player_dailysign
{

	enum
	{
		OVER_3_TIMES_MISS,  
		UNDER_3_TIMES_MISS,
		NO_TIME_MISS,
		ALREADY_AWARDED,// 已颁发月度奖
	};

	enum
	{
		NO_AWARD,
		YEAR_AWARD,		// 年度奖
		PERFECT_AWARD,	// 全勤
    };

	enum
	{
		REQ_MONTH_AWARD = 0x1,
		REQ_YEAR_AWARD  = 0x2,
		REQ_PERFECT_AWARD = 0x4,
        REQ_DAILY_AWARD = 0x8,
    };


	struct month_sign_state
	{
		month_sign_state() : _data(0) {}
		int _data; // 每月占2bit共24bit ，最高2位记录年度和全勤奖是否结算过
		operator int() const { return _data; }
		month_sign_state& operator = (month_sign_state& ot) { _data = ot._data; return *this;}		
		month_sign_state& operator = (int d) { _data = d; return *this;}		
		void clear() { _data = 0;}

		int GetState(int m)
		{ 
			if(m >= MAX_SIGN_MONTH) 
				return 0;
			m <<= 1; // each 2bit

			return (_data >> m) & 0x3;
		}	
		void SetState(int m, int state)
		{
			if(m >= MAX_SIGN_MONTH)
				return;
			m <<= 1; // each 2bit

			state &= 0x3;
			state <<= m;

			_data &= ~(0x3<<m);
			_data |= state;
		}
		
		int CheckYearAward()
		{
			int res = PERFECT_AWARD;
			for(int n = 0; n < MAX_SIGN_MONTH; ++n)
			{
				switch(GetState(n))
				{
					case OVER_3_TIMES_MISS:
						return NO_AWARD;
					case UNDER_3_TIMES_MISS:
						res = YEAR_AWARD;
						break;
				}
			}

			return res;
		};

		bool IsYearAwardClose()
		{
			return _data & 0x40000000;
		}
		void SetYearAwardClose()
		{
			_data |= 0x40000000;
		}
		bool IsPerfectAwardClose()
		{
			return _data & 0x80000000;
		}
		void SetPerfectAwardClose()
		{
			_data |= 0x80000000;
		}
	};


public:
	enum
	{
		SYNC4INIT,
		SYNC4ERR,
		SYNC4DAILY,
		SYNC4LATE,
		SYNC4AWARD
	};
	
	player_dailysign(gplayer_imp* p) : _owner(p), _update_time(0), _month_calendar(0), _awarded_times(0), _late_signin_times(0) {}

	bool Save(archive & ar)
	{
		ar << _update_time;
		ar << _month_calendar;
		ar << _curr_year._data;
		ar << _last_year._data;
        ar << _awarded_times;
        ar << _late_signin_times;
		return true;
	}
	bool Load(archive & ar)
	{
		ar >> _update_time;
		ar >> _month_calendar;
		ar >> _curr_year._data;
		ar >> _last_year._data;
        ar >> _awarded_times;
        ar >> _late_signin_times;
		return true;
	}
	void Swap(player_dailysign& rhs)
	{
		abase::swap(_update_time, rhs._update_time);
		abase::swap(_month_calendar, rhs._month_calendar);
		abase::swap(_curr_year._data, rhs._curr_year._data);
		abase::swap(_last_year._data, rhs._last_year._data);
        abase::swap(_awarded_times, rhs._awarded_times);
        abase::swap(_late_signin_times, rhs._late_signin_times);
	}

public:
	void	SaveToDB(int& uptime, int& monthcal, int& curryear, int& lastyear, char& awardedtimes, char& latesignintimes);
	void 	InitFromDB(int uptime, int monthcal, int curryear, int lastyear, char awardedtimes, char latesignintimes);

	void 	CheckPoint();
	bool 	DaySignIn(int dsttime);
	bool 	MonthSignIn(int dsttime);
	bool 	YearSignIn(int dsttime);
	bool	RequestAwards(int type,int mon);

	bool	PreSyncTime(int clientlocaltime);
	void	ClientSync(char type);
protected:
	void 	AggregateMonthSign();
	void 	AggregateYearSign();
	void 	PurchaseItem(int type, int num, int expire = 0);

private:
	gplayer_imp* const	_owner;
protected:
	int 	_update_time;
	int 	_month_calendar;
	month_sign_state  _curr_year;
	month_sign_state  _last_year;
    char    _awarded_times;
    char    _late_signin_times;
public:
	static tm dbgtime;
	static time_t MK_LOCAL_TIME()  { return (player_template::GetDebugMode() && player_dailysign::dbgtime.tm_year) ? mktime(&player_dailysign::dbgtime) : time(NULL); }
	
};


#endif
