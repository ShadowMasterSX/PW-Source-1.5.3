#ifndef __ONLINEGAME_GS_ITEM_FASHION_ITEM_H__
#define __ONLINEGAME_GS_ITEM_FASHION_ITEM_H__

#include <stddef.h>
#include <octets.h>
#include <common/packetwrapper.h>
#include "../item.h"
#include "../config.h"
#include <arandomgen.h>
#include <crc.h>

#pragma pack(1)
struct fashion_item_essence 
{
	int require_level;
	unsigned short color;
	unsigned short gender;
};
#pragma pack()

template <typename WRAPPER> WRAPPER & operator <<(WRAPPER & wrapper, const fashion_item_essence & ess)
{
	return wrapper.push_back(&ess,sizeof(ess));
}

template <typename WRAPPER> WRAPPER & operator >>(WRAPPER & wrapper, fashion_item_essence & ess)
{
	return wrapper.pop_back(&ess,sizeof(ess));
}

class fashion_item : public item_body
{
	struct {
		fashion_item_essence  ess;
		char name_type;
		char name_size;
		char name[MAX_USERNAME_LENGTH];
	}_ess;
	int _color_mask;
public:
	DECLARE_SUBSTANCE(fashion_item);
	virtual item_body* Clone() const { return  new fashion_item(*this); }
	virtual bool ArmorDecDurability(int) { return false;}
	virtual ITEM_TYPE GetItemType() {return ITEM_TYPE_FASHION; }
	virtual void GetDurability(int &dura,int &max_dura) { dura = 100; max_dura = 100; }
	virtual bool Save(archive & ar)
	{
		ar << _ess.ess << _ess.name_type << _ess.name_size; 
		if(_ess.name_size)
		{
			ar.push_back(_ess.name,_ess.name_size);
		}
		return true;
	}
	virtual bool Load(archive & ar)
	{
		ar >> _ess.ess >> _ess.name_type >> _ess.name_size; 
		if(_ess.name_size > MAX_USERNAME_LENGTH) _ess.name_size = MAX_USERNAME_LENGTH;
		if(_ess.name_size)
		{
			ar.pop_back(_ess.name,_ess.name_size);
		}
        
		_color_mask = (_ess.ess.color << 16);
		return true;
	}

    virtual void InitFromShop(); 
	    
    virtual void DyeItem(int value) 
	{
		_ess.ess.color = value & 0x7FFF;
		_color_mask = (_ess.ess.color << 16);
	}

	virtual unsigned short GetDataCRC() { return _ess.ess.color; }
	virtual void GetItemData(const void ** data, size_t &len)
	{
		*data = &_ess;
		len = sizeof(_ess.ess) + sizeof(2) + _ess.name_size;
	}
	virtual bool VerifyRequirement(item_list & list,gactive_imp* obj);
	virtual int GetIdModify();
};

#endif

