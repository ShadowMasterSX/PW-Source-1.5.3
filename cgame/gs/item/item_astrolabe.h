#ifndef __ONLINEGAME_GS_ITEM_ASTROLABE_H__
#define __ONLINEGAME_GS_ITEM_ASTROLABE_H__

#include <stddef.h>
#include <octets.h>
#include <common/packetwrapper.h>
#include "../item.h"
#include "../config.h"
#include "item_addon.h"
#include <crc.h>

#pragma pack(1)
struct astrolabe_essence
{
	int   exp;
	unsigned char  level;
	unsigned short slot;			// 属性位上是否有属性 顺时针
	unsigned short aptit[ASTROLABE_SLOT_COUNT]; // 1/1000

	int GetAptit(int index)
	{
		if(index%2 == 0) // 外圈
		{
			int i1 = (index/2) %ASTROLABE_SLOT_COUNT;
			int i2 = i1 ? i1 - 1 : (ASTROLABE_SLOT_COUNT - 1);
			return aptit[i1] + aptit[i2];
		}	
		else 
		{
			int i = (index/2) %ASTROLABE_SLOT_COUNT;
			return aptit[i];
		}
	}
	int SumAptit()
	{
		int sum = 0;
		for(int i = 0; i < ASTROLABE_SLOT_COUNT; ++i)
			sum += aptit[i];
		return sum;
	}
	void InitAptit(int alloc_aptit,int alloc_max,int alloc_min)
	{
		int astrolabe_aptit_init_total = alloc_max * ASTROLABE_SLOT_COUNT;
		if(alloc_aptit > astrolabe_aptit_init_total) alloc_aptit = astrolabe_aptit_init_total;
		
		for(int i = 0; i < ASTROLABE_SLOT_COUNT; ++i)
			aptit[i] = alloc_min;
		
		int astrolabe_aptit_init_keep = alloc_min * ASTROLABE_SLOT_COUNT;
		if(alloc_aptit < astrolabe_aptit_init_keep)	return;
		alloc_aptit -= astrolabe_aptit_init_keep;
		
		int index = abase::Rand(0,ASTROLABE_SLOT_COUNT-1);
		while(alloc_aptit > 0)
		{
			int limit = alloc_max - aptit[index];
			if(limit > 0)
			{
				limit = std::min(limit,alloc_aptit);
				int alloc = abase::Rand(1,limit);
				aptit[index] += alloc;
				alloc_aptit  -= alloc;
			}
			index = (index+1)%ASTROLABE_SLOT_COUNT;
		}
	}
	bool AddAptit(int add,int max_limit)
	{
		int index = abase::Rand(0,ASTROLABE_SLOT_COUNT-1);
		int stry = ASTROLABE_SLOT_COUNT;
		while(stry)
		{
			int alloc = max_limit - aptit[index];
			if(alloc > 0)
			{
				alloc = std::min(alloc,add);
				aptit[index] += alloc; 
				return true;
			}	
			index = (index+1)%ASTROLABE_SLOT_COUNT;
			--stry;
		}

		return false;
	}
};
 #pragma pack()

template <typename WRAPPER>
WRAPPER & operator<<(WRAPPER & wrapper, const astrolabe_essence & es)
{
	wrapper.push_back(&es, sizeof(es));
	return wrapper;
}

template <typename WRAPPER>
WRAPPER & operator>>(WRAPPER & wrapper, astrolabe_essence & es)
{
	wrapper.pop_back(&es, sizeof(es));
	return wrapper;
}

struct astrolabe_extend
{
	int swallow_exp;
	int level_limit;
	int race_limit;
};

struct astrolabe_aptit_limit
{
	int max;
	int min;
	int total;
	int init_total;
	int init_max;

	void Init();
};

class astrolabe_item : public item_body
{
protected:
	astrolabe_essence _ess;
	astrolabe_extend _extend;
	astrolabe_aptit_limit _aptit_limit;
	ADDON_LIST _total_addon;
public:
	DECLARE_SUBSTANCE(astrolabe_item);
	astrolabe_item() {  _crc = 0; }
	virtual void GetItemData(const void ** data, size_t &len)
	{
		*data = _raw_data.begin();
		len = _raw_data.size();
	}
	virtual bool Save(archive & ar);
	virtual bool Load(archive & ar);
private:
	//item_body中纯虚函数
	virtual ITEM_TYPE GetItemType()  { return ITEM_TYPE_ASTROLABE;}
	bool ArmorDecDurability(int) { return false;}
	item_body * Clone() const { return new astrolabe_item(*this);}
protected:
	abase::octets _raw_data;	//保存原始对象数据,当对象更新后需要重新生成
	unsigned short _crc;
	void CalcCRC()
	{
		_crc = crc16( (unsigned char *)_raw_data.begin(),_raw_data.size());
	}
	virtual unsigned short GetDataCRC() { return _crc; }
	
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
	void  UpdateEssence();
	float GetAddonRatio(int index);
	void  ShuffleSlot();
	void  ShuffleAddon();
public:
	virtual int  GetSwallowExp();
	virtual bool InsertExp(int& exp, bool ischeck);
	virtual int  GetIdModify();
	virtual bool DoRebirth(int arg);
	virtual bool FlushGeniusPoint();
	virtual bool AddGeniusPoint(short g0, short g1, short g2, short g3, short g4, bool ischeck);
protected:
	virtual bool VerifyRequirement(item_list & list, gactive_imp* imp);
	virtual void OnTakeOut(item::LOCATION l,size_t pos,size_t count, gactive_imp* obj);
	virtual void OnPutIn(item::LOCATION l,item_list & list,size_t pos,size_t count,gactive_imp* obj);
	virtual void OnRefreshItem();
	virtual bool OnInherit(item_body* other);
	virtual void OnUnpackage(gactive_imp*);
	virtual void OnDump(std::string& str);
	virtual void OnRebuild(void* data,size_t len);
private:	
	virtual void OnActivate(item::LOCATION l,size_t pos,size_t count, gactive_imp* obj);
	virtual void OnDeactivate(item::LOCATION l,size_t pos,size_t count,gactive_imp* obj);
	virtual int  OnGetLevel();
};

#endif
