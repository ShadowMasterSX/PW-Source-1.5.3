#ifndef __ONLINEGAME_GS_ITEMLIST_H__
#define __ONLINEGAME_GS_ITEMLIST_H__

#include "item.h"
#include <amemobj.h>
#include <amemory.h>
#include <octets.h>
#include <crc.h>
#include "playertemplate.h"
#include <db_if.h>

class item_list : public abase::ASmallObject
{
	item::LOCATION 		_location;
	abase::vector<item> 	_list;
	gactive_imp		*_owner;
	size_t			_empty_slot_count;
	inline int __try_pile(int type,size_t & count,size_t pile_limit,item*&pEmpty);
	inline void __find_empty(item*&pEmpty);
public:
	inline item::LOCATION GetLocation() {return _location;}

	item_list(item::LOCATION location, size_t size)
		:_location(location),_list(size,item()),_owner(NULL),_empty_slot_count(size)
	{} 

	~item_list()
	{
		Clear();
	}

	void Swap(item_list & rhs)
	{
		abase::swap(_location, rhs._location);
		_list.swap(rhs._list);
		abase::swap(_empty_slot_count,rhs._empty_slot_count);
	}

	//做出来的数据必须要释放
	bool MakeDBData(GDB::itemlist &list)
	{
#ifdef _DEBUG
		size_t count = 0;
		for(size_t i = 0; i < _list.size(); i ++)
		{
			if(_list[i].type == -1) count ++;
		}
		ASSERT(count == _empty_slot_count);
#endif
		ASSERT(!list.count && !list.list);
		ASSERT(list.list == NULL || list.count == 0);
		size_t item_count = _list.size() - _empty_slot_count;
		if(item_count == 0) return true;
		list.list = (GDB::itemdata*)abase::fast_allocator::alloc(item_count*sizeof(GDB::itemdata));
		list.count = item_count;
		size_t index = 0;
		GDB::itemdata * pData = list.list;
		for(size_t i = 0; i < _list.size(); i ++)
		{
			if(_list[i].type == -1) continue;
			const item & it = _list[i];
			pData[index].id = it.type;
			pData[index].index = i;
			pData[index].count =it.count;
			pData[index].max_count = it.pile_limit;
			pData[index].guid1 = it.guid.guid1;
			pData[index].guid2 = it.guid.guid2;
			pData[index].mask = it.equip_mask;
			pData[index].proctype = it.proc_type;
			pData[index].expire_date = it.expire_date;
			pData[index].data = NULL;
			pData[index].size = 0;
			if(it.body)
			{
				it.body->GetItemData(&(pData[index].data), pData[index].size);
			}
			index ++;
		}
		return true;
	}

	int GetDBData(GDB::itemdata *pData, size_t size)
	{
#ifdef _DEBUG
		size_t count = 0;
		for(size_t i = 0; i < _list.size(); i ++)
		{
			if(_list[i].type == -1) count ++;
		}
		ASSERT(count == _empty_slot_count);
#endif
		if(size < _list.size())
		{
			ASSERT(false);
			return -1;
		}
		size_t item_count = _list.size() - _empty_slot_count;
		if(item_count == 0) return 0;
		size_t index = 0;
		for(size_t i = 0; i < _list.size(); i ++)
		{
			if(_list[i].type == -1) continue;
			const item & it = _list[i];
			pData[index].id = it.type;
			pData[index].index = i;
			pData[index].count =it.count;
			pData[index].max_count = it.pile_limit;
			pData[index].guid1 = it.guid.guid1;
			pData[index].guid2 = it.guid.guid2;
			pData[index].mask = it.equip_mask;
			pData[index].proctype = it.proc_type;
			pData[index].expire_date = it.expire_date;
			pData[index].data = NULL;
			pData[index].size = 0;
			if(it.body)
			{
				it.body->GetItemData(&(pData[index].data), pData[index].size);
			}
			index ++;
		}
		return index;
	}


	static void ReleaseDBData(GDB::itemlist & list)
	{
		//释放Make出来的数据
		if(!list.list) return;
		size_t size = list.count * sizeof(GDB::itemdata);
		abase::fast_allocator::free(list.list,size);
		list.count = 0;
		list.list = 0;
	}

	bool InitFromDBData(const GDB::itemlist & list)
	{
		ASSERT(_empty_slot_count == _list.size());
		size_t count = list.count;
		/*if(count > _list.size()) 
		{
			ASSERT(false);
			return false;
		}*/
		const GDB::itemdata * pData = list.list;
		for(size_t i = 0; i < count ; i ++)
		{
			size_t index= pData[i].index;
			if(index >= _list.size()) continue;	//忽略多余的物品
			if(!MakeItemEntry(_list[index],pData[i]))
			{
				//出现错误的物品 直接清除
				GLog::log(LOG_INFO,"用户%d的物品%d没有符合的过期时间，自动消失\n", _owner->_parent->ID.id, pData[i].id);
				_list[index].Clear();
				continue;
			}
			_list[index].PutIn(_location,*this, index, _owner);
			_empty_slot_count --;
		}
		return true;
	}

	void SetSize(size_t size)
	{
		if(size <= _list.size()) return;
		abase::vector<item> 	tmplist(size,item());
		size_t offset = size - _list.size();
		for(size_t i = 0; i < _list.size(); i ++)
		{
			item tmp = _list[i];
			_list[i] = tmplist[i];
			tmplist[i] = tmp;
			tmp.Clear();
		}
		_list.swap(tmplist);

		_empty_slot_count += offset;
	}

	
	int GetItemData(size_t index, item_data & data,unsigned short & crc)
	{
		if(index >= _list.size()) return -1;
		if(_list[index].type == -1) return 0;
		
		ItemToData(_list[index],data,crc);
		return index + 1;
	}

	int GetItemData(size_t index, item_data & data)
	{
		if(index >= _list.size()) return -1;
		if(_list[index].type == -1) return 0;
		
		ItemToData(_list[index],data);
		return index + 1;
	}
	
	void SimpleSave(abase::octets & data)
	{
		data.reserve(_list.size() * (sizeof(int)*2));
		for(size_t i = 0; i < _list.size(); i ++)
		{
			int type = _list[i].type;
			data.push_back(&type,sizeof(type));
			if(type < 0) continue;
			int expire_date = _list[i].expire_date;
			data.push_back(&expire_date, sizeof(expire_date));
			int count = _list[i].count;
			if(_list[i].IsActive())
			{
				count |= 0x80000000;
			}
			data.push_back(&count,sizeof(count));
		}
	}
	
	bool DetailSave(archive & ar)
	{
		size_t count = _list.size() - _empty_slot_count;
		ar << count;
		size_t i;
		for(i = 0; i < _list.size(); i ++)
		{
			item & it = _list[i];
			if(it.type == -1)
			{
				continue;
			}
			count --;
			ar << i;
			ar << it.type;
			ar << it.expire_date;
			//ar << (int)(it.GetProctypeState());
			ar << it.proc_type;
			ar << it.count;
			const void * data;
			size_t len;
			if(it.body){
				ar << it.body->GetDataCRC();
				it.body->GetItemData(&data,len);
				ASSERT(len < 65535);
				ar << (unsigned short)len;
				ar.push_back(data,len);
			}
			else
			{
				ar << (int)0;
			}
		}
		ASSERT(count == 0);
		return true;
	}

	bool DetailSavePartial(archive & ar,const int spec_list[] , size_t ocount)
	{
		ASSERT(ocount >0 && ocount <= _list.size());
		ar << ocount;
		int i;
		size_t oindex = 0;
		for(i = 0; i < (int)_list.size(); i ++)
		{
			if(i != spec_list[oindex])
			{
				continue;
			}
			item & it = _list[i];
			if(it.type == -1)
			{
				ar << -1;
			}
			else
			{
				ar << i;
				ar << it.type;
				ar << it.expire_date;
				//ar << (int)(it.GetProctypeState());
				ar << it.proc_type;
				ar << it.count;
				const void * data;
				size_t len;
				if(it.body){
					ar << it.body->GetDataCRC();
					it.body->GetItemData(&data,len);
					ASSERT(len < 65535);
					ar << (unsigned short)len;
					ar.push_back(data,len);
				}
				else
				{
					ar << (int)0;
				}
			}
			oindex ++;
			if(oindex == ocount) break;
			ASSERT(spec_list[oindex] > spec_list[oindex-1]);
		}
		ASSERT(oindex == ocount);
		return true;
	}

	bool Save(archive & ar, bool &nosave)
	{
		ar << _list.size();
		item empty;
		size_t i;
		nosave = false;
		for(i = 0; i < _list.size(); i ++)
		{
			item & ii = _list[i];
		/*	if(ii.type != -1 && (ii.proc_type & item::ITEM_PROC_TYPE_NO_SAVE))
			{
				nosave = true;
				empty.Save(ar);
			}
			else*/
				ii.Save(ar);
		}
		return true;
	}

	bool Load(archive & ar)
	{
		size_t count;
		ar >> count;
		ASSERT(count >= _list.size());
		if(count > _list.size())
		{
			SetSize(count);
		}
		ASSERT(_empty_slot_count == count);
		Clear();
		size_t i;
		for(i = 0; i < count; i ++)
		{
			_list[i].Load(ar);
			if(_list[i].type != -1) _empty_slot_count --; 
		}
		return true;
	}
	
	static inline void ItemToData(item & it, item_data & data,unsigned short &crc)
	{
		ASSERT(it.type != -1);
		data.type = it.type;
		data.count = it.count;
		data.pile_limit = it.pile_limit;
		data.equip_mask = it.equip_mask;
		data.price = it.price;
		data.proc_type = it.proc_type;
		data.expire_date = it.expire_date;
		data.guid.guid1 = it.guid.guid1;
		data.guid.guid2 = it.guid.guid2;
		if(it.body)
		{
			crc = it.body->GetDataCRC();
			data.classid= it.body->GetGUID();
			it.body->GetItemData((const void **)&(data.item_content), data.content_length);
		}else
		{
			crc = 0;
			data.classid  = -1;
			data.content_length = 0;
			data.item_content= 0;
		}
	}

	static inline void ItemToData(item & it, item_data & data)
	{
		ASSERT(it.type != -1);
		data.type = it.type;
		data.count = it.count;
		data.pile_limit = it.pile_limit;
		data.equip_mask = it.equip_mask;
		data.price = it.price;
		data.proc_type = it.proc_type;
		data.expire_date = it.expire_date;
		data.guid.guid1 = it.guid.guid1;
		data.guid.guid2 = it.guid.guid2;
		if(it.body)
		{
			data.classid= it.body->GetGUID();
			it.body->GetItemData((const void **)&(data.item_content), data.content_length);
		}else
		{
			data.classid  = -1;
			data.content_length = 0;
			data.item_content= 0;
		}
	}

	static inline void ItemToData(item & it, GDB::itemdata & data)
	{
		ASSERT(it.type != -1);
		data.id = it.type;
		data.index = -1; 	//这里无法获得，填入一个错误值 
		data.count = it.count;
		data.max_count = it.pile_limit;
		data.guid1 = it.guid.guid1;
		data.guid2 = it.guid.guid2;
		data.mask = it.equip_mask;
		data.proctype = it.proc_type;
		data.expire_date = it.expire_date;
		data.data = NULL;
		data.size = 0;

		if(it.body)
		{
			it.body->GetItemData(&(data.data), data.size);
		}
	}

/**
 *	取得装备的表现数据，放在一个abase::octets里，只保存有的装备
 *
 */
 
	void GetEquipmentData(uint64_t & emask, abase::octets & os)
	{
		ASSERT(_location == item::BODY);
		uint64_t mask = 0;
		for(size_t i = item::EQUIP_VISUAL_START; i < item::EQUIP_VISUAL_END; i ++)
		{
			int type = _list[i].type;
			if(type == -1) continue;
			type |= _list[i].GetIdModify();
			mask |= 1ULL << (i - item::EQUIP_VISUAL_START);
			os.push_back(&type,sizeof(type));
		}
		emask = mask;
		return; 
	}

/**
 *	取得装备的表现数据，放在一个abase::octets里
 *	和原来有的数据做一个差，生成一个变化量，供周围人了解数据内容
 */
	/*void GetEquipmentData(int oldmask,ushort & newmask,ushort & addmask, ushort & delmask,abase::octets & os,abase::octets & osn)
	{
		ASSERT(_location == item::BODY);
		unsigned short mask = 0;
		unsigned short mask_add = 0;
		unsigned short mask_del = 0;
		ASSERT(os.empty());
		ASSERT(osn.empty());
		for(int i = item::EQUIP_VISUAL_START; i < item::EQUIP_VISUAL_END; i ++)
		{
			unsigned short m = 1 << (i - item::EQUIP_VISUAL_START);
			int type = _list[i].type;
			if(type == -1) 
			{
				mask_del |= m;
				continue;
			}
			if((oldmask & m) == 0) 
			{
				mask_add |= m;
				osn.push_back(&type,sizeof(type));
			}
			mask |= m;
			os.push_back(&type,sizeof(type));
		}

		newmask = mask;
		addmask = mask_add;
		delmask = mask_del;
		return;
	}*/

	void Clear()
	{
		for(size_t i = 0;i < _list.size(); i ++)
		{
			if(_list[i].type != -1)
			{
				_list[i].Release();
			}
		}
		_empty_slot_count = _list.size();
	}
	
	size_t Size() { return _list.size();}
	bool IsSlotEmpty(int index) { return _list[index].type == -1;}
	void SetOwner(gactive_imp * owner) {_owner = owner;}
	size_t GetEmptySlotCount() { return _empty_slot_count;}
	size_t GetItemCount() { return _list.size() - _empty_slot_count;}

	/*
	 *	小心使用这个操作符，不要用这种方式删除物品
	 *	会影响物品的计数策略
	 */
	const item & operator[](size_t index) const  { return _list[index];}

	/*
	 *	小心使用这个操作符，不要用这种方式删除物品
	 *	会影响物品的计数策略
	 */
	item & operator[](size_t index) { return _list[index];}

	bool IsItemActive(size_t index) { return _list[index].IsActive();}
//	item & operator[](size_t index) { return _list[index];}

	/*
	 *	寻找第一个符合标准的物品
	 *	从三start指出的位置开始寻找
	 *	如果要寻找空位，给定的type使用-1即可
	 */
	int  Find(int start,int type)
	{
		for(size_t i = start; i < _list.size(); i ++)
		{
			if(_list[i].type == type) return i;
		}
		return -1;
	}
	/**
	 *	寻找能放东西的第一个空位，考虑了堆叠因素
	 */
	int FindEmpty(int type)
	{
		for(size_t i = 0; i < _list.size(); i ++)
		{
			if(_list[i].type == -1 ) return i;
			if(_list[i].type == type && _list[i].count < _list[i].pile_limit) return i;
		}
		return -1;
	}

	/*
	 *	对特定的物品是否有空闲的位置	
	 */
	bool HasSlot(int type)
	{
		if(_empty_slot_count) return true;
		for(size_t i = 0; i < _list.size(); i ++)
		{
			if(_list[i].type == type && _list[i].count < _list[i].pile_limit) return true;
		}
		return false;
	}

	/*
	 *	对特定的物品是否有空闲的位置	 按照指定的位置来计算
	 */
	bool HasSlot(int type,int count)
	{
		if(_empty_slot_count) return true;
		for(size_t i = 0; i < _list.size() && count > 0; i ++)
		{
			if(_list[i].type == type)
			{
				count -= _list[i].pile_limit - _list[i].count;
			}
		}
		return count <= 0;
	}

	/**
	*	是否当前已经没有空的位子
	*/
	
	bool IsFull()
	{
		return !_empty_slot_count;
	}

	/**
	*	察看特定位置是否有指定的物品
	*/
	bool IsItemExist(size_t index,int type, size_t count)
	{
		if(index >= Size()) return false;
		const item & it = _list[index];
		if(it.type == -1 || it.type != type) return false;
		if(it.count < count) return false;
		return true;
	}

	bool IsItemExist(int type, size_t count)
	{
		int rst = 0;
		while((rst = Find(rst,type)) >=0)
		{
			//是不是应该累计所有位置的数目？
			if(count <= _list[rst].count) return true;
			rst ++;
		}
		return false;
	}


	 
	/**
	 *	放入一个物品，会自动寻找空闲的空位或者可以叠加的位置
	 *	如果表满，那么会返回错误(返回-1) it也不会被置空
	 */
	int Push(item & it);

	/**
	 *	从数据里放入一个物品,这个物品会从传入的数据中生成出来
	 *	如果失败，表示物品表已经满了
	 *	正常情况下，返回放入的位置索引
	 *	在此函数里，会自动调用物品对象（如果存在）的PutIn函数
	 */
	int Push(item_data & data);

	/**
	 *	从数据里放入一个物品,这个物品会从传入的数据中生成出来
	 */
	int Push(const item_data & data,int &count, int expire_date);

	/*
	 *	在空位里添加物品
	 */
	int PushInEmpty(int start, const item_data & data , int count);

	/*
	 *	在空位里添加物品
	 */
	int PushInEmpty(int start, item & it);

	/**
	*	在指定位置放入一件物品,
	*	如果该位置已经存在一样的物品且是可以叠加的，那么会进行叠加
	*	被叠加的物品不会被切分成若干部分。
	*	如果不能叠加，则会返回失败(小于0的值）
	*	如果成功的话，传入的对象会被Clear()
	* 	另外这里不判断所引的正确与否
	*/
	bool Put(int index, item & it)
	{
		item & old = _list[index]; 
		if(old.type == -1)
		{
			if(it.type == -1) return true;
			//原来没有物品，直接放入
			old = it;
			it.Clear();
			old.PutIn(_location,*this,index,_owner);
			_empty_slot_count  --;
			return true;
		}
		else
		{
			if(old.type == it.type && old.pile_limit >= old.count + it.count)
			{ 
				//原来有物品，而且可以放入
				//叠加上去，并且释放传入的物品
				ASSERT(it.pile_limit == old.pile_limit);
				old.count += it.count;
				it.Release();
				return true;
			}
		}
		return false;
	}

	/*
	 *	在指定一个位置放入一件物品，并且将该位置原来的物品换出
	 */
	void Exchange(int index, item & it)
	{
		if(it.type != -1) _empty_slot_count --;
		item & src = _list[index];
		if(src.type != -1) src.TakeOut(_location,index,_owner);
		item tmp = it; it = src; src = tmp;
		tmp.Clear();
		if(src.type != -1) src.PutIn(_location,*this,index,_owner);
		if(it.type != -1)
		{
			_empty_slot_count ++;
		}
		return ;
	}

	/*
	*	交换物品表里的两个位置,如果是装备栏的话需要在外边RefreshEquipment
	*/
	bool ExchangeItem(size_t ind1, size_t ind2 )
	{
		if(ind1 >= _list.size() || ind2 >= _list.size()) return false;
		if(ind1 == ind2) return true;

		item & src = _list[ind1];
		if(src.type != -1) src.TakeOut(_location,ind1,_owner);
		item & dest = _list[ind2];
		if(dest.type != -1) dest.TakeOut(_location,ind2,_owner);
		
		item tmp = src;
		src = dest;
		dest = tmp;
		tmp.Clear();

		if(src.type != -1) src.PutIn(_location,*this,ind1,_owner);
		if(dest.type != -1) dest.PutIn(_location,*this,ind2,_owner);

		return true;
	}

	/**
	*
	*/
	bool MoveItem(size_t src, size_t dest, size_t *pCount)
	{
		if(src >= _list.size() || dest >= _list.size()) return false;
		size_t count = *pCount;
		if(src == dest) return true;
		if(!count) return false;
		item & __src = _list[src];
		item & __dest= _list[dest];
		if(__src.type == -1 || __src.count < count) 
		{
			*pCount = 0;
			return true;
		}
		if(__dest.type == -1)
		{
			__dest = __src;
			__src.count -= count;
			if(__src.count && __src.body)
			{
				__dest.body = __src.body->Clone();
			}
			__dest.count = count;

			if(!__src.count)
			{
				__src.Clear();
			}
			else
			{
				__dest.PutIn(_location,*this,dest, _owner);
				_empty_slot_count --;
			}
			return true;
		}
		if(__dest.type != __src.type || __dest.count + count > __dest.pile_limit) 
		{
			*pCount = 0;
			return true;
		}

		__dest.count += count;
		__src.count -= count;
		if(!__src.count)
		{
			_empty_slot_count ++;
			__src.TakeOut(_location, src, _owner);
			__src.Release();
		}
		return true;
	}
	

	/*
	 *	删除并取出一个物品
	 */
	bool Remove(size_t index,item & it)
	{
		if(_list[index].type == -1) 
		{
			return true;
		}
		it = _list[index];
		_empty_slot_count ++;
		_list[index].TakeOut(_location,index,_owner);
		_list[index].Clear();
		return true;
	}

	void Remove(size_t index)
	{
		if(index >= _list.size()) 
		{
			ASSERT(false);
			return;
		}
		if(_list[index].type == -1)
		{
			return ;
		}
		_list[index].TakeOut(_location,index,_owner);
		_list[index].Release();
		_empty_slot_count ++;
		return ;
	}

	int DecAmount(size_t index,size_t count)	//返回还剩余多少，0表示对象没有了
	{
		if(_list[index].type == -1) 
		{
			ASSERT(false);
			return 0;
		}

		item & it = _list[index];
		if(it.count <= count)
		{
			it.TakeOut(_location,index,_owner);
			it.Release();
			_empty_slot_count ++;
		}
		else
		{
			it.count -= count;
		}
		return it.count;
	}

	int IncAmount(size_t index, size_t count) //返回有多少数量被加入
	{
		if(_list[index].type == -1) 
		{
			ASSERT(false);
			return -1;
		}

		item & it = _list[index];
		if(it.count >= it.pile_limit) return 0;
		size_t delta = it.pile_limit - it.count;
		if(delta > count) delta = count;
		it.count += delta;
		return (int)delta;
	}

	bool DecDurability(size_t index , int amount)
	{
		if(_list[index].type == -1) 
		{
			ASSERT(false);
			return false;
		}

		return _list[index].ArmorDecDurability(amount);
	}

	int UseItem(int index,gactive_imp * obj, size_t & count)
	{	
		if(_list[index].type == -1) 
		{
			return -1;
		}

		item & it = _list[index];
		int type = it.type;
		if(!it.CanUse(GetLocation()))
		{
			return -2;
		}
		if(count > it.count) count = it.count;
		int rst = it.Use(GetLocation(),index,obj,count);
		if(rst < 0) return -3;
		ASSERT(it.type == type && it.count > 0);
		count = rst;
		if(rst > 0)
		{
			if(it.count <= (size_t)rst)
			{
				it.TakeOut(_location,index,_owner);
				it.Release();
				_empty_slot_count ++;
			}
			else
			{
				it.count -= rst;
			}
		}
		return type;
	}

	int UseItemWithArg(int index,gactive_imp * obj,int &count, const char * arg, size_t arg_size)
	{       
		if(_list[index].type == -1)  
		{       
			return -1;
		}       

		item & it = _list[index];
		int type = it.type;
		if(!it.CanUseWithArg(GetLocation(), arg_size))
		{       
			return -2;
		}       
		int rst = it.Use(GetLocation(), index, obj, arg, arg_size);
		if(rst < 0) return -3;
		ASSERT(it.type == type && it.count > 0);
		count = rst;
		if(rst > 0)
		{       
			if(it.count <= (size_t)rst)
			{       
				it.TakeOut(_location,index,_owner);
				it.Release();
				_empty_slot_count ++;
			}       
			else    
			{       
				it.count -= rst; 
			}       
		}       
		return type;
	}       

	int UseItemWithTarget(int index,gactive_imp * obj, const XID & target,char force_attack, int & count)
	{	
		if(_list[index].type == -1) 
		{
			return -1;
		}

		item & it = _list[index];
		int type = it.type;
		if(!it.CanUseWithTarget(GetLocation()))
		{
			return -2;
		}
		int rst = it.UseWithTarget(GetLocation(),index,obj,target,force_attack);
		if(rst < 0) return -3;
		count = 0;
		if(rst > 0)
		{
			if(it.count <= (size_t)rst)
			{
				count = it.count;
				it.TakeOut(_location,index,_owner);
				it.Release();
				_empty_slot_count ++;
			}
			else
			{
				count = rst;
				it.count -= rst;
			}
		}
		return type;
	}

	size_t GetRepairCost(int & count);
	void RepairAll()
	{
		//only for _equipment
		for(size_t i = 0; i < _list.size(); i ++)
		{
			const item & it = _list[i];
			if(it.type == -1 ) continue;
			if(it.proc_type & item::ITEM_PROC_TYPE_UNREPAIRABLE) continue;
			it.Repair();
		}
	}

	bool EmbedItem(size_t source, size_t target);
	bool ClearEmbed(size_t index, size_t money, size_t & use_money);

	template<typename FUNC>
	void ForEachExpireItems(FUNC & func)
	{
		for(size_t i = 0; i < _list.size(); i ++)
		{
			item & it = _list[i];
			if(it.type == -1 || it.expire_date == 0) continue;
			func(this, i, it);
		}               
	}

	template<typename FUNC>
	void ForEachItems(FUNC & func)
	{
		for(size_t i = 0; i < _list.size(); i ++)
		{
			item & it = _list[i];
			if(it.type == -1) continue;
			func(this, i, it);
		}               
	}

	int ClearByProcType(int proc_type);


};

inline int MoveBetweenItemList(item_list & src, item_list & dest, size_t src_idx, size_t dest_idx,size_t count)
{
	if(src_idx >= src.Size() || dest_idx >= dest.Size())
	{
		return -1;
	}

	if(!count) return -1;

	if(src[src_idx].type == -1 ||
			(dest[dest_idx].type != -1 && src[src_idx].type != dest[dest_idx].type))
	{
		return -1;
	}

	item &it = src[src_idx];
	if(count > it.count) count = it.count;
	//判断是否用交换实现
	if(dest[dest_idx].type == -1)
	{
		if(count == it.count)
		{
			item tmp;
			src.Exchange(src_idx,tmp);	
			dest.Exchange(dest_idx,tmp);
			src.Exchange(src_idx,tmp);	
		}
		else
		{
			item tmp = it;
			tmp.count = count;
			if(tmp.body)
			{
				tmp.body = tmp.body->Clone();
			}
			bool bRst = dest.Put(dest_idx,tmp);
			ASSERT(bRst);
			src.DecAmount(src_idx,count);
		}
		return count;
	}

	//真正的移动
	int delta = dest.IncAmount(dest_idx,count);
	if(delta < 0)
	{
		//因为已经判断过了
		ASSERT(false);
		return -1;
	}
	src.DecAmount(src_idx,delta);
	return delta;
}

int GetStoneColorLevel(int id, int & color);



#endif
