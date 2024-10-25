#ifndef __ONLINEGAME_GS_FATE_RING_MANAGER_H__
#define __ONLINEGAME_GS_FATE_RING_MANAGER_H__

#include "vector.h"
#include "hashmap.h"
#include "config.h"

class itemdataman;
class gplayer_imp;

struct fatering_extend
{
	int max_hp;
	int damage_low;
	int damage_high;
	int magic_damage_low;
	int magic_damage_high;
	int defense;  
	int resistance[MAGIC_CLASS];
	int vigour;
};

//每一个等级的命轮属性
struct fatering_property
{
	int power;	//升级所需能量
	int require_level;	//需要玩家等级
	float ratio;	//每级的属性调整比例
};

//每种命轮的属性
class fatering_essence
{
public:
	fatering_essence():type(0) 
	{
		memset(&_extend,0,sizeof(_extend));
	}

	inline void SetType(int t) {type=t;}
	
	inline void InsertEnum(const fatering_property & fp)
	{
		fatering_props.push_back(fp);
	}

	void SetExtend(int max_hp, int damage, int magic_damage, int defense, const int res[5], int vigour)
	{
		_extend.max_hp = max_hp;
		_extend.damage_low = damage;
		_extend.damage_high = damage;
		_extend.magic_damage_low = magic_damage;
		_extend.magic_damage_high = magic_damage;
		_extend.defense = defense;
		_extend.vigour = vigour;

		int total = max_hp + damage + magic_damage + defense + vigour;

		for (size_t i = 0; i < MAGIC_CLASS; i++)
		{
			_extend.resistance[i] = res[i];
			total += res[i];
			ASSERT(_extend.resistance[i] >= 0);
		}
		ASSERT(max_hp >= 0 && damage >= 0 && magic_damage >= 0 && defense >= 0 && vigour >= 0 && total > 0);
	}

	bool GetFateRingLevelUpRequire(int level, int & require_lv, int & require_power) const
	{
		const fatering_property * fp = GetFateRingProperty(level);
		if (!fp)
			return false;

		require_lv = fp->require_level;
		require_power = fp->power;
		return true;
	}

	bool EnhanceFateRing(gplayer_imp * imp, int level) const;
	bool ImpairFateRing(gplayer_imp * imp, int level) const;

private:
	inline const fatering_property * GetFateRingProperty(int level) const
	{
		if (level < 1 || level > PLAYER_FATE_RING_MAX_LEVEL)
			return NULL;
		return &(fatering_props[level-1]);
	}

	inline float GetFateRingRatio(int level) const
	{
		const fatering_property * fp = GetFateRingProperty(level);
		if (!fp)
			return 0.f;

		return fp->ratio;
	}

	typedef abase::vector<fatering_property> FATERING_PROPS_VEC;
	FATERING_PROPS_VEC fatering_props;

	fatering_extend _extend;	//该系命轮满级增加的属性
	int type;		//命轮类型
};

/*
 * 命轮管理器
 */
class fatering_manager
{
public:
	fatering_manager() {}

	bool Initialize(itemdataman & dataman);

	inline bool GetFateRingLevelUpRequire(int index, int level, int & require_lv, int & require_power)
	{
		 const fatering_essence * fe = GetFateRingEssence(index);
		 if (!fe)
			 return false;

		 return fe->GetFateRingLevelUpRequire(level,require_lv,require_power);
	}

	bool EnhanceFateRing(gplayer_imp * imp, int index, int level) const;
	bool ImpairFateRing(gplayer_imp * imp, int index, int level) const;

private:
	inline const fatering_essence * GetFateRingEssence(int index) const
	{
		FATERING_MAP::const_iterator it = faterings.find(index);
		if (it != faterings.end())
			return &(it->second);

		return NULL;
	}

	typedef abase::hash_map<int, fatering_essence> FATERING_MAP;
	FATERING_MAP faterings;
};

#endif
