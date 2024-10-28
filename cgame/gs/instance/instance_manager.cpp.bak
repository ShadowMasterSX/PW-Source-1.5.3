#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <ASSERT.h>

#include <threadpool.h>
#include <conf.h>
#include <io/pollio.h>
#include <io/passiveio.h>
#include <gsp_if.h>
#include <db_if.h>
#include <amemory.h>
#include <meminfo.h>
#include <strtok.h>
#include <glog.h>

#include "../template/itemdataman.h"
#include "../world.h"
#include "../player_imp.h"
#include "../npc.h"
#include "../matter.h"
#include "../playertemplate.h"
#include "instance_config.h"
#include "instance_manager.h"
#include "../template/globaldataman.h"
#include <factionlib.h>
#include "faction_world_ctrl.h"
#include "../aei_filter.h"


bool 
instance_world_manager::InitNetClient(const char * gmconf)
{
	extern unsigned long _task_templ_cur_version;
	char version[1024];
	int ele_version = ELEMENTDATA_VERSION;
	int task_version = _task_templ_cur_version;
	int gshop_version = globaldata_getmalltimestamp();
	int gdividendshop_version = globaldata_getmall2timestamp();
	sprintf(version, "%x%x%x%x", ele_version, task_version, gshop_version, gdividendshop_version);

	rect rt = _plane_template->GetLocalWorld();
	ASSERT(rt.left < rt.right && rt.top < rt.bottom);
	GMSV::InitGSP(gmconf,world_manager::GetWorldIndex(),_world_tag,rt.left,rt.right,rt.top,rt.bottom,version);
	GDB::init_gamedb();
	return true;
}

static bool quit_flag = false; 
static void timer_thread()
{
	g_timer.timer_thread();
}

static void poll_thread()
{
	for(;!quit_flag;)
	{
		ONET::PollIO::Poll(50);
	}
	__PRINTF("poll thread  terminated\n");
}
static void str2rect(rect & rt,const char * str)
{
	sscanf(str,"{%f,%f} , {%f,%f}",&rt.left,&rt.top,&rt.right,&rt.bottom);
}
	
void 
instance_world_manager::timer_tick(int index,void *object,int remain)
{
	class World_Tick_Task : public ONET::Thread::Runnable, public abase::ASmallObject
	{
		public:
		instance_world_manager * _manager;
		World_Tick_Task(instance_world_manager * manager):_manager(manager){}
		virtual void Run()
		{
			_manager->Heartbeat();
			delete this;
		}
	};
	ONET::Thread::Pool::AddTask(new World_Tick_Task((instance_world_manager*)object));
}

world * 
instance_world_manager::CreateWorldTemplate()
{
	world * pPlane  = new world;
	pPlane->Init(_world_index);
	pPlane->InitManager(this);
	//设置世界的生命周期
	pPlane->w_life_time = _life_time;
	return pPlane;
}

world_message_handler * 
instance_world_manager::CreateMessageHandler()
{
	return new instance_world_message_handler(this);
}

int
instance_world_manager::Init(const char * gmconf_file,const char *  servername)
{
	_message_handler = CreateMessageHandler();
	_manager_instance = this;

	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGALRM);
	sigprocmask(SIG_BLOCK, &set, NULL);

	/*
	 *      开始初始化世界
	 */
	Conf *conf = Conf::GetInstance();
	Conf::section_type section = "Instance_";
	section += servername;

	PreInit(servername);

	//读取副本限制
	_player_limit_per_instance = atoi(conf->find(section,"player_per_instance").c_str());
	if(_player_limit_per_instance <= 0) 
	{
		__PRINTINFO("每副本中容纳玩家数必须高于0\n");
		return -1010;
	}
	_effect_player_per_instance = atoi(conf->find(section,"effect_player_per_instance").c_str());
	if(_effect_player_per_instance < 0) 
	{
		__PRINTINFO("每副本中有效玩家数必须大于或等于0\n");
		return -1010;
	}
	
	size_t instance_count = atoi(conf->find(section,"instance_capacity").c_str());
	if(instance_count > MAX_INSTANCE_PER_SVR) instance_count = MAX_INSTANCE_PER_SVR;
	_planes_capacity = instance_count;

	_pool_threshold_low  =  atoi(conf->find(section,"pool_threshold_low").c_str());
	_pool_threshold_high =  atoi(conf->find(section,"pool_threshold_high").c_str());
	int pool_threadhold_init = atoi(conf->find(section,"pool_threshold_init").c_str());

	if(_pool_threshold_low <= 0 ||  _pool_threshold_high <=0
			|| _pool_threshold_low > _pool_threshold_high)
	{
		__PRINTINFO("世界缓冲池阈值错误\n");
		return -1011;
	}

	if(pool_threadhold_init > _pool_threshold_high) pool_threadhold_init = _pool_threshold_high;
	if(pool_threadhold_init <= 0) pool_threadhold_init = (_pool_threshold_low + _pool_threshold_high)/2;
	

	_player_max_count = atoi(conf->find(section,"player_capacity").c_str());
	_npc_max_count = atoi(conf->find(section,"npc_count").c_str());
	_matter_max_count = atoi(conf->find(section,"matter_count").c_str());
	if(_player_max_count > INS_MAX_PLAYER_COUNT)  _player_max_count = INS_MAX_PLAYER_COUNT;
	if(_npc_max_count > INS_MAX_NPC_COUNT) _npc_max_count = INS_MAX_NPC_COUNT;
	if(_matter_max_count > INS_MAX_MATTER_COUNT) _matter_max_count = INS_MAX_MATTER_COUNT;
	if( _player_max_count ==0 || _npc_max_count ==0 || _matter_max_count == 0)
	{
		__PRINTINFO("invalid argument player_count npc_count matter_count\n");
		return -1002;
	}

	int itime = atoi(conf->find(section,"idle_time").c_str());
	if(itime >= 30) _idle_time = itime;

	int ltime = atoi(conf->find(section,"life_time").c_str());
	if(ltime == -1 || ltime >= 300)			//限制副本寿命不能小于5分钟 
		_life_time = ltime;
	
	__PRINTINFO("idle_time %d\n",_idle_time);
	__PRINTINFO("life_time %d\n",_life_time);
	
	if(strcmp(conf->find(section,"owner_mode").c_str(),"single") == 0)
	{
		_owner_mode = OWNER_MODE_SINGLE;
		__PRINTINFO("副本所有人模式:单人\n");
	}
	else
	{
		_owner_mode = OWNER_MODE_TEAM;
		__PRINTINFO("副本所有人模式:队伍\n");
	}
	
	glb_verbose = 200;
	time_t ct = time(NULL);
	__PRINTINFO("%s\n",ctime(&ct));

	//初始化所有的管理器
	world_manager::Init();
	if(int irst = world_manager::InitBase(section.c_str()))
	{
		//初始化基础出错
		return irst;
	}
	
	//得到根目录
	std::string root = conf->find("Template","Root");

	//得到基础目录
	std::string base_path;
	base_path = root + conf->find(section,"base_path");
	__PRINTINFO("资源根目录:'%s'\n", base_path.c_str());
	
	//得到重新启动的操作
	_restart_shell  = base_path + conf->find("Template","RestartShell");

	//得到副本需求的cid
	if(!_cid.Init(conf->find(section,"cid").c_str()))
	{
		__PRINTINFO("错误的classid 在 'cid'项\n");
		return -1008;
	}

	//初始创建一个世界 
	_plane_template = CreateWorldTemplate();
	world & plane = *_plane_template;

	/*
	 *      初始化网格数据
	 *
	 */
	std::string str = conf->find(section,"grid");
	int row=800,column=800;
	float xstart=0.f,ystart=0.f,step=12.5f;
	sscanf(str.c_str(),"{%d,%d,%f,%f,%f}",&column,&row,&step,&xstart,&ystart);

	if(!plane.CreateGrid(row,column,step,xstart,ystart)){
		__PRINTINFO("Can not create world grid!\n");
		return -1;
	}

	rect rt = plane.GetGrid().grid_region;
	__PRINTINFO("Create grid: %d*%d with step %f\n",row,column,step);
	__PRINTINFO("Grid Region: {%.2f,%.2f} - {%.2f,%.2f}\n",rt.left,rt.top,rt.right,rt.bottom);
	
	rect local_rt,base_rt;
	str2rect(base_rt,conf->find(section,"base_region").c_str());
	str2rect(local_rt,conf->find(section,"local_region").c_str());
	if(!plane.GetGrid().SetRegion(local_rt,base_rt,100.f))
	{
		__PRINTINFO("配置文件中的区域数据不正确(base_region/local_region)\n");
		return -2;
	}

	str2rect(rt,conf->find(section,"inner_region").c_str());
	plane.GetGrid().inner_region = rt;
	float grid_sight_range = atof(conf->find(section, "grid_sight_range").c_str()); 
	if(grid_sight_range < 20.f || grid_sight_range > 100.f) grid_sight_range = 100.f;
	plane.BuildSliceMask((grid_sight_range>40.f ? 40.f : grid_sight_range), grid_sight_range); 

	rt = plane.GetGrid().local_region;
	__PRINTINFO("Local Region: {%.2f,%.2f} - {%.2f,%.2f}\n",rt.left,rt.top,rt.right,rt.bottom);
	rt = plane.GetGrid().inner_region;
	__PRINTINFO("Inner Region: {%.2f,%.2f} - {%.2f,%.2f}\n",rt.left,rt.top,rt.right,rt.bottom);
	if(rt.left > rt.right - 100.f || rt.top > rt.bottom - 100.f)
	{
		__PRINTINFO("内部区域过小\n");
		return -5;
	}

	//这里统计一下内存占用，如果超过了必要值，则不能进行
	unsigned long long mem_need = row*column*sizeof(slice);

	mem_need *= _pool_threshold_high;
	mem_need += _player_max_count * sizeof(gplayer);
	mem_need += _npc_max_count * sizeof(gnpc);
	mem_need += _matter_max_count * sizeof(gmatter);
	mem_need += instance_count * sizeof(world);

	unsigned long long mem_std = mem_need;
	mem_std += _player_max_count * sizeof(gplayer_imp);
	mem_std += (unsigned long long)(0.5*_npc_max_count *sizeof(gnpc_imp)); 
	mem_std += _matter_max_count * sizeof(gmatter_imp);

	unsigned long long mem_max = mem_need;
	mem_max += (instance_count - _pool_threshold_high)*row*column*sizeof(slice);
	mem_max += (unsigned long long)(0.5*_npc_max_count *sizeof(gnpc_imp)); 
	mem_max += (_player_max_count + _npc_max_count)*1536;

	size_t mem_now = GetMemTotal();
	float mem_ratio1 = ((mem_need/1024.f)/mem_now)*100.f;
	float mem_ratio2 = ((mem_std/1024.f)/mem_now)*100.f;
	float mem_ratio3 = ((mem_max/1024.f)/mem_now)*100.f;
	__PRINTINFO("总计%d个副本空间\n",instance_count);
	__PRINTINFO("预计内存需求:\n");
	__PRINTINFO("空状态  \t %dkB(%.2f%%)\n",(int)(mem_need/1024), mem_ratio1);
	__PRINTINFO("标准状态\t %dKb(%.2f%%)\n",(int)(mem_std/1024), mem_ratio2);
	__PRINTINFO("最大极限\t %dKb(%.2f%%)\n",(int)(mem_max/1024), mem_ratio3);
	
	if(mem_now * 0.6 < mem_std/1024)
	{
		__PRINTINFO("标准状态需求的内存超出了现有物理内存的阈值(need:%dmB/threshold:%dmB/total:%dmB)\n",(int)(mem_std/(1024*1024)), (int)(mem_now*0.6/1024), mem_now/1024);
		return -1001;
	}

	//初始化地图资源: 地形、寻路、碰撞、布怪图
	if(_mapres.Init(servername, base_path, plane.GetLocalWorld(), &plane) != 0)
	{
		__PRINTF("初始化地图资源失败\n");
		return -6;	
	}

	std::string  regionfile= base_path + conf->find("Template","RegionFile");
	std::string  regionfile2= base_path + conf->find("Template","RegionFile2");
	std::string pathfile = base_path + conf->find("Template","PathFile");
	//由于player_temp会修改配置文件，所以要先保存一下文件内容

	if(!player_template::Load("ptemplate.conf",&_dataman))
	{
		__PRINTINFO("can not load player template data from file template file or 'ptemplate.conf'\n");
		return -7;
	}

	//装载城市区域
	if(!player_template::LoadRegionData(regionfile.c_str(),regionfile2.c_str()))
	{
		__PRINTINFO("can not load city region data from file '%s'\n",regionfile.c_str());
		return -7;
	}

	player_template::GetRegionTime(_region_file_tag,_precinct_file_tag);
    //初始化每个区域内的传送点 
    world_manager::InitRegionWayPointMap();

	//读取路线文件
	if(!_pathman.Init(pathfile.c_str()))
	{
		__PRINTINFO("无法打开路线文件\n");
		return -9;
	}

	__PRINTINFO("开始创建副本世界\n");

	//生成激活世界列表
	_cur_planes.insert(_cur_planes.end(),instance_count,0);
	_planes_state.insert(_planes_state.end(),instance_count,0);

	//生成标准世界池
	for(int i = 0; i < pool_threadhold_init; i ++)
	{
		_planes_pool.push_back(new world);
		_plane_template->DuplicateWorld(_planes_pool[i]);
	}

	for(int i = 0; i < pool_threadhold_init; i ++)
	{
		int rst = _planes_pool[i]->RebuildMapRes();
		sleep(1);
		ASSERT(rst == 0);
		_planes_pool[i]->w_plane_index = -1;
		_mapres.BuildNpcGenerator(_planes_pool[i]);
	}
	
	_max_active_index = 0;

	/*
	 *       创建定时器线程 
	 */
	ONET::Thread::Pool::CreateThread(timer_thread);

	//初始化管理器的tick
	g_timer.set_timer(1,0,0,timer_tick,this);

	/**
	 *      初始化PollIO，创建相应的 Poll线程
	 */
	ONET::PollIO::Init();
	ONET::Thread::Pool::CreateThread(poll_thread);

	/*
	 *      初始化游戏服务器之间的连接池
	 */
	if(!InitNetIO(servername))
	{
		return -7;
	}

	/**
	 *      初始化世界消息队列 
	 */
	_msg_queue.Init();

	//这是最后一步，做完所有操作以后才开始连接所有服务器
	InitNetClient(gmconf_file);
	GLog::init();

	FinalInit(servername);

	trace_manager2::ReleaseElement();        
	return 0;
}

bool 
instance_world_manager::InitNetIO(const char * servername)
{
	_ioman.SetPlane(this);
	//从第一个世界取得网格
	grid & g = _plane_template->GetGrid();
	return _ioman.Init(servername,g.local_region,g.inner_region);
}

void 
instance_world_manager::SendRemoteMessage(int id, const MSG & msg)
{
	//这里需要加上消息过滤
	return _ioman.SendMessage(id,msg);
}

int  
instance_world_manager::BroadcastSvrMessage(const rect & rt,const MSG & message,float extend_size)
{
	//副本不需要进行服务器广播
	return 0;
}

int 
instance_world_manager::GetServerNear(const A3DVECTOR & pos) const
{
	//副本不存在Server near
	return -1;
}

int 
instance_world_manager::GetServerGlobal(const A3DVECTOR & pos) const
{
	return _ioman.GetGlobalServer(pos,_world_tag);
}


void 
instance_world_manager::RestartProcess()
{
	//考虑让所有人都断线 
	gplayer * pPool = GetPlayerPool();
	for(size_t i = 0; i < world_manager::GetMaxPlayerCount(); i ++)
	{
		if(pPool[i].IsEmpty()) continue;
		if(!pPool[i].imp) continue;
		int cs_index = pPool[i].cs_index;
		if(cs_index <=0) continue;
		GMSV::SendDisconnect(cs_index,pPool[i].ID.id,pPool[i].cs_sid,0);
	}

	if(!fork())
	{
		for(int i =3;i < getdtablesize(); i ++)
		{
			close(i);
		}
		sleep(1);
		system(_restart_shell.c_str());
	}
}

void 
instance_world_manager::ShutDown()
{
	size_t ins_count = _max_active_index;
	for(size_t i = 0; i < ins_count ; i ++)
	{
		if(_planes_state[i] == 0)
		{
			continue;
		}
		world * pPlane = _cur_planes[i];
		if(!pPlane) continue;
		if(pPlane->w_ctrl) pPlane->w_ctrl->OnServerShutDown();
	}

	world_manager::ShutDown();
}

gplayer* 
instance_world_manager::FindPlayer(int uid, int & world_index)
{	
	int widx = GetPlayerWorldIdx(uid);
	if(widx < 0) return NULL;
	world * pPlane = _cur_planes[widx];

	if(pPlane == NULL) return NULL;
	int index = pPlane->FindPlayer(uid);
	if(index < 0) return NULL;
	world_index = widx;
	return pPlane->GetPlayerByIndex(index);
}

int
instance_world_manager::SendRemotePlayerMsg(int uid, const MSG & msg)
{
	int w_idx = GetPlayerServerIdx(uid);
	if( w_idx >= 0)
	{
		//首先要隔断一些消息
		if(w_idx != _world_index)
		{
			//转发到外部服务器
			SendRemoteMessage(w_idx,msg);
		}
		else
		{
			//转发到世界的另外一个位面 ?			
			int widx = GetPlayerWorldIdx(uid);
			if(widx < 0) return -1;
			world * pPlane = _cur_planes[w_idx];
			if(pPlane) pPlane->PostLazyMessage(msg);
		}
	}
	else
	{
		__PRINTF("can not find extern player %d\n",uid);
	}
	return w_idx;
}

size_t
instance_world_manager::GetWorldCapacity()
{
	return _planes_capacity;
}

int 
instance_world_manager::GetOnlineUserNumber()
{
	return w_player_man.GetAllocedCount();
}

world * 
instance_world_manager::GetWorldByIndex(size_t index)
{
	return _cur_planes[index];
}

void 
instance_world_manager::GetPlayerCid(player_cid & cid)
{
	cid = _cid;
}

void 
instance_world_manager::FreeWorld(world * pPlane, int index)
{
	mutex_spinlock(&_key_lock);
	if(_planes_state[index] == 0 || _cur_planes[index] != pPlane 
			|| pPlane->w_player_count || pPlane->w_obsolete == 0 ) 
	{
		mutex_spinunlock(&_key_lock);
		return;
	}
	pPlane->w_obsolete = 0;
	pPlane->w_index_in_man = -1;
	pPlane->w_activestate = 2;
	_key_map.erase(pPlane->w_ins_key);
	pPlane->w_create_timestamp = -1;	//清除创建的时间戳
	_cur_planes[index] = 0;
	_planes_state[index] = 0;
	mutex_spinunlock(&_key_lock);

	mutex_spinlock(&_pool_lock);
	_active_plane_count --; 
	mutex_spinunlock(&_pool_lock);

	//重置世界
	pPlane->ResetWorld();
	//将世界放入冷却列表
	_planes_cooldown.push_back(pPlane);
			
	__PRINTF("将世界%d放入冷却池中\n",index);
}
void 
instance_world_manager::RegroupCoolDownWorld()
{
	if(size_t count = _planes_cooldown2.size())
	{
		//现在不进行将已经冷却的世界放入世界池的操作了，而是一律直接释放
		for(size_t i = 0; i < count; i ++)
		{
			__PRINTF("释放了世界%p\n",_planes_cooldown2[i]);
			_planes_cooldown2[i]->Release();
			delete _planes_cooldown2[i];
		}
		_planes_cooldown2.clear();
	}
	_planes_cooldown2.swap(_planes_cooldown);
}

void
instance_world_manager::FillWorldPool()
{
	mutex_spinlock(&_pool_lock);
	int pool_count = _planes_pool.size();
	int r1 = _pool_threshold_low - pool_count;
	int r2 =  _planes_capacity - (1 + pool_count +  (int)_planes_cooldown.size() + _active_plane_count);
	mutex_spinunlock(&_pool_lock);

	if(r1 > r2) r1 = r2;
	if(r1 > 0)
	{
		abase::vector<world*> list;
		list.reserve(r1);
		for(int i = 0; i < r1; i ++)
		{
			world * pPlane = new world;
			_plane_template->DuplicateWorld(pPlane);
			int rst = pPlane->RebuildMapRes();
			ASSERT(rst == 0);
			pPlane->w_plane_index = -1;
			_mapres.BuildNpcGenerator(pPlane);
			list.push_back(pPlane);
			__PRINTF("创建了世界%p\n",pPlane);
		}

		//将新产生的世界放入到世界池中
		mutex_spinlock(&_pool_lock);
		for(int i = 0; i < r1; i ++)
		{
			_planes_pool.push_back(list[i]);
		}
		mutex_spinunlock(&_pool_lock);

	}

}

void
instance_world_manager::Heartbeat()
{
	_msg_queue.OnTimer(0,100);
	world_manager::Heartbeat();
	size_t ins_count = _max_active_index;
	for(size_t i = 0; i < ins_count ; i ++)
	{
		if(_planes_state[i] == 0)
		{
			continue;
		}
		world * pPlane = _cur_planes[i];
		if(!pPlane) continue;
		pPlane->RunTick();
	}

	mutex_spinlock(&_heartbeat_lock);
	
	if((++_heartbeat_counter) > TICK_PER_SEC*HEARTBEAT_CHECK_INTERVAL)
	{
		//每10秒检验一次
		for(size_t i = 0; i < ins_count ; i ++)
		{
			if(_planes_state[i] == 0) continue;	//空世界
			world * pPlane = _cur_planes[i];
			if(!pPlane) continue;
			if(pPlane->w_obsolete)
			{
				//处于等待废除状态
				if(pPlane->w_player_count)
				{
					pPlane->w_obsolete = 0;
				}
				else
				{
					if((pPlane->w_obsolete += HEARTBEAT_CHECK_INTERVAL) > _idle_time)  // 20 * 60
					{
						//没有玩家并且时间超时了，则释放世界
						FreeWorld(pPlane,i);
					}
				}
			}
			else
			{
				if(!pPlane->w_player_count)
				{
					pPlane->w_obsolete = 1;
				}
			}
			
			//进行副本寿命检查
			if(pPlane->w_life_time > 0)
			{
				pPlane->w_life_time -= HEARTBEAT_CHECK_INTERVAL;
				if(pPlane->w_life_time < 0)
					pPlane->w_life_time = 0;
			}			
		}
		_heartbeat_counter = 0;

		//进行冷却列表的处理
		RegroupCoolDownWorld();
	}

	if((++_heartbeat_counter2) > TICK_PER_SEC*HEARTBEAT_CHECK_INTERVAL)
	{
		//如果世界池的容量不足，则进行重新创建处理
		FillWorldPool();

		_heartbeat_counter2 = 0;
	}

	mutex_spinunlock(&_heartbeat_lock);
}

bool 
instance_world_manager::CompareInsKey(const instance_key & key, const instance_hash_key & hkey)
{
	instance_hash_key key2;
	TransformInstanceKey(key.essence,key2);
	return key2 == hkey;
}

int instance_world_manager::GetInstanceReenterTimeout(world* plane)
{
	if(_world_limit.can_reenter && (plane->w_life_time<0 || plane->w_life_time>INSTANCE_REENTER_INTERVAL)
		&& (!plane->w_obsolete || _idle_time - plane->w_obsolete >= INSTANCE_REENTER_INTERVAL))
		return g_timer.get_systime() + INSTANCE_REENTER_INTERVAL;
	else
		return 0;
}

int
instance_world_manager::CheckPlayerSwitchRequest(const XID & who, const instance_key * ikey,const A3DVECTOR & pos,int ins_timer)
{
	int rst = 0;
	//首先检查key是否存在
	instance_hash_key key;
	TransformInstanceKey(ikey->target,key);
	world *pPlane = NULL;
	mutex_spinlock(&_key_lock);
	int * pTmp = _key_map.nGet(key);
	if(!pTmp)
	{
		//世界不存在， 判断是否有空闲的world等待分配，如果没有则通知玩家
		bool bRst = _planes_pool.size() >= 1;
		mutex_spinunlock(&_key_lock);
		if(!bRst || (_active_plane_count + 1) >= _planes_capacity)
		{
			rst = S2C::ERR_TOO_MANY_INSTANCE;
		}
		else
		{
			if(ins_timer == 0 || (ikey->special_mask & IKSM_REENTER)) // 无法重置
			{
				rst = S2C::ERR_CAN_NOT_RESET_INSTANCE;
			}
		}
		return rst;
	}
	pPlane = _cur_planes[*pTmp];
	mutex_spinunlock(&_key_lock);
	//检查玩家的人数是否匹配
	if(pPlane)
	{
		return ((ikey->special_mask & IKSM_GM) || pPlane->w_player_count < _player_limit_per_instance)?0:S2C::ERR_TOO_MANY_PLAYER_IN_INSTANCE;
	}
	else
	{
		return S2C::ERR_CANNOT_ENTER_INSTANCE;
	}
}

world * 
instance_world_manager::AllocWorld(const instance_hash_key & key, int & world_index,int ctrl_id)
{
	spin_autolock keeper(_key_lock);
	return AllocWorldWithoutLock(key,world_index,ctrl_id);
}

world * 
instance_world_manager::AllocWorldWithoutLock(const instance_hash_key &key, int & world_index, int ctrl_id)
{
	//虽然是without lock 但是本阶段必须保持加锁
	ASSERT(_key_lock);
	//首先查询或者分配一个世界
	world *pPlane = NULL;
	int * pTmp = _key_map.nGet(key);
	world_index = -1;
	if(!pTmp)
	{
		if(!_planes_pool.size()) 
		{
			return NULL;
		}
		
		//这是开启副本（新世界）
		//检测是否需要key
		bool need_key = world_manager::GetWorldLimit().ctrlid_open_instance;
		if(need_key && ctrl_id <= 0)
		{
			return NULL;
		}
		//可以开启
		mutex_spinlock(&_pool_lock);
		if(_planes_pool.size())
		{
			pPlane = _planes_pool.back();
			_planes_pool.pop_back();
			_active_plane_count ++;
		}
		mutex_spinunlock(&_pool_lock);
		if(pPlane)
		{
			pPlane->w_destroy_timestamp = g_timer.get_systime() + 120;//防止根据destroy_time错误的释放

			//寻找可以放置世界的空位
			size_t i = 1;	
			for(; i < (size_t)_planes_capacity; i ++)
			{
				if(_cur_planes[i]) continue;
				_cur_planes[i] = pPlane;
				_planes_state[i]  = 1; 
				pPlane->w_index_in_man = i;
				pPlane->w_plane_index = i;
				pPlane->w_activestate = 1;
				world_index = i;
				break;
			}
			if(i != (size_t)_planes_capacity)
			{
				__PRINTF("分配了世界%d %p\n",i,pPlane);
				_key_map.put(key,i);
				pPlane->w_ins_key = key;
				pPlane->w_obsolete = 0;
				pPlane->w_create_timestamp = time(NULL);
				if((size_t) _max_active_index < i +1)
				{
					_max_active_index = i + 1;
				}
			}
			else
			{
			//	ASSERT(false);

				//无法加入世界，重新将生成的世界放入到世界池中
				mutex_spinlock(&_pool_lock);
				_planes_pool.push_back(pPlane);
				_active_plane_count --; 
				mutex_spinunlock(&_pool_lock);
				return NULL;
			}
			if(need_key)
			{
				bool bRst = pPlane->TriggerSpawn(ctrl_id);
				__PRINTF("开启副本时同时激活了控制器%d %d\n",ctrl_id, bRst?1:0);
			}
		}
	}
	else
	{
		//存在这样的世界 
		world_index = *pTmp;;
		pPlane = _cur_planes[world_index];
		ASSERT(pPlane);
		pPlane->w_obsolete = 0;
	}
	if(world_index < 0) return NULL;
	return pPlane;
}

world * 
instance_world_manager::GetWorldInSwitch(const instance_hash_key & ikey,int & world_index,int ctrl_id)
{
	return AllocWorld(ikey,world_index,ctrl_id);
	
}

world * 
instance_world_manager::GetWorldOnLogin(const instance_hash_key & ikey,int & world_index)
{
	return AllocWorld(ikey,world_index,0);
}

void 
instance_world_manager::HandleSwitchRequest(int link_id,int user_id,int localsid,int source,const instance_key &ikey)
{
	instance_hash_key key;
	TransformInstanceKey(ikey.target,key);
	int world_index;
	world * pPlane = GetWorldInSwitch(key,world_index,ikey.control_id);
	if(!pPlane ) 
	{
		//无法创建世界，副本数目达到最大上限 
		MSG msg;
		int error = S2C::ERR_TOO_MANY_INSTANCE;
		if(world_manager::GetWorldLimit().ctrlid_open_instance)
		{
			error = S2C::ERR_CANNOT_ENTER_INSTANCE;
		}
		BuildMessage(msg,GM_MSG_SWITCH_FAILED,XID(GM_TYPE_PLAYER,user_id),XID(0,0),A3DVECTOR(0,0,0),error);
		SendRemotePlayerMsg(user_id,msg);
		return;
	}

	ASSERT(pPlane == _cur_planes[world_index]);
	int index1 = pPlane->FindPlayer(user_id);
	if(index1 >= 0) 
	{
		//这个用户不应该存在的(但是由于时许似乎也有可能)
		//如果link和game断开，那么这种情况可能会发生
		return;
	}
	gplayer * pPlayer = pPlane->AllocPlayer();
	if(pPlayer == NULL)
	{
		//发送没有物理空间可以容纳Player的信息
		__PRINTF("用户达到物理最大限制值\n");
		GLog::log(GLOG_ERR,"副本%d用户数目达到最大上限，当用户%d转移服务器时",GetWorldTag(),user_id);
		//不发送转移消息，这样玩家会因为超时而断线
		return;
	}

	__PRINTF("player %d switch from %d\n",user_id,source );
	pPlayer->cs_sid = localsid;
	pPlayer->cs_index = link_id;
	pPlayer->ID.id = user_id;
	pPlayer->ID.type = GM_TYPE_PLAYER;
	pPlayer->login_state = gplayer::WAITING_SWITCH;
	pPlayer->pPiece = NULL;
	if(!pPlane->MapPlayer(user_id,pPlane->GetPlayerIndex(pPlayer)))
	{	
		__PRINTF("服务器转移时map player失败\n");
		GLog::log(GLOG_ERR,"副本%d在执行服务器转移时map player失败，用户%d",GetWorldTag(),user_id);

		//可能是由于时序使得后来的消息先处理，从而进行的重复的创建 
		pPlane->FreePlayer(pPlayer);
		pPlayer->Unlock();
		return;
	}

	//将这个用户设置为在这个位面
	SetPlayerWorldIdx(user_id,world_index);
	
	class switch_task : public ONET::Thread::Runnable, public abase::timer_task , public abase::ASmallObject
	{
		gplayer *_player;
		int _userid;
		world * _plane;
		instance_world_manager * _manager;
		public:
			switch_task(gplayer * pPlayer,world * pPlane,instance_world_manager * manager)
				:_player(pPlayer),_userid(pPlayer->ID.id),_plane(pPlane),_manager(manager)
			{
				//这个超时时间由2.5s延长至5s，防止gs负载高引起的超时，modify by liuguichen 20131224
				SetTimer(g_timer,TICK_PER_SEC*5,1);
				__PRINTF("timer %p %d\n",this,_timer_index);
			}
		public:
			virtual void OnTimer(int index,int rtimes)
			{
				ONET::Thread::Pool::AddTask(this);
			}

			virtual void Run()
			{
				spin_autolock keeper(_player->spinlock);
				if(_player->IsActived() && _player->ID.id == _userid && _player->login_state == gplayer::WAITING_SWITCH)
				{
					_plane->UnmapPlayer(_userid);
					_plane->FreePlayer(_player);

					//同时取消在这个位面的记录  （由于时序，这里可能会有些问题）$$$$$$$$
					//要再考虑一下
					_manager->RemovePlayerWorldIdx(_userid);
				}
				delete this;
			}
	};
	//发出等待消息
	MSG msg;
	BuildMessage(msg,GM_MSG_SWITCH_GET,pPlayer->ID,XID(GM_TYPE_SERVER,world_manager::GetWorldIndex()),A3DVECTOR(0,0,0),_world_tag, &ikey,sizeof(ikey));
	pPlane->SendRemoteMessage(source,msg);

	//设置超时
	switch_task *pTask = new switch_task(pPlayer,pPlane,this);
	pPlayer->base_info.race = (int)(abase::timer_task*)pTask;
	pPlayer->base_info.faction = pTask->GetTimerIndex();
	pPlayer->Unlock();
	
}


void 
instance_world_manager::PlayerLeaveThisWorld(int plane_index, int userid)
{
	RemovePlayerWorldIdx(userid,plane_index);
}


void 
instance_world_manager::PostMessage(world * plane, const MSG & msg)
{
	_msg_queue.AddMsg(plane,msg);
}

void 
instance_world_manager::PostMessage(world * plane, const MSG & msg,int latancy)
{
	_msg_queue.AddMsg(plane,msg,latancy);
}

void 
instance_world_manager::PostMessage(world * plane, const XID * first, const XID * last, const MSG & msg)
{
	_msg_queue.AddMultiMsg(plane,first,last,msg);
}

void 
instance_world_manager::PostPlayerMessage(world * plane, int * player_list, size_t count, const MSG & msg)
{
	_msg_queue.AddPlayerMultiMsg(plane,count, player_list,msg);
}

void
instance_world_manager::PostMultiMessage(world * plane,abase::vector<gobject*,abase::fast_alloc<> > &list, const MSG & msg)
{
	_msg_queue.AddMultiMsg(plane,list, msg);
}

void 
instance_world_manager::GetLogoutPos(gplayer_imp * pImp, int & world_tag ,A3DVECTOR & pos)
{
	if(_world_limit.savepoint && _save_point.tag > 0)
	{
		world_tag = _save_point.tag;
		pos = _save_point.pos;
	}
	else
	{
		pos = pImp->GetLogoutPos(world_tag);
	}
}

void instance_world_manager::SwitchServerCancel(int link_id,int user_id, int localsid)
{
	int index1;
	gplayer * pPlayer = FindPlayer(user_id,index1);
	if(!pPlayer)
	{
		ASSERT(false);
		//没有找到 合适的用户
		//正常情况下，这个用户应该存在的
		return;
	}
	spin_autolock keeper(pPlayer->spinlock);

	if(pPlayer->ID.id != user_id || !pPlayer->IsActived() || !pPlayer->imp)
	{
		ASSERT(false);
		return;
	}
	pPlayer->imp->CancelSwitch();
}

void	instance_user_login(int cs_index,int cs_sid,int uid,const void * auth_data, size_t auth_size, bool isshielduser, char flag);
void 
instance_world_manager::UserLogin(int cs_index,int cs_sid,int uid,const void * auth_data,size_t auth_size,bool isshielduser, char flag)
{
	instance_user_login(cs_index,cs_sid,uid,auth_data,auth_size,isshielduser,flag);
}

void 
instance_world_manager::SetFilterWhenLogin(gplayer_imp * pImp, instance_key * ikey)
{
	//加入副本key检测filter,这个filter在切换服务器时不会进行保存和恢复
	pImp->_filters.AddFilter(new aei_filter(pImp,FILTER_CHECK_INSTANCE_KEY));
}

bool instance_world_manager::IsUniqueWorld()
{
	return false;
}

void 
instance_world_manager::SetIncomingPlayerPos(gplayer * pPlayer, const A3DVECTOR & pos,int special_mask)
{
	if( (0 == (special_mask & IKSM_REENTER)) &&
		pPlayer->plane->SetIncomingPlayerPos(pPlayer,pos))	return;
	
	pPlayer->pos = pos;

	//对pos的高度进行修正
	float height = pPlayer->plane->GetHeightAt(pos.x,pos.z);
	if(pPlayer->pos.y < height) pPlayer->pos.y = height;
}

bool 
instance_world_manager::GetTownPosition(gplayer_imp *pImp, const A3DVECTOR &opos, A3DVECTOR &pos, int & world_tag)
{
	return pImp->_plane->GetTownPosition(pImp,opos,pos,world_tag) ||
		   world_manager::GetTownPosition(pImp,opos,pos,world_tag);
}
bool 
instance_world_manager::ClearSpawn(int sid)
{
	size_t ins_count = _max_active_index;
	for(size_t i = 0; i < ins_count; i++)
	{
		if(_planes_state[i] == 0)
		{
			continue;
		}
		world *pPlane = _cur_planes[i];
		if(!pPlane) continue;
		pPlane->ClearSpawn(sid);
	}
	return true;
}

/*---------------------------------   帮派副本的内容 ---------------------------------*/


bool 
faction_world_manager::FactionLogin(const instance_hash_key &hkey,const GNET::faction_fortress_data * data,const GNET::faction_fortress_data2 * data2)
{
	//首先取得或者创建一个世界 
	mutex_spinlock(&_key_lock);
	//这里必须全程保持加锁以确定状态正确，由于此种操作并不常见，全程加锁应该不会带来太大的冲突 
	//如果不进行此类锁定的话，wait list 恐怕无法保持正确状态

	//获取帮派基地数据失败或帮派基地不存在
	if(data == NULL)
	{
		ClearWaitList(hkey, S2C::ERR_CANNOT_ENTER_INSTANCE);
		mutex_spinunlock(&_key_lock);
		return false;
	}
	
	//开始分配世界,这里世界的分配方式要有所不同才对(或者考虑所有NPC都创建出来,再根据不同的情况来区分是否让某些NPC消失等) 
	int world_index;
	world * pPlane = AllocWorldWithoutLock(hkey,world_index);

	if(pPlane == NULL)
	{
		//创建世界失败， 所以无法进入游戏， 需要清空WaitList并发送无法进入的消息
		ClearWaitList(hkey, S2C::ERR_CANNOT_ENTER_INSTANCE);
		mutex_spinunlock(&_key_lock);
		return false;
	}

	//依次发送请求给各个玩家
	bool verify_key = false;	// 是否拒绝非本帮派的人进入
	bool bRst = SendReplyToWaitList(hkey,verify_key);
	if(!bRst)
	{
		//不可能出现这种情况
		ASSERT(false);
		mutex_spinunlock(&_key_lock);
		return false;
	}
	mutex_spinunlock(&_key_lock);
	
	//以消息的形式通知各个生成器, 让此生成器可以正确的清除或者加入某个NPC
	faction_world_ctrl * pCtrl = dynamic_cast<faction_world_ctrl*>(pPlane->w_ctrl);
	if(pCtrl == NULL)
	{
		//世界内部的ctrl不是合法的
		ASSERT(false);
		return false;
	}
	pCtrl->Init(pPlane,data,data2);
	__PRINTF("Faction Login. factionid=%d world_index=%d world=%p\n",data->factionid,world_index,pPlane);
	return true;
}

bool 
faction_world_manager::NotifyFactionData(GNET::faction_fortress_data2 * data2)
{
	instance_hash_key hkey;
	MakeInstanceKey(data2->factionid,hkey);
	mutex_spinlock(&_key_lock);
	int * pTmp = _key_map.nGet(hkey);
	if(pTmp)
	{
		int world_index = *pTmp;
		world * pPlane = _cur_planes[world_index];
		ASSERT(pPlane);
		mutex_spinunlock(&_key_lock);
		faction_world_ctrl * pCtrl = dynamic_cast<faction_world_ctrl*>(pPlane->w_ctrl);
		if(pCtrl == NULL)
		{
			ASSERT(false);
			return false;
		}
		pCtrl->OnNotifyData(pPlane,data2);
		return true;
	}
	mutex_spinunlock(&_key_lock);
	return false;
}

bool
faction_world_manager::SendReplyToWaitList(const instance_hash_key & hkey , bool is_verify)
{
	bool bRst = false;
	//进入临界区
	spin_autolock keeper(_wait_queue_lock);
	
	//找到指定的等待队列
	WAIT_MAP::iterator it = _wait_queue.find(hkey);

	if(it == _wait_queue.end())
	{
		
		//如果指定的队列不存在 则什么也不做
		return false;
	}
	else
	{
		//给所有玩家发送回馈消息
		MSG msg;
		BuildMessage(msg,GM_MSG_PLANE_SWITCH_REPLY,XID(-1,-1),XID(GM_TYPE_SERVER,GetWorldIndex()),
				A3DVECTOR(0,0,0),0);

		WAIT_ENTRY & v = **(it.value());
		for(size_t i = 0; i < v.size(); i ++)
		{
			if(is_verify)
			{
				instance_hash_key okey; 
				TransformInstanceKey(v[i].second.essence, okey);
				if(!(okey == hkey))
				{
					MSG nmsg;
					BuildMessage(nmsg,GM_MSG_ERROR_MESSAGE,v[i].first,XID(0,0),A3DVECTOR(0,0,0),S2C::ERR_FACTION_BASE_DENIED);
					SendRemotePlayerMsg(msg.target.id, msg);
					continue;
				}
			}
			msg.target = v[i].first;
			msg.content = &(v[i].second);
			msg.pos = v[i].pos;
			msg.content_length = sizeof(instance_key);
			SendRemotePlayerMsg(msg.target.id, msg);
			bRst = true;
		}
		delete *(it.value());
		_wait_queue.erase(it);
	}
	return bRst;

}

void 
faction_world_manager::ClearWaitList(const instance_hash_key & hkey,int err_code)
{
	//进入临界区
	spin_autolock keeper(_wait_queue_lock);
	
	//找到指定的等待队列
	WAIT_MAP::iterator it = _wait_queue.find(hkey);

	if(it == _wait_queue.end())
	{
		
		//如果指定的队列不存在 则什么也不做
		return;
	}
	else
	{
		//给所有玩家发送错误消息
		if(err_code != 0)
		{
			MSG msg;
			BuildMessage(msg,GM_MSG_ERROR_MESSAGE,XID(-1,-1),XID(0,0),A3DVECTOR(0,0,0),err_code);
			
			WAIT_ENTRY & v = **(it.value());
			for(size_t i = 0; i < v.size(); i ++)
			{
				msg.target = v[i].first;
				SendRemotePlayerMsg(msg.target.id, msg);
			}
		}
		delete *(it.value());
		_wait_queue.erase(it);
	}
	return ;
}

bool 
faction_world_manager::AddWaitList(const XID & who, const instance_hash_key & hkey, const instance_key & ikey,const A3DVECTOR & pos)
{
	class GetFactionFortress : public GNET::FactionFortressResult
	{
		instance_hash_key _hkey;
	public:
		GetFactionFortress(const instance_hash_key & hkey):_hkey(hkey)
		{}

		virtual void OnTimeOut()
		{
			OnGetData(NULL,NULL);
		}
		virtual void OnFailed()
		{
			OnGetData(NULL,NULL);
		}
		virtual void OnGetData(const GNET::faction_fortress_data * data,const GNET::faction_fortress_data2 * data2)
		{
			//传送一下数据
			world_manager::GetInstance()->FactionLogin(_hkey,data,data2);
			delete this;
		}
	};
	//检查副本容量和等待队列中的容量和是否到达上限 还未检查

	//进入临界区
	spin_autolock keeper(_wait_queue_lock);
	
	//找到指定的等待队列
	WAIT_MAP::iterator it = _wait_queue.find(hkey);

	if(it == _wait_queue.end())
	{
		//如果指定的队列不存在 将玩家加入队列中,并发出数据库读取请求
		WAIT_ENTRY *v = new WAIT_ENTRY;
		v->push_back(wait_node(who,ikey,pos));
		_wait_queue.put(hkey,v);

		GNET::get_faction_fortress(ikey.target.key_level3, new GetFactionFortress(hkey));
	}
	else
	{
		//如果指定的队列存在 检查指定的玩家是否存在,如果存在 那么忽略本次操作
		WAIT_ENTRY & v = **(it.value());
		for(size_t i = 0; i < v.size(); i ++)
		{
			if(v[i].first == who) return true;
		}
		v.push_back(wait_node(who,ikey,pos));
	}
	return true;
}

world * 
faction_world_manager::GetWorldInSwitch(const instance_hash_key & ikey,int & world_index,int ctrl_id)
{
	spin_autolock keeper(_key_lock);
	world *pPlane = NULL;
	int * pTmp = _key_map.nGet(ikey);
	world_index = -1;
	if(pTmp)
	{
		//存在这样的世界 
		world_index = *pTmp;;
		pPlane = _cur_planes[world_index];
		ASSERT(pPlane);
		pPlane->w_obsolete = 0;
	}
	if(world_index < 0) return NULL;
	return pPlane;
	
}

int 
faction_world_manager::CheckPlayerSwitchRequest(const XID & who, const instance_key * ikey,const A3DVECTOR & pos,int ins_timer)
{
	//检查是否去正确的副本
	if(ikey->target.key_level3 == 0 || ikey->essence.key_level3 == 0)
	{
		return S2C::ERR_CANNOT_ENTER_INSTANCE;
	}
	int factionid = ikey->essence.key_level3;
	
//检查帮派副本传送规则
//首先检查Key是否存在
	instance_hash_key key;
	TransformInstanceKey(ikey->target,key);
	world *pPlane = NULL;
	mutex_spinlock(&_key_lock);
	int * pTmp = _key_map.nGet(key);
	if(!pTmp)
	{
		//世界不存在，加入待等待列表
		//考虑设计非本帮派人员开启副本的情况
		int rst = -1;
		//if(ikey->target.key_level3 == ikey->essence.key_level3)
		//{
			bool bRst = AddWaitList(who,key,*ikey,pos);
			if(!bRst)
			{
				rst = S2C::ERR_TOO_MANY_INSTANCE;
			}
		//}
		//else 
		//{
			//应该是此帮派基地未开启
			//现在不允许非本帮成员开启帮派基地
		//	rst = S2C::ERR_FACTION_BASE_NOT_READY;
		//}
		mutex_spinunlock(&_key_lock);
		return rst;
	}
	pPlane = _cur_planes[*pTmp];
	mutex_spinunlock(&_key_lock);

	//检查玩家的人数是否匹配
	int rst = 0;
	if(pPlane)
	{
		if(!(ikey->special_mask & IKSM_GM) && pPlane->w_player_count >= _player_limit_per_instance)
		{
			rst = S2C::ERR_TOO_MANY_PLAYER_IN_INSTANCE;
		}
		else
		{
			faction_world_ctrl* pCtrl = (faction_world_ctrl*)pPlane->w_ctrl;
			
			if(factionid == pCtrl->factionid)
			{
				if(pCtrl->defender_count >= pCtrl->player_count_limit)
				{
					rst = S2C::ERR_TOO_MANY_PLAYER_IN_INSTANCE;	
				}
			}
			else if(pCtrl->inbattle && factionid == pCtrl->offense_faction)
			{
				if(pCtrl->attacker_count >= pCtrl->player_count_limit)
				{
					rst = S2C::ERR_TOO_MANY_PLAYER_IN_INSTANCE;	
				}
			}
			else
			{
				rst = S2C::ERR_FACTION_IS_NOT_MATCH;
			}
			//基地是不是处于踢人状态
			if(!rst && pCtrl->iskick)
			{
				rst = S2C::ERR_FACTION_FORTRESS_ISKICK;
			}
		}
	}
	else
	{
		rst = S2C::ERR_CANNOT_ENTER_INSTANCE;
	}
	return rst;
}

void 
faction_world_manager::GetLogoutPos(gplayer_imp * pImp, int & world_tag ,A3DVECTOR & pos)
{
	//这里应该用动态的savepoint 创建世界时需要指定这些数据
	pImp->GetLastInstanceSourcePos(world_tag,pos);
	if(world_tag != 1)
	{
		//这样真的好？ 不过也没办法， 不然出错了怎么办？
		world_tag = 1;
		pos = A3DVECTOR(320,0,3200);
	}
}

void
faction_world_manager::UserLogin(int cs_index,int cs_sid,int uid,const void * auth_data, size_t auth_size,bool isshielduser, char flag)
{
	//帮派基地无法直接登陆
	GMSV::SendLoginRe(cs_index,uid,cs_sid,3,flag);       // login failed
}

void 
faction_world_manager::SetFilterWhenLogin(gplayer_imp * pImp, instance_key * ikey)
{
	pImp->_filters.AddFilter(new aeff_filter(pImp,FILTER_CHECK_INSTANCE_KEY));
}

void 
faction_world_manager::SetIncomingPlayerPos(gplayer * pPlayer, const A3DVECTOR & origin_pos, int special_mask)
{
	world * pPlane = pPlayer->imp->_plane;
	faction_world_ctrl * pCtrl = (faction_world_ctrl *)pPlane->w_ctrl;
	
	if(pPlayer->id_mafia == pCtrl->factionid)
	{
		//简单起见，写死
		pPlayer->pos = A3DVECTOR(-383.295f,35.f,1929.511f);
	}
	else if(pCtrl->inbattle && pPlayer->id_mafia == pCtrl->offense_faction)
	{
		//简单起见，写死
		pPlayer->pos = A3DVECTOR(-188.f,35.f,400.f);
	}
	else
	{
		instance_world_manager::SetIncomingPlayerPos(pPlayer,origin_pos,special_mask);	
	}
}

bool 
faction_world_manager::GetTownPosition(gplayer_imp *pImp, const A3DVECTOR &opos, A3DVECTOR &pos, int & tag)
{
	gplayer * pPlayer = (gplayer *)pImp->_parent;
	if(pPlayer->IsBattleDefence())
	{
		//简单起见，写死
		pos = A3DVECTOR(-383.295f,35.f,1929.511f);
		tag = _world_tag;
		return true;
	}
	else if(pPlayer->IsBattleOffense())
	{
		//简单起见，写死
		pos = A3DVECTOR(-188.f,35.f,400.f);
		tag = _world_tag;
		return true;
	}
	return false;
}

void 
faction_world_manager::OnDeliveryConnected()
{
	GNET::SendFactionServerRegister(GetWorldIndex(),GetWorldTag());
	GMSV::SendMafiaPvPRegister(GetWorldIndex(),GetWorldTag());
}

void faction_world_manager::OnMafiaPvPStatusNotice(int status,std::vector<int>& ctrl_list)
{
	if(status)
	{
		world_flags& flags = GetWorldFlag();
		flags.mafia_pvp_flag = true;
		flags.nonpenalty_pvp_flag = true;
	}
	else
	{
		world_flags& flags = GetWorldFlag();
		flags.mafia_pvp_flag = false;
		flags.nonpenalty_pvp_flag = false;
	}
}

world * 
faction_world_manager::CreateWorldTemplate()
{
	world * pPlane  = new world;
	pPlane->Init(_world_index);
	pPlane->InitManager(this);

	pPlane->SetWorldCtrl(new faction_world_ctrl());
	return pPlane;
}

world_message_handler * 
faction_world_manager::CreateMessageHandler()
{
	return new faction_world_message_handler(this);
}

int 
faction_world_manager::OnMobDeath(world * pPlane, int faction,  int tid)
{
	npc_template * pTemplate = npc_stubs_manager::Get(tid);
	if(!pTemplate || !pTemplate->faction_building_id) return 0;
	
	faction_world_ctrl * pCtrl = (faction_world_ctrl *)pPlane->w_ctrl;
	pCtrl->OnBuildingDestroyed(pPlane, pTemplate->faction_building_id);	
	return 1;
}
