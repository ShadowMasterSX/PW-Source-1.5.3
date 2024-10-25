#ifndef __ONLINEGAME_GS_DUMMY_ITEM_H__
#define __ONLINEGAME_GS_DUMMY_ITEM_H__

#include <stddef.h>
#include <common/packetwrapper.h>
#include "../item.h"
#include "../config.h"
#include <vector.h>

class item_dummy: public item_body
{
	abase::vector<char, abase::fast_alloc<> > _ess;
protected:
	virtual bool ArmorDecDurability(int) { return false;}

public:
	item_dummy()
	{}
	

public:
	virtual ITEM_TYPE GetItemType()
	{
		return ITEM_TYPE_DUMMY;
	}

	virtual void GetItemData(const void ** data, size_t &len)
	{
		*data = _ess.begin(); 
		len = _ess.size();
	}

	virtual item_body* Clone() const
	{ 
		return  new item_dummy(*this); 
	}

	virtual bool Save(archive & ar)
	{
		ar.push_back(_ess.begin(),_ess.size());
		return true;
	}

	virtual bool Load(archive & ar)
	{
		size_t size = ar.size() - ar.offset();
		_ess.clear();
		if(size > 0)
		{
			_ess.reserve(size);
			_ess.insert(_ess.begin(), size, 0);
			ar.pop_back(_ess.begin(), size);
		}
		return true;
	}
public:
	DECLARE_SUBSTANCE(item_dummy);

};
#endif


