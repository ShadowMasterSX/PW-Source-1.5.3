#ifndef __ONLINEGAME_GS_PLAYER_CLOCK_H__
#define __ONLINEGAME_GS_PLAYER_CLOCK_H__
#include "common/packetwrapper.h"
#include "vector.h"

class player_clock;
class gplayer_imp;
/**
 * clock_listener 为所有需要被制定时间回调类的基类
   
   派生类负责将自己设定闹钟类型
   GetPlayerClock().AddNotice(this,类型,条件) 
   可多次设置到不同刻度
   条件默认-1为无
   OnClock 到达指定时刻后 最大误差30s  会发生OnClock的调用，如有多个顺序按GPC_TYPE排序，同类型的按AddNotice先后排序
   OnPassClock 如果时钟在下线时发生指定刻度的跨越则在上线的第一次heartbeat时,会发生OnPassClock 调用，顺序同上
   类型：
   GPC_PER_HOUR_GLOBAL 	每小时整点 	全服	条件为制定的某一小时(0-23) 
   GPC_PER_DAY_GLOBAL  	每天0点  	全服	条件为周几的某天(0-6)
   GPC_PER_WEEK_GLOBAL 	每周1    	全服	条件为经过了本月的第几个周1(0-5)
   GPC_PER_MONTH_GLOBAL 每月第1天	全服	条件为第几个月份(0-11)
   GPC_PER_HOUR_LOCAL 	每小时整点 	本服	条件为制定的某一小时(0-23) 
   GPC_PER_DAY_LOCAL  	每天0点  	本服	条件为周几的某天(0-6)
   GPC_PER_WEEK_LOCAL 	每周1    	本服	条件为经过了本月的第几个周1(0-5)
   GPC_PER_MONTH_LOCAL 	每月第1天	本服	条件为第几个月份(0-11)
 */
struct clock_listener
{
	// type 对应 GPC_TYPE
	virtual void OnClock(gplayer_imp* player,int type) = 0;
	virtual void OnPassClock(gplayer_imp* player,int type,int lastupdate,int now) = 0;
};

class player_clock
{
public:
	enum GPC_TYPE
	{
		GPC_PER_HOUR_GLOBAL,	// 闹钟时间为每小时  	整点	全服
		GPC_PER_DAY_GLOBAL,		// 闹钟时间为每天  		0点		全服
		GPC_PER_WEEK_GLOBAL,	// 闹钟时间为每周1 		0点		全服
		GPC_PER_MONTH_GLOBAL,	// 闹钟时间为每月第1天 	0点		全服
		GPC_PER_HOUR_LOCAL,		// 闹钟时间为每小时  	整点	本服	
		GPC_PER_DAY_LOCAL,		// 闹钟时间为每天  		0点		本服
		GPC_PER_WEEK_LOCAL,		// 闹钟时间为每周1 		0点		本服
		GPC_PER_MONTH_LOCAL,	// 闹钟时间为每月第1天 	0点		本服

		GPC_TYPE_MAX	
	};

	enum
	{
		GPC_UNINIT,
		GPC_INIT,
		GPC_NORMAL,
	};

	static tm dbgtime;
	static const int GPC_HB_IDLE = 30;  		// 30s
	static const int GPC_DAY_SEC = 86400;
	static const int GPC_INTERVAL[GPC_TYPE_MAX];

public:
	player_clock() : _idle_time(0),_state(GPC_UNINIT)
	{	
		_update_time.insert(_update_time.begin(),GPC_TYPE_MAX,player_clock::clock_update());
		_notice_list.insert(_notice_list.begin(),GPC_TYPE_MAX,player_clock::NOTICE_NODE_LIST());
	}		
	~player_clock() { _update_time.clear(); _notice_list.clear(); }

	bool Save(archive & ar)
	{
		ar << _idle_time;
		ar << _state;
		ar << _update_time.size();
		if( !_update_time.empty() )
		{			
			for(size_t n = 0; n < _update_time.size(); ++n)	
			{
				ar << _update_time[n].lasttime;
				ar << _update_time[n].nexttime;
			}
		}
		return true;	
	}
	bool Load(archive & ar)
	{
		try
		{
			ar >> _idle_time;
			ar >> _state;
			int upsize;
			ar >> upsize;
			if(upsize && upsize <= GPC_TYPE_MAX)
			{
				for(int n = 0; n < upsize; ++n)	
				{
					ar >> _update_time[n].lasttime; 
					ar >> _update_time[n].nexttime;
				}
			}
			else
			{
				_update_time.clear();
				return false;
			}
		}
		catch(...)
		{
			_idle_time = 0;
			_update_time.clear();
			_state = GPC_UNINIT;
			return false;
		}
		return true;
	}
	void Swap(player_clock& rhs)
	{
		abase::swap(_idle_time, rhs._idle_time);
		abase::swap(_state, rhs._state);
		_update_time.swap(rhs._update_time);	
	}

public:
	void InitCheck(gplayer_imp* player,int now,bool incentral);
	void CheckTime(gplayer_imp* player,int now,bool incentral);
	void OnHeartbeat(gplayer_imp* player,int now,bool incentral);

	void SaveToDB(archive & ar);
	void InitFromDB(archive & ar,int roleid);

	void AddNotice(clock_listener* listener,int type,int cond = -1) // 不判断重复加入
	{
		if(type < GPC_TYPE_MAX && listener) 	_notice_list[type].push_back(clock_node(listener,cond));
	}

	// for debug
	void Reset(gplayer_imp* player,int type) ;
public:	
	static int GetWeekNum(int nowtime);
	static int GetMonthDiff(int oldtime, int nowtime);
	static int GetMonthDayDiff(int year ,int mon);
	static int GetNextUpdatetime(int type,int nowtime);
	static int GetPassPeriod(int type, int oldtime, int nowtime);
	static bool CheckPrviateCond(int type, int cond, int stubtime, int nowtime, int passperiod, bool nopass);

private:
	struct clock_node
	{
		clock_node(clock_listener* l, int cond) : listener(l), condition(cond) {}
		clock_listener* listener;
		int	condition;
		void Init(gplayer_imp* player,int type, int old, int stub, int now, int passperiod)
		{
			if(listener && (-1 == condition || CheckPrviateCond(type,condition,stub,now,passperiod,false))) 
				listener->OnPassClock(player, type, old, now);
		}
		void Run(gplayer_imp* player,int type,int now)
		{
			if(listener && (-1 == condition || CheckPrviateCond(type,condition,now,now,0,true))) 
				listener->OnClock(player, type);
		}
	};
	
	typedef abase::vector<clock_node , abase::fast_alloc<> > NOTICE_NODE_LIST;
	typedef abase::vector<NOTICE_NODE_LIST, abase::fast_alloc<> > NOTICE_LIST;
	NOTICE_LIST _notice_list;  // 不需要存盘

	struct clock_update
	{
		clock_update() : lasttime(0), nexttime(0) {}
		int lasttime;
		int nexttime;
	};

	typedef abase::vector<clock_update, abase::fast_alloc<> > UPDATE_LIST;
	UPDATE_LIST	_update_time;

	int _idle_time;
	int _state;
};

#endif
