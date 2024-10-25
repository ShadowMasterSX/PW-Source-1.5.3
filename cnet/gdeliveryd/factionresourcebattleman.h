#ifndef __GNET_FACTION_RESOURCE_BATTLE_MAN_H
#define __GNET_FACTION_RESOURCE_BATTLE_MAN_H

#include "itimer.h"
#include "gcity"
#include "gfactionresourcebattleconfig"
#include "gfactionresourcebattlerole"

namespace GNET
{

class FactionResourceBattleMan: public IntervalTimer::Observer
{
public:
    enum {
		WEEK_DAY_CNT = 7,
        TEN_MINUTES = 600,
        FACTION_BEGIN_BATTLE_TIMEOUT = 900,
        DOMAIN_MINE_CAR_MAX_COUNT = 200,
        CONFIG_REQUEST_COUNTER_MAX = 5,
		BATTLE_START_TIME_MIN = 0, //00:00 最早00:00开启战场
		BATTLE_BONUS_TIME_MAX = (22 * 3600 + 31 * 60), //最晚22:31 战场结束开始发奖
	};
    
    enum BATTLE_MAN_STAT {
        STAT_CLOSE = 0,		//活动没有开启
		STAT_OPEN,		//活动处于开启状态
		STAT_BONUS,		//结算和发奖
        STAT_ERROR,
    };
      
    enum LIMIT_MASK{
        LIMIT_MASK_NONE = 0x00,
        LIMIT_MASK_BATTLEOPEN = 0x01,
        LIMIT_MASK_MINECAR = 0x02,
        LIMIT_MASK_MINEBASE = 0x04,
        LIMIT_MASK_ALL = (LIMIT_MASK_BATTLEOPEN | LIMIT_MASK_MINECAR | LIMIT_MASK_MINEBASE),
    };
    
    enum {
        EVENT_BATTTLE_BEGIN = 0,
        EVENT_ROB_MINECAR,
        EVENT_ROB_MINEBASE,
        EVENT_MINECAR_ARRIVED,
        EVENT_HIJACK_KILL,
        EVENT_MINECAR_PROTECT,
        EVENT_NO_OWNER_MINCAR,
        EVENT_NO_OWNER_MINBASE,
    };
    
    enum {
        FACTION_MASTER = 2,
        FACTION_VICEMASTER = 3,
        FACTION_BODYGUARD = 4,
        FACTION_POINEER = 5,
    };
    
    enum { //gs和faction server只关心这两个状态
        BATTLE_CLOSE = 0,
        BATTLE_OPEN,
    };

    enum {
        ERR_INVALID_TIME = 1,
        ERR_NO_PERMISSION = 2,
    };

    struct RegisterInfo
	{
		int world_tag;
		int server_id;

		void Init(int world_tag_, int server_id_)
        {
            world_tag = world_tag_;
            server_id = server_id_;
        }
	};
   

    typedef std::vector<GCity> DOMAIN_LIST;
    typedef std::set<int> BONUS_PLAYER_SET;
    struct FactionInfo
    {
        int faction_id;
        unsigned int score;
        unsigned short rob_minecar_count;
        unsigned short rob_minebase_count;
        DOMAIN_LIST domains;
        BONUS_PLAYER_SET bonus_players;
    };
    
    struct PlayerScore
    {
       int roleid;
       int faction_id;
       unsigned char faction_rank;
       int kill_count;
       int death_count;
       int use_tool_count;
       int score;

       void Init(int roleid_, int faction_id_, unsigned char faction_rank_, int kill_count_, int death_count_, int use_tool_count_, int score_)
       {
           roleid = roleid_;
           faction_id = faction_id_;
           faction_rank = faction_rank_;
           kill_count = kill_count_;
           death_count = death_count_;
           use_tool_count = use_tool_count_;
           score = score_;
       }
    };
    
    struct PlayerBonus
	{
		int roleid;
		int bonus;
	};

    typedef std::vector<GFactionResourceBattleConfig> CONFIG_DATA_LIST;
    typedef std::vector<int> CONTROLLER_LIST;
    typedef std::vector<RegisterInfo> SERVER_LIST;
    typedef std::map<int/*faction_id*/, FactionInfo> FACTION_INFO_MAP;
    typedef std::vector<PlayerBonus> PLAYER_BONUS_LIST;
    typedef std::map<int/*roleid*/, PlayerScore> PLAYER_SCORE_MAP;
    typedef std::map<int/*domain_id*/, int/*minecar_count*/> DOMAIN_RESOURCE_MAP;
    typedef std::vector<int> OPEN_DAY_LIST;

private:
    BATTLE_MAN_STAT _status;	//当前状态
    int _start_time;	//活动当天开始时间，从0点计时的秒数
	int _end_time;		//活动当天结束时间，从0点计时的秒数
    int _clear_time;    //清空战场数据时间，从0点计时的秒数
	int _adjust_time;	//时间的修正值，用来调试(秒)
    int _second_of_day;		//当前时间是从0点开始的多少秒,每次Update更新
    unsigned int _db_send_bonus_per_sec;
    int _open_timestamp;
	int _bonus_item_id;		//活动奖励物品的id
	int _bonus_max_count;	//奖励物品的叠加数量上限
	int _bonus_proctype;	//物品的绑定类型
    unsigned char _config_request_counter;
    unsigned char _is_status_changed;
    unsigned char _is_domain_loaded;
    FACTION_INFO_MAP _factions;
    CONFIG_DATA_LIST _config_data_list;
    CONTROLLER_LIST _controller_list;
    CONTROLLER_LIST _cur_battle_controller_list;
    SERVER_LIST _servers;
    DOMAIN_RESOURCE_MAP _domain_minecars;
    PLAYER_SCORE_MAP _player_scores;
    PLAYER_BONUS_LIST _player_bonus_list;
    OPEN_DAY_LIST _open_days;
    unsigned char _open_day_list[WEEK_DAY_CNT];   //活动每周周几开启(0 当天不开启 1当天开启)
private:
    FactionResourceBattleMan(): _status(STAT_CLOSE), _start_time(0), _end_time(0), _clear_time(0), _adjust_time(0), _second_of_day(0), _db_send_bonus_per_sec(0),
        _open_timestamp(0), _bonus_item_id(0), _bonus_max_count(0), _bonus_proctype(0), _config_request_counter(0), _is_status_changed(0), _is_domain_loaded(0)
	{
        memset(_open_day_list, 0, sizeof(_open_day_list));
	}

    int GetTime() const;
    void Clear();
    bool InitDomainInfo();
    void RequestConfigFromGs();
    int CalcBattleLimitMask(const FactionInfo& info);
    void CalcResourceControllerList(CONTROLLER_LIST& list);
    void DoCalcFactionController(CONTROLLER_LIST& list, FactionInfo& info);
    void NotifyGsBattleStatus(char status);
    void CalcBonus();
	void SendBonus();
	void DBSendBattleBonus(int roleid, int player_bonus);
    const GFactionResourceBattleConfig* GetBattleConfig(const FactionInfo& info);
    void DecreaseDomainMineCar(int domain_id);
    void DoUpdateScore(int faction_id, int score, const GFactionResourceBattleRole& role);
    void NotifyPlayerEvent(char event_type, int roleid, int score);
    void UpdatePlayerScore(char event_type, int faction_id, int score, const GFactionResourceBattleRole& leader_role, const std::vector<GFactionResourceBattleRole>& members);
    void HandleRobEvent(char event_type, int src_faction, int dest_faction, int domain_id, 
        const GFactionResourceBattleRole& leader_role, const std::vector<GFactionResourceBattleRole>& members);
    void InserPlayerToFactionBonusSet(int faction_id, int roleid);
    void SyncFactionBattleInfoOnEvent(int faction_id, LIMIT_MASK notice_mask);
    void BroadcastBattleStatus(int status);
    
public:
    static FactionResourceBattleMan* GetInstance() 
	{
		static FactionResourceBattleMan instance;
		return &instance; 
	}

    bool Initialize();
    bool Update();
    void SetConfigData(std::vector<GFactionResourceBattleConfig>& config_list, std::vector<int>& controller_list);
    void RegisterServerInfo(int world_tag, int server_id);
    void SyncAllFactionBattleInfo();
    void UpdateEvent(char event_type, int src_faction, int dest_faction, int domain_id, 
        const GFactionResourceBattleRole& leader_role, const std::vector<GFactionResourceBattleRole>& members);
    void PlayerQueryResult(int roleid, int faction_id);
    bool SendMap(int roleid, unsigned int sid, unsigned int localsid);
    bool SendRecord(int roleid, unsigned int sid, unsigned int localsid);
    void OnDomainLoadComplete() { _is_domain_loaded = 1; }
    void SetAdjustTime(int day, int t);
};

}

#endif
