#include <sstream>

#include "domaindataman.h"
#include "countrybattleman.h"
#include "mapuser.h"
#include "countrybattlesyncplayerlocation.hpp"
#include "countrybattlesyncplayerpos.hpp"
#include "countrybattleconfignotify.hpp"
#include "countrybattlegetmap_re.hpp"
#include "countrybattlegetscore_re.hpp"
#include "countrybattlestart.hpp"
#include "countrybattleenter.hpp"
#include "countrybattlepreenternotify.hpp"
#include "countrybattleresult.hpp"
#include "countrybattlesinglebattleresult.hpp"
#include "dbcountrybattlebonus.hrp"
#include "gdeliveryserver.hpp"
#include "gproviderserver.hpp"
#include "gamedbclient.hpp"
#include "countrybattleapply_re.hpp"
#include "countrybattleapplyentry"
#include "chatmulticast.hpp"
#include "centraldeliveryserver.hpp"
#include "countrybattlekingassignassault_re.hpp"
#include "countrybattlegetbattlelimit_re.hpp"
#include "countrybattlegetkingcommandpoint_re.hpp"
#include "countrybattledestroyinstance.hpp"
#include "countrybattlegetconfig_re.hpp"
#include "crosssystem.h"
#include "parsestring.h"
#include "disabled_system.h"

CountryBattleMan CountryBattleMan::_instance[GROUP_MAX_CNT];
int CountryBattleMan::_default_group;

inline int GetGroupID(int uid) { return (uid >> 16) &0xffff; }
inline int GetBaseID(int uid) { return uid & 0xffff; }
inline int MakeUniqueID(int gid,int id) { return (gid << 16) + id; }

void CountryBattleMan::CountryKing::Init(int roleid_)
{
	roleid = roleid_;
	command_point = BATTLE_CONFIG_KING_POINT;
	for(int i = 0; i < DOMAIN_BATTLE_LIMIT_CNT; ++i) {
		DomainBattleLimit& config = battle_limit_config[i];
		config.domain_id = 0;
		for(int j = 0; j < MAX_OCCUPATION_CNT; ++j) {
			config.limit[j].occupation_cnt_ceil = 0;
			config.limit[j].minor_str_floor = 0;
		}
	}
}

void CountryBattleMan::PlayerEntry::Init(int roleid_, char country_id_, char status_, int in_domain_id_, int prev_domain_id_, int worldtag_, int occupation_, int minor_str_, 
	unsigned int linksid_, unsigned int localsid_)
{
	roleid = roleid_;
	country_id = country_id_;
	status = status_;
	in_domain_id = in_domain_id_;
	prev_domain_id = prev_domain_id_;
	worldtag = worldtag_;
	occupation = occupation_;
	minor_str = minor_str_;
	linksid = linksid_;
	localsid = localsid_;
}

void CountryBattleMan::DomainInfo::Init(int id_, char owner_, char challenger_, char init_country_id_, char wartype_, char status_)
{
	id = id_;
	owner = owner_;
	challenger = challenger_;
	init_country_id = init_country_id_;
	wartype = wartype_;
	status = status_;
	
	memset(battle_config_mask, 0, sizeof(battle_config_mask));
	memset(owner_occupation_cnt, 0, sizeof(owner_occupation_cnt));
	memset(challenger_occupation_cnt, 0, sizeof(challenger_occupation_cnt));

	time = 0; //Õı³£×´Ì¬¸ÃÖµÎŞÓÃ£ ³õÊ¼»¯±ØÈ»ÊÇÕı³£×´Ì¬
}

void CountryBattleMan::MoveInfo::Init(int roleid_, int from_, int to_, int time_)
{
	roleid = roleid_;
	from = from_;
	to = to_;
	time = time_;
}

void CountryBattleMan::ColdInfo::Init(int id_, int time_)
{
	id = id_;
	time = time_;
	is_notify = false;
}

void CountryBattleMan::RegisterInfo::Init(int war_type_, int world_tag_, int server_id_)
{
	war_type = war_type_;
	world_tag = world_tag_;
	server_id = server_id_;
}

void CountryBattleMan::PlayerScoreInfo::Init(int roleid_, char country_id_, int win_cnt_, int dead_cnt_, int total_combat_time_, int total_contribute_val_, float score_)
{
	roleid = roleid_;
	country_id = country_id_;
	win_cnt = win_cnt_;
	dead_cnt = dead_cnt_;
	total_combat_time = total_combat_time_;
	total_contribute_val = total_contribute_val_,
	score = score_;
}

int CountryBattleMan::GetServerIdByDomainId(int domain_id)
{
	int server_id = -1;
	
	DOMAIN2_INFO_SERV* pData = domain2_data_getbyid(domain_id);
	if(pData == NULL) return -1;
	
	for(unsigned int i = 0; i < _servers.size(); ++i) {
		if(_servers[i].war_type == pData->wartype) {
			server_id = _servers[i].server_id;
			break;
		}
	}

	return server_id;
}

int CountryBattleMan::GetServerIdByWorldTag(int world_tag)
{
	int server_id = -1;
	for(unsigned int i = 0; i < _servers.size(); ++i) {
		if(_servers[i].world_tag == world_tag) {
			server_id = _servers[i].server_id;
			break;
		}
	}

	return server_id;
}

int CountryBattleMan::GetWorldTagByDomainId(int domain_id)
{
	int world_tag = -1;
	DOMAIN2_INFO_SERV* pData = domain2_data_getbyid(domain_id);
	if(pData == NULL) return -1;

	for(unsigned int i = 0; i < _servers.size(); ++i) {
		if(_servers[i].war_type == pData->wartype) {
			world_tag = _servers[i].world_tag;
			break;
		}
	}

	return world_tag;
}

bool CountryBattleMan::InitDomainInfo(char domain2_type)
{
	if(domain2_data_load(domain2_type) != 0) return false;
	const std::vector<DOMAIN2_INFO_SERV>* pVec = get_domain2_infos();
	if(pVec == NULL) return false;

	for(unsigned int i = 0; i < pVec->size(); ++i) {
		const DOMAIN2_INFO_SERV& info = (*pVec)[i];
		if(info.country_id <= 0 || info.country_id > COUNTRY_MAX_CNT) return false;
		if(info.wartype >= WAR_TYPE_MAX) return false;

		DomainInfo domain;
		domain.Init(info.id, info.country_id, 0, info.country_id, info.wartype, DOMAIN_STATUS_NORMAL);

		_domain_map[info.id] = domain;

		if(info.is_capital) {
			CapitalInfo capital;
			capital.id = info.id;
			for(int i = 0; i < 4; ++i) {
				capital.pos[i].x = info.mappos[i][0];
				capital.pos[i].y = info.mappos[i][1];
				capital.pos[i].z = info.mappos[i][2];

			}
			_capital_info[info.country_id - 1] = capital;
		}
	}
	
	return true;
}

bool CountryBattleMan::Initialize(int gid,bool arrange_country_by_zoneid)
{
	if(DisabledSystem::GetDisabled(SYS_COUNTRYBATTLE))
		return true;
	
	char domain2_type = DOMAIN2_TYPE_SINGLE;
	if(arrange_country_by_zoneid) domain2_type = DOMAIN2_TYPE_CROSS;

	_group_index = gid;
	//µ¼ÈëdomainÊı¾İ
	if(!InitDomainInfo(domain2_type)) 
	{
		Log::log( LOG_ERR, "InitCountryBattle[%d], int domain data error.", _group_index );
		return false;
	}
	
	//_status = ST_OPEN;
	_servers.clear();	

	Conf* conf = Conf::GetInstance();
		
	std::string key = "COUNTRYBATTLE";
	_occupation_fac_list.clear();
	for(unsigned int i = 0; i < MAX_OCCUPATION_CNT; ++i) {
		OccupationFactor factor;
		std::string num = "";
		std::stringstream ss;
		ss << i;
		ss >> num;
		std::string value = conf->find(key, "occupation" + num);
		
		int n = sscanf(value.c_str(), "%f;%f;%f;%f;%f;%f", 
			&factor.win_fac, &factor.fail_fac, &factor.attend_time_fac, &factor.kill_cnt_fac, &factor.death_cnt_fac, &factor.combat_time_fac);
		if(n != 6) {
			Log::log( LOG_ERR, "InitCountryBattle[%d], occupations factor error.", _group_index );
			return false;
		}
		
		_occupation_fac_list.push_back(factor);
	}
	
	_bonus_limit.score_limit = atof(conf->find(key, "score_limit").c_str());
	_bonus_limit.win_cnt_limit = atoi(conf->find(key, "win_count_limit").c_str());
	_bonus_limit.death_cnt_limit = atoi(conf->find(key, "death_count_limit").c_str());
	_bonus_limit.combat_time_limit = atoi(conf->find(key, "combat_time_limit").c_str());
	_bonus_limit.contribute_val_limit = atoi(conf->find(key, "contribute_val_limit").c_str());

	{
		std::string sbonus = conf->find(key, "total_bonus").c_str();
		int vbonus[GROUP_MAX_CNT] = {0};
		int nbonus = sscanf(sbonus.c_str(), "%d;%d;%d;%d;%d;%d;%d;%d;%d;%d", 
				&vbonus[0],&vbonus[1],&vbonus[2],&vbonus[3],&vbonus[4],
				&vbonus[5],&vbonus[6],&vbonus[7],&vbonus[8],&vbonus[9]);
		if(_group_index > nbonus)
			_bonus_limit.total_bonus = 0;
		else 
			_bonus_limit.total_bonus = vbonus[_group_index];

		LOG_TRACE("CountryBattleMan[%d], total_bonus=%d.", _group_index,_bonus_limit.total_bonus);
	}

	if(arrange_country_by_zoneid) {
		_arrange_country_type = ARRANGE_COUNTRY_BY_ZONEID;
		std::vector<int> zone_list;
		CentralDeliveryServer::GetInstance()->GetAcceptedZone(_group_index,zone_list);

		//¿ç·ş¹úÕ½£¬¹ú¼ÒÊıÁ¿Ó¦¸ÃÔÚ[2, COUNTRY_MAX_CNT]Ö®¼ä
		if(zone_list.size() < 2 || zone_list.size() > COUNTRY_MAX_CNT) {
			Log::log( LOG_ERR, "InitCountryBattle[%d], zone list size=%d invalid.", _group_index,zone_list.size() );
			return false;
		}

		for(unsigned int i = 0; i < zone_list.size(); ++i) {
			int zoneid = zone_list[i];
			_zone_country_map[zoneid] = 0;
		}
	} else {
		_arrange_country_type = ARRANGE_COUNTRY_RANDOM;
	}
	
	std::string open_day_str = conf->find(key, "open_day");
	std::vector<string> open_day_list;
	if(!ParseStrings(open_day_str, open_day_list)) {
		Log::log( LOG_ERR, "InitCountryBattle[%d], open_day error.", _group_index );
		return false;
	}
	if(open_day_list.size() > WEEK_DAY_CNT) {
		Log::log( LOG_ERR, "InitCountryBattle[%d], open_day_list count is invalid.", _group_index );		
		return false;
	}
	for(unsigned int i = 0; i < open_day_list.size(); ++i) {
		int day = -1;
		char  gr[128] = {0};
		int npos = sscanf(open_day_list[i].c_str(),"%d->%127s",&day,gr);
		
		bool flag = false;
		if(npos == 1 && day >=0 && day <= WEEK_DAY_CNT)	// ¼æÈİÀÏ¸ñÊ½
		{
			flag = true;	
		}
		else if(npos == 2 && day >= 0 && day <= WEEK_DAY_CNT) 
		{
			char* token = strtok( gr, ":");	
			while( NULL != token && !flag) {
				if(atoi(token) == _group_index) 
					flag = true;
				token = strtok( NULL, ":");
			}
		}
		else
		{	
			Log::log( LOG_ERR, "InitCountryBattle[%d], open_day is invalid.", _group_index );
			return false;
		}

		if(flag) 
		{
			_open_days[day] = 1;
			std::vector<int> zone_list;
			CentralDeliveryServer::GetInstance()->GetAcceptedZone(_group_index,zone_list);
			
			if(!zone_list.empty())
			{
				CrossGuardServer::GetInstance()->Register(CT_COUNTRY_BATTLE,day,
					COUNTRYBATTLE_START_TIME- 5*60,
					COUNTRYBATTLE_CLEAR_TIME+ 15*60,&zone_list[0],zone_list.size());	
			}
		}
	}
	
	IntervalTimer::Attach(this, 1000000/IntervalTimer::Resolution());
	LOG_TRACE( "CountryBattle Init Successfully!!\n");
	return true;
}

float CountryBattleMan::ConvertMajorStrength(int major_str)
{
	float ret = 0.0f;
	
	switch(major_str)
	{
		case MAJOR_STR_NONE:
			ret = 0.0f;
			break;
		case MAJOR_STR_REFINE16: //ÓµÓĞ16Æ·ÎäÆ÷µÄÍæ¼Ò
			ret = 1.0f;
			break;
		case MAJOR_STR_FORCE9: //ÓµÓĞ9¾üÎäÆ÷µÄÍæ¼Ò
			ret = 1.5f;
			break;
		default:
			ret = 0.0f;
	}

	return ret;
}

int CountryBattleMan::ArrangeCountry(bool has_major_str)
{
	int country_id = 0;
	bool is_equal = true;
	
	int min_str_country = 0;
	if(has_major_str) { //ÓµÓĞÖ÷ÒªÕ½Á¦
		for(int i = 1; i < COUNTRY_MAX_CNT; ++i) {
			if( is_equal && (_country_info[i].major_strength != _country_info[i - 1].major_strength) ) is_equal = false;
			
			if(_country_info[i].major_strength < _country_info[min_str_country].major_strength) { //Ñ°ÕÒÖ÷ÒªÕ½Á¦×îĞ¡µÄÕóÓª
				is_equal = false;
				min_str_country = i;
			}
		}
	} else { //½öÓµÓĞ´ÎÒªÕ½Á¦
		for(int i = 1; i < COUNTRY_MAX_CNT; ++i) { 
			if( is_equal && (_country_info[i].minor_strength != _country_info[i - 1].minor_strength) ) is_equal = false;

			if(_country_info[i].minor_strength < _country_info[min_str_country].minor_strength) { //Ñ°ÕÒ´ÎÒªÕ½Á¦×îĞ¡µÄÕóÓª
				is_equal = false;
				min_str_country = i;
			}
		}
	}

	if(is_equal) { //ÕóÓªÕ½Á¦Í¬µÈ
		country_id = rand() % COUNTRY_MAX_CNT + 1; //Ëæ»úÒ»¸öÕóÓª
	} else {
		country_id = min_str_country + 1;
	}

	return country_id;
}

void CountryBattleMan::ArrangeCountryByZoneID()
{
	std::set<int> _tmp_set;
	std::string result = "";
	
	for(ZONE_COUNTRY_MAP::iterator it = _zone_country_map.begin(); it != _zone_country_map.end(); ++it) {
		int zoneid = it->first;
		
		int country_id = 0;
		do {
			country_id = rand() % COUNTRY_MAX_CNT + 1;
		} while(_tmp_set.find(country_id) != _tmp_set.end());

		_tmp_set.insert(country_id);
		it->second = country_id;
		
		//ÕûÀí·ÖÅä½á¹ûµÄ×Ö·û´®£¬³ölogÓÃ
		std::string tmp = "";
		std::stringstream ss;
		ss << "zoneid=" << zoneid << "(country_id=" << country_id << "),";
		ss >> tmp;
		
		result += tmp;	
	}
	
	DEBUG_PRINT("CountryBattle CentralDSArrangeCountry, %s\n", result.c_str());
}

bool CountryBattleMan::IsPlayerMoving(int roleid)
{
	for(MOVE_LIST::iterator it = _move_info.begin(); it != _move_info.end(); ++it) {
		if(it->roleid == roleid) return true;
	}

	return false;
}

void CountryBattleMan::ClearMoveInfo(int roleid)
{
	for(MOVE_LIST::iterator it = _move_info.begin(); it != _move_info.end(); ++it) {
		if(it->roleid == roleid) {
			it = _move_info.erase(it);
			break;
		}
	}
}

void CountryBattleMan::ClearPlayerWaitInfo(int roleid)
{
	for(PLAYER_WAIT_FIGHT_LIST::iterator it = _players_wait_fight.begin(); it != _players_wait_fight.end(); ++it) {
		if(it->id == roleid) {
			it = _players_wait_fight.erase(it);
			break;
		}
	}
}

void CountryBattleMan::ClearPlayerInfo(int roleid)
{
	_player_map.erase(roleid);
	ClearMoveInfo(roleid);
	ClearPlayerWaitInfo(roleid);
}

void CountryBattleMan::ClearCountryInfo()
{
	for(int i = 0; i < COUNTRY_MAX_CNT; ++i) {
		CountryInfo& info = _country_info[i];
		info.major_strength = 0;
		info.minor_strength = 0;
		info.online_player_cnt = 0;
		info.domains_cnt = 0;
		info.country_scores = 0;
		info.player_total_scores = 0;

		info.king_info.Init(0);
		info.communication_map.clear();
	}
}

void CountryBattleMan::ResetDomainInfo()
{
	for(DOMAIN_MAP::iterator it = _domain_map.begin(); it != _domain_map.end(); ++it) {
		DomainInfo& domain = it->second;
		domain.Init(domain.id, domain.init_country_id, 0, domain.init_country_id, domain.wartype, DOMAIN_STATUS_NORMAL);
		domain.player_list.clear();
	}

}

void CountryBattleMan::PlayerChangeState(PlayerEntry& player, char new_status, int reason)
{
	LOG_TRACE("CountryBattle Player Change State: roleid=%d, old_status=%d, new_status=%d, reason=%d", player.roleid, player.status, new_status, reason);
	//¸üĞÂÍæ¼Ò×´Ì¬	
	player.status = new_status;
	int roleid = player.roleid;

	switch(new_status) {
		case PLAYER_STATUS_NORMAL:
		case PLAYER_STATUS_FIGHT:
		{
			ClearMoveInfo(roleid); //Çå³ıÍæ¼ÒµÄÒÆ¶¯ĞÅÏ¢
			ClearPlayerWaitInfo(roleid); //Çå³ıÍæ¼ÒµÈ´ıÕ½ÕùµÄĞÅÏ¢
			break;
		}
		case PLAYER_STATUS_WAIT_FIGHT:
		{
			//½«Íæ¼Ò¼ÓÈëµ½µÈ´ıÕ½ÕùµÄÁĞ±í
			bool is_wait = false;
			for(unsigned int i = 0; i < _players_wait_fight.size(); ++i) {
				if(_players_wait_fight[i].id == player.roleid) {
					is_wait = true;
					break;
				}
			}
			
			if(!is_wait) {
				ColdInfo player_wait;
				player_wait.Init(roleid, PLAYER_WAIT_TIME);
				_players_wait_fight.push_back(player_wait);
			}
			break;
		}
		default:
		{
			Log::log( LOG_ERR, "CountryBattle[%d] Player state error, state=%d", _group_index, new_status);
			break;
		}
	}
}

void CountryBattleMan::DomainChangeState(DomainInfo& domain, char new_status, int reason)
{
	LOG_TRACE("CountryBattle Domain Change State: domain_id=%d, old_status=%d, new_status=%d, reason=%d", domain.id, domain.status, new_status, reason);
	//¸üĞÂdomain×´Ì¬
	domain.status = new_status;
	
	switch(new_status) {
		case DOMAIN_STATUS_NORMAL:
			domain.challenger = 0;
			domain.time = 0;
			break;
		case DOMAIN_STATUS_WAIT_FIGHT:
			domain.time = DOMAIN_WAIT_TIME;
			break;
		case DOMAIN_STATUS_FIGHT:
			domain.time = Timer::GetTime();
			break;
		case DOMAIN_STATUS_COLD:
			domain.challenger = 0;
			domain.time = DOMAIN_COLD_TIME;
			break;
		default:
			Log::log( LOG_ERR, "CountryBattle[%d] Domain state error, state=%d", _group_index, new_status);
			break;
	}
}

int CountryBattleMan::ApplyBattleRandom( std::vector<CountryBattleApplyEntry>& apply_list )
{
	if(_country_id_ctrl > 0) return _country_id_ctrl; //DEBUGÃüÁî£¬µ±´ËÖµ±»ÉèÖÃÊ±£¬Ö±½Ó·ÖÅä¸ÃÕóÓª

	bool has_major_str = false;
	for(unsigned int i = 0; i < apply_list.size(); ++i) { //ÅĞ¶ÏÊÇ·ñÓµÓĞÖ÷ÒªÕ½Á¦
		if((apply_list[i].major_strength > MAJOR_STR_NONE) && (apply_list[i].major_strength < MAJOR_STR_MAX)) {
			has_major_str = true;
			break;
		}
	}

	//¸øÍæ¼Ò·ÖÅäÕóÓª
	int country_id = ArrangeCountry(has_major_str);	
	return country_id;
}

bool CountryBattleMan::PlayerJoinBattle(int roleid, int country_id, int world_tag, int minor_str)
{
	if(country_id <= 0 || country_id > COUNTRY_MAX_CNT) return false;
	
	CapitalInfo& capital = _capital_info[country_id - 1];
	DOMAIN_MAP::iterator it_domain = _domain_map.find(capital.id);
	if(it_domain == _domain_map.end()) return false;

	unsigned int linksid = 0;
	unsigned int localsid = 0;
	int occupation = 0;
	
	{
		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
		PlayerInfo* pInfo = UserContainer::GetInstance().FindRoleOnline( roleid );
		if (pInfo != NULL) {
			linksid = pInfo->linksid;
			localsid = pInfo->localsid;
			occupation = pInfo->cls;
		} else {
			return false;
		}
	}

	PLAYER_ENTRY_MAP::iterator it = _player_map.find(roleid);
	if(it != _player_map.end()) {
		PlayerLeaveBattle(roleid, country_id);
	}
	
	DomainInfo& domain = it_domain->second;
	domain.player_list.push_back(roleid);
	
	_player_map[roleid].Init(roleid, country_id, PLAYER_STATUS_NORMAL, capital.id, capital.id, world_tag, occupation, minor_str, linksid, localsid);
	
	AddPlayerCountryCommuicationInfo(roleid, country_id, linksid, localsid);

	++_country_info[country_id - 1].online_player_cnt;
	return true;
}

bool CountryBattleMan::PlayerLeaveBattle(int roleid, int country_id)
{
	if(country_id <= 0 || country_id > COUNTRY_MAX_CNT) return false;
	PLAYER_ENTRY_MAP::iterator it_player = _player_map.find(roleid);
	if(it_player == _player_map.end()) return false;
	
	PlayerEntry& player = it_player->second;
	int in_domain_id = player.in_domain_id;
	unsigned int linksid = player.linksid;

	RemovePlayerCountryCommuicationInfo(roleid, country_id, linksid);

	DOMAIN_MAP::iterator it_domain = _domain_map.find(in_domain_id);
	if(it_domain == _domain_map.end()) return false;
	DomainInfo& domain = it_domain->second;
	
	for(std::vector<int>::iterator it = domain.player_list.begin(); it != domain.player_list.end(); ++it) {
		if(*it == roleid) {
			it = domain.player_list.erase(it); //Çå³ıÁìµØÍæ¼ÒÁĞ±íÖĞ£¬Íæ¼ÒµÄID
			break;
		}
	}
	
	if(player.status == PLAYER_STATUS_FIGHT) { //´ÓÕ½³¡³¡¾°µôÏß
		if(player.country_id == domain.owner) {
			if(--domain.owner_occupation_cnt[player.occupation] < 0) {
				domain.owner_occupation_cnt[player.occupation] = 0;
			}
		} else {
			if(--domain.challenger_occupation_cnt[player.occupation] < 0) {
				domain.challenger_occupation_cnt[player.occupation] = 0;
			}
		}
	}

	ClearPlayerInfo(roleid); //Çå³ı¹úÕ½ÏµÍ³ÖĞÍæ¼ÒµÄ¸÷ÖÖĞÅÏ¢
	--_country_info[country_id - 1].online_player_cnt;
	return true;
}

void CountryBattleMan::JoinBattle(int roleid, int country_id, int world_tag, int major_strength, int minor_strength, char is_king)
{
	if(!PlayerJoinBattle(roleid, country_id, world_tag, minor_strength)) return;
	
	//Íæ¼Ò¼ÓÈë¹úÕ½£¬ÏàÓ¦Ôö¼Ó¸ÃÍæ¼ÒÕóÓªµÄÖ÷ÒªºÍ´ÎÒªÕ½Á¦Öµ
	CountryInfo& country = _country_info[country_id - 1];
	country.major_strength += ConvertMajorStrength(major_strength);
	country.minor_strength += minor_strength;

	if((_arrange_country_type == ARRANGE_COUNTRY_BY_ZONEID) && is_king) {
		country.king_info.Init(roleid);
	}
}

void CountryBattleMan::LeaveBattle(int roleid, int country_id, int major_strength, int minor_strength)
{
	if(!PlayerLeaveBattle(roleid, country_id)) return;
	
	//Íæ¼ÒÀë¿ª¹úÕ½£¬ÏàÓ¦¼õÉÙ¸ÃÍæ¼ÒÕóÓªµÄÖ÷ÒªºÍ´ÎÒªÕ½Á¦Öµ
	CountryInfo& country = _country_info[country_id - 1];
	
	float major_str = ConvertMajorStrength(major_strength);
	if((country.major_strength - major_str) > 0) {
		country.major_strength -= major_str;
	} else {
		country.major_strength = 0;
	}
	
	if((country.minor_strength - minor_strength) > 0) {
		country.minor_strength -= minor_strength;
	} else {
		country.minor_strength = 0;
	}
}

void CountryBattleMan::PlayerLogin(int roleid, int country_id, int world_tag, int minor_str, char is_king)
{
	PlayerJoinBattle(roleid, country_id, world_tag, minor_str);

	CountryInfo& country = _country_info[country_id - 1];
	if((_arrange_country_type == ARRANGE_COUNTRY_BY_ZONEID) && (country.king_info.roleid <= 0) && is_king) {
		country.king_info.Init(roleid);
	}
}

void CountryBattleMan::PlayerLogout(int roleid, int country_id)
{
	PlayerLeaveBattle(roleid, country_id);
}

void CountryBattleMan::PlayerEnterMap(int roleid, int worldtag)
{
	PLAYER_ENTRY_MAP::iterator it_player = _player_map.find(roleid);
	if(it_player == _player_map.end()) return;
	
	PlayerEntry& player = it_player->second;
	player.worldtag = worldtag;
	
	DOMAIN_MAP::iterator it_domain = _domain_map.find(player.in_domain_id);
	if(it_domain == _domain_map.end()) {
		SysHandlePlayer(player.roleid, REASON_SYS_HANDLE_INVALID_DOMAIN);
		return;
	}
	
	DomainInfo& domain = it_domain->second;

	if(worldtag != _capital_worldtag) { 
		int server_id = GetServerIdByWorldTag(worldtag);
		if(server_id >= 0) { //½øÈëÕ½³¡³¡¾°, ¸Ä±äÍæ¼Ò×´Ì¬
			PlayerChangeState(player, PLAYER_STATUS_FIGHT, REASON_PLAYER_ENTER_BATTLE);
			
			if(player.country_id == domain.owner) {
				++domain.owner_occupation_cnt[player.occupation];
			} else {
				++domain.challenger_occupation_cnt[player.occupation];
			}
			
		} else { //¼È·Ç¹úÕ½Õ½ÂÔ³¡¾°£¬ÓÖ²»ÊÇÕ½³¡³¡¾°£¬ËµÃ÷Íæ¼Ò²¢²»ÊÇÍ¨¹ıÕı³£µÄNPCÍ¾¾¶Ìø³öÁË¹úÕ½
			HandlePlayerUnusualSwitchMap(player);
		}
	} else { //½øÈë¹úÕ½Õ½ÂÔ³¡¾°
		if(player.status == PLAYER_STATUS_FIGHT) { //´ÓÕ½³¡³¡¾°ÍË³öµ½¹úÕ½Õ½ÂÔ³¡¾°
			PlayerChangeState(player, PLAYER_STATUS_NORMAL, REASON_PLAYER_LEAVE_BATTLE);

			//Õ½³¡·ÇÀäÈ´£¬ËµÃ÷»¹ÔÚÕ½¶·£¬±ØĞëÒª±»ÏµÍ³À­×ß
			//Õ½³¡ÀäÈ´×´Ì¬£¬ËµÃ÷Õ½¶·½áÊø£¬´ËÊ±Õ½°ÜµÄÑ¡ÊÖÒª±»ÏµÍ³À­×ß
			if((domain.status != DOMAIN_STATUS_COLD) || (domain.owner != player.country_id)) {
				SysHandlePlayer(player.roleid, REASON_SYS_HANDLE_LEAVE_BATTLE);
			}

			if(player.country_id == domain.owner) {
				if(--domain.owner_occupation_cnt[player.occupation] < 0) {
					domain.owner_occupation_cnt[player.occupation] = 0;
				}
			} else {
				if(--domain.challenger_occupation_cnt[player.occupation] < 0) {
					domain.challenger_occupation_cnt[player.occupation] = 0;
				}
			}
		} else {
			//´Ó´óÊÀ½ç½øÈë¹úÕ½Õ½ÂÔ³¡¾°µÄÂß¼­ÔÚJoinBattleº¯Êı
			//Ä¿Ç°³ıÁËÕı³£µÄ¼ÓÈë¹úÕ½£¬²»Ó¦¸Ã´æÔÚÍæ¼ÒÄÜ´ÓÍâÃæÌø×ªµ½¹úÕ½Õ½ÂÔ³¡¾°µÄÂß¼­
			HandlePlayerUnusualSwitchMap(player);
			Log::log( LOG_ERR, "CountryBattle[%d] an unusual map switch occured, roleid=%d", _group_index, roleid);
		}
	}
}

int CountryBattleMan::GetMoveUseTime(int domain_src, int domain_dest)
{
	DOMAIN2_INFO_SERV* pDomain = domain2_data_getbyid(domain_src);
	if(pDomain == NULL) return 0;
	
	std::vector<int>& neighbours = pDomain->neighbours;	
	unsigned int i = 0;
	for(i = 0; i < neighbours.size(); ++i) {
		if(neighbours[i] == domain_dest) break;
	}

	if(i >= neighbours.size()) return 0;
	return pDomain->time_neighbours[i];
}

void CountryBattleMan::MoveBetweenDomain(int roleid, int domain_src, int domain_dest)	
{
	DOMAIN_MAP::iterator it_src_domain = _domain_map.find(domain_src);
	if(it_src_domain == _domain_map.end()) return;
	
	DomainInfo& src_domain = it_src_domain->second;
	std::vector<int>::iterator it = src_domain.player_list.begin();
	while(it != src_domain.player_list.end()) {
		if(*it == roleid) {
			it = src_domain.player_list.erase(it); //½«Íæ¼Ò´ÓÔ´ÁìµØµÄÁĞ±íÖĞÉ¾³ı
			break;
		}

		++it;
	}

	DOMAIN_MAP::iterator it_dest_domain = _domain_map.find(domain_dest);
	if(it_dest_domain == _domain_map.end()) return;
	
	DomainInfo& dest_domain = it_dest_domain->second;
	dest_domain.player_list.push_back(roleid); //½«Íæ¼Ò¼ÓÈëµ½Ä¿±êÁìµØµÄÁĞ±íÖĞ

	LOG_TRACE("CountryBattle player arrived in new domain: roleid=%d, src_domain=%d, new_domain=%d, idx=%d", 
		roleid, domain_src, domain_dest, dest_domain.player_list.size() - 1);
}

void CountryBattleMan::PlayerMove(int roleid, int from, int to, int time)
{
	bool is_player_moving = false;
	for(unsigned int i = 0; i < _move_info.size(); ++i) {
		if(_move_info[i].roleid == roleid) {
			//Íæ¼ÒÒÑ¾­ÔÚÒÆ¶¯ÖĞ£¬¸üĞÂÒÆ¶¯ĞÅÏ¢
			_move_info[i].Init(roleid, from, to, time);
			is_player_moving = true;
			break;
		}
	}
	
	//Íæ¼ÒÔ­À´²¢Î´ÒÆ¶¯£¬½«Íæ¼Ò¼ÓÈëµ½ÒÆ¶¯ÁĞ±í
	if(!is_player_moving) {
		MoveInfo move;
		move.Init(roleid, from, to, time);
		_move_info.push_back(move);
	}
}

int CountryBattleMan::PlayerMove(int roleid, int dest)
{
	int ret = -1;
	if(_status != ST_OPEN) {
		Log::log( LOG_ERR, "CountryBattle[%d] move error, invalid state, state=", _group_index, _status);
		return ret;
	}

	PLAYER_ENTRY_MAP::iterator it_player = _player_map.find(roleid);
	if(it_player == _player_map.end()) {
		Log::log( LOG_ERR, "CountryBattle[%d] move error, can not find player", _group_index);
		return ret; //ÔÚ¹úÕ½ÖĞÕÒ²»µ½¸ÃÍæ¼Ò
	}
	
	PlayerEntry& player = it_player->second;
	if(IsPlayerKing(player)) {
		Log::log( LOG_ERR, "CountryBattle[%d] move error, king can not move", _group_index);
		return ret; //¹úÍõ²»¿ÉÒÔÒÆ¶¯
	}
	if(player.worldtag != _capital_worldtag) {
		Log::log( LOG_ERR, "CountryBattle[%d] move error, not in capital", _group_index);
		return ret; //Íæ¼ÒÖ»ÓĞ´¦ÓÚ¹úÕ½Õ½ÂÔ³¡¾°£¬²Å¿ÉÒÔÒÆ¶¯
	}
	
	DOMAIN_MAP::iterator it_src_domain = _domain_map.find(player.in_domain_id);
	if(it_src_domain == _domain_map.end()) {
		Log::log( LOG_ERR, "CountryBattle[%d] move error, invalid src domain, domain_id=%d", _group_index, player.in_domain_id);
		return ret;
	}

	DomainInfo& src_domain = it_src_domain->second;
	//Íæ¼Ò´¦ÓÚ·Ç×Ô¼ºÕóÓªµÄÁìÍÁ£¬²»¿ÉÒÔÒÆ¶¯
	//Íæ¼Ò´¦ÓÚ×Ô¼ºÕóÓªµÄÁìÍÁ£¬½öµ±ÁìÍÁ´¦ÓÚÕı³£×´Ì¬£¬»òÀäÈ´×´Ì¬²Å¿ÉÒÔÒÆ¶¯£¬ÆäËû×´Ì¬²»ÄÜÒÆ¶¯
	if( (src_domain.owner != player.country_id) || ((src_domain.status != DOMAIN_STATUS_NORMAL) && (src_domain.status != DOMAIN_STATUS_COLD)) ) {
		Log::log( LOG_ERR, "CountryBattle[%d] move error, invalid src domain status, domain_owner=%d, domain_status=%d", _group_index, src_domain.owner, src_domain.status);
		return ret;
	}
	
	int time_used = GetMoveUseTime(player.in_domain_id, dest); //È¡µÃÒÆ¶¯µ½Ä¿±êÁìµØËùĞèÒªµÄÊ±¼ä
	if(time_used <= 0) {
		Log::log( LOG_ERR, "CountryBattle[%d] move error, invalid dest, src_domain=%d, dest_domain=%d, time_used=%d", _group_index, player.in_domain_id, dest, time_used, _group_index);
		return ret; //Ô´ÁìµØÓëÄ¿±êÁìµØ²»ÏàÁÚ£¬²»ÄÜÒÆ¶¯
	}

	DOMAIN_MAP::iterator it_domain = _domain_map.find(dest);
	if(it_domain == _domain_map.end()) {
		Log::log( LOG_ERR, "CountryBattle[%d] move error, invalid dest, dest_domain=%d, group=%d", _group_index, dest);
		return ret;
	}
	
	DomainInfo& domain = it_domain->second;
	if(domain.owner == player.country_id) { //Ä¿±êµØÍ¼ºÍÍæ¼ÒÍ¬ÕóÓª
		PlayerMove(roleid, player.in_domain_id, dest, time_used);
		ret = 0;
	} else { //Ä¿±êµØÍ¼ºÍÍæ¼Ò²»Í¬ÕóÓª
		switch(domain.status) {
			case DOMAIN_STATUS_NORMAL:
			{
				PlayerMove(roleid, player.in_domain_id, dest, time_used);
				ret = 0;
				break;
			}
			case DOMAIN_STATUS_WAIT_FIGHT:
			case DOMAIN_STATUS_FIGHT:
			{
				if(domain.challenger == player.country_id) { //×Ô¼ºµÄÕóÓªÕıÔÚ½ø¹¥¸ÃµØÍ¼
					PlayerMove(roleid, player.in_domain_id, dest, time_used);
					ret = 0;
				}
				
				break;
			}
			case DOMAIN_STATUS_COLD: //Ä¿±êÁìµØ´¦ÓÚÀäÈ´×´Ì¬£¬²»ÄÜÒÆ¶¯
			default:
			{
				break;
			}
		}	
	}

	if(ret < 0) {
		Log::log( LOG_ERR, "CountryBattle[%d] move error, invalid dest domain status, domain_owner=%d, domain_status=%d", _group_index, domain.owner, domain.status);
	}
	
	return ret;
}

int CountryBattleMan::PlayerStopMove(int roleid)
{
	if(_status != ST_OPEN) return -1;
	PLAYER_ENTRY_MAP::iterator it_player = _player_map.find(roleid);
	if(it_player == _player_map.end()) return -1; //ÔÚ¹úÕ½ÖĞÕÒ²»µ½¸ÃÍæ¼Ò
		
	PlayerEntry& player = it_player->second;
	if(player.status != PLAYER_STATUS_NORMAL) return -1; //Íæ¼Ò´¦ÓÚ·ÇÕı³£Ì¬(ÈçµÈ´ıÕ½¶·£¬Õ½¶·×´Ì¬)£¬²»¿ÉÒÔ½ÓÊÜµ½È¡ÏûÒÆ¶¯µÄÃüÁî
	
	ClearMoveInfo(roleid); //Çå³ıÍæ¼ÒµÄÒÆ¶¯ĞÅÏ¢
	return 0;	
}

bool CountryBattleMan::SyncPlayerLocation(int roleid, int domain_id, int reason, unsigned int linksid, unsigned int localsid)
{	
	CountryBattleSyncPlayerLocation proto(roleid, domain_id, reason, localsid);
	return GDeliveryServer::GetInstance()->Send(linksid, proto);
}

bool CountryBattleMan::SyncPlayerPosToGs(int roleid, int worldtag, float posx, float posy, float posz, char is_capital)
{
	CountryBattleSyncPlayerPos proto(roleid, worldtag, posx, posy, posz, is_capital);
	
	int server_id = GetServerIdByWorldTag(worldtag);
	if(server_id >= 0) GProviderServer::GetInstance()->DispatchProtocol(server_id, proto);

	return true;
}

void CountryBattleMan::UpdateMoveInfo()
{
	//¸üĞÂÒÆ¶¯ĞÅÏ¢
	MOVE_LIST::iterator it = _move_info.begin();
	while(it != _move_info.end()) {
		MoveInfo& info = *it;
		--info.time;

		if(info.time <= 0) { //ÒÑ¾­¾­¹ıÁËµ½µ×Ä¿±êËùĞèÒªµÄÒÆ¶¯Ê±¼ä
			PLAYER_ENTRY_MAP::iterator it_player = _player_map.find(info.roleid);
			if(it_player != _player_map.end()) {
				PlayerEntry& player = it_player->second;
				player.prev_domain_id = player.in_domain_id; //¸üĞÂÍæ¼ÒÉÏÒ»²½Ëù´¦ÁìµØ
				player.in_domain_id = info.to; //¸üĞÂÍæ¼Òµ±Ç°Ëù´¦ÁìµØ
				
				DOMAIN_MAP::iterator it_domain = _domain_map.find(info.to);
				if(it_domain != _domain_map.end()) {
					DomainInfo& domain = it_domain->second;
					
					//µ±Ç°ÁìµØºÍÍæ¼Ò²»Í¬ÕóÓª£¬½«Íæ¼Ò×´Ì¬¸ü¸ÄÎªµÈ´ıÕ½¶·£¬×¼±¸½øÈëÕ½³¡
					//µ±Ç°ÁìµØºÍÍæ¼ÒÍ¬ÕóÓª£¬µ«¸ÃÁìµ¼´¦ÓÚµÈ´ıÕ½¶·»òÕ½¶·×´Ì¬£¬Íæ¼ÒĞèÖ§Ô®¸ÃÕ½³¡£¬½«Íæ¼Ò×´Ì¬¸ü¸ÄÎªµÈ´ıÕ½¶·£¬×¼±¸½øÈëÕ½³¡
					if((domain.owner != player.country_id) || ((domain.status == DOMAIN_STATUS_WAIT_FIGHT) || (domain.status == DOMAIN_STATUS_FIGHT))) {
						PlayerChangeState(player, PLAYER_STATUS_WAIT_FIGHT, REASON_PLAYER_FINISHI_MOVE);
					}
				}
				
				//½«Íæ¼Ò´ÓÔ´ÁìµØµÄÍæ¼ÒÁĞ±íÖĞÉ¾³ı£¬¼ÓÈëµ½Ä¿±êÁìµØµÄÍæ¼ÒÁĞ±í
				MoveBetweenDomain(player.roleid, info.from, info.to);
				//Í¨Öª¿Í»§¶ËÍæ¼ÒÒÑ¾­ÒÆ¶¯µ½ĞÂµÄÁìµØ
				SyncPlayerLocation(info.roleid, info.to, REASON_SYNC_MOVE, player.linksid, player.localsid);
				//Í¨ÖªGSÍæ¼ÒÒÑ¾­ÒÆ¶¯µ½ĞÂµÄÎ»ÖÃ
				DOMAIN2_INFO_SERV* pDomain = domain2_data_getbyid(info.to);
				if(pDomain != NULL) {
					int idx = rand() % 4;
					SyncPlayerPosToGs(player.roleid, player.worldtag, pDomain->mappos[idx][0], pDomain->mappos[idx][1], pDomain->mappos[idx][2], pDomain->is_capital);
				}
			}
			
			it = _move_info.erase(it); //ĞÅÏ¢¸üĞÂÍê±Ï£¬½«Íæ¼Ò´ÓÒÆ¶¯ÁĞ±íÖĞÉ¾³ı
			continue;
		} else {
			++it;
		}
	}
}

void CountryBattleMan::StartBattle(DomainInfo& domain, int challenger)
{
	//Õ½³¡±ØĞë´ÓNORMAL×´Ì¬×ª±äÎªWAIT_FIGHT×´Ì¬£¬ÆäÊµStartBattleÊÇÔÚÕ½³¡NORMAL×´Ì¬²Åµ÷ÓÃµÄ£¬ÕâÀï¼ÓÒ»¸öÈßÓàÅĞ¶ÏÒÔ·ÀÍòÒ»½«À´»áÔÚ¶à´¦StartBattle	
	if(domain.status != DOMAIN_STATUS_NORMAL) return;
	//×¼±¸¿ªÆôÕ½³¡
	domain.challenger = challenger; //ÉèÖÃÁìµØµÄÌôÕ½ÕóÓªID
	DomainChangeState(domain, DOMAIN_STATUS_WAIT_FIGHT, REASON_DOMAIN_WAIT_FIGHT); //½«Áìµ¼×ª±äÎªµÈ´ıÕ½ÕùµÄ×´Ì¬

	int player_limit_cnt = country_battle_players_cnt[(int)domain.wartype];
	int defender_player_cnt = _country_info[domain.owner - 1].online_player_cnt;
	int attacker_player_cnt = _country_info[domain.challenger - 1].online_player_cnt;
	
	int max_player_cnt = 0;
	for(int i = 0; i < COUNTRY_MAX_CNT; ++i) {
		if(_country_info[i].online_player_cnt > max_player_cnt) {
			max_player_cnt = _country_info[i].online_player_cnt;
		}
	}
	
	//Í¨ÖªGS¿ªÆôÕ½³¡¸±±¾
	CountryBattleStart proto(MakeUniqueID(_group_index,domain.id), domain.owner, domain.challenger, player_limit_cnt, Timer::GetTime() + SINGLE_BATTLE_END_TIME, defender_player_cnt, attacker_player_cnt, max_player_cnt);
	
	int server_id = GetServerIdByDomainId(domain.id);
	if(server_id >= 0) {
		//ÏòGS·¢ËÍCountryBattleStartĞ­Òé£¬Èç¹û·¢ËÍÊ§°Ü£¬Ö±½Ó½«³¬ÊĞÊ±¼äÖÃ0£¬ÂíÉÏ½øÈë³¬Ê±´¦Àí
		if(!GProviderServer::GetInstance()->DispatchProtocol(server_id, proto)) domain.time = 0;
	}

	LOG_TRACE( "CountryBattle start battle, server_id=%d, domain_id=%d, owner=%d, challenger=%d, player_limit_cnt=%d, end_time=%d, owner_player_cnt=%d, challenger_player_cnt=%d, country_max_player_cnt=%d",  server_id, domain.id, domain.owner, domain.challenger, player_limit_cnt, Timer::GetTime() + SINGLE_BATTLE_END_TIME, defender_player_cnt, attacker_player_cnt, max_player_cnt);
}

void CountryBattleMan::SysHandlePlayer(int player_id, int reason)
{
	//ÓÉÓÚÍæ¼Ò´¦ÓÚ×´Ì¬²»Ïà·ûµÄÁìµØ£¬ĞèÒªÓÉÏµÍ³½«Íæ¼ÒÀ­»ØÉÏÒ»²½µÄÎ»ÖÃ»òÊ×¶¼
	PLAYER_ENTRY_MAP::iterator it_player = _player_map.find(player_id);
	if(it_player == _player_map.end()) return;
	
	PlayerEntry& player = it_player->second;
	int src_domain_id = player.in_domain_id;
	int dest_domain_id = player.prev_domain_id;
	
	//ÉÏÒ»²½µÄÁìµØÊôÓÚÍæ¼ÒÕóÓª£¬ÇÒÊÇÕı³£×´Ì¬£¬½«Íæ¼ÒÀ­»ØÉÏÒ»²½Ëù´¦µÄÁìµØ
	DOMAIN_MAP::iterator it_domain = _domain_map.find(dest_domain_id);
	if(it_domain != _domain_map.end()) {
		DomainInfo& domain = it_domain->second;
		if((domain.owner == player.country_id) && (domain.status == DOMAIN_STATUS_NORMAL)) {
			MoveBetweenDomain(player.roleid, src_domain_id, dest_domain_id);
			player.in_domain_id = dest_domain_id;
			
			PlayerChangeState(player, PLAYER_STATUS_NORMAL, REASON_PLAYER_RETURN_PREV);
			SyncPlayerLocation(player.roleid, dest_domain_id, REASON_SYNC_PREV_STEP, player.linksid, player.localsid);

			DOMAIN2_INFO_SERV* pDomain = domain2_data_getbyid(dest_domain_id);
			if(pDomain != NULL) {
				int idx = rand() % 4;
				SyncPlayerPosToGs(player.roleid, player.worldtag, pDomain->mappos[idx][0], pDomain->mappos[idx][1], pDomain->mappos[idx][2], pDomain->is_capital);
			}

			LOG_TRACE("CountryBattle SysHandlePlayer: roleid=%d, src_domain=%d, new_domain=%d, reason=%d", player_id, src_domain_id, dest_domain_id, reason);
			return;
		} 
	}

	//Íæ¼Ò»Øµ½ÉÏÒ»²½µÄÁìµØÎ´ÄÜ³É¹¦£¬½«Íæ¼ÒÀ­»ØÊ×¶¼
	CapitalInfo& capital = _capital_info[player.country_id - 1];

	MoveBetweenDomain(player.roleid, src_domain_id, capital.id);
	PlayerChangeState(player, PLAYER_STATUS_NORMAL, REASON_PLAYER_RETURN_CAPITAL);

	player.in_domain_id = capital.id;
	player.prev_domain_id = capital.id;

	SyncPlayerLocation(player.roleid, capital.id, REASON_SYNC_CAPITAL, player.linksid, player.localsid);
	
	int idx = rand() % 4;
	SyncPlayerPosToGs(player.roleid, player.worldtag, capital.pos[idx].x, capital.pos[idx].y, capital.pos[idx].z, 1/*is_capital*/);

	LOG_TRACE("CountryBattle SysHandlePlayer: roleid=%d, src_domain=%d, new_domain=%d, reason=%d", player_id, src_domain_id, capital.id, reason);
}

void CountryBattleMan::UpdateWaitFightPlayers()
{
	//¸üĞÂµÈ´ıÕ½ÕùµÄÍæ¼ÒÁĞ±í
	
	//¸Ãº¯ÊıÄÚ²¿»áµ÷ÓÃSysHandlePlayerº¯Êı£¬¶øSysHandlePlayerº¯ÊıÄÚ¿ÉÄÜ»áÉ¾³ıµÈ´ıÕ½ÕùÍæ¼ÒÁĞ±íµÄÖĞµÄÔªËØ
	//Òò´ËÔÚwhileÑ­»·ÄÚ²»ÄÜµ÷ÓÃSysHandlePlayer£¬ĞèÒªÁíÍâµÄsys_handle_playersÁĞ±íÊÕ¼¯£¬´ıwhileÑ­»·½áÊøºóÒ»Æğ´¦Àí
	typedef std::pair<int, int> InvalidStatePlayer;
	std::vector< InvalidStatePlayer > sys_handle_players;
	
	PLAYER_WAIT_FIGHT_LIST::iterator it = _players_wait_fight.begin();
	while(it != _players_wait_fight.end()) {
		ColdInfo& info = *it;
		--info.time;
		
		if(info.time <= 0) { //µÈ´ıÕ½¶·µÄ×´Ì¬³¬Ê±£¬½«Íæ¼Ò·ÅÈëĞèÒªÏµÍ³À­»ØµÄÁĞ±íÖĞÒ»²¢´¦Àí
			++it;
			InvalidStatePlayer tmp(info.id, REASON_SYS_HANDLE_WAIT_TIMEOUT);
			sys_handle_players.push_back(tmp);
			continue;
		}
		
		PLAYER_ENTRY_MAP::iterator it_player = _player_map.find(info.id);
		if(it_player == _player_map.end()) {
			it = _players_wait_fight.erase(it);
			continue;
		}

		PlayerEntry& player = it_player->second;

		DOMAIN_MAP::iterator it_domain = _domain_map.find(player.in_domain_id);
		if(it_domain != _domain_map.end()) {
			DomainInfo& domain = it_domain->second;
			
			if(!IsPlayerBeyondBattleLimit(player, domain, 0)) { //²»Âú×ãÕ½³¡µÄ»êÁ¦ºÍÖ°ÒµÒªÇó£¬Ö±½Ó·ÅÈëÏµÍ³À­»ØÁĞ±í
				++it;
				InvalidStatePlayer tmp(info.id, REASON_SYS_HANDLE_UNDER_BATTLE_LIMIT);
				sys_handle_players.push_back(tmp);
				continue;
			}
			
			switch(domain.status) {
				case DOMAIN_STATUS_NORMAL:
				{
					if(domain.owner != player.country_id) { //¿ªÆôÕ½³¡
						StartBattle(domain, player.country_id);
					}
					break;
				}
				case DOMAIN_STATUS_WAIT_FIGHT:
				{
					//µÈ´ıÕ½³¡¿ªÆô½á¹û£¬Ê²Ã´¶¼²»×ö
					break;
				}
				case DOMAIN_STATUS_FIGHT:
				{
					if((domain.challenger == player.country_id) || (domain.owner == player.country_id)) { //Îª¹¥ÊØË«·½ÖĞµÄÒ»Ô±
						if(!info.is_notify) {
							CountryBattlePreEnterNotify proto;
							proto.battle_id = domain.id;
							proto.roleid = player.roleid;

							unsigned int linksid = 0;
							{
								Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
								PlayerInfo * pInfo = UserContainer::GetInstance().FindRoleOnline( player.roleid );
								if (pInfo != NULL) {
									linksid = pInfo->linksid;
									proto.localsid = pInfo->localsid;
								}
							}

							if(linksid > 0 && proto.localsid > 0) {
								proto.timeout = rand() % 5 + 2; //¿Í»§¶Ë³¬Ê±ÏàÓ¦µÄÊ±¼ä¿ØÖÆÔÚ[2, 6]ÃëÖ®¼ä
								GDeliveryServer::GetInstance()->Send(linksid, proto);
								
								LOG_TRACE("countrybattlepreenternotify, battle_id=%d, challenger=%d, defender=%d, roleid=%d, countryid=%d, timeout=%d", 
									domain.id, domain.challenger, domain.owner, player.roleid, player.country_id, proto.timeout);
								info.is_notify = true;
							}	
						}
					} else { //½«Íæ¼ÒÀ­»Ø
						InvalidStatePlayer tmp(info.id, REASON_SYS_HANDLE_INVALID_BATTLE);
						sys_handle_players.push_back(tmp);
					}
					break;
				}
				case DOMAIN_STATUS_COLD:
				default:
				{
					InvalidStatePlayer tmp(info.id, REASON_SYS_HANDLE_COLD_DOMAIN);
					sys_handle_players.push_back(tmp);
					break;
				}
			}	
		}

		++it;
	}
	
	//¸ÃÁĞ±íÖĞµÄÍæ¼Ò´¦ÓÚ×´Ì¬²»ÕıÈ·µÄÁìµØ£¬ĞèÒªÓÉÏµÍ³À­»Ø
	for(unsigned int i = 0; i < sys_handle_players.size(); ++i) {
		SysHandlePlayer(sys_handle_players[i].first, sys_handle_players[i].second);
	}
}

void CountryBattleMan::UpdateWaitFightDomains(DomainInfo& info)
{
	//¸üĞÂµÈ´ıÕ½ÕùµÄÁìµØÁĞ±í
	//ÓÉÓÚGS¿ªÆôÕ½³¡¸±±¾ĞèÒªÒ»¶¨Ê±¼ä£¬ËùÒÔÒªÓĞÕâ¸öµÈ´ıÕ½ÕùµÄ×´Ì¬
	--info.time;	
	if(info.time > 0) return;
		
	//µÈ´ıGS¿ªÆôÕ½³¡³¬Ê±
	//²»ÄÜÖ±½Ó±éÀúplayer_list£¬ÒòÎªSysHandlePlayer»á¸Ä±äÁĞ±íÄÚÈİ£¬ËùÒÔĞèÒªÒ»¸öÁĞ±íµÄ¿½±´
	std::vector<int> list = info.player_list;

	for(unsigned int i = 0; i < list.size(); ++i) {
		SysHandlePlayer(list[i], REASON_SYS_HANDLE_BATTLE_START_TIMEOUT);
	}	

	info.challenger = 0;
	//³¬Ê±ĞèÒª½øĞĞÁìµØ×´Ì¬µÄ×ª»»£¬ÒòÎª´æÔÚGSÖ±½ÓcoreµôµÄ¿ÉÄÜ£¬½«²»»áÓĞStartBattle_Re·µ»Ø
	//ÁìÍÁÖ±½Ó´ÓWAIT_FIGHT×ª±äÎªCOLD×´Ì¬
	DomainChangeState(info, DOMAIN_STATUS_COLD, REASON_DOMAIN_WAIT_FIGHT_TIMEOUT);
}

void CountryBattleMan::UpdateFightDomains(DomainInfo& info)
{
	//¸üĞÂ´¦ÓÚÕ½¶·×´Ì¬µÄÁìµØÁĞ±í
	//´¦ÓÚÕ½¶·×´Ì¬ÁìµØÔÚGSÉÏÓĞ¸±±¾
	if((info.time + SINGLE_BATTLE_END_TIME + TEN_MINUTES) < Timer::GetTime()) { //µ¥³¡Õ½¶·ÒÑ¾­³¬¹ıÔ¤ÆÚ½áÊøÊ±¼ä10·ÖÖÓÖ®¶à
		//ÀíÂÛÉÏËµGS»á¿ØÖÆÕ½¶·½áÊø£¬delivery³öÏÖÕâÖÖ×´¿öµÄ¿ÉÄÜĞÔ·Ç³£Ğ¡
		//Èô³öÏÖ£¬ÔòÓĞ¿ÉÄÜÊÇÓÉÓÚGSÖØÆô£¬deliveryÃ»ÓĞÍ¬Ê±ÖØÆô£¬Ôì³ÉË«·½×´Ì¬²»Ò»ÖÂ
		//²»ÄÜÖ±½Ó±éÀúplayer_list£¬ÒòÎªSysHandlePlayer»á¸Ä±äÁĞ±íÄÚÈİ£¬ËùÒÔĞèÒªÒ»¸öÁĞ±íµÄ¿½±´
		std::vector<int> list = info.player_list;

		for(unsigned int i = 0; i < list.size(); ++i) {
			SysHandlePlayer(list[i], REASON_SYS_HANDLE_FIGHT_TIMEOUT);
		}
		
		info.challenger = 0;
		//ÁìÍÁ´ÓFIGHT×ª±äÎªCOLD×´Ì¬
		DomainChangeState(info, DOMAIN_STATUS_COLD, REASON_DOMAIN_BATTLE_TIMEOUT);
	}
}

void CountryBattleMan::UpdateColdDomains(DomainInfo& info)
{
	//¸üĞÂÀäÈ´µÄÁìµØÁĞ±í
	--info.time;	
	if(info.time > 0) return;  //ÉĞÔÚÀäÈ´ÖĞ
		
	//ÀäÈ´Ê±¼äµ½
	//²»ÄÜÖ±½Ó±éÀúplayer_list£¬ÒòÎªSysHandlePlayer»á¸Ä±äÁĞ±íÄÚÈİ£¬ËùÒÔĞèÒªÒ»¸öÁĞ±íµÄ¿½±´
	std::vector<int> list = info.player_list;
			
	//½«ÁìÍÁµÄÍæ¼ÒÁĞ±íÖĞ£¬²»ÊôÓÚ¸ÃÁìÍÁËùÊôÕóÓª£¬¼´Õ½°ÜµÄÍæ¼ÒÀ­»Ø
	//ÆäÊµÔÚÕ½³¡¸±±¾½áÊø£¬Íæ¼Ò×ª»»³¡¾°Ê±£¬Õ½°ÜµÄÍæ¼ÒÓ¦¸ÃÒÑ¾­±»À­»Ø£¬´Ë´¦²»Ó¦¸Ã»¹ÓĞÕ½°ÜµÄÍæ¼ÒÔÚÁìÍÁÉÏ
	//ÕâÊÇÎªÁËÒÔ·ÀÍòÒ»¶øĞ´µÄÈßÓàÂß¼­
	for(unsigned int i = 0; i < list.size(); ++i) {
		PLAYER_ENTRY_MAP::iterator it_player = _player_map.find(list[i]);
		if(it_player != _player_map.end()) {
			PlayerEntry& player = it_player->second;
			if(player.country_id != info.owner) SysHandlePlayer(list[i], REASON_SYS_HANDLE_COLD_DOMAIN_TIMEOUT);
		}
	}
			
	//ÁìÍÁ×ª±äÎªÕı³£×´Ì¬
	DomainChangeState(info, DOMAIN_STATUS_NORMAL, REASON_DOMAIN_COLD_TIMEOUT);		
}

void CountryBattleMan::SendBattleResult(int player_bonus, int country_bonus[COUNTRY_MAX_CNT], unsigned int linksid, unsigned int localsid)
{
	CountryBattleResult result;
	result.player_bonus = player_bonus;
	result.localsid = localsid;
	for(int i = 0; i < COUNTRY_MAX_CNT; ++i) {
		result.country_bonus.push_back(country_bonus[i]);
		result.country_domains_cnt.push_back(_country_info[i].domains_cnt);
	}
	
	GDeliveryServer::GetInstance()->Send(linksid, result);
}

void CountryBattleMan::DBSendBattleBonus(int roleid, int player_bonus, int zoneid)
{
	GRoleInventory inv;
	inv.id = BONUS_ITEM_ID;
	inv.count = player_bonus;
	inv.max_count = PLAYER_MAX_BONUS;
	inv.proctype = BONUS_PROCTYPE;

	DBCountryBattleBonusArg arg;
	arg.roleid = roleid;
	arg.money = 0;
	arg.item = inv;

	Log::formatlog("countrybattlebonus","zoneid=%d:roleid=%d:amount=%d:group=%d", zoneid, roleid, player_bonus, _group_index);
	DBCountryBattleBonus* rpc = (DBCountryBattleBonus*) Rpc::Call( RPC_DBCOUNTRYBATTLEBONUS, arg);
	GameDBClient::GetInstance()->SendProtocol( rpc );
}

int CountryBattleMan::CalcPlayerBonus(const PlayerScoreInfo& info, int country_bonus[COUNTRY_MAX_CNT])
{
	int other_player_bonus = (int)(country_bonus[info.country_id - 1] * (1 - 0.015));

	float value = 0.0f;
	if(_country_info[info.country_id - 1].player_total_scores > 0) {
		value = info.score / _country_info[info.country_id - 1].player_total_scores;	
		if(value > 1.0f) return 0;
	}

	int player_bonus = (int)(value * other_player_bonus);
	if(player_bonus > PLAYER_MAX_BONUS) player_bonus = PLAYER_MAX_BONUS;

	bool can_get_bonus_condition1 = (info.win_cnt >= _bonus_limit.win_cnt_limit) && (info.total_combat_time >= _bonus_limit.combat_time_limit);
	bool can_get_bonus_condition2 = (info.win_cnt >= _bonus_limit.win_cnt_limit) && (info.total_contribute_val >= _bonus_limit.contribute_val_limit);
	bool can_get_bonus = (can_get_bonus_condition1 || can_get_bonus_condition2);
	//²»Âú×ã×îĞ¡µÃ½±Ìõ¼şµÄÍæ¼ÒÃ»ÓĞ½±Àø
	if( !can_get_bonus ) player_bonus = 0;

	return player_bonus;
}

int CountryBattleMan::CalcKingBonus(int country_bonus)
{
	int king_bonus = (int)(country_bonus * 0.015);
	if(king_bonus > PLAYER_MAX_BONUS) king_bonus = PLAYER_MAX_BONUS;

	return king_bonus;
}

void CountryBattleMan::OutputZoneCountryScoreLog()
{
	if(_arrange_country_type != ARRANGE_COUNTRY_BY_ZONEID) return;

	struct ScoreInfo
	{
		int zoneid;
		int score;
	};
	ScoreInfo info[COUNTRY_MAX_CNT];
	memset(info, 0, sizeof(info));
	
	//InitializeµÄÂß¼­£¬±£Ö¤ÁË¿ç·ş¹úÕ½£¬¹ú¼ÒÊıÁ¿Ó¦¸ÃÔÚ[2, COUNTRY_MAX_CNT]Ö®¼ä
	for(ZONE_COUNTRY_MAP::iterator it = _zone_country_map.begin(); it != _zone_country_map.end(); ++it) {
		int country_id = it->second;

		if(country_id > 0 && country_id <= COUNTRY_MAX_CNT) {
			int idx = country_id - 1;
			info[idx].zoneid = it->first;
			info[idx].score = (int)_country_info[idx].country_scores;
		}
	}
	
	Log::formatlog("countrybattlecountryscore","zone1=%d:score=%d:zone2=%d:score=%d:zone3=%d:score=%d:zone4=%d:score=%d:group=%d", 
			info[0].zoneid, info[0].score, info[1].zoneid, info[1].score, info[2].zoneid, info[2].score, info[3].zoneid, info[3].score, _group_index);
}

void CountryBattleMan::CalcBonus()
{
	OutputZoneCountryScoreLog();

	//·¢·Å½±Àø
	int min_cnt_idx = 0;
	for(int i = 1; i < COUNTRY_MAX_CNT; ++i) { //¼ÆËãÁìÍÁ»ı·Ö×îÉÙµÄÕóÓª
		if(_country_info[i].country_scores < _country_info[min_cnt_idx].country_scores) {
			min_cnt_idx = i;
		}
	}

	float country_scores[COUNTRY_MAX_CNT] = {0.0f};
	
	float total_scores = 0.0f;
	for(int i = 0; i < COUNTRY_MAX_CNT; ++i) { //¼ÆËãËùÓĞÕóÓªµÄ×Ü·ÖÊı
		country_scores[i] = _country_info[i].country_scores - _country_info[min_cnt_idx].country_scores * 0.75;
		total_scores += country_scores[i];
	}
	
	int country_bonus[COUNTRY_MAX_CNT] = {0};

	//½«×Ü¾ü×ÊµÄ35%£¬²ÎÓëµÄ¹ú¼ÒÆ½·Ö(±£µ×½±Àø)
	int base_bonus = (int)(_bonus_limit.total_bonus * 0.35);
	std::set<int> battle_country_set;
	battle_country_set.clear();
	int battle_country_cnt = GetBattleCountryCnt(battle_country_set);
	for(int i = 0; i < COUNTRY_MAX_CNT; ++i) {
		int country_id = i + 1;
		//²ÎÓëµÄ¹ú¼Ò¼´¸Ã¹úµÄÍæ¼ÒÓĞ¸öÈË»ı·Ö£¬·ñÔòÊÓÎªÎ´²ÎÓë
		if(battle_country_set.find(country_id) != battle_country_set.end()) {
			country_bonus[i] = base_bonus / battle_country_cnt;
		}
	}
	
	if(total_scores > 0.0f) {
		for(int i = 0; i < COUNTRY_MAX_CNT; ++i) { //¼ÆËãÃ¿¸öÕóÓªÓ¦µÃµÄ½±ÀøÊı
			country_bonus[i] += (int)(country_scores[i] / total_scores * (_bonus_limit.total_bonus - base_bonus)); //ÔÙ¸ù¾İËã·¨·ÖÊ£ÏÂµÄ²¿·Ö£¬¶ø²»ÊÇ·ÖÈ«²¿
		}
	}

	for(PLAYER_ENTRY_MAP::iterator it = _player_map.begin(); it != _player_map.end(); ++it) { //Í¨ÖªÔÚÏßÍæ¼Ò½á¹û
		PlayerEntry& player = it->second;
		int player_bonus = 0;
	
		if(player.roleid == _country_info[player.country_id - 1].king_info.roleid) {
			player_bonus = CalcKingBonus(country_bonus[player.country_id - 1]);
		} else {
			PLAYER_SCORE_MAP::iterator it_score = _player_score_map.find(player.roleid);
			if(it_score != _player_score_map.end()) {
				PlayerScoreInfo& info = it_score->second;	
				player_bonus = CalcPlayerBonus(info, country_bonus);	
			}
		}

		SendBattleResult(player_bonus, country_bonus, player.linksid, player.localsid);
	}
	
	for(int i = 0; i < COUNTRY_MAX_CNT; ++i) {
		int king_roleid = _country_info[i].king_info.roleid;
		if(king_roleid > 0) {
			int player_bonus = CalcKingBonus(country_bonus[i]);

			PlayerBonus data;
			data.roleid = king_roleid;
			data.bonus = player_bonus;
			data.zoneid = GetZoneIDByCountryID(i);
			_player_bonus_list.push_back(data);
		}
	}
	
	for(PLAYER_SCORE_MAP::iterator it = _player_score_map.begin(); it != _player_score_map.end(); ++it) { //¼ÆËãÃ¿¸öÍæ¼ÒÓ¦¸ÃµÃµ½µÄ½±ÀøÊı
		PlayerScoreInfo& info = it->second;	
		if(info.roleid == _country_info[info.country_id - 1].king_info.roleid) continue;

		int player_bonus = CalcPlayerBonus(info, country_bonus);	

		PlayerBonus data;
		data.roleid = info.roleid;
		data.bonus = player_bonus;
		data.zoneid = GetZoneIDByCountryID(info.country_id);
		_player_bonus_list.push_back(data);
	}

	_db_send_bonus_per_sec = _player_bonus_list.size() / 3000 + 1; //ÒªÔÚ50·ÖÖÓ£¬¼´3000ÃëÄÚ½«½±Àø·¢Íê£¬Ã¿ÃëÖÁÉÙÒ»¸ö
}

void CountryBattleMan::SendBonus()
{
	if(!_player_bonus_list.empty()) {
		if(_player_bonus_list.size() <= _db_send_bonus_per_sec) _db_send_bonus_per_sec = _player_bonus_list.size();

		for(unsigned int i = 0; i < _db_send_bonus_per_sec; ++i) {
			PlayerBonus& data = _player_bonus_list[i];
			DBSendBattleBonus(data.roleid, data.bonus, data.zoneid);
		}       

		_player_bonus_list.erase(_player_bonus_list.begin(), _player_bonus_list.begin() + _db_send_bonus_per_sec);
	}
}

void CountryBattleMan::UpdateDomains()
{
	++_calc_domains_timer;

	for(DOMAIN_MAP::iterator it = _domain_map.begin(); it != _domain_map.end(); ++it) {
		DomainInfo& domain = it->second;
		
		//Ã¿10·ÖÖÓ£¬¼ÆËã¸÷¸öÕóÓªËùÕ¼µÄÁìÍÁÊıÄ¿
		if(_calc_domains_timer > TEN_MINUTES) {
			DOMAIN2_INFO_SERV* pData = domain2_data_getbyid(domain.id);
			if(pData == NULL) continue;

			++_country_info[domain.owner - 1].domains_cnt;
			_country_info[domain.owner - 1].country_scores += 1;
		}

		switch(domain.status) {
			case DOMAIN_STATUS_NORMAL:
				break;	
			case DOMAIN_STATUS_WAIT_FIGHT:
				UpdateWaitFightDomains(domain);
				break;
			case DOMAIN_STATUS_FIGHT:
				UpdateFightDomains(domain);
				break;
			case DOMAIN_STATUS_COLD:
				UpdateColdDomains(domain);
				break;
			default:
				Log::log( LOG_ERR, "CountryBattle[%d] Domain state error, state=%d", _group_index, domain.status);
				break;
		}
	}
	
	//Çå³ı¼ÆËãÁìÍÁÊıÄ¿µÄ¼ÆÊıÆ÷
	if(_calc_domains_timer > TEN_MINUTES) _calc_domains_timer = 0;
}

bool CountryBattleMan::Update()
{	
	time_t now = GetTime();

	struct tm dt;
	localtime_r(&now, &dt);
	int second_of_day = dt.tm_hour * 3600 + dt.tm_min * 60 + dt.tm_sec;
	
	if(_status == ST_CLOSE) {
		bool is_open_day = _open_days[dt.tm_wday]; //¸ù¾İÅäÖÃÅĞ¶ÏÊÇ·ñÎª¿ªÆôÈÕ //(dt.tm_wday == 0 || dt.tm_wday == 5); //ÖÜ5ºÍÖÜÈÕ¿ªÆô
		bool is_open_time = (second_of_day >= COUNTRYBATTLE_START_TIME) && (second_of_day < COUNTRYBATTLE_BONUS_TIME);
		
		if( is_open_day && is_open_time) {
			if(_arrange_country_type == ARRANGE_COUNTRY_BY_ZONEID) {
				ArrangeCountryByZoneID();
			}

			_status = ST_OPEN;
			LOG_TRACE( "CountryBattle System Change State %d---->%d.\n",  ST_CLOSE, ST_OPEN);
		}
	} else if(_status == ST_OPEN) {
		if( (second_of_day >= COUNTRYBATTLE_BONUS_TIME) && (second_of_day < COUNTRYBATTLE_CLEAR_TIME) ) { //ÊÇ·¢½±Ê±¼ä
			_status = ST_BONUS;
			LOG_TRACE( "CountryBattle System Change State %d---->%d.\n",  ST_OPEN, ST_BONUS);

			StopMovingPlayers();		
			CalcBonus();

			_move_info.clear();
			_players_wait_fight.clear();
		} else {
			UpdateMoveInfo();
			UpdateWaitFightPlayers();
			UpdateDomains();
		}
	} else if(_status == ST_BONUS) {
		if(_player_bonus_list.empty()) {
			if(second_of_day >= COUNTRYBATTLE_CLEAR_TIME) { //Çå¿Õ¹úÕ½ĞÅÏ¢
				_status = ST_CLOSE;
				LOG_TRACE( "CountryBattle System Change State %d---->%d.\n",  ST_BONUS, ST_CLOSE);

				Clear();
			}
		} else {	
			SendBonus();
		}
		
		//·¢½±µÄÊ±¼äÓ¦¸ÃÊÇ22:20¿ªÊ¼, 23:30Ö®Ç°·¢ÍêÈ«²¿½±Àø£¬ ÔÚ23£º30Ê±ÏµÍ³×´Ì¬Ó¦¸Ã±äÎªST_CLOSE
		//Èç¹û23:40»¹ÊÇST_BONUSµÄ×´Ì¬£¬´ËÊ±ËµÃ÷ÏµÍ³ÒÑ¾­´¦ÔÚ²»ÕıÈ·µÄ×´Ì¬
		//ÕâÀïÇ¿ĞĞ½«ÏµÍ³×ª»»ÎªST_CLOSE£¬Çå³ıÊı¾İ£¬²¢¼ÇÂ¼Error Log
		if(second_of_day >= (COUNTRYBATTLE_CLEAR_TIME + TEN_MINUTES)) { //Çå¿Õ¹úÕ½ĞÅÏ¢
			_status = ST_CLOSE;
			LOG_TRACE( "CountryBattle System Change State %d---->%d.\n",  ST_BONUS, ST_CLOSE);

			Clear();
			Log::log( LOG_ERR, "Countrybattle[%d] system didn't send all bonus in time", _group_index );
		}
	}

	return true;
}

void CountryBattleMan::NotifyBattleConfig(int server_id)
{
	if(_capital_worldtag > 0) {
		CountryBattleConfigNotify notify;
		MakeCapitalsData(notify.country_capitals);

		GProviderServer::GetInstance()->DispatchProtocol(server_id, notify);
	}
}

bool CountryBattleMan::RegisterServer(int server_type, int war_type, int server_id, int worldtag)
{
	LOG_TRACE( "CountryBattle Register Server: war_type=%d server=%d map=%d.\n", war_type, server_id, worldtag );
	
	bool is_find = false;
	for(unsigned int i = 0; i < _servers.size(); ++i) {
		if(server_id == _servers[i].server_id) { //ÒÑ¾­×¢²á¹ı£¬¸üĞÂ×¢²áĞÅÏ¢
			_servers[i].Init(war_type, worldtag, server_id);
			is_find = true;
			break;
		}
	}
	
	if(!is_find) { //ĞÂ×¢²á¹úÕ½³¡¾°
		RegisterInfo info;
		info.Init(war_type, worldtag, server_id);
		_servers.push_back(info);
	}
	
	if(server_type == 0) { //Ê×¶¼³¡¾°
		_capital_worldtag = worldtag;
				
		CountryBattleConfigNotify notify;
		MakeCapitalsData(notify.country_capitals);
		for(unsigned int i = 0; i < _servers.size(); ++i) { //Í¨ÖªGSÊ×¶¼³¡¾°ĞÅÏ¢
			int gs_id = _servers[i].server_id;
			if(_capital_worldtag > 0) GProviderServer::GetInstance()->DispatchProtocol(gs_id, notify);
		}
	} else { //·ÇÊ×¶¼³¡¾°
		NotifyBattleConfig(server_id); //Í¨Öª¸Ã³¡¾°Ê×¶¼µÄĞÅÏ¢
	}
	
	return true;
}

bool CountryBattleMan::SendMap(int roleid, unsigned int sid, unsigned int localsid)
{
	PLAYER_ENTRY_MAP::iterator it_requester = _player_map.find(roleid);
	if(it_requester == _player_map.end()) return false;
	PlayerEntry& requester = it_requester->second;

	CountryBattleGetMap_Re re;
	re.retcode = 0;
	re.localsid = localsid;
	
	for(DOMAIN_MAP::iterator it = _domain_map.begin(); it != _domain_map.end(); ++it) {
		DomainInfo& info = it->second;
		
		GCountryBattleDomain domain;
		domain.id = info.id;
		domain.owner = info.owner;
		domain.challenger = info.challenger;
		domain.status = info.status;
		domain.battle_config_mask = info.battle_config_mask[requester.country_id - 1];
		domain.time = 0;
		
		if(domain.status == DOMAIN_STATUS_WAIT_FIGHT || domain.status == DOMAIN_STATUS_COLD) {
			domain.time = info.time;
		}

		domain.country_playercnt.clear();
		domain.country_playercnt.insert(domain.country_playercnt.begin(), 4, 0);

		for(unsigned int i = 0; i < info.player_list.size(); ++i) {
			int tmp_roleid = info.player_list[i];
			PLAYER_ENTRY_MAP::iterator it_player = _player_map.find(tmp_roleid);
			if(it_player == _player_map.end()) continue;

			PlayerEntry& player = it_player->second;
			if(player.country_id > 0 && player.country_id <= COUNTRY_MAX_CNT) {
				++domain.country_playercnt[player.country_id - 1];
			}
		}

		re.domains.push_back(domain);
	}
	
	for(int i = 0; i < COUNTRY_MAX_CNT; ++i) {
		int king_roleid = _country_info[i].king_info.roleid;
		re.kings.push_back(king_roleid);
	}

	GDeliveryServer::GetInstance()->Send(sid, re);
	return true;
}

bool CountryBattleMan::SendConfig(int roleid, unsigned int sid, unsigned int localsid)
{
	int start_time = GetCountryBattleStartTime();
	int end_time = GetCountryBattleEndTime();
	char is_open = IsBattleStart();
	int total_bonus = GetTotalBonus();
	char domain2_datatype = GetDomain2DataType();
	unsigned int domain2_data_timestamp = get_domain2_data_timestamp();

	CountryBattleGetConfig_Re re(start_time, end_time, total_bonus, is_open, domain2_datatype, domain2_data_timestamp, localsid);
	GDeliveryServer::GetInstance()->Send(sid, re);
	return true;
}

bool CountryBattleMan::SendCountryScore(int roleid, unsigned int sid, unsigned int localsid)
{
	CountryBattleGetScore_Re re;
	re.localsid = localsid;
	re.player_score = 0;
	
	PLAYER_SCORE_MAP::iterator it_score = _player_score_map.find(roleid);
	if(it_score != _player_score_map.end()) {
		re.player_score = (int)(it_score->second.score);
	}
		
	for(int i = 0; i < COUNTRY_MAX_CNT; ++i) {
		re.country_score.push_back(_country_info[i].country_scores);
	}

	GDeliveryServer::GetInstance()->Send(sid, re);
	return true;
}

bool CountryBattleMan::BattleStart(int battleid, int worldtag, int retcode, int defender, int attacker)
{
	//Õ½¶·¿ªÊ¼
	if(_status != ST_OPEN) return false;
	DOMAIN_MAP::iterator it_domain = _domain_map.find(battleid);
	if(it_domain == _domain_map.end()) return false;
	
	DomainInfo& domain = it_domain->second;

	//Õ½³¡Ö»ÓĞ´ÓWAIT_FIGHT×´Ì¬²ÅÄÜ×ª±äÎªFIGHT×´Ì¬£¬Í¬Ê±»¹Òª¼ì²édeliveryºÍGSÉÏ¹¥ÊØË«·½ÊÇ·ñÒ»ÖÂ
	if(domain.status != DOMAIN_STATUS_WAIT_FIGHT || domain.owner != defender || domain.challenger != attacker) { //×´Ì¬´íÎó»ò¹¥ÊØÊı¾İ²»Ò»ÖÂ
		if(retcode == 0) { //gsÕ½³¡¿ªÆô³É¹¦
			//Í¨ÖªgsÏú»Ù¸ÃÕ½³¡
			CountryBattleDestroyInstance proto(MakeUniqueID(_group_index,battleid), worldtag);
			int server_id = GetServerIdByWorldTag(worldtag);
			if(server_id >= 0) GProviderServer::GetInstance()->DispatchProtocol(server_id, proto);
		} //gsÕ½³¡¿ªÆôÊ§°ÜÊ±£¬Ö±½ÓºöÂÔ¾ÍºÃÀ²
		
		return false;
	} 
	
	//²»ÄÜÖ±½Ó±éÀúplayer_list£¬ÒòÎªSysHandlePlayer»á¸Ä±äÁĞ±íÄÚÈİ£¬ËùÒÔĞèÒªÒ»¸öÁĞ±íµÄ¿½±´
	std::vector<int> list = domain.player_list;

	int attacker_cnt = 0;
	int defender_cnt = 0;
	char defender_occupation_cnt[MAX_OCCUPATION_CNT];
	memset(defender_occupation_cnt, 0, sizeof(defender_occupation_cnt));
	char attacker_occupation_cnt[MAX_OCCUPATION_CNT] = {0};
	memset(attacker_occupation_cnt, 0, sizeof(attacker_occupation_cnt));
	
	int player_limit_cnt = country_battle_players_cnt[(int)domain.wartype];
	if(retcode == 0) {
		//×ª»»domain×´Ì¬
		DomainChangeState(domain, DOMAIN_STATUS_FIGHT, REASON_DOMAIN_BATTLE_START_SUCCESS);

		for(unsigned int i = 0; i < list.size(); ++i) {
			PLAYER_ENTRY_MAP::iterator it_player = _player_map.find(list[i]);
			if(it_player == _player_map.end()) continue;
			
			PlayerEntry& player = it_player->second;
			if(player.country_id == domain.owner) {
				bool flag1 = (defender_cnt < player_limit_cnt);
				bool flag2 = IsPlayerBeyondBattleLimit(player, domain, defender_occupation_cnt[player.occupation] );
				
				if(flag1 && flag2) {
					++defender_cnt;
					++defender_occupation_cnt[player.occupation]; 
					
					if(player.status != PLAYER_STATUS_WAIT_FIGHT) {
						ClearMoveInfo(player.roleid);
						PlayerChangeState(player, PLAYER_STATUS_WAIT_FIGHT, REASON_PLAYER_WAIT_FIGHT);
					}
				} else {
					SysHandlePlayer(player.roleid, REASON_SYS_HANDLE_BATTLE_FULL);
				}
			} else if(player.country_id == domain.challenger) {
				bool flag1 = (attacker_cnt < player_limit_cnt);
				bool flag2 = IsPlayerBeyondBattleLimit(player, domain, attacker_occupation_cnt[player.occupation]);

				if(flag1 && flag2) {
					++attacker_cnt;
					++attacker_occupation_cnt[player.occupation]; 

					if(player.status != PLAYER_STATUS_WAIT_FIGHT) {
						ClearMoveInfo(player.roleid);
						PlayerChangeState(player, PLAYER_STATUS_WAIT_FIGHT, REASON_PLAYER_WAIT_FIGHT);
					}
				} else {
					SysHandlePlayer(player.roleid, REASON_SYS_HANDLE_BATTLE_FULL);
				}
			} else {
				SysHandlePlayer(player.roleid, REASON_SYS_HANDLE_INVALID_BATTLE);
			}
		}
		
		//BroadcastCountryBattleMsg(CMSG_COUNTRYBATTLE_ATTACK, domain.challenger, domain.owner, battleid);
		//BroadcastCountryBattleMsg(CMSG_COUNTRYBATTLE_DEFEND, domain.owner, domain.challenger, battleid);

		Log::formatlog("countrybattlestart","zoneid=%d:battleid=%d:time=%d:defender=%d:attacker=%d:group=%d", 
			GDeliveryServer::GetInstance()->zoneid, battleid, time(NULL)/*GetTime()*/, domain.owner, domain.challenger, _group_index);
	} else {
		DomainChangeState(domain, DOMAIN_STATUS_NORMAL, REASON_DOMAIN_BATTLE_START_FAILED);

		for(unsigned int i = 0; i < list.size(); ++i) {
			SysHandlePlayer(list[i], REASON_SYS_HANDLE_BATTLE_START_FAILED);
		}
	}
	
	return true;
}

/*float CountryBattleMan::CalcPlayerScore(int domain_point, int total_combat_time, int battle_last_time, 
	const GCountryBattlePersonalScore& score, bool is_winner, float winner_average_score)
{
	int roleid = score.roleid;
	
	PLAYER_ENTRY_MAP::iterator it_player = _player_map.find(roleid);
	if(it_player == _player_map.end()) return 0.0f;
	
	char country_id = it_player->second.country_id;
	
	if(score.cls < 0 || score.cls >= (int)_occupation_fac_list.size()) return 0.0f;
	OccupationFactor& fac = _occupation_fac_list[score.cls];
	
	float win_lose_fac = is_winner ? fac.win_fac : fac.fail_fac;
	const int point_ceil = 25 * 60;
	
	float tmp_combat_part = 0.0;
	if(total_combat_time != 0) tmp_combat_part = (score.combat_time * fac.combat_time_fac / total_combat_time);
	
	float point = ( 0.02 * tmp_combat_part + (fac.kill_cnt_fac * 0.2 * ((float)score.kill_count / ((float)score.kill_count + 10))) 
		+ (score.attend_time * fac.attend_time_fac / (battle_last_time * 40)) ) * win_lose_fac * domain_point * point_ceil + score.contribute_val * 15;
	
	if(point > point_ceil) point = point_ceil; //Íæ¼Ò¸öÈËµÃ·Ö²»ÄÜ³¬¹ıÉÏÏŞ
	if(!is_winner && (point > winner_average_score)) point = winner_average_score; //Ê§°Ü·½¸öÈËµÃ·Ö²»ÄÜ³¬¹ıÊ¤Àû·½µÄÆ½¾ù·Ö
	
	LOG_TRACE("countrybattle calc player score, roleid=%d, cls=%d, combat_time=%d, combat_time_fac=%f, total_combat_time=%d, kill_cnt=%d, kill_cnt_fac=%f, attend_time=%d, attend_time_fac=%f, battle_last_time=%d, win_lose_fac=%f, domain_point=%d, contribute_val=%d, player_point=%f", 
		roleid, score.cls, score.combat_time, fac.combat_time_fac, total_combat_time,
		score.kill_count, fac.kill_cnt_fac, score.attend_time, fac.attend_time_fac, battle_last_time, win_lose_fac, domain_point, score.contribute_val, point);

	_country_info[country_id - 1].player_total_scores += point;
	
	PLAYER_SCORE_MAP::iterator it_score = _player_score_map.find(roleid);
	if(it_score != _player_score_map.end()) {
		PlayerScoreInfo& info = it_score->second;
		info.dead_cnt += score.death_count;
		if(is_winner) ++info.win_cnt;
		info.total_combat_time += score.combat_time;
		info.total_contribute_val += score.contribute_val;
		info.score += point;
	} else {
		int win_cnt = is_winner ? 1 : 0;
		_player_score_map[roleid].Init(roleid, country_id, win_cnt, score.death_count, score.combat_time, score.contribute_val, point);
	}

	return point;
}*/

bool compare_rank(const CountryBattleMan::PlayerRankInfo& info1, const CountryBattleMan::PlayerRankInfo& info2)
{
	if(info1.rank_val != info2.rank_val) {
		return (info1.rank_val > info2.rank_val);
	} else {
		return (info1.roleid < info2.roleid);
	}
}

void CountryBattleMan::CalcPlayerScore(int battleid, int total_score, const std::vector<GCountryBattlePersonalScore>& scores, bool is_winner, char battle_config_mask)
{
	std::vector<PlayerRankInfo> list;
	for(unsigned int i = 0; i < scores.size(); ++i) {
		const GCountryBattlePersonalScore& personal_score = scores[i];
		
		if(personal_score.cls < 0 || personal_score.cls >= (int)_occupation_fac_list.size()) continue;
		OccupationFactor& fac = _occupation_fac_list[personal_score.cls];

		PlayerRankInfo info;
		info.roleid = personal_score.roleid;
		info.cls = personal_score.cls;
		info.dead_cnt = personal_score.death_count;
		info.combat_time = personal_score.combat_time;
		info.contribute_val = personal_score.contribute_val;
		info.rank_point = 0;
		
		info.rank_val = (int)(fac.combat_time_fac * personal_score.damage_minor_str 
			+ fac.attend_time_fac * personal_score.hurt * personal_score.minor_str / 10000 
			+ fac.kill_cnt_fac * personal_score.kill_minor_str / 10 
			+ (float)personal_score.minor_str / 1000);
		
		list.push_back(info);
	}
	//¸ù¾İÅÅÃû²Î¿¼Öµ½øĞĞÅÅĞò
	std::sort(list.begin(), list.end(), compare_rank);
	
	int total_rank_point = 0;
	for(unsigned int i = 0; i < list.size(); ++i) {
		PlayerRankInfo& info = list[i];
		
		if(i < RANK_LIST_MAX) {
			info.rank_point = rank_point_list[i];
		} else  if( (i >= RANK_LIST_MAX) && (i < (RANK_LIST_MAX + 10)) ) {
			info.rank_point = 3;
		} else {
			info.rank_point = 2;
		}

		total_rank_point += info.rank_point;
	}

	const int total_rank_point_min = 152;
	if(total_rank_point < total_rank_point_min) total_rank_point = total_rank_point_min;

	CountryBattleSingleBattleResult proto;
	proto.domain_id = battleid;
	proto.single_battle_total_score = total_score;

	for(unsigned int i = 0; i < list.size(); ++i) {
		PlayerRankInfo& info = list[i];
		
		PLAYER_ENTRY_MAP::iterator it_player = _player_map.find(info.roleid);
		if(it_player == _player_map.end()) continue;
		char country_id = it_player->second.country_id;
		unsigned int linksid = it_player->second.linksid;
		unsigned int localsid = it_player->second.localsid;

		OccupationFactor& fac = _occupation_fac_list[info.cls];
		float win_lose_fac = is_winner ? fac.win_fac : fac.fail_fac;
		float point = ((float)info.rank_point / total_rank_point * total_score + info.contribute_val * 15) * win_lose_fac;

		const int point_ceil = 2150;
		if(point > point_ceil) point = point_ceil; //Íæ¼Ò¸öÈËµÃ·Ö²»ÄÜ³¬¹ıÉÏÏŞ
	
		//¸ù¾İÊÇ·ñÊ¹ÓÃÁË¼²Ô®Áî£¬¼ÆËãµÃ·Ö¼Ó³É
		if(battle_config_mask & BATTLE_CONFIG_ASSAULT1) {
			point *= 1.25;
		} else if(battle_config_mask & BATTLE_CONFIG_ASSAULT2) {
			point *= 1.5;
		}
		
		LOG_TRACE("countrybattle calc player score, roleid=%d, cls=%d, rank=%d, rank_val=%d, rank_point=%d, win_lose_fac=%f, contribute_val=%d, player_point=%f, battleid=%d, total_score=%d", 
			info.roleid, info.cls, i + 1, info.rank_val, info.rank_point, win_lose_fac, info.contribute_val, point, battleid, total_score);
		
		_country_info[country_id - 1].player_total_scores += point;

		PLAYER_SCORE_MAP::iterator it_score = _player_score_map.find(info.roleid);
		if(it_score != _player_score_map.end()) {
			PlayerScoreInfo& score_info = it_score->second;
			score_info.dead_cnt += info.dead_cnt;
			if(is_winner) ++score_info.win_cnt;
			score_info.total_combat_time += info.combat_time;
			score_info.total_contribute_val += info.contribute_val;
			score_info.score += point;
		} else {
			int win_cnt = is_winner ? 1 : 0;
			_player_score_map[info.roleid].Init(info.roleid, country_id, win_cnt, info.dead_cnt, info.combat_time, info.contribute_val, point);
		}

		proto.player_single_battle_score = (int)point;
		proto.player_rank = i + 1;
		proto.localsid = localsid;
		GDeliveryServer::GetInstance()->Send(linksid, proto);
	}
}

int CountryBattleMan::CalcSingleBattleTotalScore(int domain_point, int battle_last_time, const std::vector<GCountryBattlePersonalScore>& enemy_score)
{
	const int total_score_max = 49500;

	int total_enemy_damage = 0;
	int total_score = domain_point * 750 + battle_last_time;
	
	for(unsigned int i = 0; i < enemy_score.size(); ++i) {
		total_score += (int)((float)enemy_score[i].minor_str / 60000 * enemy_score[i].attend_time);
		total_enemy_damage += enemy_score[i].damage;
	}

	total_score += (int)((float)total_enemy_damage / 80);
	if(total_score > total_score_max) total_score = total_score_max;
	
	LOG_TRACE("countrybattle single battle total score, last_time=%d, total_score=%d",  battle_last_time, total_score);

	return total_score;
}

void CountryBattleMan::CalcBattleScore(int battleid, int domain_point, int battle_last_time, 
	const std::vector<GCountryBattlePersonalScore>& winner_score, const std::vector<GCountryBattlePersonalScore>& loser_score, char winner_battle_mask, char loser_battle_mask)
{
	
	/*int winner_total_combat_time = 0;
	for(unsigned int i = 0; i < winner_score.size(); ++i) {
		winner_total_combat_time += winner_score[i].combat_time;
	}
	
	int loser_total_combat_time = 0;
	for(unsigned int i = 0; i < loser_score.size(); ++i) {
		loser_total_combat_time += loser_score[i].combat_time;
	}
		
	float winner_total_score = 0.0f;
	for(unsigned int i = 0; i < winner_score.size(); ++i) {
		winner_total_score += CalcPlayerScore(domain_point, winner_total_combat_time, battle_last_time, winner_score[i], true, 0.0f);
	}
	
	float winner_average_score = 0.0f;
	if( (winner_total_score > 0.0f) && (!winner_score.empty()) ) winner_average_score = winner_total_score / winner_score.size();
	
	for(unsigned int i = 0; i < loser_score.size(); ++i) {
		CalcPlayerScore(domain_point, loser_total_combat_time, battle_last_time, loser_score[i], false, winner_average_score);
	}*/

	int winner_total_score = CalcSingleBattleTotalScore(domain_point, battle_last_time, loser_score);
	CalcPlayerScore(battleid, winner_total_score, winner_score, true/*is_winner*/, winner_battle_mask);

	int loser_total_score = CalcSingleBattleTotalScore(domain_point, battle_last_time, winner_score);
	CalcPlayerScore(battleid, loser_total_score, loser_score, false/*is_winner*/, loser_battle_mask);
}

void CountryBattleMan::ClearBattleConfig(DomainInfo& domain)
{
	for(int k = 0; k < COUNTRY_MAX_CNT; ++k) {
		char mask = domain.battle_config_mask[k];
		if(mask & BATTLE_CONFIG_LIMIT) {
			CountryKing& king = _country_info[k].king_info;
			
			for(int i = 0; i < DOMAIN_BATTLE_LIMIT_CNT; ++i) {
				DomainBattleLimit& limit_data = king.battle_limit_config[i];
				if(limit_data.domain_id == domain.id) {
					limit_data.domain_id = 0;
					memset(limit_data.limit, 0, sizeof(limit_data.limit));
					break;
				}
			}
		}
		
		domain.battle_config_mask[k] = 0;
	}
}

bool CountryBattleMan::BattleEnd(int battleid, int result, int defender, int attacker, 
	const std::vector<GCountryBattlePersonalScore>& defender_score, const std::vector<GCountryBattlePersonalScore>& attacker_score)
{
	//Õ½³¡½áÊø
	if(_status != ST_OPEN) return false;
	
	DOMAIN_MAP::iterator it_domain = _domain_map.find(battleid);
	if(it_domain == _domain_map.end()) return false;
	
	DomainInfo& domain = it_domain->second;
	if(domain.status != DOMAIN_STATUS_FIGHT) return false;
	
	DOMAIN2_INFO_SERV* pData = domain2_data_getbyid(battleid);
	if(pData == NULL) return false;
	
	int battle_last_time = Timer::GetTime() - domain.time; //¼ÆËãÕ½ÕùÊ±¼ä
	if(battle_last_time <= 0) return false;
		
	Log::formatlog("countrybattleend","zoneid=%d:battleid=%d:time=%d:result=%d:defender=%d:attacker=%d:group=%d", 
		GDeliveryServer::GetInstance()->zoneid, battleid, time(NULL)/*GetTime()*/, result, defender, attacker, _group_index);

	
	//¼ÆËãÊ¤Àû·½
	int winner = defender;
	int loser = attacker;
	if(result == 1) {
		winner = attacker;
		loser = defender;
	}
	
	domain.owner = winner; //ÉèÖÃÁìµØËùÊôÎªÊ¤Àû·½
	domain.challenger = 0; //Çå³ıÌôÕ½ÕóÓªĞÅÏ¢
	
	//Õ½Õù½áÊø£¬½«ÁìµØÉèÎªÀäÈ´×´Ì¬
	DomainChangeState(domain, DOMAIN_STATUS_COLD, REASON_DOMAIN_BATTLE_END);
	
	//È¡µÃ¹¥ÊØË«·½ÊÇ·ñÊ¹ÓÃ¼²Ô®ÁîµÄmask£¬ÓÃÀ´¼ÆËãÍæ¼Ò±¾³¡Õ½¶·µÄ·ÖÊı¼Ó³É
	char attacker_battle_mask = domain.battle_config_mask[attacker - 1];
	char defender_battle_mask = domain.battle_config_mask[defender - 1];
	
	//Çå¿ÕÁìµØĞÅÏ¢maskÉÏ£¬¹ØÓÚ¼²Ô®ÁîºÍÖ°Òµ»êÁ¦µÄÏŞÖÆ
	ClearBattleConfig(domain);
	
	if(winner == attacker) { //¹¥·½Ê¤Àû
		BroadcastCountryBattleMsg(CMSG_COUNTRYBATTLE_WIN, winner, loser, battleid);
		BroadcastCountryBattleMsg(CMSG_COUNTRYBATTLE_LOSE, loser, winner, battleid);
	}
	
	//¼ÆËãÕ½ÕùµÃ·Ö
	if(winner == defender) {
		CalcBattleScore(battleid, pData->point, battle_last_time, defender_score, attacker_score, defender_battle_mask, attacker_battle_mask);
	} else {
		CalcBattleScore(battleid, pData->point, battle_last_time, attacker_score, defender_score, attacker_battle_mask, defender_battle_mask);	
	}
	
	return true;
}

int CountryBattleMan::GetPlayerDomainId(int roleid)
{
	PLAYER_ENTRY_MAP::iterator it_player = _player_map.find(roleid);
	if(it_player == _player_map.end()) return -1;
	
	return it_player->second.in_domain_id;
}

bool CountryBattleMan::PlayerPreEnter(int battle_id, int roleid)
{
	LOG_TRACE("countrybattlepreenter, battle_id=%d, roleid=%d", battle_id, roleid);

	if(_status != ST_OPEN) return false;
	DOMAIN_MAP::iterator it_domain = _domain_map.find(GetBaseID(battle_id));
	if(it_domain == _domain_map.end()) return false;
		
	DomainInfo& domain = it_domain->second;
	if(domain.status != DOMAIN_STATUS_FIGHT) return false;
	
	std::vector<int>& list = domain.player_list;
	for(unsigned int i = 0; i < list.size(); ++i) {
		if(list[i] == roleid) {
			CountryBattleEnter proto;
			proto.battle_id = MakeUniqueID(_group_index,domain.id);
			proto.roleid = roleid;
			proto.world_tag = GetWorldTagByDomainId(battle_id);
			
			int server_id = GetServerIdByWorldTag(_capital_worldtag);
			if(server_id >= 0) GProviderServer::GetInstance()->DispatchProtocol(server_id, proto);
			
			LOG_TRACE("countrybattleenter send, battle_id=%d, roleid=%d", battle_id, roleid);
			break;
		}
	}

	return true;
}

void CountryBattleMan::PlayerReturnCapital(int roleid)
{
	PLAYER_ENTRY_MAP::iterator it_player = _player_map.find(roleid);
	if(it_player == _player_map.end()) return; //ÔÚ¹úÕ½ÖĞÕÒ²»µ½¸ÃÍæ¼Ò
	
	PlayerEntry& player = it_player->second;
	int src_domain_id = player.in_domain_id;
	
	if(_status == ST_OPEN) { //¹úÕ½×´Ì¬£¬¾­¹ı¼ì²é²ÅÄÜ·µ»Ø
		if(player.worldtag != _capital_worldtag) return; //Íæ¼ÒÖ»ÓĞ´¦ÓÚ¹úÕ½Õ½ÂÔ³¡¾°£¬²Å¿ÉÒÔ·µ»ØÊ×¶¼
		if(player.status != PLAYER_STATUS_NORMAL) return; //Íæ¼Ò´¦ÓÚ·ÇÕı³£Ì¬(ÈçµÈ´ıÕ½¶·£¬Õ½¶·×´Ì¬)£¬²»¿ÉÒÔ½ÓÊÜµ½·µ»ØÊ×¶¼µÄÃüÁî
		if( IsPlayerMoving(roleid) ) return;

		DOMAIN_MAP::iterator it_src_domain = _domain_map.find(src_domain_id);
		if(it_src_domain == _domain_map.end()) return;

		DomainInfo& src_domain = it_src_domain->second;
		//Íæ¼Ò´¦ÓÚ·Ç×Ô¼ºÕóÓªµÄÁìÍÁ£¬²»¿ÉÒÔ·µ»ØÊ×¶¼
		//Íæ¼Ò´¦ÓÚ×Ô¼ºÕóÓªµÄÁìÍÁ£¬½öµ±ÁìÍÁ´¦ÓÚÕı³£×´Ì¬£¬»òÀäÈ´×´Ì¬²Å¿ÉÒÔ·µ»ØÊ×¶¼
		if( (src_domain.owner != player.country_id) || ((src_domain.status != DOMAIN_STATUS_NORMAL) && (src_domain.status != DOMAIN_STATUS_COLD)) ) return;
	} //ÆäËû×´Ì¬£¬¿ÉÒÔËæÊ±»ØÈ¥
	
	CapitalInfo& capital = _capital_info[player.country_id - 1];
	player.in_domain_id = capital.id;
	player.prev_domain_id = capital.id;
	MoveBetweenDomain(roleid, src_domain_id, capital.id);

	SyncPlayerLocation(roleid, capital.id, REASON_SYNC_CAPITAL, player.linksid, player.localsid);
	
	int idx = rand() % 4;
	SyncPlayerPosToGs(roleid, player.worldtag, capital.pos[idx].x, capital.pos[idx].y, capital.pos[idx].z, 1/*is_capital*/);
}

void CountryBattleMan::SendApplyResultToGs(int country_id, const std::vector<CountryBattleApplyEntry>& apply_list, unsigned int sid)
{
	int country_id_invalid_timestamp = GetCountryBattleFinishTime();
	CapitalInfo& capital = _capital_info[country_id - 1];

	int worldtag = GetCapitalWorldTag();

	int idx = rand() % 4;
	float posx = capital.pos[idx].x;
	float posy = capital.pos[idx].y;
	float posz = capital.pos[idx].z;

	std::string list_str = "";

	for(unsigned int i = 0; i < apply_list.size(); ++i) {
		string tmp = "";
		std::stringstream ss;
		ss << apply_list[i].roleid;
		ss >> tmp;

		list_str += (tmp + ",");
	}

	DEBUG_PRINT("countrybattleapply, country_id=%d, time=%d, country_id_invalid_timestamp=%d, apply_list_size=%d, apply_list=%s", 
			country_id, Timer::GetTime(), country_id_invalid_timestamp, apply_list.size(), list_str.c_str());

	CountryBattleApply_Re res(ERR_SUCCESS, MakeUniqueID(_group_index,country_id), country_id_invalid_timestamp, worldtag, posx, posy, posz, apply_list);
	GProviderServer::GetInstance()->Send(sid, res);
}

void CountryBattleMan::PlayersApplyBattleByZoneID( std::vector<CountryBattleApplyEntry>& apply_list, unsigned int sid)
{
	if(apply_list.empty()) return;
	
	typedef std::map< int/*country_id*/, std::vector<CountryBattleApplyEntry> > COUNTRY_APPLY_MAP;
	COUNTRY_APPLY_MAP country_applies;
	
	for(unsigned int i = 0; i < apply_list.size(); ++i) { 
		CrossInfoData* pTmpCrsRole = UserContainer::GetInstance().GetRoleCrossInfo(apply_list[i].roleid);
		if(pTmpCrsRole == NULL) continue;

		ZONE_COUNTRY_MAP::iterator it = _zone_country_map.find(pTmpCrsRole->src_zoneid);
		if(it == _zone_country_map.end()) continue;

		int country_id = it->second;
		
		country_applies[country_id].push_back(apply_list[i]);
	}
	
	COUNTRY_APPLY_MAP::iterator it = country_applies.begin();
	while(it != country_applies.end()) {
		int country_id = it->first;
		std::vector<CountryBattleApplyEntry>& result_list = it->second;
		SendApplyResultToGs(country_id, result_list, sid);
		++it;
	}
}

void CountryBattleMan::PlayersApplyBattleRandom( std::vector<CountryBattleApplyEntry>& apply_list, unsigned int sid)
{
	int country_id = ApplyBattleRandom(apply_list);
	SendApplyResultToGs(country_id, apply_list, sid);
}

void CountryBattleMan::PlayersApplyBattle( std::vector<CountryBattleApplyEntry>& apply_list, unsigned int sid)
{
	if(_status != ST_OPEN) return;
	if(_arrange_country_type == ARRANGE_COUNTRY_BY_ZONEID) {
		PlayersApplyBattleByZoneID(apply_list, sid);
	} else {
		PlayersApplyBattleRandom(apply_list, sid);
	}
}

void CountryBattleMan::MakeCapitalsData(GCountryCapitalVector& capital_list)
{
	for(int j = 0; j < COUNTRY_MAX_CNT; ++j) {
		CapitalInfo& data = _capital_info[j];
		
		for(int i = 0; i < 4; ++i) {			
			GCountryCapital gcountry_capital;
			gcountry_capital.country_id = j + 1;
			gcountry_capital.worldtag = _capital_worldtag;
			gcountry_capital.posx = data.pos[i].x;
			gcountry_capital.posy = data.pos[i].y;
			gcountry_capital.posz = data.pos[i].z;
			capital_list.push_back(gcountry_capital);
		}
	}
}

const std::map<int/*linksid*/, PlayerVector>* CountryBattleMan::GetCountryOnlinePlayers(int roleid)
{
	if(_status != ST_OPEN && _status != ST_BONUS) return NULL;

	PLAYER_ENTRY_MAP::iterator it_player = _player_map.find(roleid);
	if(it_player == _player_map.end()) return NULL; //ÔÚ¹úÕ½ÖĞÕÒ²»µ½¸ÃÍæ¼Ò
	
	PlayerEntry& player = it_player->second;
	CountryInfo& info = _country_info[player.country_id - 1];

	return &(info.communication_map);
}

void CountryBattleMan::AddPlayerCountryCommuicationInfo(int roleid, int country_id, unsigned int linksid, unsigned int localsid)
{
	Player data;
	data.roleid = roleid;
	data.localsid = localsid;

	CountryInfo& info = _country_info[country_id - 1];
	COUNTRY_COMMUNICATION_MAP& map = info.communication_map;

	COUNTRY_COMMUNICATION_MAP::iterator it = map.find(linksid);
	if(it != map.end()) {
		PlayerVector& vec = it->second;

		bool is_find = false;
		for(unsigned int i = 0; i < vec.size(); ++i) {
			if(vec[i].roleid == roleid) {
				vec[i].localsid = localsid;
				is_find = true;
				break;
			}
		}
		
		if(!is_find) vec.push_back(data); 	
	} else {
		PlayerVector vec;
		vec.push_back(data);
		map.insert(std::make_pair(linksid, vec));
	}
}

void CountryBattleMan::RemovePlayerCountryCommuicationInfo(int roleid, int country_id, unsigned int linksid)
{
	CountryInfo& info = _country_info[country_id - 1];
	COUNTRY_COMMUNICATION_MAP& map = info.communication_map;

	COUNTRY_COMMUNICATION_MAP::iterator it = map.find(linksid);
	if(it == map.end()) return;

	PlayerVector& vec = it->second;
	PlayerVector::iterator it_player = vec.begin();
	while(it_player != vec.end()) {
		if(it_player->roleid == roleid) {
			it_player = vec.erase(it_player);
			break;
		}

		++it_player;
	}
}

void CountryBattleMan::BroadcastCountryBattleMsg(int src, int self_country_id, int target_country_id, int domain_id)
{
	CountryInfo& info = _country_info[self_country_id - 1];
	COUNTRY_COMMUNICATION_MAP& map = info.communication_map;
	
	ChatMultiCast chat;
	chat.channel = GN_CHAT_CHANNEL_SYSTEM;
	chat.srcroleid = src;

	Marshal::OctetsStream os;
	os << target_country_id << domain_id;
	chat.msg = os;
	
	for(COUNTRY_COMMUNICATION_MAP::iterator it = map.begin(); it != map.end(); ++it) {
		chat.playerlist = it->second;		
		GDeliveryServer::GetInstance()->Send(it->first, chat);
	}
}

void CountryBattleMan::StopMovingPlayers()
{
	for(MOVE_LIST::iterator it = _move_info.begin(); it != _move_info.end(); ++it) {
		PLAYER_ENTRY_MAP::iterator it_player = _player_map.find(it->roleid);
		if(it_player == _player_map.end()) continue; //ÔÚ¹úÕ½ÖĞÕÒ²»µ½¸ÃÍæ¼Ò

		PlayerEntry& player = it_player->second;
		SyncPlayerLocation(it->roleid, player.in_domain_id, REASON_SYNC_FINISH, player.linksid, player.localsid);
	}
}

int CountryBattleMan::GetTime()
{
	return Timer::GetTime() + _adjust_time; 
}

void CountryBattleMan::SetAdjustTime(int t)
{
	if(t == 0) {
		_adjust_time = 0;
	} else if(t > 0) {
		_adjust_time = t;
	}

	DEBUG_PRINT("CountryBattleMan::SetAdjustTime t=%d", t);
}

void CountryBattleMan::HandlePlayerUnusualSwitchMap(PlayerEntry& player)	
{
	//¸Ä±äÍæ¼ÒÎªÕı³£×´Ì¬£¬ÄÚ²¿»áÇå³ıÍæ¼ÒÒÆ¶¯£¬µÈ´ıÕ½ÕùµÄĞÅÏ¢	
	PlayerChangeState(player, PLAYER_STATUS_NORMAL, REASON_PLAYER_LEAVE_BATTLE);
	//½«Íæ¼ÒÇ¿ĞĞÀ­»ØÊ×¶¼
	int src_domain_id = player.in_domain_id;
	CapitalInfo& capital = _capital_info[player.country_id - 1];
	player.in_domain_id = capital.id;
	player.prev_domain_id = capital.id;
	MoveBetweenDomain(player.roleid, src_domain_id, capital.id);
}

bool CountryBattleMan::IsPlayerKing(const PlayerEntry& player)
{
	CountryKing& king = _country_info[player.country_id - 1].king_info;
	if(king.roleid == player.roleid) return true;
	return false;
}

void CountryBattleMan::BroadcastCountryBattleMsg2(int src, int country_id, const Marshal::OctetsStream& os)
{
	CountryInfo& info = _country_info[country_id - 1];
	COUNTRY_COMMUNICATION_MAP& map = info.communication_map;
	
	ChatMultiCast chat;
	chat.channel = GN_CHAT_CHANNEL_SYSTEM;
	chat.srcroleid = src;
	chat.msg = os;
	
	for(COUNTRY_COMMUNICATION_MAP::iterator it = map.begin(); it != map.end(); ++it) {
		chat.playerlist = it->second;		
		GDeliveryServer::GetInstance()->Send(it->first, chat);
	}
}

void CountryBattleMan::KingAssignAssault(int king_roleid, int domain_id, char assault_type)
{
	PLAYER_ENTRY_MAP::iterator it_player = _player_map.find(king_roleid);
	if(it_player == _player_map.end()) return;
	PlayerEntry& player = it_player->second;
	
	CountryKing& king = _country_info[player.country_id - 1].king_info;
	if(king.roleid != player.roleid) return;

	DOMAIN_MAP::iterator it_domain = _domain_map.find(domain_id);
	if(it_domain == _domain_map.end()) return;
	DomainInfo& domain = it_domain->second;
	
	char mask = domain.battle_config_mask[player.country_id - 1];
	if(mask & (BATTLE_CONFIG_ASSAULT1 | BATTLE_CONFIG_ASSAULT2)) return;
	
	CountryBattleKingAssignAssault_Re re;
	re.retcode = -1;
	re.domain_id = domain_id;
	re.assault_type = assault_type;
	re.localsid = player.localsid;

	if(assault_type == BATTLE_CONFIG_ASSAULT1) {
		if(king.command_point < BATTLE_CONFIG_ASSAULT1_POINT) {
			re.command_point = king.command_point;
			GDeliveryServer::GetInstance()->Send(player.linksid, re);
			return;
		}
		
		king.command_point -= BATTLE_CONFIG_ASSAULT1_POINT;
		mask |= BATTLE_CONFIG_ASSAULT1;
	
	} else if(assault_type == BATTLE_CONFIG_ASSAULT2) {
		if(king.command_point < BATTLE_CONFIG_ASSAULT2_POINT) {
			re.command_point = king.command_point;
			GDeliveryServer::GetInstance()->Send(player.linksid, re);
			return;
		}
		
		king.command_point -= BATTLE_CONFIG_ASSAULT2_POINT;
		mask |= BATTLE_CONFIG_ASSAULT2;
	}
	
	domain.battle_config_mask[player.country_id - 1] = mask;
	
	Marshal::OctetsStream os;
	os << king_roleid << domain_id << assault_type;
	BroadcastCountryBattleMsg2(CMSG_COUNTRYBATTLE_KING_ASSIGN_ASSAULT, player.country_id, os);

	re.retcode = ERR_SUCCESS;
	re.command_point = king.command_point;
	GDeliveryServer::GetInstance()->Send(player.linksid, re);

	LOG_TRACE( "CountryBattle KingAssignAssault king_roleid=%d, domain_id=%d, char assault_type=%d, king_command_point=%d\n", king_roleid, domain_id, assault_type, king.command_point);
}

void CountryBattleMan::ClearBattleLimit(int country_id, CountryKing& king, DomainInfo& domain)
{
	char mask = domain.battle_config_mask[country_id - 1];
	if(!(mask & BATTLE_CONFIG_LIMIT)) return;

	for(int i = 0; i < DOMAIN_BATTLE_LIMIT_CNT; ++i) {
		DomainBattleLimit& limit_data = king.battle_limit_config[i];
		if(limit_data.domain_id == domain.id) {
			limit_data.domain_id = 0;
			memset(limit_data.limit, 0, sizeof(limit_data.limit));

			mask &= ~BATTLE_CONFIG_LIMIT;
			break;
		}
	}

	domain.battle_config_mask[country_id - 1] = mask;
}

void CountryBattleMan::SetBattleLimit(int country_id, CountryKing& king, DomainInfo& domain, const std::vector<GCountryBattleLimit>& battle_limit)
{
	if(battle_limit.size() != MAX_OCCUPATION_CNT) return;

	char mask = domain.battle_config_mask[country_id - 1];
	int idx = -1;
	for(int i = 0; i < DOMAIN_BATTLE_LIMIT_CNT; ++i) {
		DomainBattleLimit& limit_data = king.battle_limit_config[i];

		if(limit_data.domain_id == domain.id) {
			idx = i;
			break;
		} else if(limit_data.domain_id == 0) {
			idx = i;
		}
	}

	if(idx >=0 && idx < DOMAIN_BATTLE_LIMIT_CNT) {
		DomainBattleLimit& limit_data = king.battle_limit_config[idx];
		limit_data.domain_id = domain.id;

		for(int k = 0; k < MAX_OCCUPATION_CNT; ++k) {
			limit_data.limit[k] = battle_limit[k];	
		}

		mask |= BATTLE_CONFIG_LIMIT;

		Marshal::OctetsStream os;
		os << king.roleid << domain.id;
		BroadcastCountryBattleMsg2(CMSG_COUNTRYBATTLE_KING_SET_BATTLE_LIMIT, country_id, os);
	}

	domain.battle_config_mask[country_id - 1] = mask;
}

void CountryBattleMan::KingResetBattleLimit(int king_roleid, int domain_id, char op, const std::vector<GCountryBattleLimit>& battle_limit)
{
	PLAYER_ENTRY_MAP::iterator it_player = _player_map.find(king_roleid);
	if(it_player == _player_map.end()) return;
	PlayerEntry& player = it_player->second;
	
	CountryKing& king = _country_info[player.country_id - 1].king_info;
	if(king.roleid != player.roleid) return;

	DOMAIN_MAP::iterator it_domain = _domain_map.find(domain_id);
	if(it_domain == _domain_map.end()) return;
	DomainInfo& domain = it_domain->second;

	if(op == BATTLE_CONFIG_LIMIT_CLEAR) {
		ClearBattleLimit(player.country_id, king, domain);
	} else if(op == BATTLE_CONFIG_LIMIT_SET) {
		SetBattleLimit(player.country_id, king, domain, battle_limit);
	}
}

void CountryBattleMan::SendBattleLimit(int roleid, int domain_id)
{
	PLAYER_ENTRY_MAP::iterator it_player = _player_map.find(roleid);
	if(it_player == _player_map.end()) return;
	PlayerEntry& player = it_player->second;
	
	DOMAIN_MAP::iterator it_domain = _domain_map.find(domain_id);
	if(it_domain == _domain_map.end()) return;
	DomainInfo& domain = it_domain->second;
	
	char mask = domain.battle_config_mask[player.country_id - 1];
	if(!(mask & BATTLE_CONFIG_LIMIT)) return;
	
	CountryBattleGetBattleLimit_Re re;
	re.retcode = ERR_SUCCESS;
	re.domain_id = domain_id;

	CountryKing& king = _country_info[player.country_id - 1].king_info;	
	bool can_find = false;
	for(int i = 0; i < DOMAIN_BATTLE_LIMIT_CNT; ++i) {
		DomainBattleLimit& info = king.battle_limit_config[i];

		if(info.domain_id == domain_id) {
			can_find = true;
			
			if(roleid == king.roleid) {
				re.limit.insert(re.limit.begin(), info.limit, info.limit + MAX_OCCUPATION_CNT);
			} else {
				re.limit.push_back(info.limit[player.occupation]);	
			}
		}
	}

	if(can_find) {
		re.localsid = player.localsid;
		GDeliveryServer::GetInstance()->Send(player.linksid, re);
	} else {
		Log::log( LOG_ERR, "CountryBattle[%d] SendCountryBattleLimit, limit data not match.", _group_index );
	}
}

bool CountryBattleMan::IsPlayerBeyondBattleLimit(const PlayerEntry& player, const DomainInfo& domain, int occupation_wait_fight_cnt)
{
	char mask = domain.battle_config_mask[player.country_id - 1];
	if(!(mask & BATTLE_CONFIG_LIMIT)) return true; //ÁìµØ²¢Ã»ÓĞÉèÖÃÏŞÖÆ
	
	CountryKing& king = _country_info[player.country_id - 1].king_info;	
	
	bool can_find = false;
	bool ret = false;
	for(int i = 0; i < DOMAIN_BATTLE_LIMIT_CNT; ++i) {
		DomainBattleLimit& info = king.battle_limit_config[i];
		
		if(info.domain_id == domain.id) {
			can_find = true;

			bool flag1 = (player.minor_str >= info.limit[player.occupation].minor_str_floor);
			bool flag2 = (occupation_wait_fight_cnt < info.limit[player.occupation].occupation_cnt_ceil);
			bool flag3 = (player.country_id == domain.owner) && (domain.owner_occupation_cnt[player.occupation] < info.limit[player.occupation].occupation_cnt_ceil);
			bool flag4 = (player.country_id != domain.owner) && (domain.challenger_occupation_cnt[player.occupation] < info.limit[player.occupation].occupation_cnt_ceil);
			
			if(flag1 && flag2 && (flag3 || flag4)) ret = true;
			break;
		}
	}

	if(!can_find) {
		Log::log( LOG_ERR, "CountryBattle[%d] IsPlayerBeyondBattleLimit, limit data not match.", _group_index );
	}

	return ret;
}

void CountryBattleMan::SendKingCmdPoint(int roleid)
{
	PLAYER_ENTRY_MAP::iterator it_player = _player_map.find(roleid);
	if(it_player == _player_map.end()) return;
	PlayerEntry& player = it_player->second;
	
	CountryKing& king = _country_info[player.country_id - 1].king_info;
	if(king.roleid != roleid) return;
	
	CountryBattleGetKingCommandPoint_Re re;
	re.command_point = king.command_point;
	re.localsid = player.localsid;

	GDeliveryServer::GetInstance()->Send(player.linksid, re);
}

char CountryBattleMan::GetDomain2DataType() 
{ 
	return ((_arrange_country_type == ARRANGE_COUNTRY_BY_ZONEID) ? DOMAIN2_TYPE_CROSS : DOMAIN2_TYPE_SINGLE); 
}

int CountryBattleMan::GetBattleCountryCnt(std::set<int>& country_set)
{
	int ret = COUNTRY_MAX_CNT;
	
	for(PLAYER_SCORE_MAP::iterator it = _player_score_map.begin(); it != _player_score_map.end(); ++it) {
		PlayerScoreInfo& info = it->second;	
		country_set.insert(info.country_id);
		
		if(_arrange_country_type != ARRANGE_COUNTRY_BY_ZONEID) {
			if(country_set.size() >= COUNTRY_MAX_CNT) break;
		} else {
			if(country_set.size() >= _zone_country_map.size()) break;
		}
	}

	if(country_set.size() >= 1 && country_set.size() <= COUNTRY_MAX_CNT) {
		ret = country_set.size();
	} 

	return ret;
}

/////////////////////////////////////////////////////////////////////////////////////
// ¿çÎ¨Ò»·ş¹úÕ½ĞÂÔö½Ó¿Ú
/////////////////////////////////////////////////////////////////////////////////////
int CountryBattleMan::GetGroupIdByRoleId(int rid)
{
	PlayerInfo * pinfo = UserContainer::GetInstance().FindRole( rid );
	if(pinfo && pinfo->user)
	{
		if(GDeliveryServer::GetInstance()->IsCentralDS())
			return CentralDeliveryServer::GetInstance()->GetGroupIdByZoneId(pinfo->user->src_zoneid);
		else
			return CountryBattleMan::_default_group; // ·Çµ÷ÊÔ ±¾·şÄ¬ÈÏ0ºÅ·Ö×é
	}
	return -1;	
}

CountryBattleMan* CountryBattleMan::GetActiveCountryBattle(int group)
{
	if(group >= 0 && group < GROUP_MAX_CNT && CountryBattleMan::_instance[group].IsActive())
		return  &CountryBattleMan::_instance[group];
	return NULL;
}

void CountryBattleMan::OnPlayersApplyBattle(std::vector<CountryBattleApplyEntry>& apply_list, unsigned int sid)
{
	typedef	std::map<int/*group_id*/ , std::vector<CountryBattleApplyEntry> > GROUP_APPLY_MAP;
	GROUP_APPLY_MAP	group_apply;
	for(size_t i = 0; i < apply_list.size(); ++i){
		int gid = CountryBattleMan::GetGroupIdByRoleId(apply_list[i].roleid);
		group_apply[gid].push_back(apply_list[i]);
	}

	GROUP_APPLY_MAP::iterator it = group_apply.begin();
	while(it != group_apply.end()){
		int gid = it->first;
		CountryBattleMan* instance = CountryBattleMan::GetActiveCountryBattle(gid);
		if(instance) instance->PlayersApplyBattle(it->second,sid);
		++it;
	}
}

#define ACTIVE_BAT_BEG CountryBattleMan* instance = CountryBattleMan::GetActiveCountryBattle(CountryBattleMan::GetGroupIdByRoleId(rid));\
					  if(instance){							   
#define ACTIVE_BAT_END }

void CountryBattleMan::OnPlayerGetConfig(int rid,unsigned int sid, unsigned int localsid)
{
	if(DisabledSystem::GetDisabled(SYS_COUNTRYBATTLE))
		return;

	PlayerInfo * pinfo = UserContainer::GetInstance().FindRole( rid );
	if(pinfo && pinfo->user)
	{
		int group = CountryBattleMan::_default_group;
		if(GDeliveryServer::GetInstance()->IsCentralDS())
			group = CentralDeliveryServer::GetInstance()->GetGroupIdByZoneId(pinfo->user->src_zoneid);
		if(group < 0 || group > GROUP_MAX_CNT)
			return;
	
		if(!CountryBattleMan::_instance[group].IsActive() && !pinfo->IsGM())		//Ö»ÓĞ gm ÄÜ²éÑ¯·Ç»î¶¯ÆÚ¹úÕ½ĞÅÏ¢
			return;

		CountryBattleMan::_instance[group].SendConfig(rid,sid,localsid);
	}
}

void CountryBattleMan::OnPlayerJoinBattle(int rid, int country_id, int world_tag, int major_strength, int minor_strength, char is_king)
{
	ACTIVE_BAT_BEG
	instance->JoinBattle(rid, GetBaseID(country_id), world_tag, major_strength, minor_strength, is_king);
	ACTIVE_BAT_END
}

void CountryBattleMan::OnPlayerLeaveBattle(int rid, int country_id, int major_strength, int minor_strength)
{
	ACTIVE_BAT_BEG
	instance->LeaveBattle(rid, GetBaseID(country_id), major_strength, minor_strength);
	ACTIVE_BAT_END
}

const std::map<int/*linksid*/, PlayerVector>* CountryBattleMan::OnGetCountryOnlinePlayers(int rid)
{
	ACTIVE_BAT_BEG
	return instance->GetCountryOnlinePlayers(rid);	
	ACTIVE_BAT_END
	return NULL;	
}

int CountryBattleMan::OnPlayerMove(int rid, int dest)
{
	ACTIVE_BAT_BEG
	return instance->PlayerMove(rid,dest);
	ACTIVE_BAT_END
	return -1;
}

int CountryBattleMan::OnPlayerStopMove(int rid)
{
	ACTIVE_BAT_BEG
	return instance->PlayerStopMove(rid);
	ACTIVE_BAT_END
	return -1;
}

void CountryBattleMan::OnPlayerGetMap(int rid, unsigned int sid, unsigned int localsid)
{
	ACTIVE_BAT_BEG
	instance->SendMap(rid,sid,localsid);
	ACTIVE_BAT_END
}

int CountryBattleMan::OnPlayerGetDomainId(int rid)
{
	ACTIVE_BAT_BEG
	return instance->GetPlayerDomainId(rid);
	ACTIVE_BAT_END
	return -1;
}

void CountryBattleMan::OnPlayerGetScore(int rid, unsigned int sid, unsigned int localsid)
{
	ACTIVE_BAT_BEG
	instance->SendCountryScore(rid, sid, localsid);
	ACTIVE_BAT_END
}

bool CountryBattleMan::OnPlayerPreEnter(int rid, int battle_id)
{
	ACTIVE_BAT_BEG
	return instance->PlayerPreEnter(battle_id,rid);
	ACTIVE_BAT_END
	return false;	
}

void CountryBattleMan::OnPlayerReturnCapital(int rid)
{
	ACTIVE_BAT_BEG
	instance->PlayerReturnCapital(rid);
	ACTIVE_BAT_END
}

void CountryBattleMan::OnKingAssignAssault(int rid, int domain_id, char assault_type)
{
	ACTIVE_BAT_BEG
	instance->KingAssignAssault(rid,domain_id,assault_type);
	ACTIVE_BAT_END
}

void CountryBattleMan::OnKingResetBattleLimit(int rid, int domain_id, char op, const std::vector<GCountryBattleLimit>& limit)
{
	ACTIVE_BAT_BEG
	instance->KingResetBattleLimit(rid,domain_id,op,limit);
	ACTIVE_BAT_END
}

void CountryBattleMan::OnPlayerGetBattleLimit(int rid, int domain_id)
{
	ACTIVE_BAT_BEG
	instance->SendBattleLimit(rid, domain_id);	
	ACTIVE_BAT_END
}

void CountryBattleMan::OnKingGetCommandPoint(int rid)
{
	ACTIVE_BAT_BEG
	instance->SendKingCmdPoint(rid);
	ACTIVE_BAT_END
}

void CountryBattleMan::OnPlayerLogin(int rid, int country_id, int world_tag, int minor_str, char is_king)
{
	ACTIVE_BAT_BEG
	instance->PlayerLogin(rid, GetBaseID(country_id), world_tag, minor_str, is_king);
	ACTIVE_BAT_END
}

void CountryBattleMan::OnPlayerLogout(int rid, int country_id)
{
	ACTIVE_BAT_BEG
	instance->PlayerLogout(rid, GetBaseID(country_id));	
	ACTIVE_BAT_END
}

void CountryBattleMan::OnPlayerEnterMap(int rid, int worldtag)
{
	ACTIVE_BAT_BEG
	instance->PlayerEnterMap(rid,worldtag);
	ACTIVE_BAT_END
}

void CountryBattleMan::OnRegisterServer(int server_type, int war_type, int server_id, int worldtag)
{
	CountryBattleMan::_instance[0].RegisterServer(server_type, war_type, server_id, worldtag);
	for(size_t i = 1; i < GROUP_MAX_CNT; ++i)
		CountryBattleMan::_instance[i].CloneServerInfo(CountryBattleMan::_instance[0]);
}

bool CountryBattleMan::OnBattleStart(int battleid, int worldtag, int retcode, int defender, int attacker)
{
	int group = GetGroupID(battleid);
	CountryBattleMan* instance = CountryBattleMan::GetActiveCountryBattle(group);
	if(instance) return instance->BattleStart(GetBaseID(battleid), worldtag, retcode, defender, attacker);
	return false;
}

bool CountryBattleMan::OnBattleEnd(int battleid, int result, int defender, int attacker, 
		const std::vector<GCountryBattlePersonalScore>& defender_score, const std::vector<GCountryBattlePersonalScore>& attacker_score)
{
	int group = GetGroupID(battleid);
	CountryBattleMan* instance = CountryBattleMan::GetActiveCountryBattle(group);
	if(instance) return instance->BattleEnd(GetBaseID(battleid), result, defender, attacker, defender_score, attacker_score);
	return false;
}

bool CountryBattleMan::OnInitialize(int cur_group, int group_count, bool arrange_country_by_zoneid)
{
	CountryBattleMan::_default_group = 0;
	bool ret = true;
	group_count = GDeliveryServer::GetInstance()->IsCentralDS() ?  group_count : 1; // 1 for local countrybattle
	group_count = group_count < GROUP_MAX_CNT ? group_count : GROUP_MAX_CNT;
	for(int i = 0; i < group_count; ++i){
		ret &= CountryBattleMan::_instance[i].Initialize(i,arrange_country_by_zoneid);
	}

	return ret;
}

void CountryBattleMan::OnSetCountryIDCtrl(int id)
{
	 for(int i = 0; i < GROUP_MAX_CNT; ++i){	
		 CountryBattleMan::_instance[i].SetCountryIDCtrl(id);
	 }
}

void CountryBattleMan::OnSetAdjustTime(int t)
{
	 for(int i = 0; i < GROUP_MAX_CNT; ++i){	
		 CountryBattleMan::_instance[i].SetAdjustTime(t);
	 }
}

void  CountryBattleMan::OnSetDefaultGroup(int gid)
{
	CountryBattleMan::_default_group = gid;
}

