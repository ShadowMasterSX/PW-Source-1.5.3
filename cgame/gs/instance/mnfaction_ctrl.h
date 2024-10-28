#ifndef __ONLINEGAME_GS_MNFACTION_CTRL_H__
#define __ONLINEGAME_GS_MNFACTION_CTRL_H__

#include "../world.h"
#include "../worldmanager.h"
#include "../player_imp.h"

#define NEUTRAL_DEGREE_PERCENT 0.4
#define MAX_ATTACKER_PLAYER_COUNT 60//攻击帮派最大进入人数
#define MAX_DEFENDER_PLAYER_COUNT 60//同上
#define RESURRECT_POS_INTERVAL    30  //(秒)
#define SEND_ALL_POS_INFO_TIME    5  //(S)5秒把所有玩家坐标发送一遍
#define FIRST_TRANSMIT_PLAYER_INTERVAL 60 * 5
#define RESOURCE_TOWER_GATHER_TIME 60*5
#define NEUTUAL_TOWER_GATHER_TIME  60*2
#define TRANSMIT_POS_GATHER_TIME   60*2

struct MNFactionStateInfo;

struct mnfaction_player_pos_info
{
	int _roleid;
	A3DVECTOR _player_pos;
	mnfaction_player_pos_info(){}
	mnfaction_player_pos_info(int roleid, A3DVECTOR player_pos):_roleid(roleid),_player_pos(player_pos)
	{
	}
};

class mnfaction_ctrl : public world_data_ctrl
{
public:
	enum
	{
		STATE_ATTACKER = 0,//攻方占领
		STATE_ATTACKER_GATHER,//攻方正在采集
		STATE_NEUTRAL,//中立
		STATE_DEFENDER_GATHER,
		STATE_DEFENDER,
	};

	enum
	{
		BATTLE_RESULT_NULL             = 0x0,
		BATTLE_RESULT_WINNER_OFFENSE   = 0x01,
		BATTLE_RESULT_WINNER_DEFENCE   = 0x02,
		BATTLE_RESULT_WINNER_TIE       = 0x04,//平局
		BATTLE_RESULT_WINNER_RAND      = 0x08,//平局，战场无初始归属，随机生成获胜者
	};
	
	struct _transmit_point_pos
	{
		int _index;
		int _matter_type;//采集资源点的物品tid
		A3DVECTOR _transmit_pos;
		int _state;//枚举中的五种状态，初始为中立状态
		int _controller_id[5];
		int _time_out;
		int _gather_roleid;//采集传送点的玩家roleid
		int _gather_faction;
		int _owner_faction;//传送点所属帮派，初始为0
		_transmit_point_pos(int index, int matter_type, float transmit_pos[], int controller_id[]):_index(index),_matter_type(matter_type),_transmit_pos(transmit_pos[0], transmit_pos[1], transmit_pos[2]),_state(STATE_NEUTRAL),_time_out(0),_gather_roleid(0),_gather_faction(0),_owner_faction(0)
		{
			_controller_id[0] = controller_id[0];
			_controller_id[1] = controller_id[1];
			_controller_id[2] = controller_id[2];
			_controller_id[3] = controller_id[3];
			_controller_id[4] = controller_id[4];
		}
		_transmit_point_pos(){}
	};

	struct _tower//资源塔数据结构
	{
		int _index;
		int _controller_id[3];//资源塔共三种状态，正常、被转化、损毁
		int _guard_controller_id;//资源塔上的守卫控制器
		int _matter_type;//采集资源塔的物品tid
		int _state;//枚举中的五种状态，有初始状态 STATE_ATTACKER or STATE_DEFENDER
		int _time_out;//处于转化状态时用于计时
		int _gather_roleid;
		int _gather_faction;
		int _owner_faction;
		_tower(int index,int controller_id[], int guard_controller_id, int matter_type,int state,int owner_faction):_index(index),_guard_controller_id(guard_controller_id),_matter_type(matter_type),_state(state),_time_out(-1),_gather_roleid(0),_gather_faction(0),_owner_faction(owner_faction)
		{
			_controller_id[0] = controller_id[0];
			_controller_id[1] = controller_id[1];
			_controller_id[2] = controller_id[2];
		}
		_tower(){}
	};

	struct _switch_tower//中立防御塔数据结构
	{
		int _index;
		int _controller_id[5];//中立防御塔共五种状态    攻方占领、攻方转换、中立、守方转换、守方占领
		int _matter_type;
		int _state;//初始状态为STATE_NEUTRAL
		int _time_out;
		int _gather_roleid;
		int _gather_faction;
		int _owner_faction;	
		_switch_tower(int index,int controller_id[], int matter_type):_index(index),_matter_type(matter_type),_state(STATE_NEUTRAL),_time_out(0),_gather_roleid(0),_gather_faction(0),_owner_faction(0)
		{
			_controller_id[0] = controller_id[0];
			_controller_id[1] = controller_id[1];
			_controller_id[2] = controller_id[2];
			_controller_id[3] = controller_id[3];
			_controller_id[4] = controller_id[4];
		}
		_switch_tower(){}
	};
	
	//Delivery传送的数据
	unsigned char _domain_type;
	int _domain_id;
	int64_t _owner_faction_id;
	int64_t _attacker_faction_id;
	int64_t _defender_faction_id;
	int _end_timestamp;
	int _start_timestamp;

	//
	int _attend_attacker_player_count;//攻击帮派已进入人数
	int _attend_defender_player_count;//同上
	int _attacker_killed_player_count;//攻方击杀
	int _defender_killed_player_count;
	A3DVECTOR _resurrect_pos[2];//复活点
	float _resurrect_pos_range;//复活点半径
	A3DVECTOR _resource_point_pos[2];//可争夺的资源点坐标
	float _resource_point_range;//可争夺的资源点范围，圆形区域
	int _degree_total;//总刻度
	int _degree_per_person_second;//每秒/人增加的刻度值
	int _gain_resource_point_per_second;//当资源点被占领时每次增加的资源点
	int _gain_resource_point_interval;//增加资源点的间隔
	int _reduce_resource_point_on_death;//每名玩家死亡后扣除的资源点

	int _small_boss_death_reduce_point;//小boss死亡扣除的资源点
	int _attacker_small_boss_controller_id;//攻方小boss控制器ID
	int _defender_small_boss_controller_id;//同上
	
	int _small_boss_appear_time;//小boss出现距战场结束的时间(s)
	int _debuff_appear_time;//DEBUFF出现距战场结束的时间(s)
	float _debuff_init_ratio;//DEBUFF起始易伤效果值
	float _debuff_enhance_ratio_per_minute;//DEBUFF每分钟增强值

	abase::vector<int> _cur_degree;
	int _degree_attacker;
	int _degree_defender;
	//    攻方增加资源           中立           守方增加资源
	//  |_________________|___________________|_______________|
	//            _degree_attacker    _degree_defender  
	//例如：总资源点为1000 则_degree_attacker = 300, _degree_defender = 700
	// ( 0 - 299 ) ( 300 - 700 ) ( 701 - 1000 )
	A3DVECTOR _attacker_incoming_pos;
	A3DVECTOR _defender_incoming_pos;
	A3DVECTOR _attacker_transmit_pos;
	A3DVECTOR _defender_transmit_pos;
	int _attacker_transmit_pos_target_waypoint;
	int _defender_transmit_pos_target_waypoint;
	int _max_resource_point;
	int _attacker_resource_point;//攻方的资源点，降为0时战斗失败，守方胜利
	int _defender_resource_point;//同上
	int _resource_tower_destroy_reduce_point;

	typedef std::map<int, _tower> RESOURSE_TOWER;
	
	RESOURSE_TOWER _attacker_resourse_tower;//攻方资源塔 mine_type -> _tower
	RESOURSE_TOWER _defender_resourse_tower;//守方资源塔
	
	std::set<int> _attacker_resourse_tower_mine_types;
	std::set<int> _defender_resourse_tower_mine_types;
	
	typedef std::map<int, _switch_tower> SWITCH_TOWER;
	
	SWITCH_TOWER _neutral_tower;//中立防御塔,初始化owner_faction=0
	std::set<int> _neutral_tower_mine_types;
	
	typedef std::map<int, _transmit_point_pos> TRANSMIT_POS;
	TRANSMIT_POS _transmit_pos_map;//传送点 mine_type -> transmit_pos

	abase::vector<int> _transmit_index_to_mine_type;//根据传送点index找到采集传送点的matter_type

	int64_t _winner_faction_id;
	
	cs_user_map _attend_attacker_player_node_list;
	cs_user_map _attend_defender_player_node_list;

	//typedef abase::hash_map<int, mnfaction_player_pos_info, abase::_hash_function,abase::fast_alloc<> > player_pos_info_map;
	typedef std::map<int, mnfaction_player_pos_info> player_pos_info_map;
	player_pos_info_map _attacker_player_pos_info_map;
	player_pos_info_map _defender_player_pos_info_map;

	//std::list<mnfaction_player_pos_info *> _attacker_player_pos_send_list;
	//std::list<mnfaction_player_pos_info *> _defender_player_pos_send_list;

	int _recored_attend_attacker_player_count;
	int _recored_attend_defender_player_count;
	
	int _lock;
	int _battle_result;
	int _tick_counter;
	int _battle_end_timer;
	int _second_resurrect;
	int _second_resource_point;
	int _player_node_list_lock;
	int _player_pos_info_lock;
	int _sync_pos_tick;
public:
	mnfaction_ctrl():_attend_attacker_player_count(0),_attend_defender_player_count(0),_attacker_killed_player_count(0),_defender_killed_player_count(0),_resource_point_range(0),_max_resource_point(0),_attacker_resource_point(0),_defender_resource_point(0),_winner_faction_id(0),_lock(0),_battle_result(BATTLE_RESULT_NULL),_tick_counter(0),_battle_end_timer(0),_second_resurrect(0),_second_resource_point(0),_player_node_list_lock(0),_player_pos_info_lock(0)/*,_tbuf(MAX_ATTACKER_PLAYER_COUNT / SEND_ALL_POS_INFO_TIME * sizeof(mnfaction_player_pos_info) + 10)*/
	{
		_domain_id = -1;
		_is_small_boss_appear = false;
		_is_small_boss_attacker_death = false;
		_is_small_boss_defender_death = false;
		_is_first_transmit_player     = true;
		_recored_attend_attacker_player_count = 0;
		_recored_attend_defender_player_count = 0;
		_sync_pos_tick = 0;
		_resurrect_interval = FIRST_TRANSMIT_PLAYER_INTERVAL; 
	}

	virtual ~mnfaction_ctrl(){}
	virtual world_data_ctrl * Clone()
	{
		return new mnfaction_ctrl(*this);
	}
	virtual void Reset();
	void Init(world * pPlane);
	virtual void Tick(world * pPlane);
	virtual void OnPlayerDeath(gplayer * pPlayer, const XID & killer, int player_soulpower,const A3DVECTOR& pos);
	void CheckBattleResult(world * pPlane);
	bool AddPlayerNodeList(cs_user_map &player_node_list, gplayer *pPlayer);
	bool DelPlayerNodeList(cs_user_map &player_node_list, gplayer * pPlayer);
	bool AddAttacker(gplayer *pPlayer);
	bool AddDefender(gplayer *pPlayer);
	bool DelAttacker(gplayer *pPlayer);
	bool DelDefender(gplayer *pPlayer);

	virtual bool GetMnFactionInfo(int &attacker_resource_point, int &defender_resource_point, int &attend_attacker_player_count, int &attend_defender_player_count, abase::vector<int> &cur_degree, abase::vector<MNFactionStateInfo> &attacker_resouse_tower_state, abase::vector<MNFactionStateInfo> &defender_resouse_tower_state, abase::vector<MNFactionStateInfo> &switch_tower_state, abase::vector< MNFactionStateInfo> &transmit_pos_state, int &attacker_killed_player_count, int &defender_killed_player_count, bool &is_small_boss_appear);
	bool GetIsFirstTransmit()
	{
		return _is_first_transmit_player;
	}
	int PlayerSetBattleDelayStartime();
public:
	int CanBeGathered(int player_faction, int mine_tid, const XID &player_xid);
	int OnMineGathered(int mine_tid, gplayer* pPlayer);
	int GatherResourseTower(RESOURSE_TOWER & resouce_tower_map,int mine_tid, gplayer *pPlayer);
	int OnBossDeath(int faction, world * pPlane);
	int OnSmallBossDeath(int faction, world * pPlane);
	int PlayerTransmitInMNFaction(gplayer_imp * pImp, int index, A3DVECTOR &pos);
	void GetDebuffInfo(int &debuff_appear_time, float &debuff_init_ratio, float &debuff_enhance_ratio_per_minute);
	void GetTownPosition(gplayer_imp *pImp, A3DVECTOR &resurrect_pos);
	void PlayerPosInfoSync(int roleid, int faction, A3DVECTOR &pos);
	void DelPlayerPosInfoOnLeave(gplayer *pPlayer, player_pos_info_map & pos_info_map);
	virtual void BattleFactionSay(int faction, const void * buf, size_t size, char emote_id, const void * aux_data, size_t dsize, int self_id, int self_level);
private:
	bool _is_small_boss_appear;
	bool _is_small_boss_attacker_death;
	bool _is_small_boss_defender_death;
	bool _is_first_transmit_player;
	int  _resurrect_interval;
	packet_wrapper _tbuf;
	
	void BattleEnd(world * pPlane);
	void SendDSBattleEnd();

	int GatherResouseTower(RESOURSE_TOWER & resouce_tower_map, int mine_tid, gplayer* pPlayer);
	int GatherNeutralTower(SWITCH_TOWER & neutral_tower_map, int mine_tid, gplayer* pPlayer);
	int GatherTransmitPos(TRANSMIT_POS & transmit_pos_map, int mine_tid, gplayer* pPlayer); 

	void CheckResourceTowerState(RESOURSE_TOWER &resouce_tower_map, world * pPlane);

	void CheckSwitchTowerState(SWITCH_TOWER & neutral_tower_map, world * pPlane);
	void CheckTransmitState(TRANSMIT_POS & transmit_pos_map, world * pPlane);
	void CheckResourcePointState(world *pPlane);
	void CalResoursePoint();
	void ResourceTowerDestroy(int faction, world *pPlane);
	void TransmitResurrectPosPlayer(world * pPlane);
	void SendClientPlayerPosInfo();
	void RecordLog();
};

struct visible_collector
{
	world * _plane;
	float _squared_radius;
	int &_attacker_count;
	int &_defender_count;
	visible_collector(world * plane,int &attacker_count, int &defender_count,float radius)
		:_plane(plane),_squared_radius(radius*radius),_attacker_count(attacker_count),_defender_count(defender_count){}

	inline void operator()(slice *pPiece,const A3DVECTOR & pos)
	{
		if(!pPiece->player_list) return;
		pPiece->Lock();
		gplayer * pPlayer = (gplayer *)pPiece->player_list;
		while(pPlayer)
		{
			A3DVECTOR player_pos = pPlayer->pos;
			player_pos.y = 0;
			if(pos.squared_distance(player_pos) < _squared_radius && !pPlayer->IsInvisible() && !pPlayer->IsZombie())
			{
				if(pPlayer->IsBattleOffense())
				{
					++_attacker_count;
				}
				else
				{
					++_defender_count;
				}
			}
			pPlayer = (gplayer *)pPlayer->pNext;
		}
		pPiece->Unlock();
	}
};
#endif
