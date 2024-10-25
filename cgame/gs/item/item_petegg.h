#ifndef __ONLINEGAME_GS_PETEGG_ITEM_H__
#define __ONLINEGAME_GS_PETEGG_ITEM_H__

#include <stddef.h>
#include <octets.h>
#include <common/packetwrapper.h>
#include "../item.h"
#include "../config.h"
#include <crc.h>

#pragma pack(1)
struct pe_essence
{
	int require_level;	//需求玩家级别
	int require_class;	//需求玩家职业
	int honor_point;        //好感度
	int pet_tid;            //宠物的模板ID
	int pet_vis_tid;        //宠物的可见ID（如果为0，则表示无特殊可见ID）
	int pet_egg_tid;        //宠物蛋的ID
	int pet_class;          //宠物类型 战宠，骑宠，观赏宠
	short level;            //宠物级别
	unsigned short color;   //宠物颜色，最高位为1表示有效，目前仅对骑宠有效
	int exp;                //宠物当前经验
	int skill_point;        //剩余技能点
	unsigned short name_len;//名字长度 
	unsigned short skill_count;//技能数量
	char name[16];          //名字内容
	struct
	{
		int skill;
		int level;
	}skills[];
};

struct evo_prop
{
	int r_attack;
	int r_defense;
	int r_hp;
	int r_atk_lvl;
	int r_def_lvl;
	int nature;
}; //进化宠的专有属性，随机系数和性格
#pragma pack()

class item_pet_egg: public item_body
{
	pe_essence * _ess;
	size_t _size;
protected:
	virtual bool IsItemCanUse(item::LOCATION l) { return false;}
	virtual bool ArmorDecDurability(int) { return false;}
	virtual int OnGetUseDuration() { return 60;}	
	virtual bool IsItemBroadcastUse() {return true;}
	virtual bool VerifyRequirement(item_list & list,gactive_imp* obj) { return false;}
	virtual int OnUse(item::LOCATION l,int index,gactive_imp * obj,size_t count);

public:
	item_pet_egg():_ess(NULL),_size(0)
	{}

	item_pet_egg(const item_pet_egg & rhs)
	{
		_size = rhs._size;
		if(_size)
		{
			_ess = (pe_essence*)abase::fastalloc(_size);
			memcpy(_ess,rhs._ess,_size);
		}

	}
	

	~item_pet_egg()
	{
		if(_ess)
		{
			abase::fastfree(_ess,_size);
		}
	}
	

public:
	virtual ITEM_TYPE GetItemType()
	{
		return ITEM_TYPE_PET_EGG;
	}

	virtual void GetItemData(const void ** data, size_t &len)
	{
		*data = _ess; 
		len = _size;
	}

	virtual item_body* Clone() const
	{ 
		return  new item_pet_egg(*this); 
	}

	virtual bool Save(archive & ar)
	{
		ar.push_back(_ess,_size);
		return true;
	}

	virtual bool Load(archive & ar);

public:
	DECLARE_SUBSTANCE(item_pet_egg);

};
#endif

