#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <math.h>
#include <algorithm>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "config.h"
#include "world.h"
#include "io/msgio.h"
#include "clstab.h"
#include "npc.h"
#include "matter.h"
#include "player_imp.h"

#include "usermsg.h"
#include "template/itemdataman.h"
#include "task/taskman.h"
#include "../common/protocol_imp.h"
#include "../common/packetwrapper.h"

#include "global_controller.h"
#include "mapresman.h"
#include "pathfinding/pathfinding.h"
#include "../collision/traceman.h"

abase::timer 	g_timer(1000,300000);

world::world():w_player_map(10),w_npc_map(4096),w_pmap_lock(0),w_nmap_lock(0),w_ctrl(NULL)
{
	w_index = 0;
	w_world_man = NULL;
	_message_handle_count = 0;
	memset(w_message_counter,0,sizeof(w_message_counter));
	w_map_generator = NULL;
	w_terrain = NULL;
	w_movemap = NULL;
	w_traceman = NULL;
	w_player_count = 0;
	w_obsolete = 0;
	w_life_time = -1;
	w_activestate = 0;
	w_index_in_man = -1;
	w_create_timestamp = -1;
	w_ins_kick = 1;
	w_battle_result = 0;
	w_offense_goal	= 0;
	w_offense_cur_score = 0;
	w_defence_goal = 0;
	w_defence_cur_score = 0;
	w_end_timestamp = 0;
	w_player_node_lock = 0;
	w_scene_service_npcs_lock = 0;
}

world::~world()
{
	if(w_ctrl)
	{
		delete w_ctrl;
		w_ctrl = NULL;
	}
	if(w_map_generator)
	{
		delete w_map_generator;
		w_map_generator = NULL;
	}
	if(w_terrain)
	{
		delete w_terrain;
		w_terrain = NULL;
	}
	if(w_movemap)
	{
		delete w_movemap;
		w_movemap = NULL;
	}
	if(w_traceman)
	{
		delete w_traceman;
		w_traceman = NULL;
	}
}

void 
world::SetWorldCtrl(world_data_ctrl * ctrl)
{
	ASSERT(w_ctrl == NULL);
	w_ctrl = ctrl;
}

bool
world::Init(int world_index)
{
//	w_ext_man.Init(); //这里不调用了， 因为没有用了
	w_plane_index = 0;
	w_index = world_index;
	return true;
}

void
world::RunTick()
{
	//第二套运行方式
	w_npc_gen.Run();
	//
	w_ext_man.RunTick();
	if(w_ctrl)
	{
		w_ctrl->Tick(this);
	}
}


bool    
world::InitNPCGenerator(CNPCGenMan & npcgen)
{
	if(!w_npc_gen.LoadGenData(this,npcgen,GetGrid().base_region))
	{
		return false;
	}
	__PRINTF("npc/dyn/mine collision count = %d, world_tag = %d\n",GetTraceMan()->GetElementCount(),world_manager::GetWorldTag());
	ASSERT(w_collision_flags.size() == 0);
	w_collision_flags.insert(w_collision_flags.begin(), GetTraceMan()->GetElementCount()+1, 0);
	w_npc_gen.InitIncubator(this);
	w_npc_gen.StartHeartbeat();
	if(!w_npc_gen.BeginSpawn()) return false;
	return true;
}
bool
world::InitNPCGenerator(CNPCGenMan & ctrldata, npcgen_data_list& npcgen_list)
{
    if(!npcgen_list.size())
        return false; 
	
	//加入一个默认的控制器
	w_npc_gen.InsertSpawnControl(0, 0, true,0,0,0);

    npcgen_data_list::iterator ibeg = npcgen_list.begin();
    npcgen_data_list::iterator iend = npcgen_list.end();
 
	for(; ibeg != iend; ++ibeg)
	{
		npcgen_data_node_t& gen_data = *ibeg;
		if(!gen_data.npcgen || !w_npc_gen.AddSpawnData(this, ctrldata, *gen_data.npcgen,gen_data.blockid,gen_data.offset,true,false))
			   return false;
	}
	__PRINTF("npc/dyn/mine collision count = %d, world_tag = %d\n",GetTraceMan()->GetElementCount(),world_manager::GetWorldTag());
	
	ASSERT(w_collision_flags.size() == 0);
	
	w_collision_flags.insert(w_collision_flags.begin(), GetTraceMan()->GetElementCount()+1, 0);
	w_npc_gen.InitIncubator(this);
	w_npc_gen.StartHeartbeat();
	if(!w_npc_gen.BeginSpawn()) return false;
	return true;
}

bool 
world::InitNPCGeneratorByClone(CNPCGenMan & ctrldata, npcgen_data_list& npcgen_list)
{
    if(!npcgen_list.size())
        return false; 
	
	//加入一个默认的控制器
	w_npc_gen.InsertSpawnControl(0, 0, true,0,0,0);

    npcgen_data_list::iterator ibeg = npcgen_list.begin();
    npcgen_data_list::iterator iend = npcgen_list.end();
 
	for(; ibeg != iend; ++ibeg)
	{
		npcgen_data_node_t& gen_data = *ibeg;
		if(ibeg == npcgen_list.begin())
		{
			if(!gen_data.npcgen || !w_npc_gen.AddSpawnData(this, ctrldata, *gen_data.npcgen,gen_data.blockid,gen_data.offset,true,true))
			   return false;
		}
		else
		{
			if(!gen_data.npcgen || !w_npc_gen.AddSpawnData(this, ctrldata, *gen_data.npcgen,gen_data.blockid,gen_data.offset,false,true))
			   return false;
		}
	}
	__PRINTF("npc/dyn/mine collision count = %d, world_tag = %d\n",GetTraceMan()->GetElementCount(),world_manager::GetWorldTag());
	
	ASSERT(w_collision_flags.size() == 0);
	
	w_collision_flags.insert(w_collision_flags.begin(), GetTraceMan()->GetElementCount()+1, 0);
	w_npc_gen.InitIncubator(this);
	w_npc_gen.StartHeartbeat();
	if(!w_npc_gen.BeginSpawn()) return false;
	return true;
}

bool 
world::TriggerSpawn(int condition, bool notify_world_ctrl)
{
	if(condition == 0) return false;
	if(world_manager::GetGlobalController().CheckServerForbid(SERVER_FORBID_CTRL,condition))
		return false;
	if(notify_world_ctrl && w_ctrl) w_ctrl->OnTriggerSpawn(condition);
	return w_npc_gen.TriggerSpawn(condition);
}

bool 
world::ClearSpawn(int condition, bool notify_world_ctrl)
{
	if(condition == 0) return false;
	if(notify_world_ctrl && w_ctrl) w_ctrl->OnClearSpawn(condition);
	w_npc_gen.ClearSpawn(condition);
	return true;
}

bool 
world::CreateGrid(int row,int column,float step,float sx,float sy)
{
	return w_grid.Create(row,column,step,sx,sy);
}

static void 
insert_unique(abase::vector<world::off_node_t, abase::fast_alloc<> > &list,const world::off_node_t& node)
{
	if(std::find(list.begin(),list.end(),node) == list.end())
	{
		list.push_back(node);
	}
}

int 
world::BuildSliceMask(float near,float far)
{
	if(far < near) return -1;
	w_off_list.clear();
	float inv_step = w_grid.inv_step;
	int n1 = (int)(near * inv_step); 
	int f1 = (int)(far * inv_step); 
	int tf1 = (int)((far-w_grid.slice_step) * inv_step);
	if( fabs(near - n1 * w_grid.slice_step) > 1e-3) n1 ++;
	if( fabs(far - f1 * w_grid.slice_step) > 1e-3) f1 ++;
	for(int i = 1; i <= f1; i ++)
	{
		for(int j =-i; j < i; j ++)
		{
			insert_unique(w_off_list,off_node_t(w_grid,j,-i));
			insert_unique(w_off_list,off_node_t(w_grid,i,j));
			insert_unique(w_off_list,off_node_t(w_grid,-j,i));
			insert_unique(w_off_list,off_node_t(w_grid,-i,-j));
		}
		if(n1 == i) w_near_vision = w_off_list.size();
		if(tf1 == i) w_true_vision = w_off_list.size();
	}
	w_far_vision = w_off_list.size();
	w_vision = far;
	return 0;
}
	
void 
world::RemovePlayer(gplayer *pPlayer)
{
	slice * pPiece = pPlayer->pPiece;
	ASSERT(pPiece);

	pPiece->Lock();
	pPiece->RemovePlayer(pPlayer);
	pPiece->Unlock();
}
void 
world::RemoveNPC(gnpc *pNPC)
{
	slice * pPiece = pNPC->pPiece;
	if(pPiece)
	{
		pPiece->Lock();
		pPiece->RemoveNPC(pNPC);
		pPiece->Unlock();
	}
	RemoveNPCFromMan(pNPC);
}
void 
world::RemoveMatter(gmatter *pMatter)
{
	slice * pPiece = pMatter->pPiece;
	if(pPiece)
	{
		pPiece->Lock();
		pPiece->RemoveMatter(pMatter);
		pPiece->Unlock();
	}
	RemoveMatterFromMan(pMatter);
}

int 	
world::InsertPlayer(gplayer * pPlayer)
{
	slice *pPiece  = w_grid.Locate(pPlayer->pos.x,pPlayer->pos.z);
	if(pPiece == NULL) return -1;
	pPiece->Lock();
	pPiece->InsertPlayer(pPlayer);
	pPiece->Unlock();
	return pPiece - w_grid.pTable;
}

bool 
world::IsPlayerExist(int player_id)
{
	return w_world_man->GetPlayerServerIdx(player_id) >= 0;
}

int 	
world::InsertMatter(gmatter * pMatter)
{
	slice *pPiece  = w_grid.Locate(pMatter->pos.x,pMatter->pos.z);
	if(pPiece == NULL) return -1;
	pPiece->Lock();
	pPiece->InsertMatter(pMatter);
	pPiece->Unlock();
	w_world_man->InsertMatterToMan(pMatter);
	return pPiece - w_grid.pTable;
}


int 	
world::InsertNPC(gnpc * pNPC)
{
	slice *pPiece  = w_grid.Locate(pNPC->pos.x,pNPC->pos.z);
	if(pPiece == NULL) return -1;
	pPiece->Lock();
	pPiece->InsertNPC(pNPC);
	pPiece->Unlock();
	w_world_man->InsertNPCToMan(pNPC);
	return pPiece - w_grid.pTable;
}

static inline void call_message_handler(gobject * obj, world *plane, const MSG &msg)
{
	//obj在外面lock
	int rst = 0;
#ifdef _DEBUG
	obj->cur_msg = msg.message;
#endif
	if(obj->imp) rst = obj->imp->DispatchMessage(plane,msg);
	if(!rst) 
	{ 
		ASSERT(obj->spinlock && "这里必须是上锁状态"); 
		obj->Unlock();
	}
	else
	{
		ASSERT(!obj->spinlock && "没有解开锁，可能是时序问题，但更可能是错误");
	}
}

void
world::try_dispatch_extern_msg(const MSG & msg)
{
	if(msg.ttl <= 0) 
	{
		__PRINTF("exter message %d (%d->%d) ttl 0\n",msg.message,msg.source.id,msg.target.id);
		return;
	}
	((MSG&)msg).ttl --;
	switch(msg.target.type)
	{
		case GM_TYPE_PLAYER:
			{
				//不在本地的外部列表中查询了
				int w_idx = w_world_man->SendRemotePlayerMsg(msg.target.id, msg);
				__PRINTF("handle message %d to %d extern %d \n",msg.message,msg.target.id,w_idx);
				/*
				//在本地的外部列表中查询
				int w_idx = w_ext_man.QueryServer(msg.target.id);
				if(w_idx >= 0 && w_idx != w_index) 
					SendRemoteMessage(w_idx,msg);
				else
				{
					//在所有的列表中寻找
					__PRINTF("try other server message\n");
					w_world_man->SendRemotePlayerMsg(msg.target.id, msg);
				}
				__PRINTF("handle message %d to %d extern %d \n",msg.message,msg.target.id,w_idx);
				*/
			}
			break;
		case GM_TYPE_NPC:
		case GM_TYPE_MATTER:
			{
				//npc 和matter处理的方法一样
				size_t w_idx = ID2WIDX(msg.target.id);
				//发送到目标机器上
				if(w_idx != (size_t)w_index && w_idx < MAX_GS_NUM) SendRemoteMessage(w_idx,msg);
				__PRINTF("handle message %d to extern %d \n",msg.message,w_idx);
			}
			break;
		default:
			//ASSERT(false && "不能辨识的目标类型");
			GLog::log(GLOG_ERR,"消息%d中存在不能辨识的目标类型(%d,%d)",msg.message,msg.target.type,msg.target.id);
			return ;
	}
	((MSG&)msg).ttl ++;
}

gobject * 
world::locate_object_from_msg(const MSG & msg)
{
	int id = msg.target.id;
	switch(msg.target.type)
	{
		case GM_TYPE_PLAYER:
			{
				int index = FindPlayer(id);
				if(index == -1) return NULL;
				return GetPlayerByIndex(index);
			}
			break;
		case GM_TYPE_NPC:
			{
				int w_idx = ID2WIDX(id);
				if(w_idx == w_index ) 
				{
					size_t index = ID2IDX(id);
					if(index >= world_manager::GetMaxNPCCount())
					{
						return NULL;
					}
					gnpc *pNPC = GetNPCByIndex(index);
					if(pNPC->ID.id != id) return NULL;
					if(pNPC->plane != this) return NULL;
					return  pNPC;
				}
				else
				{
					//不在本服务器上的NPC，可能是在外部的，所以需要在外部NPC列表中查询
					int eid = GetNPCExternID(id);
					if(eid >= 0)
					{
						gnpc *pNPC = GetNPCByIndex(eid);
						if(pNPC->ID.id == id) return pNPC;
					}
					return NULL;
				}
			}
			break;
		case GM_TYPE_MATTER:
			{
				int w_idx = ID2WIDX(id);
				if(w_idx == w_index) 
				{
					size_t index = ID2IDX(id);
					if(index >= world_manager::GetMaxMatterCount())
					{
						return NULL;
					}
					gmatter *pMatter = GetMatterByIndex(index);
					if(pMatter->ID.id != id) return NULL;
					if(pMatter->plane != this) return NULL;
					return  pMatter;
				}
				else
				{
					__PRINTF("大错误：物品不应该出现在其他服务器上\n");
					return NULL;
				}
			}
			break;
		case GM_TYPE_OBJECT:
			{
				//暂时不支持
				return NULL;
			}
			break;
		default:
			__PRINTF("msg:%d target:(%d %d)\n",msg.message,msg.target.type,msg.target.id);
			GLog::log(LOG_ERR,"消息内存在不能辨识的目标类型 Msg:%d taraget(%d,%d)",msg.message,msg.target.type,msg.target.id);
			//ASSERT(false && "不能辨识的目标类型");
			return NULL;
	}
}


int
world::DispatchMessage(const MSG & msg)
{
#ifndef _NDEBUG
	InterlockedCounter keeper(&_message_handle_count, 1);
#endif
	if(((size_t)(msg.message)) < GM_MSG_MAX)
	{
		interlocked_increment(w_message_counter + msg.message);
	}
	else
	{
		ASSERT(false);
	}
	switch(msg.target.type)
	{
		case GM_TYPE_SERVER:
		case GM_TYPE_BROADCAST:
			//服务端处理的数据，单独处理
			return w_world_man->HandleWorldMessage(this,msg);
		default:
			{
				gobject * obj = locate_object_from_msg(msg);
				if(obj == NULL || !obj->IsActived()) {
					//有可能是在其他服务器上或者什么
					try_dispatch_extern_msg(msg);
					return 0;
				}
#ifdef _DEBUG
				if(msg.message != 31)	__PRINTF("handle message %d to %p\n",msg.message,obj); 
				//滤掉session repeat的函数
#endif
				obj->Lock();
				if(obj->ID.id == msg.target.id)
				{
					call_message_handler(obj,this,msg);
				}
				else
				{
					obj->Unlock();
				}
			}
	}
	return 0;
}

int 
world::DispatchMessage(gobject * obj, const MSG & msg)
{
	obj->Lock();
	call_message_handler(obj,this,msg);
	return 0;
}


namespace
{
template <typename T>
inline gobject * GetList(slice * pPiece)
{
	ASSERT(false && "");
	return NULL;
}

template<>
inline gobject * GetList<gnpc>(slice *pPiece)
{
	return pPiece->npc_list;
}

template<>
inline gobject * GetList<gplayer>(slice *pPiece)
{
	return pPiece->player_list;
}

template<>
inline gobject * GetList<gmatter>(slice *pPiece)
{
	return pPiece->matter_list;
}

template <typename T, typename FUNC>
inline static void ForEachObject(slice * pPiece, const A3DVECTOR & pos, FUNC & func)
{
	gobject * pObj = GetList<T>(pPiece);
	while(pObj)
	{
		func(pObj,pos);
		pObj = pObj->pNext;
	}
}

template <typename T1>
class Foreach1
{
public:
	template <typename FUNC>
	inline static void ForEachInPiece(slice * pPiece, const A3DVECTOR &pos, FUNC & func)
	{
		pPiece->Lock();
		ForEachObject<T1>(pPiece,pos,func);
		pPiece->Unlock();
	};
};

template <typename T1,typename T2>
class Foreach2
{
public:
	template <typename FUNC>
	inline static void ForEachInPiece(slice * pPiece, const A3DVECTOR &pos, FUNC & func)
	{
		pPiece->Lock();
		ForEachObject<T1>(pPiece,pos,func);
		ForEachObject<T2>(pPiece,pos,func);
		pPiece->Unlock();
	};
};

template <typename T1,typename T2,typename T3>
class Foreach3
{
public:
	
	template <typename FUNC>
	inline static void ForEachInPiece(slice * pPiece, const A3DVECTOR &pos, FUNC & func)
	{
		pPiece->Lock();
		ForEachObject<T1>(pPiece,pos,func);
		ForEachObject<T2>(pPiece,pos,func);
		ForEachObject<T3>(pPiece,pos,func);
		pPiece->Unlock();
	};
};

template <typename FOREACH>
struct object_collector
{
	world * _plane;
	int _mask;
	abase::vector<gobject *,abase::fast_alloc<> > &_list;
	float _squared_radius;
	object_collector(world * plane,int mask,abase::vector<gobject *,abase::fast_alloc<> > &list,float radius):_plane(plane),_mask(mask),_list(list),_squared_radius(radius*radius){}

	inline void operator()(gobject * pObj, const A3DVECTOR & pos)
	{
		if((pObj->msg_mask & _mask) && pos.squared_distance(pObj->pos) < _squared_radius)
		{
			_list.push_back(pObj);
		}
	}
	
	inline void operator()(slice *pPiece,const A3DVECTOR & pos)
	{
		FOREACH::ForEachInPiece(pPiece,pos,*this);
	}
};

template <typename FOREACH>
struct object_sphere_collector
{
	world * _plane;
	abase::vector<gobject *,abase::fast_alloc<> > &_list;
	float _squared_radius;
	object_sphere_collector(world * plane,abase::vector<gobject *,abase::fast_alloc<> > &list,float radius)
		:_plane(plane),_list(list),_squared_radius(radius*radius){}
	
	inline void operator()(gobject * pObj, const A3DVECTOR & pos)
	{
		if(pos.squared_distance(pObj->pos) < _squared_radius)
		{
			_list.push_back(pObj);
		}
	}

	inline void operator()(slice *pPiece,const A3DVECTOR & pos)
	{
		FOREACH::ForEachInPiece(pPiece,pos,*this);
	}
};

template <typename FOREACH>
struct object_cylinder_collector
{
	world * _plane;
	abase::vector<gobject *,abase::fast_alloc<> > &_list;
	float _squared_radius;
	float _squared_range;
	float _inv_range;
	A3DVECTOR _offset;
	object_cylinder_collector(world * plane,abase::vector<gobject *,abase::fast_alloc<> > &list,
			const A3DVECTOR &start, const A3DVECTOR &target,
			float radius):_plane(plane),_list(list),_squared_radius(radius*radius), _offset(target)
	{
		_offset -= start;
		_squared_range = _offset.squared_magnitude();
		_inv_range = 1.f/_squared_range;
	}
	
	inline void operator()(gobject * pObj, const A3DVECTOR & pos)
	{
		A3DVECTOR vec= pObj->pos;
		vec -= pos;
		float dp = vec.dot_product(_offset);
		if(dp > 0 && dp < _squared_range)
		{
			if(vec.squared_magnitude() - dp*dp*_inv_range <= _squared_radius)
			{
				_list.push_back(pObj);
			}
		}
	}

	inline void operator()(slice *pPiece,const A3DVECTOR & pos)
	{
		FOREACH::ForEachInPiece(pPiece,pos,*this);
	}
};

template <typename FOREACH>
struct object_taper_collector
{
	world * _plane;
	abase::vector<gobject *,abase::fast_alloc<> > &_list;
	float _squared_radius;
	A3DVECTOR _offset;
	float _trans_value;	// cos^2(1/2*angle) * distance^2 
	object_taper_collector(world * plane, abase::vector<gobject *,abase::fast_alloc<> > &list,
			const A3DVECTOR &start, const A3DVECTOR &target, float radius,
			float cos_halfangle):_plane(plane),_list(list),_squared_radius(radius*radius),
				      _offset(target),_trans_value(cos_halfangle*cos_halfangle)
	{
		_offset -= start;
		_trans_value *= _offset.squared_magnitude();
	}
	
	inline void operator()(gobject * pObj, const A3DVECTOR & pos)
	{
		A3DVECTOR vec = pObj->pos;
		vec -= pos;
		float dis = vec.squared_magnitude(); 
		if(dis <= _squared_radius)
		{
			float dp = vec.dot_product(_offset);
			if(dp >=0  && dp * dp > _trans_value * dis)
			{
				//cos * cos == (cos^angle * distance^2 * new_distance^2
				_list.push_back(pObj);
			}
		}
	}

	inline void operator()(slice *pPiece,const A3DVECTOR & pos)
	{
		FOREACH::ForEachInPiece(pPiece,pos,*this);
	}
};

template <typename FOREACH>
struct object_box_collector
{
	world * _plane;
	abase::vector<gobject *,abase::fast_alloc<> > &_list;
	rect _rt;
	object_box_collector(world * plane,abase::vector<gobject *,abase::fast_alloc<> > &list,const rect &rt)
		:_plane(plane),_list(list),_rt(rt){}
	
	inline void operator()(gobject * pObj, const A3DVECTOR & pos)
	{
		if(_rt.IsIn(pos.x,pos.z))
		{
			_list.push_back(pObj);
		}
	}

	inline void operator()(slice *pPiece,const A3DVECTOR & pos)
	{
		FOREACH::ForEachInPiece(pPiece,pos,*this);
	}
};

}

int 
world::BroadcastLocalMessage(const MSG & msg,float fRadius,int mask)
{
	abase::vector<gobject*,abase::fast_alloc<> > list;
	switch(msg.target.type)
	{
		case GM_TYPE_PLAYER:
			{
				object_collector<Foreach1<gplayer> > worker(this,mask,list,fRadius);
				ForEachSlice(msg.pos,fRadius,worker);
			}
			break;
		case GM_TYPE_NPC:
			{
				object_collector<Foreach1<gnpc> > worker(this,mask,list,fRadius);
				ForEachSlice(msg.pos,fRadius,worker);
			}
			break;
		case GM_TYPE_MATTER:
			{
				object_collector<Foreach1<gmatter> > worker(this,mask,list,fRadius);
				ForEachSlice(msg.pos,fRadius,worker);
			}
			break;
		case GM_TYPE_ACTIVE:
			{
				//这里可以想办法减少一些构造的负担
				object_collector<Foreach1<gplayer> > worker1(this,mask,list,fRadius);
				ForEachSlice(msg.pos,fRadius,worker1);

				object_collector<Foreach1<gnpc> > worker2(this,mask,list,fRadius);
				ForEachSlice(msg.pos,fRadius,worker2);
			}
			break;
		case -1:		//-1表示全部发送
			{
				object_collector<Foreach1<gplayer> > worker1(this,mask,list,fRadius);
				ForEachSlice(msg.pos,fRadius,worker1);
				
				object_collector<Foreach1<gnpc> > worker2(this,mask,list,fRadius);
				ForEachSlice(msg.pos,fRadius,worker2);
				
				object_collector<Foreach1<gmatter> > worker3(this,mask,list,fRadius);
				ForEachSlice(msg.pos,fRadius,worker3);
			}
			break;
		default:
			ASSERT(false);
			return 0;
	}
	if(!list.empty())
	{
		//ONET::Thread::Pool::AddTask(new SendMessageTask(this,list,msg));
		//__PRINTF("broadcast count %d\n",list.size());
		w_world_man->PostMultiMessage(this,list,msg);
	}
	return 0;
}

int 
world::BroadcastMessage(const MSG & msg,float fRadius,int mask)
{
	//首先判断是否和外面的区域相交
	rect rt(msg.pos.x - fRadius,msg.pos.z - fRadius,msg.pos.x + fRadius,msg.pos.z + fRadius);
	if(!GetGrid().inner_region.IsIn(rt))
	{
		//不是完全在自己的范围之内，开始扫描
		MSG *pMsg  = NULL;
		pMsg = (MSG*) abase::fast_allocator::raw_alloc(sizeof(MSG) + sizeof(MSG) + msg.content_length);
		memset(pMsg,0,sizeof(MSG));
		char * content;
		pMsg->content = content = ((char *)pMsg) + sizeof(MSG);
		memcpy(content,&msg,sizeof(MSG));
		if(msg.content_length) memcpy(content + sizeof(MSG),msg.content,msg.content_length);
		pMsg->pos.x  = fRadius;
		pMsg->param = mask;
		pMsg->ttl = 1;
		pMsg->message = GM_MSG_FORWARD_BROADCAST;
		pMsg->source.type = GM_TYPE_SERVER;
		pMsg->content_length = sizeof(MSG) + msg.content_length;

		BroadcastSvrMessage(rt,*pMsg,0.f);
		abase::fast_allocator::raw_free(pMsg);
	}
	return BroadcastLocalMessage(msg,fRadius,mask);
}


int
world::BroadcastSphereMessage(const MSG & msg,const A3DVECTOR & target, float fRadius)
{
	//首先判断是否和外面的区域相交
	rect rt(target.x - fRadius,target.z - fRadius,target.x + fRadius,target.z + fRadius);
	if(!GetGrid().inner_region.IsIn(rt))
	{
		//不是完全在自己的范围之内，开始扫描
		MSG *pMsg  = NULL;
		pMsg = (MSG*) abase::fast_allocator::raw_alloc(sizeof(MSG) + sizeof(MSG) + msg.content_length);
		memset(pMsg,0,sizeof(MSG));
		char * content;
		pMsg->content = content = ((char *)pMsg) + sizeof(MSG);
		memcpy(content,&msg,sizeof(MSG));
		if(msg.content_length) memcpy(content + sizeof(MSG),msg.content,msg.content_length);
		pMsg->pos = target;
		pMsg->param = *(int*)&fRadius;
		pMsg->ttl = 1;
		pMsg->message = GM_MSG_FORWARD_BROADCAST_SPHERE;
		pMsg->source.type = GM_TYPE_SERVER;
		pMsg->content_length = sizeof(MSG) + msg.content_length;

		BroadcastSvrMessage(rt,*pMsg,0.f);
		abase::fast_allocator::raw_free(pMsg);
	}
	return BroadcastLocalSphereMessage(msg,target,fRadius);
}

int 
world::BroadcastCylinderMessage(const MSG & msg,const A3DVECTOR & target, float fRadius)
{
	//首先判断是否和外面的区域相交
	rect rt(msg.pos,target);
	if(!GetGrid().inner_region.IsIn(rt))
	{
		//不是完全在自己的范围之内，开始扫描
		MSG *pMsg  = NULL;
		pMsg = (MSG*) abase::fast_allocator::raw_alloc(sizeof(MSG) + sizeof(MSG) + msg.content_length);
		memset(pMsg,0,sizeof(MSG));
		char * content;
		pMsg->content = content = ((char *)pMsg) + sizeof(MSG);
		memcpy(content,&msg,sizeof(MSG));
		if(msg.content_length) memcpy(content + sizeof(MSG),msg.content,msg.content_length);
		pMsg->pos = target;
		pMsg->param = *(int*)&fRadius;
		pMsg->ttl = 1;
		pMsg->message = GM_MSG_FORWARD_BROADCAST_CYLINDER;
		pMsg->source.type = GM_TYPE_SERVER;
		pMsg->content_length = sizeof(MSG) + msg.content_length;

		BroadcastSvrMessage(rt,*pMsg,0.f);
		abase::fast_allocator::raw_free(pMsg);
	}
	return BroadcastLocalCylinderMessage(msg,target,fRadius);
}

int 
world::BroadcastTaperMessage(const MSG & msg,const A3DVECTOR& target,float fRadius,float cos_halfangle)
{
	//首先判断是否和外面的区域相交
	rect rt(msg.pos.x - fRadius,msg.pos.z - fRadius,msg.pos.x + fRadius,msg.pos.z + fRadius);
	if(!GetGrid().inner_region.IsIn(rt))
	{
		//不是完全在自己的范围之内，开始扫描
		MSG *pMsg  = NULL;
		pMsg = (MSG*) abase::fast_allocator::raw_alloc(sizeof(MSG) + sizeof(MSG) + msg.content_length);
		memset(pMsg,0,sizeof(MSG));
		char * content;
		pMsg->content = content = ((char *)pMsg) + sizeof(MSG);
		memcpy(content,&msg,sizeof(MSG));
		if(msg.content_length) memcpy(content + sizeof(MSG),msg.content,msg.content_length);
		pMsg->pos = target;
		pMsg->param = *(int*)&fRadius;
		pMsg->ttl = 1;
		pMsg->message = GM_MSG_FORWARD_BROADCAST_TAPER;
		pMsg->source.type = GM_TYPE_SERVER;
		pMsg->source.id = *(int*)&cos_halfangle;
		pMsg->content_length = sizeof(MSG) + msg.content_length;

		BroadcastSvrMessage(rt,*pMsg,0.f);
		abase::fast_allocator::raw_free(pMsg);
	}
	return BroadcastLocalTaperMessage(msg,target,fRadius,cos_halfangle);
}

int 
world::BroadcastLocalBoxMessage(const MSG & msg, const rect & rt)
{
	abase::vector<gobject*,abase::fast_alloc<> > list;
	switch(msg.target.type)
	{
		case GM_TYPE_PLAYER:
			{
				object_box_collector<Foreach1<gplayer> > worker(this,list,rt);
				ForEachSlice(msg.pos,rt,worker);
			}
			break;
		case GM_TYPE_NPC:
			{
				object_box_collector<Foreach1<gnpc> > worker(this,list,rt);
				ForEachSlice(msg.pos,rt,worker);
			}
			break;
		case GM_TYPE_MATTER:
			{
				object_box_collector<Foreach1<gmatter> > worker(this,list,rt);
				ForEachSlice(msg.pos,rt,worker);
			}
			break;
		case GM_TYPE_ACTIVE:
			{
				object_box_collector<Foreach2<gplayer,gnpc> > worker(this,list,rt);
				ForEachSlice(msg.pos,rt,worker);
			}
			break;
		case -1:		//-1表示全部发送
			{
				object_box_collector<Foreach3<gplayer,gnpc,gmatter> > worker(this,list,rt);
				ForEachSlice(msg.pos,rt,worker);
			}
			break;
		default:
			ASSERT(false);
			return 0;
	}
	if(!list.empty())
	{
		//ONET::Thread::Pool::AddTask(new SendMessageTask(this,list,msg));
		//__PRINTF("broadcast count %d\n",list.size());
		w_world_man->PostMultiMessage(this,list,msg);
	}
	return 0;
}

int 
world::BroadcastLocalSphereMessage(const MSG & msg,const A3DVECTOR & target, float fRadius)
{
	abase::vector<gobject*,abase::fast_alloc<> > list;
	switch(msg.target.type)
	{
		case GM_TYPE_PLAYER:
			{
				object_sphere_collector<Foreach1<gplayer> > worker(this,list,fRadius);
				ForEachSlice(target,fRadius,worker);
			}
			break;
		case GM_TYPE_NPC:
			{
				object_sphere_collector<Foreach1<gnpc> > worker(this,list,fRadius);
				ForEachSlice(target,fRadius,worker);
			}
			break;
		case GM_TYPE_MATTER:
			{
				object_sphere_collector<Foreach1<gmatter> > worker(this,list,fRadius);
				ForEachSlice(target,fRadius,worker);
			}
			break;
		case GM_TYPE_ACTIVE:
			{
				object_sphere_collector<Foreach2<gplayer,gnpc> > worker(this,list,fRadius);
				ForEachSlice(target,fRadius,worker);
			}
			break;
		case -1:		//-1表示全部发送
			{
				object_sphere_collector<Foreach3<gplayer,gnpc,gmatter> > worker(this,list,fRadius);
				ForEachSlice(target,fRadius,worker);
			}
			break;
		default:
			ASSERT(false);
			return 0;
	}
	if(!list.empty())
	{
		//ONET::Thread::Pool::AddTask(new SendMessageTask(this,list,msg));
		//__PRINTF("broadcast count %d\n",list.size());
		w_world_man->PostMultiMessage(this,list,msg);
	}
	return 0;
}
int 
world::BroadcastLocalCylinderMessage(const MSG & msg,const A3DVECTOR & target, float fRadius)
{
	abase::vector<gobject*,abase::fast_alloc<> > list;
	switch(msg.target.type)
	{
		case GM_TYPE_PLAYER:
			{
				object_cylinder_collector<Foreach1<gplayer> > worker(this,list,msg.pos,target,fRadius);
				ForEachSlice(msg.pos,target,worker);
			}
			break;
		case GM_TYPE_NPC:
			{
				object_cylinder_collector<Foreach1<gnpc> > worker(this,list,msg.pos,target,fRadius);
				ForEachSlice(msg.pos,target,worker);
			}
			break;
		case GM_TYPE_MATTER:
			{
				object_cylinder_collector<Foreach1<gmatter> > worker(this,list,msg.pos,target,fRadius);
				ForEachSlice(msg.pos,target,worker);
			}
			break;
		case GM_TYPE_ACTIVE:
			{
				object_cylinder_collector<Foreach2<gplayer,gnpc> > worker(this,list,msg.pos,target,fRadius);
				ForEachSlice(msg.pos,target,worker);
			}
			break;
		case -1:		//-1表示全部发送
			{
				object_cylinder_collector<Foreach3<gplayer,gnpc,gmatter> > worker(this,list,msg.pos,target,fRadius);
				ForEachSlice(msg.pos,target,worker);
			}
			break;
		default:
			ASSERT(false);
			return 0;
	}
	if(!list.empty())
	{
		//ONET::Thread::Pool::AddTask(new SendMessageTask(this,list,msg));
		//__PRINTF("broadcast count %d\n",list.size());
		w_world_man->PostMultiMessage(this,list,msg);
	}
	return 0;
}

int 
world::BroadcastLocalTaperMessage(const MSG & msg,const A3DVECTOR & target,float fRadius,float cos_halfangle)
{
	abase::vector<gobject*,abase::fast_alloc<> > list;
	switch(msg.target.type)
	{
		case GM_TYPE_PLAYER:
			{
				object_taper_collector<Foreach1<gplayer> > worker(this,list,msg.pos,target,fRadius,cos_halfangle);
				ForEachSlice(msg.pos,target,worker);
			}
			break;
		case GM_TYPE_NPC:
			{
				object_taper_collector<Foreach1<gnpc> > worker(this,list,msg.pos,target,fRadius,cos_halfangle);
				ForEachSlice(msg.pos,target,worker);
			}
			break;
		case GM_TYPE_MATTER:
			{
				object_taper_collector<Foreach1<gmatter> > worker(this,list,msg.pos,target,fRadius,cos_halfangle);
				ForEachSlice(msg.pos,target,worker);
			}
			break;
		case GM_TYPE_ACTIVE:
			{
				object_taper_collector<Foreach2<gplayer,gnpc> > worker(this,list,msg.pos,target,fRadius,cos_halfangle);
				ForEachSlice(msg.pos,target,worker);
			}
			break;
		case -1:		//-1表示全部发送
			{
				object_taper_collector<Foreach3<gplayer,gnpc,gmatter> > worker2(this,list,msg.pos,target,fRadius,cos_halfangle);
				ForEachSlice(msg.pos,target,worker2);
				
			}
			break;
		default:
			ASSERT(false);
			return 0;
	}
	if(!list.empty())
	{
		//ONET::Thread::Pool::AddTask(new SendMessageTask(this,list,msg));
		//__PRINTF("broadcast count %d\n",list.size());
		w_world_man->PostMultiMessage(this,list,msg);
	}
	return 0;
}


namespace
{
void MakeObjectInfo(gactive_object * pObj, world::object_info & info)
{
	if(pObj->IsZombie())
	{
		info.state = world::QUERY_OBJECT_STATE_ZOMBIE;
	}
	else
	{
		info.state = world::QUERY_OBJECT_STATE_ACTIVE;
	}
	if(pObj->b_disconnect)
	{
		info.state |= world::QUERY_OBJECT_STATE_DISCONNECT;
	}
	info.pos = pObj->pos;
	info.body_size = pObj->body_size;
	info.race = pObj->base_info.race;
	info.faction = pObj->base_info.faction;
	info.hp = pObj->base_info.hp;
	info.mp = pObj->base_info.mp;
	info.level = pObj->base_info.level;
	info.max_hp = pObj->base_info.max_hp;
	info.invisible_degree = pObj->invisible_degree;
	info.anti_invisible_degree = pObj->anti_invisible_degree;
	info.object_state = pObj->object_state;
	info.object_state2 = pObj->object_state2;
	info.mafia_id = 0;
}
void MakeObjectInfo(gmatter * pObj, world::object_info & info)
{
	info.state = world::QUERY_OBJECT_STATE_ACTIVE;
	info.pos = pObj->pos;
	info.body_size = pObj->body_size;
	info.race = pObj->matter_type;
	info.faction = 0;
	info.hp = 0;
	info.mp = 0;
	info.level = 0;
	info.max_hp = 0;
	info.invisible_degree = 0;
	info.anti_invisible_degree = 0;
	info.object_state = 0;
	info.object_state2 = 0;
	info.mafia_id = 0;
}
void MakeObjectInfo(gnpc* pNpc, world::object_info & info)
{
	MakeObjectInfo((gactive_object*) pNpc, info);
	info.mafia_id = pNpc->mafia_id;
	info.master_id = pNpc->master_id;
}
void MakeObjectInfo(gplayer* pPlayer, world::object_info & info)
{
	MakeObjectInfo((gactive_object*) pPlayer, info); 
	info.mafia_id = pPlayer->id_mafia;
}

}


bool 
world::QueryObject(const XID & id,object_info & info)
{	
	//对一个对象进行查询
	//查询策略是：
	//1.首先在本地寻找,
	//2.如果本地没有找到的话,在本地外部列表和全局列表中寻找
	//3.如果仍然没有找到,那么对该对象可能所在的服务器发出搜寻请求(仅限于NPC和物品)
	//由于绝大部分查询都应该在本地找到,因此要着重考虑本地的优化
	switch(id.type)
	{
		case GM_TYPE_NPC:
		{
			int widx = ID2WIDX(id.id);
			size_t index = ID2IDX(id.id);
			if(index >= world_manager::GetMaxNPCCount()) return false;
			if(widx == world_manager::GetWorldIndex())
			{
				//是本地原生的,一定有记录
				gnpc * pNPC = GetNPCByIndex(index);
				if(pNPC->IsActived() && pNPC->plane == this)
				{
					MakeObjectInfo(pNPC,info);
					return true;
				}
				return false;
			}
			else
			{
				//在本地的外部对象列表zhon个查询
				index = GetNPCExternID(id.id);
				if(index < world_manager::GetMaxNPCCount())
				{
					gnpc * pNPC = GetNPCByIndex(index);
					if(pNPC->IsActived())
					{
						MakeObjectInfo(pNPC,info);
						return true;
					}
					return false;
				}

				//在外部列表中查询
				return w_ext_man.QueryObject(id.id,info);
			}
			//现在也没有检测外面服务器的npc 
		}
		return false;
		break;
		case GM_TYPE_PLAYER:
		{
			int index = FindPlayer(id.id);
			if(index >=0)
			{
				gplayer * pPlayer = GetPlayerByIndex(index);
				if(pPlayer->IsActived())
				{
					MakeObjectInfo(pPlayer,info);
					return true;
				}
				return false;
			}
			else
			{
				//在外部列表中查询
				return w_ext_man.QueryObject(id.id,info);
			}
		}
		return false;
		break;

		case GM_TYPE_MATTER:
		{
			int widx = ID2WIDX(id.id);
			size_t index = ID2IDX(id.id);
			if(index >= world_manager::GetMaxMatterCount()) return false;
			if(widx == world_manager::GetWorldIndex())
			{
				//是本地原生的,一定在本地
				gmatter * pMatter= GetMatterByIndex(index);
				if(pMatter->IsActived() && pMatter->plane == this)
				{
					MakeObjectInfo(pMatter,info);
					return true;
				}
				return false;
			}
			else
			{
				//事实上，如果是本地能够查询的物品，必然在边界处，所以本地应该可以知道
				//在外部列表中查询
				return w_ext_man.QueryObject(id.id,info);
			}
		}
		return false;
		default:
		ASSERT(false);
		return false;
	}

}

void 	
world::DuplicateWorld(world * dest) const
{
	dest->Init(w_index);
	dest->InitManager(w_world_man);
	
	dest->w_grid = w_grid;
	dest->w_off_list	= w_off_list;
	dest->w_near_vision	= w_near_vision;
	dest->w_far_vision	= w_far_vision;
	dest->w_true_vision	= w_true_vision;
	dest->w_vision		= w_vision;
	if(w_ctrl) dest->w_ctrl	= w_ctrl->Clone();
	dest->w_life_time = w_life_time;
}

void world::DumpMessageCount()
{
	for(size_t i =0;i < GM_MSG_MAX;i ++)
	{
		size_t count = w_message_counter[i];
		if(count > 0)
		{
			__PRINTINFO("MESSAGE:%4d\tcounter:%8d\n",i,count);
		}
	}
}

void
world::ResetWorld()
{
	if(w_ctrl) w_ctrl->Reset();
	//现在Reset只做很少的事情 
	//现在发现这样是不对的，应该让所有的NPC回到正常的状态并且ClearSession才是
	//而现在直接将所有世界中存在的对象释放
	//这个世界的心跳将会停止，所以再也不会有重生和新怪物出现了
	int count = w_grid.reg_row * w_grid.reg_column;
	abase::vector<gnpc *, abase::fast_alloc<> > list1;
	abase::vector<gmatter *, abase::fast_alloc<> > list2;
	list1.reserve(1000);
	list2.reserve(1000);

	//锁住所有的slice先 先小后大， 所以不会有死锁
	for(int i = 0; i < count; i ++)
	{
		w_grid.pTable[i].Lock();
	}

	//收集所有的NPC和matter，用完就解开锁
	for(int i = 0; i < count; i ++)
	{
		slice & piece = w_grid.pTable[i];
		gnpc * pNPC = (gnpc*)piece.npc_list;
		while(pNPC)
		{
			list1.push_back(pNPC);
			pNPC = (gnpc*)pNPC->pNext;
		}

		gmatter * pMatter = (gmatter*)w_grid.pTable[i].matter_list;
		while(pMatter)
		{
			list2.push_back(pMatter);
			pMatter = (gmatter*)pMatter->pNext;
		}
		piece.Unlock();
	}

	//释放收集到的NPC
	for(size_t i =0; i < list1.size(); i ++)
	{
		gnpc * pNPC = list1[i];
		//理论上存在这时正好消失和死亡的npc
		pNPC->Lock();
		if(pNPC->imp && pNPC->IsActived())
		{
			pNPC->imp->_commander->Release();
		}
		else
		{
			//什么也不做
			//这个NPC应该已经释放了
			//或者被管理器管理了
		}
		pNPC->Unlock();
	}

	//释放收集到的Matter
	for(size_t i =0; i < list2.size(); i ++)
	{
		gmatter * pMatter = list2[i];
		//理论上存在这时正好消失和死亡的matter
		pMatter->Lock();
		if(pMatter->imp && pMatter->IsActived())
		{
			pMatter->imp->_commander->Release();
		}
		else
		{
			//什么也不做
			//这个NPC应该已经释放了
			//或者被管理器管理了
		}
		pMatter->Unlock();
	}
}

void world::Release()
{	
	w_battle_result = 0;

	//将管理器中的对象(待复生的和其它的)和内容释放
	w_npc_gen.Release();
}

void 
world::BattleFactionSay(int faction, const void * msg, size_t size, char emote_id, const void * aux_data, size_t dsize, int self_id, int self_level)
{
	if(w_ctrl)
	{
		w_ctrl->BattleFactionSay(faction, msg, size, emote_id, aux_data, dsize, self_id, self_level);
	}
}

void 
world::BattleSay(const void * msg, size_t size)
{
	if(w_ctrl)
	{
		w_ctrl->BattleSay(msg, size);
	}
}

void world::InstanceSay(const void * msg, size_t size, bool middle, const void* data, size_t dsize)
{
	if(!world_manager::GetWorldLimit().common_data_notify) return;		//只有这个功能打开，player_list才是有效的
	spin_autolock keeper(w_player_node_lock);
	multi_send_chat_msg(w_player_node_list,msg,size,GMSV::CHAT_CHANNEL_INSTANCE,0,data,dsize,middle?1:0,0);
}

void 
world::SetCommonValue(int key,int value,bool notify_world_ctrl)
{
	w_common_data.SetValue(key, value);
	if(key > 100000)
	{
		//通知所有玩家
		CommonDataNotify(key, value);
		if(notify_world_ctrl && w_ctrl) w_ctrl->OnSetCommonValue(key, value);
	}
}

int 
world::GetCommonValue(int key)
{
	return w_common_data.GetValue(key);
}

int 
world::ModifyCommonValue(int key, int offset)
{
	int rst =  w_common_data.ModifyValue(key,offset);
	if(key > 100000)
	{
		//通知所有玩家
		CommonDataNotify(key, rst);
		if(w_ctrl) w_ctrl->OnSetCommonValue(key, rst);
	}
	return rst;
}

void
world::AddPlayerNode(gplayer * pPlayer)
{
	spin_autolock keeper(w_player_node_lock);
	int cs_index = pPlayer->cs_index;
	std::pair<int,int> val(pPlayer->ID.id,pPlayer->cs_sid);
	if(cs_index >= 0 && val.first >= 0)
	{
		w_player_node_list[cs_index].push_back(val);
	}
}

void
world::DelPlayerNode(gplayer * pPlayer)
{
	spin_autolock keeper(w_player_node_lock);
	int cs_index = pPlayer->cs_index;
	std::pair<int,int> val(pPlayer->ID.id,pPlayer->cs_sid);
	if(cs_index >= 0 && val.first >= 0)
	{
		cs_user_list & list = w_player_node_list[cs_index];
		int id = pPlayer->ID.id;
		for(size_t i = 0; i < list.size(); i ++)
		{
			if(list[i].first == id)
			{
				list.erase(list.begin() + i);
				i --;
			}
		}
	}
}


void 
world::CommonDataNotify(int key, int value)
{
	packet_wrapper h1(64);
	using namespace S2C;
	CMD::Make<CMD::common_data_notify>::From(h1);
	CMD::Make<CMD::common_data_notify>::Add(h1, key, value);
	
	spin_autolock keeper(w_player_node_lock);
	multi_send_ls_msg(w_player_node_list, h1.data(), h1.size(), 0);
}

void 
world::AddSceneServiceNpc(int tid, int id)
{
	spin_autolock keeper(w_scene_service_npcs_lock);
	abase::static_multimap<int, int>::iterator it = w_scene_service_npcs.find(tid);
	if(it == w_scene_service_npcs.end())
	{
		w_scene_service_npcs[tid] = id;
	}
}

void 
world::DelSceneServiceNpc(int tid, int id)
{
	spin_autolock keeper(w_scene_service_npcs_lock);
	abase::static_multimap<int, int>::iterator it = w_scene_service_npcs.find(tid);
	if(it != w_scene_service_npcs.end() && it->second == id)
	{
		w_scene_service_npcs.erase(tid);
	}
}

void 
world::GetSceneServiceNpc(abase::vector<int> & list)
{
	spin_autolock keeper(w_scene_service_npcs_lock);
	size_t count = w_scene_service_npcs.size();
	if(count)
	{
		list.reserve(count * 2);
		for(abase::static_multimap<int, int>::iterator it = w_scene_service_npcs.begin();
				it != w_scene_service_npcs.end(); ++it)
		{
			list.push_back(it->first);
			list.push_back(it->second);
		}
	}
}

int 
world::GetSpherePlayerListSize(const A3DVECTOR& target, float fRadius)
{
	abase::vector< gobject*,abase::fast_alloc<> > list;
	object_sphere_collector<Foreach1<gplayer> > worker(this,list,fRadius);
	ForEachSlice(target,fRadius,worker);
	return list.size();
}

int 
world::RebuildMapRes()
{
	int mapres_type = w_world_man->GetMapRes().GetType(); 
	if(mapres_type == MAPRES_TYPE_ORIGIN) return 0;
	if(w_map_generator || w_terrain || w_movemap || w_traceman) return -16;
	
	switch(mapres_type)
	{
		case MAPRES_TYPE_RANDOM:
			w_map_generator = new random_map_generator;
			break;
		case MAPRES_TYPE_MAZE:
			w_map_generator = new maze_map_generator;
			break;
		case MAPRES_TYPE_SEQUENCE:
			w_map_generator = new sequence_map_generator;
			break;
		case MAPRES_TYPE_SOLO_CHALLENGE:
			w_map_generator = new solo_challenge_map_generator;
			return 0;
		default:
			return -1;
	}

	try{	
		const rect & local_rt = GetLocalWorld();
		MapResManager & mapres = w_world_man->GetMapRes();

		if(!w_map_generator->Generate(local_rt, mapres.GetMapResInfo())) throw -2;

		w_terrain = mapres.CreateTerrain(w_map_generator);
		if(!w_terrain) throw -3;

		w_movemap = mapres.CreateMoveMap(w_map_generator, this);
		if(!w_movemap) throw -4;

		w_traceman = mapres.CreateTraceMan(w_map_generator);
		if(!w_traceman) throw -5;
	}
	catch(int err)
	{
		ASSERT(err != 0);
		if(w_map_generator){ delete w_map_generator; w_map_generator = NULL; }
		if(w_terrain){ delete w_terrain; w_terrain = NULL; }
		if(w_movemap){ delete w_movemap; w_movemap = NULL; }
		if(w_traceman){ delete w_traceman; w_traceman = NULL; }
		return err;
	}
	return 0;
}

void 
world::SyncPlayerWorldGen(gplayer* pPlayer)
{
	if(w_map_generator) w_map_generator->SyncPlayerWorldGen((gplayer_imp*)pPlayer->imp);
}

