#include "string.h"

#include "world.h"
#include "item_list.h"

/**
 *		物品列表
 */
inline int 
item_list::__try_pile(int __type,size_t & __count,size_t __pile_limit,item*&pEmpty)
{
	int last_index = -1;
	//试图堆叠
	for(size_t i = 0; i < _list.size(); i ++)
	{
		int type = _list[i].type;
		if(type == -1)
		{
			if(pEmpty == NULL) pEmpty = &(_list[i]);
		}
		else if(type == __type && _list[i].count < _list[i].pile_limit)
		{
			//ASSERT(_list[i].pile_limit == __pile_limit);
			int tmp = __count;
			if(tmp + _list[i].count > _list[i].pile_limit)
			{
				tmp =  _list[i].pile_limit - _list[i].count;
			}
			
			_list[i].count += tmp;
			__count -= tmp;
			last_index = i;
			if(__count > 0) continue;
			break;
		}
	}
	return last_index;
}

inline void
item_list::__find_empty(item*&pEmpty)
{
	for(size_t i = 0; i < _list.size(); i ++)
	{
		int type = _list[i].type;
		if(type == -1)
		{
			pEmpty = &(_list[i]);
			break;
		}
	}
	return ;
}

int 
item_list::Push(item_data & data)
{
	if(data.type == -1)
	{
		ASSERT(false && "物品数据不正确");
		return -1;
	}
	item * pEmpty = NULL;
	if(data.pile_limit > 1)	//可以堆叠的物品
	{	
		size_t oldcount = data.count;
		int rst = __try_pile(data.type,data.count,data.pile_limit,pEmpty);
		ASSERT(data.count >=0);
		if(data.count == 0) return rst;
		if(data.count != oldcount && pEmpty == NULL) return rst;
	}
	else
	{
		__find_empty(pEmpty);
	}

	if(pEmpty == NULL) return -1;
	item it;
	if(!MakeItemEntry(it,data)) 
	{

		it.Clear();
		return -1;
	}
	*pEmpty  = it;
	it.Clear();
	data.count = 0;
	size_t pos = pEmpty - _list.begin();
	pEmpty->PutIn(_location,*this,pos,_owner);
	_empty_slot_count --;
	return pos;
}

int 
item_list::Push(const item_data & __data, int & count,int expire_date)
{
	ASSERT(count > 0);
	item_data data = __data;
	data.count = count;
	data.expire_date = expire_date;
	int rst = Push(data);
	count = data.count;
	return rst;
}

int 
item_list::Push(item & it)
{
	if(it.type == -1)
	{
		ASSERT(false && "物品数据不正确");
		return -1;
	}
	if(_empty_slot_count == 0 && it.pile_limit <=1)  return -1;

	item * pEmpty = NULL;
	if(it.pile_limit > 1)	//可以堆叠的物品
	{
		size_t oldcount = it.count;
		int rst = __try_pile(it.type,it.count,it.pile_limit,pEmpty);
		ASSERT(it.count >=0);
		if(it.count == 0) 
		{
			//全部放入，释放物品
			it.Release();
			return rst;
		}
		if(it.count != oldcount && pEmpty == NULL)
		{
			//部分放入 返回错误 不释放物品
			return -1;
		}
	}
	else
	{
		__find_empty(pEmpty);
	}

	if(pEmpty == NULL) return -1;
	*pEmpty = it;
	it.Clear();
	size_t pos = pEmpty - _list.begin();
	pEmpty->PutIn(_location,*this,pos,_owner);
	_empty_slot_count --;
	return pos;
}

int 
item_list::PushInEmpty(int start, const item_data & data , int count)
{
	ASSERT(data.pile_limit >= (size_t)count);
	for(int i = start; (size_t)i < _list.size(); i ++)
	{
		if(_list[i].type != -1) continue;

		item it;
		if(!MakeItemEntry(it,data)) 
		{
			it.Clear();
			return -1;
		}
		it.count = count;
		_list[i] = it;
		it.Clear();
		_list[i].PutIn(_location,*this,i,_owner);
		_empty_slot_count --;
		return i;
	}
	return -1;
}

int 
item_list::PushInEmpty(int start, item & it)
{
	for(int i = start; (size_t)i < _list.size(); i ++)
	{
		if(_list[i].type != -1) continue;

		_list[i] = it;
		it.Clear();
		_list[i].PutIn(_location,*this,i,_owner);
		_empty_slot_count --;
		return i;
	}
	return -1;
}

bool item_list::ClearEmbed(size_t index, size_t money, size_t & use_money)
{
	if(index >= Size()) return false;
	item & it = _list[index];
	if(it.type == -1 || it.body == NULL) return false;
	size_t size = it.body->GetSocketCount();
	int count = 0;
	use_money = 0;
	for(size_t i = 0; i < size; i ++)
	{
		int type = it.body->GetSocketType(i);
		if(type <= 0) continue;
		DATA_TYPE dt;
		STONE_ESSENCE * ess = (STONE_ESSENCE*) world_manager::GetDataMan().get_data_ptr(type, ID_SPACE_ESSENCE, dt);
		if( dt == DT_STONE_ESSENCE)
		{
			count ++;
			use_money += ess->uninstall_price; 
		}
	}
	if(!count  || money < use_money)
	{
		return false;
	}
	it.body->ClearChips();
	return true;
}

size_t 
item_list::GetRepairCost(int & count)
{
	//only for _equipment
	count = 0;
	float cost = 0;
	for(size_t i = 0; i < _list.size(); i ++)
	{
		const item & it = _list[i];
		if(it.type == -1 ) continue;
		if(it.proc_type & item::ITEM_PROC_TYPE_UNREPAIRABLE) continue;
		int durability;
		int max_durability;
		it.GetDurability(durability,max_durability);
		int offset = max_durability - durability;
		if(offset > 0)
		{
			count ++;
			int repair_fee = world_manager::GetDataMan().get_item_repair_fee(it.type);
			cost += player_template::GetRepairCost(offset,max_durability,repair_fee);
		}
	}
	return (size_t)cost;
}

bool 
item_list::EmbedItem(size_t source, size_t target)
{
	if(source >= Size() || target >= Size()) return false;
	if(_list[source].type == -1 || _list[target].type == -1) return false;

	//判断是否宝石和宝石级别是否正确
	DATA_TYPE dt;
	STONE_ESSENCE * st_ess = (STONE_ESSENCE *) world_manager::GetDataMan().get_data_ptr(_list[source].type,ID_SPACE_ESSENCE,dt);
	if(dt != DT_STONE_ESSENCE || ! st_ess) return false;

	const void * pess = world_manager::GetDataMan().get_data_ptr(_list[target].type,ID_SPACE_ESSENCE,dt);
	if(!pess) return false;
	switch(dt)
	{
		case DT_WEAPON_ESSENCE:
		if(st_ess->level > ((WEAPON_ESSENCE*)pess)->level) return false;
		break;
		case DT_ARMOR_ESSENCE:
		if(st_ess->level > ((ARMOR_ESSENCE*)pess)->level) return false;
		break;
		default:
		return false;
	};

	bool bRst = _list[source].InsertTo<0>(_list[target]);
	if(bRst) DecAmount(source,1);
	return bRst;
}

int
item_list::ClearByProcType(int proc_type)
{
	int count = 0;
	for(size_t i = 0; i < _list.size(); i ++)
	{
		item & it = _list[i];
		if(it.type == -1 || (it.proc_type & proc_type) == 0) continue;
		it.TakeOut(_location, i, _owner);
		_empty_slot_count ++;
		it.Release();
		count ++;
	}
	return count;
}

int GetStoneColorLevel(int id,int & color)
{
	DATA_TYPE dt;
	const void * pess = world_manager::GetDataMan().get_data_ptr(id,ID_SPACE_ESSENCE,dt);
	if(!pess || dt != DT_STONE_ESSENCE) return 0;

	STONE_ESSENCE *pSt = (STONE_ESSENCE *)pess;
	color = pSt->color;
	return pSt->level;
}
