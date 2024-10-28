#ifndef __ONLINE_GAME_GS_ITEM_FIREWORKS_H__
#define __ONLINE_GAME_GS_ITEM_FIREWORKS_H__

#include "../item.h"

class item_fireworks: public item_body
{
protected:
	virtual bool IsItemCanUse(item::LOCATION l) { return true;}
	virtual ITEM_TYPE GetItemType() { return ITEM_TYPE_FIREWORKS; }

	virtual item_body * Clone() const { return new item_fireworks(*this);}
	virtual bool ArmorDecDurability(int) { return false;}
	virtual bool IsItemBroadcastUse() {return true;}

public:
	item_fireworks()
	{}

public:
	virtual bool Save(archive & ar)
	{
		return true;
	}

	virtual bool Load(archive & ar)
	{
		return true;
	}

	virtual void GetItemData(const void ** data, size_t &len)
	{
		*data = "";
		len = 0;
	}

	virtual int OnUse(item::LOCATION l,gactive_imp * obj,size_t count);

public:
	DECLARE_SUBSTANCE(item_fireworks);
};

#pragma pack(1)
	struct _firework_args
	{
		int  target_role_id;
		char target_username[MAX_USERNAME_LENGTH];
		char is_friend_list;
	};
#pragma pack()

class item_fireworks2: public item_body
{
protected:
	virtual bool IsItemCanUseWithArg(item::LOCATION l,size_t buf_size) { return buf_size == sizeof(_firework_args);}
	virtual ITEM_TYPE GetItemType() { return ITEM_TYPE_FIREWORKS; }

	virtual item_body * Clone() const { return new item_fireworks2(*this);}
	virtual bool ArmorDecDurability(int) { return false;}
	virtual bool IsItemBroadcastUse() {return true;}
	virtual bool IsItemBroadcastArgUse() {return true;}

public:
	item_fireworks2()
	{}

public:
	virtual bool Save(archive & ar)
	{
		return true;
	}

	virtual bool Load(archive & ar)
	{
		return true;
	}

	virtual void GetItemData(const void ** data, size_t &len)
	{
		*data = "";
		len = 0;
	}

	virtual int OnUse(item::LOCATION ,int index, gactive_imp*,const char * arg, size_t arg_size);

public:
	DECLARE_SUBSTANCE(item_fireworks2);
};

#endif 

