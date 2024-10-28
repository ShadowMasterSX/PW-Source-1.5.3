#ifndef __NETGAME_GS_NPC_GENERATOR_H__
#define __NETGAME_GS_NPC_GENERATOR_H__

#include <hashtab.h>
#include <timer.h>
#include <threadpool.h>
#include <arandomgen.h>
#include <common/types.h>
#include <glog.h>

#include "staticmap.h"
#include "property.h"

class gplayer_imp;
struct pet_data;
struct npc_template
{
	int tid;
	int addon_choice_count;
	unsigned int	id_addon[16];
	float	probability_addon[16];

	struct  __mine_info
	{
		int is_mine;			//如果这个值不为零，表示这是一个matter
		int std_amount;			//标准数量 
		int bonus_amount;		//附加后的数量
		float bonus_prop;		//使用附加数量的概率
		int time_min;			//采集时间下限
		int time_max;			//采集时间上限
		unsigned int produce_kinds;
		unsigned int id_produce[16];
		float id_produce_prop[16];
		int id_produce_life[16];
		int need_equipment;
		int level;
		int exp;
		int sp;
		int task_in;
		int task_out;
		bool no_interrupted;
		bool gather_no_disappear;
		bool eliminate_tool;
		int   ask_help_faction;
		float ask_help_range;
		int   ask_help_aggro;
		int set_owner;
		float gather_dist;
		int life;
		int gather_player_max;
		int mine_type;
		float success_prob;
		int broadcast_on_gain;	//采集成功的话广播给周围
		struct 
		{
			int id_monster;
			int num;
			float radius;
			int remain_time;
		} monster_list[4];
	} mine_info;

	unsigned int faction;			
	unsigned int enemy_faction;		
	unsigned int monster_faction;		//怪物的小派系，用于求救
	unsigned int id_strategy;
	unsigned int inhabit_type;		
	unsigned int inhabit_mode;		//换算成了pathfinding的模式	
	int role_in_war;			//城战中的角色
	int immune_type;
	float body_size;
	float sight_range;
	unsigned int 	aggressive_mode;
	unsigned int	monster_faction_ask_help;
	unsigned int	monster_faction_can_help;
	int petegg_id;
	int short_range_mode;	//是否远程攻击
	float aggro_range;
	int aggro_time;
	int damage_delay;
	int trigger_policy;
	struct condition_skill
	{
		int skill;
		int level;
		float prob;
	};
	condition_skill skill_hp75[5];
	condition_skill skill_hp50[5];
	condition_skill skill_hp25[5];
	unsigned int after_death;
	int drop_no_protected;	//掉落物品不保护
	int drop_no_profit_limit;//掉落不受收益时间影响
	float drop_mine_prob;	//掉落矿物的概率
	int drop_mine_group;	//掉落矿物的分组
	int patrol_mode;
	int normal_heartbeat_in_idle;		//在周围无人是也用正常速度的心跳
	float max_move_range;	//最大移动范围
	
	struct
	{
		int as_count;
		int bs_count;
		int cs_count;
		struct 
		{
			int id;
			int level;
		} attack_skills[8];
		
		struct 
		{
			int id;
			int level;
		} bless_skills[8];

		struct
		{
			int id;
			int level;
		} curse_skills[8];

	}skills;

	int 		aggro_strategy_count;
	unsigned int	aggro_strategy_ids[4];
	float		aggro_strategy_probs[4];	

	basic_prop bp;
	extend_prop ep;
	int		hp_adjust_common_value;			//	血量调整全局变量ID
	int		defence_adjust_common_value;	//	防御调整全局变量ID
	int		attack_adjust_common_value;		//	攻击调整全局变量ID
	int		attack_degree;
	int		defend_degree;
	int		invisible_degree;
	int		anti_invisible_degree;
	int 		no_accept_player_buff;
	int 	no_auto_fight;		//不可进入战斗状态
	int 	fixed_direction;	//朝向不变
	int 	faction_building_id;//帮派基地建筑id
	int		has_collision;		//是否参与服碰撞检测
	int		set_owner;			//是否在出生时设置所有者
	bool	record_dps_rank;	//被玩家杀死后是否记入dps排行榜,set_owner非0时才有效	
	unsigned int domain_related; // 领土相关
	int		local_var[3];		//npc拥有的特征值 	

	//下面是功能npc的内容
	struct npc_statement
	{
		enum 
		{
			NPC_TYPE_NORMAL,
			NPC_TYPE_GUARD,
		};

		int is_npc;			//此值非零,表示是一个NPC
		int src_monster;
		int refresh_time;
		int attack_rule;
		int faction;		//可能不需要 
		float tax_rate;
		int life;		//可能不需要
		int npc_type;		//npc 类型
		int need_domain;	//需要帮派地盘才可以进行处理
		bool serve_distance_unlimited;	//服务距离无限制

		//各种服务的代码
		
		int  service_sell_num;
		int  service_sell_goods[1536];

		int  service_transmit_target_num;
		struct  __st_ent
		{
			int 		world_tag;
		        A3DVECTOR      	target;
			size_t          fee;
			int 		require_level;
			int		target_waypoint;
		}transmit_entry[32];		//这个定义必须与serviceprovider里面的一致

		int service_task_in_num;
		int service_task_in_list[256];

		int service_task_out_num;
		int service_task_out_list[258];

		int service_task_matter_num;
		int service_task_matter_list[32];

		int service_teach_skill_num;
		int service_teach_skill_list[256];

		struct __service_produce
		{
			int type;	//0 生产 1 合成 2 升级生产 3 新继承生产
			int produce_skill;
			int produce_num;
			int produce_items[256];
		}service_produce;
		
		int service_decompose_skill;

		bool service_identify;
		int  service_identify_fee;

		int service_vehicle_count;
		struct vehicle_path_entry 
		{
			int line_no;
			int target_waypoint;		//目标路点的id是多少
			A3DVECTOR dest_pos;
			float speed;
			int vehicle;
			int min_time;	//in sec
			int max_time;	//in sec
			size_t fee;
		} vehicle_path_list[16]; 

		int service_waypoint_id;	//如果是 <=0的为无效

		int service_refine_transmit;	//精炼转移服务

		int npc_tower_build_size;	//可以造出几种塔
		struct __npc_tower_build
		{
			int id_in_build;	
			int id_buildup;	
			int id_object_need;	
			int time_use;	
			int fee;		
		}npc_tower_build[4];

		int service_reset_prop_count;	//可以洗点的条目数目
		struct __reset_prop
		{
			int object_need;
			int str_delta;	
			int agi_delta;	
			int vit_delta;	
			int eng_delta;	
		}reset_prop[15];
		
		int service_change_pet_name;
		struct 
		{
			int money_need;
			int item_need;
		}change_pet_name_prop;

		int service_forget_pet_skill;
		struct 
		{
			int money_need;
			int item_need;
		}forget_pet_skill_prop;

		int service_pet_skill_num;
		int service_pet_skill_list[128];

		int service_equip_bind;
		struct 
		{
			int money_need;
			int can_webtrade;
			int item_need[4];
		} service_bind_prop;

		int service_destroy_bind;
		struct 
		{
			int money_need;
			int item_need;
		} service_destroy_bind_prop;

		int service_undestroy_bind;
		struct 
		{
			int money_need;
			int item_need;
		} service_undestroy_bind_prop;

		struct  __pw_ent
		{
			A3DVECTOR       target_pos;
			int             target_tag;
			int             require_level;
			size_t          fee;
		};

		int service_elf_learn_skill_num;//是否有小精灵学习技能服务
		int service_elf_learn_skill_list[128];//所有小精灵的技能id
		int service_playerforce_tid;	//玩家势力服务,此id为势力的模板id
		
		struct
		{
			int service_make_slot:1;
			int service_purchase:1;
			int service_repair:1;
			int service_heal:1;
			int service_install:1;
			int service_uninstall:1;
			int service_storage:1;
			int service_unlearn_skill:1;

			int service_faction:1;		//是否可处理帮派服务
			int service_face_ticket:1;	//是否可以进行整容
			int service_mail:1;		//是否有邮件服务
			int service_auction:1;		//是否有拍卖服务
			int service_double_exp:1;		//是否有双倍经验服务
			int service_hatch_pet:1;		//孵化宠物
			int service_recover_pet:1;	//还原宠物蛋
			int service_war_management:1;	//城战管理

			int service_war_leave_battle:1;	//离开战场服务
			int service_cash_trade:1;		//点卡寄售
			int service_refine:1;		//精炼服务
			int service_dye:1;		//染色服务 
			int service_elf_dec_attributie:1;	//是否有小精灵洗属性点服务
			int service_elf_flush_genius:1;		//是否有小精灵洗天赋点服务
			int service_elf_forget_skill:1;		//是否有小精灵遗忘技能服务
			int service_elf_refine:1;			//是否有小精灵精炼服务
			
			int service_elf_refine_transmit:1;	//是否有小精灵精炼转移服务
			int service_elf_decompose:1;		//是否有小精灵拆分服务
			int service_elf_destroy_item:1;		//是否有小精灵拆卸装备服务
			int service_dye_suit:1;				//成套染色服务	
			int service_repair_damaged_item:1;	//修理pk死亡后损毁物品服务
			int service_user_trashbox:1;		//帐号仓库服务
			int service_webtrade:1;				//交易平台服务
			int service_god_evil_convert:1;		//仙魔转换服务
			
			int service_wedding_book:1;			//婚礼预定服务
			int service_wedding_invite:1;		//婚礼邀请服务
			int service_factionfortress:1;		//帮派基地服务
			int service_factionfortress2:1;		//帮派基地服务2
			int service_factionfortress_material_exchange:1;	//帮派基地材料兑换
			int service_dye_pet:1;				//骑宠染色服务
			int service_trashbox_view:1;		//仓库查看服务
			int service_dpsrank:1;				//DPS排行榜服务
			
			int service_country_management:1;	//加入离开国家等
			int service_countrybattle_leave:1;	//离开国战战场
			int service_equip_sign:1;			//装备签名
			int service_change_ds_forward:1;	//跨服:原服->跨服
			int service_change_ds_backward:1;	//跨服:跨服->原服
			int service_player_rename:1;		//玩家付费改名
			int service_addon_change:1;     	// 魂石转化
			int service_addon_replace:1;    	// 魂石替换
			
			int service_kingelection:1;     	//国王选举
			int service_decompose_fashion_item:1;//时装拆解
			int service_player_shop:1;      	//玩家寄卖系统
			int service_reincarnation:1;		//玩家转生
			int service_giftcardredeem:1;		//礼品卡兑
			int service_trickbattle_apply:1;	//战场申请
			int service_generalcard_rebirth:1;	//卡牌重生
			int service_improve_flysword:1;		//飞剑改良
			int service_mafia_pvp_signup:1;		//帮派pvp宣战
			int service_gold_shop:1;			//npc元宝商城
			int service_dividend_shop:1;		//npc鸿利商城
            int service_player_change_gender:1; //npc角色变性
            int service_make_slot_for_decoration:1;     // 饰品打孔
			int service_select_solo_tower_challenge_stage:1;//单人副本选择关卡
            int service_solo_challenge_rank:1;          // 单人副本排行榜
			int service_mnfaction_sign_up:1;
			int service_mnfaction_award:1;
			int service_mnfaction_rank:1;
			int service_mnfaction_battle_transmit:1;
			int service_mnfaction_join_leave:1;
            int service_solo_challenge_rank_global:1;
		};

		struct __service_engrave
		{
			int engrave_num;
			int engrave_entrys[16];
		}service_engrave;
		
		struct __service_addonregen
		{
			int addonregen_num;
			int addonregen_entrys[16];
		}service_addonregen;

		struct __service_cross
		{
			int	activity_type;				//	活动类型(type=cross_server_activity)
			int	player_count_limit;			//	人数限制
			int	time_out;					//	活动时长_秒
			int	need_item_tid;				//	所需物品id(type=all_type)
			int	need_item_count;			//	所需物品数量
			bool cost_item;					//	是否收消耗物品(type=bool)
			int	history_max_level;			//	历史最高等级限制
			int	second_level;				//	修真等级要求(type=taoist_rank_require)
			int	realm_level;				//	境界等级要求
		}service_cross;

	}npc_data;
};

struct recipe_template
{
	int recipe_id;
	int produce_skill;
	int require_level;
	int recipe_level;
	int level;
	int exp;
	int sp;
	float null_prob;
	size_t use_time;
	size_t count;
	size_t fee;
	int material_num;
	int material_total_count;
	int bind_type;		//天人合一状态 0 如前 1 肯定天人 2 跟随材料
	int proc_type;		//生成物品的处理方式,当bind_type为0时生效，最后生成物品处理方式为ess.proc_type|proc_type
	int equipment_need_upgrade;	//生产类型为升级生产时有效，表示待升级的装备
	float inherit_fee_factor;		//生产类型为升级生产时有效，表示继承待升级装备所需要的费用系数
	float inherit_engrave_fee_factor;//生产类型为升级生产时有效，表示继承待升级装备的镌刻属性所需要的费用系数
    float inherit_addon_fee_factor;	//生产类型为生产5，即保留附加属性生产所需要的费用系数

	struct __material
	{
		int item_id;
		size_t count;
	} material_list[32];
	struct __target
	{
		int id;
		float  prob;
	}targets[4];
};

struct decompose_recipe_template
{
	//下面是分解需要的内容
	int    id;
	int produce_skill;
	int require_level;
	int recipe_level;
	size_t decompose_fee;
	size_t decompose_time;
	size_t element_num;
	int element_id;
};

struct engrave_recipe_template
{
	int recipe_id;
	int engrave_level;		//镌刻等级，目前无用
	int equip_mask;			//装备位置限制
	int equip_level_min;	//要求装备品阶下限
	int equip_level_max;	//要求装备品阶上限
	size_t use_time;		//过程占用时间
	float prob_addon_num[4];//镌刻产生0-3条属性的概率
	int material_total_count;
	int material_num;
	struct __material
	{
		int item_id;
		size_t count;
	} material_list[8];
	struct __target_addon
	{
		int addon_id;
		float  prob;
	} target_addons[32];
};

struct addonregen_recipe_template
{
	int recipe_id;
	int produce_skill;		//需要的生产技能id
	int require_level;		//需要的生产技能等级
	size_t use_time;		//过程占用时间
	size_t fee;				//花费金币
	int equip_id_list[32];	//配方适用的装备id列表
	int material_total_count;
	int material_num;
	struct __material
	{
		int item_id;
		size_t count;
	} material_list[8];
};

struct addonchange_recipe_template
{
	int recipe_id;
	int	id_old_stone;				// 转化前宝石
	int	id_new_stone;				// 转化后宝石
	
	unsigned int	fee;						// 所需金钱

	int material_num;
	struct
	{
		int	item_id;				// 转化需要原料id
		size_t	count;					// 转化需要原料数量
	}material_list[8];

};

class itemdataman;
class npc_stubs_manager
{
	typedef abase::hashtab<npc_template, int ,abase::_hash_function> MAP;
	MAP _nt_map;
	bool __InsertTemplate(const npc_template & nt)
	{
		return _nt_map.put(nt.tid, nt);
	}

	npc_template * __GetTemplate(int tid)
	{
		return _nt_map.nGet(tid);
	}
	static npc_stubs_manager & __GetInstance()
	{
		static npc_stubs_manager __Singleton;
		return __Singleton;
	}
	npc_stubs_manager():_nt_map(1024) {}
public:
	static bool Insert(const npc_template & nt)
	{
		bool rst = __GetInstance().__InsertTemplate(nt);
		ASSERT(rst);
		return rst;
	}

	static npc_template * Get(int tid)
	{
		return __GetInstance().__GetTemplate(tid);
	}

	static bool LoadTemplate(itemdataman & dataman);
};

class recipe_manager
{
	typedef abase::hashtab<recipe_template, int ,abase::_hash_function> MAP;
	typedef abase::hashtab<decompose_recipe_template, int ,abase::_hash_function> MAP2;
	typedef abase::hashtab<engrave_recipe_template, int ,abase::_hash_function> ENGRAVE_MAP;
	typedef abase::hashtab<addonregen_recipe_template, int ,abase::_hash_function> ADDONREGEN_MAP;
	typedef abase::hashtab<addonchange_recipe_template, int ,abase::_hash_function> ADDONCHANGE_MAP;

	MAP _rt_map;
	MAP2 _drt_map;
	ENGRAVE_MAP _ert_map;
	ADDONREGEN_MAP _arrt_map;
	ADDONCHANGE_MAP _acrt_map;

	bool __InsertTemplate(const recipe_template & rt)
	{
		recipe_template tmp= rt;
		tmp.material_total_count = 0;
		for(int i = 0; i < rt.material_num; i ++)
		{
			tmp.material_total_count += rt.material_list[i].count;
		}
	//	if(tmp.material_total_count > 0) 
			return _rt_map.put(rt.recipe_id, tmp);
	//	else 
	//		return false;
	}

	bool __InsertTemplate(const decompose_recipe_template & rt)
	{
		decompose_recipe_template tmp= rt;
		return _drt_map.put(rt.id, tmp);
	}

	bool __InsertTemplate(const engrave_recipe_template & ert)
	{
		return _ert_map.put(ert.recipe_id, ert);
	}

	bool __InsertTemplate(const addonregen_recipe_template & arrt)
	{
		return _arrt_map.put(arrt.recipe_id, arrt);
	}

	bool __InsertTemplate(const addonchange_recipe_template & acrt)
	{
		return _acrt_map.put(acrt.recipe_id, acrt);
	}


	recipe_template * __GetTemplate(int id)
	{
		return _rt_map.nGet(id);
	}

	decompose_recipe_template * __GetDecomposeTemplate(int id)
	{
		return _drt_map.nGet(id);
	}

	engrave_recipe_template * __GetEngraveTemplate(int id)
	{
		return _ert_map.nGet(id);
	}

	addonregen_recipe_template * __GetAddonRegenTemplate(int id)
	{
		return _arrt_map.nGet(id);
	}

	addonchange_recipe_template * __GetAddonChangeTemplate(int id)
	{
		return _acrt_map.nGet(id);
	}


	static recipe_manager & __GetInstance()
	{
		static recipe_manager __Singleton;
		return __Singleton;
	}

	recipe_manager():_rt_map(1024) ,_drt_map(1024), _ert_map(1024), _arrt_map(1024), _acrt_map(1024) {}

	recipe_template * __GetByItemID(int id)
	{
		if(id <=0) return NULL;
		MAP::iterator it = _rt_map.begin();
		for(;it != _rt_map.end(); ++it)
		{
			recipe_template & pTmp = *it;
			if(pTmp.targets[0].id == id ||
					pTmp.targets[1].id == id ||
					pTmp.targets[2].id == id ||
					pTmp.targets[3].id == id)
					{
						return &pTmp;
					}
		}
		return NULL;
	}
public:
	static bool Insert(const recipe_template & rt)
	{
		return __GetInstance().__InsertTemplate(rt);
	}

	static bool Insert(const decompose_recipe_template & drt)
	{
		return __GetInstance().__InsertTemplate(drt);
	}

	static bool Insert(const engrave_recipe_template & ert)
	{
		return __GetInstance().__InsertTemplate(ert);
	}

	static bool Insert(const addonregen_recipe_template & arrt)
	{
		return __GetInstance().__InsertTemplate(arrt);
	}
	
	static bool Insert(const addonchange_recipe_template & acrt)
	{
		return __GetInstance().__InsertTemplate(acrt);
	}

	static recipe_template * GetRecipe(int id)
	{
		return __GetInstance().__GetTemplate(id);
	}

	static decompose_recipe_template * GetDecomposeRecipe(int id)
	{
		return __GetInstance().__GetDecomposeTemplate(id);
	}
	
	static engrave_recipe_template * GetEngraveRecipe(int id)
	{
		return __GetInstance().__GetEngraveTemplate(id);
	}
	
	static addonregen_recipe_template * GetAddonRegenRecipe(int id)
	{
		return __GetInstance().__GetAddonRegenTemplate(id);
	}

	static addonchange_recipe_template * GetAddonChangeRecipe(int id)
	{
		return __GetInstance().__GetAddonChangeTemplate(id);
	}
	


	static recipe_template * GetByItemID(int id)
	{
		return __GetInstance().__GetByItemID(id);
	}

	static bool LoadTemplate(itemdataman & dataman);
};

struct gnpc;
class world;
class gnpc_imp;
class gmatter_mine_imp;
class base_spawner
{
public:
	class generate_pos : public substance
	{
		public:
			virtual void Generate(const A3DVECTOR &pos_min,const A3DVECTOR &pos_max,A3DVECTOR &pos,float offset, world * plane)=0;
			virtual float GenerateY(float x, float y,float z, float offset,world * plane) = 0;
	};
	base_spawner():_region(0,0,0,0),_pos_min(0,0,0),_pos_max(0,0,0),_pos_generator(0),_pool_lock(0),
		       _auto_respawn(true),_is_spawned(false),_dir(0),_dir1(0),_rad(0),_collision_id(0)
		       {}
protected:
	rect	_region;
	A3DVECTOR _pos_min;				//最小的坐标位置
	A3DVECTOR _pos_max;				//最大的坐标位置
	generate_pos * _pos_generator;
	int _pool_lock;					//这是可能修改NPC池的操作时加的锁
	abase::static_multimap<XID,int, abase::fast_alloc<> > _xid_list;
	bool	_auto_respawn;				//自动重生
	bool	_is_spawned;				//是否有效
	virtual ~base_spawner() { delete _pos_generator;}
	virtual void Release() = 0;
public:
	char _dir;					//保存模板里的方向，供NPC使用
	char _dir1;
	char _rad;
	int  _collision_id;
	void SetDir(unsigned char dir0,unsigned char dir1, unsigned char rad)
	{
		_dir = dir0;
		_dir1 = dir1;
		_rad = rad;
	}
	virtual void GeneratePos(A3DVECTOR &pos,float offset_terrain, world * plane)
	{
		_pos_generator->Generate(_pos_min, _pos_max,pos,offset_terrain,plane);
	}
	virtual char GenDir()
	{
		if(_pos_min.squared_distance( _pos_max) < 1e-3)
		{
			return _dir;
		}
		else
		{
			return abase::Rand(0,255);
		}
	}
	void SetRegion(int region_type, const float vPos[3],const float vExts[3], const float vDir[3]);
	const rect & GetRegion() { return _region;}
	void BuildRegionCollision(world * plane, int tid, int did, float offset_terrain, int region_idx);//固定地图使用
	void BuildRegionCollision2(world * plane, int tid, int did, float offset_terrain);//随机地图使用
	virtual void BuildRegionCollision(world * plane, int region_idx){}	//固定地图使用
	virtual void BuildRegionCollision2(world * plane){}	//随机地图使用
	void ReleaseSelf()
	{
		Release();
		delete this;
	}
	void SetRespawn(bool are) 
	{ 
		_auto_respawn = are;
	}

	void Heartbeat(world * pPlane)
	{
		mutex_spinlock(&_pool_lock);
		if(IsSpawned())
		{
			OnHeartbeat(pPlane);
		}
		mutex_spinunlock(&_pool_lock);
	}

	bool BeginSpawn(world * pPlane)
	{
		bool bRst = false;
		mutex_spinlock(&_pool_lock);
		if(!IsSpawned())
		{
			bRst = CreateObjects(pPlane);	
			SetSpawned(bRst);
		}
		mutex_spinunlock(&_pool_lock);
		return bRst;
	}
	
	bool EndSpawn(world * pPlane)
	{
		bool bRst = false;
		mutex_spinlock(&_pool_lock);
		if(IsSpawned())
		{
			ClearObjects(pPlane);	
			SetSpawned(false);
			bRst = true;
		}
		mutex_spinunlock(&_pool_lock);
		return bRst;
	}
private:
	virtual void OnHeartbeat(world * pPlane) = 0;
	virtual bool CreateObjects(world *pPlane) = 0;
	virtual void ClearObjects(world *pPlane) = 0;
	void SetSpawned(bool sp)
	{
		_is_spawned = sp;
	}
	bool IsSpawned() {return _is_spawned;}
	
};


struct crontab_t
{
private:
	int min;
	int hour;
	int month;
	int year;
	int day_of_months;
	int day_of_week;
public:
	struct entry_t
	{
		int min;
		int hour;
		int month;
		int year;
		int day_of_months;
		int day_of_week;
	};
public:
	crontab_t():min(-1), hour(-1), month(-1), year(-1), day_of_months(-1), day_of_week(-1)
	{}

	bool SetParam(int __year, int __month, int __day_of_months, int __hour, int __min, int __day_of_week)
	{
		min		= __min	;
		hour		= __hour;	
		month		= __month;	
		year		= __year;	
		day_of_months	= __day_of_months;	
		day_of_week	= __day_of_week;
		
		return min >= 0 && !(day_of_week >=0 && day_of_months >= 1) && day_of_months != 0;
	}

	bool SetParam(const entry_t & param)
	{
		min		= param.min;
		hour		= param.hour;	
		month		= param.month;	
		year		= param.year;	
		day_of_months	= param.day_of_months;	
		day_of_week	= param.day_of_week;
		
		return min >= 0 && !(day_of_week >=0 && day_of_months >= 1) && day_of_months != 0;
	}

	int CheckTime(const tm &tt);
	int CheckTime2(time_t t1,int DUR);
};

class spawner_ctrl 
{
	int _spawn_id;			//生成区的分类ID
	bool _auto_spawn;		//是否自动生成
	bool _spawn_flag;		//是否已经生成
	bool _active_flag;		//是否已经激活		激活后不一定立刻生成
	abase::vector<base_spawner *, abase::fast_alloc<> > _list;

	int _spawn_after_active;	//激活到生成的缓冲时间
	int _active_life;		//激活后的寿命 0为无穷

	bool _has_active_date;
	bool _has_stop_date;
	int _active_date_duration;
	crontab_t _active_date;
	crontab_t _stop_date;

	int  _date_counter_down;
	int  _cur_active_life;	
	int  _time_before_spawn;
	int  _lock;
public:
	
	spawner_ctrl():_spawn_id(0),_auto_spawn(true),_spawn_flag(false),_active_flag(false)
	{
		_spawn_after_active = 0;
		_active_life = 0;
		_has_active_date = 0;
		_has_stop_date = 0;

		_date_counter_down = 0;
		_cur_active_life = 0;
		_time_before_spawn = 0;
		_active_date_duration = 0;
		_lock = 0;
	}
	inline bool IsAutoSpawn() { return _auto_spawn;}
	inline bool IsActived() { return _active_flag;}
	inline void AddSpawner(base_spawner * sp) {_list.push_back(sp);}

	void SetCondition(int spawn_id, bool auto_spawn, int spawn_delay = 0, int active_life = 0)
	{
		_spawn_id = spawn_id;
		_auto_spawn = auto_spawn;
		_spawn_after_active =  spawn_delay;
		_active_life = active_life;
	}

	bool SetActiveDate(const crontab_t::entry_t & ent, int duration)
	{
		_has_active_date = false;
		if(!_active_date.SetParam(ent)) return false;
		_active_date_duration = duration;
		_has_active_date = true;
		return true;
	}

	bool SetStopDate(const crontab_t::entry_t & ent)
	{
		_has_stop_date = false;
		if(!_stop_date.SetParam(ent)) return false;
		_has_stop_date = true;
		return true;
	}

	void OnHeartbeat(world * pPlane);
protected:
	inline void SetSpawnFlag(bool sf) { _spawn_flag = sf;}
	inline bool IsSpawned() { return _spawn_flag;}
	void Spawn(world * pPlane);
	void Active(world * pPlane);
	void Stop(world * pPlane);

public:
	bool BeginSpawn(world * pPlane)
	{
		spin_autolock keeper(_lock);
		if(IsActived()) return false;
		Active(pPlane);
		return true; 
	}

	bool EndSpawn(world * pPlane)
	{
		spin_autolock keeper(_lock);
		if(!IsActived()) return false;
		Stop(pPlane);
		return true; 
	}
};

class gnpc_imp;
class npc_spawner : public base_spawner
{
	public:
		struct entry_t
		{
			int npc_tid;
			int msg_mask_or;
			int msg_mask_and;
			int alarm_mask;			//未用 
			int enemy_faction;		//未用 
			bool has_faction;
			int faction;			//未用 
			bool ask_for_help;
			int monster_faction_ask_help;
			bool accept_ask_for_help; 
			int monster_faction_accept_for_help;
			int reborn_time_upper;	//重生时间上限
			int reborn_time_lower;	//重生时间下限
			int path_id;			//巡逻路线，如果是0则未用
			int path_type;			//巡论类型
			int corpse_delay;		//尸体残留时间，单位为秒 
			bool speed_flag;		//速度标记
			size_t mobs_count;
			float offset_terrain;		//和地形高度的偏移量
		};
	protected:
		int _reborn_time;				//总的重生时间，给group_spawner使用
		abase::vector<entry_t, abase::fast_alloc<> > _entry_list;//本生成器中的所有entry列表
		abase::vector<gnpc *, abase::fast_alloc<> > _npc_pool;   //npc的生成池，所有释放的npc 会被缓冲到这里
		int _mobs_total_gen_num;				//能够生成多少对象
		int _mobs_cur_gen_num;					//当前生成了多少对象
		int _mob_life;						//创建怪物的寿命

	public:
		npc_spawner():_reborn_time(0),_mobs_total_gen_num(0),_mobs_cur_gen_num(0),_mob_life(0){}
		virtual ~npc_spawner() {}
		virtual int Init(const void * buf, size_t len) { return 0;}
		virtual void OnHeartbeat(world * pPlane) = 0;
		virtual bool Reclaim(world * pPlane, gnpc * pNPC, gnpc_imp * imp,bool is_reset) = 0;
		virtual bool CreateMobs(world *pPlane) {return false;}
		virtual void ForwardFirstAggro(world * pPlane,const XID & id, int rage) {}

		virtual bool CreateObjects(world *pPlane)
		{
			_mobs_cur_gen_num = 0;
			return CreateMobs(pPlane);
		}
		virtual void ClearObjects(world * pPlane);

		static gnpc * CreateMobBase(npc_spawner * __this,world * pPlane,const entry_t & et,
				int spawn_index, const A3DVECTOR & pos,const int cid[3],unsigned char dir, 
				int ai_policy_cid,int aggro_policy, gnpc * orign_npc,int life = 0);
		static gnpc * CreatePetBase(gplayer_imp *pImp, const pet_data * pData, const A3DVECTOR & pos, char inhabit_mode,
				const int cid[3],unsigned char dir, unsigned char pet_stamp, 
				int ai_policy_cid, int aggro_policy);
		static gnpc * CreatePetBase2(gplayer_imp *pImp, const pet_data * pData, const A3DVECTOR & pos, char inhabit_mode,
				const int cid[3],unsigned char dir, unsigned char pet_stamp, 
				int ai_policy_cid, int aggro_policy, int skill_level);
		static gnpc * CreatePetBase3(gplayer_imp * pMaster, const pet_data * pData, const A3DVECTOR & pos, char inhabit_mode, const int cid[3],
				unsigned char dir,  unsigned char pet_stamp, 
				int ai_policy_cid , int aggro_policy);
		static gnpc * CreateNPCBase(npc_spawner * __this, world * pPlane, const entry_t & et,
				int spawn_index, const A3DVECTOR & pos,const int cid[3], unsigned char dir,
				int ai_policy_cid = -1,int aggro_policy = 0,gnpc * origin_npc = NULL, int life = 0);

		static void AdjustPropByCommonValue(gnpc_imp * pImp, world * pPlane, npc_template * pTemplate);
		void RegenAddon(gnpc_imp * pImp, int npc_id);		
		
		void SetGenLimit(int life, int total_num)
		{
			_mob_life = life;
			_mobs_total_gen_num = total_num;
		}


	public:
		bool AddEntry(const entry_t & ent)
		{
			_entry_list.push_back(ent);
			_npc_pool.push_back(NULL);
			return true;
		}
		
		virtual void BuildRegionCollision(world * plane, int region_idx);	//固定地图使用
		virtual void BuildRegionCollision2(world * plane);	//随机地图使用

		int CutRegion(const rect & rt)
		{
			float oa = _region.GetArea();
			if(oa <= 0.01f) 
			{
				//对于面积非常小的区域
				//认为在其中
				int total_count = 0;
				for(size_t i = 0; i < _entry_list.size(); i ++)
				{
					total_count += _entry_list[i].mobs_count;
				}
				return total_count;
			}
			
			float na = rt.GetArea();
			float factor = na/oa;
			int total_count = 0;
			//根据面积比例调整数值
			for(size_t i = 0; i < _entry_list.size(); i ++)
			{
				ASSERT(_npc_pool[i] == 0);
				int mobs_count = (int)(_entry_list[i].mobs_count * factor + 0.5f);
				if(mobs_count <= 0)
				{
					//应该删除这个条目
					_entry_list.erase(_entry_list.begin() + i);
					_npc_pool.erase(_npc_pool.begin() + i);
					--i;
					continue;
				}
				_entry_list[i].mobs_count = mobs_count;
				total_count += mobs_count;
			}
			_region = rt;
			_pos_min.x = rt.left; _pos_min.z = rt.top;
			_pos_max.x = rt.right; _pos_max.z = rt.bottom;
			return total_count;
		}

		void SetRebornTime(int rtime) 
		{
			_reborn_time = rtime;
		}
};

class mine_spawner : public base_spawner
{
public:
	struct entry_t
	{
		int mid;
		int mine_count;
		int reborn_time;
	};
protected:
	abase::vector<entry_t,abase::fast_alloc<> > _entry_list;//本生成器中的所有entry列表
	abase::vector<gmatter *,abase::fast_alloc<> > _mine_pool;//npc的生成池，所有释放的npc 会被缓冲到这里
	float _offset_terrain;
	int _mine_total_gen_num;				//能够生成多少对象
	int _mine_cur_gen_num;					//当前生成了多少对象

public:
	mine_spawner():_offset_terrain(0),_mine_total_gen_num(0),_mine_cur_gen_num(0) {}
	virtual ~mine_spawner() {}
	virtual void OnHeartbeat(world * pPlane);
	virtual bool Reclaim(world * pPlane,gmatter * pMatter, gmatter_mine_imp * imp);
	virtual bool CreateMines(world * pPlane);
	static gmatter * CreateMine(mine_spawner* ,const A3DVECTOR & pos, world * pPlane,int index,const entry_t & ent);
	static gmatter * CreateMine2(mine_spawner* ,const A3DVECTOR & pos, world * pPlane,int index,const entry_t & ent,unsigned char dir, unsigned char dir1, unsigned char rad);
	static void GenerateMineParam(gmatter_mine_imp * imp, npc_template * pTemplate);
	void Reborn(world * pPlane,gmatter * header, gmatter * tail,int mid,int index);
	void SetOffsetTerrain(float offset) { _offset_terrain = offset; }
	void Release();
	void SetGenLimit(int total_num)
	{
		_mine_total_gen_num = total_num;
	}


	virtual bool CreateObjects(world *pPlane)
	{
		_mine_cur_gen_num = 0;
		return CreateMines(pPlane);
	}

	virtual void ClearObjects(world * pPlane);

public:
	bool AddEntry(const entry_t & ent)
	{
		_entry_list.push_back(ent);
		_mine_pool.push_back(NULL);
		return true;
	}

	virtual void BuildRegionCollision(world * plane, int region_idx);	//固定地图使用
	virtual void BuildRegionCollision2(world * plane);	//随机地图使用

	int CutRegion(const rect & rt)
	{
		float oa = _region.GetArea();
		if(oa <= 0.01f) 
		{
			//对于面积非常小的区域
			//认为在其中
			int total_count = 0;
			for(size_t i = 0; i < _entry_list.size(); i ++)
			{
				total_count += _entry_list[i].mine_count;
			}
			return total_count;
		}

		float na = rt.GetArea();
		float factor = na/oa;
		int total_count = 0;
		//根据面积比例调整数值
		for(size_t i = 0; i < _entry_list.size(); i ++)
		{
			ASSERT(_mine_pool[i] == 0);
			int mine_count = (int)(_entry_list[i].mine_count * factor + 0.5f);
			if(mine_count <= 0)
			{
				//应该删除这个条目
				_entry_list.erase(_entry_list.begin() + i);
				_mine_pool.erase(_mine_pool.begin() + i);
				--i;
				continue;
			}
			_entry_list[i].mine_count = mine_count;
			total_count += mine_count;
		}
		_region = rt;
		_pos_min.x = rt.left; _pos_min.z = rt.top;
		_pos_max.x = rt.right; _pos_max.z = rt.bottom;
		return total_count;
	}
};
class dyn_object_spawner : public mine_spawner  
{       
	unsigned char _scale;
public:         
	virtual bool CreateMines(world * pPlane);
	dyn_object_spawner() :_scale(0) {} 
	inline void SetScale(unsigned char s) {_scale = s;}
protected:      
	gmatter * CreateDynObject(const A3DVECTOR & pos,size_t index, world * pPlane,const entry_t & ent);
};

class CNPCGenMan;
class npc_generator : public ONET::Thread::Runnable
{

private:
//	abase::vector<npc_spawner*,abase::fast_alloc<> > _spawner_list;
//	abase::vector<mine_spawner*,abase::fast_alloc<> > _mine_spawner_list;

	abase::vector<base_spawner *,abase::fast_alloc<> > _sp_list;
	abase::vector<spawner_ctrl *,abase::fast_alloc<> > _ctrl_list;

	abase::hash_map<int, spawner_ctrl *> _ctrl_map;		//condition id-->ctrl 的对照表，里面并未包含所有ctrl
	abase::hash_map<int, spawner_ctrl *> _ctrl_idx_map; 	//ctrl id-->ctrl的对照表 里面包含了所有的ctrl

	npc_generator & operator=(const npc_generator & );
public:
	npc_generator():_task_offset(0),_tcount(0),_tlock(0),_plane(0)
	{
		//_spawner_list.reserve(MAX_SPAWN_COUNT);
		//_mine_spawner_list.reserve(MAX_SPAWN_COUNT);
		_sp_list.reserve(MAX_SPAWN_COUNT);
	}

	bool LoadGenData(world* plane, CNPCGenMan & npcgen, rect & region);
	bool AddCtrlData(CNPCGenMan& ctrldata,unsigned int ctrl_id, unsigned char block_id);
	bool AddSpawnData(world* plane, CNPCGenMan& ctrldata,CNPCGenMan& spawndata, unsigned char block_id, const A3DVECTOR& p_offset, bool global_ctrl_gen, bool unique_resource);
	
	bool InsertSpawner(int ctrl_id, base_spawner * sp)
	{
		spawner_ctrl * ctrl = _ctrl_idx_map[ctrl_id];
		if(ctrl == NULL) 
		{
			__PRINTF("布置区域无法找到合适的控制对象\n");
			return false;
		}

		_sp_list.push_back(sp);
		ctrl->AddSpawner(sp);
		return true;
	}

	bool InsertSpawnControl(int ctrl_idx, int condition_id, bool auto_spawn,int spawn_delay, int active_life,
				int active_date_duration,
				const crontab_t::entry_t * active_date = NULL, 
				const crontab_t::entry_t * stop_date = NULL)
	{
		spawner_ctrl * ctrl = _ctrl_idx_map[ctrl_idx];
		if(ctrl) return false;
		if(condition_id > 0)
		{
			ctrl = _ctrl_map[condition_id];
			if(ctrl) return false;
		}
		ctrl = new  spawner_ctrl();
		ctrl->SetCondition(condition_id, auto_spawn, spawn_delay, active_life);
		bool bRst = true;
		if(active_date)
		{
			bRst = bRst  && ctrl->SetActiveDate(*active_date,active_date_duration);
		}

		if(stop_date)
		{
			bRst = bRst  && ctrl->SetStopDate(*stop_date);
		}

		_ctrl_idx_map[ctrl_idx] = ctrl;
		_ctrl_list.push_back(ctrl);

		if(condition_id > 0) _ctrl_map[condition_id] = ctrl;
		return bRst;
	}

	bool InitIncubator(world * pPlane);		//初始化所有的spawner，去掉不属于自己的部分

	bool BeginSpawn();
	bool TriggerSpawn(int condition);
	void ClearSpawn(int condition);
	void Release();
public:
	
	void StartHeartbeat()
	{
		_task_offset = 0;
		_task_offset2 = 0;
		_tcount = 0;
		_tcount2 = 0;
		_tlock = 0;
	}
	
	static int GenBlockUniqueID(int id, unsigned char block_id)
	{
		return (id&0xFFFFFF) + ((block_id << 24)& 0xFF000000);
	}

protected:
	size_t _task_offset;
	size_t _task_offset2;
	int _tcount;
	int _tcount2;
	int _tlock;
	world * _plane;

	void OnTimer(int index,int rtimes)
	{
		ONET::Thread::Pool::AddTask(this);
	}
public:

/*
	virtual void Run()
	{
		spin_autolock keeper(_tlock);
		_tcount += _spawner_list.size();

		while(_tcount > TICK_PER_SEC)
		{
			_spawner_list[_task_offset]->OnHeartbeat(_plane);
			_task_offset ++;
			if(_task_offset >= _spawner_list.size()) _task_offset = 0;
			_tcount -= TICK_PER_SEC;
		}

		_tcount2 += _mine_spawner_list.size();
		while(_tcount2 > TICK_PER_SEC)
		{
			_mine_spawner_list[_task_offset2]->OnHeartbeat(_plane);
			_task_offset2 ++;
			if(_task_offset2 >= _mine_spawner_list.size()) _task_offset2 = 0;
			_tcount2 -= TICK_PER_SEC;
		}
	}*/

	virtual void Run()
	{
		spin_autolock keeper(_tlock);
		_tcount += _sp_list.size();

		while(_tcount > TICK_PER_SEC)
		{
			_sp_list[_task_offset]->Heartbeat(_plane);
			_task_offset ++;
			if(_task_offset >= _sp_list.size()) _task_offset = 0;
			_tcount -= TICK_PER_SEC;
		}

		_tcount2 += _ctrl_list.size();
		while(_tcount2 > TICK_PER_SEC)
		{
			_ctrl_list[_task_offset2]->OnHeartbeat(_plane);
			_task_offset2 ++;
			if(_task_offset2 >= _ctrl_list.size()) _task_offset2 = 0;
			_tcount2 -= TICK_PER_SEC;
		}
	}

};

class mobs_spawner : public npc_spawner
{
public:
	int _reborn_count;
	int _rrcount;
protected:
	//path_graph ..
	gnpc * CreateMob(world * pPlane,int spawn_index,const entry_t & et);
	void Reborn(world * pPlane,gnpc * header, gnpc * tail,float height,int tid);

	virtual void ReCreate(world * pPlane, gnpc * pNPC, const A3DVECTOR & pos, int index);
public:
	mobs_spawner():_reborn_count(0),_rrcount(50){}
	void OnHeartbeat(world * pPlane);

	//创建所有的mobs 
	virtual bool CreateMobs(world * pPlane);
	virtual bool Reclaim(world * pPlane,gnpc * pNPC,gnpc_imp * pImp, bool is_reset);
	virtual void Release();

};

//npc服务，是否应该将若干npc放在一个spawner里面
class server_spawner : public mobs_spawner
{
public:
protected:
	//path_graph ..
	gnpc* CreateNPC(world * pPlane, int spawn_index,const entry_t & et);
	gnpc* CreateNPC(world * pPlane, int spawn_index,const entry_t & et, const A3DVECTOR & pos, gnpc * pNPC);

	//void Reborn(world * pPlane,gnpc * header, gnpc * tail,float height,int tid);
	virtual void ReCreate(world * pPlane, gnpc * pNPC, const A3DVECTOR & pos, int index);
public:
	server_spawner(){}
	void OnHeartbeat(world * pPlane);

	//创建所有的mobs 
	virtual bool CreateMobs(world * pPlane);
	virtual bool Reclaim(world * pPlane,gnpc * pNPC,gnpc_imp * pImp, bool is_reset);
	
};

//普通群怪
class group_spawner : public mobs_spawner
{
protected:
	int _next_time;
	int _lock;				//这是群怪的锁，当操作重生时间时加这个锁，理论上这个锁可以归并到_pool_lock中
	int _leader_id;
	A3DVECTOR _leader_pos;
	bool _gen_pos_mode;
	gnpc * CreateMasterMob(world * pPlane,int spawner_index, const entry_t &et);
	gnpc * CreateMinorMob(world * pPlane,int spawner_index, int leader_id, const A3DVECTOR & leaderpos, const entry_t &et);
public:
	group_spawner():_next_time(0),_lock(0),_leader_id(0),_gen_pos_mode(false)
	{
		_rrcount = 1;
	}
	virtual void ClearObjects(world *pPlane) 
	{
		mobs_spawner::ClearObjects(pPlane);
		_next_time = 0;
		_leader_id = 0;
	}
	
	void OnHeartbeat(world * pPlane);
	virtual bool CreateMobs(world *pPlane);
	virtual bool Reclaim(world * pPlane, gnpc * pNPC, gnpc_imp * imp, bool is_reset);
	virtual void GeneratePos(A3DVECTOR &pos,float offset_terrain,world * plane);
};

//特殊群怪
class boss_spawner : public group_spawner
{
protected:
	abase::vector<XID,abase::fast_alloc<> > _mobs_list;
	int _mobs_list_lock;
public:
	boss_spawner()
	{
		_mobs_list_lock = 0;
	}
	virtual void ClearObjects(world *pPlane) 
	{
		group_spawner::ClearObjects(pPlane);
		spin_autolock keep(_mobs_list_lock);
		_mobs_list.clear();
	}
	
	virtual bool CreateMobs(world *pPlane);
	gnpc * CreateMasterMob(world * pPlane,int spawner_index, const entry_t &et);
	gnpc * CreateMinorMob(world * pPlane,int spawner_index, int leader_id, const A3DVECTOR & leaderpos, const entry_t &et);
	virtual void ForwardFirstAggro(world* pPlane,const XID & id, int rage);
};
#endif

