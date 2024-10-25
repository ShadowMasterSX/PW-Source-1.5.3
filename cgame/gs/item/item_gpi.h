#ifndef __ONLINE_GAME_GS_ITEM_GPI_H__
#define __ONLINE_GAME_GS_ITEM_GPI_H__

//GPI -- General Purpose Item

#include <vector.h>

template <int cd_index, int cd_time>
class gpi_item: public item_body
{
protected:
	abase::vector<char, abase::fast_alloc<> > _buf;
	unsigned short _crc;
	virtual bool IsItemCanUse(item::LOCATION l) { return true;}
	virtual ITEM_TYPE GetItemType() { return ITEM_TYPE_GPI; }

public:
	gpi_item():_crc(0)
	{}

public:
	virtual bool Save(archive & ar)
	{
		ar << (int)_buf.size();
		if(_buf.size())
		{
			_ar.push_back(_buf.begin(),_buf.size());
		}
		return true;
	}

	virtual bool Load(archive & ar)
	{
		int size;
		ar >> size;
		if(size > 0)
		{
			_buf.clear();
			_buf.insert(_buf.end(),ar.cur_data(),size);
		}
		CalcCRC();
		return true;
	}

	void CalcCRC()
	{
		//计算crc要跳过前面的当前时间
		_crc = crc16(((unsigned char *)&_ess) + CRC_OFFSET,sizeof(ESSENCE) - CRC_OFFSET);
	}

	virtual void GetItemData(const void ** data, size_t &len)
	{
		*data = _buf.begin();
		len = _buf.size();
	}

	virtual int OnUse(item::LOCATION l,gactive_imp * obj,size_t count)
	{
		gplayer_imp * pImp = substance::DynamicCast<gplayer_imp>(obj);
		if(!pImp) return -1;
		if(!obj->CheckCoolDown(cd_index)) 
		{
		//$$$$ 发送冷却信息
			return -1;
		}
		obj->SetCoolDown(cd_index,cd_time);
	//	obj->_runner->use_gpi(_tid,_buf.begin(),_buf.size());
		return 1;
	}
};

