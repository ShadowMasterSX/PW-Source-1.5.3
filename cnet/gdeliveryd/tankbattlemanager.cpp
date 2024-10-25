#include <sstream>

#include "conf.h"
#include "glog.h"
#include "timer.h"
#include "parsestring.h"

#include "localmacro.h"
#include "mapuser.h"
#include "gproviderserver.hpp"
#include "gdeliveryserver.hpp"
#include "gamedbclient.hpp"
#include "tankbattlemanager.h"
#include "tankbattlestart.hpp"
#include "tankbattleenter.hpp"
#include "tankbattleplayerapply_re.hpp"
#include "dbtankbattlebonus.hrp"

namespace GNET
{

bool TankBattleManager::Initialize()
{
	std::string key = "TANKBATTLE";
	Conf* conf = Conf::GetInstance();
	
	std::string open_day_str = conf->find(key, "open_day");
	std::vector<string> open_day_vec;
	if(!ParseStrings(open_day_str, open_day_vec)) {
		Log::log( LOG_ERR,"TankBattleManager Initialize failed. opendaystr=%s.",open_day_str.c_str());
		return false;
	}
	for(size_t i = 0; i < open_day_vec.size(); ++i)
	{
		int openday = atoi(open_day_vec[i].c_str());
		if (openday < 0 || openday > 6)
		{
			Log::log( LOG_ERR,"TankBattleManager Initialize failed. openday=%d.",openday);
			return false;
		}
		_open_days[openday] = 1;
		LOG_TRACE("TankBattleManager Initialize open_day = %d\n",openday);
	}
	
	int hour=0,min=0;
	std::string start_time_str = conf->find(key, "start_time");
	sscanf(start_time_str.c_str(), "%d:%d",&hour,&min);
	if (hour < 0 || hour >= 23 || min < 0 || min >= 60)
	{
		Log::log( LOG_ERR,"TankBattleManager Initialize failed. start_time=%s.",start_time_str.c_str());
		return false;
	}
	_start_time = hour*3600 + min*60;
	LOG_TRACE("TankBattleManager Initialize start_hour = %d start_min=%d\n",hour,min);

	std::string end_time_str = conf->find(key, "end_time");
	sscanf(end_time_str.c_str(), "%d:%d",&hour,&min);
	if (hour < 0 || hour >= 23 || min < 0 || min >= 60)
	{
		Log::log( LOG_ERR,"TankBattleManager Initialize failed. end_time=%s.",end_time_str.c_str());
		return false;
	}
	_end_time = hour*3600 + min*60;
	LOG_TRACE("TankBattleManager Initialize end_hour = %d end_min=%d\n",hour,min);

	_min_time = 60*atoi(conf->find(key, "min_time").c_str());
	_max_time = 60*atoi(conf->find(key, "max_time").c_str());
	if (_min_time < 20*60 || _max_time > 60*60 || _min_time > _max_time)
	{
		Log::log(LOG_ERR,"TankBattleManager Initialize wrong. min_time=%d max_time=%d",_min_time,_max_time);
		return false;
	}
	
	if (_end_time <= _start_time + _max_time)
	{
		Log::log( LOG_ERR,"TankBattleManager Initialize failed. starttime=%d endtime=%d _max_time=%d.",_start_time,_end_time,_max_time);
		return false;
	}

	_max_player = atoi(conf->find(key, "max_player").c_str());
	if (_max_player < 1 || _max_player > 100)
	{
		Log::log(LOG_ERR,"TankBattleManager Initialize wrong. max_player=%d",_min_time,_max_player);
		return false;
	}

	_no_new_battle_time = 60*atoi(conf->find(key, "no_new_battle_time").c_str());
	if (_no_new_battle_time < 0 || _no_new_battle_time >= _min_time || 
			_min_time - _no_new_battle_time > TankBattleConfig::battle_man_wait_bonus_time)
	{
		Log::log(LOG_ERR,"TankBattleManager Initialize wrong. no_new_battle_time=%d _min_time=%d",_no_new_battle_time,_min_time);
		return false;
	}

	_cant_enter_time = 60*atoi(conf->find(key, "cant_enter_time").c_str());
	if (_cant_enter_time < 1*60 || _cant_enter_time >= _min_time)
	{
		Log::log(LOG_ERR,"TankBattleManager Initialize wrong. cant_enter_time=%d",_cant_enter_time);
		return false;
	}

	_bonus_item_id = atoi(conf->find(key, "bonus_item_id").c_str());
	if (_bonus_item_id <= 0)
	{
		Log::log(LOG_ERR,"TankBattleManager Initialize wrong. bonus_item_id=%d",_bonus_item_id);
		return false;
	}

	_bonus_max_count = atoi(conf->find(key, "bonus_max_count").c_str());
	if (_bonus_max_count <= 0)
	{
		Log::log(LOG_ERR,"TankBattleManager Initialize wrong. bonus_max_count=%d",_bonus_max_count);
		return false;
	}

	_bonus_proctype = atoi(conf->find(key, "bonus_proctype").c_str());
	if (_bonus_proctype <= 0)
	{
		Log::log(LOG_ERR,"TankBattleManager Initialize wrong. bonus_proctype=%d",_bonus_proctype);
		return false;
	}
	
	std::string bonus_count_str = conf->find(key, "bonus_count");
	if (bonus_count_str.length() > 1023 || bonus_count_str.length() == 0)
	{
		Log::log(LOG_ERR,"TankBattleManager Initialize wrong. bonus_count_str=%s",bonus_count_str.c_str());
		return false;
	}

	char buffer[1024];
	strncpy( buffer, bonus_count_str.c_str(), std::min(sizeof(buffer)-1,bonus_count_str.length()) );
	buffer[sizeof(buffer)-1] = 0;
	
	char * cur = buffer;
	char * token = strchr( cur, '(' );
	while( NULL != token )
	{
		BonusEntry be;
		memset(&be,0,sizeof(be));
		
		cur = token+1;
		token = strchr( cur, ',' );
		if( NULL == token )	break;
		*token = 0;
		be.rank = atoi(cur);

		cur = token+1;
		token = strchr( cur, ')' );
		if( NULL == token )	break;
		*token = 0;
		be.count = atoi(cur);

		if (be.rank <= 0 || be.count <= 0)
		{
			Log::log(LOG_ERR,"TankBattleManager Initialize wrong. bonus_count_str=%s",bonus_count_str.c_str());
			return false;
		}

		for (size_t i = 0; i < _bonus_entry_vec.size(); i++)
		{
			if (be.rank <= _bonus_entry_vec[i].rank)
			{
				//保证排名必须是从小到大的
				Log::log(LOG_ERR,"TankBattleManager Initialize wrong. bonus_count_str=%s",bonus_count_str.c_str());
				return false;
			}
		}

		LOG_TRACE("TankBattleManager Initialize BonusEntry. rank=%d count=%d",be.rank,be.count);

		_bonus_entry_vec.push_back(be);

		cur = token+1;
		token = strchr( cur, '(' );
	}
	if (_bonus_entry_vec.size() == 0)
	{
		Log::log(LOG_ERR,"TankBattleManager Initialize wrong. bonus_count_str=%s",bonus_count_str.c_str());
		return false;
	}

	IntervalTimer::Attach(this, 1000000/IntervalTimer::Resolution());
	return true;
}

void TankBattleManager::ResetBattleMgr()
{
	_send_bonus_per_second = 0;
	_calc_bonus = false;
	_battle_map.clear();
	_player_apply_map.clear();
	_player_entry_map.clear();
	_player_score_map.clear();
	_player_bonus_vec.clear();
	_top_score_vec.clear();
}

bool TankBattleManager::Update()
{
	time_t now = GetTime();

	struct tm dt;
	localtime_r(&now, &dt);
	_second_of_day = dt.tm_hour * 3600 + dt.tm_min * 60 + dt.tm_sec;

	if (BATTLE_MAN_STAT_CLOSE == _status)
	{
		bool is_open_day = _open_days[dt.tm_wday];
		bool is_open_time = (_second_of_day >= _start_time) && (_second_of_day < _end_time);
		if (is_open_day && is_open_time)
		{
			LOG_TRACE("TankBattleManager BattleBegin.");
			_status = BATTLE_MAN_STAT_OPEN;
			StartBattle();
		}
	}
	else if (BATTLE_MAN_STAT_OPEN == _status)
	{
		bool is_open_day = _open_days[dt.tm_wday];
		bool is_open_time = (_second_of_day >= _start_time) && (_second_of_day < _end_time);
		if (!is_open_day || !is_open_time)
		{
			LOG_TRACE("TankBattleManager BattleWillBonus.");
			_status = BATTLE_MAN_STAT_WAIT_BONUS;
		}
		else
		{
			UpdateServerInfo(now);
			UpdatePlayerInfo(now);
			UpdateBattleInfo(now);
		}
	}
	else if (BATTLE_MAN_STAT_WAIT_BONUS == _status)
	{
		//等待所有的战场房间全部结束
		if (_second_of_day > _end_time + TankBattleConfig::battle_man_wait_bonus_time + TankBattleConfig::battle_man_wait_bonus_time_out)
		{
			Log::log(LOG_ERR,"TankBattleManager BattleWillBonus timeout.");
			_status = BATTLE_MAN_STAT_BONUS;
		}
		else
		{
			UpdateBattleInfo(now);

			BATTLE_MAP::iterator bit=_battle_map.begin(), ebit=_battle_map.end();
			for ( ; bit!=ebit; ++bit)
			{
				if (BATTLE_STAT_CLOSE != (bit->second).status)
					return true;
			}
			LOG_TRACE("TankBattleManager BattleBonus.");
			_status = BATTLE_MAN_STAT_BONUS;
		}
	}
	else if (BATTLE_MAN_STAT_BONUS == _status)
	{
		//所有房间都已经结束，开始结算和发奖
		if (!_calc_bonus)
			CalcAllBonus();
		else
		{
			if(SendAllBonus() <= 0)
			{
				//已经发完奖励了
				LOG_TRACE("TankBattleManager BattleWillClose.");
				_status = BATTLE_MAN_STAT_WAIT_CLOSE;
			}
		}
	}
	else if (BATTLE_MAN_STAT_WAIT_CLOSE == _status)
	{
		if (_second_of_day > _end_time + TankBattleConfig::battle_man_before_close_time || 
				_second_of_day < _start_time)
		{
			LOG_TRACE("TankBattleManager BattleClose.");
			_status = BATTLE_MAN_STAT_CLOSE;
			ResetBattleMgr();
		}
	}

	return true;
}

void TankBattleManager::RegisterServerInfo(int world_tag, int server_id)
{
	SERVER_INFO_MAP::iterator it = _server_info_map.find(world_tag);
	if (it == _server_info_map.end())
	{
		if (TankBattleConfig::only_one_battle_server && !_server_info_map.empty())
		{
			//目前只开放了注册一个战场服务器
			Log::log(LOG_ERR,"TankBattleManager RegisterServerInfo duplicate. world_tag=%d,server_id=%d",world_tag,server_id);
			return;
		}

		ServerInfo si;
		si.world_tag = world_tag;
		si.server_id = server_id;
		si.time_stamp = GetTime();
		si.stat = SERVER_STAT_NORMAL;
		
		_server_info_map[world_tag] = si;
	}
	else
	{
		if (SERVER_STAT_DISCONNECT != (it->second).stat)
		{
			Log::log(LOG_ERR,"TankBattleManager RegisterServerInfo stat wrong. world_tag=%d server_id=%d stat=%d",
					world_tag,server_id,(int)(it->second).stat);
			return;
		}
		ServerInfo & si = it->second;
		if (si.battle_count >=  TankBattleConfig::max_battle_count)
		{
			si.time_stamp = GetTime();
			si.stat = SERVER_STAT_FULL;
		}
		else
		{
			si.time_stamp = GetTime();
			si.stat = SERVER_STAT_NORMAL;
		}
	}
	
	LOG_TRACE("TankBattleManager RegisterServerInfo worldtag=%d server_id=%d\n",world_tag,server_id);
}

void TankBattleManager::DisableServerInfo(int world_tag)
{
	ServerInfo * psi = GetServerInfoByWorldTag(world_tag);
	if (psi)
	{
		psi->time_stamp = GetTime();
		psi->stat = SERVER_STAT_DISCONNECT;
		
		LOG_TRACE("TankBattleManager DisableServerInfo worldtag=%d",world_tag);
	}
}

void TankBattleManager::StartBattle()
{
	if (_status != BATTLE_MAN_STAT_OPEN)
		return;

	SERVER_INFO_MAP::iterator it=_server_info_map.begin(), eit=_server_info_map.end();
	SERVER_INFO_MAP::iterator tit = eit;
	int battle_count = TankBattleConfig::max_battle_count;
	for ( ; it != eit; ++it)
	{
		ServerInfo & si = it->second;
		if (si.stat == SERVER_STAT_CREATE)
			return;	//正在创建副本呢，不要同时创建

		if (si.stat == SERVER_STAT_NORMAL && si.battle_count < battle_count)
		{
			tit = it;
			battle_count = si.battle_count;
		}
	}
	if (tit != eit)
	{
		ServerInfo & si = tit->second;
		// 通知gs开启战场
		int remain_time = _end_time - _second_of_day;
		if (remain_time < _no_new_battle_time)
			return;		//剩余时间太短了，不开放新的战场了

		if (remain_time < _min_time)
			remain_time = _min_time;

		int battle_id = GetFreeBattleID();
		int end_time = GetTime() + (_max_time > remain_time ? remain_time : _max_time);

		//设置ServerInfo处于正在创建副本状态
		si.stat = SERVER_STAT_CREATE;
		si.time_stamp = GetTime();
		
		//插入一个创建状态的BattleInfo
		BattleInfo bi;
		bi.battle_id = battle_id;
		bi.world_tag = tit->first;
		bi.end_time = end_time;
		bi.time_stamp = GetTime();
		bi.status = BATTLE_STAT_CREATE;

		_battle_map[battle_id] = bi;

		//发送消息给副本服务器创建副本
		TankBattleStart proto;
		proto.battle_id = battle_id;
		proto.end_time = end_time - _adjust_time;	//需要去掉战场自己的修正时间
		proto.max_player_cnt = _max_player;
		//服务器断开的话是不会处于Normal状态的，所以这里发送消息理论上不会失败
		GProviderServer::GetInstance()->DispatchProtocol(si.server_id, proto);

		LOG_TRACE("TankBattleManager StartBattle notify gs. serverid=%d battleid=%d", si.server_id, battle_id);
	}
	else
	{
		Log::log(LOG_ERR,"TankBattleManager StartBattle no free gs!");
	}
}

void TankBattleManager::OnBattleStart(int world_tag, int battleid, int retcode)
{
	BattleInfo * pbi = GetBattleByBattleID(battleid);
	if (!pbi || BATTLE_STAT_CREATE != pbi->status)
	{
		Log::log(LOG_ERR,"TankBattleManager OnBattleStart wrong battle. world_tag=%d battleid=%d",
				world_tag,battleid);
		return;
	}
	ServerInfo * psi = GetServerInfoByWorldTag(world_tag);
	if (!psi || SERVER_STAT_CREATE != psi->stat)
	{
		Log::log(LOG_ERR,"TankBattleManager OnBattleStart wrong serverinfo. world_tag=%d battleid=%d",world_tag,battleid);
		return;
	}
	if (ERR_SUCCESS == retcode)
	{
		psi->battle_count ++;

		if (psi->battle_count >= TankBattleConfig::max_battle_count)
		{
			psi->time_stamp = GetTime();
			psi->stat = SERVER_STAT_FULL;
		}
		else
		{
			psi->time_stamp = GetTime();
			psi->stat = SERVER_STAT_NORMAL;
		}
		
		pbi->status = BATTLE_STAT_OPEN;
		pbi->time_stamp = GetTime();
	}
	else
	{
		psi->time_stamp = GetTime();
		psi->stat = SERVER_STAT_ERROR;

		_battle_map.erase(battleid);

		//在别的gs上重新开启一个战场
		StartBattle();

		Log::log(LOG_ERR,"TankBattleManager OnBattleStart error. world_tag=%d battleid=%d, retcode=%d",
				world_tag, battleid, retcode);
	}

	LOG_TRACE("TankBattleManager OnBattleStart world_tag=%d battleid=%d, retcode=%d\n",world_tag,battleid,retcode);
}

void TankBattleManager::OnBattleEnd(int world_tag, int battleid)
{
	BattleInfo * pbi = GetBattleByBattleID(battleid);
	if (!pbi || BATTLE_STAT_OPEN != pbi->status)
	{
		Log::log(LOG_ERR,"TankBattleManager OnBattleEnd wrong battleinfo. world_tag=%d battle_id=%d",
				world_tag,battleid);
		return;
	}
	
	pbi->status = BATTLE_STAT_WAIT_CLOSE;
	pbi->time_stamp = GetTime();
	//不删除战场里的玩家了，等玩家出战场的协议删或者超时删
	
	ServerInfo * psi = GetServerInfoByWorldTag(world_tag);
	if (psi)
	{
		psi->battle_count --;
		
		//如果处于错误状态或者副本满了的状态，置回正常状态，下次能继续分配副本
		if (SERVER_STAT_ERROR == psi->stat || SERVER_STAT_FULL == psi->stat)
		{
			psi->time_stamp = GetTime();
			psi->stat = SERVER_STAT_NORMAL;
		}
	}

	LOG_TRACE("TankBattleManager OnBattleEnd worldtag=%d battleid=%d\n",world_tag,battleid);
}

void TankBattleManager::PlayerApply(int roleid, int model)
{
	LOG_TRACE("TankBattleManager PlayerApply roleid=%d model=%d\n",roleid,model);

	unsigned int linksid = 0;
	unsigned int localsid = 0;
	int gameid = -1;
	//查找玩家在线信息
	{
		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
		PlayerInfo* pInfo = UserContainer::GetInstance().FindRoleOnline(roleid);
		if (NULL == pInfo) 
			return;
		else
		{
			linksid = pInfo->linksid;
			localsid = pInfo->localsid;
			gameid = pInfo->gameid;
		}
	}

	if (_status != BATTLE_MAN_STAT_OPEN)
	{
		SendPlayerApplyRe(roleid,ERR_TANK_BATTLE_NOT_OPEN,linksid,localsid);
		return;
	}
	PLAYER_ENTRY_MAP::iterator it = _player_entry_map.find(roleid);
	if (it != _player_entry_map.end() && (it->second).stat != PLAYER_STAT_LEAVE)
	{
		SendPlayerApplyRe(roleid,ERR_TANK_BATTLE_ALREADY_APPLY,linksid,localsid);
		return;
	}
	if (_player_apply_map.find(roleid) != _player_apply_map.end())
	{
		SendPlayerApplyRe(roleid,ERR_TANK_BATTLE_ALREADY_APPLY,linksid,localsid);
		return;
	}

	PlayerApplyEntry pa;
	memset(&pa,0,sizeof(pa));
	pa.gameid = gameid;
	pa.model = model;

	_player_apply_map[roleid] = pa;

	if (it == _player_entry_map.end())
	{
		PlayerEntry pe;
		pe.roleid = roleid;
		pe.time_stamp = GetTime();
		pe.stat = PLAYER_STAT_APPLY;
		
		_player_entry_map[roleid] = pe;
	}
	else
	{
		PlayerEntry & pe = it->second;
		pe.roleid = roleid;
		pe.battle_id = 0;
		pe.time_stamp = GetTime();
		pe.stat = PLAYER_STAT_APPLY;
	}

	//SendPlayerApplyRe(roleid,ERR_SUCCESS,linksid,localsid);
}

void TankBattleManager::SendPlayerApplyRe(int roleid, int retcode, unsigned int linksid, unsigned int localsid)
{
	if (linksid == 0 && localsid == 0)
	{
		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
		PlayerInfo* pInfo = UserContainer::GetInstance().FindRoleOnline(roleid);
		if (NULL == pInfo) 
			return;
		else
		{
			linksid = pInfo->linksid;
			localsid = pInfo->localsid;
		}
	}

	TankBattlePlayerApply_Re proto;
	proto.roleid = roleid;
	proto.localsid = localsid;
	proto.retcode = retcode;

	GDeliveryServer::GetInstance()->Send(linksid, proto);
}

void TankBattleManager::PlayerEnterBattle(int roleid, int battle_id, int world_tag)
{
	BattleInfo * pbi = GetBattleByBattleID(battle_id);
	if (!pbi || pbi->world_tag != world_tag || pbi->status != BATTLE_STAT_OPEN)
	{
		Log::log(LOG_ERR,"TankBattleManager PlayerEnterBattle wrong. roleid=%d battle_id=%d world_tag=%d",
				roleid, battle_id, world_tag);
		return;
	}

	PlayerEntry * pe = GetPlayerEntryByRoleID(roleid);
	if (pe && pe->stat == PLAYER_STAT_SWITCH)
	{
		pe->time_stamp = GetTime();
		pe->stat = PLAYER_STAT_ENTER;
	}
	else
	{
		Log::log(LOG_ERR,"TankBattleManager PlayerEnterBattle entry wrong. roleid=%d stat=%d",roleid,(int)pe->stat);
	}

	//在报名列表里删掉，防止UpdatePlayerInfo里再次遍历到该玩家
	_player_apply_map.erase(roleid);

	LOG_TRACE("TankBattleManager PlayerEnterBattle roleid=%d battle_id=%d world_tag=%d total=%d\n",
			roleid,battle_id,world_tag,pbi->players.size());
}

void TankBattleManager::PlayerLeaveBattle(int roleid, int battle_id, int world_tag)
{
	BattleInfo * pbi = GetBattleByBattleID(battle_id);
	if (!pbi || pbi->world_tag != world_tag || BATTLE_STAT_CREATE == pbi->status || BATTLE_STAT_CLOSE == pbi->status)
	{
		Log::log(LOG_ERR,"TankBattleManager PlayerLeaveBattle wrong. roleid=%d battle_id=%d world_tag=%d",
				roleid, battle_id, world_tag);
		return;
	}

	PlayerEntry * pe = GetPlayerEntryByRoleID(roleid);
	if (pe)
	{
		pe->stat = PLAYER_STAT_LEAVE;
		pe->time_stamp = GetTime();
		pe->battle_id = 0;
	}
	
	_player_apply_map.erase(roleid);	//这里是安全性考虑，正常应该已经被删除了
	pbi->DelPlayer(roleid);

	LOG_TRACE("TankBattleManager PlayerLeaveBattle roleid=%d battle_id=%d world_tag=%d total=%d\n",
			roleid, battle_id, world_tag, pbi->players.size());
}

TankBattleManager::BattleInfo * TankBattleManager::GetBattleByBattleID(int battle_id)
{
	BATTLE_MAP::iterator it = _battle_map.find(battle_id);
	if (it != _battle_map.end())
		return &(it->second);

	return NULL;
}

TankBattleManager::ServerInfo * TankBattleManager::GetServerInfoByWorldTag(int world_tag)
{
	SERVER_INFO_MAP::iterator it = _server_info_map.find(world_tag);
	if (it != _server_info_map.end())
		return &(it->second);

	return NULL;
}

TankBattleManager::PlayerEntry * TankBattleManager::GetPlayerEntryByRoleID(int roleid)
{
	PLAYER_ENTRY_MAP::iterator it = _player_entry_map.find(roleid);
	if (it != _player_entry_map.end())
		return &(it->second);

	return NULL;
}

TankBattleManager::BattleInfo * TankBattleManager::GetMostPlayerAndHaveVacancyBattle()
{
	BATTLE_MAP::iterator it = _battle_map.begin(), eit = _battle_map.end();
	BATTLE_MAP::iterator tit = _battle_map.end();
	int ttotal = -1;
	for ( ; it != eit; ++it)
	{
		const BattleInfo & bi = it->second;
		if (bi.status != BATTLE_STAT_OPEN)
			continue;	//战场副本不是开启状态
		ServerInfo * psi = GetServerInfoByWorldTag(bi.world_tag);
		if (!psi || SERVER_STAT_DISCONNECT == psi->stat)
			continue;	//战场副本服务器已经断线
		if (bi.end_time - GetTime() < _cant_enter_time)
			continue;	//战场快结束了，不让人进了

		int total = (int)bi.players.size();
		if (total < _max_player && total > ttotal)
		{
			ttotal = total;
			tit = it;
		}
	}
	if (tit != eit)
		return &(tit->second);

	return NULL;
}

void TankBattleManager::UpdateServerInfo(time_t now_time)
{
	SERVER_INFO_MAP::iterator it = _server_info_map.begin(), eit = _server_info_map.end();
	for ( ; it != eit; ++it)
	{
		ServerInfo & si = it->second;
		if (SERVER_STAT_CREATE == si.stat)
		{
			if (now_time - si.time_stamp > TankBattleConfig::server_create_time_out)
			{
				si.time_stamp = GetTime();
				si.stat = SERVER_STAT_NORMAL;
				
				Log::log(LOG_ERR,"TankBattleManager UpdateServerInfo create timeout. worldtag=%d",si.world_tag);
			}
		}
		else if (SERVER_STAT_ERROR == si.stat)
		{
			if (now_time - si.time_stamp > TankBattleConfig::server_error_time_out)
			{
				si.time_stamp = GetTime();
				si.stat = SERVER_STAT_NORMAL;

				LOG_TRACE("TankBattleManager UpdateServerInfo error timeout. worldtag=%d\n",si.world_tag);
			}
		}
	}
}

void TankBattleManager::UpdateBattleInfo(time_t now_time)
{
	bool have_empty_battle = false;
	BATTLE_MAP::iterator it = _battle_map.begin();
	for ( ; it != _battle_map.end(); )
	{
		BattleInfo & bi = it->second;
		if (BATTLE_STAT_CREATE == bi.status)
		{
			if (now_time - bi.time_stamp > TankBattleConfig::server_create_time_out)
			{
				Log::log(LOG_ERR,"TankBattleManager UpdateBattleInfo create timeout. worldtag=%d battleid=%d",bi.world_tag,it->first);
				_battle_map.erase(it++);
				continue;
			}
		}
		else if (BATTLE_STAT_WAIT_CLOSE == bi.status)
		{
			if (now_time - bi.time_stamp > TankBattleConfig::battle_close_clear_time)
			{
				if (!bi.players.empty())
				{
					//战场已经结束一段时间了，还有玩家没有发送离开副本消息，一般不会出现
					std::vector<int>::iterator _it = bi.players.begin(), _eit = bi.players.end();
					for ( ; _it != _eit ; ++_it)
					{
						_player_apply_map.erase(*_it);
						_player_entry_map.erase(*_it);
					}
					bi.players.clear();
				}

				//把战场设置为正式关闭状态,删掉也可以,再斟酌
				bi.time_stamp = GetTime();
				bi.status = BATTLE_STAT_CLOSE;
			}
		}
		else if (BATTLE_STAT_OPEN == bi.status)
		{
			if (now_time > bi.end_time && now_time - bi.end_time > TankBattleConfig::battle_close_time_out)
			{
				Log::log(LOG_ERR,"TankBattleManager UpdateBattleInfo open time out. worldtag=%d battleid=%d",bi.world_tag, bi.battle_id);
				OnBattleEnd(bi.world_tag, bi.battle_id);
			}
			//这里就是找一下是否有空的战场
			else if (!have_empty_battle && bi.players.empty() && bi.end_time - now_time > _cant_enter_time)
				have_empty_battle = true;
		}

		it ++;
	}

	//如果没有空的战场了，提前开辟一个准备着
	if (!have_empty_battle)
		StartBattle();
}

void TankBattleManager::UpdatePlayerInfo(time_t now_time)
{
	size_t assign_count = 0;

	PLAYER_APPLY_MAP::iterator it = _player_apply_map.begin();
	for ( ; it != _player_apply_map.end(); )
	{
		PLAYER_ENTRY_MAP::iterator _it = _player_entry_map.find(it->first);
		if (_it == _player_entry_map.end())
		{
			//找不到玩家的信息了，这个不太可能出现
			Log::log(LOG_ERR,"TankBattleManager UpdatePlayerBattle cant find playerentry. roleid=%d",it->first);
			_player_apply_map.erase(it++);
			continue;
		}

		PlayerEntry & pe = _it->second;
		
		if (PLAYER_STAT_SWITCH == pe.stat)
		{
			if ((now_time - pe.time_stamp) > TankBattleConfig::player_switch_time_out)
			{
				//玩家跳转服务器超时，一般不可能出现
				Log::log(LOG_ERR,"TankBattleManager UpdatePlayerBattle playerswitch timeout.roleid=%d",it->first);

				BattleInfo * pbi = GetBattleByBattleID(pe.battle_id);
				if (pbi) pbi->DelPlayer(_it->first);
				
				SendPlayerApplyRe(_it->first, ERR_TANK_BATTLE_SWITCH_TIMEOUT);
				
				_player_entry_map.erase(_it);
				_player_apply_map.erase(it++);
				continue;
			}
		}
		else if (PLAYER_STAT_APPLY == pe.stat)
		{
			BattleInfo * pbi = GetMostPlayerAndHaveVacancyBattle();
			if (pbi)
			{
				int gameid = -1;
				//查看玩家在不在线
				{
					Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
					PlayerInfo* pInfo = UserContainer::GetInstance().FindRoleOnline(_it->first);
					if (NULL == pInfo) 
					{
						//玩家不在线了，报名信息删了吧
						_player_entry_map.erase(_it);
						_player_apply_map.erase(it++);
						continue;
					}
					gameid = pInfo->gameid;
				}
				if (gameid != (it->second).gameid)
				{
					//玩家在申请之后切换过地图，不让他进了
					SendPlayerApplyRe(it->first, ERR_TANK_BATTLE_NOT_APPLY_MAP);
					
					_player_entry_map.erase(_it);
					_player_apply_map.erase(it++);
					continue;
				}
				//GetMostPlayerAndHaveVacancyBattle里已经检查过serverinfo信息了，不用再检查了
				SERVER_INFO_MAP::iterator sit = _server_info_map.find(pbi->world_tag);

				//分配一个战场位置
				pbi->players.push_back(it->first);

				//设置玩家信息
				pe.time_stamp = GetTime();
				pe.stat = PLAYER_STAT_SWITCH;
				pe.battle_id = pbi->battle_id;

				//传送玩家进副本
				TankBattleEnter proto;
				proto.roleid = it->first;
				proto.battle_id = pbi->battle_id;
				proto.world_tag = pbi->world_tag;
				proto.model = (it->second).model;
				GProviderServer::GetInstance()->DispatchProtocol(gameid, proto);

				LOG_TRACE("TankBattleManager UpdatePlayerBattle assign player. roleid=%d battleid=%d worldtag=%d game_id=%d total=%d",
						it->first, pbi->battle_id, pbi->world_tag, gameid, pbi->players.size());
				
				if (TankBattleConfig::assign_count_each_tick != 0 && ++ assign_count >= TankBattleConfig::assign_count_each_tick)
					break;
			}
			else
			{
				//重新开启一个战场
				StartBattle();
				break;
			}
		}

		it++;
	}
}

int TankBattleManager::GetTime() const
{
	return Timer::GetTime() + _adjust_time;
}

void TankBattleManager::PlayerScoreUpdate(int battle_id, int world_tag, const TankBattlePlayerScoreInfoVector & player_scores)
{
	if (BATTLE_MAN_STAT_OPEN != _status && BATTLE_MAN_STAT_WAIT_BONUS != _status)
		return;

	BattleInfo * pbi = GetBattleByBattleID(battle_id);
	if (!pbi || pbi->world_tag != world_tag || BATTLE_STAT_CREATE == pbi->status)
	{
		Log::log(LOG_ERR,"TankBattleManager PlayerScoreUpdate wrong. battleid=%d world_tag=%d",battle_id,world_tag);
		return;
	}

	TankBattlePlayerScoreInfoVector::const_iterator it = player_scores.begin(), eit = player_scores.end();
	for ( ; it != eit; ++it)
	{
		PLAYER_SCORE_MAP::iterator sit = _player_score_map.find((*it).roleid);
		if (sit == _player_score_map.end())
		{
			_player_score_map[(*it).roleid] = *it;
		}
		else
		{
			TankBattlePlayerScoreInfo & psi = sit->second;
			psi.kill_cnt += (*it).kill_cnt;
			psi.dead_cnt += (*it).dead_cnt;
			psi.score += (*it).score;
		}

		TankBattlePlayerScoreInfo & s = _player_score_map[(*it).roleid];
		LOG_TRACE("PlayerScoreUpdate roleid=%d killcnt=%d deadcnt=%d score=%d tkillcnt=%d tdeadcnt=%d tscore=%d\n",
				(*it).roleid,(*it).kill_cnt,(*it).dead_cnt,(*it).score,s.kill_cnt,s.dead_cnt,s.score);
	}

	//重新计算排行榜，比较耗，所以尽量少调用更新玩家积分
	CalcPlayerRank();

	LOG_TRACE("PlayerScoreUpdate battle_id=%d world_tag=%d player_scores size=%d topsize=%d\n",
			battle_id,world_tag,player_scores.size(),_top_score_vec.size());
}

void TankBattleManager::CalcPlayerRank()
{
	_top_score_vec.clear();

	//把所有玩家的积分和roleid插入到一个从大到小排序的multimap中进行排序
	PLAYER_SCORE_RANK_MAP psrm;

	PLAYER_SCORE_MAP::iterator it = _player_score_map.begin(), eit = _player_score_map.end();
	for ( ; it != eit; ++it)
	{
		psrm.insert(std::make_pair((it->second).score,&(it->second)));
	}

	//更新一下玩家积分信息里的排名，方便玩家查询自己的排名，不用重新遍历
	//同时把前rank_show_count名的信息拷贝到一个vector中，增加查询的检索速度
	int rank = 1;
	PLAYER_SCORE_RANK_MAP::iterator rit = psrm.begin(), erit = psrm.end();
	for ( ; rit != erit; ++rit)
	{
		(rit->second)->rank = rank;

		if (rank++ <= TankBattleConfig::rank_show_count)
		{
			_top_score_vec.push_back(rit->second);
		}
	}
}

int TankBattleManager::PlayerGetRank(int roleid, TankBattlePlayerScoreInfo & self_score, 
			TankBattlePlayerScoreInfoVector & player_scores) const
{
	if (BATTLE_MAN_STAT_CLOSE == _status)
		return ERR_TANK_BATTLE_NOT_OPEN;

	//先找到自己的排名信息
	PLAYER_SCORE_MAP::const_iterator it = _player_score_map.find(roleid);
	if (it != _player_score_map.end())
	{
		self_score = it->second;
	}

	//查找前rank_show_count名的信息
	TOP_SCORE_VEC::const_iterator rit = _top_score_vec.begin(), erit = _top_score_vec.end();
	for ( ; rit != erit; ++rit)
	{
		player_scores.push_back(**rit);
	}
	return ERR_SUCCESS;
}

void TankBattleManager::CalcAllBonus()
{
	_calc_bonus = true;

	//计算所有玩家的奖励,插入到_player_bonus_vec中
	PLAYER_SCORE_MAP::iterator it = _player_score_map.begin(), eit = _player_score_map.end();
	for ( ; it!=eit; ++it)
	{
		int rank = (it->second).rank;
		int bonus = CalcPlayerBonus(rank);
		if (bonus < 0)
			continue;

		PlayerBonus pb;
		pb.roleid = it->first;
		pb.rank = rank;
		pb.bonus = bonus;
		_player_bonus_vec.push_back(pb);
	}

	_send_bonus_per_second = _player_bonus_vec.size()/1800 + 1;	//在30分钟内发完所有奖励
	LOG_TRACE("TankBattleManager CalcAllBonus. player_bonus_vec size=%d _send_bonus_per_second=%d\n",
			_player_bonus_vec.size(), _send_bonus_per_second);
}

int TankBattleManager::CalcPlayerBonus(int rank)
{
	int count = 0;
	size_t size = _bonus_entry_vec.size();
	for (size_t i = 0; i < size; i++)
	{
		BonusEntry & be = _bonus_entry_vec[i];
		if (rank <= be.rank)
		{
			count = be.count;
			break;
		}
	}

	return count > _bonus_max_count ? _bonus_max_count : count;
}

int TankBattleManager::SendAllBonus()
{
	if (_player_bonus_vec.empty())
		return 0;

	int count = 0;
	PLAYER_BONUS_VEC::iterator it=_player_bonus_vec.begin(),eit=_player_bonus_vec.end();
	for ( ; it!=eit; ++it)
	{
		SendPlayerBonus((*it).roleid, (*it).bonus, (*it).rank);
		
		if (++count >= _send_bonus_per_second)
			break;
	}

	_player_bonus_vec.erase(_player_bonus_vec.begin(), _player_bonus_vec.begin() + count);
	LOG_TRACE("TankBattleManager SendAllBonus. count=%d",count);
	return count;
}

void TankBattleManager::SendPlayerBonus(int roleid, int player_bonus, int rank)
{
	GRoleInventory inv;
	inv.id = _bonus_item_id;
	inv.count = player_bonus;
	inv.max_count = _bonus_max_count;
	inv.proctype = _bonus_proctype;

	DBTankBattleBonusArg arg;
	arg.roleid = roleid;
	arg.rank = rank;
	arg.money = 0;
	arg.item = inv;

	Log::formatlog("tankbattlebonus","zoneid=%d:roleid=%d:amount=%d", GDeliveryServer::GetInstance()->GetZoneid(), roleid, player_bonus);
	DBTankBattleBonus* rpc = (DBTankBattleBonus*) Rpc::Call( RPC_DBTANKBATTLEBONUS, arg);
	GameDBClient::GetInstance()->SendProtocol( rpc );
}

}
