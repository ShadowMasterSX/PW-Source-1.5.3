#ifndef _REWARDMANAGER_H_o
#define _REWARDMANAGER_H_o

#include <map>
#include <set>
#include <vector>
#include "itimer.h"
#include "rewarditem"
#include "grewardstore"

namespace GNET
{
class BufferedFlag
{
	enum {
		DIRTY		=	0x0001,
		WRITEBACK	=	0x0002,
	};
	int flag;
public:
	BufferedFlag():flag(0){}
	void SetDirty(bool _dirty)
	{
		if (_dirty)
			flag |= DIRTY;
		else
			flag &= ~DIRTY;
	}
	void SetWriteback(bool _wb)
	{
		if (_wb)
			flag |= WRITEBACK;
		else
			flag &= ~WRITEBACK;
	}
	bool IsDirty() { return (flag & DIRTY); }
	bool IsWriteback() { return (flag & WRITEBACK); }
};
	
class Reward
{
	enum{ REWARD_ITEM_MAX = 200, };
	int roleid;
	int userid;
	int consume_points;
	BufferedFlag points_flag;       //DBPutConsumePoints
	int bonus_reward;
	RewardItemVector rewardlist;
	BufferedFlag reward_flag;  	//DBRewardMature协议是否正在进行数据库操作
	bool logout;
	friend class RewardManager;
	GRewardItemVector Reward::DumpRewardList();
public:
	Reward():roleid(0), userid(0), consume_points(0), bonus_reward(0), logout(false)
	{
	}
	void OnUseCash(int cashused);
	void OnTaskReward(int bonus_add);
	void SetLogout(bool _logout) { logout = _logout; } 
	void OnLoad(int _roleid, int _userid, const GRewardStore & reward, bool need_clear);
	int GetList(int & points, int & total, int start_index, RewardItemVector & list);
	bool IsLogout() { return logout; } 
	bool SyncPointsDB();
	void CheckRewardList(time_t now);
//	void OnRewardListUpdate(int retcode, int now, int bonus_add);
	int ExchangePoints(int points_dec, int bonus_add, int interval, int times, int endtime);
	//void OnLogin();
	void SetRoleId(int rid){ roleid = rid; }
};

class RewardManager : public IntervalTimer::Observer
{
	enum {
		ST_OPEN 	=	0x0001,
		ST_ACTIVE	=	0x0002,
	};
	enum { CHECKNUM_ONUPDATE = 30 };
	//返还配置信息 从gamesys.conf中读取
	int begin_time;
	int end_time;
	typedef std::pair<int/*points*/, int/*bonus*/> RewardType;
	typedef std::vector<RewardType> RewardTypeList;
	RewardTypeList typelist;
	int reward_interval;
	int reward_times;

	int status;
	typedef std::map<int, Reward> RewardMap;
	RewardMap rewards;		//key 由roleid改为userid，liuguichen
	int wb_cursor;
	int reward_cursor;
	RewardManager():status(0), wb_cursor(0), reward_cursor(0){ }
	
public:
	static RewardManager *GetInstance() { static RewardManager instance; return &instance; }

	bool Initialize();
	std::string Identification() const { return "ConsumeReward"; }
	bool Update();
	void OnLogin(int roleid, int userid);
	void OnLogout(int roleid, int userid);
	void OnLoadReward(int roleid, int userid, const GRewardStore &reward);
	int GetRewardList(int userid, int & points, int & total, int start_index, RewardItemVector & list);
	void OnDBPointsUpdate(int retcode, int userid);
	void OnDBBonusUpdate(int retcode, int userid);
	void OnDBPointsExchange(int retcode, int userid);
	void OnRewardListUpdate(int retcode, int userid);
/*
	Referrer *GetReferrer(int userid)
	{
		ReferrerMap::iterator it = referrers.find(userid);
		if (it != referrers.end())
			return &it->second;
		else 
			return NULL;
	}
	Referral *GetReferral(int userid)
	{
		ReferralMap::iterator it = referrals.find(userid);
		if (it != referrals.end())
			return &it->second;
		else
			return NULL;
	}
*/
	int ExchangePoints(int userid, int rewardtype, int &bonus_add);
	void OnUseCash(int roleid, int cashused);
	void OnTaskReward(int roleid, int bonus_add);
	int SetRewardTime(int b_year, int b_mon, int b_day, int b_hour, int e_year, int e_mon, int e_day, int e_hour, int interval, int times);
	int CheckRewardList(int userid, time_t future);
private:
	bool LoadConfig(time_t now);
};

};
#endif
