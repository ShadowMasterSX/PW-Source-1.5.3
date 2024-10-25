#ifndef __ONLINE_GAME_GS_PET_MANAGER_H__
#define __ONLINE_GAME_GS_PET_MANAGER_H__

#include <common/types.h>
#include <amemory.h>
#include <common/base_wrapper.h>

class world;
class pet_gen_pos
{
public:
	static bool FindGroundPos(A3DVECTOR & pos,float dis,A3DVECTOR & offset, world * plane);
	static bool FindWaterPos(A3DVECTOR & pos,float dis,A3DVECTOR & offset, world * plane);
	static bool FindAirPos(A3DVECTOR & pos,float dis,A3DVECTOR & offset, world * plane);
	static bool IsValidInhabit(char leader_layer, int inhabit_type);
	static int FindValidPos(A3DVECTOR & pos, char & inhabit_mode, char leader_layer, int inhabit_type, world * plane, float dis=0.f, A3DVECTOR offset=A3DVECTOR(0.f,0.f,0.f));
};

struct extend_prop;
struct msg_pet_hp_notify;
#pragma pack(1)
struct pet_data
{
	enum
	{
		MAX_PET_SKILL_COUNT = 8,
		MAX_NAME_LEN = 16,
	};

	int honor_point;	//好感度	会经常变化
	int hunger_gauge;	//饥饿度	会经常变化
	int feed_period;	//上次喂养到现在的计时时间
	int pet_tid;		//宠物的模板ID
	int pet_vis_tid;	//宠物的可见ID（如果为0，则表示无特殊可见ID）
	int pet_egg_tid;	//宠物蛋的ID
	int pet_class;		//宠物类型 战宠，骑宠，观赏宠
	float hp_factor;	//血量比例（复活和收回时使用） 0则为死亡
	short level;		//宠物级别
	unsigned short color;   //宠物颜色，最高位为1表示有效，目前仅对骑宠有效
	int exp;		//宠物当前经验
	int skill_point;	//剩余技能点
	char is_bind;		//是否处于绑定状态,现在是一个mask 0x01绑定 0x02可寻宝网交易
	char unused;		
	unsigned short name_len;//名字长度
	char name[MAX_NAME_LEN];//名字内容
	struct __skills
	{
		int skill;
		int level;
	} skills[MAX_PET_SKILL_COUNT];
	struct __evo_prop
	{
		int r_attack;
		int r_defense;
		int r_hp;
		int r_atk_lvl;
		int r_def_lvl;
		int nature;
	} evo_prop;
	int reserved[10];//未用
	enum
	{
		PET_CLASS_MOUNT,		//骑宠
		PET_CLASS_COMBAT,		//战斗宠物
		PET_CLASS_FOLLOW,		//跟随宠物
		PET_CLASS_SUMMON,		//召唤物
		PET_CLASS_PLANT,		//植物
		PET_CLASS_EVOLUTION,	//进化宠物
		PET_CLASS_MAX,
	};

	enum
	{
		HUNGER_LEVEL_0,			//饱食
		HUNGER_LEVEL_1,			//正常
		HUNGER_LEVEL_2,			//饿程度一级 
		HUNGER_LEVEL_3,	
		HUNGER_LEVEL_4,			//饿程度二级
		HUNGER_LEVEL_5,
		HUNGER_LEVEL_6,
		HUNGER_LEVEL_7,			//饿程度三级
		HUNGER_LEVEL_8,
		HUNGER_LEVEL_9,
		HUNGER_LEVEL_A,
		HUNGER_LEVEL_B,			//饿程度四级 
		HUNGER_LEVEL_COUNT
	};

	enum 
	{
		FEED_TIME_UNIT = 300,		//300	喂养的时间单位
	};

	enum
	{
		HONER_LEVEL_0,			//野性难驯	0-50
		HONER_LEVEL_1,			//反复无偿	51-150
		HONER_LEVEL_2,			//乖巧听话	151-500
		HONER_LEVEL_3,			//忠心耿耿	501-999
		HONOR_LEVEL_COUNT,

		HONOR_POINT_MAX	= 999
	};
};

#pragma pack()

class gplayer_imp;
class pet_manager
{
public:
	enum
	{
		MAX_PET_CAPACITY = 20,
		NOTIFY_MASTER_TIME = 16,
		MAX_SUMMON_CAPACITY = 1,				//只能有一个召唤物,不能改动
		SUMMON_SLOT	= MAX_PET_CAPACITY, 		//召唤物在pet_list中索引
	};

	enum RECALL_REASON
	{
		NONE = 0,
		PET_DEATH,			//宠物死亡
		PET_LIFE_EXHAUST,	//宠物寿命到
		PET_SACRIFICE,		//宠物牺牲
	};
	
protected:
	pet_data * 	_pet_list[MAX_PET_CAPACITY + MAX_SUMMON_CAPACITY];		//宠物列表+召唤物列表
	size_t		_active_pet_slot;			//最大有多少格宠物包裹能激活
	int 		_cur_active_pet;			//当前换出的宠物索引

public:
	XID		_cur_pet_id;				//当前宠物ID
	unsigned char	_pet_summon_stamp;			//宠物召唤次数，用于区别不同的召唤
	bool		_is_on_underwater;			//是否在水下进行特殊处理
	char		_cur_pet_aggro_state;			//当前宠物的仇恨方式
	char		_cur_pet_stay_state;			//当前宠物的跟随方式
	char		_cur_pet_combat_state;			//当前宠物的攻击状态
	bool		_need_feed;				//需要喂食宠物
	short		_cur_pet_counter;			//当前宠物召唤计时，用于统计经验值
	short		_cur_pet_notify_counter;		//当前发送给宠物更新数据的消息计时
	size_t		_cur_pet_state;				//如果宠物通知了自己的信息，则此标志置位
	int		_cur_pet_hp;				//当前宠物的血值 
	int		_cur_pet_inhabit;			//当前宠物的栖息地
	float	_cur_pet_mp_factor;
	int		_cur_pet_mp;
	int 	_cur_pet_life;				//当前宠物寿命,0为永久
	int		_summon_skill_level;		//召唤出当前宠物所用的技能等级

protected:
	void ClearCurPet(gplayer_imp * pImp)
	{
		_cur_active_pet = -1;
		_cur_pet_id = XID(-1,-1);
		//do others ....
	}
	void HandleFeedTimeTick(gplayer_imp * pImp, pet_data * pData);
	void ModifyHonor(pet_data * pData, int offset);
	void ModifyHungerGauge(pet_data * pData, int offset);
public:
	pet_manager();
	~pet_manager();

	void ActiveNoFeed(bool nofeed)
	{
		_need_feed = !nofeed;
	}

	void SetAvailPetSlot(size_t slot)
	{
		if(slot > _active_pet_slot && slot <= MAX_PET_CAPACITY)
		{
			_active_pet_slot = slot;
		}
	}

	size_t GetAvailPetSlot()
	{
		return _active_pet_slot;
	}

	bool IsPetActive() 
	{
		return _cur_active_pet >= 0;
	}

	int GetCurActivePet()
	{
		return _cur_active_pet;
	}

	void SetTestUnderWater(bool val)
	{
		_is_on_underwater = val;
	}

	const XID & GetCurPet()
	{
		return _cur_pet_id;
	}

	pet_data * GetPetData(size_t index)
	{
		if(index < MAX_PET_CAPACITY + MAX_SUMMON_CAPACITY)
		{
			return _pet_list[index];
		}
		else
		{
			return NULL;
		}
	}

	size_t GetPetsCount()
	{
		size_t cnt = 0;
		for(size_t i=0; i<MAX_PET_CAPACITY; i++)
			if(_pet_list[i]) ++cnt;
		return cnt;
	}

	bool DBSetPetData(size_t index, const void * data, size_t size);
	void ClientGetPetRoom(gplayer_imp * pImp);

protected:
	bool DoActiveMount(gplayer_imp * pImp,pet_data * pData);
	void TestUnderWater(gplayer_imp * pImp, float offset);
	void DoHeartbeat(gplayer_imp * pImp);

public:
	void OnUnderWater(gplayer_imp * pImp, float offset)
	{
		if(!_is_on_underwater) return;
		TestUnderWater(pImp,offset);
	}	
	void Save(archive & ar);
	void Load(archive & ar);
	void Swap(pet_manager & rhs);

public:
	int AddPetData(const pet_data & data);
	bool BanishPet(gplayer_imp * pImp, size_t index);
	void FreePet(gplayer_imp * pIMp, size_t index);		//较少校验的释放

	bool ActivePet(gplayer_imp * pImp, size_t index);
	bool RecallPet(gplayer_imp * pImp);
	bool RecallPetWithoutFree(gplayer_imp * pImp, char reason=0);	//仅在内部调用

	bool FeedCurPet(gplayer_imp * pImp, int food_type, int honor);
	bool RelocatePos(gplayer_imp * pImp,const XID & who , int stamp,bool is_disappear);
	bool NotifyPetHP(gplayer_imp * pImp,const XID & who , int stamp,const msg_pet_hp_notify & info);
	bool PetDeath(gplayer_imp * pImp,const XID & who , int stamp);
	bool PlayerPetCtrl(gplayer_imp * pImp,int cur_target,int pet_cmd, const void * buf, size_t size);
	void KillMob(gplayer_imp * pImp, int mob_level);
	int ResurrectPet(gplayer_imp * pImp, size_t index);
	int ResurrectPet(gplayer_imp * pImp);
	void NotifyMasterInfo(gplayer_imp * pImp,bool atonce);	
	void PreSwitchServer(gplayer_imp * pImp);
	void PostSwitchServer(gplayer_imp * pImp);
	void PlayerBeAttacked(gplayer_imp * pImp, const XID & target);
	void PetSetCoolDown(gplayer_imp *pImp, const XID & who, int idx, int msec);
	void NotifyStartAttack(gplayer_imp *pImp, const XID & target, char force_attack);
	int ChangePetName(gplayer_imp * pImp, size_t index, const char * name, size_t name_len);
	int ForgetPetSkill(gplayer_imp * pImp,int skill_id);
	int LearnSkill(gplayer_imp * pImp,int skill_id, int * level_result);
	void NotifyInvisibleData(gplayer_imp *pImp);

	void RecvExp(gplayer_imp * pImp, size_t pet_index,int exp);
	bool AddExp(gplayer_imp *pImp,size_t pet_index,int exp);
	inline void Heartbeat(gplayer_imp * pImp);
	friend class petdata_imp;

	bool ActivePet2(gplayer_imp * pImp, pet_data & data, int life, int skill_level);
	void TryFreePet(gplayer_imp * pImp);		//试图释放已经被召回的召唤物
	bool PetSacrifice(gplayer_imp * pImp);
	bool DyePet(gplayer_imp * pImp, size_t index, unsigned short color);
	bool EvolutionPet(gplayer_imp *pImp,size_t index,int evolution_id,int pet_nature,int skill1,int level1,int skill2,int level2);
	bool RebuildInheritRatio(int pet_id,int &r_attack,int &r_defense,int &r_hp,int &r_atk_lvl,int &r_def_lvl); 
	void PetAcceptInheritRatioResult(gplayer_imp *pImp,size_t index,int r_attack,int r_defense,int r_hp,int r_atk_lvl,int r_def_lvl);
	void PetAcceptNatureResult(gplayer_imp *pImp,size_t index,int nature,int skill1,int level1,int skill2,int level2);
	int GetNormalSkillNum(gplayer_imp *pImo,pet_data *pData);
	bool IsSkillNormal(gplayer_imp *pImp,pet_data *pData,int skill_id);
	void OnMountSpeedEnChanged(gplayer_imp *pImp);
};

class petdata_imp
{
public:
	virtual bool DoActivePet(gplayer_imp * pImp, pet_manager * pMan,pet_data * pData, extend_prop * pProp) = 0;
	virtual bool DoRecallPet(gplayer_imp * pImp, pet_manager * pMan,pet_data * pData) = 0;
	virtual void TestUnderWater(gplayer_imp * pImp, pet_manager * pMan,pet_data * pData,float offset) = 0;
	virtual void LevelUp(gplayer_imp *pImp, pet_manager * pMan,pet_data * pData) = 0;
	virtual void Heartbeat(gplayer_imp *pImp, pet_manager * pMan,pet_data * pData) = 0;
	virtual void OnHonorModify(gplayer_imp *pImp, pet_manager * pMan,pet_data * pData, int old_honor) = 0;
	virtual bool OnPetRelocate(gplayer_imp *pImp, pet_manager * pMan,pet_data *pData,const XID &who,int stamp,bool is_disappear) = 0; 
	virtual bool OnPetNotifyHP(gplayer_imp *pImp, pet_manager * pMan,pet_data *pData,const XID &who,int stamp,const msg_pet_hp_notify & info) = 0;
	virtual bool OnPetDeath(gplayer_imp *pImp, pet_manager * pMan,pet_data *pData,const XID &who,int stamp) = 0;
	virtual bool OnPetCtrl(gplayer_imp * pImp, pet_manager * pMan,pet_data *pData,int cur_target,int pet_cmd, const void * buf, size_t size) = 0;
	virtual void OnKillMob(gplayer_imp * pImp, pet_manager * pMan, pet_data * pData, int mob_level) = 0;
	virtual void OnNotifyMasterInfo(gplayer_imp * pImp,pet_manager * pMan, pet_data * pData, bool at_once) = 0;
	virtual void PreSwitchServer(gplayer_imp * pImp, pet_manager * pman, pet_data * pData) = 0;
	virtual void PostSwitchServer(gplayer_imp * pImp, pet_manager * pman, pet_data * pData) = 0;
	virtual void OnMasterBeAttacked(gplayer_imp * pImp,pet_manager *pman, pet_data *pData,const XID &who) = 0; 
	virtual bool OnChangeName(gplayer_imp * pImp,pet_manager *pman, pet_data *pData,const char *name, size_t len) = 0; 
	virtual bool OnForgetSkill(gplayer_imp * pImp,pet_manager *pman, pet_data *pData, int skill_id) = 0;
	virtual bool OnLearnSkill(gplayer_imp * pImp,pet_manager *pMan,pet_data *pData,int skill_id, int * level_result) = 0;
	virtual bool OnDyePet(gplayer_imp * pImp,pet_manager *pMan,pet_data *pData,unsigned short color) = 0;
	virtual bool OnEvolution(gplayer_imp *pImp,pet_manager *pMan,pet_data *pData,int evolution_pet_id,int pet_nature,int skill1,int level1,int skill2,int level2) = 0;
	virtual void OnMountSpeedEnChanged(gplayer_imp *pImp,pet_manager * pMan, pet_data * pData) = 0;
protected:
	inline void ClearCurPet(gplayer_imp * pImp, pet_manager * pMan)
	{
		pMan->ClearCurPet(pImp);
	}
};

inline void pet_manager::Heartbeat(gplayer_imp * pImp)
{
	if(_cur_active_pet >= 0)
	{
		DoHeartbeat(pImp);
	}
}


//以下是植物宠管理器,因植物切换服务器消失，所以不需保存
struct msg_plant_pet_hp_notify;
struct plant_pet_data
{
	XID id;
	int tid;
	int plant_group;
	int group_limit;
	int life;
	float hp_factor;
	int hp;
	float mp_factor;
	int mp;
	size_t plant_state;
	bool is_suicide;
};

class plant_pet_manager
{
public:
	enum
	{
		PET_STATE_THRESHOLD		 = 15,	//15秒内宠物没有通知信息，则召回宠物
		NOTIFY_MASTER_TIME		 = 15,	//每15秒通知宠物master info
	};
	
	enum
	{
		PLANT_DEATH = 0,	//植物死亡
		PLANT_LIFE_EXHAUST,	//植物寿命到
		PLANT_OUT_OF_RANGE,	//植物超出范围
		PLANT_SUICIDE,		//植物自爆
		PLANT_GROUP_LIMIT,	//植物数量超出组上限
	};
	
	typedef abase::vector<plant_pet_data, abase::fast_alloc<> > PLANT_LIST;
	PLANT_LIST _plant_list;					//植物列表
	size_t _plant_notify_counter;

public:
	plant_pet_manager():_plant_notify_counter(0){}

	bool ActivePlant(gplayer_imp * pImp, pet_data & data, int life, int skill_level, const XID & target, char force_attack);
	bool PlantSuicide(gplayer_imp * pImp, float distance, const XID & target, char force_attack);
	void Heartbeat(gplayer_imp * pImp);
	void NotifyMasterInfo(gplayer_imp * pImp);	
	void PreSwitchServer(gplayer_imp * pImp);
	void NotifyStartAttack(gplayer_imp *pImp, const XID & target, char force_attack);
	void PlayerBeAttacked(gplayer_imp * pImp, const XID & target);
public:
	bool PlantDeath(gplayer_imp * pImp,const XID & who , int stamp);
	bool NotifyPlantHP(gplayer_imp * pImp,const XID & who , int stamp,const msg_plant_pet_hp_notify & info);
	bool PlantDisappear(gplayer_imp * pImp,const XID & who , int stamp);

};	
#endif

