#ifndef __ONLINEGAME_GS_EQUIP_ITEM_H__
#define __ONLINEGAME_GS_EQUIP_ITEM_H__

#include <stddef.h>
#include <octets.h>
#include <common/packetwrapper.h>
#include "../item.h"
#include "../config.h"
#include "item_addon.h"
#include <crc.h>

#include "../worldmanager.h"

class stone_item : public item_body
{
private:
	stone_item * Clone() const { return new stone_item(*this);}
public:
	typedef abase::vector<addon_data,abase::fast_alloc<> > ADDON_LIST;
	abase::octets _raw_data;	//保存原始对象数据
	ADDON_LIST _weapon_addon;	//保存加入到武器上的addon
	ADDON_LIST _armor_addon;	//保存加入到防具上的addon
    ADDON_LIST _decoration_addon; //保存加入到饰品上的addon

public:
	DECLARE_SUBSTANCE(stone_item);
	stone_item()
	{}

public:
	virtual void GetItemData(const void ** data, size_t &len)
	{
		*data = _raw_data.begin();
		len = _raw_data.size();
	}

	virtual bool Save(archive & ar);
	virtual bool Load(archive & ar);
protected:
	virtual bool VerifyRequirement(item_list & list,gactive_imp* obj) {return false;}
	virtual void SetSocketCount(size_t count) {}
	virtual void SetSocketType(size_t index, int type) {} 
	virtual size_t GetSocketCount() { return 0;}
	virtual int GetSocketType(size_t index) { ASSERT(false);return 0;}
	virtual bool IsItemCanUse(item::LOCATION l) { return false; }
	virtual bool ArmorDecDurability(int amount) { return false; }
	virtual unsigned short GetDataCRC() { return 0; }
private:
	virtual int OnUse(item::LOCATION l,gactive_imp * obj,size_t count) {return 0;}
	virtual ITEM_TYPE GetItemType()  { return ITEM_TYPE_DECORATION;}
	virtual bool OnInsertToOther(int self_type, item_body * body)
	{
		ITEM_TYPE type = body->GetItemType();
		if(type == ITEM_TYPE_WEAPON)
		{
			return body->InsertChip(self_type,_weapon_addon.begin(), _weapon_addon.size());
		}
		else if(type == ITEM_TYPE_ARMOR)
		{
			return body->InsertChip(self_type,_armor_addon.begin(), _armor_addon.size());
		}
        else if (type == ITEM_TYPE_DECORATION)
        {
            return body->InsertChip(self_type, _decoration_addon.begin(), _decoration_addon.size());
        }
		return false;
	}

protected:
    void UpdateEssence();

	void LoadAddOn(archive &ar,ADDON_LIST & list)
	{
		ASSERT(list.size() == 0);
		size_t count;
		int argcount;
		ar >> count;
		if(count <0 || count > 128) throw -100; 
        if (count == 0) return;

		list.reserve(count);
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
			list.push_back(entry);
		}
	}

	void SaveAddOn(archive & ar,ADDON_LIST & list)
	{
		size_t count;
		count = list.size();
		ar << count;
		for(size_t i = 0; i < count;i ++)
		{
			int id = list[i].id;
			int argcount = addon_manager::GetArgCount(id);
			ar << id;
			for(int j = 0; j < argcount; j++)
			{
				ar << list[i].arg[j];
			}
		}
	}

};
#endif

