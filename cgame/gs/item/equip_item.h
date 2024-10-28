#ifndef __ONLINEGAME_GS_EQUIP_ITEM_H__
#define __ONLINEGAME_GS_EQUIP_ITEM_H__

#include <stddef.h>
#include <octets.h>
#include <common/packetwrapper.h>
#include "../item.h"
#include "../config.h"
#include "item_addon.h"
#include "set_addon.h"
#include <crc.h>

class addon_equip_item : public item_body
{
	//这个基础类只负责管理addon，具体使用和生效不由其负责
protected:
	ADDON_LIST _active_addon;	//保存需要激活的属性
	ADDON_LIST _total_addon;	//保存所有的属性
	const ADDON_LIST * _extra_addon;//额外附加的addon 主要用于套装和其它属性
	addon_data _use_addon;		//保存需要使用的属性
	int	_addon_expire_date;		//保存过期附加属性的过期时间
protected:
	addon_equip_item()
	{
		_use_addon.id= -1;
		_extra_addon = NULL;
		_addon_expire_date = 0;
	}
	


	void LoadAddOn(archive &ar)
	{
		size_t count;
		int argcount;
		ar >> count;
		if(count <0 || count > 128)
		{
			throw -100;
		}

		for(size_t i = 0; i < count ; i++)
		{
			addon_data entry;
			memset(&entry,0,sizeof(entry));

			ar >> entry.id;
			argcount = addon_manager::GetArgCount(entry.id);
			for(int j= 0; j < argcount ;j ++)
			{
				ar >> entry.arg[j];
			}
			_total_addon.push_back(entry);
		}
		
		//试图进行对sepc_addon进行定位
		_extra_addon = set_addon_manager::GetAddonList(_tid);
	}

	void SaveAddOn(archive & ar)
	{
		size_t count;
		count = _total_addon.size();
		ar << count;
		for(size_t i = 0; i < count;i ++)
		{
			int id = _total_addon[i].id;
			int argcount = addon_manager::GetArgCount(id);
			ar << id;
			for(int j = 0; j < argcount; j++)
			{
				ar << _total_addon[i].arg[j];
			}
		}
	}
	virtual bool IsItemCanUse(item::LOCATION l)
	{
		return 	_use_addon.id != -1;
	}
	virtual void ClearData()
	{
		_use_addon.id = -1;
		_addon_expire_date = 0;
	}

	
};


//这几个有关本体的结构只是用于参考，而不是用来处理数据的
//因为数据会被之间放入装备代码之中
//但是在特定的对象里仍然需要保存这些数据，这主要是用于存盘的考虑。
struct weapon_essence
{
	enum
	{
		WEAPON_TYPE_MELEE = 0,
		WEAPON_TYPE_RANGE = 1,
		WEAPON_TYPE_MELEE_ASN = 2,	//刺客使用的近程武器，除敏捷影响物攻外，其他与近程相同
	};
	short weapon_type;		//武器类别 对应模板里的进程远程标志
	short weapon_delay;		//武器的攻击延迟时间，以50ms为单位
	int weapon_class;		//武器子类 比如刀剑 长兵等
	int weapon_level;		//武器等级 有些操作需要等级匹配 
	int require_projectile;		//需要弹药的类型，见item.h
	int damage_low;			//物理攻击最小加值
	int damage_high;		//物理攻击最大加值
	int magic_damage_low;		//魔法攻击
	int magic_damage_high;		//魔法攻击
	int attack_speed;		//攻击速度
	float attack_range;		//攻击距离
	float attack_short_range; 	//攻击的最短距离（适用于远程武器）
};

template <typename WRAPPER>
WRAPPER & operator<<(WRAPPER & wrapper, const weapon_essence & es)
{
	wrapper.push_back(&es,sizeof(es));
	return wrapper;
}

template <typename WRAPPER>
WRAPPER & operator>>(WRAPPER & wrapper, weapon_essence & es)
{
	wrapper.pop_back(&es,sizeof(es));
	return wrapper;
}

struct projectile_essence 
{
	int projectile_type;		//弹药类型 
	int enhance_damage;		//增强武器的攻击力 
	int scale_enhance_damage;	//按照比例增强的攻击力 
	int weapon_level_low;
	int weapon_level_high;
};


template <typename WRAPPER>
WRAPPER & operator<<(WRAPPER & wrapper, const projectile_essence & es)
{
	return wrapper.push_back(&es,sizeof(es));
}

template <typename WRAPPER>
WRAPPER & operator>>(WRAPPER & wrapper, projectile_essence & es)
{
	return wrapper.pop_back(&es,sizeof(es));
}

struct armor_essence
{
	int defense;
	int armor;
	int mp_enhance;
	int hp_enhance;
	int resistance[MAGIC_CLASS];
};

template <typename WRAPPER>
WRAPPER & operator<<(WRAPPER & wrapper, const armor_essence & es)
{
	return wrapper.push_back(&es,sizeof(es));
}

template <typename WRAPPER>
WRAPPER & operator>>(WRAPPER & wrapper, armor_essence & es)
{
	return wrapper.pop_back(&es,sizeof(es));
}

struct decoration_essence
{
	int damage;
	int magic_damage;
	int defense;
	int armor;
	int resistance[MAGIC_CLASS];
};

template <typename WRAPPER>
WRAPPER & operator<<(WRAPPER & wrapper, const decoration_essence & es)
{
	return wrapper.push_back(&es,sizeof(es));
}

template <typename WRAPPER>
WRAPPER & operator>>(WRAPPER & wrapper, decoration_essence & es)
{
	return wrapper.pop_back(&es,sizeof(es));
}

class equip_item : public addon_equip_item
{
public:
	struct  base_data
	{
		int damage_low;		//物理攻击最小加值
		int damage_high;	//物理攻击最大加值
		int magic_damage_low;	//魔法攻击
		int magic_damage_high;	//魔法攻击
		int defense;		//防御加值
		int armor;		//闪避率加值
	};

	struct scale_data
	{
		int damage;
		int magic_damage;
		int defense;
		int armor;
	};

	struct made_tag_t
	{
		char tag_type;
		unsigned char tag_size;
		char tag_content[MAX_USERNAME_LENGTH];
	};

protected:
	abase::octets _raw_data;	//保存原始对象数据,当对象更新后需要重新生成
	made_tag_t _m_tag;		//制造者标签
	unsigned short _crc;
	unsigned short _modify_mask;	//高位的mask

public:
	struct __prerequisition : public prerequisition
	{
		template <typename WRAPPER>
			WRAPPER & operator <<(WRAPPER & wrapper)
			{
				return 	wrapper >> level >> race
						>> strength >> vitality
						>> agility  >> energy
						>> durability >> max_durability;
			}

		template <typename WRAPPER>
			WRAPPER & operator >>(WRAPPER & wrapper)
			{
				return 	wrapper << level << race
						<< strength << vitality
						<< agility  << energy
						<< durability << max_durability;
			}
	} _base_limit;


	/*
	 *	将所有属性都放在这里的目的是供各种AddOn修改
	 */
	struct base_data 	_base_param;
	struct scale_data 	_base_param_percent;
	

public:
	DECLARE_SUBSTANCE(equip_item);
	equip_item()
	{
		memset(&_base_limit,0,sizeof(_base_limit));
		memset(&_base_param,0,sizeof(_base_param));
		memset(&_base_param_percent,0,sizeof(_base_param_percent));
		_crc = 0;
	}
	virtual void ClearData()
	{
		addon_equip_item::ClearData();
		memset(&_base_param,0,sizeof(_base_param));
		memset(&_base_param_percent,0,sizeof(_base_param_percent));
	}
	~equip_item();

public:
	virtual void GetItemData(const void ** data, size_t &len)
	{
		*data = _raw_data.begin();
		len = _raw_data.size();
	}

	virtual bool Save(archive & ar);
	virtual bool Load(archive & ar);
	size_t LoadMadeTag(archive & ar);
	void SaveMadeTag(archive & ar,size_t ess_size);
protected:
	virtual void * GetEssence() = 0;
	virtual size_t GetEssenceSize() = 0;
	virtual void OnTakeOut(item::LOCATION l,size_t pos,size_t count, gactive_imp* obj);
	virtual bool VerifyRequirement(item_list & list,gactive_imp* obj);
	virtual bool LoadEssence(archive & ar) = 0;	//装载本体属性
	virtual bool SaveEssence(archive & ar) = 0;	//装载本体属性
	virtual void UpdateEssence() = 0;		//更新基础属性到通用数据中
	virtual void UpdateData() = 0;			//根据各种属性和增强属性对数据进行最后的调整
	virtual void SetSocketCount(size_t count) {}
	virtual void SetSocketType(size_t index, int type) { ASSERT(false);}
	virtual size_t GetSocketCount() { return 0;}
	virtual int GetSocketType(size_t index) { return 0;}
	virtual void EssenceActivate(item::LOCATION l,gactive_imp* obj) = 0;
	virtual void EssenceDeactivate(item::LOCATION l,gactive_imp* obj) = 0;
	virtual bool ArmorDecDurability(int amount) 
	{ 
		ASSERT(CheckRawDurability());
		_base_limit.durability -= amount;
		bool bRst = false;
		if(_base_limit.durability < 0)
		{
			_base_limit.durability = 0;
			bRst = true; //表示需要更新装备信息
		}
		UpdateRawDurability();
		return bRst;
	}

	virtual void GetDurability(int &dura,int &max_dura) 
	{ 
		dura = _base_limit.durability; 
		max_dura = _base_limit.max_durability;
	}
	virtual void Repair()
	{
		_base_limit.durability = _base_limit.max_durability;
		UpdateRawDurability();
	}

	virtual unsigned short GetDataCRC() { return _crc; }
	virtual bool RegenAddon(int item_id,bool (*regen_addon)(int item_id, addon_data& ent));
	virtual int RefineAddon(int addon_id, int & level_result, float adjust[4], float adjust2[12]);
	virtual int RefineAddon(int addon_id, int & level_result, float adjust[4], float adjust2[12], int material_id);
	virtual int GetAddonExpireDate(){ return _addon_expire_date; }
	virtual int RemoveExpireAddon(int cur_t);	//返回更新后的addon_expire_date
	virtual bool Sharpen(addon_data * addon_list, size_t count, int sharpener_gfx);
	virtual bool Engrave(addon_data * addon_list, size_t count);
	virtual size_t GetEngraveAddon(addon_data * addon_list, size_t max_count);
    virtual bool InheritAddon(addon_data * addon_list, size_t count);
    virtual size_t GetCanInheritAddon(addon_data * addon_list, size_t max_count, int ex_addon_id);
	virtual int RegenInherentAddon();
	virtual int GetRefineLevel(int addon_id);
	virtual int SetRefineLevel(int addon_id , int level);
	virtual void OnRefreshItem();
	virtual bool Sign(unsigned short color, const char * signature, unsigned int signature_len);
private:
	virtual void OnActivate(item::LOCATION l,size_t pos,size_t count, gactive_imp* obj);
	virtual void OnDeactivate(item::LOCATION l,size_t pos,size_t count,gactive_imp* obj);
	virtual int OnUse(item::LOCATION l,gactive_imp * obj,size_t count);
	virtual int GetIdModify();

protected:

	void UpdateAddOn(const addon_data & data)
	{
		int mask = addon_manager::CheckAndUpdate(data,this);
		if(mask == addon_manager::ADDON_MASK_INVALID)
		{
			throw -120;
		}

		if(mask & addon_manager::ADDON_MASK_ACTIVATE)
		{
			_active_addon.push_back(data);
		}

		if(mask & addon_manager::ADDON_MASK_USE)
		{
			if(_use_addon.id == -1)
				_use_addon = data;
		}
		//计算附加属性的过期时间	
		int tmp = addon_manager::GetExpireDate(data);
		if(tmp > 0)
		{
			if(_addon_expire_date == 0)
				_addon_expire_date = tmp;
			else if(_addon_expire_date > tmp)
				_addon_expire_date = tmp;				
		}
	}

	void UpdateAddOn()
	{
		size_t i;
		for(i = 0; i < _total_addon.size(); i ++)
		{
			UpdateAddOn(_total_addon[i]);
		}
	}

	void LoadLimit(archive &ar);
	
	void SaveLimit(archive & ar)
	{
		_base_limit >> ar;
	}
	
	void LoadSocketData(archive & ar)
	{
		unsigned short count;
		ar >> count >> _modify_mask;
		if(count > MAX_SOCKET_COUNT) throw -103;
		SetSocketCount(count);
		for(size_t i = 0; i < count; i++)
		{
			int type;
			ar >> type;
			SetSocketType(i,type);
		}
	}

	void SaveSocketData(archive & ar)
	{
		unsigned short count = GetSocketCount();
		ar << count << _modify_mask;
		for(size_t i = 0; i < count; i++)
		{
			ar << GetSocketType(i);
		}
	}
	
	bool CheckRawDurability()
	{
		return ((prerequisition*)(_raw_data.begin()))->durability== _base_limit.durability;
	}

	void UpdateRawDurability()
	{
		((prerequisition*)(_raw_data.begin()))->durability = _base_limit.durability;
	}

	bool CheckRawRace()
	{
		return ((prerequisition*)(_raw_data.begin()))->race == _base_limit.race;
	}

	void UpdateRawRace()
	{
		((prerequisition*)(_raw_data.begin()))->race = _base_limit.race;
	}

	void CalcCRC()
	{
		ASSERT(_raw_data.size() > sizeof(prerequisition));
		_crc = crc16( (unsigned char *)_raw_data.begin() + sizeof(prerequisition),_raw_data.size() - sizeof(prerequisition));
	}
	
};


/*
	struct
	{
		限制条件;
		short 本体结构大小;
		制造者内容  //这是为了和以前的物品兼容
		char 本体结构[];
		int 孔槽的数目;
		int 孔槽的类型表[];
		int 属性表
		char 属性表[];		//addon
	};

*/

/*
 *	可以嵌入物体的对象
 */
class socket_item : public equip_item
{
protected:
	abase::vector<int> _socket_list;
protected:
	virtual bool OnHasSocket() { return !_socket_list.empty();}
	virtual bool OnInsertChip(int chip_type,addon_data * data, size_t count);
	virtual bool OnClearChips();
	virtual void SetSocketCount(size_t count); 
	virtual void SetSocketType(size_t index, int type);
	virtual size_t GetSocketCount(); 
	virtual int GetSocketType(size_t index);
	virtual void AfterChipChanged() = 0;
	virtual bool HasAddonAtSocket(unsigned char s_idx,int s_type) { return s_idx >= _socket_list.size() ? false : _socket_list[s_idx] == s_type;}
	
	virtual	bool RemoveAddon(unsigned char s_idx);
public:
	virtual bool ModifyAddonAtSocket(unsigned char s_idx,int stone_id);

};

class weapon_item : public socket_item
{
protected:
	weapon_essence _ess;
public:
	DECLARE_SUBSTANCE(weapon_item);

	static int GetWeaponType(const item_data * pData)
	{
		weapon_item * pTmp;
		const char * pos = ((char*)pData->item_content) + sizeof(pTmp->_base_limit);
		unsigned char namesize = *(pos + sizeof(short) + sizeof(char));
		ASSERT(namesize <=MAX_USERNAME_LENGTH);
		pos += sizeof(int) + offsetof(weapon_essence,weapon_type) + namesize;
		short type = *(const short*)pos;
		ASSERT(type == 0 || type == 1 || type == 2);
		return type;
	}
	virtual ITEM_TYPE GetItemType()  { return ITEM_TYPE_WEAPON;}
	virtual int MakeSlot(gactive_imp*, int& count, unsigned int material_id = 0, int material_count = 0);
protected:
	virtual void * GetEssence()  {return &_ess;}
	virtual size_t GetEssenceSize() {return sizeof(_ess);}
	virtual bool LoadEssence(archive & ar);
	virtual bool SaveEssence(archive & ar);
	virtual void EssenceActivate(item::LOCATION l,gactive_imp* obj);
	virtual void EssenceDeactivate(item::LOCATION l,gactive_imp* obj);
	virtual void UpdateEssence();
	virtual void UpdateData();
	virtual int OnGetProjectileReqType() const ;
	virtual void OnAfterAttack(item_list & list, bool * pUpdate);
	virtual bool ArmorDecDurability(int amount) { ASSERT(false); return false;}
	virtual void AfterChipChanged(); 
	virtual void SetSocketAndStone(int count, int * stone_type);
	virtual int Is16Por9JWeapon();
};

class armor_item : public socket_item
{
protected:
	armor_essence _ess;
public:
	DECLARE_SUBSTANCE(armor_item);
	armor_item * Clone() const { return new armor_item(*this);}

	virtual ITEM_TYPE GetItemType()  { return ITEM_TYPE_ARMOR;}
	virtual int MakeSlot(gactive_imp*, int& count, unsigned int material_id = 0, int material_count = 0);
protected:
	virtual void * GetEssence()  {return &_ess;}
	virtual size_t GetEssenceSize() {return sizeof(_ess);}
	virtual bool LoadEssence(archive & ar);
	virtual bool SaveEssence(archive & ar);
	virtual void EssenceActivate(item::LOCATION l,gactive_imp* obj);
	virtual void EssenceDeactivate(item::LOCATION l,gactive_imp* obj);
	virtual void UpdateEssence();
	virtual void UpdateData();
	virtual bool ArmorDecDurability(int amount) 
	{ 
		_base_limit.durability -= amount;
		bool bRst = false;
		if(_base_limit.durability < 0)
		{
			_base_limit.durability = 0;
			bRst = true; //表示需要更新装备信息
		}
		UpdateRawDurability();
		return bRst;
	}
	virtual void AfterChipChanged(); 
	virtual void SetSocketAndStone(int count, int * stone_type);
};

class melee_weapon_item: public weapon_item
{
public:
	DECLARE_SUBSTANCE(melee_weapon_item);
	virtual melee_weapon_item * Clone() const { return new melee_weapon_item(*this);}
private:
	virtual bool OnCheckAttack(item_list & list);
};

class range_weapon_item: public weapon_item
{
public:
	DECLARE_SUBSTANCE(range_weapon_item);
	virtual range_weapon_item * Clone() const { return new range_weapon_item(*this);}
private:
	virtual bool OnCheckAttack(item_list & list);
	virtual bool VerifyRequirement(item_list & list,gactive_imp* obj);
};

class projectile_equip_item : public socket_item
{
	projectile_essence _ess;

public:
	DECLARE_SUBSTANCE(projectile_equip_item);
	virtual projectile_equip_item * Clone() const { return new projectile_equip_item(*this);}
	virtual ITEM_TYPE GetItemType()  { return ITEM_TYPE_PROJECTILE;}
protected:
	virtual void * GetEssence()  {return &_ess;}
	virtual size_t GetEssenceSize() {return sizeof(_ess);}
	virtual bool LoadEssence(archive & ar);
	virtual bool SaveEssence(archive & ar);
	virtual void EssenceActivate(item::LOCATION l,gactive_imp* obj);
	virtual void EssenceDeactivate(item::LOCATION l,gactive_imp* obj);
	virtual void UpdateEssence();
	virtual void UpdateData();
	virtual bool VerifyRequirement(item_list & list,gactive_imp* obj);
	virtual int OnGetProjectileType() const;
	virtual void AfterChipChanged() {}
};

class decoration_equip_item : public socket_item
{
	decoration_essence _ess;
public:
	DECLARE_SUBSTANCE(decoration_equip_item);
	virtual decoration_equip_item * Clone()  const { return new decoration_equip_item(*this);}
	virtual ITEM_TYPE GetItemType()  { return ITEM_TYPE_DECORATION;}
    virtual int MakeSlot(gactive_imp*, int& count, unsigned int material_id = 0, int material_count = 0);
protected:
	virtual void * GetEssence()  {return &_ess;}
	virtual size_t GetEssenceSize() {return sizeof(_ess);}
	virtual bool LoadEssence(archive & ar);
	virtual bool SaveEssence(archive & ar);
	virtual void EssenceActivate(item::LOCATION l,gactive_imp* obj);
	virtual void EssenceDeactivate(item::LOCATION l,gactive_imp* obj);
	virtual void UpdateEssence();
	virtual void UpdateData();
    virtual void AfterChipChanged() { /* 无光效效果 */ };
    virtual void SetSocketAndStone(int count, int* stone_type);
};


#endif

