#include "rewardmanager.h"
#include "mapuser.h"
#include "timer.h"
#include <time.h>
#include "gamedbclient.hpp"
#include "gproviderserver.hpp"
#include "gdeliveryserver.hpp"
#include <limits.h>
#include "localmacro.h"
#include "conf.h"
#include "dbrewardmature.hrp"
#include "dbgetreward.hrp"
#include "dbputconsumepoints.hrp"
#include "dbputrewardbonus.hrp"
#include "dbexchangeconsumepoints.hrp"
#include "sendrewardaddbonus.hpp"
#include "rewardmaturenotice.hpp"

namespace GNET
{
void Reward::OnLoad(int _roleid, int _userid, const GRewardStore & reward, bool need_clear)
{
	LOG_TRACE("reward load roleid %d", _roleid);
	roleid = _roleid;
	userid = _userid;
	if (reward.consume_points > 0)
	{
		if (need_clear)
		{
			consume_points = 0;
			points_flag.SetDirty(true);
		}
		else
			consume_points = reward.consume_points;
	}
	bonus_reward = reward.bonus_reward;
	rewardlist.clear();
	GRewardItemVector::const_iterator it, ite;
	for (it=reward.rewardlist.begin(),ite=reward.rewardlist.end(); it!=ite; ++it)
		rewardlist.push_back(RewardItem(it->reward_time, it->reward_bonus));
}

GRewardItemVector Reward::DumpRewardList()
{
	GRewardItemVector tmp_list;
	RewardItemVector::const_iterator it, ite;
	for (it=rewardlist.begin(),ite=rewardlist.end(); it!=ite; ++it)
		tmp_list.push_back(GRewardItem(it->reward_time, it->reward_bonus));
	return tmp_list;
}

int Reward::GetList(int & points, int & total, int start_index, RewardItemVector & list)
{
	points = consume_points;
	total = (int)rewardlist.size();
	if (start_index < 0 || start_index >= total)
	{
		list.clear();
		return ERR_SUCCESS;
	}
//	int list_num = std::min(REWARD_ITEM_PER_PAGE, total-start_index);
	RewardItemVector::const_iterator it, ite=rewardlist.end();
	it = rewardlist.begin();
	for (int i=0; i<start_index&&it!=ite; i++,++it)
		;
	for (int i=0; i<REWARD_NUM_PER_PAGE&&it!=ite; i++,++it)
		list.push_back(*it);
	return ERR_SUCCESS;
}

void Reward::OnTaskReward(int bonus_add)
{
	LOG_TRACE("roleid %d task add bonus %d", roleid, bonus_add);
	bonus_reward += bonus_add;
	reward_flag.SetDirty(true);
	//此处不能再延迟写了 否则玩家小退或大退之后gs从db读取的鸿利值信息可能是旧的20091202ly
	if (!reward_flag.IsWriteback())
	{
		DBPutRewardBonusArg arg(roleid, userid, bonus_reward);
		DBPutRewardBonus *rpc = (DBPutRewardBonus *)Rpc::Call(RPC_DBPUTREWARDBONUS, arg);
		reward_flag.SetDirty(false);
		reward_flag.SetWriteback(true);
		if (GameDBClient::GetInstance()->SendProtocol(rpc))
			LOG_TRACE("DBPutRewardBonus(task) Send to DB roleid %d bonus_reward %d", roleid, bonus_reward);
		else
			Log::log(LOG_ERR, "DBPutRewardBonus(task) Send to DB error roleid %d", roleid);
	}
	PlayerInfo *pinfo = UserContainer::GetInstance().FindRoleOnline(roleid);
	if (pinfo != NULL)
	{
//		GDeliveryServer::GetInstance()->Send(pinfo->linksid, RewardMatureNotice(roleid, bonus_add, pinfo->localsid));
		GProviderServer::GetInstance()->DispatchProtocol(pinfo->gameid, SendRewardAddBonus(roleid, bonus_add));
	}
}

void Reward::CheckRewardList(time_t now)
{
	if (!IsLogout() && !reward_flag.IsWriteback())
	{
		bool mature = false;
		RewardItemVector::iterator it = rewardlist.begin(),ite=rewardlist.end();
		if (it != ite && now > it->reward_time)
			mature = true;
/*
		//		int bonus_add = 0;
		for (it=rewardlist.begin(); it!=end;)
		{
			if (now >= it->reward_time)
			{
				mature = true;
	//			bonus_add += it->reward_bonus;
	//			it = rewardlist.erase(it);
			}
			else
				break;
		}
*/
		if (mature)
		{
			int bonus_add = 0;
			for (it=rewardlist.begin(); it!=rewardlist.end();)
			{
				if (now >= it->reward_time)
				{
					bonus_add += it->reward_bonus;
					it = rewardlist.erase(it);
				}
				else
					break;
			}
			bonus_add = std::min((INT_MAX-bonus_reward), bonus_add);
			bonus_reward += bonus_add;
			PlayerInfo *pinfo = UserContainer::GetInstance().FindRoleOnline(roleid);
			if (pinfo != NULL)
			{
				GDeliveryServer::GetInstance()->Send(pinfo->linksid, RewardMatureNotice(roleid, bonus_add, pinfo->localsid));
				GProviderServer::GetInstance()->DispatchProtocol(pinfo->gameid, SendRewardAddBonus(roleid, bonus_add));
			}

			DBRewardMatureArg arg(roleid, userid, bonus_reward, DumpRewardList());
			DBRewardMature *rpc = (DBRewardMature *)Rpc::Call(RPC_DBREWARDMATURE, arg);
			reward_flag.SetDirty(false);
			reward_flag.SetWriteback(true);
			if (GameDBClient::GetInstance()->SendProtocol(rpc))
				LOG_TRACE("DBRewardMature roleid %d", roleid);
			else
				Log::log(LOG_ERR, "DBRewardMature send to DB error roleid %d", roleid);
		}
	}	
}
#if 0
void Reward::OnRewardListUpdate(int retcode, int bonus_add)
{
	reward_flag.SetWriteback(false);
	if (retcode == ERR_SUCCESS)
	{
/*
		int bonus_add2 = 0;
		RewardItemVector::iterator it;
		for (it=rewardlist.begin(); it!=rewardlist.end();)
		{
			if (now >= it->reward_time)
			{
				bonus_add2 += it->reward_bonus;
				it = rewardlist.erase(it);
			}
			else
				break;
		}
		bonus_add2 = std::min((INT_MAX-bonus_reward), bonus_add2);
		bonus_reward += bonus_add2;
		if (bonus_add != bonus_add2)
		{
			Log::log(LOG_ERR, "OnRewardListUpdate roleid %d bonus_add %d in memory, bonus_add %d in DB",
							roleid, bonus_add2, bonus_add);
		}
*/
/*
		PlayerInfo *pinfo = UserContainer::GetInstance().FindRoleOnline(roleid);
		if (pinfo != NULL)
		{
			GProviderServer::GetInstance()->DispatchProtocol(pinfo->gameid, SendRewardAddBonus(roleid, bonus_add));
		}
*/
	}
}
#endif

bool RewardManager::Initialize()
{
	time_t now = Timer::GetTime();
	if (!LoadConfig(now))
	{
		LOG_TRACE("RewardManager load conf failed");
		return false;
	}
	if (status & ST_OPEN)
	{
		if (now >= begin_time && now <= end_time)
			status |= ST_ACTIVE;
	}
	//for test
	//IntervalTimer::Attach(this,10000000/IntervalTimer::Resolution());
	IntervalTimer::Attach(this,50000000/IntervalTimer::Resolution());
	return true;
}

bool RewardManager::LoadConfig(time_t now)
{
	Conf * conf = Conf::GetInstance();
	char open = 0;
	open = atoi(conf->find(Identification(), "open").c_str());
	if (!open)
	{
		LOG_TRACE("RewardManager:: consume reward does not open!");
		return true;
	}
	status |= ST_OPEN;
	int _year, _mon, _day, _hour;
	int ret = 0;
	ret = sscanf( conf->find(Identification(), "begin_time").c_str(), "%d-%d-%d-%d", &_year, &_mon, &_day, &_hour);
	if (ret != 4)
	{
		LOG_TRACE("RewardManager:: invalid begin_time format");
		return false;
	}
	LOG_TRACE("RewardManager:: begin timme %d %d %d %d", _year, _mon, _day, _hour);
	struct tm dt;
	localtime_r(&now, &dt);
	dt.tm_year = _year-1900;
	dt.tm_mon = _mon-1;
	dt.tm_mday = _day;
	dt.tm_hour = _hour;
	dt.tm_min = 0;
	dt.tm_sec = 0;
	begin_time = mktime(&dt);
	if (begin_time == -1)
	{
		LOG_TRACE("RewardManager:: invalid begin_time");
		return false;
	}
	ret = sscanf( conf->find(Identification(), "end_time").c_str(), "%d-%d-%d-%d", &_year, &_mon, &_day, &_hour);
	if (ret != 4)
	{
		LOG_TRACE("RewardManager:: invalid end_time format");
		return false;
	}
	LOG_TRACE("RewardManager:: end timme %d %d %d %d", _year, _mon, _day, _hour);
	dt.tm_year = _year-1900;
	dt.tm_mon = _mon-1;
	dt.tm_mday = _day;
	dt.tm_hour = _hour;
	end_time = mktime(&dt);
	if (end_time == -1)
	{
		LOG_TRACE("RewardManager:: invalid end_time");
		return false;
	}
	if (begin_time >= end_time)
	{
		LOG_TRACE("RewardManager:: begin_time[%d] is later than end_time[%d]", begin_time, end_time);
		return false;
	}

	std::string type_str = conf->find(Identification(), "reward_type");
	if( type_str.length() > 1023 )
	{               
		LOG_TRACE("RewardManager:: reward_type is too long");
		return false;
	}
	char type_buffer[1024];
	strncpy( type_buffer, type_str.c_str(), std::min(sizeof(type_buffer)-1,type_str.length()) );
	type_buffer[sizeof(type_buffer)-1] = 0;
	char * cur = type_buffer;
	char * token = strchr( cur, '(' );
	while( NULL != token )
	{       
		cur = token+1;
		token = strchr( cur, ',' );
		if( NULL == token )     break;
		*token = 0;
		int points = atol(cur);
		if (points <= 0)
		{
			LOG_TRACE("RewardManager:: reward_type points %d not positive", points);
			return false;
		}
		cur = token+1;
		token = strchr( cur, ')' );
		if( NULL == token )     break;
		*token = 0;
		int bonus = atol(cur);

		typelist.push_back(std::make_pair(points, bonus));
		cur = token+1;
		token = strchr( cur, '(' );
	}       
	RewardTypeList::const_iterator it, ite;
	for (it=typelist.begin(),ite=typelist.end(); it!=ite; ++it)
		LOG_TRACE("points[%d]->bonus[%d]", it->first, it->second);

	std::string time_str = conf->find(Identification(), "reward_time");
	if( time_str.length() > 1023 )
	{               
		LOG_TRACE("RewardManager:: reward_time is too long");
		return false;
	}
	char time_buffer[1024];
	strncpy( time_buffer, time_str.c_str(), std::min(sizeof(time_buffer)-1,time_str.length()) );
	time_buffer[sizeof(time_buffer)-1] = 0;
	cur = time_buffer;
	token = strchr( cur, '(' );
	while( NULL != token )
	{       
		cur = token+1;
		token = strchr( cur, ',' );
		if( NULL == token )     break;
		*token = 0;
		reward_interval = atol(cur);
		reward_interval *= 3600*24;

		cur = token+1;
		token = strchr( cur, ')' );
		if( NULL == token )     break;
		*token = 0;
		reward_times = atol(cur);

		cur = token+1;
		token = strchr( cur, '(' );
	}       
	if (reward_interval <= 0 || reward_times <= 0)
	{
		if (!(reward_interval == 0 && reward_times == 1))
		{
			LOG_TRACE("RewardManager:: invalid reward interval %d times %d", reward_interval, reward_times);
			return false;
		}
	}
	LOG_TRACE("RewardManager:: reward interval %d times %d", reward_interval, reward_times);
	return true;
}

int RewardManager::SetRewardTime(int b_mon, int b_day, int b_hour, int b_min, int e_mon, int e_day, int e_hour, int e_min, int interval, int times)
{
	int begin_t = 0, end_t = 0;
	time_t now = Timer::GetTime();
	LOG_TRACE("RewardManager:: begin timme %d %d %d %d", b_mon, b_day, b_hour, b_min);
	struct tm dt;
	localtime_r(&now, &dt);
	dt.tm_mon = b_mon-1;
	dt.tm_mday = b_day;
	dt.tm_hour = b_hour;
	dt.tm_min = b_min;
	dt.tm_sec = 0;
	begin_t = mktime(&dt);
	if (begin_t == -1)
	{
		LOG_TRACE("RewardManager:: invalid begin_time");
		return -1;
	}
	LOG_TRACE("RewardManager:: end timme %d %d %d %d",  e_mon, e_day, e_hour, e_min);
	dt.tm_mon = e_mon-1;
	dt.tm_mday = e_day;
	dt.tm_hour = e_hour;
	dt.tm_min = e_min;
	dt.tm_sec = 0;
	end_t = mktime(&dt);
	if (end_t == -1)
	{
		LOG_TRACE("RewardManager:: invalid end_time");
		return -2;
	}
	if (begin_t >= end_t)
	{
		LOG_TRACE("RewardManager:: begin_time[%d] is later than end_time[%d]", begin_t, end_t);
		return -3;
	}
	if (times > 100)
		return -4;
	if (interval <= 0 || times <= 0)
	{
		if (!(interval == 0 && times == 1))
		{
			LOG_TRACE("RewardManager:: invalid reward interval %d times %d", interval, times);
			return -5;
		}
	}
	LOG_TRACE("RewardManager:: reward interval %d times %d", interval, times);
	begin_time = begin_t;
	end_time = end_t;
	reward_interval = interval;
	reward_times = times;
	return 0;
}
//enterworld时候调用
void RewardManager::OnLogin(int roleid, int userid)
{
//	任务奖励鸿利值时也通过增加bonus_reward来实现
//	if (!(status & ST_OPEN)) return;
	RewardMap::iterator it = rewards.find(userid);
	if (it == rewards.end())
	{
		DBGetReward *rpc = (DBGetReward *)Rpc::Call(RPC_DBGETREWARD, DBGetRewardArg(roleid, userid));
		GameDBClient::GetInstance()->SendProtocol(rpc);
	}
	else
	{
		it->second.SetRoleId(roleid);	//要重新设置roleid，因为可能更换了角色进行登陆
		it->second.SetLogout(false);
/*
		it->second.OnLogin();
		if (userinfo->real_referrer > 0)
		{
			Referrer &referrer = referrers[userinfo->real_referrer];
			referrer.AddReferral(userinfo->userid);
			if (!referrer.IsLoaded())
			{
				DBRefGetReferrer *rpc = (DBRefGetReferrer *)Rpc::Call(RPC_DBREFGETREFERRER, Integer(userinfo->real_referrer));
				GameDBClient::GetInstance()->SendProtocol(rpc);
			}
		}
*/
	}
}

void RewardManager::OnLogout(int roleid, int userid)
{
//	if (!(status & ST_OPEN)) return;
	RewardMap::iterator it = rewards.find(userid);
	if (it != rewards.end())
	{
		if (it->second.points_flag.IsDirty() || it->second.points_flag.IsWriteback() || it->second.reward_flag.IsDirty() || it->second.reward_flag.IsWriteback())
			it->second.SetLogout(true);
		else
			rewards.erase(it);
	}
}

void RewardManager::OnLoadReward(int roleid, int userid, const GRewardStore &reward)
{
	//DBGetReward返回时间过晚 玩家 已经下线
	PlayerInfo *pinfo = UserContainer::GetInstance().FindRoleOnline(roleid);
	if (pinfo==NULL || pinfo->user==NULL) return;
	bool need_clear = false;
	if (status & ST_OPEN)
	{
		if (!(status & ST_ACTIVE))
			need_clear = true;
		else
		{
			//虽然现在是活动期，有可能是上次活动期遗留的积分，玩家一直没上线，因此也就没清除
			if (pinfo->user->lastlogin < begin_time)
				need_clear = true;
		}
	}
	rewards[userid].OnLoad(roleid, userid, reward, need_clear);
}

bool RewardManager::Update()
{
	time_t now = Timer::GetTime();
	if (status & ST_OPEN)
	{
		if (now >= begin_time && now <= end_time)
		{
			if (!(status & ST_ACTIVE))
			{
				//广播活动开始
				LOG_TRACE("consume reward restart!!!!");
				status |= ST_ACTIVE;
			}
		}
		else
		{
			if (status & ST_ACTIVE)
			{
//				清玩家points
				RewardMap::iterator it,ite;
				for (it=rewards.begin(),ite=rewards.end(); it!=ite; ++it)
				{
					if (it->second.consume_points != 0)
					{
						it->second.consume_points = 0;
						it->second.points_flag.SetDirty(true);
					}
				}
				LOG_TRACE("consume reward stop!!!!");
				status &= ~ST_ACTIVE;
			}
		}
	}
	if (!rewards.empty())
	{
		RewardMap::iterator it = rewards.lower_bound(wb_cursor);
		RewardMap::iterator ie = rewards.end();
		int i = 0; 
		while (it!=ie && i < CHECKNUM_ONUPDATE)
		{
			if (it->second.points_flag.IsDirty() || it->second.points_flag.IsWriteback())
			{
				if (!it->second.points_flag.IsWriteback())
				{
					DBPutConsumePointsArg arg(it->second.roleid, it->second.userid, it->second.consume_points);
					DBPutConsumePoints *rpc = (DBPutConsumePoints *)Rpc::Call(RPC_DBPUTCONSUMEPOINTS, arg);
					it->second.points_flag.SetDirty(false);
					it->second.points_flag.SetWriteback(true);
					if (GameDBClient::GetInstance()->SendProtocol(rpc))
						LOG_TRACE("DBPutConsumePoints Send to DB roleid %d consume_points %d", it->second.roleid, it->second.consume_points);
					else
						Log::log(LOG_ERR, "DBPutConsumePoints Send to DB error roleid %d", it->second.roleid);
				}
				else
					LOG_TRACE("DBPutConsumePoints roleid %d still in Writeback state", it->second.roleid);
				++it;
			}
			else if (it->second.reward_flag.IsDirty() || it->second.reward_flag.IsWriteback())
			{
				if (!it->second.reward_flag.IsWriteback())
				{
					DBPutRewardBonusArg arg(it->second.roleid, it->second.userid, it->second.bonus_reward);
					DBPutRewardBonus *rpc = (DBPutRewardBonus *)Rpc::Call(RPC_DBPUTREWARDBONUS, arg);
					it->second.reward_flag.SetDirty(false);
					it->second.reward_flag.SetWriteback(true);
					if (GameDBClient::GetInstance()->SendProtocol(rpc))
						LOG_TRACE("DBPutRewardBonus Send to DB roleid %d bonus_reward %d", it->second.roleid, it->second.bonus_reward);
					else
						Log::log(LOG_ERR, "DBPutRewardBonus Send to DB error roleid %d", it->second.roleid);
				}
				else
					LOG_TRACE("DBPutRewardBonus roleid %d still in Writeback state", it->second.roleid);
				++it;
			}
			else if (it->second.IsLogout())
			{
				rewards.erase(it++);
			}
			else 
				++it;
			++i;
		}
		if (it != ie)
			wb_cursor = it->first;
		else
			wb_cursor = 0;
	}
	if (!rewards.empty())
	{
		RewardMap::iterator it = rewards.lower_bound(reward_cursor);
		RewardMap::iterator ie = rewards.end();
		int i = 0;
		while (it!=ie && i < CHECKNUM_ONUPDATE)
		{
			it->second.CheckRewardList(now);
			++it;
			++i;
		}
		if (it != ie)
			reward_cursor = it->first;
		else
			reward_cursor = 0;
	}
	return true;
}

void Reward::OnUseCash(int cashused)
{
	consume_points += cashused;
	points_flag.SetDirty(true);
}

void RewardManager::OnUseCash(int roleid, int cashused)
{
	if (!(status & ST_ACTIVE))
		return;
	PlayerInfo *pinfo = UserContainer::GetInstance().FindRoleOnline(roleid);
	if (pinfo == NULL)
		return;
	RewardMap::iterator it = rewards.find(pinfo->userid);
	if (it != rewards.end()) it->second.OnUseCash(cashused);
}

void RewardManager::OnTaskReward(int roleid, int bonus_add)
{
	PlayerInfo *pinfo = UserContainer::GetInstance().FindRoleOnline(roleid);
	if (pinfo == NULL)
		return;
	RewardMap::iterator it = rewards.find(pinfo->userid);
	if (it != rewards.end()) it->second.OnTaskReward(bonus_add);
}

void RewardManager::OnDBPointsUpdate(int retcode, int userid)
{
	RewardMap::iterator it = rewards.find(userid);
	if (it != rewards.end())
	{
		it->second.points_flag.SetWriteback(false);
		if (retcode != ERR_SUCCESS)
			it->second.points_flag.SetDirty(true);
	}
}

void RewardManager::OnDBBonusUpdate(int retcode, int userid)
{
	RewardMap::iterator it = rewards.find(userid);
	if (it != rewards.end())
	{
		it->second.reward_flag.SetWriteback(false);
		if (retcode != ERR_SUCCESS)
			it->second.reward_flag.SetDirty(true);
	}
}

void RewardManager::OnDBPointsExchange(int retcode, int userid)
{
	RewardMap::iterator it = rewards.find(userid);
	if (it != rewards.end())
	{
		it->second.points_flag.SetWriteback(false);
		it->second.reward_flag.SetWriteback(false);
		if (retcode != ERR_SUCCESS)
			it->second.points_flag.SetDirty(true);
//		else
//			it->second.OnLoad(roleid, userid, reward, false);
	}
}

void RewardManager::OnRewardListUpdate(int retcode, int userid)
{
	RewardMap::iterator it = rewards.find(userid);
	if (it != rewards.end())
	{
//		it->second.OnRewardListUpdate(retcode, bonus_add);
		it->second.reward_flag.SetWriteback(false);
		if (retcode != ERR_SUCCESS)
			it->second.reward_flag.SetDirty(true);
	}
}

int Reward::ExchangePoints(int points_dec, int bonus_add, int interval, int times, int end_time)
{
	PlayerInfo *pinfo = UserContainer::GetInstance().FindRoleOnline(roleid);
	if (pinfo == NULL)
		return ERR_REWARD_PLAYER_NOT_ONLINE;
	if (points_flag.IsWriteback() || reward_flag.IsWriteback())
		return ERR_REWARD_DB_BUSY;
	if (consume_points < points_dec)
		return ERR_REWARD_POINTS_INADEQUATE;
	if (rewardlist.size() >= REWARD_ITEM_MAX)	//add by liuguichen 20110811
		return ERR_REWARD_EXCHANGE_REACH_UPLIMIT;
/////////
	consume_points -= points_dec;
	RewardItemVector itemadd;
	if (interval == 0)
	{
		itemadd.push_back(RewardItem(Timer::GetTime(), bonus_add));
	}
	else
	{
		for (int i = 0; i < times; i++)
		{
			if (i==times-1)
				itemadd.push_back(RewardItem(end_time+i*interval, bonus_add-(bonus_add/times)*(times-1)));
			else
				itemadd.push_back(RewardItem(end_time+i*interval, bonus_add/times));
		}
	}
	RewardItemVector::const_iterator it, ite;
	RewardItemVector::iterator it2, ite2;
	for (it=itemadd.begin(),ite=itemadd.end(); it!=ite; ++it)
	{
		for (it2=rewardlist.begin(),ite2=rewardlist.end(); it2!=ite2; ++it2)
		{
			if (it->reward_time < it2->reward_time)
			{
				break;
			}
		}
		rewardlist.insert(it2, *it);
	}
/////////
	DBExchangeConsumePointsArg arg(roleid, userid, consume_points, DumpRewardList()); 
	DBExchangeConsumePoints *rpc = (DBExchangeConsumePoints *)Rpc::Call(RPC_DBEXCHANGECONSUMEPOINTS, arg);
	points_flag.SetDirty(false);
	points_flag.SetWriteback(true);
	reward_flag.SetWriteback(true);
	GameDBClient::GetInstance()->SendProtocol(rpc);
	return ERR_SUCCESS;
/*
	{
		return ERR_SUCCESS;
	}
	else
		return ERR_REWARD_DB_ERR;
*/
}

int RewardManager::ExchangePoints(int userid, int rewardtype, int & bonus_add) 
{
	if (!(status & ST_ACTIVE))
		return ERR_REWARD_NOT_ACTIVE;
	if (rewardtype < 0 || rewardtype >= (int)typelist.size())
		return ERR_REWARD_INVALID_TYPE;
	RewardMap::iterator it = rewards.find(userid);
	if (it == rewards.end() || it->second.IsLogout())
		return ERR_REWARD_PLAYER_NOT_ONLINE;
	bonus_add = typelist[rewardtype].second;
	return it->second.ExchangePoints(typelist[rewardtype].first, typelist[rewardtype].second,
				reward_interval, reward_times, end_time);
}

int RewardManager::GetRewardList(int userid, int & points, int & total, int start_index, RewardItemVector & list) 
{
	if (!(status & ST_OPEN)) return ERR_REWARD_NOT_ACTIVE;
	RewardMap::iterator it = rewards.find(userid);
	if (it == rewards.end() || it->second.IsLogout())
		return ERR_REWARD_PLAYER_NOT_ONLINE;
	return it->second.GetList(points, total, start_index, list);
}

int RewardManager::CheckRewardList(int userid, time_t future)
{
	if (!(status & ST_OPEN)) return ERR_REWARD_NOT_ACTIVE;
	RewardMap::iterator it = rewards.find(userid);
	if (it == rewards.end() || it->second.IsLogout())
		return ERR_REWARD_PLAYER_NOT_ONLINE;
	it->second.CheckRewardList(future);
	return ERR_SUCCESS;
}

};
