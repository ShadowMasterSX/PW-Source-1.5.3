#ifndef __ONLINEGAME_GS_ITEM_ADDON_H__
#define __ONLINEGAME_GS_ITEM_ADDON_H__

#include <hashmap.h>
#include <amemory.h>
#include "../template/exptypes.h"
#include "../config.h"

class equip_item;
class gactive_imp;
class addon_handler;

struct  addon_data
{
	int id;
	int arg[3];
};

typedef abase::vector<addon_data,abase::fast_alloc<> > ADDON_LIST;

struct addon_entry
{
	int id;
	addon_handler * handler;
	int ratio_calc;
	addon_entry():id(-1),handler(NULL),ratio_calc(0)
	{}
};

struct prerequisition
{
	short level;
	short race;
	short strength;
	short vitality;
	short agility;
	short energy;
	int durability;
	int max_durability;
}; 

//这里的对象会在最初初始化，不能使用ASmallObject
class addon_handler
{
public:
	virtual ~addon_handler(){}
	virtual int GenerateParam(int datatype,addon_data & data,int argnum) = 0;
	virtual int ApplyAtGeneration(const addon_data &, void * essence,size_t size,prerequisition * require) { return 0;}
	virtual int TestUpdate() = 0;
	virtual int UpdateItem(const addon_data & , equip_item *) = 0;
	virtual int Use(const addon_data & , equip_item *, gactive_imp *) = 0;
	virtual int Activate(const addon_data & , equip_item *, gactive_imp *) = 0;
	virtual int Deactivate(const addon_data & , equip_item *, gactive_imp *) = 0;
	virtual bool IsRandomAddon() { return false;}
//	virtual int OnRefineData(addon_data & srcdata,const addon_data & newdata,bool reverse) {return -1;}
	virtual int GetExpireDate(const addon_data & data){ return 0; }
};



class addon_manager
{
public:
	typedef abase::hashtab<addon_entry,int,abase::_hash_function>  ADDON_MAP;
private:
	addon_manager():_map(500){}
	ADDON_MAP _map;

	addon_entry * Find(int id)
	{
		ADDON_MAP::iterator it = _map.find(id);
		if(it == _map.end())
		{
			return NULL;
		}
		return it.value();
	}

	int __CheckAndUpdate(const addon_data & data,equip_item * item)
	{	
		int id = data.id & ADDON_PURE_TYPE_MASK;	//清除可能的嵌入对象标志再进行
		addon_entry * ent = Find(id);
		if(ent == NULL) return ADDON_MASK_INVALID;
		return ent->handler->UpdateItem(data,item);
	}

	int __TestUpdate(const addon_data &data)
	{
		int id = data.id & ADDON_PURE_TYPE_MASK;	//清除可能的嵌入对象标志再进行
		addon_entry * ent = Find(id);
		if(ent == NULL) return ADDON_MASK_INVALID;
		return ent->handler->TestUpdate();
	}

	int __Use(const addon_data & data,equip_item * item,gactive_imp * imp)
	{
		int id = data.id & ADDON_PURE_TYPE_MASK;	//清除可能的嵌入对象标志再进行
		ADDON_MAP::iterator it = _map.find(id);
		if(it == _map.end()) return ADDON_MASK_INVALID;
		it->handler->Use(data,item,imp);
		return 0;
	}

	inline addon_data __CalcRatio(const addon_data& data, int type, float ratio)
	{
		if(!type) return data;
		addon_data res = data;
		switch(type)
		{
			case ADDON_PARAM_RATIO_ALL_INT:
				for(int n = 0; n < 3; ++n) 
					res.arg[n] = (int)(res.arg[n] * ratio);
				break;
			case ADDON_PARAM_RATIO_ALL_FLOAT:	
				for(int n = 0; n < 3; ++n)
				{
					float fv = (*(float*)&(res.arg[n]) ) * ratio;
					memcpy(&(res.arg[n]),&fv,sizeof(res.arg[n]));
				}
				break;
		}
		return res;
	}

	int __Activate(const addon_data & data, equip_item *item, gactive_imp * imp, float ratio)
	{
		int id = data.id & ADDON_PURE_TYPE_MASK;	//清除可能的嵌入对象标志再进行
		ADDON_MAP::iterator it = _map.find(id);
		if(it == _map.end()) return ADDON_MASK_INVALID;
		addon_data rdata = __CalcRatio(data,it->ratio_calc,ratio);
		it->handler->Activate(rdata,item,imp);
		return 0;
	}

	int __Deactivate(const addon_data & data, equip_item *item, gactive_imp * imp, float ratio)
	{
		int id = data.id & ADDON_PURE_TYPE_MASK;	//清除可能的嵌入对象标志再进行
		ADDON_MAP::iterator it = _map.find(id);
		if(it == _map.end()) return ADDON_MASK_INVALID;
		addon_data rdata = __CalcRatio(data,it->ratio_calc,ratio);
		it->handler->Deactivate(rdata,item,imp);
		return 0;
	}
	
	void __Insert(const addon_entry & ent)
	{
		ASSERT(GetArgCount(ent.id) == 0);
		ADDON_MAP::iterator it = _map.find(ent.id);
		ASSERT(it == _map.end());
		if(!_map.put(ent.id, ent))
		{
			ASSERT(false);
			throw -101;
		}
	}

	int __GenerateAddonArg(DATA_TYPE type,addon_data & data,int arg_num)
	{
		int id = data.id & ADDON_PURE_TYPE_MASK;
		ADDON_MAP::iterator it = _map.find(id);
		if(it == _map.end()) return -1;
		return 	it->handler->GenerateParam(type,data,arg_num);
	}

	int __UpdateAddonEssData(const addon_data & data, void * essence,size_t ess_size,prerequisition * require)
	{
		int id = data.id & ADDON_PURE_TYPE_MASK;
		ADDON_MAP::iterator it = _map.find(id);
		if(it == _map.end()) {
			ASSERT(false);
			return -1;
		}
		return 	it->handler->ApplyAtGeneration(data,essence,ess_size,require);
	}

/*	int __RefineAddonData(addon_data & srcdata, const addon_data &newdata, bool reverse)
	{
		if(srcdata.id != newdata.id) return -1;
		int id = srcdata.id & ADDON_PURE_TYPE_MASK;
		ADDON_MAP::iterator it = _map.find(id);
		if(it == _map.end()) {
			ASSERT(false);
			return -1;
		}
		return 	it->handler->OnRefineData(srcdata, newdata,reverse);
	}*/

	bool __IsRandomAddon(int addon_id)
	{
		int id = addon_id & ADDON_PURE_TYPE_MASK;
		ADDON_MAP::iterator it = _map.find(id);
		if(it == _map.end()) {
			return false;
		}
		return 	it->handler->IsRandomAddon();
	}
	
	int __GetExpireDate(const addon_data & data)
	{
		int id = data.id & ADDON_PURE_TYPE_MASK;
		ADDON_MAP::iterator it = _map.find(id);
		if(it == _map.end()) {
			return 0;
		}
		return 	it->handler->GetExpireDate(data);
	}

public:
	friend class addon_inserter;
	static addon_manager & GetInstance()
	{
		static addon_manager __instance;
		return __instance;
	}

//附加属性对应的作用模版
	enum
	{
		ADDON_MASK_INVALID	= -1,
		ADDON_MASK_STATIC 	= 1,		//静态属性，在item Load/Refresh时调用
		ADDON_MASK_ACTIVATE	= 2,		//增强(filter)属性，在装备的时候调用
		ADDON_MASK_USE		= 4,		//物品可以使用，在使用的时候调用，同时只能有一个
		ADDON_MASK_ESSENCE	= 8,		//本体属性，在物品生成时作用于ESSENCE
	};
//附加属性参数百分比计算方式	
	enum
	{
		ADDON_PARAM_RATIO_NO = 0,		//默认 不受百分比影响
		ADDON_PARAM_RATIO_ALL_INT,		//所有参数均为int
		ADDON_PARAM_RATIO_ALL_FLOAT,	//所有参数均为float
	};

/**
 *		这个函数负责检查addon的起作用方式，如果是静态方式的话,
 *		此函数会将数据更新的物品的基础属性和增强属性之中
 *		它的返回值是一个mask，代表在何时起作用
 *		附加的gactive_imp 是供不时之需
 */
	static int CheckAndUpdate(const addon_data & data,equip_item * item)
	{
		return GetInstance().__CheckAndUpdate(data,item);
	}


	static int TestUpdate(const addon_data &data)
	{
		return GetInstance().__TestUpdate(data);
	}

/*
 *	这个函数表明使用特定的装备,只能是无穷次使用的,但是这样的话cooldown如何加?
 *	考虑记录在equp_item里面 ,这样在存盘时也需要额外考虑这一属性
 */
	static int Use(const addon_data & data,equip_item *item,gactive_imp *imp)
	{
		return GetInstance().__Use(data,item,imp);
	}
/*
 *	返回一个Addon是否随机属性
 */
	static bool IsRandomAddon(int addon_id) 
	{ 
		return GetInstance().__IsRandomAddon(addon_id);
	}

/*
 *	这个函数负责激活类型的addon，当物品被装备时，这个函数会被调用到
 */
	static int Activate(const addon_data & data, equip_item *item, gactive_imp * imp, float ratio=1.0f)
	{
		return GetInstance().__Activate(data,item,imp,ratio);
	}

/*
 *	这个函数是激活函数的反函数，当物品被卸下时，这个函数会被调用到
 */
	static int Deactivate(const addon_data & data, equip_item *item, gactive_imp *imp, float ratio=1.0f)
	{
		return GetInstance().__Deactivate(data,item,imp,ratio);
	}
	
	static int GenerateAddonArg(DATA_TYPE type,addon_data & data,int arg_num)
	{
		return GetInstance().__GenerateAddonArg(type,data,arg_num);
	}

	static int UpdateAddonEssData(const addon_data &data, void * essence,size_t ess_size,prerequisition *require)
	{
		return GetInstance().__UpdateAddonEssData(data, essence,ess_size,require);
	}

//	static int RefineAddonData(addon_data & srcdata, const addon_data &newdata, bool reverse)
//	{
//		return GetInstance().__RefineAddonData(srcdata,newdata,reverse);
//	}

	static int GetExpireDate(const addon_data & data)
	{
		return GetInstance().__GetExpireDate(data);
	}
	
public:
	//使用type id的13和14位表示参数的个数
	//使用type id的15位表示是否镶嵌属性
	//使用type id的17位表示是否镌刻属性
    //使用type id的18位表示是否是生产5继承的属性
	enum
	{
		ADDON_EMBEDDED	= 0x8000,
		ADDON_SET_ADDON	= 0x10000,	//未使用
		ADDON_ENGRAVED	= 0x20000,
        ADDON_INHERITED   = 0x40000,
		ADDON_PURE_TYPE_MASK = ~(ADDON_ENGRAVED | ADDON_SET_ADDON | ADDON_EMBEDDED | ADDON_INHERITED | 0x6000),
	};
	static inline void SetAddOnEmbedded(addon_data & data)
	{
		ASSERT((data.id & ADDON_EMBEDDED) == 0);
		data.id |= ADDON_EMBEDDED;
	}

	static inline void ClearAddOnEmbedded(addon_data & data)
	{
		ASSERT(data.id & ADDON_EMBEDDED);
		data.id &= ~ADDON_EMBEDDED;
	}

	static inline bool IsAddOnEmbedded(const addon_data & data)
	{
		return data.id & ADDON_EMBEDDED;
	}
	
	static inline void SetAddOnEngraved(addon_data & data)
	{
		ASSERT((data.id & ADDON_ENGRAVED) == 0);
		data.id |= ADDON_ENGRAVED;
	}

	static inline void ClearAddOnEngraved(addon_data & data)
	{
		ASSERT((data.id & ADDON_ENGRAVED) != 0);
		data.id &= ~ADDON_ENGRAVED;
	}

	static inline bool IsAddOnEngraved(const addon_data & data)
	{
		return data.id & ADDON_ENGRAVED;
	}
    
	static inline void SetAddOnInherited(addon_data & data)
	{
		ASSERT((data.id & ADDON_INHERITED) == 0);
		data.id |= ADDON_INHERITED;
	}

	static inline void ClearAddOnInherited(addon_data & data)
	{
		ASSERT((data.id & ADDON_INHERITED) != 0);
		data.id &= ~ADDON_INHERITED;
	}

	static inline bool IsAddOnInherited(const addon_data & data)
	{
		return data.id & ADDON_INHERITED;
	}

	static inline int GetArgCount(int id)
	{
		return (id & 0x6000)>>13;
	}

	static inline int GetAddonID(int id)
	{
		return id & 0x1FFF;
	}
};

class addon_inserter
{
public:
//	sample:
//	addon_inserter foo(0,100,(test_addon_handler*)NULL);
	template <typename T>
	addon_inserter(int id,T * ,int rt = 0)
	{
		addon_entry entry;
		entry.id = id;
		entry.handler = new T;
		entry.ratio_calc = rt;
		addon_manager::GetInstance().__Insert(entry);
	}
	
	static int _counter;
	static void foo(const addon_inserter & test)
	{
		_counter ++;
	}
};

#define INSERT_ADDON(id,T) addon_inserter::foo(addon_inserter(id, (T*)NULL))
#define INSERT_ADDON_RATIO(id,T,rt) addon_inserter::foo(addon_inserter(id, (T*)NULL, rt))

bool InitAllAddon();

class essence_addon : public addon_handler
{
public:
	virtual int UpdateItem(const addon_data & , equip_item *)
	{
		//do nothing
		return addon_manager::ADDON_MASK_ESSENCE;
	}
	virtual int TestUpdate()
	{
		return addon_manager::ADDON_MASK_ESSENCE;
	}
	
	virtual int Use(const addon_data & , equip_item *, gactive_imp *)
	{
		ASSERT(false);
		return 0;
	}
	
	virtual int Activate(const addon_data & , equip_item *, gactive_imp *)
	{
		ASSERT(false);
		return 0;
	}
	virtual int Deactivate(const addon_data & , equip_item *, gactive_imp *)
	{
		ASSERT(false);
		return 0;
	}
};
class empty_addon : public essence_addon
{
	public:
	virtual int GenerateParam(int datatype,addon_data & data,int arg_num)
	{
		return addon_manager::GetArgCount(data.id);
	}
};


inline int addon_generate_arg(DATA_TYPE type,addon_data & data,int arg_num)
{
	return addon_manager::GenerateAddonArg(type,data,arg_num);
}

inline int addon_update_ess_data(const addon_data & data, void * essence,size_t ess_size,prerequisition * require)
{
	return addon_manager::UpdateAddonEssData(data,essence,ess_size,require);
}

inline void update_require_data(prerequisition *require)
{
	require->durability *= DURABILITY_UNIT_COUNT;
	require->max_durability *= DURABILITY_UNIT_COUNT;
}

#endif

