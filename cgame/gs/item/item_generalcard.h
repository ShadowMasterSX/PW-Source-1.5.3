#ifndef __ONLINEGAME_GS_ITEM_GENERALCARD_H__
#define __ONLINEGAME_GS_ITEM_GENERALCARD_H__

#include "../config.h"
#include "../item.h" 

enum { GENERALCARD_RANK_S = 3, };

struct generalcard_essence
{
	int type;					//类型,与装备位置相对应,破军,破阵,长生,完璧,玄魂,玄命
	int rank;					//品级,C,B,A,S,S+
	int require_level;			//需求等级
	int require_leadership;		//需求统率力
	int max_level;				//最大成长等级
	int level;					//成长等级
	int exp;					//成长经验
	int rebirth_times;			//转生次数
};

template <typename WRAPPER>
WRAPPER & operator<<(WRAPPER & wrapper, const generalcard_essence & es)
{
	wrapper.push_back(&es, sizeof(es));
	return wrapper;
}

template <typename WRAPPER>
WRAPPER & operator>>(WRAPPER & wrapper, generalcard_essence & es)
{
	wrapper.pop_back(&es, sizeof(es));
	return wrapper;
}

struct generalcard_extend
{
	int max_hp;
	int damage_low;
	int damage_high;
	int damage_magic_low;
	int damage_magic_high;
	int defense;  
	int resistance[MAGIC_CLASS];
	int vigour;
};

class generalcard_item : public item_body
{
	generalcard_essence _ess;				//需要存盘的本体数据
	generalcard_extend _extend;
	const ADDON_LIST * _extra_addon;
	
public:
	DECLARE_SUBSTANCE(generalcard_item);
	generalcard_item():_extra_addon(NULL),_crc(0)
	{
		memset(&_ess, 0, sizeof(_ess));
		memset(&_extend, 0, sizeof(_extend));
	}

	virtual bool Load(archive & ar);
	virtual void GetItemData(const void ** data, size_t &len)
	{
		*data = &_ess;
		len = sizeof(_ess);
 	}
	
	unsigned short _crc;
	virtual unsigned short GetDataCRC() { return _crc; }
	void CalcCRC()
	{   
		_crc = crc16( (unsigned char *)&_ess, sizeof(_ess));
	}

	void ClearData();
	void UpdateEssence();
	virtual void OnRefreshItem();

private:
	//item_body中纯虚函数
	ITEM_TYPE GetItemType()  { return ITEM_TYPE_GENERALCARD;}
	bool ArmorDecDurability(int) { return false;}
	item_body * Clone() const { return new generalcard_item(*this);}

private:
	//装备卡牌相关
	virtual bool VerifyRequirement(item_list & list, gactive_imp* imp);
	virtual void OnActivate(item::LOCATION, size_t pos, size_t count, gactive_imp* imp);
	virtual void OnDeactivate(item::LOCATION, size_t pos, size_t count,gactive_imp* imp);
	virtual void OnPutIn(item::LOCATION l,item_list & list,size_t pos,size_t count,gactive_imp* obj);
	virtual void OnTakeOut(item::LOCATION l,size_t pos,size_t count, gactive_imp* obj);

	virtual int GetRank(){ return _ess.rank; }
	virtual int GetRebirthTimes(){ return _ess.rebirth_times; }
	virtual bool CheckRebirthCondition(int material_rebirth_times);
	virtual bool DoRebirth(int arg);
	virtual bool InsertExp(int& exp, bool ischeck);
	virtual int GetSwallowExp();
	virtual bool IsGeneralCardMatchPos(size_t pos)
	{
		return GetGeneralCardColumnIndex(pos) == _ess.type;
	}

	inline int GetGeneralCardColumnIndex(size_t pos)
	{
		return (int)pos - item::EQUIP_INDEX_GENERALCARD1;
	}
};

#endif
