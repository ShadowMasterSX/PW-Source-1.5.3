#include "cdcmnfbattleman.h"
#include "mnfactionproclaim.hpp"
#include "battlemanager.h"
#include "gamedbclient.hpp"
#include "dbmnfactioncacheupdate.hpp"
#include "dbmnfactionapplyresnotify.hrp"
#include "gproviderserver.hpp"
#include "mndomainsendbonusdata_re.hpp"
#include "dbmnsendbattlebonus.hrp"
#include "mnfetchfiltrateresult.hpp"
#include "mnfetchtoplist.hpp"
#include "mngetfactionbriefinfo_re.hpp"
#include "mngettoplist_re.hpp"
#include "mndomaininfogsnotice.hpp"
#include "dbmnfactionapplyinfoget.hrp"
#include "mngetfactioninfo_re.hpp"
#include "mapuser.h"
#include "disabled_system.h"

void CDC_MNFactionBattleMan::BattleFactionInfo::Init(MNFactionInfo& info)
{
	unifid			= info.unifid;
	fid				= info.fid;
	faction_name	= info.faction_name;
	master_name		= info.master_name;
	domain_num		= info.domain_num;
	zoneid			= info.zoneid;
	credit			= info.credit;
	credit_this_week= info.credit_this_week;
	credit_get_time = info.credit_get_time;
	invite_count	= info.invite_count;
	accept_sn		= info.accept_sn;
	bonus_sn		= info.bonus_sn;
	version			= info.version;
}

bool CDC_MNFactionBattleMan::LoadConfig()
{
	std::string key = "MNFBATTLE";
	Conf* conf = Conf::GetInstance();

#if 0
	int day = 0 ,hour = 0, min = 0, sec = 0;

	std::string apply_begin_time_str = conf->find(key, "apply_begin_time");
	sscanf(apply_begin_time_str.c_str(), "[%d]%d:%d:%d", &day, &hour, &min, &sec);
	_apply_begin_time = day * 24 * 3600 + hour * 3600 + min * 60 + sec;

	std::string apply_end_time_str = conf->find(key, "apply_end_time");
	sscanf(apply_end_time_str.c_str(), "[%d]%d:%d:%d", &day, &hour, &min, &sec);
	_apply_end_time = day * 24 * 3600 + hour * 3600 + min * 60 + sec;

	_fetch_filtrate_res_time = _apply_end_time + TIME_ADJUST_FETCH_APPLYRES_TO_APPLYEND;

	std::string cross_begin_time_str = conf->find(key, "cross_begin_time");
	sscanf(cross_begin_time_str.c_str(), "[%d]%d:%d:%d", &day, &hour, &min, &sec);
	_cross_begin_time = day * 24 * 3600 + hour * 3600 + min * 60 + sec;

	//战斗结束，就不允许跨服了
	std::string battle_end_time_str = conf->find(key, "battle_end_time");
	sscanf(battle_end_time_str.c_str(), "[%d]%d:%d:%d", &day, &hour, &min, &sec);
	int battle_end_time = day * 24 * 3600 + hour * 3600 + min * 60 + sec;
	_cross_end_time = battle_end_time + TIME_ADJUST_BONUS_TO_BATTLEEND + TIME_ADJUST_CLOSE_TO_BONUS;

	_close_time = _cross_end_time + 10*60; //本服在跨服结束后十分钟活动结束

	if(!(_apply_begin_time < _apply_end_time && _apply_end_time < _fetch_filtrate_res_time
		&& _fetch_filtrate_res_time < _cross_begin_time && _cross_begin_time < _cross_end_time && _cross_end_time < _close_time))
	{
		return false;
	}
#endif

	_domain_count_lvl1 = atoi(conf->find(key, "domain_count_lvl1").c_str());
	_domain_count_lvl2 = atoi(conf->find(key, "domain_count_lvl2").c_str());
	_domain_count_lvl3 = atoi(conf->find(key, "domain_count_lvl3").c_str());
	_max_apply_faction_num = atoi(conf->find(key, "max_apply_faction_num").c_str());
	DEBUG_PRINT("_domain_count_lvl1=%d,_domain_count_lvl2=%d,_domain_count_lvl3=%d,_max_apply_faction_num=%d",_domain_count_lvl1, _domain_count_lvl2, _domain_count_lvl3, _max_apply_faction_num);

	if(!(_domain_count_lvl1 < _domain_count_lvl2 && _domain_count_lvl2 < _domain_count_lvl3 && _max_apply_faction_num >= 0))
	{
		return false;
	}
	
	return true;
}

bool CDC_MNFactionBattleMan::Initialize()
{
	if(!LoadConfig()) return false;
	//InitCurrentState();
	IntervalTimer::Attach(this, 3000000/IntervalTimer::Resolution());
	return true;
}

void CDC_MNFactionBattleMan::InitCurrentState(int apply_begin_time, int apply_end_time, int cross_begin_time, int battle_begin_time, int battle_end_time)
{
	_apply_begin_time = apply_begin_time;
	_apply_end_time		= apply_end_time;
	_fetch_filtrate_res_time = _apply_end_time + TIME_ADJUST_FETCH_APPLYRES_TO_APPLYEND;
	_cross_begin_time= cross_begin_time;
	_cross_end_time = battle_end_time + TIME_ADJUST_BONUS_TO_BATTLEEND + TIME_ADJUST_CLOSE_TO_BONUS;
	_close_time = _cross_end_time + 10*60; //本服在跨服结束后十分钟活动结束

	if(!(_apply_begin_time < _apply_end_time && _apply_end_time < _fetch_filtrate_res_time
		&& _fetch_filtrate_res_time < _cross_begin_time && _cross_begin_time < _cross_end_time && _cross_end_time < _close_time))
	{
		Log::log(LOG_ERR, "[func=%s] time err", __FUNCTION__);
		return ;
	}
	
	time_t now = GetTime();
	struct tm dt;
	localtime_r(&now, &dt);
	int wday = (dt.tm_wday == 0)? (7):(dt.tm_wday);
	int time_in_a_week = wday * 24 * 3600
		+ dt.tm_hour * 3600
		+ dt.tm_min * 60
		+ dt.tm_sec;

	if(time_in_a_week >= _close_time)
	{
		_state = STATE_CLOSE; 
	}
	else if(time_in_a_week >= _cross_end_time)
	{
		_state = STATE_CROSS_END;
	}
	else if(time_in_a_week >= _cross_begin_time)
	{
		_state = STATE_CROSS_BEGIN;
	}
	else if(time_in_a_week >= _fetch_filtrate_res_time)
	{
		_state = STATE_FETCH_CDS_FILTRATE_RES;
	}
	else if(time_in_a_week >= _apply_end_time)
	{
		_state = STATE_APPLY_END;
	}
	else if(time_in_a_week >= _apply_begin_time)
	{
		_state = STATE_APPLY_BEGIN;
	}
	else 
	{
		_state = STATE_CLOSE;
	}
	DebugDumpFlag(__FUNCTION__, __LINE__);
}

bool CDC_MNFactionBattleMan::Update()
{
	if(!_is_init_db) return true;
	if(!GameDBClient::GetInstance()->IsConnect()) return true;
	if(!_is_init_central) return true;
	if(!CentralDeliveryClient::GetInstance()->IsConnect()) {EnableUpdateBattleCacheFlag(); return true;} 
		
	time_t now = GetTime();
	struct tm dt;
	localtime_r(&now, &dt);

	int wday = (dt.tm_wday == 0)? (7):(dt.tm_wday);
	int time_in_a_week = wday * 24 * 3600
		+ dt.tm_hour * 3600
		+ dt.tm_min * 60
		+ dt.tm_sec;
	switch(_state)
	{
		case STATE_CLOSE:
			{
				if((time_in_a_week >= _apply_begin_time) && (time_in_a_week < _apply_end_time))
				{
					GSMNDomainInfoBroadcast(); //广播给gamed，domaininfo信息
					_state = STATE_APPLY_BEGIN;
					DebugDumpFlag(__FUNCTION__, __LINE__);
				}
			}
			break;

		case STATE_APPLY_BEGIN:
			{
				//可以接受gs的报名
				if(time_in_a_week >= _apply_end_time)
				{
					_state = STATE_APPLY_END;
					DebugDumpFlag(__FUNCTION__, __LINE__);
				}
			}
			break;

		case STATE_APPLY_END:
			{
				//(1)报名结束，本服筛选
				//(2)通知DB
				if(time_in_a_week >= _fetch_filtrate_res_time)
				{
					//下个状态需要更新缓存
					_is_need_update_battle_cache = true;
					_state = STATE_FETCH_CDS_FILTRATE_RES;
					DebugDumpFlag(__FUNCTION__, __LINE__);
				}

				if(!_is_filtrated_cdc)
				{
					DebugDumpFlag(__FUNCTION__, __LINE__);
					//筛选报名资格(本服),将结果通知DB
					std::vector<int64_t> chosen_list;
					FiltrateApplyInfoINCDC(chosen_list);	
					NotifyDBFiltrateRes(chosen_list);
					_is_filtrated_cdc = true;
				}

				if(!_is_send_proclaim_success && _is_notify_db_apply_res)
				{
					DebugDumpFlag(__FUNCTION__, __LINE__);
					SendCDSProclaim();
				}
			}
			break;

		case STATE_FETCH_CDS_FILTRATE_RES: 
			{
				//(1)更新缓存
				//(2)获取跨服筛选结果
				if(time_in_a_week >= _cross_begin_time)
				{
					_state = STATE_CROSS_BEGIN;
					DebugDumpFlag(__FUNCTION__, __LINE__);
				}

				if(NeedUpdateBattleCache())
				{
					DebugDumpFlag(__FUNCTION__, __LINE__);
					GetCDSBattleCache();		
					FetchCDSFiltrateRes();
				}

			}
			break;

		case STATE_CROSS_BEGIN:
			{
				if(time_in_a_week >= _cross_end_time)
				{
					//下个状态需要更新排行榜
					CDC_MNToplistMan::GetInstance()->EnableNeedFetchToplistFlag();
					//下个状态需要更新缓存
					_is_need_update_battle_cache = true;
					_state = STATE_CROSS_END;
					DebugDumpFlag(__FUNCTION__, __LINE__);
				}

				if(!_is_get_cds_filtrate_res)
				{
					DebugDumpFlag(__FUNCTION__, __LINE__);
					//主要是为了清空apply缓存,为了避免服务器在之前状态下出现停机
					FetchCDSFiltrateRes();
				}
			}
			break;

		case STATE_CROSS_END:
			{
				// (1)战斗结束，更新缓存
				// (2)获取排行榜数据
				if(time_in_a_week >= _close_time)
				{
					_state = STATE_CLOSE;
					DebugDumpFlag(__FUNCTION__, __LINE__);
					Clear();
				}

				if(NeedUpdateBattleCache())
				{
					DebugDumpFlag(__FUNCTION__, __LINE__);
					GetCDSBattleCache();		
				}

				if(!_is_get_cds_filtrate_res)
				{
					DebugDumpFlag(__FUNCTION__, __LINE__);
					//主要是为了清空apply缓存,为了避免服务器在之前状态下出现停机
					FetchCDSFiltrateRes();
				}

				if(CDC_MNToplistMan::GetInstance()->NeedFetchToplist())
				{
					DebugDumpFlag(__FUNCTION__, __LINE__);
					CDC_MNToplistMan::GetInstance()->GetMNToplist();
				}
			}
			break;

		default:
			break;
	}

	return true;
}

void CDC_MNFactionBattleMan::OnInitialize()
{
	DEBUG_PRINT("CDC_MNFactionBattleMan:func=%s\n",__FUNCTION__);
	GameDBClient::GetInstance()->SendProtocol(Rpc::Call(RPC_DBMNFACTIONAPPLYINFOGET, DBMNFactionApplyInfoGetArg()));
}

void CDC_MNFactionBattleMan::GSMNDomainInfoNotice(unsigned int sid)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
	MNDomainInfoGSNotice pro;
	for(DOMAIN_MAP::iterator it = _domain_map.begin(); it != _domain_map.end(); ++ it)
	{
		pro.domaininfo_list.push_back(it->second);
	}
	GProviderServer::GetInstance()->Send(sid, pro);
	//DebugDumpDomainInfo(__FUNCTION__, pro.domaininfo_list);
}

void CDC_MNFactionBattleMan::GSMNDomainInfoBroadcast()
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
	MNDomainInfoGSNotice pro;
	for(DOMAIN_MAP::iterator it = _domain_map.begin(); it != _domain_map.end(); ++ it)
	{
		pro.domaininfo_list.push_back(it->second);
	}
	GProviderServer::GetInstance()->BroadcastProtocol(pro);
	//DebugDumpDomainInfo(__FUNCTION__, pro.domaininfo_list);
}

void CDC_MNFactionBattleMan::SendCDSProclaim()
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
	if(!_is_send_proclaim_success)
	{
		MNFactionProclaim pro;
		pro.zoneid = GDeliveryServer::GetInstance()->GetZoneid();
		for(APPLYINFO_MAP::iterator it = _apply_map.begin(); it != _apply_map.end(); ++ it)
		{
			FACTION_MAP::iterator fit = _faction_map.find(it->first);
			if(fit == _faction_map.end())
			{
				Log::log(LOG_ERR, "[func=%s] applyinfo [unifid=%lld] not found in faction map", __FUNCTION__, it->first); 
				continue;
			}
			
			pro.applyinfo_list.push_back(it->second);

			MNFactionInfo info;
			info.unifid				= fit->second.unifid;
			info.fid				= fit->second.fid;
			info.faction_name		= fit->second.faction_name;
			info.master_name		= fit->second.master_name;
			info.zoneid				= fit->second.zoneid;
			info.credit				= fit->second.credit;
			info.credit_this_week	= fit->second.credit_this_week;
			info.credit_get_time	= fit->second.credit_get_time;
			info.invite_count		= fit->second.invite_count;
			info.accept_sn			= fit->second.accept_sn;
			info.bonus_sn			= fit->second.bonus_sn;
			info.version			= fit->second.version;
			info.domain_num.reserve(3);
			info.domain_num.push_back(0);
			info.domain_num.push_back(0);
			info.domain_num.push_back(0);
			pro.factioninfo_list.push_back(info);

			if(fit->second.domain_num[DOMAIN_TYPE_C] == 0 && fit->second.domain_num[DOMAIN_TYPE_B] == 0 
			&& it->second.dest == DOMAIN_TYPE_C && GetDomainCount(fit->second.fid) >= _domain_count_lvl3)
			{
				pro.lvl3_list.push_back(info.unifid);
			}
		}

		CentralDeliveryClient::GetInstance()->SendProtocol(pro);
		DebugDumpApplyInfo(__FUNCTION__, pro.applyinfo_list);
		DebugDumpUnifid("pro.lvl3_list", pro.lvl3_list);
	}
}

void CDC_MNFactionBattleMan::GetCDSBattleCache()
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
	//if(NeedUpdateBattleCache())
	{
		MNFactionCacheGet pro;
		pro.zoneid = GDeliveryServer::GetInstance()->GetZoneid();
		CentralDeliveryClient::GetInstance()->SendProtocol(pro);
	}
}

void CDC_MNFactionBattleMan::OnGetCDSBattleCache(unsigned int sn, int apply_begin_time, int apply_end_time, int cross_begin_time, int battle_begin_time,
		int battle_end_time, MNDomainInfoVector & domaininfo_list, MNFactionInfoVector & factioninfo_list)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
	//if(NeedUpdateBattleCache())
	{
		_sn = sn;
		UpdateMNFactionInfoMap(factioninfo_list);
		UpdateMNDomainInfoMap(domaininfo_list);
		SaveForClientDomainData();
		SaveForClientApplyData();
		UpdateDBMNFactionCache(domaininfo_list, factioninfo_list);
		GSMNDomainInfoBroadcast(); //广播给gamed，domaininfo信息
		InitCurrentState(apply_begin_time, apply_end_time, cross_begin_time, battle_begin_time, battle_end_time);
		_is_need_update_battle_cache = false;
		_is_init_central = true;
		DEBUG_PRINT("CDC CDS is init");
	}
}

void CDC_MNFactionBattleMan::OnGetMNFApplyInfoList(MNFactionApplyInfoVector &applyinfo_list)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
	for(MNFactionApplyInfoVector::iterator it = applyinfo_list.begin(); it != applyinfo_list.end(); ++ it)
	{
		_apply_map[it->unifid] = *it;
	}
	_is_init_db = true;
	DEBUG_PRINT("CDC DB is init");
	DebugDumpApplyInfo(__FUNCTION__);
}

void CDC_MNFactionBattleMan::UpdateMNFactionInfo(MNFactionInfo& factioninfo)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
	if(factioninfo.domain_num.size() != DOMAIN_TYPE_COUNT)
	{
		Log::log(LOG_ERR, "[func=%s]faction domain_num size error[unifid=%lld][size=%d]", __FUNCTION__, factioninfo.unifid, factioninfo.domain_num.size());
		return;
	}
	BattleFactionInfo info;
	info.Init(factioninfo);
	_faction_map[factioninfo.unifid] = info;
	DebugDumpFactionInfo(__FUNCTION__);
}

void CDC_MNFactionBattleMan::UpdateMNFactionInfoMap(MNFactionInfoVector& factioninfo_list)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
	for(MNFactionInfoVector::iterator it = factioninfo_list.begin(); it != factioninfo_list.end(); ++ it)
	{
		if(it->domain_num.size() != DOMAIN_TYPE_COUNT)
		{
			Log::log(LOG_ERR, "[func=%s]faction domain_num size error[unifid=%lld][size=%d]", __FUNCTION__, it->unifid, it->domain_num.size());
			continue;
		}
		BattleFactionInfo info;
		info.Init(*it);
		_faction_map[it->unifid] = info;
	}
	DebugDumpFactionInfo(__FUNCTION__, factioninfo_list);
}

void CDC_MNFactionBattleMan::UpdateMNDomainInfoMap(MNDomainInfoVector &domaininfo_list)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
	for(MNDomainInfoVector::iterator it = domaininfo_list.begin(); it != domaininfo_list.end(); ++ it)
	{
		_domain_map[it->domain_id] = *it;
	}
	DebugDumpDomainInfo(__FUNCTION__, domaininfo_list);
}

void CDC_MNFactionBattleMan::UpdateDBMNFactionCache(MNDomainInfoVector &domaininfo_list, MNFactionInfoVector& factioninfo_list)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
	DBMNFactionCacheUpdate pro;
	pro.factioninfo_list = factioninfo_list;
	GameDBClient::GetInstance()->SendProtocol(pro);
}

void CDC_MNFactionBattleMan::FiltrateApplyInfoINCDC(std::vector<int64_t>& chosen_list)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
	
	std::vector<int64_t> candidates_lvl1, candidates_lvl2, candidates_lvl3;
	for(APPLYINFO_MAP::iterator it = _apply_map.begin(); it != _apply_map.end(); )
	{
		int64_t unifid = it->first;
		FACTION_MAP::iterator fit = _faction_map.find(unifid);
		if(fit == _faction_map.end())
		{
			Log::log(LOG_ERR, "[func=%s]apply faction not found in faction map [unifid=%lld]", __FUNCTION__, unifid);
			_apply_map.erase(it ++);
			continue;
		}
		unsigned int fid = fit->second.fid;
		if(1 == UpdateCandidatesFaction(fid, unifid, candidates_lvl1, candidates_lvl2, candidates_lvl3))
		{
			chosen_list.push_back(unifid);
		}
		++ it;
	}

	int num = (int)_max_apply_faction_num;
	if(candidates_lvl3.size() > 0)
	{
		for(std::vector<int64_t>::iterator it = candidates_lvl3.begin();
				it != candidates_lvl3.end(); ++ it)
		{
			APPLYINFO_MAP::iterator ait = _apply_map.find(*it);
			if(ait != _apply_map.end())
			{
				chosen_list.push_back(ait->first);
				-- num;
			}
			else
			{
				Log::log(LOG_ERR, "[func=%s]apply faction not found in faction map [unifid=%lld]", __FUNCTION__, ait->first);
			}
		}
	}

	if((int)candidates_lvl2.size() <= num)
	{
		for(std::vector<int64_t>::iterator it = candidates_lvl2.begin();
				it != candidates_lvl2.end(); ++ it)
		{
			APPLYINFO_MAP::iterator ait = _apply_map.find(*it);
			if(ait != _apply_map.end())
			{
				chosen_list.push_back(ait->first);
				-- num;
			}
			else
			{
				Log::log(LOG_ERR, "[func=%s]apply faction not found in faction map [unifid=%lld]", __FUNCTION__, ait->first);
			}
		}
	}
	else
	{
		std::vector<int64_t> candidates = candidates_lvl2;
		while(num > 0)
		{
			int index = rand() % candidates.size();
			int64_t unifid = candidates[index];
			APPLYINFO_MAP::iterator ait = _apply_map.find(unifid);
			if(ait != _apply_map.end())
			{
				chosen_list.push_back(ait->first);
			}
			else
			{
				Log::log(LOG_ERR, "[func=%s]apply faction not found in faction map [unifid=%lld]", __FUNCTION__, unifid);
			}
			-- num;
			candidates.erase(candidates.begin() + index);
		}
	}

	if((int)candidates_lvl1.size() <= num)
	{
		for(std::vector<int64_t>::iterator it = candidates_lvl1.begin();
				it != candidates_lvl1.end(); ++ it)
		{
			APPLYINFO_MAP::iterator ait = _apply_map.find(*it);
			if(ait != _apply_map.end())
			{
				chosen_list.push_back(ait->first);
				-- num;
			}
			else
			{
				Log::log(LOG_ERR, "[func=%s]apply faction not found in faction map [unifid=%lld]", __FUNCTION__, ait->first);
			}
		}
	}
	else
	{
		std::vector<int64_t> candidates = candidates_lvl1;
		while(num > 0)
		{
			int index = rand() % candidates.size();
			int64_t unifid = candidates[index];
			APPLYINFO_MAP::iterator ait = _apply_map.find(unifid);
			if(ait != _apply_map.end())
			{
				chosen_list.push_back(ait->first);
			}
			else
			{
				Log::log(LOG_ERR, "[func=%s]apply faction not found in faction map [unifid=%lld]", __FUNCTION__, ait->first);
			}
			-- num;
			candidates.erase(candidates.begin() + index);
		}
	}

	DEBUG_PRINT("file=%s\tfunc=%s\tchosen_list:size=%d\n", __FILE__, __FUNCTION__, chosen_list.size());
	for(std::vector<int64_t>::iterator it = chosen_list.begin(); it != chosen_list.end(); ++ it)
	{
		DEBUG_PRINT("%lld\t", *it);
	}
}

int CDC_MNFactionBattleMan::CheckMNFApply(unsigned int fid, int64_t unifid, unsigned char target)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
	if(DisabledSystem::GetDisabled(SYS_FORBIDCROSS)) return ERR_MNF_WRONG_TIME;
	if(STATE_APPLY_BEGIN != _state) return ERR_MNF_WRONG_TIME;
	if(_apply_map.find(unifid) != _apply_map.end()) return ERR_MNF_APPLY_DUNPLICATE; 

	for(FACTION_MAP::iterator it = _faction_map.begin(); it != _faction_map.end(); ++ it)
	{
		if(it->second.fid == fid)
		{
			if(_apply_map.find(it->first) != _apply_map.end())
			{
				return ERR_MNF_APPLY_DUNPLICATE;
			}
		}
	}

	//这个帮派是否占有这种类型的所有领地
	if(unifid != 0)
	{
		bool is_all_domain_owner = true;
		for(DOMAIN_MAP::iterator dit = _domain_map.begin(); dit != _domain_map.end(); ++ dit)
		{
			if(dit->second.domain_type == target && dit->second.owner_unifid != unifid)
			{
				is_all_domain_owner = false;
				break;
			}
		}
		if(is_all_domain_owner) return ERR_MNF_APPLY_TYPE; 
	}

	switch(target)
	{
		case DOMAIN_TYPE_A:
			{
				FACTION_MAP::iterator it = _faction_map.find(unifid);
				if(it == _faction_map.end()) return ERR_MNF_CROSS_DOMAIN_NOTENOUGH;
				if(it->second.domain_num.size() != DOMAIN_TYPE_COUNT || it->second.domain_num[DOMAIN_TYPE_C] <= 0 || it->second.domain_num[DOMAIN_TYPE_B] <= 0)
				{
					return ERR_MNF_CROSS_DOMAIN_NOTENOUGH;
				}
			}
			break;

		case DOMAIN_TYPE_B:
			{
				FACTION_MAP::iterator it = _faction_map.find(unifid);
				if(it == _faction_map.end()) return ERR_MNF_CROSS_DOMAIN_NOTENOUGH;
				if(it->second.domain_num[DOMAIN_TYPE_C] <= 0 && it->second.domain_num[DOMAIN_TYPE_B] <= 0)
				{
					return ERR_MNF_CROSS_DOMAIN_NOTENOUGH;
				}
			}
			break;

		case DOMAIN_TYPE_C:
			{
				FACTION_MAP::iterator it = _faction_map.find(unifid);
				unsigned int count = GetDomainCount(fid);
				if(it == _faction_map.end())
				{
					if(count < _domain_count_lvl1) return ERR_MNF_DOMAIN_NOTENOUGH;
				}
				else
				{
					if(count < _domain_count_lvl1 &&
						it->second.domain_num[DOMAIN_TYPE_C] <= 0 && it->second.domain_num[DOMAIN_TYPE_B] <= 0)
					{
						return ERR_MNF_DOMAIN_NOTENOUGH;
					}
				}
			}
			break;

		default:
			return ERR_MNF_APPLY_TYPE;
	}

	DEBUG_PRINT("file=%s\tfunc=%s\tline=%dERR_SUCCESS\n", __FILE__, __FUNCTION__, __LINE__);
	return ERR_SUCCESS;
}

unsigned int CDC_MNFactionBattleMan::GetDomainCount(unsigned int fid)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
	unsigned int count = 0;
	BattleManager::TVector::iterator it = BattleManager::GetInstance()->cities.begin();
	BattleManager::TVector::iterator ite = BattleManager::GetInstance()->cities.end();
	for(;it != ite; ++ it)
	{
		if(it->owner == fid) ++ count;
	}

	return count;
}

void CDC_MNFactionBattleMan::FetchCDSFiltrateRes()
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
	if(_apply_map.size() > 0)
	{
		MNFetchFiltrateResult pro;
		pro.zoneid = GDeliveryServer::GetInstance()->GetZoneid();
		CentralDeliveryClient::GetInstance()->SendProtocol(pro);
	}
	else
	{
		_is_get_cds_filtrate_res = true;
	}
}

void CDC_MNFactionBattleMan::NotifyDBFiltrateRes(std::vector<int64_t>& chosen_list)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
	std::vector<int64_t> rejected_list;
	for(APPLYINFO_MAP::iterator it = _apply_map.begin(); it != _apply_map.end(); ++ it)
	{
		if(std::find(chosen_list.begin(), chosen_list.end(), it->first) == chosen_list.end())
		{
			rejected_list.push_back(it->first);
		}
	}
	DBMNFactionApplyResNotifyArg arg;
	arg.status = SENDER_APPLY_REJECTED_CLIENT;
	arg.rejected_list = rejected_list;
	GameDBClient::GetInstance()->SendProtocol(Rpc::Call(RPC_DBMNFACTIONAPPLYRESNOTIFY, arg));
}

void CDC_MNFactionBattleMan::OnGetCDSFiltrateResult(std::vector<int64_t>& chosen_list)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
	if(!_is_get_cds_filtrate_res)
	{
		DBMNFactionApplyResNotifyArg arg;
		arg.status = SENDER_APPLY_REJECTED_SERVER;
		arg.chosen_list = chosen_list;
		for(APPLYINFO_MAP::iterator it = _apply_map.begin(); it != _apply_map.end(); ++ it)
		{
			if(std::find(chosen_list.begin(), chosen_list.end(), it->first) == chosen_list.end())
			{
				arg.rejected_list.push_back(it->first); 
			}
		}

		GameDBClient::GetInstance()->SendProtocol(Rpc::Call(RPC_DBMNFACTIONAPPLYRESNOTIFY, arg));
		_is_get_cds_filtrate_res = true;

		DEBUG_PRINT("file=%s\tchosen_list:size=%d\n", __FILE__, arg.chosen_list.size()); 
		for(std::vector<int64_t>::iterator it = arg.chosen_list.begin(); it != arg.chosen_list.end(); ++ it)
		{
			DEBUG_PRINT("chosen unifid=%lld", *it);
		}
		DEBUG_PRINT("file=%s\trejected_list:size=%d\n", __FILE__, arg.rejected_list.size()); 
		for(std::vector<int64_t>::iterator it = arg.rejected_list.begin(); it != arg.rejected_list.end(); ++ it)
		{
			DEBUG_PRINT("rejected unifid=%lld", *it);
		}
	}
}

void CDC_MNFactionBattleMan::OnNotifyDBApplyResSuccess(unsigned char status, std::vector<int64_t>& rejected_list)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
	if(status == SENDER_APPLY_REJECTED_CLIENT)
	{
		for(std::vector<int64_t>::iterator it = rejected_list.begin(); it != rejected_list.end(); ++ it)
		{
			APPLYINFO_MAP::iterator ait = _apply_map.find(*it);
			if(ait != _apply_map.end())
			{
				_apply_map.erase(ait);
			}
		}
		_is_notify_db_apply_res = true;
	}
	else if(status == SENDER_APPLY_REJECTED_SERVER)
	{
		_apply_map.erase(_apply_map.begin(), _apply_map.end());
	}
	
	SaveForClientApplyData();
	DebugDumpApplyInfo(__FUNCTION__);
}

void CDC_MNFactionBattleMan::UpdateMNFApplyInfo(MNFactionApplyInfo& applyinfo)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
	_apply_map[applyinfo.unifid] = applyinfo;
	SaveForClientApplyData(); 
	DebugDumpApplyInfo(__FUNCTION__);
}

int CDC_MNFactionBattleMan::UpdateCandidatesFaction(unsigned int fid, int64_t unifid, std::vector<int64_t>& candidates_lvl1, std::vector<int64_t>& candidates_lvl2, std::vector<int64_t>& candidates_lvl3)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
	FACTION_MAP::iterator it = _faction_map.find(unifid);
	if(it->second.domain_num[DOMAIN_TYPE_C] > 0 || it->second.domain_num[DOMAIN_TYPE_B] > 0) return 1;

	unsigned int domain_count = GetDomainCount(fid);
	if(domain_count >= _domain_count_lvl1 && domain_count < _domain_count_lvl2)
	{
		candidates_lvl1.push_back(unifid);
	}
	else if(domain_count >= _domain_count_lvl2 && domain_count < _domain_count_lvl3)
	{
		candidates_lvl2.push_back(unifid);
	}
	else if(domain_count >= _domain_count_lvl3)
	{
		candidates_lvl3.push_back(unifid);
	}

	return 0;
}

void CDC_MNFactionBattleMan::OnMNFactionPlayerCross(int64_t unifid, int roleid, bool backflag)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\tunifid=%lld\troleid=%d\tbackflag=%d\n", __FILE__, __FUNCTION__, __LINE__, unifid, roleid, backflag); 

	FACTION_MAP::iterator fit = _faction_map.find(unifid);
	if(fit == _faction_map.end())
	{
		DEBUG_PRINT("CDC_MNFactionBattleMan[%s]ERR:%d\n", __FUNCTION__,__LINE__);
		return;
	}

	std::vector<int>::iterator rit = std::find(fit->second.cross_rolelist.begin(), 
			fit->second.cross_rolelist.end(), roleid);

	if(backflag)
	{
		//跨服到原服
		if(rit == fit->second.cross_rolelist.end())
		{
			Log::log(LOG_ERR, "CDC_MNFactionBattleMan cds->cdc player not in cross_rolelist[%s]ERR:%d\n", __FUNCTION__,__LINE__);
			return;
		}
		fit->second.cross_rolelist.erase(rit);
	}
	else
	{
		//原服到跨服
		if(rit != fit->second.cross_rolelist.end())
		{
			Log::log(LOG_ERR, "CDC_MNFactionBattleMan cdc->cds player in cross_rolelist[%s]ERR:%d\n", __FUNCTION__,__LINE__);
			return;
		}
		fit->second.cross_rolelist.push_back(roleid);
	}
	return;
}

int CDC_MNFactionBattleMan::CheckMNFactionPlayerCross(int64_t unifid)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\tunifid=%lld\n", __FILE__, __FUNCTION__, __LINE__, unifid);

	FACTION_MAP::iterator fit = _faction_map.find(unifid);
	if(fit == _faction_map.end() || unifid == 0)
	{
		DEBUG_PRINT("CDC_MNFactionBattleMan[%s]ERR:%d\n", __FUNCTION__,__LINE__);
		return ERR_MNF_FACTION_NOT_INVITED;
	}
	
	if(fit->second.accept_sn != _sn)
	{
		DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\tunifid=%lld\tretcode=%d\n", __FILE__, __FUNCTION__, __LINE__, unifid, ERR_MNF_FACTION_NOT_INVITED);
		return ERR_MNF_FACTION_NOT_INVITED;
	}

	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\tunifid=%lld\tcross_rolelist.size=%d\tinvite_count=%d\n", __FILE__, __FUNCTION__, __LINE__, unifid, fit->second.cross_rolelist.size(), fit->second.invite_count);
	if(fit->second.cross_rolelist.size() >= fit->second.invite_count)
	{
		DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\tunifid=%lld\tretcode=%d\n", __FILE__, __FUNCTION__, __LINE__, unifid, ERR_MNF_CROSS_MAXMUM);
		return ERR_MNF_CROSS_MAXMUM;
	}

	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\tunifid=%lld\tretcode=%d\n", __FILE__, __FUNCTION__, __LINE__, unifid, ERR_SUCCESS);
	return ERR_SUCCESS;
}

void CDC_MNFactionBattleMan::OnRecvBonusData(MNDomainBonus& bonus) 
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
	DBMNSendBattleBonusArg arg;
	arg.bonus = bonus;
	GameDBClient::GetInstance()->SendProtocol(Rpc::Call(RPC_DBMNSENDBATTLEBONUS, arg));
}

void CDC_MNFactionBattleMan::SendClientFactionInfo(int roleid, int64_t unifid)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
	PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline(roleid);
	if(pinfo == NULL) return;

	unsigned int linksid = pinfo->linksid;
	unsigned int localsid = pinfo->localsid;
	MNGetFactionInfo_Re re;

	FACTION_MAP::iterator fit = _faction_map.find(unifid);
	if(fit == _faction_map.end())
	{
		re.retcode = ERR_MNF_FACTION_NOT_FOUND;	
	}
	else
	{
		re.retcode				= ERR_SUCCESS;
		MNFactionInfo info;
		info.unifid				= fit->second.unifid;
		info.fid				= fit->second.fid;
		info.faction_name		= fit->second.faction_name;
		info.master_name		= fit->second.master_name;
		info.zoneid				= fit->second.zoneid;
		info.credit				= fit->second.credit;
		info.credit_this_week	= fit->second.credit_this_week;
		info.credit_get_time	= fit->second.credit_get_time;
		info.invite_count		= fit->second.invite_count;
		info.accept_sn			= fit->second.accept_sn;
		info.bonus_sn			= fit->second.bonus_sn;
		info.version			= fit->second.version;
		info.domain_num.reserve(3);
		info.domain_num.push_back(0);
		info.domain_num.push_back(0);
		info.domain_num.push_back(0);
		re.mnfactioninfo		= info;
	}
	re.localsid = localsid;
	GDeliveryServer::GetInstance()->Send(linksid, re);
}

void CDC_MNFactionBattleMan::SendClientDomainData(int roleid)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\tdomain size=%d\tapply_size=%d\n", __FILE__, __FUNCTION__, __LINE__, _domain_data.size(), _apply_data.size());
	PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline(roleid);
	if(pinfo == NULL) return;

	unsigned int linksid = pinfo->linksid;
	unsigned int localsid = pinfo->localsid;
	MNGetDomainData_Re re;
	time_t now = GetTime();
	struct tm dt;
	localtime_r(&now, &dt);
	int wday = (dt.tm_wday == 0)? (7):(dt.tm_wday);
	int time_in_a_week = wday * 24 * 3600
		+ dt.tm_hour * 3600
		+ dt.tm_min * 60
		+ dt.tm_sec;

	if(time_in_a_week >= _cross_begin_time)
	{
		re.retcode 	= ERR_SUCCESS;
	}
	else
	{
		re.retcode	= 1; //不显示对手,只显示报名结果
	}

	re.domain_data	= _domain_data;
	re.apply_data	= _apply_data;
	re.localsid		= localsid;
	GDeliveryServer::GetInstance()->Send(linksid, re);
}

void CDC_MNFactionBattleMan::SaveForClientDomainData()
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
	_domain_data.clear();
	for(DOMAIN_MAP::iterator it = _domain_map.begin(); it != _domain_map.end(); ++it)
	{
		MNDomainData data;
		data.domain_id = it->second.domain_id;
		data.domain_type = it->second.domain_type;

		int64_t owner = it->second.owner_unifid;
		FACTION_MAP::iterator fit = _faction_map.find(owner);
		if(fit != _faction_map.end())
		{
			data.owner_unifid	= fit->second.unifid;
			data.faction_name	= fit->second.faction_name;
			data.owner_rank		= CDC_MNToplistMan::GetInstance()->GetMNFactionRank(fit->second.fid, fit->second.zoneid);
			data.master_name	= fit->second.master_name;
			data.zoneid			= fit->second.zoneid;
			data.credit			= fit->second.credit;
		}
		else
		{
			data.owner_unifid 	= 0;
			data.faction_name	= Octets();
			data.owner_rank		= 0; 
			data.master_name    = Octets();
			data.zoneid			= 0;
			data.credit			= 0;
		}

		int64_t attacker = it->second.attacker_unifid;
		FACTION_MAP::iterator attacker_fit = _faction_map.find(attacker);
		if(attacker_fit != _faction_map.end())
		{
			data.attacker_unifid	= attacker_fit->second.unifid;
			data.attacker_name		= attacker_fit->second.faction_name;
			data.attacker_zoneid	= attacker_fit->second.zoneid;
			data.attacker_rank		= CDC_MNToplistMan::GetInstance()->GetMNFactionRank(attacker_fit->second.fid, attacker_fit->second.zoneid);
		}
		else
		{
			data.attacker_unifid 	= 0;
			data.attacker_name		= Octets();
			data.attacker_zoneid	= 0;
			data.attacker_rank		= 0;
		}

		int64_t defender = it->second.defender_unifid;
		FACTION_MAP::iterator defender_fit = _faction_map.find(defender);
		if(defender_fit != _faction_map.end())
		{
			data.defender_unifid	= defender_fit->second.unifid;
			data.defender_name		= defender_fit->second.faction_name;
			data.defender_zoneid	= defender_fit->second.zoneid;
			data.defender_rank		= CDC_MNToplistMan::GetInstance()->GetMNFactionRank(defender_fit->second.fid, defender_fit->second.zoneid);
		}
		else
		{
			data.defender_unifid	= 0;
			data.defender_name		= Octets();
			data.defender_zoneid	= 0;
			data.defender_rank		= 0; 
		}

		_domain_data.push_back(data);
	}
	DebugDumpDomainData(__FUNCTION__, __LINE__);
}

void CDC_MNFactionBattleMan::SaveForClientApplyData()
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
	_apply_data.clear();
	for(APPLYINFO_MAP::iterator it = _apply_map.begin(); it != _apply_map.end(); ++ it)
	{
		MNFactionApplyData data;
		int64_t unifid = it->second.unifid;
		FACTION_MAP::iterator fit = _faction_map.find(unifid);
		if(fit != _faction_map.end())
		{
			data.fid = fit->second.fid;
		}
		data.dest = it->second.dest;
		_apply_data.push_back(data);
	}
	DebugDumpApplyData(__FUNCTION__, __LINE__);
}

void CDC_MNFactionBattleMan::SendMNFactionBriefInfo(unsigned int fid, int zoneid, int roleid)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
	PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline(roleid);
	if(pinfo == NULL) return;

	unsigned int linksid = pinfo->linksid;
	unsigned int localsid = pinfo->localsid;
	MNGetFactionBriefInfo_Re re;
	re.retcode = ERR_MNF_FACTION_NOT_FOUND;
	for(FACTION_MAP::iterator it = _faction_map.begin(); it != _faction_map.end(); ++ it)
	{
		if(it->second.fid == fid && it->second.zoneid == zoneid)	
		{
			re.retcode				= ERR_SUCCESS;
			re.rank					= CDC_MNToplistMan::GetInstance()->GetMNFactionRank(fid, zoneid);
			if(is_same_week(it->second.credit_get_time, Timer::GetTime()))
			{
				re.credit_this_week	= it->second.credit_this_week;
			}
			else
			{
				re.credit_this_week = 0;
			}
			re.info.fid				= it->second.fid;
			re.info.faction_name	= it->second.faction_name;
			re.info.master_name		= it->second.master_name;
			re.info.zoneid			= it->second.zoneid;
			re.info.credit			= it->second.credit;
		}
	}

	re.localsid = localsid;
	GDeliveryServer::GetInstance()->Send(linksid, re);
}

inline bool CDC_MNFactionBattleMan::is_same_week(time_t t1, time_t t2) 
{ 
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
	if(t1 < t2) std::swap(t1, t2);

	struct tm tm1; 
	struct tm tm2;
	localtime_r(&t1, &tm1);
	localtime_r(&t2, &tm2);

	time_t zone = 0;
	struct tm begintm; 
	localtime_r(&zone, &begintm);
	int delta = (begintm.tm_wday)*86400 + begintm.tm_gmtoff;
	return abs((t1 - delta)/(86400*7) - (t2 - delta)/(86400*7)) == 0;
} 

void CDC_MNFactionBattleMan::DebugDumpDomainInfo(const char* func, MNDomainInfoVector& domaininfo_list)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tdomaininfo_list:size=%d\n", __FILE__, func, domaininfo_list.size());
	for(MNDomainInfoVector::iterator it = domaininfo_list.begin(); it != domaininfo_list.end(); ++ it)
	{
		DEBUG_PRINT("domain_id=%d,\tdomain_type=%d,\towner_unifid=%lld,\tattacker_unifid=%lld,\tdefender_unifid=%lld,\n",it->domain_id, it->domain_type, it->owner_unifid, it->attacker_unifid, it->defender_unifid);
	}
	
}

void CDC_MNFactionBattleMan::DebugDumpDomainInfo(const char* func)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tdomaininfo_map:size=%d\n", __FILE__, func, _domain_map.size());
	for(DOMAIN_MAP::iterator it = _domain_map.begin(); it != _domain_map.end(); ++ it)
	{
		DEBUG_PRINT("domain_id=%d,\tdomain_type=%d,\towner_unifid=%lld,\tattacker_unifid=%lld,\tdefender_unifid=%lld,\n",it->second.domain_id, it->second.domain_type, it->second.owner_unifid, it->second.attacker_unifid, it->second.defender_unifid);
	}
}

void CDC_MNFactionBattleMan::DebugDumpFactionInfo(const char* func, MNFactionInfoVector& factioninfo_list)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tfactioninfo_list:size=%d\n", __FILE__, func, factioninfo_list.size());
	for(MNFactionInfoVector::iterator it = factioninfo_list.begin(); it != factioninfo_list.end(); ++ it)
	{
		DEBUG_PRINT("unifid=%lld,\tfid=%d,\tzoneid=%d,\tdomaina=%d,\tdomainb=%d,\tdomainc=%d,\tcredit=%d,\t,credit_this_week=%d,\tcredit_get_time=%d,\tinvite_count=%d,\taccept_sn=%d,\tbonus_sn=%d,\tversion=%d,\n", it->unifid, it->fid, it->zoneid, it->domain_num[0], it->domain_num[1], it->domain_num[2], it->credit, it->credit_this_week, it->credit_get_time, it->invite_count, it->accept_sn, it->bonus_sn, it->version);
	}
			
}

void CDC_MNFactionBattleMan::DebugDumpFactionInfo(const char* func)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tfactioninfo_map:size=%d\n", __FILE__, func, _faction_map.size());
	for(FACTION_MAP::iterator it = _faction_map.begin(); it != _faction_map.end(); ++ it)
	{
		DEBUG_PRINT("unifid=%lld,\tfid=%d,\tzoneid=%d,\tdomaina=%d,\tdomainb=%d,\tdomainc=%d,\tcredit=%d,\t,credit_this_week=%d,\tcredit_get_time=%d,\tinvite_count=%d,\taccept_sn=%d,\tbonus_sn=%d,\tversion=%d,\trolelist size=%d\n", it->second.unifid, it->second.fid, it->second.zoneid, it->second.domain_num[0], it->second.domain_num[1], it->second.domain_num[2], it->second.credit, it->second.credit_this_week, it->second.credit_get_time, it->second.invite_count, it->second.accept_sn, it->second.bonus_sn, it->second.version, it->second.cross_rolelist.size());
	}
}

void CDC_MNFactionBattleMan::DebugDumpApplyInfo(const char* func, MNFactionApplyInfoVector& applyinfo_list)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tapplyinfo_list:size=%d\n", __FILE__, func, applyinfo_list.size());
	for(MNFactionApplyInfoVector::iterator it = applyinfo_list.begin(); it != applyinfo_list.end(); ++ it)
	{
		DEBUG_PRINT("unifid=%lld,\tapplicant_id=%d,\tdest=%d,\tcost=%d,\n", it->unifid, it->applicant_id, it->dest, it->cost);
	}
}

void CDC_MNFactionBattleMan::DebugDumpApplyInfo(const char* func)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tapplyinfo_map:size=%d\n", __FILE__, func, _apply_map.size());
	for(APPLYINFO_MAP::iterator it = _apply_map.begin(); it != _apply_map.end(); ++ it)
	{
		DEBUG_PRINT("unifid=%lld,\tapplicant_id=%d,\tdest=%d,\tcost=%d,\n", it->second.unifid, it->second.applicant_id, it->second.dest, it->second.cost);
	}
}

void CDC_MNFactionBattleMan::DebugDumpUnifid(const char* str, std::vector<int64_t>& list)
{
	DEBUG_PRINT("file=%s\tstr=%s\tlist:size=%d\n", __FILE__, str, list.size());
	for(std::vector<int64_t>::iterator it = list.begin(); it != list.end(); ++ it)
	{
		DEBUG_PRINT("unifid=%lld", *it);
	}
}

void CDC_MNFactionBattleMan::DebugDumpFlag(const char* func, int line)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\tflag:\n", __FILE__, func, line);
	DEBUG_PRINT("_sn=%d,\t", _sn);
	DEBUG_PRINT("_state=%d,\t", _state);
	DEBUG_PRINT("_is_filtrated_cdc=%d,\t", _is_filtrated_cdc);
	DEBUG_PRINT("_is_need_update_battle_cache=%d,\t", _is_need_update_battle_cache);
	DEBUG_PRINT("_is_notify_db_apply_res=%d,\t", _is_notify_db_apply_res);
	DEBUG_PRINT("_is_send_proclaim_success=%d,\t", _is_send_proclaim_success);
	DEBUG_PRINT("_is_get_cds_filtrate_res=%d,\t", _is_get_cds_filtrate_res);
	DEBUG_PRINT("_is_get_cds_faction_toplist=%d,\n", _is_get_cds_faction_toplist);
	DEBUG_PRINT("_apply_begin_time=%d,\n", _apply_begin_time);
	DEBUG_PRINT("_apply_end_time=%d,\n", _apply_end_time);
	DEBUG_PRINT("_fetch_filtrate_res_time=%d,\n", _fetch_filtrate_res_time);
	DEBUG_PRINT("_cross_begin_time=%d,\n", _cross_begin_time);
	DEBUG_PRINT("_cross_end_time=%d,\n", _cross_end_time);
	DEBUG_PRINT("_close_time=%d,\n", _close_time);
}

void CDC_MNFactionBattleMan::DebugDumpDomainData(const char* func, int line)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\tflag:\n", __FILE__, func, line);
	for(MNDomainDataVector::iterator it = _domain_data.begin(); it != _domain_data.end(); ++ it)
	{
		DEBUG_PRINT("domain_id=%d,\tdomain_type=%d,\towner=%lld,\trank=%d,\tzoneid=%d,\tcredit=%d,\tattacker=%lld,\tattacker_rank=%d,\tattacker_zoneid=%d,\tdefender=%lld,\tdefender_rank%d,\tdefender_zoneid=%d\n", it->domain_id, it->domain_type, it->owner_unifid, it->owner_rank, it->zoneid, it->credit, it->attacker_unifid, it->attacker_rank, it->attacker_zoneid, it->defender_unifid, it->defender_rank, it->defender_zoneid);
	}
}

void CDC_MNFactionBattleMan::DebugDumpApplyData(const char* func, int line)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\tflag:\n", __FILE__, func, line);
	for(MNFactionApplyDataVector::iterator it = _apply_data.begin(); it != _apply_data.end(); ++ it)
	{
		DEBUG_PRINT("fid=%d\tdest=%d\n", it->fid, it->dest);
	}
}

void CDC_MNFactionBattleMan::DebugFiltrateApplyInfo(MNFactionInfo& factioninfo, MNFactionApplyInfo& applyinfo)
{
	//更新中心服的faction和domain信息，只是作为测试 todo
}

void CDC_MNFactionBattleMan::Clear()
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
	_is_filtrated_cdc = false;
	_is_need_update_battle_cache = true;
	_is_notify_db_apply_res = false;
	_is_send_proclaim_success = false;
	_is_get_cds_filtrate_res = false;
	_is_get_cds_faction_toplist = false;
	CDC_MNToplistMan::GetInstance()->EnableNeedFetchToplistFlag();
	for(APPLYINFO_MAP::iterator it = _apply_map.begin(); it != _apply_map.end(); ++ it)
	{
		Log::log(LOG_ERR, "CDC_MNFactionBattleMan [%s] applymap: unifid=%lld,appid=%d,dest=%d,cost=%d\n", __FUNCTION__, it->second.unifid, it->second.applicant_id, it->second.dest, it->second.cost);
	}
	
	_apply_map.clear();
	_apply_data.clear();

	for(FACTION_MAP::iterator it = _faction_map.begin(); it != _faction_map.end(); ++ it)
	{
		it->second.cross_rolelist.clear();
	}
}

void CDC_MNFactionBattleMan::SetAdjustTime(int offset)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
	time_t now = Timer::GetTime();
	struct tm dt;
	localtime_r(&now, &dt);
	int wday = (dt.tm_wday == 0)? (7):(dt.tm_wday);
	int time_in_a_week = wday * 24 * 3600
		+ dt.tm_hour * 3600
		+ dt.tm_min * 60
		+ dt.tm_sec;
	
	_adjust_time = _apply_begin_time - time_in_a_week + offset;
}

void CDC_MNFactionBattleMan::SetState(int state)
{
	if(state == STATE_APPLY_BEGIN)
	{
		SetAdjustTime(0);
	}
	else if(state == STATE_APPLY_END)
	{
		SetAdjustTime(_apply_end_time - _apply_begin_time);
	}
	else if(state == STATE_FETCH_CDS_FILTRATE_RES)
	{
		SetAdjustTime(_fetch_filtrate_res_time - _apply_begin_time);
	}
	else if(state == STATE_CROSS_BEGIN)
	{
		SetAdjustTime(_cross_begin_time - _apply_begin_time);
	}
	else if(state == STATE_CROSS_END)
	{
		SetAdjustTime(_cross_end_time - _apply_begin_time);
	}
	else if(state == STATE_CLOSE)
	{
		SetAdjustTime(_close_time - _apply_begin_time);
	}
	else
	{
		return;
	}

	if(state == 0)
	{
		_state = STATE_CROSS_END;
	}
	else
	{
		_state = state - 1;
	}
}

time_t CDC_MNFactionBattleMan::GetTime()
{
#define MNF_DEBUG
#ifdef MNF_DEBUG
	// now = 活动开启的时间 + offset
	time_t now = Timer::GetTime() + _adjust_time;
#else
	time_t now = Timer::GetTime();
#endif
	return now;
}

void CDC_MNToplistMan::OnGetCDSToplist(MNFactionBriefInfoVector& toplist)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
	_toplist = toplist;
	//客户端数据包含排行榜
	CDC_MNFactionBattleMan::GetInstance()->SaveForClientDomainData();
	_need_fetch_toplist_flag = false; 
	DebugDumpToplist(__FUNCTION__);
}

int CDC_MNToplistMan::GetMNFactionRank(unsigned int fid, int zoneid)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
	int index = 1;
	for(MNFactionBriefInfoVector::iterator it = _toplist.begin(); it != _toplist.end(); ++ it, ++ index)
	{
		if(it->fid == fid && it->zoneid == zoneid) return index; 
	}

	return 0;
}

/*
int CDC_MNToplistMan::GetMNFactionRank(int64_t unifid)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%dunifid=%lld\n", __FILE__, __FUNCTION__, __LINE__, unifid);
	int index = 1;
	for(MNFactionBriefInfoVector::iterator it = _toplist.begin(); it != _toplist.end(); ++ it, ++ index)
	{
		if(it->unifid == unifid)  return index;
	}

	return 0;

}
*/

void CDC_MNToplistMan::GetMNToplist()
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
	if(CDC_MNToplistMan::GetInstance()->NeedFetchToplist())
	{
		MNFetchTopList pro;
		pro.zoneid = GDeliveryServer::GetInstance()->GetZoneid();
		CentralDeliveryClient::GetInstance()->SendProtocol(pro);
	}
}

void CDC_MNToplistMan::SendClientToplist(int roleid)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
	PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline(roleid);
	if(pinfo == NULL) return;

	unsigned int linksid = pinfo->linksid;
	unsigned int localsid = pinfo->localsid;
	MNGetTopList_Re re;
	re.retcode	= ERR_SUCCESS;
	re.toplist	= _toplist;
	re.localsid	= localsid;
	GDeliveryServer::GetInstance()->Send(linksid, re);
}

void CDC_MNToplistMan::DebugDumpToplist(const char* func)
{
	DEBUG_PRINT("file=%s\tfunc=%s\ttoplist:size=%d\n", __FILE__, func, _toplist.size());
	for(MNFactionBriefInfoVector::iterator it = _toplist.begin(); it != _toplist.end(); ++ it)
	{
		DEBUG_PRINT("fid=%d,\tzoneid=%d,\tcredit=%d\n", it->fid, it->zoneid, it->credit);
	}
}
