#include "cdsmnfbattleman.h"
#include "mnfactioncacheget_re.hpp"
#include "centraldeliveryserver.hpp"
#include "dbmnfactioninfoupdate.hrp"
#include "gamedbclient.hpp"
#include "mndomainbattlestart.hpp"
#include "gproviderserver.hpp"
#include "mndomainbattleenter_re.hpp"
#include "mapuser.h"
#include "mndomainsendbonusdata.hpp"
#include "dbmnputbattlebonus.hrp"
#include "mnfetchfiltrateresult_re.hpp"
#include "mnfetchtoplist_re.hpp"
#include "dbmnsendbonusnotify.hrp"
#include "dbmnfactionapplyinfoput.hrp"
#include "dbmndomaininfoupdate.hrp"
#include "mngetplayerlastenterinfo_re.hpp"
#include "dbmnfactionstateupdate.hrp"
#include "crosssystem.h"
#include "mnfactionproclaim_re.hpp"
#include "mndomaininfogsnotice.hpp"
#include "dbmnfactioninfoget.hrp"
#include "mngetfactioninfo_re.hpp"

void CDS_MNFactionBattleMan::BattleDomainInfo::Init(int domain_id_, unsigned char domain_type_, int64_t owner_, int64_t attacker_, int64_t defender_)
{
	domain_id					= domain_id_;
	domain_type					= domain_type_;
	owner_unifid				= owner_;
	attacker_unifid				= attacker_;
	defender_unifid				= defender_;
	enter_domain_num_defender	= 0;
	enter_domain_num_attacker	= 0;
	has_enter_domain_num_defender	= 0;
	has_enter_domain_num_attacker	= 0;
}

void CDS_MNFactionBattleMan::FactionDomainCredit::Init(int credit_a, int credit_b, int credit_c)
{
	credit_domain_a = credit_a;
	credit_domain_b = credit_b;
	credit_domain_c = credit_c;
}

bool CDS_MNFactionBattleMan::LoadConfig()
{
	std::string key = "MNFBATTLE";
	Conf* conf = Conf::GetInstance();
	int day = 0 ,hour = 0, min = 0, sec = 0;

	std::string apply_begin_time_str = conf->find(key, "apply_begin_time");
	sscanf(apply_begin_time_str.c_str(), "[%d]%d:%d:%d", &day, &hour, &min, &sec);
	_apply_begin_time = day * 24 * 3600 + hour * 3600 + min * 60 + sec;

	std::string apply_end_time_str = conf->find(key, "apply_end_time");
	sscanf(apply_end_time_str.c_str(), "[%d]%d:%d:%d", &day, &hour, &min, &sec);
	int apply_end_time = day * 24 * 3600 + hour * 3600 + min * 60 + sec;
	_apply_filtrated_begin_time = apply_end_time;
	_apply_filtrated_end_time	= apply_end_time + TIME_ADJUST_FILTRATE_END_TO_BEGIN;

	std::string cross_begin_time_str = conf->find(key, "cross_begin_time");
	sscanf(cross_begin_time_str.c_str(), "[%d]%d:%d:%d", &day, &hour, &min, &sec);
	_cross_begin_time = day * 24 * 3600 + hour * 3600 + min * 60 + sec;
	_cross_day = day;

	//battle_begin_time这个时间应该设置的稍早一点
	std::string battle_begin_time_str = conf->find(key, "battle_begin_time");
	sscanf(battle_begin_time_str.c_str(), "[%d]%d:%d:%d", &day, &hour, &min, &sec);
	_battle_begin_time = day * 24 * 3600 + hour * 3600 + min * 60 + sec;

	std::string battle_end_time_str = conf->find(key, "battle_end_time");
	sscanf(battle_end_time_str.c_str(), "[%d]%d:%d:%d", &day, &hour, &min, &sec);
	_battle_end_time = day * 24 * 3600 + hour * 3600 + min * 60 + sec;

	_bonus_time = _battle_end_time + TIME_ADJUST_BONUS_TO_BATTLEEND;
	_close_time = _bonus_time + TIME_ADJUST_CLOSE_TO_BONUS;

	if(!(_apply_filtrated_begin_time <= _apply_filtrated_end_time && _apply_filtrated_end_time <= _cross_begin_time && _cross_begin_time <= _battle_begin_time
		&& _battle_begin_time <= _battle_end_time && _battle_end_time <= _bonus_time && _bonus_time <= _close_time))
	{
		Log::log(LOG_ERR,"config time err!");	
		return false;
	}

	int item_id = 0, item_num = 0, max_count = 0, proc_type = 0;
	std::string item_a = conf->find(key, "bonus_item_a");
	sscanf(item_a.c_str(), "%d:%d:%d:%d", &item_id, &item_num, &max_count, &proc_type);
	if (item_id <= 0 || item_num <= 0 || max_count <= 0 || proc_type < 0/*proc_type可能为0*/)
	{
		Log::log(LOG_ERR, "item A config err"); 
		return false;
	}
	DEBUG_PRINT("CDS_MNFactionBattleMan:func=%s,item A item_id=%d,item_num=%d,max_count=%d,proc_type=%d\n",__FUNCTION__, item_id, item_num, max_count, proc_type);
	_item_a.item_id		= item_id;
	_item_a.item_num	= item_num;
	_item_a.max_count	= max_count;
	_item_a.proc_type	= proc_type;

	std::string item_b = conf->find(key, "bonus_item_b");
	sscanf(item_b.c_str(), "%d:%d:%d:%d", &item_id, &item_num, &max_count, &proc_type);
	if (item_id <= 0 || item_num <= 0 || max_count <= 0 || proc_type < 0/*proc_type可能为0*/)
	{
		Log::log(LOG_ERR, "item B config err"); 
		return false;
	}
	DEBUG_PRINT("CDS_MNFactionBattleMan:func=%s,item B item_id=%d,item_num=%d,max_count=%d,proc_type=%d\n",__FUNCTION__, item_id, item_num, max_count, proc_type);
	_item_b.item_id		= item_id;
	_item_b.item_num	= item_num;
	_item_b.max_count	= max_count;
	_item_b.proc_type	= proc_type;

	std::string item_c = conf->find(key, "bonus_item_c");
	sscanf(item_c.c_str(), "%d:%d:%d:%d", &item_id, &item_num, &max_count, &proc_type);
	if (item_id <= 0 || item_num <= 0 || max_count <= 0 || proc_type < 0/*proc_type可能为0*/)
	{
		Log::log(LOG_ERR, "item C config err"); 
		return false;
	}
	DEBUG_PRINT("CDS_MNFactionBattleMan:func=%s,item C item_id=%d,item_num=%d,max_count=%d,proc_type=%d\n",__FUNCTION__, item_id, item_num, max_count, proc_type);
	_item_c.item_id		= item_id;
	_item_c.item_num	= item_num;
	_item_c.max_count	= max_count;
	_item_c.proc_type	= proc_type;

	std::string item_master = conf->find(key, "bonus_item_master");
	sscanf(item_master.c_str(), "%d:%d:%d:%d", &item_id, &item_num, &max_count, &proc_type);
	if (item_id <= 0 || item_num <= 0 || max_count <= 0 || proc_type < 0/*proc_type可能为0*/)
	{
		Log::log(LOG_ERR, "item master config err"); 
		return false;
	}
	DEBUG_PRINT("CDS_MNFactionBattleMan:func=%s,item master item_id=%d,item_num=%d,max_count=%d,proc_type=%d\n",__FUNCTION__, item_id, item_num, max_count, proc_type);
	_item_master.item_id	= item_id;
	_item_master.item_num	= item_num;
	_item_master.max_count	= max_count;
	_item_master.proc_type	= proc_type;

	int credit_a = atoi(conf->find(key, "credit_a").c_str());
	int credit_b = atoi(conf->find(key, "credit_b").c_str());
	int credit_c = atoi(conf->find(key, "credit_c").c_str());
	if (credit_a < 0 || credit_b < 0 || credit_c < 0)
	{
		Log::log(LOG_ERR, "credit_a < 0 || credit_b < 0 || credit_c < 0"); 
		return false;
	}
	faction_credit.Init(credit_a, credit_b, credit_c);

	return true;
}

bool CDS_MNFactionBattleMan::Initialize()
{
	if(!LoadConfig()) return false;
	int zonelist[1] = { -1 }; 
	int cross_day = (_cross_day == 7)?(0):_cross_day;
	CrossGuardServer::GetInstance()->Register(CT_MNFACTION_BATTLE, cross_day, _cross_begin_time%ONE_DAY - 60*5, _battle_end_time%ONE_DAY + 60*10, zonelist,1);
	//CrossGuardServer::GetInstance()->Register(CT_MNFACTION_BATTLE, 6, 1, 86399, zonelist,1); //debug
	IntervalTimer::Attach(this, 3000000/IntervalTimer::Resolution());
	return true;
}

bool CDS_MNFactionBattleMan::Update()
{
	if(!_is_init) return true;
	if(!GameDBClient::GetInstance()->IsConnect()) return true;

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
				if((time_in_a_week >= _apply_begin_time) && (time_in_a_week < _apply_filtrated_end_time))
				{
					_state = STATE_APPLY_FILTRATED_BEGIN;
					UpdateDBMNFactionState();
					DebugDumpFlag(__FUNCTION__, __LINE__);
				}
			}
			break;

		case STATE_APPLY_FILTRATED_BEGIN:
			{
				if(time_in_a_week >= _apply_filtrated_end_time)
				{
					_state = STATE_APPLY_FILTRATED_END;
					UpdateDBMNFactionState();
					DebugDumpFlag(__FUNCTION__, __LINE__);
				}
			}
			break;

		case STATE_APPLY_FILTRATED_END:
			{
				//筛选报名资格(跨服)
				FiltrateApplyInfo();
				UpdateMNFactionInviteCount();
				UpdateDBMNFactionInfo(); //更新factioninfo 和 domaininfo
				UpdateDBMNDomainInfo();
				_state = STATE_BATTLE_WAIT_BEGIN;
				UpdateDBMNFactionState();
				DebugDumpFlag(__FUNCTION__, __LINE__);
			}
			break;

		case STATE_BATTLE_WAIT_BEGIN:
			{
				if(time_in_a_week >= _battle_begin_time)
				{
					_state = STATE_BATTLE_BEGIN;
					UpdateDBMNFactionState();
					DebugDumpFlag(__FUNCTION__, __LINE__);
				}
			}
			break;

		case STATE_BATTLE_BEGIN:
			{
				if(time_in_a_week >= _battle_end_time)
				{
					_state = STATE_BATTLE_END;
					UpdateDBMNFactionState();
					DebugDumpFlag(__FUNCTION__, __LINE__);
				}

				if(_has_init_instance_domain_num < _domain_map.size())
				{
					InitDomainInstance();
				}
			}
			break;

		case STATE_BATTLE_END:
			{
				if(time_in_a_week >= _bonus_time)
				{
					_state = STATE_CALC_BONUS;
					UpdateDBMNFactionState();
					DebugDumpFlag(__FUNCTION__, __LINE__);
				}
			}
			break;

		case STATE_CALC_BONUS:
			{
				CalcBonusCredit();
				_state = STATE_SAVE_DB_BONUS;
				DebugDumpFlag(__FUNCTION__, __LINE__);
				UpdateDBMNFactionState();
			}
			break;
			
		case STATE_SAVE_DB_BONUS:
			{
				UpdateDBMNFactionInfo(); //更新factioninfo 和 domaininfo
				UpdateDBMNDomainInfo();
				UpdateDBDomainBonus();
				_state = STATE_SEND_BONUS;
				DebugDumpFlag(__FUNCTION__, __LINE__);
				UpdateDBMNFactionState();
			}
			break;

		case STATE_SEND_BONUS:
			{
				if(time_in_a_week >= _close_time)
				{
					Clear(); 
					_state = STATE_CLOSE;
					UpdateDBMNFactionState();
					DebugDumpFlag(__FUNCTION__, __LINE__);
				}

				if(_bonus_map.size() > 0)
				{
					SendDomainBonusData();	
				}

			}
			break;

		default:
			break;
	}
	return true;
}

void CDS_MNFactionBattleMan::OnInitialize()
{
	DEBUG_PRINT("CDS_MNFactionBattleMan:func=%s\n",__FUNCTION__);
	GameDBClient::GetInstance()->SendProtocol(Rpc::Call(RPC_DBMNFACTIONINFOGET, DBMNFactionInfoGetArg()));
}

void CDS_MNFactionBattleMan::OnGetDBMNFactionCache(unsigned int sn, unsigned char state, MNDomainInfoVector& domaininfo_list, MNFactionInfoVector& factioninfo_list)
{
	DEBUG_PRINT("CDS_MNFactionBattleMan:func=%s\tsn=%d\tstate=%d\n",__FUNCTION__, sn, state);
	_sn = sn;
	_state = state;
	UpdateMNFactionInfoMap(factioninfo_list);
	InitDomainInfoMap(domaininfo_list);
	CDS_MNToplistMan::GetInstance()->UpdateMNToplist(factioninfo_list);//更新排行榜
	SaveForClientDomainData();
	GSMNDomainInfoBroadcast();
	_is_init = true;
}

void CDS_MNFactionBattleMan::InitDomainInfoMap(MNDomainInfoVector & domaininfo_list)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
	if(domaininfo_list.size() == 0 && _domain_map.size() == 0)
	{
		int domain_id = 0;
		for(; domain_id < DOMAIN_NUM_A; ++ domain_id)
		{
			BattleDomainInfo info;
			info.Init(domain_id, DOMAIN_TYPE_A, 0, 0, 0);
			_domain_map.insert(std::make_pair(domain_id, info));
		}

		for(; domain_id < (DOMAIN_NUM_B + DOMAIN_NUM_A); ++ domain_id)
		{
			BattleDomainInfo info;
			info.Init(domain_id, DOMAIN_TYPE_B, 0, 0, 0);
			_domain_map.insert(std::make_pair(domain_id, info));
		}

		for(; domain_id < (DOMAIN_NUM_C + DOMAIN_NUM_B + DOMAIN_NUM_A); ++ domain_id)
		{
			BattleDomainInfo info;
			info.Init(domain_id, DOMAIN_TYPE_C, 0, 0, 0);
			_domain_map.insert(std::make_pair(domain_id, info));
		}
	}
	else
	{
		UpdateDomainInfoMap(domaininfo_list);
	}
}

void CDS_MNFactionBattleMan::UpdateMNFactionInfoMap(MNFactionInfoVector& factioninfo_list)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
	for(MNFactionInfoVector::iterator it = factioninfo_list.begin(); it != factioninfo_list.end(); ++ it)
	{
		if(it->domain_num.size() != DOMAIN_TYPE_COUNT)
		{
			Log::log(LOG_ERR, "[func=%s]faction domain_num size error[unifid=%lld][size=%d]", __FUNCTION__, it->unifid, it->domain_num.size());
			continue;
		}
		_faction_map[it->unifid] = *it;
	}
	DebugDumpFactionInfo(__FUNCTION__, factioninfo_list);
}

void CDS_MNFactionBattleMan::UpdateCDCMNFactionInfoMap(MNFactionInfoVector& factioninfo_list)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\t%d\n", __FILE__, __FUNCTION__, __LINE__, factioninfo_list.size());
	for(MNFactionInfoVector::iterator it = factioninfo_list.begin(); it != factioninfo_list.end(); ++ it)
	{
		if(it->domain_num.size() != DOMAIN_TYPE_COUNT)
		{
			Log::log(LOG_ERR, "[func=%s]faction domain_num size error[unifid=%lld][size=%d]", __FUNCTION__, it->unifid, it->domain_num.size());
			continue;
		}
		
		if(_faction_map.find(it->unifid) != _faction_map.end())
		{
			_faction_map[it->unifid].fid			= it->fid;
			_faction_map[it->unifid].faction_name	= it->faction_name;
			_faction_map[it->unifid].master_name	= it->master_name;
			_faction_map[it->unifid].zoneid			= it->zoneid; 
			_faction_map[it->unifid].version		= it->version; 
		}
		else
		{
			_faction_map[it->unifid] = *it;
		}
	}
	DebugDumpFactionInfo(__FUNCTION__, factioninfo_list);
	//DebugDumpFactionInfo(__FUNCTION__);
}

void CDS_MNFactionBattleMan::UpdateDomainInfoMap(MNDomainInfoVector &domaininfo_list)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
	for(MNDomainInfoVector::iterator it = domaininfo_list.begin(); it != domaininfo_list.end(); ++ it)
	{
		BattleDomainInfo info;
		info.Init(it->domain_id, it->domain_type, it->owner_unifid, it->attacker_unifid, it->defender_unifid);
		_domain_map[it->domain_id] = info;
	}
	DebugDumpDomainInfo(__FUNCTION__, domaininfo_list);
	SaveForClientDomainData();
}

void CDS_MNFactionBattleMan::UpdateDBMNFactionState()
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
	DBMNFactionStateUpdateArg arg;
	arg.state = _state;
	DBMNFactionStateUpdate* rpc = (DBMNFactionStateUpdate*)Rpc::Call(RPC_DBMNFACTIONSTATEUPDATE, arg);
	GameDBClient::GetInstance()->SendProtocol(rpc);
}

void CDS_MNFactionBattleMan::SendMNFBattleCache(int zoneid)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\tzoneid=%d\n", __FILE__, __FUNCTION__, __LINE__, zoneid);
	if(!_is_init) return;
	MNFactionInfoVector faction_list;
	std::vector<int64_t> faction_id_list;
	FACTION_MAP::iterator fit = _faction_map.begin();
	while(fit != _faction_map.end())
	{
		if(fit->second.zoneid == zoneid)
		{
			faction_list.push_back(fit->second);
			faction_id_list.push_back(fit->second.unifid);
		}
		++ fit;
	}

	MNDomainInfoVector domain_list;
	DOMAIN_MAP::iterator dit = _domain_map.begin();
	while(dit != _domain_map.end())
	{
		MNDomainInfo info;
		info.domain_id			= dit->second.domain_id;
		info.domain_type		= dit->second.domain_type;
		info.owner_unifid		= dit->second.owner_unifid;
		info.attacker_unifid	= dit->second.attacker_unifid;
		info.defender_unifid	= dit->second.defender_unifid;
		domain_list.push_back(info);

		//方便普通服查看战场信息,把跨服城池的帮派信息也发过去
		if(std::find(faction_id_list.begin(), faction_id_list.end(), info.owner_unifid) == faction_id_list.end())
		{
			FACTION_MAP::iterator it_owner = _faction_map.find(info.owner_unifid);
			if(it_owner != _faction_map.end() && info.owner_unifid != 0)
			{
				faction_list.push_back(it_owner->second);
				faction_id_list.push_back(it_owner->first);
			}
		}

		if(std::find(faction_id_list.begin(), faction_id_list.end(), info.attacker_unifid) == faction_id_list.end())
		{
			FACTION_MAP::iterator it_attacker = _faction_map.find(info.attacker_unifid);
			if(it_attacker != _faction_map.end() && info.attacker_unifid != 0)
			{
				faction_list.push_back(it_attacker->second);
				faction_id_list.push_back(it_attacker->first);
			}
		}

		if(std::find(faction_id_list.begin(), faction_id_list.end(), info.defender_unifid) == faction_id_list.end())
		{
			FACTION_MAP::iterator it_defender = _faction_map.find(info.defender_unifid);
			if(it_defender != _faction_map.end() && info.defender_unifid != 0)
			{
				faction_list.push_back(it_defender->second);
				faction_id_list.push_back(it_defender->first);
			}
		}

		++ dit;
	}

	MNFactionCacheGet_Re re;
	re.sn				= _sn;
	re.apply_begin_time = _apply_begin_time;
	re.apply_end_time	= _apply_filtrated_begin_time;
	re.cross_begin_time = _cross_begin_time;
	re.battle_begin_time= _battle_begin_time;
	re.battle_end_time	= _battle_end_time;
	re.domaininfo_list 	= domain_list; 
	re.factioninfo_list = faction_list;
	CentralDeliveryServer::GetInstance()->DispatchProtocol(zoneid, re);
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\tzoneid=%d\tdomain.size=%d\tfaction.size=%d\n", __FILE__, __FUNCTION__, __LINE__, zoneid, domain_list.size(), faction_list.size());
}

void CDS_MNFactionBattleMan::FiltrateApplyInfo()
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
	std::vector<int64_t> candidates_a, candidates_b, candidates_c;
	ClassifyApplyInfo(candidates_a, candidates_b, candidates_c);
	FiltrateApplyInfo_A(candidates_a);
	FiltrateApplyInfo_B(candidates_b);
	FiltrateApplyInfo_C(candidates_c);
	SaveForClientDomainData();
	//DebugDumpDomainInfo(__FUNCTION__);
}

void CDS_MNFactionBattleMan::ClassifyApplyInfo(std::vector<int64_t> &candidates_a, std::vector<int64_t> &candidates_b, std::vector<int64_t> &candidates_c)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\tapply.size=%d\n", __FILE__, __FUNCTION__, __LINE__, _apply_map.size());
	for(APPLYINFO_MAP::iterator it = _apply_map.begin(); it != _apply_map.end(); ++ it)
	{
		if(it->second.dest == DOMAIN_TYPE_A)
		{
			candidates_a.push_back(it->first);	
		}
		else if(it->second.dest == DOMAIN_TYPE_B)
		{
			candidates_b.push_back(it->first);	
		}
		else if(it->second.dest == DOMAIN_TYPE_C)
		{
			candidates_c.push_back(it->first);	
		}
	}
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\tcandidates_a.size=%d\tcandidates_b.size=%d\tcandidates_c.size=%d\n", __FILE__, __FUNCTION__, __LINE__, candidates_a.size(), candidates_b.size(), candidates_c.size());
}

void CDS_MNFactionBattleMan::FiltrateApplyInfo_C(std::vector<int64_t> &candidates_c)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\tcandidates_c.size=%d\n", __FILE__, __FUNCTION__, __LINE__, candidates_c.size());
	std::vector<int64_t> candidates = candidates_c;

	int max_req_faction_num = DOMAIN_NUM_C * 2;
	for(DOMAIN_MAP::iterator it = _domain_map.begin(); it != _domain_map.end(); ++ it)
	{
		if(it->second.domain_type == DOMAIN_TYPE_C && it->second.owner_unifid != 0) -- max_req_faction_num; 
	}

	std::vector<int64_t> chosen_list;
	std::vector<int64_t>::iterator it = candidates.begin();

	//先选出有b城没有c城的
	if((int)candidates.size() > max_req_faction_num)
	{
		while(it != candidates.end() && max_req_faction_num > 0 && candidates.size() > 0)
		{
			FACTION_MAP::iterator fit = _faction_map.find(*it);
			if(fit != _faction_map.end())
			{
				if(fit->second.domain_num[DOMAIN_TYPE_B] > 0 && fit->second.domain_num[DOMAIN_TYPE_C] == 0)
				{
					chosen_list.push_back(*it);
					it = candidates.erase(it);
					-- max_req_faction_num;
					if(max_req_faction_num == 0) break; //需要保证A城数量 < B < C ，否则不是随机选择的结果
				}
				else
				{
					++ it;
				}
			}
			else
			{
				Log::log(LOG_ERR, "[func=%s]faction id not found[unifid=%lld]", __FUNCTION__, *it);
				it = candidates.erase(it);
			}
		}
	}
	else
	{
		chosen_list = candidates;
		candidates.erase(candidates.begin(), candidates.end());
	}

	DebugDumpUnifid("filtrate C step 1:", chosen_list);

	//选出什么城都没有的,并且原服有12座城以上
	//候选人太多随机选择，否则就全部选择
	it = candidates.begin();
	std::vector<int64_t> candidates_without_domain_lvl3;
	while(it != candidates.end())
	{
		FACTION_MAP::iterator fit = _faction_map.find(*it);
		if(fit != _faction_map.end())
		{
			if(fit->second.domain_num[DOMAIN_TYPE_B] == 0 && fit->second.domain_num[DOMAIN_TYPE_C] == 0
			&& std::find(_lvl3_list.begin(), _lvl3_list.end(), *it) != _lvl3_list.end())
			{
				candidates_without_domain_lvl3.push_back(*it);
				it = candidates.erase(it);
			}
			else
			{
				++ it;
			}
		}
		else
		{
			Log::log(LOG_ERR, "[func=%s]faction id not found[unifid=%lld]", __FUNCTION__, *it);
			it = candidates.erase(it);
		}
	}

	if((int)candidates_without_domain_lvl3.size() > max_req_faction_num)
	{
		while(max_req_faction_num > 0)
		{
			int index = rand() % candidates_without_domain_lvl3.size();
			chosen_list.push_back(candidates_without_domain_lvl3[index]);
			candidates_without_domain_lvl3.erase(candidates_without_domain_lvl3.begin() + index);
			-- max_req_faction_num;
		}
	}
	else
	{
		for(std::vector<int64_t>::iterator it = candidates_without_domain_lvl3.begin(); it != candidates_without_domain_lvl3.end(); ++ it)
		{
			chosen_list.push_back(*it);
			--	max_req_faction_num;
			if(max_req_faction_num == 0) break;
		}
	}
	DebugDumpUnifid("filtrate C step 2:", chosen_list);

	//选出什么城都没有的,
	//候选人太多随机选择，否则就全部选择
	it = candidates.begin();
	std::vector<int64_t> candidates_without_domain;
	while(it != candidates.end())
	{
		FACTION_MAP::iterator fit = _faction_map.find(*it);
		if(fit != _faction_map.end())
		{
			if(fit->second.domain_num[DOMAIN_TYPE_B] == 0 && fit->second.domain_num[DOMAIN_TYPE_C] == 0)
			{
				candidates_without_domain.push_back(*it);
				it = candidates.erase(it);
			}
			else
			{
				++ it;
			}
		}
		else
		{
			Log::log(LOG_ERR, "[func=%s]faction id not found[unifid=%lld]", __FUNCTION__, *it);
			it = candidates.erase(it);
		}
	}

	if((int)candidates_without_domain.size() > max_req_faction_num)
	{
		while(max_req_faction_num > 0)
		{
			int index = rand() % candidates_without_domain.size();
			chosen_list.push_back(candidates_without_domain[index]);
			candidates_without_domain.erase(candidates_without_domain.begin() + index);
			-- max_req_faction_num;
		}
	}
	else
	{
		for(std::vector<int64_t>::iterator it = candidates_without_domain.begin(); it != candidates_without_domain.end(); ++ it)
		{
			chosen_list.push_back(*it);
			--	max_req_faction_num;
			if(max_req_faction_num == 0) break;
		}
	}
	DebugDumpUnifid("filtrate C step 3:", chosen_list);

	//剩下的随机选择
	if((int)candidates.size() > max_req_faction_num)
	{
		while(max_req_faction_num > 0)
		{
			int index = rand() % candidates.size();
			chosen_list.push_back(candidates[index]);
			-- max_req_faction_num;
			candidates.erase(candidates.begin() + index);
		}
	}
	else
	{
		for(std::vector<int64_t>::iterator it = candidates.begin(); it != candidates.end(); ++ it)
		{
			chosen_list.push_back(*it);	
			--	max_req_faction_num;
			if(max_req_faction_num == 0) break;
		}
	}
	DebugDumpUnifid("filtrate C step 4:", chosen_list);

	_chosen_list.insert(_chosen_list.end(), chosen_list.begin(), chosen_list.end());
	RandMatchBattle(chosen_list, DOMAIN_TYPE_C);
}

void CDS_MNFactionBattleMan::FiltrateApplyInfo_B(std::vector<int64_t> &candidates_b)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\tcandidates_b.size=%d\n", __FILE__, __FUNCTION__, __LINE__, candidates_b.size());

	std::vector<int64_t> candidates = candidates_b;
	int max_req_faction_num = DOMAIN_NUM_B * 2;
	for(DOMAIN_MAP::iterator it = _domain_map.begin(); it != _domain_map.end(); ++ it)
	{
		if(it->second.domain_type == DOMAIN_TYPE_B && it->second.owner_unifid != 0) -- max_req_faction_num; 
	}
	std::vector<int64_t> chosen_list;
	std::vector<int64_t>::iterator it = candidates.begin();

	if((int)candidates.size() > max_req_faction_num)
	{
		//先选出有c城没有b城的
		while(it != candidates.end() && max_req_faction_num > 0)
		{
			FACTION_MAP::iterator fit = _faction_map.find(*it);
			if(fit != _faction_map.end())
			{
				if(fit->second.domain_num[DOMAIN_TYPE_B] == 0 && fit->second.domain_num[DOMAIN_TYPE_C] > 0)
				{
					chosen_list.push_back(*it);
					it = candidates.erase(it);
					-- max_req_faction_num;
				}
				else
				{
					++it;
				}
			}
			else
			{
				Log::log(LOG_ERR, "[func=%s]candidate faction id not found [unifid=%lld]", __FUNCTION__, *it);
				it = candidates.erase(it);
			}
		}
	}
	else
	{
		chosen_list = candidates;
		candidates.erase(candidates.begin(), candidates.end());
	}
	DebugDumpUnifid("filtrate B step 1:", chosen_list);

	it = candidates.begin(); 
	if((int)candidates.size() > max_req_faction_num)
	{
		while(max_req_faction_num > 0)
		{
			int index = rand() % candidates.size();
			chosen_list.push_back(candidates[index]);
			-- max_req_faction_num;
			candidates.erase(candidates.begin() + index);
		}
	}
	else
	{
		while(it != candidates.end())
		{
			chosen_list.push_back(*it);
			-- max_req_faction_num;
			++ it;
		}
		candidates.erase(candidates.begin(), candidates.end());
	}
	DebugDumpUnifid("filtrate B step 2:", chosen_list);

	_chosen_list.insert(_chosen_list.end(), chosen_list.begin(), chosen_list.end());
	RandMatchBattle(chosen_list, DOMAIN_TYPE_B);
}

void CDS_MNFactionBattleMan::FiltrateApplyInfo_A(std::vector<int64_t> &candidates_a)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\tcandidates_a.size=%d\n", __FILE__, __FUNCTION__, __LINE__, candidates_a.size());
	//打a的资格已经筛选过了，直接随机就行了。
	std::vector<int64_t> chosen_list = candidates_a;
	for(DOMAIN_MAP::iterator it = _domain_map.begin(); it != _domain_map.end(); ++ it)
	{
		if(it->second.domain_type == DOMAIN_TYPE_A) 
		{
			it->second.owner_unifid	= 0;
			it->second.attacker_unifid = 0;
			it->second.defender_unifid = 0;
		}
	}
	_chosen_list.insert(_chosen_list.end(), candidates_a.begin(), candidates_a.end());
	RandMatchBattle(chosen_list, DOMAIN_TYPE_A);
}

void CDS_MNFactionBattleMan::RandMatchBattle(std::vector<int64_t> &chosen_list, DOMAIN_TYPE domain_type)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
	DOMAIN_MAP domain;
	DOMAIN_MAP domain_without_owner;
	DOMAIN_MAP::iterator dit = _domain_map.begin();

	//初始化
	while(dit != _domain_map.end())
	{
		if(dit->second.domain_type == domain_type)
		{
			if(dit->second.owner_unifid == 0)
			{
				domain_without_owner[dit->second.domain_id] = dit->second;	
			}
			else 
			{
				//初始化defender_unifid
				dit->second.defender_unifid = dit->second.owner_unifid;
			}
			domain[dit->second.domain_id] = dit->second;	
		}
		++ dit;
	}

	//分配无人占领的战场
	for(DOMAIN_MAP::iterator it = domain_without_owner.begin(); it != domain_without_owner.end(); ++ it)
	{
		int index = 0;
		//分配进攻方
		if(chosen_list.size() == 0) break;
		index = rand() % chosen_list.size();
		it->second.attacker_unifid = chosen_list[index];
		chosen_list.erase(chosen_list.begin() + index);
		
		//分配防守方
		if(chosen_list.size() == 0) break;
		index = rand() % chosen_list.size();
		it->second.defender_unifid = chosen_list[index];
		chosen_list.erase(chosen_list.begin() + index);
	}

	CopyDomainMap(domain_without_owner, domain);

	//分配有人占领的战场, 随机后需要保证attacker != defender
	for(DOMAIN_MAP::iterator it = domain.begin(); it != domain.end(); ++ it)
	{
		if(chosen_list.size() == 0) break;
		if(it->second.attacker_unifid != 0) continue;

		int index = 0;
		do
		{
			index = rand() % chosen_list.size();
			it->second.attacker_unifid = chosen_list[index];
			if((chosen_list.size() == 1) && (it->second.defender_unifid == it->second.attacker_unifid)) 
			{
				//最后随机的帮派和当前城池的占有帮派是同一个,
				//就将这个id和domain的第一个enemy换，直到enemy和owner不相等,一定能找到不相等的,因为一个帮派不会把C城都占了，B城也是
				for(DOMAIN_MAP::iterator dit = domain.begin(); dit != domain.end(); ++ dit)
				{
					if(dit->second.defender_unifid != chosen_list[index])
					{
						it->second.attacker_unifid 	= dit->second.attacker_unifid;
						dit->second.attacker_unifid = chosen_list[index];
						break;
					}
				}
				break;
			}
		}while(it->second.attacker_unifid == it->second.defender_unifid);

		chosen_list.erase(chosen_list.begin() + index);
	}

	CopyDomainMap(domain, _domain_map);
}

void CDS_MNFactionBattleMan::CopyDomainMap(const DOMAIN_MAP& src, DOMAIN_MAP& dest)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
	for(DOMAIN_MAP::const_iterator it = src.begin(); it != src.end(); ++ it)
	{
		dest[it->first] = it->second;
	}
}

void CDS_MNFactionBattleMan::UpdateMNFactionInviteCount()
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);

	//init
	for(FACTION_MAP::iterator it = _faction_map.begin(); it != _faction_map.end(); ++ it)
	{
		it->second.invite_count = 0;
	}
	
	//update
	for(DOMAIN_MAP::iterator it = _domain_map.begin(); it != _domain_map.end(); ++ it)
	{
		FACTION_MAP::iterator fit1 = _faction_map.find(it->second.defender_unifid);
		if(fit1 != _faction_map.end())
		{
			if(fit1->second.accept_sn != _sn)
			{
				fit1->second.accept_sn = _sn;
			}
			fit1->second.invite_count += INVITE_COUNT_PER_DOMAIN;
			if(fit1->second.invite_count > INVITE_COUNT_MAX)
			{
				fit1->second.invite_count = INVITE_COUNT_MAX;
			}
		}

		FACTION_MAP::iterator fit2 = _faction_map.find(it->second.attacker_unifid);
		if(fit2 != _faction_map.end())
		{
			if(fit2->second.accept_sn != _sn)
			{
				fit2->second.accept_sn = _sn;
			}
			fit2->second.invite_count += INVITE_COUNT_PER_DOMAIN;
			if(fit2->second.invite_count > INVITE_COUNT_MAX)
			{
				fit2->second.invite_count = INVITE_COUNT_MAX;
			}
		}
	}

}

void CDS_MNFactionBattleMan::SendFiltrateResult(int zoneid)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\tzoneid=%d\n", __FILE__, __FUNCTION__, __LINE__, zoneid);
	if(_state != STATE_BATTLE_WAIT_BEGIN) return;
	MNFetchFiltrateResult_Re re;
	std::vector<int64_t> chosen_list;
	for(std::vector<int64_t>::iterator it = _chosen_list.begin(); it != _chosen_list.end(); ++ it)
	{
		FACTION_MAP::iterator fit = _faction_map.find(*it);
		if(fit != _faction_map.end())
		{
			if(fit->second.zoneid == zoneid)
			{
				chosen_list.push_back(*it);
			}
		}
		else
		{
			Log::log(LOG_ERR, "[file=%s][func=%s][line=%d][unifid=%lld]", __FILE__, __FUNCTION__, __LINE__, *it);
		}
	}
	re.chosen_list = chosen_list;
	CentralDeliveryServer::GetInstance()->DispatchProtocol(zoneid, re);
	DEBUG_PRINT("file=%s\tchosen_list:size=%d\n", __FILE__, re.chosen_list.size()); 
	for(std::vector<int64_t>::iterator it = re.chosen_list.begin(); it != re.chosen_list.end(); ++ it)
	{       
		DEBUG_PRINT("chosen unifid=%lld", *it);
	}
}


void CDS_MNFactionBattleMan::OnGetMNFactionProclaim(int zoneid, MNFactionApplyInfoVector& applyinfo_list, MNFactionInfoVector& factioninfo_list, std::vector<int64_t>& lvl3_list)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\tzoneid=%d\tapplyinfo_list.size=%d\tfactioninfo_list.size=%d\tlvl3_list.size()=%d\n", __FILE__, __FUNCTION__, __LINE__, zoneid, applyinfo_list.size(), factioninfo_list.size(), lvl3_list.size());

	UpdateApplyInfoMap(applyinfo_list);
	UpdateCDCMNFactionInfoMap(factioninfo_list);
	UpdateDBMNFactionInfo(factioninfo_list);

	_lvl3_list.insert(_lvl3_list.end(), lvl3_list.begin(), lvl3_list.end());
	DebugDumpUnifid("lvl3_list", _lvl3_list); 
	MNFactionProclaim_Re re;
	re.retcode = ERR_SUCCESS;
	CentralDeliveryServer::GetInstance()->DispatchProtocol(zoneid, re);
	DebugDumpApplyInfo(__FUNCTION__, applyinfo_list);
}

/*
void CDS_MNFactionBattleMan::OnGetMNFApplyInfoList(MNFactionApplyInfoVector& applyinfo_list)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
	UpdateApplyInfoMap(applyinfo_list);
}
*/

void CDS_MNFactionBattleMan::UpdateApplyInfoMap(MNFactionApplyInfoVector& applyinfo_list)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\tapplyinfo_list.size()\n", __FILE__, __FUNCTION__, __LINE__, applyinfo_list.size());
	for(MNFactionApplyInfoVector::iterator it = applyinfo_list.begin(); it != applyinfo_list.end(); ++ it)
	{
		_apply_map[it->unifid] = *it;
	}
	DebugDumpApplyInfo(__FUNCTION__, applyinfo_list);
}

void CDS_MNFactionBattleMan::UpdateDBMNDomainInfo()
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
	//更新domain info表
	DBMNDomainInfoUpdateArg arg;
	for(DOMAIN_MAP::iterator dit = _domain_map.begin(); dit != _domain_map.end(); ++ dit)
	{
		MNDomainInfo info;
		info.domain_id			= dit->second.domain_id;
		info.domain_type		= dit->second.domain_type;
		info.owner_unifid		= dit->second.owner_unifid;
		info.attacker_unifid	= dit->second.attacker_unifid;
		info.defender_unifid	= dit->second.defender_unifid;
		arg.domaininfo_list.push_back(info);
	}

	DBMNDomainInfoUpdate* rpc = (DBMNDomainInfoUpdate* )Rpc::Call(RPC_DBMNDOMAININFOUPDATE, arg);
	GameDBClient::GetInstance()->SendProtocol(rpc);
	//DebugDumpDomainInfo(__FUNCTION__, arg.domaininfo_list);
}

void CDS_MNFactionBattleMan::GSMNDomainInfoNotice(unsigned int sid)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
	MNDomainInfoGSNotice pro;
	for(DOMAIN_MAP::iterator it = _domain_map.begin(); it != _domain_map.end(); ++ it)
	{
		MNDomainInfo info;
		info.domain_id			= it->second.domain_id;
		info.domain_type		= it->second.domain_type;
		info.owner_unifid		= it->second.owner_unifid;
		info.attacker_unifid	= it->second.attacker_unifid;
		info.defender_unifid	= it->second.defender_unifid;
		pro.domaininfo_list.push_back(info);
	}
	GProviderServer::GetInstance()->Send(sid, pro);
}

void CDS_MNFactionBattleMan::GSMNDomainInfoBroadcast()
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
	MNDomainInfoGSNotice pro;
	for(DOMAIN_MAP::iterator it = _domain_map.begin(); it != _domain_map.end(); ++ it)
	{
		MNDomainInfo info;
		info.domain_id			= it->second.domain_id;
		info.domain_type		= it->second.domain_type;
		info.owner_unifid		= it->second.owner_unifid;
		info.attacker_unifid	= it->second.attacker_unifid;
		info.defender_unifid	= it->second.defender_unifid;
		pro.domaininfo_list.push_back(info);
	}
	GProviderServer::GetInstance()->BroadcastProtocol(pro);
}

void CDS_MNFactionBattleMan::UpdateDBMNFactionInfo()
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
	//更新faction info表
	DBMNFactionInfoUpdateArg arg;
	for(FACTION_MAP::iterator it = _faction_map.begin(); it != _faction_map.end(); ++ it)
	{
		if(it->second.bonus_sn == _sn || it->second.accept_sn == _sn)
		{
			arg.factioninfo_list.push_back(it->second);
		}
	}

	DBMNFactionInfoUpdate* rpc = (DBMNFactionInfoUpdate* )Rpc::Call(RPC_DBMNFACTIONINFOUPDATE, arg);
	GameDBClient::GetInstance()->SendProtocol(rpc);
	DebugDumpFactionInfo(__FUNCTION__, arg.factioninfo_list);
}

void CDS_MNFactionBattleMan::UpdateDBMNFactionInfo(MNFactionInfoVector& factioninfo_list)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
	//更新faction info表

	MNFactionInfoVector list;
	for(MNFactionInfoVector::iterator it = factioninfo_list.begin(); it != factioninfo_list.end(); ++ it)
	{
		if(it->domain_num.size() != DOMAIN_TYPE_COUNT)
		{
			Log::log(LOG_ERR, "[func=%s]faction domain_num size error[unifid=%lld][size=%d]", __FUNCTION__, it->unifid, it->domain_num.size());
			continue;
		}
	
		FACTION_MAP::iterator fit = _faction_map.find(it->unifid);
		if(fit != _faction_map.end())
		{
			list.push_back(fit->second);
		}
		else
		{
			list.push_back(*it);
			Log::log(LOG_ERR, "[file=%s][func=%s][unifid=%lld]", __FILE__, __FUNCTION__, it->unifid);
		}
	}
	
	DBMNFactionInfoUpdateArg arg;
	arg.factioninfo_list = list;
	DBMNFactionInfoUpdate* rpc = (DBMNFactionInfoUpdate* )Rpc::Call(RPC_DBMNFACTIONINFOUPDATE, arg);
	GameDBClient::GetInstance()->SendProtocol(rpc);
	DebugDumpFactionInfo(__FUNCTION__, arg.factioninfo_list);
}

void CDS_MNFactionBattleMan::OnRegisterServer(int server_id, int world_tag)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\tserverid=%d\tworld_tag=%d\n", __FILE__, __FUNCTION__, __LINE__, server_id, world_tag);
	_server_id = server_id;
	_world_tag = world_tag;
}

void CDS_MNFactionBattleMan::ClearEnterBattlePlayerNum()
{
	for(DOMAIN_MAP::iterator it = _domain_map.begin(); it != _domain_map.end(); ++ it)
	{
		it->second.enter_domain_num_defender = 0;
		it->second.enter_domain_num_attacker = 0;
		it->second.has_enter_domain_num_defender = 0;
		it->second.has_enter_domain_num_attacker = 0;
	}
}

void CDS_MNFactionBattleMan::InitDomainInstance()
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d_has_init_instance_domain_num=%d\n", __FILE__, __FUNCTION__, __LINE__, _has_init_instance_domain_num);
	for(unsigned int i = _has_init_instance_domain_num; i < _domain_map.size(); ++ i)
	{
		MNDomainBattleStart pro;
		pro.domain_id 	= _domain_map[i].domain_id;
		pro.domain_type = _domain_map[i].domain_type;
		pro.owner		= _domain_map[i].owner_unifid;
		pro.attacker 	= _domain_map[i].attacker_unifid;
		pro.defender 	= _domain_map[i].defender_unifid;
		pro.expiretime 	= Timer::GetTime() + _battle_end_time - _battle_begin_time;
		if(i >= _has_init_instance_domain_num + INIT_INSTANCE_ONCE) break; //每次创建INIT_INSTANCE_ONCE个
		if(!GProviderServer::GetInstance()->DispatchProtocol(_server_id, pro)) 
		{
			Log::log(LOG_ERR, "[func=%s] [id=%d] [serverid=%d] failed", __FUNCTION__, pro.domain_id, _server_id);
			return;
		}
	}
	_has_init_instance_domain_num += INIT_INSTANCE_ONCE;
}

void CDS_MNFactionBattleMan::OnPlayerBattleEnter(int roleid, int64_t unifid, int domain_id)
{
	int retcode = CheckPlayerBattleEnter(roleid, unifid, domain_id); 
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\troleid=%d\tdomain_id=%d\tunifid=%lld\tretcode=%d\n", __FILE__, __FUNCTION__, __LINE__, roleid, domain_id, unifid, retcode);
	PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline(roleid);
	if(pinfo == NULL) return;
	int gameid = pinfo->gameid;
	MNDomainBattleEnter_Re re;
	re.retcode		= retcode;
	re.roleid		= roleid;
	re.unifid		= unifid;
	re.world_tag	= _world_tag;
	re.domain_id	= domain_id;
	
	GProviderServer::GetInstance()->DispatchProtocol(gameid, re);
}

void CDS_MNFactionBattleMan::OnPlayerBattleEnterSuccess(int roleid, int64_t unifid, int domain_id)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\troleid=%d\tunifid=%lld\tdomain_id=%d\n", __FILE__, __FUNCTION__, __LINE__, roleid, unifid, domain_id);
	int remote_roleid = UserContainer::GetInstance().GetRemoteRoleid(roleid); 
	if(remote_roleid == 0) return;

	PLAYER_ENTRY_MAP::iterator it = _player_entry_map.find(roleid);
	if(it == _player_entry_map.end())
	{
		PlayerEntry entry;
		entry.roleid 		= roleid;
		entry.unifid		= unifid;
		entry.domain_id 	= domain_id;
		_player_entry_map[roleid] = entry; 
		DOMAIN_MAP::iterator dit = _domain_map.find(domain_id);
		if(dit == _domain_map.end())
		{
			Log::log(LOG_ERR, "func=%s\ttroleid=%d\tunifid=%lld[domain_id=%d err]\n", __FUNCTION__, roleid, unifid, domain_id);
			return;
		}
		if(dit->second.attacker_unifid == unifid)
		{
			++ dit->second.has_enter_domain_num_attacker;
		}
		else if(dit->second.defender_unifid == unifid)
		{
			++ dit->second.has_enter_domain_num_defender;
		}
		else
		{
			Log::log(LOG_ERR, "file=%s\tfunc=%s\tline=%d\troleid=%d\t unifid=%lld is not defender or attacker domain_id=%d\n", __FILE__, __FUNCTION__, __LINE__, roleid, unifid, domain_id);
			return; 
		}
	}

	DOMAIN_MAP::iterator dit = _domain_map.find(domain_id);
	if(dit->second.attacker_unifid == unifid)
	{
		++ dit->second.enter_domain_num_attacker;
	}
	else if(dit->second.defender_unifid == unifid)
	{
		++ dit->second.enter_domain_num_defender;
	}
	else
	{
		Log::log(LOG_ERR, "file=%s\tfunc=%s\tline=%d\troleid=%d\tunifid=%lld\tdomain_id=%d\n", __FILE__, __FUNCTION__, __LINE__, roleid, unifid, domain_id);
		return;
	}

	BONUS_MAP::iterator bit = _bonus_map.find(unifid);
	if(bit != _bonus_map.end())
	{
		if(std::find(bit->second.rolelist.begin(), bit->second.rolelist.end(), remote_roleid) == bit->second.rolelist.end())
		{
			bit->second.rolelist.push_back(remote_roleid);
			DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\troleid=%d\tunifid=%lld\tdomain_id=%d\n", __FILE__, __FUNCTION__, __LINE__, roleid, unifid, domain_id);
		}
	}
	else
	{
		//本帮派第一个进入成功的
		MNDomainBonus bonus;
		bonus.unifid = unifid;
		bonus.reward_list.resize(3);
		bonus.rolelist.push_back(remote_roleid);
		_bonus_map[unifid] = bonus;
		DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\troleid=%d\tunifid=%lld\tdomain_id=%d\n", __FILE__, __FUNCTION__, __LINE__, roleid, unifid, domain_id);
	}

	//DebugDumpBonus(__FUNCTION__);
}

int CDS_MNFactionBattleMan::CheckPlayerBattleEnter(int roleid, int64_t unifid, int domain_id)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\troleid=%d\tdomain_id=%d\n", __FILE__, __FUNCTION__, __LINE__, roleid, domain_id);
	if(_state != STATE_BATTLE_BEGIN) return ERR_MNF_WRONG_TIME;	

	PLAYER_ENTRY_MAP::iterator it = _player_entry_map.find(roleid);
	if(it != _player_entry_map.end())
	{
		if(unifid != it->second.unifid || domain_id != it->second.domain_id)
		{
			return ERR_MNF_MULTI_DOMAIN;
		}
	}

	DOMAIN_MAP::iterator dit = _domain_map.find(domain_id);
	if(dit == _domain_map.end()) return ERR_MNF_WRONG_ARGUMENT;
	if(dit->second.defender_unifid == unifid)
	{
		if(dit->second.enter_domain_num_defender >= INVITE_COUNT_PER_DOMAIN_MAX ||
		dit->second.has_enter_domain_num_defender >= INVITE_COUNT_PER_DOMAIN_HAS_ENTER_MAX)
		{
			return ERR_MNF_INVITE_COUNT_PERDOMAIN_MAXMUM;
		}
	}
	else if(dit->second.attacker_unifid == unifid)
	{
		if(dit->second.enter_domain_num_attacker >= INVITE_COUNT_PER_DOMAIN_MAX ||
		dit->second.has_enter_domain_num_attacker >= INVITE_COUNT_PER_DOMAIN_HAS_ENTER_MAX)
		{
			return ERR_MNF_INVITE_COUNT_PERDOMAIN_MAXMUM;
		}
	}
	else
	{
		return ERR_MNF_FACTION_NOT_INVITED;
	}

	return ERR_SUCCESS;
}

void CDS_MNFactionBattleMan::OnPlayerBattleLeave(int roleid/*todo 暂时没啥用*/, int64_t unifid, int domain_id)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\troleid=%d\tunifid=%lld\tdomain_id=%d\n", __FILE__, __FUNCTION__, __LINE__, roleid, unifid, domain_id);
	DOMAIN_MAP::iterator dit = _domain_map.find(domain_id);
	if(dit == _domain_map.end()) 
	{
		Log::log(LOG_ERR, "[func=%s] [domainid=%d] err ", __FUNCTION__, domain_id);
		return; 
	}
	if(dit->second.defender_unifid == unifid)
	{
		-- dit->second.enter_domain_num_defender;
		if(dit->second.enter_domain_num_defender < 0)
		{
			dit->second.enter_domain_num_defender = 0;
			Log::log(LOG_ERR, "[func=%s] [domainid=%d] [unifid=%lld] enter_domain_num_defender < 0", __FUNCTION__, domain_id, unifid);
		}
	}
	else if(dit->second.attacker_unifid == unifid)
	{
		-- dit->second.enter_domain_num_attacker;
		if(dit->second.enter_domain_num_attacker < 0)
		{
			dit->second.enter_domain_num_attacker = 0;
			Log::log(LOG_ERR, "[func=%s] [domainid=%d] [unifid=%lld] enter_domain_num_attacker < 0", __FUNCTION__, domain_id, unifid);
		}
	}
}

void CDS_MNFactionBattleMan::OnDomainBattleEnd(int domain_id, int64_t winner_fid)
{
	Log::formatlog("CDS_MNFactionBattleMan","file=%s\tfunc=%s\tline=%d\tdomain_id=%d\twinner_fid=%lld\n", __FILE__, __FUNCTION__, __LINE__, domain_id, winner_fid);
	DOMAIN_MAP::iterator it = _domain_map.find(domain_id); 
	if(it == _domain_map.end()) return;

	int64_t loser_fid = 0;
	if(it->second.attacker_unifid == winner_fid)
	{
		loser_fid = it->second.defender_unifid;
		it->second.defender_unifid = 0;
	}
	else if(it->second.defender_unifid == winner_fid)
	{
		loser_fid = it->second.attacker_unifid;
		it->second.attacker_unifid = 0;
	}
	else
	{
		Log::log(LOG_ERR, "[%s]CDS_MNFactionBattleMan winner is not defender or attacker", __FUNCTION__); 
		loser_fid = it->second.attacker_unifid;
		winner_fid = it->second.defender_unifid;
		it->second.attacker_unifid = 0; //没有胜负，算是防守方赢
	}

	AnnounceWinner(domain_id,winner_fid,loser_fid);
}

void CDS_MNFactionBattleMan::AnnounceWinner(int domain_id,int64_t winner,int64_t loser)
{
#define MNF_BATTLE_RES_CHAT_MSG_ID  108
	if(winner == 0) return;
	FACTION_MAP::iterator fit = _faction_map.find(winner);
	if(fit == _faction_map.end()) return;
	MNFactionInfo& winner_faction = fit->second;

	struct
	{
		int  domain_id;
		int  winner_zoneid;
		char winner_name[MAX_FACTION_NAME_SIZE];
		int  loser_zoneid;
		char loser_name[MAX_FACTION_NAME_SIZE];
	}data;
	memset(&data,0,sizeof(data));
	data.domain_id = domain_id;
	data.winner_zoneid = winner_faction.zoneid;
	int name_len = winner_faction.faction_name.size();
	if(name_len > MAX_FACTION_NAME_SIZE) name_len = MAX_FACTION_NAME_SIZE;
	memcpy(data.winner_name,winner_faction.faction_name.begin(),name_len);

	fit = _faction_map.find(loser);
	if(fit != _faction_map.end()) 
	{
		data.loser_zoneid = fit->second.zoneid;
		name_len = fit->second.faction_name.size();
		if(name_len > MAX_FACTION_NAME_SIZE) name_len = MAX_FACTION_NAME_SIZE;
		memcpy(data.loser_name,fit->second.faction_name.begin(),name_len);
	}

	Octets msg(&data,sizeof(data));

	CrossChatServer::GetInstance()->OnSend(MNF_BATTLE_RES_CHAT_MSG_ID,GN_CHAT_CHANNEL_SYSTEM,0,Octets(),msg,Octets(),winner_faction.zoneid);
#undef MNF_BATTLE_RES_CHAT_MSG_ID
}

void CDS_MNFactionBattleMan::CalcBonusCredit()
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);

	//检查战斗结果
	for(DOMAIN_MAP::iterator it= _domain_map.begin(); it != _domain_map.end(); ++ it)
	{
		if(it->second.defender_unifid != 0 && it->second.attacker_unifid != 0)
		{
			Log::log(LOG_ERR, "[%s]CDS_MNFactionBattleMan defender[%lld]!= 0 && attacker[%lld]!=0", __FUNCTION__, it->second.defender_unifid, it->second.attacker_unifid);
			it->second.attacker_unifid = 0; //没有胜负，算是防守方赢
		}
	}
	
	CalcBonusCreditA();
	CalcBonusCreditB();
	CalcBonusCreditC();

	//清空进攻方和防守方
	for(DOMAIN_MAP::iterator it= _domain_map.begin(); it != _domain_map.end(); ++ it)
	{
		int64_t unifid = (it->second.attacker_unifid == 0)?(it->second.defender_unifid):(it->second.attacker_unifid); 
		if(unifid != 0)
		{
			it->second.owner_unifid = unifid;
		}
		it->second.attacker_unifid = 0;
		it->second.defender_unifid = 0;
	}
	
	//更新帮派表的domain_num
	UpdateDomainNum();

	//更新排行榜
	CDS_MNToplistMan::GetInstance()->UpdateMNToplist(_faction_map);	

	//更新发给客户端的数据,客户端的数据包含排行榜的信息
	SaveForClientDomainData();

	DebugDumpBonus(__FUNCTION__);
	DebugDumpDomainInfo(__FUNCTION__);
	DebugDumpFactionInfo(__FUNCTION__);
}

void CDS_MNFactionBattleMan::CalcBonusCreditA()
{
	//计算A城奖励
	std::vector<int64_t> rewarded_factions_a;
	for(DOMAIN_MAP::iterator it = _domain_map.begin(); it != _domain_map.end(); ++ it)
	{
		if(it->second.domain_type != DOMAIN_TYPE_A) continue;
		if(it->second.defender_unifid == 0 && it->second.attacker_unifid == 0) continue;
		int64_t winner = (it->second.defender_unifid == 0)?it->second.attacker_unifid: it->second.defender_unifid;
		DEBUG_PRINT("func=%s,domain_id=%d,owner=%lld,attacker=%lld,defender=%lld,winner=%lld", __FUNCTION__, it->second.domain_id, it->second.owner_unifid, it->second.attacker_unifid, it->second.defender_unifid, winner);
		if(winner != 0) it->second.owner_unifid = winner;
		if(std::find(rewarded_factions_a.begin(), rewarded_factions_a.end(), winner) != rewarded_factions_a.end()) continue; //a城已经奖励过了
		rewarded_factions_a.push_back(winner);
		BONUS_MAP::iterator bit = _bonus_map.find(winner);
		if(bit == _bonus_map.end()) 
		{
			Log::log(LOG_ERR, "[func=%s]faction [unifid=%lld] not found in bonus_map", __FUNCTION__, winner);
		}
		else
		{
			bit->second.bonus_sn = _sn;
			bit->second.reward_list[DOMAIN_TYPE_A] = _item_a;

			//帮主奖励
			bit->second.master_reward = _item_master;
		}

		//计算积分
		FACTION_MAP::iterator fit = _faction_map.find(winner);
		if(fit == _faction_map.end())
		{
			Log::log(LOG_ERR, "[func=%s]faction [unifid=%lld] not found in faction map", __FUNCTION__, winner);
			continue;
		}
		fit->second.bonus_sn = _sn;
		int credit = 0;
		GetCredit(it->second.domain_type, credit);
		fit->second.credit += credit;
		int now = Timer::GetTime();
		if(is_same_week(now, fit->second.credit_get_time))
		{
			fit->second.credit_this_week += credit;
		}
		else
		{
			fit->second.credit_get_time = now;
			fit->second.credit_this_week = credit;
		}
		DEBUG_PRINT("func=%s,unifid=%lld,\tfid=%d,\tzoneid=%d,\tdomaina=%d,\tdomainb=%d,\tdomainc=%d,\tcredit=%d,\t,credit_this_week=%d,\tcredit_get_time=%d,\tinvite_count=%d,\taccept_sn=%d,\tbonus_sn=%d,\tversion=%d,\n", __FUNCTION__, fit->second.unifid, fit->second.fid, fit->second.zoneid, fit->second.domain_num[0], fit->second.domain_num[1], fit->second.domain_num[2], fit->second.credit, fit->second.credit_this_week, fit->second.credit_get_time, fit->second.invite_count, fit->second.accept_sn, fit->second.bonus_sn, fit->second.version);
	}
	DebugDumpDomainInfo(__FUNCTION__);
}

void CDS_MNFactionBattleMan::CalcBonusCreditB()
{
	//计算B城奖励
	std::vector<int64_t> rewarded_factions_b;
	for(DOMAIN_MAP::iterator it = _domain_map.begin(); it != _domain_map.end(); ++ it)
	{
		if(it->second.domain_type != DOMAIN_TYPE_B) continue;
		if(it->second.defender_unifid  == it->second.owner_unifid && it->second.attacker_unifid == 0) continue;
		int64_t winner = 0;
		if(it->second.attacker_unifid == 0)
		{
			winner = it->second.defender_unifid;	
		}
		else if(it->second.defender_unifid == 0)
		{
			winner = it->second.attacker_unifid;	
		}
		else
		{
			Log::log(LOG_ERR, "[func=%s]faction [owner=%lld] [defender=0]", __FUNCTION__, it->second.owner_unifid);
			continue;
		}
		DEBUG_PRINT("func=%s,domain_id=%d,owner=%lld,attacker=%lld,defender=%lld,winner=%lld", __FUNCTION__, it->second.domain_id, it->second.owner_unifid, it->second.attacker_unifid, it->second.defender_unifid, winner);
		it->second.owner_unifid = winner;
		if(std::find(rewarded_factions_b.begin(), rewarded_factions_b.end(), winner) != rewarded_factions_b.end()) continue; //b城已经奖励过了
		rewarded_factions_b.push_back(winner);
		BONUS_MAP::iterator bit = _bonus_map.find(winner);
		if(bit == _bonus_map.end()) 
		{
			Log::log(LOG_ERR, "[func=%s]faction [unifid=%lld] not found in bonus_map", __FUNCTION__, winner);
		}
		else
		{
			bit->second.bonus_sn = _sn;
			bit->second.reward_list[DOMAIN_TYPE_B] = _item_b;
		}
		

		//计算积分
		FACTION_MAP::iterator fit = _faction_map.find(winner);
		if(fit == _faction_map.end())
		{
			Log::log(LOG_ERR, "[func=%s]faction [unifid=%lld] not found in faction map", __FUNCTION__, winner);
			continue;
		}
		fit->second.bonus_sn = _sn;
		int credit = 0;
		GetCredit(DOMAIN_TYPE_B, credit);
		fit->second.credit += credit;
		int now = Timer::GetTime();
		if(is_same_week(now, fit->second.credit_get_time))
		{
			fit->second.credit_this_week += credit;
		}
		else
		{
			fit->second.credit_get_time = now;
			fit->second.credit_this_week = credit;
		}
		DEBUG_PRINT("func=%s,unifid=%lld,\tfid=%d,\tzoneid=%d,\tdomaina=%d,\tdomainb=%d,\tdomainc=%d,\tcredit=%d,\t,credit_this_week=%d,\tcredit_get_time=%d,\tinvite_count=%d,\taccept_sn=%d,\tbonus_sn=%d,\tversion=%d,\n", __FUNCTION__, fit->second.unifid, fit->second.fid, fit->second.zoneid, fit->second.domain_num[0], fit->second.domain_num[1], fit->second.domain_num[2], fit->second.credit, fit->second.credit_this_week, fit->second.credit_get_time, fit->second.invite_count, fit->second.accept_sn, fit->second.bonus_sn, fit->second.version);
	}
	DebugDumpDomainInfo(__FUNCTION__);
}

void CDS_MNFactionBattleMan::CalcBonusCreditC()
{
	//计算C成奖励

	for(DOMAIN_MAP::iterator it = _domain_map.begin(); it != _domain_map.end(); ++ it)
	{
		if(it->second.attacker_unifid == 0 && it->second.defender_unifid == 0) continue;
		it->second.owner_unifid = (it->second.defender_unifid == 0)?it->second.attacker_unifid: it->second.defender_unifid;
	}
	
	std::vector<int64_t> rewarded_factions_c;
	for(DOMAIN_MAP::iterator it1 = _domain_map.begin(); it1 != _domain_map.end(); ++ it1)
	{
		if(it1->second.domain_type == DOMAIN_TYPE_A) continue;
		//if(it1->second.defender_unifid == 0 && it1->second.attacker_unifid == 0) continue;
		DEBUG_PRINT("func=%s,domain_id=%d,owner=%lld,attacker=%lld,defender=%lld", __FUNCTION__, it1->second.domain_id, it1->second.owner_unifid, it1->second.attacker_unifid, it1->second.defender_unifid);
		int64_t unifid = it1->second.owner_unifid;
		if(unifid == 0) continue;
		if(std::find(rewarded_factions_c.begin(), rewarded_factions_c.end(), unifid) != rewarded_factions_c.end())
		{
			continue;//已经奖励过了
		}
		rewarded_factions_c.push_back(unifid);
		int domain_c_num = 0;
		int domain_b_num = 0;
		for(DOMAIN_MAP::iterator it2 = it1; it2 != _domain_map.end(); ++ it2)
		{
			if(unifid == it2->second.owner_unifid)
			{
				if(it2->second.domain_type == DOMAIN_TYPE_C)
				{
					++ domain_c_num;		
				}
				else if(it2->second.domain_type == DOMAIN_TYPE_B)
				{
					++ domain_b_num;		
				}
			}
		}
		if(domain_c_num == 0) continue;

		//奖励计算公式
		BONUS_MAP::iterator bit = _bonus_map.find(unifid); 
		if(bit == _bonus_map.end())
		{
			Log::log(LOG_ERR, "[func=%s]faction [unifid=%lld] not found in bonus map", __FUNCTION__, unifid);
		}
		else
		{
			bit->second.bonus_sn = _sn;
			MNDomainBonusRewardItem item = _item_c;
			item.item_num = (domain_c_num + RATIO_P * domain_b_num)*(_item_c.item_num);
			bit->second.reward_list[DOMAIN_TYPE_C] = item;
		}

		//计算积分
		FACTION_MAP::iterator fit = _faction_map.find(unifid);
		if(fit == _faction_map.end())
		{
			Log::log(LOG_ERR, "[func=%s]faction [unifid=%lld] not found in faction map", __FUNCTION__, unifid);
			continue;
		}
		fit->second.bonus_sn = _sn;
		int credit = 0;
		GetCredit(DOMAIN_TYPE_C, credit);
		int total_credit = credit*(domain_c_num + RATIO_P * domain_b_num);
		fit->second.credit += total_credit;
		int now = Timer::GetTime();
		if(is_same_week(now, fit->second.credit_get_time))
		{
			fit->second.credit_this_week += total_credit;
		}
		else
		{
			fit->second.credit_get_time = now;
			fit->second.credit_this_week = total_credit;
		}
		DEBUG_PRINT("func=%s,unifid=%lld,\tfid=%d,\tzoneid=%d,\tdomaina=%d,\tdomainb=%d,\tdomainc=%d,\tcredit=%d,\t,credit_this_week=%d,\tcredit_get_time=%d,\tinvite_count=%d,\taccept_sn=%d,\tbonus_sn=%d,\tversion=%d,\n", __FUNCTION__, fit->second.unifid, fit->second.fid, fit->second.zoneid, fit->second.domain_num[0], fit->second.domain_num[1], fit->second.domain_num[2], fit->second.credit, fit->second.credit_this_week, fit->second.credit_get_time, fit->second.invite_count, fit->second.accept_sn, fit->second.bonus_sn, fit->second.version);
	}
	DebugDumpDomainInfo(__FUNCTION__);
}

void CDS_MNFactionBattleMan::UpdateDomainNum()
{
	for(FACTION_MAP::iterator it = _faction_map.begin(); it != _faction_map.end(); ++ it)
	{
		for(std::vector<char>::iterator nit = it->second.domain_num.begin(); nit != it->second.domain_num.end(); ++ nit)
		{
			*nit = 0;
		}
	}

	for(DOMAIN_MAP::iterator it = _domain_map.begin(); it != _domain_map.end(); ++ it)
	{
		int64_t unifid = it->second.owner_unifid;
		if(unifid == 0) continue;
		FACTION_MAP::iterator fit = _faction_map.find(unifid);
		if(fit == _faction_map.end()) 
		{
			Log::log(LOG_ERR, "CDS_MNFactionBattleMan[%s]ERR:[unifid=%lld] not found\n", __FUNCTION__, unifid);
			continue;
		}

		if(fit->second.domain_num.size() != DOMAIN_TYPE_COUNT)
		{
			Log::log(LOG_ERR, "[func=%s]faction domain_num size error[unifid=%lld][size=%d]", __FUNCTION__, unifid, fit->second.domain_num.size());
			continue;
		}

		++ fit->second.domain_num[it->second.domain_type];
	}
}

void CDS_MNFactionBattleMan::UpdateDBDomainBonus()
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
	MNDomainBonusVector bonus_list;
	for(BONUS_MAP::iterator it = _bonus_map.begin(); it != _bonus_map.end(); )
	{
		if(it->second.reward_list.size() > 0)
		{
			bonus_list.push_back(it->second);
			++ it;
		}
		else
		{
			_bonus_map.erase(it++);
		}
	}
	DBMNPutBattleBonusArg arg;
	arg.sn = ++ _sn;
	arg.bonus_list = bonus_list;
	DBMNPutBattleBonus* rpc =(DBMNPutBattleBonus*)Rpc::Call(RPC_DBMNPUTBATTLEBONUS, arg);
	GameDBClient::GetInstance()->SendProtocol(rpc);
	DebugDumpBonus(__FUNCTION__, arg.bonus_list);
}

void CDS_MNFactionBattleMan::GetCredit(int domain_type, int& credit)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
	if(domain_type == DOMAIN_TYPE_A)
	{
		credit = faction_credit.credit_domain_a; 
	}
	else if(domain_type == DOMAIN_TYPE_B)
	{
		credit = faction_credit.credit_domain_b; 
	}
	else if(domain_type == DOMAIN_TYPE_C)
	{
		credit = faction_credit.credit_domain_c; 
	}
}

void CDS_MNFactionBattleMan::SendDomainBonusData()
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
	for(BONUS_MAP::iterator it = _bonus_map.begin(); it != _bonus_map.end(); ++ it)
	{
		MNDomainSendBonusData data; 
		FACTION_MAP::iterator fit = _faction_map.find(it->first);
		if(fit != _faction_map.end()) 
		{
			int zoneid = fit->second.zoneid;
			data.bonus = it->second;
			CentralDeliveryServer::GetInstance()->DispatchProtocol(zoneid, data);
			DEBUG_PRINT("unifid=%lld,\tbonus_sn=%d,\trolelist.size=%d,\n", data.bonus.unifid, data.bonus.bonus_sn, data.bonus.rolelist.size());
			int index = 0;
			for(MNDomainBonusRewardItemVector::iterator it1 = data.bonus.reward_list.begin(); it1 != data.bonus.reward_list.end(); ++ it1)
			{
				DEBUG_PRINT("reward[%d]:item_id=%d\t, item_num=%d\t, proc_type=%d\t, max_count=%d\n", index++, it1->item_id, it1->item_num, it1->proc_type, it1->max_count);
			}
			DEBUG_PRINT("master reward:item_id=%d\t, item_num=%d\t, proc_type=%d\t, max_count=%d\n", data.bonus.master_reward.item_id, data.bonus.master_reward.item_num, data.bonus.master_reward.proc_type, data.bonus.master_reward.max_count);
		}
	}
}

void CDS_MNFactionBattleMan::SendPlayerLastEnterInfo(int roleid)
{
	PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline(roleid);
	if(pinfo == NULL) return;

	MNGetPlayerLastEnterInfo_Re re;
	re.localsid = pinfo->localsid;
	PLAYER_ENTRY_MAP::iterator it = _player_entry_map.find(roleid);
	if(it == _player_entry_map.end())
	{
		re.retcode = ERR_MNF_PLAYER_NOT_FOUND;
	}
	else
	{
		int domain_id = it->second.domain_id;
		DOMAIN_MAP::iterator dit = _domain_map.find(domain_id);
		if(dit == _domain_map.end())
		{
			re.retcode = ERR_MNF_PLAYER_NOT_FOUND;
		}
		else
		{
			re.retcode		= ERR_SUCCESS;
			re.domain_id	= domain_id;
			re.domain_type	= dit->second.domain_type;
		}
	}

	GDeliveryServer::GetInstance()->Send(pinfo->linksid, re);
}

void CDS_MNFactionBattleMan::OnRecvSendBonusDataResult(int64_t unifid)
{
	DBMNSendBonusNotifyArg arg;
	arg.unifid = unifid;
	DBMNSendBonusNotify* rpc =(DBMNSendBonusNotify*)Rpc::Call(RPC_DBMNSENDBONUSNOTIFY, arg);
	GameDBClient::GetInstance()->SendProtocol(rpc);
}

void CDS_MNFactionBattleMan::OnSendBonusSuccess(int64_t unifid)
{
	BONUS_MAP::iterator it = _bonus_map.find(unifid);
	if(it == _bonus_map.end())
	{
		Log::log(LOG_ERR, "CDS_MNFactionBattleMan[%s]ERR:[unifid=%lld]\n", __FUNCTION__, unifid);
		return;
	}
	DEBUG_PRINT("CDS_MNFactionBattleMan[%s]:erase[unifid=%lld][bonus size=%d]\n", __FUNCTION__, unifid, _bonus_map.size());
	DebugDumpBonus(__FUNCTION__);
	_bonus_map.erase(it);
	DebugDumpBonus(__FUNCTION__);
}

void CDS_MNFactionBattleMan::SendClientDomainData(int roleid)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\tdomain size=%d\n", __FILE__, __FUNCTION__, __LINE__, _domain_data.size());
	PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline(roleid);
	if(pinfo == NULL) return;

	unsigned int linksid = pinfo->linksid;
	unsigned int localsid = pinfo->localsid;
	MNGetDomainData_Re re;
	re.retcode 		= ERR_SUCCESS;
	re.domain_data	= _domain_data;
	re.localsid		= localsid;
	GDeliveryServer::GetInstance()->Send(linksid, re);
}

void CDS_MNFactionBattleMan::SaveForClientDomainData()
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
			data.owner_rank		= CDS_MNToplistMan::GetInstance()->GetMNFactionRank(fit->second.fid, fit->second.zoneid);
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
			data.attacker_rank		= CDS_MNToplistMan::GetInstance()->GetMNFactionRank(attacker_fit->second.fid, attacker_fit->second.zoneid);
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
			data.defender_rank		= CDS_MNToplistMan::GetInstance()->GetMNFactionRank(defender_fit->second.fid, defender_fit->second.zoneid);
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

void CDS_MNFactionBattleMan::SendClientFactionInfo(int roleid, int64_t unifid)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
	PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline(roleid);
	if(pinfo == NULL) return;

	unsigned int linksid = pinfo->linksid;
	unsigned int localsid = pinfo->localsid;
	MNGetFactionInfo_Re re;

	FACTION_MAP::iterator it = _faction_map.find(unifid);
	if(it == _faction_map.end())
	{
		re.retcode = ERR_MNF_FACTION_NOT_FOUND;	
	}
	else
	{
		re.mnfactioninfo = it->second;
		re.retcode		 = ERR_SUCCESS;
	}
	re.localsid = localsid;
	GDeliveryServer::GetInstance()->Send(linksid, re);
}

void CDS_MNFactionBattleMan::Clear()
{
	_apply_map.clear();
	_lvl3_list.clear();
	_player_entry_map.clear();
	for(BONUS_MAP::iterator it = _bonus_map.begin(); it != _bonus_map.end(); ++ it)
	{
		Log::log(LOG_ERR,"Bonus NO Send:unifid=%lld,\tbonus_sn=%d,\trolelist.size=%d,\n", it->second.unifid, it->second.bonus_sn, it->second.rolelist.size());
		int index = 0;
		for(MNDomainBonusRewardItemVector::iterator it1 = it->second.reward_list.begin(); it1 != it->second.reward_list.end(); ++ it1)
		{
			Log::log(LOG_ERR,"reward[%d]:item_id=%d\t, item_num=%d\t, proc_type=%d\t, max_count=%d\n", index++, it1->item_id, it1->item_num, it1->proc_type, it1->max_count);
		}
		Log::log(LOG_ERR,"master reward:item_id=%d\t, item_num=%d\t, proc_type=%d\t, max_count=%d\n", it->second.master_reward.item_id, it->second.master_reward.item_num, it->second.master_reward.proc_type, it->second.master_reward.max_count);
	}
	_bonus_map.clear();
	_chosen_list.clear();
	ClearEnterBattlePlayerNum();
	_has_init_instance_domain_num = 0;
}

void CDS_MNFactionBattleMan::SetAdjustTime(int offset)
{
	time_t now = Timer::GetTime();
	struct tm dt;
	localtime_r(&now, &dt);
	int time_in_a_week = dt.tm_wday * 24 * 3600
		+ dt.tm_hour * 3600
		+ dt.tm_min * 60
		+ dt.tm_sec;
	
	_adjust_time = _apply_begin_time - time_in_a_week + offset;
}

void CDS_MNFactionBattleMan::SetState(int state)
{
	_state = state;
	UpdateDBMNFactionState();
	return;
}

void CDS_MNFactionBattleMan::SetDomainInfo(int domain_id, unsigned int attacker_fid, int attacker_zoneid, unsigned int defender_fid, int defender_zoneid)
{
	DOMAIN_MAP::iterator it = _domain_map.find(domain_id);
	if(it == _domain_map.end()) return;
	
	int64_t attacker = 0;
	for(FACTION_MAP::iterator it = _faction_map.begin(); it != _faction_map.end(); ++ it)
	{
		if(it->second.fid == attacker_fid && it->second.zoneid == attacker_zoneid)
		{
			attacker = it->first;
		}
	}
	if(attacker == 0) 
	{
		DEBUG_PRINT("attacker not found");
		return;
	}

	int64_t defender = 0;
	for(FACTION_MAP::iterator it = _faction_map.begin(); it != _faction_map.end(); ++ it)
	{
		if(it->second.fid == defender_fid && it->second.zoneid == defender_zoneid)
		{
			defender = it->first;
		}
	}
	if(defender == 0)
	{
		DEBUG_PRINT("defender not found");
		return;
	}

	it->second.attacker_unifid = attacker;
	it->second.defender_unifid = defender;

	UpdateDBMNDomainInfo();
	SaveForClientDomainData();
	return;
}

void CDS_MNFactionBattleMan::SetDomainWinner(int domain_id, bool attacker_or_defender)
{
	DOMAIN_MAP::iterator it = _domain_map.find(domain_id);
	if(it == _domain_map.end()) return;

	int64_t winner_fid = 0;
	if(attacker_or_defender)
	{
		winner_fid = _domain_map[domain_id].attacker_unifid;
	}
	else
	{
		winner_fid = _domain_map[domain_id].defender_unifid; 
	}
	OnDomainBattleEnd(domain_id, winner_fid);
}

void CDS_MNFactionBattleMan::SetPlayerBattleEnterSuccess(int roleid, unsigned int fid, int zoneid, int domain_id)
{
	int64_t unifid = 0;
	for(FACTION_MAP::iterator it = _faction_map.begin(); it != _faction_map.end(); ++ it)
	{
		if(it->second.fid == fid && it->second.zoneid == zoneid)
		{
			unifid = it->first;
		}
	}
	if(unifid == 0)
	{
		DEBUG_PRINT("func=%sfaction not found", __FUNCTION__);
		return;
	}

	OnPlayerBattleEnterSuccess(roleid, unifid, domain_id);
}

void CDS_MNFactionBattleMan::SetInitDomainInstance()
{
	_has_init_instance_domain_num = 0;
}

void CDS_MNFactionBattleMan::TestBonus()
{
	//初始化domain_map
	for(DOMAIN_MAP::iterator dit = _domain_map.begin(); dit != _domain_map.end(); ++ dit)
	{
		std::vector<int64_t> unifid_list;
		for(FACTION_MAP::iterator it = _faction_map.begin(); it != _faction_map.end(); ++ it)
		{
			unifid_list.push_back(it->first);
		}

		if(unifid_list.size() == 0) 
		{
			DEBUG_PRINT("ERR TESTBONUS");
			return;
		}

		int index = rand() % unifid_list.size();
		dit->second.owner_unifid = unifid_list[index];
		dit->second.defender_unifid = unifid_list[index];
		unifid_list.erase(unifid_list.begin() + index);

		if(unifid_list.size() == 0) 
		{
			DEBUG_PRINT("ERR TESTBONUS");
			return;
		}
		
		index = rand() % unifid_list.size();
		dit->second.attacker_unifid = unifid_list[index];
	}
	

	//初始化bonus
	OnPlayerBattleEnterSuccess(1068258, 6509273851795341312LL, 0);
	OnPlayerBattleEnterSuccess(1083795, 6509273984268238849LL, 0);
	OnPlayerBattleEnterSuccess(1084048, 6509273987489464322LL, 0);
	
	OnPlayerBattleEnterSuccess(257879, 6581331398840287232LL, 8);
	OnPlayerBattleEnterSuccess(257876, 6581331403152031745LL, 8);
	
	OnPlayerBattleEnterSuccess(257877, 6581331415466508290LL, 24);
	OnPlayerBattleEnterSuccess(257878, 6581331430683443203LL, 24);
	
	DEBUG_PRINT("before");
	DebugDumpDomainInfo(__FUNCTION__);
	DebugDumpBonus(__FUNCTION__);

	//设置胜负方
	for(DOMAIN_MAP::iterator it = _domain_map.begin(); it != _domain_map.end(); ++ it)
	{
		int r = rand() % 2;
		bool attacker_or_defender;
		if(r == 1) attacker_or_defender = true;
		else attacker_or_defender = false;
	
		SetDomainWinner(it->first, attacker_or_defender);
	}

	DEBUG_PRINT("after");
	DebugDumpDomainInfo(__FUNCTION__);

	//开始发奖
	_state = STATE_CALC_BONUS;

	return;
}

void CDS_MNFactionBattleMan::TestUpdateDomainNum()
{
	UpdateDomainNum();
}

void CDS_MNFactionBattleMan::ClearDomainMap()
{
	MNDomainInfoVector domaininfo_list;
	InitDomainInfoMap(domaininfo_list);
	DebugDumpDomainInfo(__FUNCTION__);
}

inline bool CDS_MNFactionBattleMan::is_same_week(time_t t1, time_t t2) 
{ 
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

time_t CDS_MNFactionBattleMan::GetTime()
{
#ifdef MNF_DEBUG
	// now = 活动开启的时间 + offset
	time_t now = Timer::GetTime() + _adjust_time;
#else
	time_t now = Timer::GetTime();
#endif
	return now;
}

void CDS_MNFactionBattleMan::DebugDumpDomainData(const char* func, int line)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\tflag:\n", __FILE__, func, line);
	for(MNDomainDataVector::iterator it = _domain_data.begin(); it != _domain_data.end(); ++ it)
	{
		DEBUG_PRINT("domain_id=%d,\tdomain_type=%d,\towner=%lld,\trank=%d,\tzoneid=%d,\tcredit=%d,\tattacker=%lld,\tattacker_rank=%d,\tattacker_zoneid=%d,\tdefender=%lld,\tdefender_rank%d,\tdefender_zoneid=%d\n", it->domain_id, it->domain_type, it->owner_unifid, it->owner_rank, it->zoneid, it->credit, it->attacker_unifid, it->attacker_rank, it->attacker_zoneid, it->defender_unifid, it->defender_rank, it->defender_zoneid);
	}
}

void CDS_MNFactionBattleMan::DebugDumpDomainInfo(const char* func, MNDomainInfoVector& domaininfo_list)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tdomaininfo_list:size=%d\n", __FILE__, func, domaininfo_list.size());
	for(MNDomainInfoVector::iterator it = domaininfo_list.begin(); it != domaininfo_list.end(); ++ it)
	{
		DEBUG_PRINT("domain_id=%d,\tdomain_type=%d,\towner_unifid=%lld,\tattacker_unifid=%lld,\tdefender_unifid=%lld,\n",it->domain_id, it->domain_type, it->owner_unifid, it->attacker_unifid, it->defender_unifid);
	}
	
}

void CDS_MNFactionBattleMan::DebugDumpDomainInfo(const char* func)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tdomaininfo_map:size=%d\n", __FILE__, func, _domain_map.size());
	for(DOMAIN_MAP::iterator it = _domain_map.begin(); it != _domain_map.end(); ++ it)
	{
		DEBUG_PRINT("domain_id=%d,\tdomain_type=%d,\towner_unifid=%lld,\tattacker_unifid=%lld,\tdefender_unifid=%lld,\tenter_domain_num_defender=%d,\tenter_domain_num_attacker=%d,\t\n",it->second.domain_id, it->second.domain_type, it->second.owner_unifid, it->second.attacker_unifid, it->second.defender_unifid, it->second.enter_domain_num_defender, it->second.enter_domain_num_attacker);
	}
}

void CDS_MNFactionBattleMan::DebugDumpFactionInfo(const char* func, MNFactionInfoVector& factioninfo_list)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tfactioninfo_list:size=%d\n", __FILE__, func, factioninfo_list.size());
	for(MNFactionInfoVector::iterator it = factioninfo_list.begin(); it != factioninfo_list.end(); ++ it)
	{
		DEBUG_PRINT("unifid=%lld,\tfid=%d,\tzoneid=%d,\tdomaina=%d,\tdomainb=%d,\tdomainc=%d,\tcredit=%d,\t,credit_this_week=%d,\tcredit_get_time=%d,\tinvite_count=%d,\taccept_sn=%d,\tbonus_sn=%d,\tversion=%d,\n", it->unifid, it->fid, it->zoneid, it->domain_num[0], it->domain_num[1], it->domain_num[2], it->credit, it->credit_this_week, it->credit_get_time, it->invite_count, it->accept_sn, it->bonus_sn, it->version);
	}
			
}

void CDS_MNFactionBattleMan::DebugDumpFactionInfo(const char* func)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tfactioninfo_map:size=%d\n", __FILE__, func, _faction_map.size());
	for(FACTION_MAP::iterator it = _faction_map.begin(); it != _faction_map.end(); ++ it)
	{
		DEBUG_PRINT("unifid=%lld,\tfid=%d,\tzoneid=%d,\tdomaina=%d,\tdomainb=%d,\tdomainc=%d,\tcredit=%d,\t,credit_this_week=%d,\tcredit_get_time=%d,\tinvite_count=%d,\taccept_sn=%d,\tbonus_sn=%d,\tversion=%d,\n", it->second.unifid, it->second.fid, it->second.zoneid, it->second.domain_num[0], it->second.domain_num[1], it->second.domain_num[2], it->second.credit, it->second.credit_this_week, it->second.credit_get_time, it->second.invite_count, it->second.accept_sn, it->second.bonus_sn, it->second.version);
	}
}

void CDS_MNFactionBattleMan::DebugDumpApplyInfo(const char* func, MNFactionApplyInfoVector& applyinfo_list)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tapplyinfo_list:size=%d\n", __FILE__, func, applyinfo_list.size());
	for(MNFactionApplyInfoVector::iterator it = applyinfo_list.begin(); it != applyinfo_list.end(); ++ it)
	{
		DEBUG_PRINT("unifid=%lld,\tapplicant_id=%d,\tdest=%d,\tcost=%d,\n", it->unifid, it->applicant_id, it->dest, it->cost);
	}
}

void CDS_MNFactionBattleMan::DebugDumpApplyInfo(const char* func)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tapplyinfo_map:size=%d\n", __FILE__, func, _apply_map.size());
	for(APPLYINFO_MAP::iterator it = _apply_map.begin(); it != _apply_map.end(); ++ it)
	{
		DEBUG_PRINT("unifid=%lld,\tapplicant_id=%d,\tdest=%d,\tcost=%d,\n", it->second.unifid, it->second.applicant_id, it->second.dest, it->second.cost);
	}
}

void CDS_MNFactionBattleMan::DebugDumpBonus(const char* func)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\tbonus_map:size=%d\n", __FILE__, __FUNCTION__, __LINE__, _bonus_map.size());
	for(BONUS_MAP::iterator it = _bonus_map.begin(); it != _bonus_map.end(); ++ it)
	{
		DEBUG_PRINT("unifid=%lld,\tbonus_sn=%d,\trolelist.size=%d,\n", it->second.unifid, it->second.bonus_sn, it->second.rolelist.size());
		int index = 0;
		for(MNDomainBonusRewardItemVector::iterator it1 = it->second.reward_list.begin(); it1 != it->second.reward_list.end(); ++ it1)
		{
			DEBUG_PRINT("reward[%d]:item_id=%d\t, item_num=%d\t, proc_type=%d\t, max_count=%d\n", index++, it1->item_id, it1->item_num, it1->proc_type, it1->max_count);
		}
		DEBUG_PRINT("master reward:item_id=%d\t, item_num=%d\t, proc_type=%d\t, max_count=%d\n", it->second.master_reward.item_id, it->second.master_reward.item_num, it->second.master_reward.proc_type, it->second.master_reward.max_count);
	}
}

void CDS_MNFactionBattleMan::DebugDumpBonus(const char* func, MNDomainBonusVector& bonus_list)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\tbonus_list:size=%d\n", __FILE__, __FUNCTION__, __LINE__, bonus_list.size());
	for(MNDomainBonusVector::iterator it = bonus_list.begin(); it != bonus_list.end(); ++ it)
	{
		DEBUG_PRINT("unifid=%lld,\tbonus_sn=%d,\trolelist.size=%d,\n", it->unifid, it->bonus_sn, it->rolelist.size());
		int index = 0;
		for(MNDomainBonusRewardItemVector::iterator it1 = it->reward_list.begin(); it1 != it->reward_list.end(); ++ it1)
		{
			DEBUG_PRINT("reward[%d]:item_id=%d\t, item_num=%d\t, proc_type=%d\t, max_count=%d\n", index++, it1->item_id, it1->item_num, it1->proc_type, it1->max_count);
		}
		DEBUG_PRINT("master reward:item_id=%d\t, item_num=%d\t, proc_type=%d\t, max_count=%d\n", it->master_reward.item_id, it->master_reward.item_num, it->master_reward.proc_type, it->master_reward.max_count);
	}
		
}

void CDS_MNFactionBattleMan::DebugDumpFlag(const char* func, int line)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\tflag:\n", __FILE__, func, line);
	DEBUG_PRINT("_sn=%d,\t", _sn);
	DEBUG_PRINT("_state=%d,\t", _state);
}

void CDS_MNFactionBattleMan::DebugDumpUnifid(const char* str, std::vector<int64_t>& list)
{
	DEBUG_PRINT("file=%s\tstr=%s\tlist:size=%d\n", __FILE__, str, list.size());
	for(std::vector<int64_t>::iterator it = list.begin(); it != list.end(); ++ it)
	{
		DEBUG_PRINT("unifid=%lld", *it);
	}
}

void CDS_MNToplistMan::SendTopList(int zoneid)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\tzoneid=%d\n", __FILE__, __FUNCTION__, __LINE__, zoneid);
	MNFetchTopList_Re re;
	re.toplist = _toplist;
	CentralDeliveryServer::GetInstance()->DispatchProtocol(zoneid, re);
}

void CDS_MNToplistMan::UpdateMNToplist(const MNFactionInfoVector& info_list)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
	_toplist_map.clear();
	for(MNFactionInfoVector::const_iterator it = info_list.begin(); it != info_list.end(); ++ it)
	{
		UpdateMNFactionRank(*it);
	}
	SaveForClientToplist();
}

void CDS_MNToplistMan::UpdateMNToplist(const CDS_MNFactionBattleMan::FACTION_MAP& info_list)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
	_toplist_map.clear();
	for(CDS_MNFactionBattleMan::FACTION_MAP::const_iterator it = info_list.begin(); it != info_list.end(); ++ it)
	{
		UpdateMNFactionRank(it->second);
	}
	SaveForClientToplist();
	
	if(!_toplist_map.empty())
	{
		AnnounceTop(); //通知全服排名第一
	}
}

void CDS_MNToplistMan::AnnounceTop()
{
#define MNF_BATTLE_TOP_CHAT_MSG_ID  109
#define MAX_TIED_TOP_COUNT			5
	if(_toplist_map.empty()) return;

	struct
	{	
		size_t   count;
		size_t   total;
		struct
		{
			int  zoneid; 
			char name[MAX_FACTION_NAME_SIZE];
		}tops[MAX_TIED_TOP_COUNT];
	}data;	
	memset(&data,0,sizeof(data));

	int top_credit = _toplist_map.rbegin()->first; 
	for(TOPLIST_MAP::iterator iter = _toplist_map.lower_bound(top_credit); 
			iter != _toplist_map.upper_bound(top_credit); ++iter)
	{
		if(++data.total <= MAX_TIED_TOP_COUNT)
		{
			data.tops[data.count].zoneid = iter->second.zoneid;
			int name_len = iter->second.faction_name.size();
			if(name_len > MAX_FACTION_NAME_SIZE) name_len = MAX_FACTION_NAME_SIZE;
			memcpy(data.tops[data.count].name,iter->second.faction_name.begin(),name_len);
			++data.count;
		}
	}
	
	Octets msg(&data,sizeof(data));

	CrossChatServer::GetInstance()->OnSend(MNF_BATTLE_TOP_CHAT_MSG_ID,GN_CHAT_CHANNEL_SYSTEM,0,Octets(),msg,Octets(),-1);
#undef MNF_BATTLE_TOP_CHAT_MSG_ID
#undef MAX_TIED_TOP_COUNT	
}

void CDS_MNToplistMan::UpdateMNFactionRank(const MNFactionInfo& info)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__); 
	if(info.credit == 0) return;
	for(TOPLIST_MAP::iterator it = _toplist_map.begin(); it != _toplist_map.end(); )
	{
		if(it->second.fid == info.fid && it->second.zoneid == info.zoneid)
		{
			_toplist_map.erase(it ++);
			break;
		}
		
		it ++;
	}

	MNFactionBriefInfo brief_info;
	brief_info.fid			= info.fid;
	brief_info.faction_name	= info.faction_name;
	brief_info.master_name	= info.master_name;
	brief_info.zoneid		= info.zoneid;
	brief_info.credit		= info.credit;

	if(_toplist_map.size() < TOPLIST_MAX_SIZE)
	{
		_toplist_map.insert(std::make_pair(info.credit, brief_info));
	}
	else if(info.credit > _toplist_map.begin()->first)
	{
		_toplist_map.erase(_toplist_map.begin());
		_toplist_map.insert(std::make_pair(info.credit, brief_info));
	}
}

void CDS_MNToplistMan::SaveForClientToplist()
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
	_toplist.clear();
	for(TOPLIST_MAP::reverse_iterator it = _toplist_map.rbegin(); it != _toplist_map.rend(); ++ it)
	{
		if(_toplist.size() < TOPLIST_MAX_SIZE) _toplist.push_back(it->second);
	}
	DebugDumpToplist(__FUNCTION__);
}

void CDS_MNToplistMan::DebugDumpToplist(const char* func)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, func, __LINE__);
	for(MNFactionBriefInfoVector::iterator it = _toplist.begin(); it != _toplist.end(); ++ it)
	{
		DEBUG_PRINT("CDS_MNToplistMan credit=%d\tfid=%d\tzoneid=%d\n", it->credit, it->fid, it->zoneid);	
	}
}

/*
int CDS_MNToplistMan::GetMNFactionRank(int64_t unifid)
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

int CDS_MNToplistMan::GetMNFactionRank(unsigned int fid, int zoneid)
{
	DEBUG_PRINT("file=%s\tfunc=%s\tline=%dfid=%dzoneid=%d\n", __FILE__, __FUNCTION__, __LINE__, fid, zoneid);
	int index = 1;
	for(MNFactionBriefInfoVector::iterator it = _toplist.begin(); it != _toplist.end(); ++ it, ++ index)
	{
		if(it->fid == fid && it->zoneid == zoneid) return index; 
	}

	return 0;
}
