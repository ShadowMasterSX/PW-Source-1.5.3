#ifndef __ONLINEGAME_GS_WORLD_H__
#define __ONLINEGAME_GS_WORLD_H__

#include <map>
#include <hashtab.h>
#include <amemory.h>
#include <timer.h>
#include <threadpool.h>
#include <common/types.h>
#include <set>
#include <glog.h>

#include "gimp.h"
#include "grid.h"
#include "msgqueue.h"
#include "io/msgio.h"
#include "terrain.h"
#include "template/itemdataman.h"
#include "worldmanager.h"
#include "npcgenerator.h"
#include "commondata.h"
#include "usermsg.h"
#include "staticmap.h"

namespace NPCMoveMap
{
	class CMap;
}
class trace_manager2;

class MsgDispatcher;
class GSvrPool;
class CNPCGenMan;

class  world_data_ctrl
{
//这个类用于世界的数据控制，会添加一系列的虚函数
	public:
	virtual ~world_data_ctrl() {}
	virtual world_data_ctrl * Clone() = 0;
	virtual void Reset() = 0;
	virtual void Tick(world * pPlane) = 0;
	virtual void BattleFactionSay(int faction, const void * buf, size_t size, char emote_id, const void * aux_data, size_t dsize, int self_id, int self_level){}
	virtual void BattleSay(const void * buf, size_t size){};
	virtual void OnSetCommonValue(int key, int value){}
	virtual void OnTriggerSpawn(int controller_id){}
	virtual void OnClearSpawn(int controller_id){}
	virtual void OnServerShutDown(){}
	//帮派基地相关
	virtual int GetFactionId(){ return 0; }
	virtual bool LevelUp(){ return false; }
	virtual bool SetTechPoint(size_t tech_index){ return false; }
	virtual bool ResetTechPoint(world * pPlane, size_t tech_index){ return false; }
	virtual bool Construct(world * pPlane, int id, int accelerate){ return false; }
	virtual bool HandInMaterial(int id, size_t count){ return false; }
	virtual bool HandInContrib(int contrib){ return false; }
	virtual bool MaterialExchange(size_t src_index,size_t dst_index,int material){ return false; }
	virtual bool Dismantle(world * pPlane, int id){ return false; }
	virtual bool GetInfo(int roleid, int cs_index, int cs_sid){ return false; }
	//国战战场相关
	virtual void UpdatePersonalScore(bool offense, int roleid, int combat_time, int attend_time, int dmg_output, int dmg_output_weighted, int dmg_endure, int dmg_output_npc){}
	virtual void OnPlayerDeath(gplayer * pPlayer, const XID & killer, int player_soulpower, const A3DVECTOR & pos){}
	virtual bool PickUpFlag(gplayer * pPlayer){ return false;}
	virtual bool HandInFlag(gplayer * pPlayer){ return false;}
	virtual void UpdateFlagCarrier(int roleid, const A3DVECTOR & pos){}
	virtual void OnTowerDestroyed(world * pPlane, bool offense, int tid){}
	virtual void OccupyStrongHold(int mine_tid, gplayer* pPlayer){};
	virtual bool GetStrongholdNearby(bool offense, const A3DVECTOR &opos, A3DVECTOR &pos, int & tag){return false;}
	virtual bool GetPersonalScore(bool offense, int roleid, int& combat_time, int& attend_time, int& kill_count, int& death_count, int& country_kill_count,int& country_death_count){ return false; }
	virtual bool GetCountryBattleInfo(int & attacker_count, int & defender_count){ return false; }
	virtual void GetStongholdState(int roleid, int cs_index, int cs_sid){}
	virtual bool GetLiveShowResult(int roleid, int cs_index, int cs_sid, world* pPlane){ return false; }

	//战车战场相关
	virtual void UpdatePersonalScore(int roleid, int kill, int death, int score){}
	virtual void AddChariot(int type, int chariot) {}
	virtual void DelChariot(int type, int chariot) {}
	virtual void GetChariots(int type, abase::hash_map<int, int> & chariot_map) {}
};

class world
{
	typedef abase::hashtab<int,int,abase::_hash_function,abase::fast_alloc<> >	query_map;//用户的查询表
	extern_object_manager w_ext_man;	//保存其他服务器上对象信息的管理器

	grid 	w_grid;
	int 	w_index;	//世界区域的索引
	query_map w_player_map;	//玩家的查找表
	query_map w_npc_map;	//在本地的外部npc查询表
	int	w_pmap_lock;	//player 的userid -> index id的map锁
	int	w_nmap_lock;	//npc 外部npc-->本地连接的查询表
	int 	w_message_counter[GM_MSG_MAX];	//消息数目的计数表
	npc_generator 	w_npc_gen; //怪物生成管理器
	
	map_generator * w_map_generator;
	CTerrain * w_terrain;
	NPCMoveMap::CMap * w_movemap;
	trace_manager2 * w_traceman;
public:
	struct off_node_t{
		int idx_off;
		int x_off;
		int z_off;
		off_node_t(grid & grid,int offset_x,int offset_y):x_off(offset_x),z_off(offset_y)
		{
			idx_off = offset_y*grid.reg_column + offset_x;
		}
		bool operator==(const off_node_t & rhs) const 
		{
			return rhs.idx_off == idx_off;
		}
	};
	abase::vector<off_node_t,abase::fast_alloc<> > w_off_list;
	int	w_near_vision;		//暂时没有使用，近点的索引
	int	w_far_vision;		//最远的距离所涵盖的范围
	int	w_true_vision;		//完全可视的范围,暂时没有使用
	int	w_plane_index;		//在世界中的位面索引
	int	w_player_count;		//本世界中玩家的数目
	float	w_vision;		//视野范围，和far_vision对应的距离
	world_manager * w_world_man;
	int 	w_obsolete;		//由manager使用这个变量
	int 	w_life_time;	//world剩余生存时间,-1代表无限,0代表已经过期,由manager使用这个变量
	instance_hash_key w_ins_key;	//副本使用的instance_key ， 由manager使用这个变量
	int	w_activestate;		//激活状态 0:未激活 1:激活 2:冷却  由manager来控制
	int	w_index_in_man;		//在管理器中的索引，为MsgQueue2和manager所使用
	int	w_create_timestamp;	//创建的时间戳，由manager使用 
	int	w_destroy_timestamp;	//删除的时间戳，由manager使用 不是所有的副本这个都有效的
	int	w_ins_kick;		//是否踢出instance key不符合的玩家
	int	w_battle_result;	//给战场用的 战场结果
	int	w_offense_goal;		//战场攻方目标
	int	w_offense_cur_score;	//战场攻方得分
	int	w_defence_goal;		//战场守方目标
	int	w_defence_cur_score;	//战场守方得分
	int 	w_end_timestamp; 	//战场结束时间，到此时间时，所有玩家都将被自动踢出 
					//只有battle_result有效后,此值才会被使用

	int 	w_player_node_lock;	//玩家锁数据，用于更新世界中的玩家列表时的锁
	cs_user_map w_player_node_list;	//玩家数据
	common_data w_common_data;	//本世界通用数据
	abase::vector<char>	w_collision_flags;	//npc dyn mine 碰撞是否激活的标志
	int w_scene_service_npcs_lock;
	abase::static_multimap<int, int> w_scene_service_npcs;	//全场景服务npc，使用服务时无距离限制
public:
	void SetCommonValue(int key,int value, bool notify_world_ctrl = true);
	int GetCommonValue(int key);
	int ModifyCommonValue(int key, int offset);
	void AddPlayerNode(gplayer * pPlayer);
	void DelPlayerNode(gplayer * pPlayer);
	void SyncPlayerWorldGen(gplayer* pPlayer);
	void AddSceneServiceNpc(int tid, int id);
	void DelSceneServiceNpc(int tid, int id);
	void GetSceneServiceNpc(abase::vector<int> & list);
private:	
	void CommonDataNotify(int key, int value);
public:
//初始化函数
	world();
	~world();
	bool 	Init(int world_index);
	void 	InitManager(world_manager * man) { w_world_man = man;}
	bool 	InitNPCGenerator(CNPCGenMan & npcgen);
	bool    InitNPCGenerator(CNPCGenMan & ctrldata, npcgen_data_list& npcgen_list);
	bool	TriggerSpawn(int condition, bool notify_world_ctrl = true);
	bool 	ClearSpawn(int condition, bool notify_world_ctrl = true);
	void 	InitTimerTick();
	bool 	CreateGrid(int row,int column,float step,float startX,float startY);
	int	BuildSliceMask(float near,float far);			//创建距离查询所需要的mask

	void 	DuplicateWorld(world * dest) const;	//复制世界，除了NPC生成器 .....
	
	inline world_manager * GetWorldManager() { return w_world_man;}
	//分配一个NPC数据，返回一个上了锁了NPC结构
	inline gnpc 	*AllocNPC() 
	{ 
		gnpc *pNPC = w_world_man->AllocNPC(); 
		if(pNPC) pNPC->plane = this;
		return pNPC;
	}
	inline void 	FreeNPC(gnpc* pNPC) 
	{ 
		ASSERT(pNPC->plane == this);
		pNPC->plane = NULL;
		return w_world_man->FreeNPC(pNPC); 
	}

	inline bool CheckPlayerDropCondition()
	{
		return w_world_man->CheckPlayerDropCondition();
	}

	//设置此npc为外部npc
	inline void 	SetNPCExtern(gnpc * pNPC)
	{
		spin_autolock alock(w_nmap_lock); 
		w_npc_map.put(pNPC->ID.id,GetNPCIndex(pNPC));
	}

	inline int 	GetNPCExternID(int id)
	{
		spin_autolock alock(w_nmap_lock);
		query_map::pair_type p = w_npc_map.get(id);
		if(!p.second) return -1;
		return *p.first;
	}

	inline void 	EraseExternNPC(int id)
	{
		spin_autolock alock(w_nmap_lock);
		w_npc_map.erase(id);
	}


	//分配一个Matter数据，返回一个上了锁的Matter结构
	inline gmatter *AllocMatter() 
	{ 
		gmatter * pMatter = w_world_man->AllocMatter(); 
		if(pMatter) pMatter->plane = this;
		return pMatter;
	}
	inline void 	FreeMatter(gmatter *pMatter) 
	{ 
		ASSERT(pMatter->plane == this);
		pMatter->plane = NULL;
		return w_world_man->FreeMatter(pMatter); 
	}

	//	增加/删除玩家和对象的函数
	//分配一个玩家数据，并返回一个锁定的玩家结构
	inline gplayer *AllocPlayer() 
	{ 
		gplayer * pPlayer = w_world_man->AllocPlayer(); 
		if(pPlayer) 
		{
			pPlayer->plane = this;
			interlocked_increment(&w_player_count);
		}
		return pPlayer;
	}

	inline void 	FreePlayer(gplayer * pPlayer)
	{
		w_world_man->PlayerLeaveThisWorld(w_plane_index,pPlayer->ID.id);
		ASSERT(pPlayer->plane == this);
		interlocked_decrement(&w_player_count);
		pPlayer->plane = NULL;
		return w_world_man->FreePlayer(pPlayer);
	}

	//AttachPlayer,DetachPlayer 用于玩家在本gs进行换线操作时使用
	inline void AttachPlayer(gplayer * pPlayer)
	{
		ASSERT(pPlayer->plane == NULL);
		pPlayer->plane = this;
		interlocked_increment(&w_player_count);
	}

	inline void DetachPlayer(gplayer * pPlayer)
	{
		w_world_man->PlayerLeaveThisWorld(w_plane_index,pPlayer->ID.id);
		ASSERT(pPlayer->plane == this);
		interlocked_decrement(&w_player_count);
		pPlayer->plane = NULL;
	}

	inline int GetPlayerInWorld() 
	{
		return w_player_count; 
	}

	inline void InsertPlayerToMan(gplayer *pPlayer) 
	{ 
		w_world_man->InsertPlayerToMan(pPlayer);
	}
	inline void RemovePlayerToMan(gplayer *pPlayer) 
	{ 	
		w_world_man->RemovePlayerToMan(pPlayer);
	}

	int InsertPlayer(gplayer *);		//根据位置，插入一个对象到世界中，返回插入的区域索引号
	int InsertNPC(gnpc*);			//根据位置，插入一个对象到世界中，返回插入的区域索引号
	int InsertMatter(gmatter *);		//根据位置，插入一个对象到世界中，返回插入的区域索引号
	
	void RemovePlayer(gplayer *pPlayer); 	//从世界中移出一个对象，不free
	void RemoveNPC(gnpc *pNPC);		//从世界中移出一个对象，不free
	void RemoveMatter(gmatter *pMatter);	//从世界中移出一个对象，不free

	//从管理器中移出NPC，用于不在场景中的npc
	inline void RemoveNPCFromMan(gnpc * pNPC)
	{
		w_world_man->RemoveNPCFromMan(pNPC);
	}

	inline void RemoveMatterFromMan(gmatter * pMatter)
	{
		w_world_man->RemoveMatterFromMan(pMatter);
	}

	bool IsPlayerExist(int player_id);	//查询玩家是否在世界中（或者在其他服务器中）

	void Release();

public:
//	取得属性的inline函数
	inline gmatter * GetMatterByIndex(size_t index) const  { return w_world_man->GetMatterByIndex(index);}
	inline gplayer*  GetPlayerByIndex(size_t index) const   {return w_world_man->GetPlayerByIndex(index);}
	inline gnpc* 	 GetNPCByIndex(size_t index) const   { return w_world_man->GetNPCByIndex(index);}
	inline size_t GetPlayerIndex(gplayer *pPlayer)  const  { return w_world_man->GetPlayerIndex(pPlayer);}
	inline size_t GetMatterIndex(gmatter *pMatter)  const  { return w_world_man->GetMatterIndex(pMatter);}
	inline size_t GetNPCIndex(gnpc *pNPC)  const  { return w_world_man->GetNPCIndex(pNPC);}
	inline grid&	 GetGrid() { return w_grid;}
	inline extern_object_manager & GetExtObjMan() { return  w_ext_man;}
	inline const rect & GetLocalWorld() { return w_grid.local_region;}
	inline bool PosInWorld(const A3DVECTOR & pos)
	{
		return w_grid.IsLocal(pos.x,pos.z);
	}

	inline bool MapPlayer(int uid,int index) { 
		spin_autolock alock(w_pmap_lock); 
		return w_player_map.put(uid,index);
	}
	
	inline int UnmapPlayer(int uid) {
		spin_autolock alock(w_pmap_lock);
		return w_player_map.erase(uid);
	}
	
	inline int FindPlayer(int uid) {
		spin_autolock alock(w_pmap_lock);
		query_map::pair_type p = w_player_map.get(uid);
		if(!p.second) return -1;
		return *p.first;
	}

	inline gplayer * GetPlayerByID(int uid)
	{
		int index = FindPlayer(uid);
		if(index < 0) return NULL;
		return GetPlayerByIndex(index);
	}

	inline void ExtManRefresh(int id, const A3DVECTOR &pos, const extern_object_manager::object_appear & obj)
	{
		w_ext_man.Refresh(id,pos,obj);
	}

	inline void ExtManRefreshHP(int id, const A3DVECTOR &pos, int hp)
	{
		w_ext_man.RefreshHP(id,pos,hp);
	}

	inline void ExtManRemoveObject(int id)
	{
		w_ext_man.RemoveObject(id);
	}
	
	inline int GetPlayerCount()
	{
		spin_autolock alock(w_pmap_lock); 
		return w_player_map.size();
	}

	//
	enum
	{
		QUERY_OBJECT_STATE_ACTIVE = 0x01,
		QUERY_OBJECT_STATE_ZOMBIE = 0x02,
		QUERY_OBJECT_STATE_DISCONNECT = 0x04,
	};
	struct object_info
	{
		int state;
		A3DVECTOR pos;
		float body_size;
		int race;
		int faction;
		int level;
		int hp;
		int mp;
		int max_hp;
		int invisible_degree;
		int anti_invisible_degree;
		unsigned int object_state;		//仅player npc有效
		unsigned int object_state2;		//仅player npc有效
		int mafia_id;
	};

	bool QueryObject(const XID & id,object_info & info);	//查询一个其他对象的状态

public:
	int RebuildMapRes();
	inline float GetHeightAt(float x, float z)
	{
		if(w_terrain) return w_terrain->GetHeightAt(x, z);
		return w_world_man->GetMapRes().GetUniqueTerrain()->GetHeightAt(x, z);	
	}

	inline NPCMoveMap::CMap * GetMoveMap()
	{
		if(w_movemap) return w_movemap;
		return w_world_man->GetMapRes().GetUniqueMoveMap();
	}

	inline trace_manager2 * GetTraceMan()
	{
		if(w_traceman) return w_traceman;
		return w_world_man->GetMapRes().GetUniqueTraceMan();
	}
	
	inline const map_generator* GetMapGen() const { return w_map_generator; }

	inline int GetBlockID(float x, float z) const {	return w_map_generator ? w_map_generator->GetBlockID(x,z) : 0;}
	inline int GetRoomIndex(float x, float z) const { return w_map_generator ? w_map_generator->GetRoomIndex(x,z) : 0;}
	inline bool GetTownPosition(gplayer_imp *pImp, const A3DVECTOR &opos, A3DVECTOR &pos, int & tag) const { return w_map_generator ? w_map_generator->GetTownPosition(pImp,opos,pos,tag) : false; }
	inline bool SetIncomingPlayerPos(gplayer * pPlayer, const A3DVECTOR & origin_pos) const { return w_map_generator ? w_map_generator->SetIncomingPlayerPos(pPlayer,origin_pos) : false; }	
private:
	//处理消息的内部函数
	gobject * locate_object_from_msg(const MSG & msg);		//根据消息定位对象
	void 	try_dispatch_extern_msg(const MSG & msg);
public:
	void RunTick();		//由manager控制调用或者内部自动调用
	void ResetWorld();	//重置世界，只有副本才会调用这个（这个操作会重生所有的怪物等）
	void DumpMessageCount();

	void SetWorldCtrl(world_data_ctrl * ctrl);
	world_data_ctrl * w_ctrl;
private:
	friend class MsgQueue;
public:

//发送消息的函数
	/*
	 *	指定目标的发送消息，已经完成了对象确定，因此不再需要再次进行查找
	 */
	int DispatchMessage(gobject * obj, const MSG & message);

	/*
	 *	延时发送一条消息发送一条消息
	 *	delay_tick以50ms为单位
	 *	在config.h里定义了最大的delay_tick MAX_MESSAGE_DELAY
	 *	PostMessageQueue 是用SendLazyMessage(message,0)来实现的
	 *	如果delay_tick过大，那么何时发送该消息是未定义的
	 */
	inline void PostLazyMessage(const MSG & message, size_t delay_tick)
	{
		w_world_man->PostMessage(this,message,delay_tick);
	}

	/*
	 *	发送一条消息，可能会延迟发送，也可能会较快的发送
	 *	在每次SendMessage之后，都会检测是否有消息等待发送
	 */
	inline void PostLazyMessage(const MSG & message)
	{
		w_world_man->PostMessage(this,message);
	}

	/*
	 *	给一堆player发送消息
	 */
	inline void SendPlayerMessage(size_t count, int * player_list, const MSG & msg)
	{
		w_world_man->PostPlayerMessage(this,player_list,count,msg);
	}

	/*
	 *	给一堆ID发送消息
	 */
	void SendMessage(const XID * first, const XID * last, const MSG & msg)
	{
		w_world_man->PostMessage(this,first,last,msg);
	}

	/*
	 *	发送一条消息到远程服务器
	 *	分发操作由远程服务器完成
	 */
	void SendRemoteMessage(int id,const MSG & msg)
	{
		w_world_man->SendRemoteMessage(id, msg);
	}

	/*
	
	*  获取player_list的大小，也就是被广播的人数
	*/
	int GetSpherePlayerListSize(const A3DVECTOR& target,float fRadius);
	
	/*
	 *	将一条消息发送到和给出的rect参数相交的远程服务器上
	 */
	int BroadcastSvrMessage(const rect & rt,const MSG & message,float extend_size)
	{
		return w_world_man->BroadcastSvrMessage(rt,message,extend_size);
	}

	/*
	 *	广播消息，按照距离将包转发给周围的所有对?
	 *	msg.target 的类型决定了收到者，msg.target的ID必须为-1
	 *	mask用于过滤消息接收对象
	 *	在座这个广播时，会自动判断是否要转发到其他的服务器上
	 *	msg.source不会收到这个广播消息
	 */
	int BroadcastMessage(const MSG & message,float fRadius,int mask); 		

	/*
	 *	广播盒形消息，按照距离将包转发给周围的所有对象
	 *	msg.target 的类型决定了收到者，msg.target的ID必须为-1
	 *	msg.source不会收到这个广播消息
	 */
	int BroadcastLocalBoxMessage(const MSG & message,const rect & rt);

	/*
	 *	广播球形消息，按照距离将包转发给周围的所有对象
	 *	msg.target 的类型决定了收到者，msg.target的ID必须为-1
	 *	mask用于过滤消息接收对象(已经被取消)
	 *	在座这个广播时，会自动判断是否要转发到其他的服务器上
	 *	msg.source不会收到这个广播消息
	 */
	int BroadcastSphereMessage(const MSG & message,const A3DVECTOR & target, float fRadius);

	/*
	 *	广播柱形消息，按照距离将包转发给周围的所有对象,该对象必须在柱中
	 *	柱的起始坐标在message中，终点是target
	 *	msg.target 的类型决定了收到者，msg.target的ID必须为-1
	 *	mask用于过滤消息接收对象
	 *	在座这个广播时，会自动判断是否要转发到其他的服务器上
	 *	msg.source不会收到这个广播消息
	 */
	int BroadcastCylinderMessage(const MSG & message,const A3DVECTOR & target, float fRadius);

	/*
	 *	广播椎形消息，在椎中的对象会收到这个消息
	 *	圆锥的圆心即为消息的发出点
	 *	msg.target 的类型决定了收到者，msg.target的ID必须为-1
	 *	mask用于过滤消息接收对象
	 *	在座这个广播时，会自动判断是否要转发到其他的服务器上
	 *	msg.source不会收到这个广播消息
	 */
	int BroadcastTaperMessage(const MSG & message,const A3DVECTOR & target,float fRadius,float cos_halfangle);


	/*
	 *	同BroadcastMessage，唯一的区别是只在本地做相应的转发操作
	 *	BroadcastMessage的本地发送是通过调用本函数完成的
	 */
	int BroadcastLocalMessage(const MSG & message,float fRadius,int mask);

	/*
	 *	广播球形消息，按照距离将包转发给周围的所有对象
	 *	msg.target 的类型决定了收到者，msg.target的ID必须为-1
	 *	mask用于过滤消息接收对象
	 *	在座这个广播时，会自动判断是否要转发到其他的服务器上
	 *	msg.source不会收到这个广播消息
	 */
	int BroadcastLocalSphereMessage(const MSG & message,const A3DVECTOR & target, float fRadius);

	/*
	 *	广播柱形消息，按照距离将包转发给周围的所有对象,该对象必须在柱中
	 *	柱的起始坐标在message中，终点是target
	 *	msg.target 的类型决定了收到者，msg.target的ID必须为-1
	 *	mask用于过滤消息接收对象
	 *	在座这个广播时，会自动判断是否要转发到其他的服务器上
	 *	msg.source不会收到这个广播消息
	 */
	int BroadcastLocalCylinderMessage(const MSG & message,const A3DVECTOR & target, float fRadius);

	/*
	 *	广播椎形消息，在椎中的对象会收到这个消息
	 *	圆锥的圆心即为消息的发出点
	 *	msg.target 的类型决定了收到者，msg.target的ID必须为-1
	 *	mask用于过滤消息接收对象
	 *	在座这个广播时，会自动判断是否要转发到其他的服务器上
	 *	msg.source不会收到这个广播消息
	 */
	int BroadcastLocalTaperMessage(const MSG & message,const A3DVECTOR & target,float fRadius,float cos_halfangle);

	/*
	 *	分发消息，这个操作不应该由用户调用
	 *	分发消息的操作在SendMessage里被自动调用
	 */
	int DispatchMessage(const MSG & message);

	/*
	 *	计算某个位置应该属于哪个服务器 ,返回找到的第一个服务器
	 *	返回-1表示没有找到
	 */
	int GetSvrNear(const A3DVECTOR & pos) const
	{
		return w_world_man->GetServerNear(pos);
	}

	/*
	 *	计算某个位置应该属于大地图中的那个服务器，副本服务器不在此列
	 *	返回-1表示没有找到
	 */
	int GetGlobalServer(const A3DVECTOR & pos) const
	{
		return w_world_man->GetServerGlobal(pos);
	}
	
	/*
	 *	检查一个坐标加值后是否仍然在正确范围内,内部使用的函数.
	 */
	inline static bool check_index(const grid * g,int x,int z, const world::off_node_t &node)
	{
		int nx = x + node.x_off;
		if(nx < 0 || nx >= g->reg_column) return false;
		int nz = z + node.z_off;
		if(nz < 0 || nz >= g->reg_row) return false;
		return true;
	}

	void BattleFactionSay(int faction, const void * msg, size_t size, char emote_id=0, const void * aux_data=NULL, size_t dsize=0, int self_id=0, int self_level=0);
	void BattleSay(const void * msg, size_t size);
	void InstanceSay(const void * msg, size_t size, bool middle, const void* data=NULL, size_t dsize = 0);

public:
//模板函数接口
	/*
	 *	当一个对象在两个格子间移动,判断该对象离开了哪些格子的视野,会调用相应的enter和leave函数对象
	 */
	template <typename ENTER,typename LEAVE>
	inline void MoveBetweenSlice(slice * pPiece, slice * pNewPiece,ENTER enter,LEAVE leave)
	{
		int i;
		grid * pGrid = &GetGrid();
		int ox,oy,nx,ny;
		pGrid->GetSlicePos(pPiece,ox,oy);
		pGrid->GetSlicePos(pNewPiece,nx,ny);
		float vision = w_vision + pGrid->slice_step - 1e-3;
		float dis = pNewPiece->Distance(pPiece);
		if(dis > vision)
		{
			//本格的无法看见 所以要进行离开本格的操作,后面的循环并没有判断本格
			leave(pPiece);
			enter(pNewPiece);
			if(dis > vision*2)
			{
				for(i = 0; i < w_far_vision; i ++)
				{
					const world::off_node_t &node = w_off_list[i];
					slice * pTmpPiece = pPiece + node.idx_off;
					leave(pTmpPiece);
				}

				for(i = 0; i < w_far_vision; i ++)
				{
					const world::off_node_t &node = w_off_list[i];
					slice * pTmpPiece = pNewPiece + node.idx_off;
					enter(pTmpPiece);
				}
				return ;
			}
		}

		for(i = 0; i < w_far_vision; i ++)
		{
			const world::off_node_t &node = w_off_list[i];
			if(check_index(pGrid,ox,oy,node)) 
			{
				slice * pTmpPiece = pPiece + node.idx_off;
				if(pTmpPiece->Distance(pNewPiece) > vision && pTmpPiece->IsInWorld())
				{
					leave(pTmpPiece);
					//离开了这个slice
				}
			}

			if(check_index(pGrid,nx,ny,node))
			{
				slice * pTmpPiece = pNewPiece + node.idx_off;
				if(pTmpPiece->Distance(pPiece) > vision && pTmpPiece->IsInWorld())
				{
					enter(pTmpPiece);
				}
			}
		}
	}

	/*
	 *	扫描附近所有的小格子,按照预定的范围来扫描,这里不进行格子是否在本服务器的判断 
	 */
	template <typename FUNC >
	inline void ForEachSlice(slice * pStart, FUNC func,int vlevel = 0)
	{
		int total = vlevel?w_near_vision:w_far_vision;
		int slice_x,slice_z;
		GetGrid().GetSlicePos(pStart,slice_x,slice_z);
		for(int i = 0; i <total; i ++)
		{
			off_node_t &node = w_off_list[i]; 
			int nx = slice_x + node.x_off; 
			int nz = slice_z + node.z_off; 
			if(nx < 0 || nz < 0 || nx >= GetGrid().reg_column || nz >= GetGrid().reg_row) continue;
			slice * pNewPiece = pStart+ node.idx_off;
			func(i,pNewPiece);
		}
	}

	/*
	 * 按照位置和范围扫描附近所有的小格,并且依次调用相应的处理函数对象
	 * 这里会首先判断格子是否在当前服务器中,否则不会发送到func函数中
	 */
	template <typename FUNC>
	inline void ForEachSlice(const A3DVECTOR &pos, float fRadius, FUNC func)
	{
		grid * pGrid = &GetGrid();
		float fx = pos.x - pGrid->grid_region.left;
		float fz = pos.z - pGrid->grid_region.top;
		float inv_step = pGrid->inv_step;
		int ofx1 = (int)((fx - fRadius) * inv_step);
		int ofx2 = (int)((fx + fRadius) * inv_step);
		int ofz1 = (int)((fz - fRadius) * inv_step);
		int ofz2 = (int)((fz + fRadius) * inv_step);
		if(ofx1 < 0) ofx1 = 0;
		if(ofx2 >= pGrid->reg_column) ofx2 = pGrid->reg_column -1;
		if(ofz1 < 0) ofz1 = 0;
		if(ofz2 >= pGrid->reg_row) ofz2 = pGrid->reg_row - 1;

		slice * pPiece = pGrid->GetSlice(ofx1,ofz1);
		for(int i = ofz1;i <= ofz2; i ++,pPiece += pGrid->reg_column)
		{
			slice * pStart = pPiece;
			for(int j = ofx1; j <= ofx2; j ++, pStart++)
			{
				if(pStart->IsInWorld()) func(pStart,pos);
			}
		}
	}

	template <typename FUNC>
	inline void ForEachSlice(const A3DVECTOR &pos, const rect & rt, FUNC func)
	{
		grid * pGrid = &GetGrid();
		
		float inv_step = pGrid->inv_step;
		int ofx1 = (int)((rt.left   - pGrid->grid_region.left) * inv_step);
		int ofx2 = (int)((rt.right  - pGrid->grid_region.left) * inv_step);
		int ofz1 = (int)((rt.top    - pGrid->grid_region.top ) * inv_step);
		int ofz2 = (int)((rt.bottom - pGrid->grid_region.top ) * inv_step);
		if(ofx1 < 0) ofx1 = 0;
		if(ofx2 >= pGrid->reg_column) ofx2 = pGrid->reg_column -1;
		if(ofz1 < 0) ofz1 = 0;
		if(ofz2 >= pGrid->reg_row) ofz2 = pGrid->reg_row - 1;

		slice * pPiece = pGrid->GetSlice(ofx1,ofz1);
		for(int i = ofz1;i <= ofz2; i ++,pPiece += pGrid->reg_column)
		{
			slice * pStart = pPiece;
			for(int j = ofx1; j <= ofx2; j ++, pStart++)
			{
				if(pStart->IsInWorld()) func(pStart,pos);
			}
		}
	}

	template <typename FUNC>
	inline void ForEachSlice(const A3DVECTOR &pos1, const A3DVECTOR & pos2, FUNC func)
	{
		rect rt(pos1,pos2);
		ForEachSlice(pos1,rt,func);
	}

	template <int foo>
	inline static void InspirePieceNPC(slice * pPiece,int tick)
	{
		int timestamp = pPiece->idle_timestamp;
		if(tick - timestamp < 40)
		{
			return;
		}
		pPiece->Lock();
		if(tick - pPiece->idle_timestamp < 40)	//由piece决定每两秒设置一次
		{
			pPiece->Unlock();
			return;
		}
		pPiece->idle_timestamp = tick;
		gnpc * pNPC = (gnpc*)(pPiece->npc_list);
		while(pNPC)
		{
			pNPC->idle_timer = NPC_IDLE_TIMER;
			pNPC = (gnpc*)(pNPC->pNext);
		}
		pPiece->Unlock();
	}
	/*
	 *	扫描附近所有的小格子,按照预定的范围来扫描,这里不进行格子是否在本服务器的判断 
	 */
	template <int foo>
	inline void InspireNPC(slice * pStart, int vlevel = 0)
	{
		int total = vlevel?w_near_vision:w_far_vision;
		int slice_x,slice_z;
		GetGrid().GetSlicePos(pStart,slice_x,slice_z);
		int tick = g_timer.get_tick();
		InspirePieceNPC<0>(pStart,tick);

		for(int i = 0; i <total; i ++)
		{
			off_node_t &node = w_off_list[i]; 
			int nx = slice_x + node.x_off; 
			int nz = slice_z + node.z_off; 
			if(nx < 0 || nz < 0 || nx >= GetGrid().reg_column || nz >= GetGrid().reg_row) continue;
			slice * pNewPiece = pStart+ node.idx_off;
			InspirePieceNPC<0>(pNewPiece,tick);
		}
	}

private:
	void CheckGSvrPoolUpdate();						//检查当前在线的游戏服务器列表
	void ConnectGSvr(int index, const char * ipaddr, const char * unixaddr);	//连接另外一台服务器 

	/*
	 * 这个类是定期刷新游戏服务器的类,它将作为线程池的一个任务来进行
	 * 生成它的地方是相关的定时器函数
	 */

	int _message_handle_count;				//记录当前message处理的嵌套 

private:
	//消息发送器的函数，这两个函数只有消息发送器连接上和断开时调用
};

template <typename WRAPPER>
inline int WrapObject(WRAPPER & wrapper,controller * ctrl, gobject_imp * imp, dispatcher * runner)
{
	ctrl->SaveInstance(wrapper);
	imp->SaveInstance(wrapper);
	runner->SaveInstance(wrapper);
	return 0;
}

template <typename WRAPPER,typename OBJECT>
inline int RestoreObject(WRAPPER & wrapper,OBJECT *obj,world * pPlane)
{
	controller * ctrl =NULL;
	gobject_imp * imp = NULL;
	dispatcher * runner = NULL;

	ctrl = substance::DynamicCast<controller>(substance::LoadInstance(wrapper));
	if(ctrl) imp = substance::DynamicCast<gobject_imp>(substance::LoadInstance(wrapper));
	if(imp) runner = substance::DynamicCast<dispatcher>(substance::LoadInstance(wrapper));
	if(!ctrl || !runner ||!imp) 
	{
		delete imp;
		delete ctrl;
		return -1;
	}
	obj->imp = imp;
	imp->_runner = runner;
	imp->_commander = ctrl;
	imp->Init(pPlane,obj);
	ctrl->Init(imp);
	runner->init(imp);
	//以后还需要调用ReInit
	return 0;
}

extern abase::timer	g_timer;

#endif

