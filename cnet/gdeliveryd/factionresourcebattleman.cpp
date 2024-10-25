#include <sstream>

#include "factionresourcebattleman.h"
#include "battlemanager.h"
#include "conf.h"
#include "glog.h"
#include "timer.h"
#include "parsestring.h"
#include "gproviderserver.hpp"
#include "gfactionclient.hpp"
#include "mapuser.h"
#include "gdeliveryserver.hpp"
#include "gamedbclient.hpp"
#include "citywar"
#include "gfactionresourcebattlelimit"
#include "factionresourcebattlerequestconfig.hpp"
#include "factionresourcebattlestatusnotice.hpp"
#include "factionresourcebattlelimitnotice.hpp"
#include "factionresourcebattleplayerqueryresult.hpp"
#include "factionresourcebattlegetmap_re.hpp"
#include "factionresourcebattlegetrecord_re.hpp"
#include "factionresourcebattlenotifyplayerevent.hpp"
#include "dbfactionresourcebattlebonus.hrp"
#include "factionchat.hpp"
#include "maplinkserver.h"
#include "chatbroadcast.hpp"

namespace GNET
{

bool FactionResourceBattleMan::Initialize()
{
    std::string key = "FACTIONRESOURCEBATTLE";
    Conf* conf = Conf::GetInstance();
    
    std::string open_day_str = conf->find(key, "open_day");
    std::vector<string> open_day_vec;
    if (!ParseStrings(open_day_str, open_day_vec)) {
        Log::log( LOG_ERR,"FactionResourceBattleMan Initialize failed. opendaystr=%s.", open_day_str.c_str());
        return false;
    }
    if (open_day_vec.size() > WEEK_DAY_CNT) {
        Log::log( LOG_ERR, "FactionResourceBattleMan Initialize failed. open_day_list count is invalid." );        
        return false;
    }
    _open_days.clear();
    for (unsigned int i = 0; i < open_day_vec.size(); ++i) {
        int openday = atoi(open_day_vec[i].c_str());
        if (openday < 0 || openday >= WEEK_DAY_CNT) {
            Log::log( LOG_ERR,"FactionResourceBattleMan Initialize failed. openday=%d.", openday);
            return false;
        }
        _open_days.push_back(openday);
        //_open_day_list[openday] = 1;
        //LOG_TRACE("FactionResourceBattleMan Initialize open_day = %d\n", openday);
    }
    
    memset(_open_day_list, 0, sizeof(_open_day_list));
    int index = rand() % _open_days.size();
    int day = _open_days[index];
    _open_day_list[day] = 1;
    LOG_TRACE("FactionResourceBattleMan Initialize open_day = %d\n", day);

    int hour = 0;
    int min = 0;
    std::string start_time_str = conf->find(key, "start_time");
    sscanf(start_time_str.c_str(), "%d:%d", &hour, &min);
    _start_time = hour * 3600 + min * 60;
    if ((_start_time < BATTLE_START_TIME_MIN) || (_start_time >= BATTLE_BONUS_TIME_MAX)) {
        Log::log( LOG_ERR,"FactionResourceBattleMan Initialize failed. start_time=%s.", start_time_str.c_str());
        return false;
    }
    LOG_TRACE("FactionResourceBattleMan Initialize start_hour = %d start_min=%d\n", hour, min);

    std::string end_time_str = conf->find(key, "end_time");
    sscanf(end_time_str.c_str(), "%d:%d", &hour, &min);
    _end_time = hour * 3600 + min * 60;
    if ((_end_time < BATTLE_START_TIME_MIN) || (_end_time >= BATTLE_BONUS_TIME_MAX)) {
        Log::log( LOG_ERR,"FactionResourceBattleMan Initialize failed. end_time=%s.", end_time_str.c_str());
        return false;
    }
    LOG_TRACE("FactionResourceBattleMan Initialize end_hour = %d end_min=%d\n", hour, min);
    
    if (_end_time <= _start_time) {
        Log::log(LOG_ERR, "FactionResourceBattleMan Initialize failed. starttime=%d endtime=%d.", _start_time, _end_time);
        return false;
    }
    _clear_time = _end_time + 3600; //战场结束后一小时内为发奖时间，一小时后清理战场

    _bonus_item_id = atoi(conf->find(key, "bonus_item_id").c_str());
    if (_bonus_item_id <= 0) {
        Log::log(LOG_ERR,"FactionResourceBattleMan Initialize wrong. bonus_item_id=%d", _bonus_item_id);
        return false;
    }

    _bonus_max_count = atoi(conf->find(key, "bonus_max_count").c_str());
    if (_bonus_max_count <= 0) {
        Log::log(LOG_ERR,"FactionResourceBattleMan Initialize wrong. bonus_max_count=%d", _bonus_max_count);
        return false;
    }
    
    _bonus_proctype = atoi(conf->find(key, "bonus_proctype").c_str());
    if (_bonus_proctype <= 0) {
        Log::log(LOG_ERR,"FactionResourceBattleMan Initialize wrong. bonus_proctype=%d", _bonus_proctype);
        return false;
    }
    
    IntervalTimer::Attach(this, 1000000/IntervalTimer::Resolution());
    return true;
}

bool FactionResourceBattleMan::InitDomainInfo()
{
    CityWar war; 
    BattleManager::GetInstance()->GetCityInfo(war);

    for (unsigned int i = 0; i < war.citylist.size(); ++i) {
        GCity& city = war.citylist[i];
        if (city.owner <= 0) return false;

        FACTION_INFO_MAP::iterator it = _factions.find(city.owner);
        if (it == _factions.end()) {
            FactionInfo info;
            info.faction_id = city.owner;
            info.score = 0;
            info.rob_minecar_count = 0;
            info.rob_minebase_count = 0;
            info.domains.push_back(city);

            _factions[city.owner] = info;
        } else {
            FactionInfo& info = it->second;
            info.domains.push_back(city);
        }
    }

    return true;
}

void FactionResourceBattleMan::RequestConfigFromGs()
{
    FactionResourceBattleRequestConfig proto;
    proto.timestamp = Timer::GetTime();
    GProviderServer::GetInstance()->DispatchProtocol(1, proto);
}

bool FactionResourceBattleMan::Update()
{
    time_t now = GetTime();

    struct tm dt;
    localtime_r(&now, &dt);
    _second_of_day = dt.tm_hour * 3600 + dt.tm_min * 60 + dt.tm_sec;
    
    if (_status == STAT_CLOSE) {
        bool is_open_day = _open_day_list[dt.tm_wday]; //根据配置判断是否为开启日
        bool is_open_time = (_second_of_day >= _start_time) && (_second_of_day < _end_time);
        
        if(is_open_day && is_open_time) {
            Clear();
            _status = STAT_OPEN;
            _open_timestamp = now;
            _is_status_changed = 1;
            LOG_TRACE("FactionResourceBattle System Change State %d---->%d. open_day=%d\n", STAT_CLOSE, STAT_OPEN, dt.tm_wday);
        }
    } else if (_status == STAT_OPEN) {
        if ((_second_of_day >= _end_time) && (_second_of_day < _clear_time)) { //是发奖时间
            _status = STAT_BONUS;
            _is_status_changed = 1;
            LOG_TRACE("FactionResourceBattle System Change State %d---->%d.\n", STAT_OPEN, STAT_BONUS);

            CalcBonus();

        } else {
            if (_is_domain_loaded == 0) return true;
            if (_factions.empty()) {
                bool ret = InitDomainInfo();
                if (!ret) {
                    _status = STAT_ERROR;
                    _is_status_changed = 1;
                    LOG_TRACE("FactionResourceBattle System Change State %d---->%d.\n", STAT_OPEN, STAT_ERROR);
                    return true;
                }
            }

            if (_config_data_list.empty() || _controller_list.empty()) { 
                if (_config_request_counter == 0) {
                    RequestConfigFromGs();
                }
                
                if (++_config_request_counter == CONFIG_REQUEST_COUNTER_MAX) _config_request_counter = 0;
                return true;
            }
            

            if (_is_status_changed) { 
                NotifyGsBattleStatus(BATTLE_OPEN);
                SyncAllFactionBattleInfo();
                BroadcastBattleStatus(CMSG_FRB_BATTLE_OPEN);
                _is_status_changed = 0;
            }
        }
    } else if (_status == STAT_BONUS) {
        if (_is_status_changed) { 
            NotifyGsBattleStatus(BATTLE_CLOSE);
            SyncAllFactionBattleInfo();
            BroadcastBattleStatus(CMSG_FRB_BATTLE_CLOSE);
            _is_status_changed = 0;
        }

        if(_player_bonus_list.empty()) {
            if(_second_of_day >= _clear_time) { //清空战场信息
                _status = STAT_CLOSE;
                _is_status_changed = 1;
                LOG_TRACE("FactionResourceBattle System Change State %d---->%d.\n", STAT_BONUS, STAT_CLOSE);

                //Clear();
            }
        } else {    
            SendBonus();
        }
        
        //这里强行将系统转换为ST_CLOSE，清除数据，并记录Error Log
        if(_second_of_day >= (_clear_time + TEN_MINUTES)) { //清空战场信息
            _status = STAT_CLOSE;
            _is_status_changed = 1;
            LOG_TRACE("FactionResourceBattle System Change State %d---->%d.\n", STAT_BONUS, STAT_CLOSE);

            //Clear();
            Log::log(LOG_ERR, "FactionResourceBattle system didn't send all bonus in time" );
        }
    } else if (_status == STAT_ERROR) {
        if (_second_of_day >= _clear_time) { //清空战场信息
            _status = STAT_CLOSE;
            _is_status_changed = 1;
            LOG_TRACE("FactionResourceBattle Change State %d---->%d.\n", STAT_ERROR, STAT_CLOSE);

            //Clear();
            return true;
        }
    }

    return true;
}

bool CompareConfigData(const GFactionResourceBattleConfig& config1, const GFactionResourceBattleConfig& config2)
{
    return config1.domain_count > config2.domain_count;
}

void FactionResourceBattleMan::SetConfigData(std::vector<GFactionResourceBattleConfig>& config_list, std::vector<int>& controller_list)
{
    _config_data_list.swap(config_list);
    std::sort(_config_data_list.begin(), _config_data_list.end(), CompareConfigData);
    _controller_list.swap(controller_list);
    _cur_battle_controller_list.clear();
    CalcResourceControllerList(_cur_battle_controller_list);
    LOG_TRACE("FactionResourceBattle SetConfigData data_size=%d, controller_list_size=%d\n", _config_data_list.size(), _controller_list.size());
}

int FactionResourceBattleMan::GetTime() const
{
    return Timer::GetTime() + _adjust_time;
}

void FactionResourceBattleMan::Clear()
{
    _open_timestamp = 0;
    _db_send_bonus_per_sec = 0;
    _config_request_counter = 0;
    _is_status_changed = 0;

    _factions.clear();
    _config_data_list.clear();
    _controller_list.clear();
    _cur_battle_controller_list.clear();
    _domain_minecars.clear();
    _player_scores.clear();
    _player_bonus_list.clear();
}

void FactionResourceBattleMan::RegisterServerInfo(int world_tag, int server_id)
{
    LOG_TRACE( "FactionResourceBattle Register Server: world_tag=%d, server_id=%d.\n", world_tag, server_id);
    
    bool is_find = false;
    for (unsigned int i = 0; i < _servers.size(); ++i) {
        if (server_id == _servers[i].server_id) { //已经注册过，更新注册信息
            _servers[i].Init(world_tag, server_id);
            is_find = true;
            break;
        }
    }
    
    if (!is_find) {
        RegisterInfo info;
        info.Init(world_tag, server_id);
        _servers.push_back(info);
    }
    
    FactionResourceBattleStatusNotice proto;
    proto.status = BATTLE_CLOSE;
    if ((_status == STAT_OPEN) && !_factions.empty() && !_config_data_list.empty()) {
        proto.status = BATTLE_OPEN;
        proto.controller_list = _cur_battle_controller_list;
    }       
    
    GProviderServer::GetInstance()->DispatchProtocol(server_id, proto);
}

void FactionResourceBattleMan::NotifyGsBattleStatus(char status)
{
    FactionResourceBattleStatusNotice proto;
    proto.status = status;
    proto.controller_list = _cur_battle_controller_list;

    for (unsigned int i = 0; i < _servers.size(); ++i) {
        GProviderServer::GetInstance()->DispatchProtocol(_servers[i].server_id, proto);
    }
}

void FactionResourceBattleMan::DoCalcFactionController(CONTROLLER_LIST& list, FactionInfo& info)
{
    std::set<int> domain_id_set;
    int domain_count = info.domains.size();
    for (unsigned int i = 0; i < _config_data_list.size(); ++i) {
        GFactionResourceBattleConfig& config = _config_data_list[i];
        if (domain_count >= config.domain_count) {
            info.score += config.bonus_base;
            while (true) {
                unsigned int index = rand() % domain_count;
                int id = info.domains[index].id;
                domain_id_set.insert(id);

                if (domain_id_set.size() >= config.minebase_count) break;
            }
        
            break;
        }
    }
    
    std::string tmp = "";
    std::stringstream ss;
    ss << "faction_id=" << info.faction_id << ",faction_domain_size=" << domain_count << ",random_domain_id=";
    for (std::set<int>::iterator it = domain_id_set.begin(); it != domain_id_set.end(); ++it) {
        unsigned int controller_index = *it - 1;
        list.push_back(_controller_list[controller_index]);
        _domain_minecars[*it] = DOMAIN_MINE_CAR_MAX_COUNT;

        ss << *it << "(" << _controller_list[controller_index] << "),";
    }
    ss >> tmp;
    DEBUG_PRINT("FactionResourceBattle, random faction dommain, %s\n", tmp.c_str());
}

void FactionResourceBattleMan::CalcResourceControllerList(CONTROLLER_LIST& list)
{
    FACTION_INFO_MAP::iterator it = _factions.begin();
    while (it != _factions.end()) {
        FactionInfo& info = it->second;
        DoCalcFactionController(list, info);
        ++it;
    }
}

int FactionResourceBattleMan::CalcBattleLimitMask(const FactionInfo& info)
{
    int mask = LIMIT_MASK_ALL;
    int domain_count = info.domains.size();
    for (unsigned int i = 0; i < _config_data_list.size(); ++i) {
        GFactionResourceBattleConfig& config = _config_data_list[i];
        if (domain_count >= config.domain_count) {
            if (info.rob_minecar_count >= config.rob_minecar_limit) {
                mask &= ~LIMIT_MASK_MINECAR;
            }
            
            if (info.rob_minebase_count >= config.rob_minebase_limit) {
                mask &= ~LIMIT_MASK_MINEBASE;
            }
            
            break;
        }
    }

    return mask;
}

void FactionResourceBattleMan::SyncAllFactionBattleInfo()
{
    FactionResourceBattleLimitNotice proto;
    char result = BATTLE_CLOSE;
    if ((_status == STAT_OPEN) && !_factions.empty() && !_config_data_list.empty()) {
        result = BATTLE_OPEN;

        FACTION_INFO_MAP::iterator it = _factions.begin();
        while (it != _factions.end()) {
            FactionInfo& info = it->second;
            int mask = CalcBattleLimitMask(info);
            GFactionResourceBattleLimit limit;
            limit.faction_id = info.faction_id;
            limit.limit_mask = mask;
            proto.limit_data.push_back(limit);
            
            ++it;
        }
    } 

    proto.status = result;
    GFactionClient::GetInstance()->SendProtocol(proto);
}

void FactionResourceBattleMan::SyncFactionBattleInfoOnEvent(int faction_id, LIMIT_MASK notice_mask)
{
    if ((_status == STAT_OPEN) && !_factions.empty() && !_config_data_list.empty()) {
        FactionResourceBattleLimitNotice proto;
        proto.status = BATTLE_OPEN;
        
        FACTION_INFO_MAP::iterator it = _factions.find(faction_id);
        int mask = LIMIT_MASK_NONE;

        if (it != _factions.end()) {
            FactionInfo& info = it->second;
            mask = CalcBattleLimitMask(info);
        } else {
            FactionInfo info;
            info.faction_id = faction_id;
            info.score = 0;
            info.rob_minecar_count = 0;
            info.rob_minebase_count = 0;
            info.domains.clear();
            info.bonus_players.clear();
            _factions[faction_id] = info;

            mask = CalcBattleLimitMask(info);
        }

        if ((notice_mask == LIMIT_MASK_ALL) || ((mask & notice_mask) == 0)) {
            GFactionResourceBattleLimit limit;
            limit.faction_id = faction_id;
            limit.limit_mask = mask;

            proto.limit_data.push_back(limit);
            GFactionClient::GetInstance()->SendProtocol(proto);    
        }
        
        FactionChat chat;
        chat.channel = GN_CHAT_CHANNEL_SYSTEM;
        chat.dst_localsid = faction_id;
        
        if (notice_mask & LIMIT_MASK_BATTLEOPEN) {
            chat.src_roleid = CMSG_FRB_BATTLE_ON_PVP;
            GFactionClient::GetInstance()->SendProtocol(chat);
        }
        
        if ((mask & notice_mask) == 0) {
            if (notice_mask == LIMIT_MASK_MINECAR) {
                chat.src_roleid = CMSG_FRB_MINECAR_LIMIT;
            } else if (notice_mask == LIMIT_MASK_MINEBASE) {
                chat.src_roleid = CMSG_FRB_MINEBASE_LIMIT;
            } else {
                return;
            }

            GFactionClient::GetInstance()->SendProtocol(chat);
        }
    }       
}    

void FactionResourceBattleMan::InserPlayerToFactionBonusSet(int faction_id, int roleid)
{
    FACTION_INFO_MAP::iterator it = _factions.find(faction_id);
    if (it == _factions.end()) return;
    it->second.bonus_players.insert(roleid);
}

void FactionResourceBattleMan::DoUpdateScore(int faction_id, int score, const GFactionResourceBattleRole& role)
{
    PLAYER_SCORE_MAP::iterator it = _player_scores.find(role.roleid);
    if (it == _player_scores.end()) {
        PlayerScore info;
        info.Init(role.roleid, faction_id, role.rank, 0, 0, 0, score);
        _player_scores[info.roleid] = info;

        InserPlayerToFactionBonusSet(faction_id, role.roleid);
    } else {
        it->second.score += score;
    }
}

void FactionResourceBattleMan::NotifyPlayerEvent(char event_type, int roleid, int score)
{
    PlayerInfo* pInfo = UserContainer::GetInstance().FindRoleOnline(roleid);
    if (pInfo == NULL) return;

    FactionResourceBattleNotifyPlayerEvent proto;
    proto.event_type = event_type;
    proto.roleid = roleid;
    proto.score = score;
    proto.localsid = pInfo->localsid;

    GDeliveryServer::GetInstance()->Send(pInfo->linksid, proto);
}

void FactionResourceBattleMan::UpdatePlayerScore(char event_type, int faction_id, int score, 
    const GFactionResourceBattleRole& leader_role, const std::vector<GFactionResourceBattleRole>& members)
{
    DoUpdateScore(faction_id, score, leader_role);
    NotifyPlayerEvent(event_type, leader_role.roleid, score);

    for (unsigned int i = 0; i < members.size(); ++i) {
        DoUpdateScore(faction_id, score, members[i]);
        NotifyPlayerEvent(event_type, members[i].roleid, score);
    }
}

void FactionResourceBattleMan::HandleRobEvent(char event_type, int src_faction, int dest_faction, int domain_id, 
        const GFactionResourceBattleRole& leader_role, const std::vector<GFactionResourceBattleRole>& members)
{
    FACTION_INFO_MAP::iterator it_src = _factions.find(src_faction);
    if (it_src == _factions.end()) return;
    FactionInfo& info_src = it_src->second;

    FACTION_INFO_MAP::iterator it_dest = _factions.find(dest_faction);
    if (it_dest == _factions.end()) return;
    FactionInfo& info_dest = it_dest->second;
    
    const GFactionResourceBattleConfig* pSrcConfig = GetBattleConfig(info_src);
    if (pSrcConfig == NULL) return;
 
    const GFactionResourceBattleConfig* pDestConfig = GetBattleConfig(info_dest);
    if (pDestConfig == NULL) return;
    
    int score = 0;
    if (event_type == EVENT_ROB_MINECAR) {
        if (info_src.rob_minecar_count >= pSrcConfig->rob_minecar_limit) {
            DecreaseDomainMineCar(domain_id);
            return;
        }
        
        score = pDestConfig->bonus_minecar;
        ++info_src.rob_minecar_count;
        SyncFactionBattleInfoOnEvent(src_faction, LIMIT_MASK_MINECAR);
        DecreaseDomainMineCar(domain_id);
    } else if (event_type == EVENT_ROB_MINEBASE) {
        DOMAIN_RESOURCE_MAP::iterator it_domain = _domain_minecars.find(domain_id);
        if (it_domain == _domain_minecars.end()) return;
        if (info_src.rob_minebase_count >= pSrcConfig->rob_minebase_limit) {
            it_domain->second = 0;
            return;
        }

        int minecar_left = it_domain->second;
        score = pDestConfig->bonus_minecar * minecar_left;
        ++info_src.rob_minebase_count;
        SyncFactionBattleInfoOnEvent(src_faction, LIMIT_MASK_MINEBASE);
        
        LOG_TRACE("FactionResourceBattle RobMineBaseEvent event=%d, src_faction=%d, dest_faction=%d, domain_id=%d, leader_roleid=%d, members_size=%d, minecar_left=%d\n", 
            event_type, src_faction, dest_faction, domain_id, leader_role.roleid, members.size(), minecar_left);
        it_domain->second = 0;
    }
    
    info_src.score += score;
    UpdatePlayerScore(event_type, src_faction, score, leader_role, members);
}
 
void FactionResourceBattleMan::UpdateEvent(char event_type, int src_faction, int dest_faction, int domain_id, 
        const GFactionResourceBattleRole& leader_role, const std::vector<GFactionResourceBattleRole>& members)
{
    LOG_TRACE("FactionResourceBattle UpdateEvent event=%d, src_faction=%d, dest_faction=%d, domain_id=%d, leader_roleid=%d, members_size=%d\n", 
        event_type, src_faction, dest_faction, domain_id, leader_role.roleid, members.size());

    switch (event_type) 
    {
    case EVENT_BATTTLE_BEGIN:
    {
        if ((GetTime() - _open_timestamp) > FACTION_BEGIN_BATTLE_TIMEOUT) return;
        SyncFactionBattleInfoOnEvent(src_faction, LIMIT_MASK_ALL);
        break;
    }
    case EVENT_ROB_MINECAR:
    {
        HandleRobEvent(EVENT_ROB_MINECAR, src_faction, dest_faction, domain_id, leader_role, members);
        break;
    }
    case EVENT_ROB_MINEBASE:
    {
        HandleRobEvent(EVENT_ROB_MINEBASE, src_faction, dest_faction, domain_id, leader_role, members);
        break;
    }
    case EVENT_MINECAR_ARRIVED:
    {
        FACTION_INFO_MAP::iterator it = _factions.find(src_faction);
        if (it == _factions.end()) return;
        FactionInfo& info = it->second;
        const GFactionResourceBattleConfig* pConfig = GetBattleConfig(info);
        if (pConfig == NULL) return;

        info.score += pConfig->bonus_minecar;        
        DecreaseDomainMineCar(domain_id);
        break;
    }
    case EVENT_HIJACK_KILL:
    {
        PLAYER_SCORE_MAP::iterator it_killer = _player_scores.find(leader_role.roleid);
        if (it_killer == _player_scores.end()) {
            PlayerScore info;
            info.Init(leader_role.roleid, src_faction, leader_role.rank, 1, 0, 0, 0);
            _player_scores[info.roleid] = info;

            InserPlayerToFactionBonusSet(src_faction, leader_role.roleid);
        } else {
            ++(it_killer->second.kill_count);
        }

        if (members.empty()) return; 
        const GFactionResourceBattleRole& dead_role = members[0];
        PLAYER_SCORE_MAP::iterator it_dead = _player_scores.find(dead_role.roleid);
        if (it_dead != _player_scores.end()) {
            ++(it_dead->second.death_count);
        }
        
        break;
    }
    case EVENT_MINECAR_PROTECT:
    {
        PLAYER_SCORE_MAP::iterator it = _player_scores.find(leader_role.roleid);
        if (it == _player_scores.end()) {
            PlayerScore info;
            info.Init(leader_role.roleid, src_faction, leader_role.rank, 0, 0, 1, 0);
            _player_scores[info.roleid] = info;

            InserPlayerToFactionBonusSet(src_faction, leader_role.roleid);
        } else {
            ++(it->second.use_tool_count);
        }
        
        break;
    }
    case EVENT_NO_OWNER_MINCAR:
    {
        DecreaseDomainMineCar(domain_id);
        break;
    }
    case EVENT_NO_OWNER_MINBASE:
    {
        DOMAIN_RESOURCE_MAP::iterator it_domain = _domain_minecars.find(domain_id);
        if (it_domain == _domain_minecars.end()) return;
        LOG_TRACE("FactionResourceBattle NoOwnerMineBaseEvent event=%d, src_faction=%d, dest_faction=%d, domain_id=%d, leader_roleid=%d, members_size=%d, minecar_left=%d\n", 
            event_type, src_faction, dest_faction, domain_id, leader_role.roleid, members.size(), it_domain->second);
 
        it_domain->second = 0;
        break;
    }
    default:
    {
        break;
    }
    }
}

void FactionResourceBattleMan::PlayerQueryResult(int roleid, int faction_id)
{
    FACTION_INFO_MAP::iterator it = _factions.find(faction_id);
    if (it == _factions.end()) return;
    FactionInfo& info = it->second;
    
    PlayerInfo* pInfo = UserContainer::GetInstance().FindRoleOnline(roleid);
    if (pInfo == NULL) return;

    FactionResourceBattlePlayerQueryResult proto;
    proto.faction_id = faction_id;
    proto.score = info.score;
    proto.rob_minecar_count = info.rob_minecar_count;
    proto.rob_minebase_count = info.rob_minebase_count;
    proto.localsid = pInfo->localsid;
    
    proto.can_get_bonus = 0;
    if (_player_scores.find(roleid) != _player_scores.end()) {
        proto.can_get_bonus = 1;
    }
    
    GDeliveryServer::GetInstance()->Send(pInfo->linksid, proto);
}

const GFactionResourceBattleConfig* FactionResourceBattleMan::GetBattleConfig(const FactionInfo& info)
{
    int domain_count = info.domains.size();
    for (unsigned int i = 0; i < _config_data_list.size(); ++i) {
        GFactionResourceBattleConfig& config = _config_data_list[i];
        if (domain_count >= config.domain_count) return &config;
    }

    return NULL;
}

void FactionResourceBattleMan::DecreaseDomainMineCar(int domain_id)
{
    DOMAIN_RESOURCE_MAP::iterator it = _domain_minecars.find(domain_id);
    if (it == _domain_minecars.end()) return;
    --(it->second);
}

bool FactionResourceBattleMan::SendMap(int roleid, unsigned int sid, unsigned int localsid)
{
    FactionResourceBattleGetMap_Re re;
    re.retcode = 0;
    re.localsid = localsid;

    if (_status != STAT_OPEN) {
        re.retcode = -1;
        GDeliveryServer::GetInstance()->Send(sid, re);
        return false;
    }
        
    for (DOMAIN_RESOURCE_MAP::iterator it = _domain_minecars.begin(); it != _domain_minecars.end(); ++it) {
        if (it->second > 0) re.domains.push_back(it->first);
    }
    
    GDeliveryServer::GetInstance()->Send(sid, re);
    return true;
}

bool FactionResourceBattleMan::SendRecord(int roleid, unsigned int sid, unsigned int localsid)
{
    FactionResourceBattleGetRecord_Re re;
    re.retcode = ERR_SUCCESS;
    re.localsid = localsid;

    if ((_status != STAT_CLOSE) && (_status != STAT_BONUS)) {
        re.retcode = ERR_INVALID_TIME; 
        GDeliveryServer::GetInstance()->Send(sid, re);
        return true;
    }
    
    PLAYER_SCORE_MAP::iterator it = _player_scores.find(roleid);
    if (it == _player_scores.end()) {
        re.retcode = ERR_NO_PERMISSION;
        GDeliveryServer::GetInstance()->Send(sid, re);
        return true;
    }
    
    int faction_id = it->second.faction_id;
    FACTION_INFO_MAP::iterator it_faction = _factions.find(faction_id);
    if (it_faction == _factions.end()) return false; 
    FactionInfo& info = it_faction->second;
    
    BONUS_PLAYER_SET& bonus_players = info.bonus_players;
    for (BONUS_PLAYER_SET::iterator it_bonus = bonus_players.begin(); it_bonus != bonus_players.end(); ++it_bonus) {
        int roleid = *it_bonus;

        PLAYER_SCORE_MAP::iterator it_score = _player_scores.find(roleid);
        if (it_score == _player_scores.end()) continue;
        PlayerScore& score = it_score->second;

        GFactionResourceBattleRecord data;
        data.roleid = score.roleid;
        data.kill_count = score.kill_count;
        data.death_count = score.death_count;
        data.use_tool_count = score.use_tool_count;
        data.score = score.score;

        re.records.push_back(data);
    }

    GDeliveryServer::GetInstance()->Send(sid, re);
    return true;
}

void FactionResourceBattleMan::CalcBonus()
{
    for (FACTION_INFO_MAP::iterator it = _factions.begin(); it != _factions.end(); ++it) {
        FactionInfo& info = it->second;
        BONUS_PLAYER_SET& bonus_players = it->second.bonus_players;
        for (BONUS_PLAYER_SET::iterator it_bonus = bonus_players.begin(); it_bonus != bonus_players.end(); ++it_bonus) {
            int roleid = *it_bonus;

            PLAYER_SCORE_MAP::iterator it_score = _player_scores.find(roleid);
            if (it_score == _player_scores.end()) {
                Log::log(LOG_ERR,"FactionResourceBattleMan CalcBonus error, invalid role. roleid=%d.", roleid);
                continue;
            }
            
            float rank_fac = 1.0f;
            unsigned char rank = it_score->second.faction_rank;
            if (rank == FACTION_MASTER) {
                rank_fac = 2.0f;
            } else if (rank == FACTION_VICEMASTER) {
                rank_fac = 1.8f;
            } else if (rank == FACTION_BODYGUARD) {
                rank_fac = 1.6f;
            } else if (rank == FACTION_POINEER) {
                rank_fac = 1.3f;
            }

            int bonus = (int)((info.score * 100000 * rank_fac) / ((info.score + 5000) * (bonus_players.size() + 30)));

            PlayerBonus data;
            data.roleid = roleid;
            data.bonus = bonus;
            _player_bonus_list.push_back(data);
        }
    }

    _db_send_bonus_per_sec = _player_bonus_list.size() / 3600 + 1; //要在60分钟，即3600秒内将奖励发完，每秒至少一个
}

void FactionResourceBattleMan::SendBonus()
{
    if (!_player_bonus_list.empty()) {
        if (_player_bonus_list.size() <= _db_send_bonus_per_sec) _db_send_bonus_per_sec = _player_bonus_list.size();

        for (unsigned int i = 0; i < _db_send_bonus_per_sec; ++i) {
            PlayerBonus& data = _player_bonus_list[i];
            DBSendBattleBonus(data.roleid, data.bonus);
        }       

        _player_bonus_list.erase(_player_bonus_list.begin(), _player_bonus_list.begin() + _db_send_bonus_per_sec);
    }

}

void FactionResourceBattleMan::DBSendBattleBonus(int roleid, int player_bonus)
{
    GRoleInventory inv;
    inv.id = _bonus_item_id;
    inv.count = player_bonus;
    inv.max_count = _bonus_max_count;
    inv.proctype = _bonus_proctype;

    DBFactionResourceBattleBonusArg arg;
    arg.roleid = roleid;
    arg.money = 0;
    arg.item = inv;

    Log::formatlog("factionresourcebattlebonus","zoneid=%d:roleid=%d:amount=%d", GDeliveryServer::GetInstance()->GetZoneid(), roleid, player_bonus);
    DBFactionResourceBattleBonus* rpc = (DBFactionResourceBattleBonus*)Rpc::Call(RPC_DBFACTIONRESOURCEBATTLEBONUS, arg);
    GameDBClient::GetInstance()->SendProtocol(rpc);
}

void FactionResourceBattleMan::SetAdjustTime(int day, int t)
{
	if(t == 0) {
		_adjust_time = 0;
	} else if(t > 0) {
		_adjust_time = t;
	}
    
	DEBUG_PRINT("FactionResourceBattleMan::SetAdjustTime day=%d t=%d", day, t);
}

void FactionResourceBattleMan::BroadcastBattleStatus(int status)
{
    ChatBroadCast cbc;
    cbc.channel = GN_CHAT_CHANNEL_SYSTEM;
    cbc.srcroleid = status;
    LinkServer::GetInstance().BroadcastProtocol(cbc);
}

}
