#ifndef __ONLINEGAME_GS_WORLD_MANANGER_H__
#define __ONLINEGAME_GS_WORLD_MANANGER_H__

#include <string>
#include <hashtab.h>
#include <spinlock.h>
#include <common/types.h>
#include "terrain.h"
#include "template/itemdataman.h"
#include "template/pathman.h"
#include "io/msgio.h"
#include "config.h"
#include "object.h"
#include "player.h"
#include "aiman.h"
#include "playermall.h"
#include "../collision/traceman.h"
#include "timer.h"
#include "forceglobaldataman.h"
#include "substance.h"
#include "npcgenerator.h"
#include "titlemanager.h"
#include "uniquedataclient.h"
#include "historymanager.h"
#include "autoteamman.h"
#include "fateringmanager.h"
#include "mapresman.h"

extern abase::timer     g_timer;

struct gnpc;
struct gmatter;
struct gplayer;
struct battle_ground_param;
struct country_battle_param;
struct trick_battle_param;
struct mnfaction_battle_param;

class gplayer_imp;
#include "objmanager.h"

class public_quest_manager;
class ForceGlobalDataMan;
class GlobalController;

namespace GNET
{
	struct faction_fortress_data;
	struct faction_fortress_data2;
}

namespace GMSV
{
	struct CBConfig;
}

struct world_pos
{
	int tag;
	A3DVECTOR pos;
};

struct world_limit
{
	bool nofly;
	bool nothrow;
	bool clearap;
	bool allowroot;
	bool savepoint;
	bool nomount;
	bool gmfree;
	bool noduel;
	bool nobind;
	bool ctrlid_open_instance;
	bool anti_cheat;
	bool no_market;
	bool height_limit;
	bool common_data_notify;
	bool lowjump;
	bool no_multi_exp;
	bool profit_time;	//战斗且打怪时生效
	bool profit_time2;	//任何时候都生效
	bool online_award;
	bool nothrow_anyway;	//禁止任何方式的物品掉落(玩家主动丢弃、红名死亡掉落、受技能攻击掉落、死亡必定掉落物品等),谨慎使用
	bool can_reenter;
	bool noauto_resurrect;
	bool need_visa;
    bool noauto_genhp;
    bool noauto_genmp;
	bool permit_fix_position_transmit;//允许定位传送
    bool nocash_resurrect;
};


struct world_param
{
	bool double_exp;	//当打开双倍经验开关时，实际经验调整比例由world_manager::_double_exp_factor决定
	bool forbid_trade;
	bool forbid_faction;
	bool forbid_mail;
	bool forbid_auction;
	bool double_money;
	bool double_drop;
	bool double_sp;
	bool forbid_cash_trade;
	bool pve_mode;		//是否PVE模式
	bool anti_wallow;	//是否防沉迷
	bool anti_cheat;
	bool korea_shop;
	bool southamerican_shop;
	bool random_shop_limit; // 随机商城限制
};

struct world_config		//一些来自element的配置信息
{
	int profit_time;	//玩家战斗收益时间
};

struct world_flags
{
	bool mafia_pvp_flag : 1;
	bool nonpenalty_pvp_flag : 1;
};

//双倍打卡时间控制
struct rest_time_ctrl
{
	struct 
	{
		int min;
		int hour;
		int day; 		//day in week
	}clear_time;
	int first_rest_time;		//sec
	int rest_period;		//rest period, hour
	int rest_time_unit;		//sec
	int max_rest_time;		//sec
};

struct group_pos
{
	int group;
	A3DVECTOR pos;
	group_pos(int g = 0,A3DVECTOR p = A3DVECTOR(0,0,0)) : group(g),pos(p) {}
};

enum
{
	WORLD_TYPE_BIG_WORLD,
	WORLD_TYPE_INSTANCE,
	WORLD_TYPE_FACTION,
	WORLD_TYPE_BATTLEGROUND,
	WORLD_TYPE_COUNTRYBATTLE,
	WORLD_TYPE_COUNTRYTERRITORY,
	WORLD_TYPE_MOBILESERVER,
	WORLD_TYPE_PARALLEL_WORLD,
	WORLD_TYPE_TRICKBATTLE,
	WORLD_TYPE_MNFACTION,
};

struct MSG;
struct gplayer;
class world;
class gplayer_imp;
class world_manager;
class world_message_handler
{
public:
	world_message_handler() {}
	virtual ~world_message_handler(){}
	virtual int RecvExternMessage(int msg_tag,const MSG & msg) = 0;
	virtual int HandleMessage(world * pPlane,const MSG& msg);
};

class world_manager
{
protected:
	static MapResManager _mapres;
	static itemdataman _dataman;
	static path_manager _pathman;
	static int _world_index;
	static XID _server_xid;
	static world_manager * _manager_instance;
	static size_t _player_max_count;
	static size_t _npc_max_count;
	static size_t _matter_max_count;
	static int _world_tag;	//世界的tag， 表示是哪个位面，用于区别多个大世界和副本
	static world_limit _world_limit;
	static int _region_file_tag;		//区域文件的标签
	static int _precinct_file_tag;		//回城文件的标签
	static world_pos _save_point;		//可能存在的存盘点
	static world_pos _kickout_point;	//可能存在的踢出点
	static float _max_mob_sight_range;	//区域内最大的视野
	static rest_time_ctrl _rest_ctrl;	//双倍打卡时间的参数控制
	static world_param _world_param;
	static ai_trigger::manager _trigger_man;
	static int _npc_idle_heartbeat;
	static netgame::mall _player_mall;
	static netgame::mall _player_mall2;
	static netgame::mall _player_mall3;
	static netgame::touchshop _touch_shop;
	static int _lua_data_version;
	static abase::hash_map<int, int> _expire_items;
	static abase::hash_map<int, int> _noputin_usertrash_items;	//不可放入帐号仓库物品
	static float _height_limit;
	static abase::hash_map<int, int> _region_collision_table;	//npc/dyn/mine区域序号与collision_id对应表
	static float _double_exp_factor;
	static public_quest_manager _public_quest_man;
	static ForceGlobalDataMan _force_global_data_man;
	static title_manager	_title_man;
	static history_manager	_history_man;
	static UniqueDataClient _unique_man;
	static std::set<int> _consumption_items_shopping; // 购买时计入消费值的物品列表
	static abase::hash_map<int, int> _consumption_items_binding; // 天人合一时计入消费值的物品列表
	static abase::hash_map<int, int> _consumption_items_destroying; // 消耗时计入消费值的物品列表
	static unsigned int _world_team_uid;
	static int _world_team_uid_lock;
	static GlobalController _global_controller;
	static abase::hash_map<int, group_pos> _central_server_birth_pos_map;	//中央服务器上玩家的出生坐标(tag = 142)
	static autoteam_man _autoteam_man;
	static fatering_manager _fatering_man;
	static world_config _world_config;
	static world_flags _world_flags;
    static abase::hash_map< int, abase::vector<int> > _region_waypoint_map; //区域内的传送点
	static bool is_solo_tower_challenge_instance;                           //是否为单人爬塔副本

protected:
//公开的管理器
	struct Insertor
	{
		static int push_back(abase::vector<gnpc*> & list, gnpc * pNPC) 
		{ 
#ifdef __TEST_PERFORMANCE__		
			list.push_back(pNPC);
			return 0;
#else
			if(pNPC->idle_timer > 0)
			{
				list.push_back(pNPC);
				return 0;
			}
			else
			{
				if((--(pNPC->idle_timer_count)) <= 0)
				{
					pNPC->idle_timer_count = pNPC->npc_idle_heartbeat?pNPC->npc_idle_heartbeat:_npc_idle_heartbeat;
					list.push_back(pNPC);
				}
				return 1;
			}
#endif
		}
		template <typename T>
		static int push_back(abase::vector<T*> &list, T *obj)
		{
			list.push_back(obj);
			return 0;
		}
	};

	obj_manager<gnpc   ,TICK_PER_SEC      ,Insertor> w_npc_man;
	obj_manager<gmatter,TICK_PER_SEC * MATTER_HEARTBEAT_SEC,Insertor> w_matter_man;
	obj_manager<gplayer,TICK_PER_SEC      ,Insertor> w_player_man;

	abase::hash_map<int, A3DVECTOR> w_service_npc_list;
	abase::hash_map<int, A3DVECTOR> w_normal_mobs_list;

	typedef npc_template::npc_statement::__st_ent TRANSMIT_ENTRY;
	typedef std::multimap<int, TRANSMIT_ENTRY> TRANSMIT_MAP;
	TRANSMIT_MAP w_transmit_map;
	int w_transmit_lock;
public:
	struct player_cid
	{
		int cid[3];
		player_cid()
		{
			cid[0] = cid[1] = cid[2] = -1;
		}
		bool Init(const char * str);
	};

public:
	static MapResManager & GetMapRes(){ return _mapres; }
	static itemdataman & GetDataMan() { return _dataman;}
	static path_manager& GetPathMan() { return _pathman;}
	static int GetWorldIndex() { return _world_index;}
	static const XID & GetServerID() { return _server_xid;}
	static void  SetWorldIndex(int world_index);
	static bool InitQuestSystem(const char * path,const char *path2);
	static bool InitNPCTemplate();
	static world_manager * GetInstance() { return _manager_instance;}
	static size_t GetMaxPlayerCount() { return _player_max_count;}
	static size_t GetMaxNPCCount() { return _npc_max_count;}
	static size_t GetMaxMatterCount() { return _matter_max_count;}
	static int GetWorldTag() { return _world_tag;}
	static const world_limit & GetWorldLimit() { return _world_limit;}
	static int GetRegionTag(){ return _region_file_tag;};
	static int GetPrecinctTag(){ return _precinct_file_tag;};
	static world_param & GetWorldParam() { return _world_param;}
	static unsigned int GetWorldTeamUID() { 
		spin_autolock keeper(&_world_team_uid_lock);
		return _world_team_uid++;
		}

	static abase::vector<int> _instance_tag_list;	//包含了所有副本的tag 不包括公共副本
	static bool InitTagList();
	static bool InitWorldLimit(const char * servername);
	static bool InitExpireItems();
	static bool InitNoPutInUserTrashItems();
	static bool InitWallowParam();
	static bool InitCentralServerBirthPos();
	static bool InitWorldConfig();
	static const world_pos & GetSavePoint() { return _save_point;}
	static const world_pos & GetKickoutPoint() { return _kickout_point;}
	static float GetMaxMobSightRange() { return _max_mob_sight_range;}
	static void CalcRestTime(int & last_time, int & rest_pool_cur, int &rest_pool_cap,int mafia_id,
				 int & mafia_rest_cap, int & mafia_last_time);
	static ai_trigger::manager & GetTriggerMan() { return _trigger_man;}
	static bool IsRareItem(int item_id);
	static netgame::mall & GetPlayerMall() { return _player_mall;}
	static netgame::mall & GetPlayerMall2() { return _player_mall2;}
	static netgame::mall & GetPlayerMall3() { return _player_mall3;}
	static netgame::touchshop &GetTouchShop() { return _touch_shop;}
	static void SetCashItem(size_t id);
	static void TestCashItemGenerated(size_t id,int count);
	static void SetDeathDropItem(int item_id);
	static bool IsDeathDropItem(int item_id);
	static int GetLuaVersion() {return _lua_data_version;}
	static bool IsExpireItem(int id);
	static bool IsNoPutInUserTrashItem(int id) { return _noputin_usertrash_items.find(id) != _noputin_usertrash_items.end();}
	static float GetHeightLimit() { return _height_limit;}
	static int GetRegionCollisionId(int region_idx){ 
		abase::hash_map<int, int>::iterator it = _region_collision_table.find(region_idx);
		return it == _region_collision_table.end() ? 0 : it->second;
	}
	static void MapRegionCollisionId(int region_idx, int collision_id){
		_region_collision_table[region_idx] = collision_id;
	}
	static float GetDoubleExpFactor() { return _double_exp_factor; }
	static void SetDoubleExpFactor(float f) { _double_exp_factor = f; }
	static public_quest_manager & GetPublicQuestMan() { return _public_quest_man; }
	static ForceGlobalDataMan & GetForceGlobalDataMan() { return _force_global_data_man; }
	static title_manager& 	GetTitleMan() { return _title_man; }
	static history_manager& GetHistoryMan() { return _history_man; }
	static UniqueDataClient& GetUniqueDataMan() { return _unique_man; }
	static bool IsMallConsumptionItemShopping(int id);
	static bool GetMallConsumptionValueBinding(int id, int& value);
	static bool GetMallConsumptionValueDestroying(int id, int& value);
	static GlobalController & GetGlobalController(){ return _global_controller; }
	static A3DVECTOR GetCentralServerBrithPos(int zoneid, int& groupid)
	{
		abase::hash_map<int, group_pos>::iterator it = _central_server_birth_pos_map.find(zoneid);
		if(it != _central_server_birth_pos_map.end())
		{
			groupid = it->second.group;
			return it->second.pos;
		}
		groupid = 0;
		return A3DVECTOR(0,0,0);
	}
	static autoteam_man& GetAutoTeamMan() { return _autoteam_man; }
	static fatering_manager& GetFateRingMan() { return _fatering_man;}
	static world_config & GetWorldConfig(){ return _world_config; }
	static world_flags & GetWorldFlag() { return _world_flags; } 	
	static bool GetIsSoloTowerChallengeInstance() {return is_solo_tower_challenge_instance;}
    
    /**
     * 初始化每个区域内的传送点
     * 注意，这个函数必须在读取完所有传送点信息，读取完所有区域信息后调用
     * 传送点信息是全局的，在FirstStepInit函数内读取
     * 区域信息每个地图各有不同，在global_mananger或instance_manager的Init函数内读取
     */
    static bool InitRegionWayPointMap();
    static const abase::hash_map< int, abase::vector<int> >& GetRegionWaypoints() { return _region_waypoint_map; }

protected:
	static void LoadRareItemList(const char * file);
	static bool LoadLuaVersion(const char * file);
	static bool LoadMallConsumptionConfig();

	static void SetWorldTag(int tag ) {_world_tag = tag;}
	int InitBase(const char * section);
protected:
	typedef abase::hashtab<int,int,abase::_hash_function,abase::fast_alloc<> >	query_map;//用户的查询表
	world_message_handler * _message_handler;
	MsgIOManager 		_ioman;		//和其他逻辑服务器的连接器
	int			_psvr_lock;	//所有玩家的锁
	query_map 		_psvr_map; 	//所有玩家所在的服务器的列表
public:
	static int FirstStepInit();
	bool GetServiceNPCPos(int id, A3DVECTOR & pos)
	{
		abase::hash_map<int, A3DVECTOR>::iterator it = w_service_npc_list.find(id);
		if(it == w_service_npc_list.end())
		{
			return false;
		}
		else
		{
			pos = it->second;
			return true;
		}
	}

	bool GetMobNPCPos(int id, A3DVECTOR & pos)
	{
		abase::hash_map<int, A3DVECTOR>::iterator it = w_normal_mobs_list.find(id);
		if(it == w_normal_mobs_list.end())
		{
			return false;
		}
		else
		{
			pos = it->second;
			return true;
		}
	}

	bool GetTransmitEntry(int src_wp, int dst_wp, TRANSMIT_ENTRY& entry)
	{
		spin_autolock keeper(w_transmit_lock);
		TRANSMIT_MAP::iterator it = w_transmit_map.lower_bound(src_wp);
		if(it != w_transmit_map.end() && it->first == src_wp)
		{
			for(TRANSMIT_MAP::iterator ie=w_transmit_map.upper_bound(src_wp); it!=ie; ++it)
			{
				if(it->second.target_waypoint == dst_wp)
				{
					entry = it->second;	
					return true;
				}			
			}		
		}
		return false;
	}

	void InsertTransmitEntry(int src_wp, const TRANSMIT_ENTRY& entry)
	{
		spin_autolock keeper(w_transmit_lock);
		TRANSMIT_MAP::iterator it = w_transmit_map.lower_bound(src_wp);
		if(it != w_transmit_map.end() && it->first == src_wp)
		{
			for(TRANSMIT_MAP::iterator ie=w_transmit_map.upper_bound(src_wp); it!=ie; ++it)
				if(it->second.target_waypoint == entry.target_waypoint) return;
		}
		__PRINTF("Insert Transmit Path: %d -> %d\n",src_wp,entry.target_waypoint);
		w_transmit_map.insert(std::make_pair(src_wp,entry));
	}
	
	world_manager():w_transmit_lock(0),_message_handler(0),_psvr_lock(0),_psvr_map(MAX_PLAYER_IN_WORLD), _serverdata_init(false), _load_task(NULL), _write_timer(10000)
	{}

	virtual ~world_manager() 
	{
		if(_message_handler)
		{
			delete _message_handler;
			_message_handler = NULL;
		}
		if(_load_task)
		{
			_load_task->RemoveTimer();
			delete _load_task;
			_load_task = NULL;
		}
	}

	int PlaneSwitch(gplayer_imp * pImp,const A3DVECTOR & pos,int tag,const instance_key & key, size_t fee);
public:
	inline static bool ProfitMap()
	{
		return _world_limit.profit_time || _world_limit.profit_time2;
	}

	inline static bool ProfitTimeLimit()
	{
		return _world_limit.profit_time;
	}

	inline static bool ProfitTimeLimit2()
	{
		return _world_limit.profit_time2;
	}

	inline static bool NeedVisa()
	{
		return _world_limit.need_visa;
	}

	inline static bool AntiWallow()
	{
		return _world_param.anti_wallow;
	}

	inline static bool AntiCheat()
	{
		return _world_param.anti_cheat;
	}
	
	inline gnpc 	*AllocNPC()
	{
		return w_npc_man.Alloc();
	}
	inline void 	FreeNPC(gnpc* pNPC)
	{
		w_npc_man.Free(pNPC);
	}

	inline gmatter *AllocMatter()
	{
		return w_matter_man.Alloc();
	}
	inline void 	FreeMatter(gmatter *pMatter)
	{
		return w_matter_man.Free(pMatter);
	}

	inline gplayer *AllocPlayer()
	{
		return w_player_man.Alloc();
	}
	inline void 	FreePlayer(gplayer * pPlayer)
	{
		return w_player_man.Free(pPlayer);
	}

	inline int GetPlayerAlloced()
	{
		return w_player_man.GetAllocedCount();
	}
	inline void InsertPlayerToMan(gplayer *pPlayer) 
	{ 
		w_player_man.Insert(pPlayer);
	}
	inline void RemovePlayerToMan(gplayer *pPlayer) 
	{ 	
		w_player_man.Remove(pPlayer);
	}

	inline void InsertNPCToMan(gnpc * pNPC)
	{
		w_npc_man.Insert(pNPC);
	}

	inline void RemoveNPCFromMan(gnpc * pNPC)
	{
		w_npc_man.Remove(pNPC);
	}

	inline void InsertMatterToMan(gmatter * pMatter)
	{
		w_matter_man.Insert(pMatter);
	}

	inline void RemoveMatterFromMan(gmatter * pMatter)
	{
		w_matter_man.Remove(pMatter);
	}

	inline gmatter * GetMatterPool() const  { return w_matter_man.GetPool();}
	inline gplayer*  GetPlayerPool() const   { return w_player_man.GetPool();}
	inline gnpc* 	 GetNPCPool() const   { return w_npc_man.GetPool();}
	inline gmatter * GetMatterByIndex(size_t index) const  { return w_matter_man.GetByIndex(index);}
	inline gplayer*  GetPlayerByIndex(size_t index) const   {return w_player_man.GetByIndex(index);}
	inline gnpc* 	 GetNPCByIndex(size_t index) const   { return w_npc_man.GetByIndex(index);}
	inline size_t GetPlayerIndex(gplayer *pPlayer)  const  { return w_player_man.GetIndex(pPlayer);}
	inline size_t GetMatterIndex(gmatter *pMatter)  const  { return w_matter_man.GetIndex(pMatter);}
	inline size_t GetNPCIndex(gnpc *pNPC)  const  { return w_npc_man.GetIndex(pNPC);}
	inline bool CheckPlayerDropCondition()
	{
		size_t cap = w_matter_man.GetCapacity();
		size_t count = w_matter_man.GetAllocedCount();
		return (cap - count) >= (cap >> 2);
	}

public:	
	void Init();
	virtual int GetWorldType() = 0;
	virtual void Heartbeat();	//一秒20次的心跳
	virtual void RestartProcess() = 0;
	virtual void ShutDown();
	virtual bool InitNetIO(const char * servername) = 0;
	virtual void GetPlayerCid(player_cid & cid) = 0;
	virtual bool CompareInsKey(const instance_key & key, const instance_hash_key & hkey) { return true;}
	virtual bool CheckKeyInvalid(const instance_key & key,const instance_hash_key &cur_key) {return true;}
	virtual int GetPlayerLimitPerInstance(){ return 0; }
	virtual int GetEffectPlayerPerInstance(){ return 0; }
	virtual bool IsUniqueWorld() = 0;
	virtual world_message_handler * CreateMessageHandler() = 0;
	virtual bool TriggerSpawn(int sid) {return false;}
	virtual bool ClearSpawn(int sid) {return false;}
	virtual int GetInstanceReenterTimeout(world* plane) { return 0;}
public:
//世界管理
	virtual gplayer* FindPlayer(int uid, int & world_index) = 0;
	virtual int GetServerNear(const A3DVECTOR & pos) const = 0;	//返回在大世界中符合的服务器(附近)
	virtual int GetServerGlobal(const A3DVECTOR & pos) const = 0;	//返回在大世界中符合的服务器
	virtual world * GetWorldByIndex(size_t index) = 0;
	virtual size_t GetWorldCapacity() = 0;
	virtual int GetOnlineUserNumber() { return 0;}
	virtual void HandleSwitchRequest(int lid,int uid, int sid,int source, const instance_key & key) = 0;
	virtual void PlayerLeaveThisWorld(int plane_index, int useid){}
	virtual void GetLogoutPos(gplayer_imp * pImp, int & world_tag ,A3DVECTOR & pos) = 0;
	virtual void SwitchServerCancel(int link_id,int user_id, int localsid) = 0;
	virtual void UserLogin(int cs_index,int cs_sid,int uid,const void * auth_data, size_t auth_size, bool isshielduser, char flag) = 0;
	virtual void SetFilterWhenLogin(gplayer_imp * pImp, instance_key * ikey = NULL){}
	virtual void PlayerAfterSwitch(gplayer_imp * pImp);
	virtual int PlayerSwitchWorld(gplayer * pPlayer, const instance_hash_key & key){ return -1; }
	virtual void PlayerQueryWorld(gplayer * pPlayer){}
	virtual bool IsBattleWorld(){ return false; }
	virtual bool CreateBattleGround(const battle_ground_param &) { return false;} //收到要求创建战场的协议
	virtual bool GetTownPosition(gplayer_imp *pImp, const A3DVECTOR &opos, A3DVECTOR &pos, int & tag);
	virtual void RecordTownPos(const A3DVECTOR &pos,int faction) {}
	virtual void RecordMob(int type, int tid, const A3DVECTOR &pos,int faction,int cnt); //type: 0标志性建筑 1:npc,2:mob
	virtual void OnDeliveryConnected() {}
	virtual int OnMobDeath(world * pPlane, int faction,  int tid) { return 0;}
	virtual int OnMineGathered(world * pPlane, int mine_tid, gplayer* pPlayer){ return 0; }
	virtual int GenerateFlag(){ return 0; }
	virtual bool IsReachFlagGoal(bool offense, const A3DVECTOR& pos){ return false; }
	virtual bool CanBeGathered(int player_faction, int mine_tid){ return true; }
	virtual int CanBeGathered(int player_faction, int mine_tid, world *pPlane,const XID &player_xid){ return 0; }
	virtual bool IsMobileWorld(){ return false; }
	virtual bool CreateTrickBattle(const trick_battle_param &) { return false;}
	virtual instance_hash_key  GetLogoutInstanceKey(gplayer_imp *pImp) const;
public:
	//帮派pvp相关
	virtual void OnMafiaPvPStatusNotice(int status,std::vector<int> &ctrl_list) {};		
	virtual void OnMafiaPvPElementRequest(unsigned int version) {};		
public:
	//帮派基地相关
	virtual bool IsFactionWorld(){ return false; }
	virtual bool FactionLogin(const instance_hash_key &hkey,const GNET::faction_fortress_data * data, const GNET::faction_fortress_data2 * data2) { return false; }
	virtual bool NotifyFactionData(GNET::faction_fortress_data2 * data2){ return false; }

public:
	//国战相关
	virtual bool IsCountryTerritoryWorld(){ return false; }
	virtual bool IsCountryBattleWorld(){ return false; }
	virtual void NotifyCountryBattleConfig(GMSV::CBConfig * config){}
	virtual bool CreateCountryBattle(const country_battle_param &) { return false;}
	virtual void DestroyCountryBattle(int battleid) { }

public:
	//跨服帮战相关
	virtual int CreateMNFactionBattle(const mnfaction_battle_param &) {return 0;}
	
public:
	//serverdata相关
	virtual bool IsWeddingServer(){return false;}
	virtual bool WeddingCheckOngoing(int groom, int bride, int scene){return false;}
	virtual bool WeddingCheckOngoing(int id){return false;}
	virtual bool WeddingSendBookingList(int id, int cs_index, int cs_sid){return false;}
	virtual bool WeddingCheckBook(int start_time, int end_time, int scene, int card_year, int card_month, int card_day){return false;}
	virtual bool WeddingTryBook(int start_time, int end_time, int groom, int bride, int scene){return false;}
	virtual bool WeddingCheckCancelBook(int start_time, int end_time, int groom, int bride, int scene){return false;}
	virtual bool WeddingTryCancelBook(int start_time, int end_time, int groom, int bride, int scene){return false;}
	virtual bool WeddingDBLoad(archive & ar){return false;}
	virtual bool WeddingDBSave(archive & ar){return false;}

	virtual bool HasDpsRank(){return false;}
	virtual bool DpsRankUpdateRankInfo(int roleid, int level, int cls, int dps, int dph){ return false;}
	virtual bool DpsRankSendRank(int link_id, int roleid, int link_sid, unsigned char rank_mask){ return false;}
	virtual bool DpsRankDBLoad(archive & ar){return false;}
	virtual bool DpsRankDBSave(archive & ar){return false;}
	
	class serverdata_load_task : public abase::timer_task, public abase::ASmallObject
	{
		world_manager * pMan;
	public:
		serverdata_load_task(world_manager * man) : pMan(man)
		{
			SetTimer(g_timer,60*TICK_PER_SEC,0);		
			__PRINTF("timer %p %d\n",this,_timer_index);
		}
		virtual void OnTimer(int index,int rtimes)
		{
			pMan->LoadServerData();
		}
	};
	bool _serverdata_init;
	serverdata_load_task * _load_task;
	int _write_timer;
	int GetWriteTimer(){ return _write_timer; }
	void LoadServerData();
	void AutoSaveServerData();
	void ShutDownSaveServerData();
public:
//消息处理
	virtual int  SendRemotePlayerMsg(int uid, const MSG & msg) = 0;
	virtual void SendRemoteMessage(int id, const MSG & msg) = 0;
	virtual int  BroadcastSvrMessage(const rect & rt,const MSG & message,float extend_size) = 0;
	virtual void PostMessage(world * plane, const MSG & msg) = 0;
	virtual void PostMessage(world * plane, const MSG & msg,int latancy) = 0;
	virtual void PostMessage(world * plane, const XID * first, const XID * last, const MSG & msg) = 0;
	virtual void PostPlayerMessage(world * plane, int * player_list, size_t count, const MSG & msg) = 0;
	virtual void PostMultiMessage(world * plane,abase::vector<gobject*,abase::fast_alloc<> > &list, const MSG & msg) = 0;

public:
	inline MsgIOManager & GetIOMan() { return _ioman;}
	int ReceiveMessage(int msg_tag,const MSG & message);
	inline int HandleWorldMessage(world * pPlane, const MSG & message)
	{
		return _message_handler->HandleMessage(pPlane,message);
	}
	inline int GetPlayerServerIdx(int uid)
	{
		int rst;
		{
			mutex_spinlock(&_psvr_lock);
			int * pTmp = _psvr_map.nGet(uid);
			rst = pTmp?*pTmp:-1;
			mutex_spinunlock(&_psvr_lock);
		}
		return rst;
	}

	inline void SetPlayerServerIdx(int uid, int svr)
	{
		spin_autolock alock(_psvr_lock);
		_psvr_map.find_or_insert(uid,svr) = svr;
	}

	inline void RemovePlayerServerIdx(int uid)
	{
		spin_autolock alock(_psvr_lock);
		_psvr_map.erase(uid);
	}

	inline void BatchSetPlayerServer(int * pdata,size_t size, size_t step)
	{
		spin_autolock alock(_psvr_lock);
		for(size_t i = 0; i < size; i ++,pdata =(int*)(((char*)pdata)+step) )
		{
			_psvr_map.find_or_insert(*pdata,*(pdata+1)) = *(pdata+1);
		}
	}
};

#endif

