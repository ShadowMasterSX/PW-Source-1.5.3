#ifndef __GNET_COUNTRY_BATTLE_MAN_H
#define __GNET_COUNTRY_BATTLE_MAN_H

#include <vector>
#include <map>

#include "countrybattleapplyentry"
#include "gcountrycapital"
#include "player"
#include "gcountrybattlepersonalscore"
#include "gcountrybattlelimit"
#include "localmacro.h"

namespace GNET
{
#define RANK_LIST_MAX 20
#define WAR_TYPE_MAX 3

const int rank_point_list[RANK_LIST_MAX] = {40, 35, 30, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12 ,11, 10, 9, 8};
const int country_battle_players_cnt[WAR_TYPE_MAX] = {30, 20, 40};

class CountryBattleMan: public IntervalTimer::Observer
{
public:		
	enum {
		ARRANGE_COUNTRY_RANDOM = 0,
		ARRANGE_COUNTRY_BY_ZONEID,
	};
	
	enum {
		WAR_TYPE_FLAG = 0,
		WAR_TYPE_TOWER,
		WAR_TYPE_CRYSTAL,
	};
	
	enum {
		COUNTRY_MAX_CNT = 4,
		GROUP_MAX_CNT = 10,
		PLAYER_WAIT_TIME = 10,
		DOMAIN_WAIT_TIME = 60,
		DOMAIN_COLD_TIME = 60,
		BATTLE_LAST_TIME = 10800,
		SINGLE_BATTLE_END_TIME = 1500,
		TEN_MINUTES = 600,
		BONUS_ITEM_ID = 36343,
		MAX_OCCUPATION_CNT = USER_CLASS_COUNT,
		PLAYER_MAX_BONUS = 10000,
		BONUS_PROCTYPE = 16403,
		DOMAIN_BATTLE_LIMIT_CNT = 7,
		WEEK_DAY_CNT = 7,
		
		COUNTRYBATTLE_START_TIME = (20 * 3600 + 20 * 60), //20:20 开启国战
		COUNTRYBATTLE_BONUS_TIME = (22 * 3600 + 21 * 60), //22:21 国战结束，开始发奖
		COUNTRYBATTLE_CLEAR_TIME = (23 * 3600 + 30 * 60), //23:30 清空国战信息
	};
	
	enum {
		MAJOR_STR_NONE = 0,
		MAJOR_STR_REFINE16,
		MAJOR_STR_FORCE9,
		MAJOR_STR_MAX,
	};
	
	enum {
		ST_CLOSE = 0,
		ST_OPEN,
		ST_BONUS,
		ST_MAX,
	};
	
	enum {
		PLAYER_STATUS_NORMAL = 0,
		PLAYER_STATUS_WAIT_FIGHT,
		PLAYER_STATUS_FIGHT,
		PLAYER_STATUS_MAX,
	};
	
	enum {
		DOMAIN_STATUS_NORMAL = 0,
		DOMAIN_STATUS_WAIT_FIGHT,
		DOMAIN_STATUS_FIGHT,
		DOMAIN_STATUS_COLD,
		DOMAIN_STATUS_MAX,
	};
	
	enum {
		REASON_SYNC_MOVE = 0,
		REASON_SYNC_PREV_STEP,
		REASON_SYNC_CAPITAL,
		REASON_SYNC_CLI_REQUEST,
		REASON_SYNC_FINISH,
	};
	
	enum {
		REASON_PLAYER_ENTER_BATTLE = 0,
		REASON_PLAYER_LEAVE_BATTLE,
		REASON_PLAYER_FINISHI_MOVE,
		REASON_PLAYER_RETURN_PREV,
		REASON_PLAYER_RETURN_CAPITAL,
		REASON_PLAYER_WAIT_FIGHT,
	};
	
	enum {
		REASON_DOMAIN_WAIT_FIGHT = 0,
		REASON_DOMAIN_BATTLE_TIMEOUT,
		REASON_DOMAIN_COLD_TIMEOUT,
		REASON_DOMAIN_BATTLE_START_SUCCESS,		
		REASON_DOMAIN_BATTLE_START_FAILED,	
		REASON_DOMAIN_BATTLE_END,
		REASON_DOMAIN_WAIT_FIGHT_TIMEOUT,
	};

	enum {
		REASON_SYS_HANDLE_LEAVE_BATTLE = 0,
		REASON_SYS_HANDLE_INVALID_DOMAIN,
		REASON_SYS_HANDLE_WAIT_TIMEOUT,
		REASON_SYS_HANDLE_INVALID_BATTLE,
		REASON_SYS_HANDLE_COLD_DOMAIN,
		REASON_SYS_HANDLE_BATTLE_START_TIMEOUT,
		REASON_SYS_HANDLE_BATTLE_START_FAILED,
		REASON_SYS_HANDLE_FIGHT_TIMEOUT,
		REASON_SYS_HANDLE_COLD_DOMAIN_TIMEOUT,
		REASON_SYS_HANDLE_BATTLE_FULL,
		REASON_SYS_HANDLE_UNDER_BATTLE_LIMIT,
	};
	
	enum {
		BATTLE_CONFIG_NULL = 0x00,
		BATTLE_CONFIG_ASSAULT1 = 0x01,
		BATTLE_CONFIG_ASSAULT2 = 0x02,
		BATTLE_CONFIG_LIMIT = 0x80,
	};
	
	enum {
		BATTLE_CONFIG_KING_POINT = 3000,
		BATTLE_CONFIG_ASSAULT1_POINT = 50,
		BATTLE_CONFIG_ASSAULT2_POINT = 100,
		BATTLE_CONFIG_LIMIT_CLEAR = 0,
		BATTLE_CONFIG_LIMIT_SET = 1,
	};

	struct DomainBattleLimit
	{
		int domain_id;
		GCountryBattleLimit limit[MAX_OCCUPATION_CNT];
	};
	
	struct CountryKing
	{
		int roleid;
		int command_point;
		DomainBattleLimit battle_limit_config[DOMAIN_BATTLE_LIMIT_CNT];

		void Init(int roleid_);
	};
	
	struct PlayerEntry
	{
		int roleid;
		char country_id;
		char status;
		int in_domain_id;
		int prev_domain_id;
		int worldtag;
		int occupation;
		int minor_str; 
		unsigned int linksid;
		unsigned int localsid;
		
		void Init(int roleid_, char country_id_, char status_, int in_domain_id_, int prev_domain_id_, int worldtag_, int occupation_, int minor_str_, 
			unsigned int linksid_, unsigned int localsid_);
	};
	
	struct DomainInfo
	{
		int id;
		char owner;
		char challenger;
		char init_country_id;
		char wartype;
		char status;	
		char battle_config_mask[COUNTRY_MAX_CNT];
		char owner_occupation_cnt[MAX_OCCUPATION_CNT];
		char challenger_occupation_cnt[MAX_OCCUPATION_CNT];
		int time; //正常状态该值无用，等待战争状态是超时时间，战斗状态时战斗开启时间，冷却状态是冷却时间
		std::vector<int> player_list;
		
		void Init(int id_, char owner_, char challenger_, char init_country_id_, char wartype_, char status_);
	};
	
	struct CapitalInfo
	{
		int id;
		struct CapitalPos{
			float x;
			float y;
			float z;
		};
		
		CapitalPos pos[4];
	};
	
	typedef std::map<int/*linkid*/, PlayerVector> COUNTRY_COMMUNICATION_MAP;
	struct CountryInfo
	{
		float major_strength;
		int minor_strength;
		int online_player_cnt;
		int domains_cnt; //记录阵营占据多少领地
		int country_scores;
		float player_total_scores; //记录阵营玩家总得分

		CountryKing king_info;
		COUNTRY_COMMUNICATION_MAP communication_map;
	};
	
	struct MoveInfo
	{
		int roleid;
		int from;
		int to;
		int time;
		
		void Init(int roleid_, int from_, int to_, int time_);
	};
	
	struct ColdInfo
	{
		int id;
		int time;
		bool is_notify;

		void Init(int id_, int time_);
	};
	
	struct RegisterInfo
	{
		int war_type;
		int world_tag;
		int server_id;

		void Init(int war_type_, int world_tag_, int server_id_);
	};
	
	struct PlayerScoreInfo
	{
		int roleid;
		char country_id;
		short win_cnt;
		short dead_cnt;
		int total_combat_time;
		int total_contribute_val;
		float score;
		
		void Init(int roleid_, char country_id_, int win_cnt_, int dead_cnt_, int total_combat_time_, int total_contribute_val_, float score_);
	};

	struct OccupationFactor
	{
		float win_fac;
		float fail_fac;
		float attend_time_fac;
		float kill_cnt_fac;
		float death_cnt_fac;
		float combat_time_fac;
	};
	
	struct BonusLimit
	{
		float score_limit;
		int win_cnt_limit;
		int death_cnt_limit;
		int combat_time_limit;
		int contribute_val_limit;
		int total_bonus;
	};
	
	struct PlayerBonus
	{
		int roleid;
		int bonus;
		int zoneid;
	};
	
	struct PlayerRankInfo
	{
		int roleid;
		int cls;
		int dead_cnt;
		int combat_time;
		int contribute_val;
		int rank_val;
		int rank_point;
	};
	
	typedef std::map<int/*domainid*/, DomainInfo> DOMAIN_MAP;
	typedef std::map<int/*roleid*/, PlayerEntry> PLAYER_ENTRY_MAP;
	typedef std::map<int/*roleid*/, PlayerScoreInfo> PLAYER_SCORE_MAP;
	typedef std::vector<MoveInfo> MOVE_LIST;
	typedef std::vector<ColdInfo> PLAYER_WAIT_FIGHT_LIST;
	typedef std::vector<RegisterInfo> SERVER_LIST;
	typedef std::vector<OccupationFactor> OCCUPATION_FAC_LIST;
	typedef std::vector<PlayerBonus> PLAYER_BONUS_LIST;
	typedef std::map<int/*zoneid*/, int/*countryid*/> ZONE_COUNTRY_MAP;

private:
	int _group_index;
	int _arrange_country_type;
	int _status; //国战状态
	int _capital_worldtag; //国战战略场景的worldtag
	int _calc_domains_timer; //计算各阵营占据多少领土的计数器
	int _country_id_ctrl;
	int	_adjust_time; //测试用，当前时间的调整值
	unsigned int _db_send_bonus_per_sec;
	BonusLimit _bonus_limit; //获得奖励条件的结构
	SERVER_LIST _servers; //记录了各个战场副本通信的结构
	DOMAIN_MAP _domain_map; //管理领土的map
	PLAYER_ENTRY_MAP _player_map; //管理国战中玩家信息的map
	PLAYER_SCORE_MAP _player_score_map; //记录玩家国战得分的map
	MOVE_LIST _move_info; //正在移动的玩家信息列表
	PLAYER_WAIT_FIGHT_LIST _players_wait_fight; //处于等待战争状态的玩家列表
	OCCUPATION_FAC_LIST _occupation_fac_list; //国战计算得分的职业系数列表
	PLAYER_BONUS_LIST _player_bonus_list;
	CapitalInfo _capital_info[COUNTRY_MAX_CNT]; //首都信息，用于内部信息查找
	CountryInfo _country_info[COUNTRY_MAX_CNT]; //阵营战力的列表
	unsigned char _open_days[WEEK_DAY_CNT];
	ZONE_COUNTRY_MAP  _zone_country_map;

	static CountryBattleMan _instance[GROUP_MAX_CNT]; 
	static int _default_group;
private:
	CountryBattleMan(): _group_index(-1),_arrange_country_type(ARRANGE_COUNTRY_RANDOM), _status(ST_CLOSE), _capital_worldtag(0), _calc_domains_timer(0), _country_id_ctrl(0), _adjust_time(0), _db_send_bonus_per_sec(0)
	{
		memset(_capital_info, 0, sizeof(_capital_info));
		memset(_open_days, 0, sizeof(_open_days));
		ClearCountryInfo();
	}

	void CloneServerInfo(const CountryBattleMan& cm)
	{
		_servers.clear();
		_servers.insert(_servers.begin(),cm._servers.begin(),cm._servers.end());
		_capital_worldtag = cm._capital_worldtag;		
	}

	bool IsActive() { return _status != ST_CLOSE ; }
	void Clear()
	{
		_calc_domains_timer = 0;
		_db_send_bonus_per_sec = 0;
		_player_score_map.clear();
		_player_bonus_list.clear();
		
		ClearCountryInfo();
		ResetDomainInfo();
	}
	
	/**
	 * 导入domain数据
	 * @param domain2_type 根据type决定读入的配置是普通服还是跨服的中央服
	 * @return true 导入成功 false 导入失败
	 */
	bool InitDomainInfo(char domain2_type);

	/**
	 * 将GS发来主要战力数据换算成delivery需要的值
	 * @param major_str GS服务器上，玩家的装备数值，0代表啥都木有，1代表有16品武器，2代表有9军武器
	 * @return 主要战力在delivery上的数值表示
	 */
	float ConvertMajorStrength(int major_str);
	
	/**
	 * 玩家按随机分配国家的方式加入国战
	 * @param apply_list 申请加入国战的玩家列表
	 * @return 阵营ID
	 */
	int ApplyBattleRandom( std::vector<CountryBattleApplyEntry>& apply_list );
	
	/**
	 * 分配阵营
	 * @param has_major_str 玩家是否拥有主要战斗力
	 * @return 阵营ID
	 */
	int ArrangeCountry(bool has_major_str);
	void ArrangeCountryByZoneID();

	/**
	 * 玩家加入国战
	 * @param roleid 玩家ID
	 * @param country_id 阵营ID
	 * @param world_tag 地图tag
	 * @param minor_str 玩家的魂力值
	 * @return true 加入成功 false 加入失败
	 */
	bool PlayerJoinBattle(int roleid, int country_id, int world_tag, int minor_str);
	
	/**
	 * 玩家离开国战
	 * @param roleid 玩家ID
	 * @param country_id 阵营ID
	 * @return true 离开成功 false 离开失败
	 */
	bool PlayerLeaveBattle(int roleid, int country_id);

	/**
	 * 玩家在战略地图上移动
	 * @param roleid 玩家ID
	 * @param from 起始domain ID
	 * @param to 目标domain ID
	 * @param time 移动需要的时间
	 */
	void PlayerMove(int roleid, int from, int to, int time);
	
	/**
	 * 玩家在战略地图上移动
	 * @param domain 领土信息
	 * @param challenger 挑战阵营ID
	 */
	void StartBattle(DomainInfo& domain, int challenger);

	void SysHandlePlayer(int player_id, int reason);
	
	/**
	 * 取得相邻地图间移动所需要花费的时间
	 * @param domain_src 移动的源地图
	 * @param domain_dest 移动的目标地图
	 * @return 移动需要花费的时间，若返回值小于等于0则说明不相邻
	 */
	int GetMoveUseTime(int domain_src, int domain_dest);
	int GetServerIdByDomainId(int domain_id);
	int GetServerIdByWorldTag(int world_tag);
	int GetWorldTagByDomainId(int domain_id);
	void MoveBetweenDomain(int roleid, int domain_src, int domain_dest);	
	int CalcPlayerBonus(const PlayerScoreInfo& info, int country_bonus[COUNTRY_MAX_CNT]);
	int CalcKingBonus(int country_bonus);
	void CalcBonus();
	void SendBonus();
	void UpdateMoveInfo();
	void UpdateWaitFightPlayers();
	void UpdateWaitFightDomains(DomainInfo& info);
	void UpdateFightDomains(DomainInfo& info);
	void UpdateColdDomains(DomainInfo& info);
	void UpdateDomains();
	void ClearMoveInfo(int roleid);
	void ClearPlayerWaitInfo(int roleid);	
	void ClearPlayerInfo(int roleid);
	void ClearCountryInfo();
	void ResetDomainInfo();
	bool IsPlayerMoving(int roleid);
	bool SyncPlayerLocation(int roleid, int domain_id, int reason, unsigned int linksid, unsigned int localsid);
	bool SyncPlayerPosToGs(int roleid, int worldtag, float posx, float posy, float posz, char is_capital);
	void SendBattleResult(int player_bonus, int country_bonus[COUNTRY_MAX_CNT], unsigned int linksid, unsigned int localsid);
	void DBSendBattleBonus(int roleid, int player_bonus, int zoneid);
	void NotifyBattleConfig(int server_id);
	//float CalcPlayerScore(int domain_point, int total_combat_time, int battle_last_time, 
	//	const GCountryBattlePersonalScore& score, bool is_winner, float winner_average_score);
	void CalcBattleScore(int battleid, int domain_point, int battle_last_time, 
		const std::vector<GCountryBattlePersonalScore>& winner_score, const std::vector<GCountryBattlePersonalScore>& loser_score, char winner_battle_mask, char loser_battle_mask);
	void PlayerChangeState(PlayerEntry& player, char new_status, int reason);
	void DomainChangeState(DomainInfo& domain, char new_status, int reason);
	void MakeCapitalsData(GCountryCapitalVector& capital_list);
	void AddPlayerCountryCommuicationInfo(int roleid, int country_id, unsigned int linksid, unsigned int localsid);
	void RemovePlayerCountryCommuicationInfo(int roleid, int country_id, unsigned int linksid);
	void BroadcastCountryBattleMsg(int src, int self_country_id, int target_country_id, int domain_id);
	void StopMovingPlayers();
	void HandlePlayerUnusualSwitchMap(PlayerEntry& player);	
	int CalcSingleBattleTotalScore(int domain_point, int battle_last_time, const std::vector<GCountryBattlePersonalScore>& enemy_score);
	void CalcPlayerScore(int battleid, int total_score, const std::vector<GCountryBattlePersonalScore>& scores, bool is_winner, char battle_config_mask);
	
	bool IsPlayerKing(const PlayerEntry& player);
	bool IsPlayerBeyondBattleLimit(const PlayerEntry& player, const DomainInfo& domain, int occupation_wait_fight_cnt);
	void BroadcastCountryBattleMsg2(int src, int country_id, const Marshal::OctetsStream& os);
	void ClearBattleLimit(int country_id, CountryKing& king, DomainInfo& domain);
	void SetBattleLimit(int country_id, CountryKing& king, DomainInfo& domain, const std::vector<GCountryBattleLimit>& battle_limit);
	void ClearBattleConfig(DomainInfo& domain);
	void OutputZoneCountryScoreLog();
	int GetBattleCountryCnt(std::set<int>& country_set);

public:
//	static CountryBattleMan* GetInstance() { return &_instance; }    
	static int  GetGroupIdByRoleId(int rid);
	static CountryBattleMan* GetActiveCountryBattle(int group);
	static CountryBattleMan* GetDefault(int index=0) { if(index < 0 || index > GROUP_MAX_CNT ) return &_instance[0]; else return &_instance[index];}

	static void OnSetCountryIDCtrl(int id);
	static void OnSetAdjustTime(int t); 
	static void OnSetDefaultGroup(int gid);
	static const std::map<int/*linksid*/, PlayerVector>* OnGetCountryOnlinePlayers(int rid);
	static bool OnInitialize(int cur_group, int group_count, bool arrange_country_by_zoneid);
	static void OnPlayersApplyBattle(std::vector<CountryBattleApplyEntry>& apply_list, unsigned int sid);
	static void OnPlayerGetConfig(int roleid,unsigned int sid, unsigned int localsid);
	static void OnPlayerJoinBattle(int roleid, int country_id, int world_tag, int major_strength, int minor_strength, char is_king);
	static void OnPlayerLeaveBattle(int roleid, int country_id, int major_strength, int minor_strength);
	static void OnPlayerLogin(int roleid, int country_id, int world_tag, int minor_str, char is_king);
	static void OnPlayerLogout(int roleid, int country_id);
	static void OnPlayerEnterMap(int roleid, int worldtag);
	static int  OnPlayerMove(int roleid, int dest);
	static int  OnPlayerStopMove(int roleid);
	static void OnPlayerGetMap(int roleid, unsigned int sid, unsigned int localsid);
	static void OnPlayerGetScore(int roleid, unsigned int sid, unsigned int localsid);
	static int  OnPlayerGetDomainId(int roleid);
	static void OnPlayerGetBattleLimit(int roleid, int domain_id);
	static bool OnPlayerPreEnter(int roleid, int battle_id);
	static void OnPlayerReturnCapital(int roleid);
	static void OnKingAssignAssault(int king_roleid, int domain_id, char assault_type);
	static void OnKingResetBattleLimit(int king_roleid, int domain_id, char op, const std::vector<GCountryBattleLimit>& limit); 
	static void OnKingGetCommandPoint(int king_roleid);
	static void OnRegisterServer(int server_type, int war_type, int server_id, int worldtag);
	static bool OnBattleStart(int battleid, int worldtag, int retcode, int defender, int attacker);
	static bool OnBattleEnd(int battleid, int result, int defender, int attacker, 
		const std::vector<GCountryBattlePersonalScore>& defender_score, const std::vector<GCountryBattlePersonalScore>& attacker_score);

public:	
	/**
	 * 国战系统初始化
	 * @return true 初始化成功 false 初始化失败
	 */
	bool Initialize(int gid, bool arrange_country_by_zoneid);

	/**
	 * 国战系统是否已经开启
	 * @return true 开启 false 未开启
	 */
	bool IsBattleStart() const { return _status == ST_OPEN; }

	/**
	 * 取得国战战略场景的worldtag
	 * @return 国战战略场景的worldtag
	 */
	int GetCapitalWorldTag() const { return _capital_worldtag; }
	
	/**
	 * 玩家加入国战
	 * @param roleid 玩家ID
	 * @param country_id 阵营ID
	 * @param world_tag 玩家所处场景的world_tag
	 * @param major_strength 玩家主要战力
	 * @param minor_strength 玩家次要战力
	 * @param is_king 是否国王
	 */
	void JoinBattle(int roleid, int country_id, int world_tag, int major_strength, int minor_strength, char is_king);

	/**
	 * 玩家离开国战
	 * @param roleid 玩家ID
	 * @param country_id 阵营ID
	 * @param major_strength 玩家主要战力
	 * @param minor_strength 玩家次要战力
	 */
	void LeaveBattle(int roleid, int country_id, int major_strength, int minor_strength);

	/**
	 * 玩家登陆时，国战系统的处理
	 * @param roleid 玩家ID
	 * @param country_id 阵营ID
	 * @param world_tag 玩家所处场景的world_tag
	 * @param minor_str 玩家次要战力
	 * @param is_king 是否国王
	 */
	void PlayerLogin(int roleid, int country_id, int world_tag, int minor_str, char is_king);

	/**
	 * 玩家登出时，国战系统的处理
	 * @param roleid 玩家ID
	 * @param country_id 阵营ID
	 */
	void PlayerLogout(int roleid, int country_id);

	/**
	 * 玩家切换地图场景时，国战系统的处理
	 * @param roleid 玩家ID
	 * @param world_tag 玩家所处场景的world_tag
	 */
	void PlayerEnterMap(int roleid, int worldtag);

	/**
	 * 玩家接受到客户端移动请求的处理
	 * @param roleid 玩家ID
	 * @param dest 移动的目标领土
	 */
	int PlayerMove(int roleid, int dest);

	/**
	 * 玩家接受到客户端停止移动请求的处理
	 * @param roleid 玩家ID
	 */
	int PlayerStopMove(int roleid);
	
	void PlayerReturnCapital(int roleid);

	void SendApplyResultToGs(int country_id, const std::vector<CountryBattleApplyEntry>& apply_list, unsigned int sid);
	void PlayersApplyBattleByZoneID(std::vector<CountryBattleApplyEntry>& apply_list, unsigned int sid);
	void PlayersApplyBattleRandom(std::vector<CountryBattleApplyEntry>& apply_list, unsigned int sid);
	void PlayersApplyBattle(std::vector<CountryBattleApplyEntry>& apply_list, unsigned int sid);

	/**
	 * 战斗副本开启时的处理
	 * @param battleid 正在战斗的领土ID
	 * @param worldtag 正在战斗领土场景的worldtag
	 * @param retcode 副本开启的结果，0为开启成功，其他为失败
	 * @return true 战斗开始 false 未能正确开始战斗
	 */
	bool BattleStart(int battleid, int worldtag, int retcode, int defender, int attacker);

	/**
	 * 战斗结束时的处理
	 * @param battleid 正在战斗的领土ID
	 * @param result 战斗结果 1为攻方胜利 2为守方胜利
	 * @param defender 守方阵营ID
	 * @param attacker 攻方阵营ID
	 * @param defender_score 守方得分
	 * @param attacker_score 攻方得分
	 * @return true 战斗正常结束 false 战斗未能正确结束
	 */
	bool BattleEnd(int battleid, int result, int defender, int attacker, 
		const std::vector<GCountryBattlePersonalScore>& defender_score, const std::vector<GCountryBattlePersonalScore>& attacker_score);
	bool PlayerPreEnter(int battle_id, int roleid);
	bool SendMap(int roleid, unsigned int sid, unsigned int localsid);
	bool SendConfig(int roleid, unsigned int sid, unsigned int localsid);
	bool SendCountryScore(int roleid, unsigned int sid, unsigned int localsid);
	bool RegisterServer(int server_type, int war_type, int server_id, int worldtag);
	bool Update();
	int GetPlayerDomainId(int roleid);
	
	time_t GetCountryBattleStartTime() 
	{ 
		//用于客户端显示国战开启时间
		int hour = 20;
		int minute = 20;
		return (hour * 3600 + minute * 60);
	}
	
	time_t GetCountryBattleEndTime() 
	{ 
		//用于客户端显示国战战斗结束时间
		int hour = 22;
		int minute = 20;
		return (hour * 3600 + minute * 60);
	}

	time_t GetCountryBattleFinishTime() 
	{ 
		time_t now = GetTime();

		struct tm dt;
		localtime_r(&now, &dt);
		//因为22：20国战战斗结束，23:30发奖结束后清空数据
		//控制国战ID过期时间在22:30:00至23:30:00这一小时之间
		dt.tm_hour = 22;
		dt.tm_min = 30;
		dt.tm_sec = 0;
		int base = mktime(&dt);
		int offset = rand() % 3600; //随机一个一小时(3600秒)之间的秒数
		return (base + offset);
	}

	int GetZoneIDByCountryID(int cid)
	{
		for(ZONE_COUNTRY_MAP::iterator iter = _zone_country_map.begin();
				iter != _zone_country_map.end(); ++ iter)
		{
			if(iter->second == cid)
				return iter->first;
		}
		return 0;		
	}

	int GetTotalBonus() { return _bonus_limit.total_bonus; }
	char GetDomain2DataType();
	void SetCountryIDCtrl(int id) { _country_id_ctrl = id; }
	int GetTime();
	void SetAdjustTime(int t);

	const std::map<int/*linksid*/, PlayerVector>* GetCountryOnlinePlayers(int roleid);
	void KingAssignAssault(int king_roleid, int domain_id, char assault_type);
	void KingResetBattleLimit(int king_roleid, int domain_id, char op, const std::vector<GCountryBattleLimit>& limit);
	void SendBattleLimit(int roleid, int domain_id);
	void SendKingCmdPoint(int roleid);
};

}

#endif
