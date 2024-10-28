#ifndef __ONLINEGAME_GS_FILTER_MANAGER_H__
#define __ONLINEGAME_GS_FILTER_MANAGER_H__

#include "filter.h"
#include <vector.h>
#include <algorithm>
#include <functional>
#include "staticmap.h"
#include <amemory.h>
#include <map>

class gactive_imp;
class filter_man
{
	//typedef abase::static_multimap<int,filter*, abase::fast_alloc<> >  FILTER_MAP;
	typedef std::multimap<int,filter*> FILTER_MAP;
	typedef abase::vector<filter*,abase::fast_alloc<> > list;
	abase::vector<list,abase::fast_alloc<> > _filter_pool;
	FILTER_MAP _filter_map;
	int _call_mask;
//	gactive_imp * _parent;
	bool _has_wait_delete;
	list _wait_for_insert;
	bool _delay_add_execute;
	int _operation_count;
//	bool _in_heartbeat;
protected:
	void RemoveFilterFromPool(filter * obj)
	{
		ASSERT(!(obj->GetMask() & _call_mask));
		int oldmask = obj->GetMask();
		for(size_t i = 0; i < FILTER_IDX_MAX; ++i)
		{
			if( (1<<i) & oldmask) 
			{
				list & __list = _filter_pool[i];
				list::iterator it = std::find(__list.begin(),__list.end(),obj);
				if(it != __list.end())
				{
					__list.erase(it);
				}
				else
				{
					//这个错误是不应该的
					ASSERT(false);
				}
			}
		}
	}

	void RemoveFilterFromMap(filter * obj)
	{
		int id = obj->GetFilterID();
		FILTER_MAP::iterator it = _filter_map.find(id);
		ASSERT(it != _filter_map.end());
		while(it != _filter_map.end() && it->first == id)
		{
			if(it -> second == obj)
			{
				FILTER_MAP::iterator it2 = it;
				++ it2;
				_filter_map.erase(it);
				it = it2;
				obj->Release();
				break;
			}
			++it;
		}
	}

	void __RawAddFilter(filter * obj)
	{
		int mask = obj->GetMask(); 
		int id = obj->GetFilterID();
		_filter_map.insert(std::pair<int,filter*>(id,obj));
		//依次加入到正确的位置中
		for(size_t i = 0; i < FILTER_IDX_MAX; ++i)
		{
			if( (1<<i) & mask) _filter_pool[i].push_back(obj);
		}
		return ;
	}

	void __DelayAddFilter()
	{
		if(!_wait_for_insert.empty())	
		{
			list tmp;
			tmp.swap(_wait_for_insert);
			_delay_add_execute = true;
			for(size_t i = 0; i < tmp.size(); i ++)
				AddFilter(tmp[i]);
			_delay_add_execute = false;
		}
	}
public:
	filter_man():_filter_pool(FILTER_IDX_MAX,list()), _call_mask(0),_has_wait_delete(false),_delay_add_execute(false),_operation_count(0)
	{}

	~filter_man()
	{
		Clear();
	}
	void Swap(filter_man & rhs, gactive_imp * imp)
	{
		_filter_pool.swap(rhs._filter_pool);
		_filter_map.swap(rhs._filter_map);
		abase::swap(_has_wait_delete,rhs._has_wait_delete);
		_wait_for_insert.swap(rhs._wait_for_insert);
		abase::swap(_delay_add_execute,rhs._delay_add_execute);
		abase::swap(_operation_count,rhs._operation_count);
		//重新设置新的object_interface
		Init(imp);
	}

	void Init(gactive_imp * imp)
	{
		FILTER_MAP::iterator it =  _filter_map.begin();
		object_interface parent(imp);
		for(;it != _filter_map.end(); ++it)
		{
			it->second->_parent = parent;
		}
	}

	void DBInit(gactive_imp * imp)
	{
		FILTER_MAP::iterator it =  _filter_map.begin();
		object_interface parent(imp);
		for(;it != _filter_map.end(); ++it)
		{
			it->second->_parent = parent;
			it->second->OnAttach();
		}
	}
	
	void Clear()
	{
		FILTER_MAP::iterator it =  _filter_map.begin();
		for(;it != _filter_map.end(); ++it)
		{
			//这里直接释放
			delete it->second;
		}
		_filter_pool.clear();
		_filter_pool.insert(_filter_pool.begin(),FILTER_IDX_MAX,list());
		
		list::iterator lit = _wait_for_insert.begin();
		for(;lit != _wait_for_insert.end(); ++lit)
			delete *lit;
		_wait_for_insert.clear();
		_delay_add_execute = false;
	}

	inline bool IsFilterExist(int id)
	{
		return !(_filter_map.find(id) == _filter_map.end());
	}

	bool AddFilter(filter * obj)
	{
		int mask = obj->GetMask();
		//ASSERT(!(mask & _call_mask));// todo ddr 20140506
		if(mask & _call_mask) 
		{	
			if(!_delay_add_execute)
				_wait_for_insert.push_back(obj);
			else
				ASSERT(false && "filter 延迟插入依旧冲突");
			return true;
		}

		int id = obj->GetFilterID();
		FILTER_MAP::iterator it = _filter_map.find(id);
		if(it != _filter_map.end())
		if((mask & filter::FILTER_MASK_UNIQUE))
		{
			//filter是排他的
			//存在这样的filter 删除之
			while(it != _filter_map.end() && it->first == id)
			{
				filter * old = it->second;
				RemoveFilterFromPool(old);
				it->second->Release();

				FILTER_MAP::iterator it2 = it;
				++ it2;
				_filter_map.erase(it);
				it = it2;
			}
		}
		else if(mask & filter::FILTER_MASK_WEAK)
		{
			//filter 是弱的,即有相同的filter存在的情况下不再次加入
			//这里应该直接释放 
			delete obj;
			return false;
		}
		else if(mask & filter::FILTER_MASK_MERGE)
		{
			//是需要融合的
			filter * old = it->second;
			ASSERT(old->GetMask() & filter::FILTER_MASK_MERGE);
			old->Merge(obj);
			//这里应该直接释放 
			delete obj;
			return false;
		}
		

		_filter_map.insert(std::pair<int,filter*>(id,obj));
		//依次加入到正确的位置中
		for(size_t i = 0; i < FILTER_IDX_MAX; ++i)
		{
			if( (1<<i) & mask) _filter_pool[i].push_back(obj);
		}
		_call_mask |= mask & filter::FILTER_MASK_ALL;
		obj->OnAttach();
		_call_mask &= ~(mask & filter::FILTER_MASK_ALL);
		return true;
	}

	void RemoveFilter(int id)
	{
		FILTER_MAP::iterator it = _filter_map.find(id);
		while(it != _filter_map.end() && it->first == id)
		{
			filter * old = it->second;
			if(_call_mask & old->GetMask())
			{
				//如果嵌套调用，则标记删除
				old->_is_deleted = true;
				_has_wait_delete = true;
				++it;
			}
			else
			{
				//如果没有嵌套删除，直接清除
				RemoveFilterFromPool(old);
				FILTER_MAP::iterator it2 = it;
				++ it2;
				_filter_map.erase(it);
				it = it2;
				old->Release();
			}
		}
	}

	void RemoveFilter(filter * obj)
	{
		int id = obj->GetFilterID();
		ASSERT(!(obj->GetMask() & _call_mask));
		FILTER_MAP::iterator it = _filter_map.find(id);
		ASSERT(it != _filter_map.end());
		while(it != _filter_map.end() && it->first == id)
		{
			if(it -> second == obj)
			{
				filter * old = it->second;
				RemoveFilterFromPool(old);
				FILTER_MAP::iterator it2 = it;
				++ it2;
				_filter_map.erase(it);
				it = it2;
				old->Release();
				break;
			}
			++it;
		}
	}

	/*
	*	此函数只是标记为删除，而不是真正删除
	*/
	bool ClearSpecFilter(int mask = filter::FILTER_MASK_DEBUFF)
	{
		bool rst = false;
		FILTER_MAP::iterator it = _filter_map.begin();
		for(; it != _filter_map.end();)
		{
			//存在这样的filter 删除之
			filter * old = it->second;
			int oldmask = old->GetMask();
			if(oldmask & mask)
			{
				old->_is_deleted = true;
				_has_wait_delete = true;
				rst = true;
			}
			++it;
		}

		if(_call_mask == 0) 
		{
			RemoveDeletedFilter();
		}
		return rst;
	}

	inline void RemoveDeletedFilter()
	{
		if(!_has_wait_delete) return;
		FILTER_MAP::iterator it = _filter_map.begin();
		for(; it != _filter_map.end();)
		{
			//存在这样的filter 删除之
			filter * old = it->second;
			if(old->IsDeleted())
			{
				if(old->GetMask() & filter::FILTER_MASK_ALL) RemoveFilterFromPool(old);
				FILTER_MAP::iterator it2 = it;
				++ it2;
				_filter_map.erase(it);
				it = it2;
				old->Release();
			}
			else
			{
				++it;
			}
		}
		_has_wait_delete = false;
	}

	bool ModifyFilter(int filterid, int ctrlname, void * ctrlval, size_t ctrllen)
	{
		FILTER_MAP::iterator it = _filter_map.find(filterid);
		if(it == _filter_map.end()) return false;

		//
		bool is_set_call_mask = false;
		if(_call_mask == 0)
		{
			//do something
			_call_mask = filter::FILTER_MASK_ALL;
			is_set_call_mask = true;
		}
		while(it != _filter_map.end() && it->first == filterid)
		{
			filter * obj = it->second;
			if(!obj->IsDeleted())
			{
				obj->Modify(ctrlname,ctrlval,ctrllen);
			}
			++it;
		}
		
		if(is_set_call_mask)
		{
			_call_mask = 0;
		}
		return true;
	}

	int QueryFilter(int filterid, int index) // 只查询第一个
	{
		FILTER_MAP::iterator it = _filter_map.find(filterid);
		if(it == _filter_map.end()) return 0;
		
		return it->second->OnQuery(index);
	}
private:

	inline void RemoveInList(int inv_mask, list & __list, list::iterator & it)
	{
		filter * obj = *it;
		obj->_mask &= inv_mask;
		it = __list.erase(it);
		_has_wait_delete = true;
	}
	template <int MASK>
	class call_mask_keeper
	{
		int & _call_mask;
		filter_man& _man;
		public:
			call_mask_keeper(int & mask,filter_man& man):_call_mask(mask),_man(man)
			{
				ASSERT(!(_call_mask & MASK));
				_call_mask |= MASK;
				++_man._operation_count;
			}

			~call_mask_keeper()
			{
				_call_mask &= ~MASK;
				if(--_man._operation_count == 0)
				{
					_man.__DelayAddFilter();
				}
			}

	};
public:
	inline void EF_TransSendAttack(const XID & target, attack_msg &arg)
	{
		call_mask_keeper<filter::FILTER_MASK_TRANSLATE_SEND_MSG> keeper(_call_mask,*this);
		list & __list = _filter_pool[FILTER_IDX_TRANSLATE_SEND_MSG];
		for(list::iterator it = __list.begin(); it != __list.end(); )
		{
			filter * obj = *it;
			if(!obj->IsDeleted()) 
			{
				obj->TranslateSendAttack(target,arg);
				if(obj->_is_deleted) 
				{
					//先从自己的表里删除
					RemoveInList( ~(1 << FILTER_IDX_TRANSLATE_SEND_MSG),__list,it);
					continue;
				}
			}
			++it;
		}
		return ;
	}

	inline void EF_TransSendEnchant(const XID & target, enchant_msg & enchant)
	{
		call_mask_keeper<filter::FILTER_MASK_TRANSLATE_SEND_ENCHANT> keeper(_call_mask,*this);
		list & __list = _filter_pool[FILTER_IDX_TRANSLATE_SEND_ENCHANT];
		for(list::iterator it = __list.begin(); it != __list.end(); )
		{
			filter * obj = *it;
			if(!obj->IsDeleted()) 
			{
				obj->TranslateSendEnchant(target,enchant);
				if(obj->_is_deleted) 
				{
					//先从自己的表里删除
					RemoveInList( ~(1 << FILTER_IDX_TRANSLATE_SEND_ENCHANT),__list,it);
					continue;
				}
			}
			++it;
		}
		return ;
	}

	inline void EF_TransRecvAttack(const XID & attacker, attack_msg & arg)
	{
		call_mask_keeper<filter::FILTER_MASK_TRANSLATE_RECV_MSG> keeper(_call_mask,*this);
		list & __list = _filter_pool[FILTER_IDX_TRANSLATE_RECV_MSG];
		for(list::iterator it = __list.begin(); it != __list.end();)
		{
			filter * obj = *it;
			if(!obj->IsDeleted()) 
			{
				obj->TranslateRecvAttack(attacker,arg);
				if(obj->_is_deleted)
				{
					RemoveInList( ~(1 << FILTER_IDX_TRANSLATE_RECV_MSG),__list,it);
					continue;
				}
			}
			++it;
		}
		return ;
	}
	
	inline void EF_TransRecvEnchant(const XID & attacker, enchant_msg & arg)
	{
		call_mask_keeper<filter::FILTER_MASK_TRANSLATE_ENCHANT> keeper(_call_mask,*this);
		list & __list = _filter_pool[FILTER_IDX_TRANSLATE_ENCHANT];
		for(list::iterator it = __list.begin(); it != __list.end();)
		{
			filter * obj = *it;
			if(!obj->IsDeleted()) 
			{
				obj->TranslateEnchant(attacker,arg);
				if(obj->_is_deleted)
				{
					RemoveInList( ~(1 << FILTER_IDX_TRANSLATE_ENCHANT),__list,it);
					continue;
				}
			}
			++it;
		}
		return ;
	}
	
	inline void EF_Heartbeat(int tick)
	{
		//清除可能有的需要被删除的对象
		RemoveDeletedFilter();
		//这里不用keeper，因为与removedeletedfilter有冲突
		//恢复使用keeper 加{}生命期限制 todo ddr 20140807
		{
		call_mask_keeper<filter::FILTER_MASK_HEARTBEAT> keeper(_call_mask,*this);
		list & __list = _filter_pool[FILTER_IDX_HEARTBEAT];
		for(list::iterator it = __list.begin(); it != __list.end();)
		{
			filter * obj = *it;
			obj->Heartbeat(tick);
			if(obj->_is_deleted)
			{
				RemoveInList( ~(1 << FILTER_IDX_HEARTBEAT),__list,it);
				ASSERT(it <= __list.end());
				continue;
			}
			++it;
		}
		}
		//再次清除可能有的需要被删除的对象
		RemoveDeletedFilter();
		return ;
	}

	inline void EF_AdjustDamage(damage_entry & dmg,const XID & attacker, const attack_msg & msg,float damage_adjust) 
	{
		call_mask_keeper<filter::FILTER_MASK_ADJUST_DAMAGE> keeper(_call_mask,*this);
		list & __list = _filter_pool[FILTER_IDX_ADJUST_DAMAGE];
		for(list::iterator it = __list.begin(); it != __list.end();)
		{
			filter * obj = *it;
			if(!obj->IsDeleted()) 
			{
				obj->AdjustDamage(dmg,attacker,msg,damage_adjust);
				if(obj->_is_deleted) 
				{
					RemoveInList( ~(1 << FILTER_IDX_ADJUST_DAMAGE),__list,it);
					continue;
				}
			}
			++it;
		}
		return ;
	}
	
	inline void EF_DoDamage(const damage_entry & dmg) 
	{
		call_mask_keeper<filter::FILTER_MASK_DO_DAMAGE> keeper(_call_mask,*this);
		list & __list = _filter_pool[FILTER_IDX_DO_DAMAGE];
		for(list::iterator it = __list.begin(); it != __list.end();)
		{
			filter * obj = *it;
			if(!obj->IsDeleted()) 
			{
				obj->DoDamage(dmg);
				if(obj->_is_deleted)
				{
					RemoveInList( ~(1 << FILTER_IDX_DO_DAMAGE),__list,it);
					continue;
				}
			}
			++it;
		}
		return ;
	}

	inline void EF_AdjustExp(int type, int & exp)
	{
		call_mask_keeper<filter::FILTER_MASK_ADJUST_EXP> keeper(_call_mask,*this);
		list & __list = _filter_pool[FILTER_IDX_ADJUST_EXP];
		for(list::iterator it = __list.begin(); it != __list.end();)
		{
			filter * obj = *it;
			if(!obj->IsDeleted()) 
			{
				obj->AdjustExp(type,exp);
				if(obj->_is_deleted) 
				{
					RemoveInList( ~(1 << FILTER_IDX_ADJUST_EXP),__list,it);
					continue;
				}
			}
			++it;
		}
		return ;
	}

	inline void EF_AdjustManaCost(int & mana)
	{
		call_mask_keeper<filter::FILTER_MASK_ADJUST_MANA_COST> keeper(_call_mask,*this);
		list & __list = _filter_pool[FILTER_IDX_ADJUST_MANA_COST];
		for(list::iterator it = __list.begin(); it != __list.end();)
		{
			filter * obj = *it;
			if(!obj->IsDeleted()) 
			{
				obj->AdjustManaCost(mana);
				if(obj->_is_deleted)
				{
					RemoveInList( ~(1 << FILTER_IDX_ADJUST_MANA_COST),__list,it);
					continue;
				}
			}
			++it;
		}
		return ;
	}

	inline void EF_BeforeDeath(const XID & attacker, char attacker_mode)
	{
		call_mask_keeper<filter::FILTER_MASK_BEFORE_DEATH> keeper(_call_mask,*this);
		list & __list = _filter_pool[FILTER_IDX_BEFORE_DEATH];
		for(list::iterator it = __list.begin(); it != __list.end();)
		{
			filter * obj = *it;
			if(!obj->IsDeleted()) 
			{
				obj->BeforeDeath(attacker,attacker_mode);
				if(obj->_is_deleted) 
				{
					RemoveInList( ~(1 << FILTER_IDX_BEFORE_DEATH),__list,it);
					continue;
				}
			}
			++it;
		}
		return ;
	}
	inline void EF_AdjustHeal(int& heal,char heal_type)
	{
		call_mask_keeper<filter::FILTER_MASK_ADJUST_HEAL> keeper(_call_mask,*this);
		list & __list = _filter_pool[FILTER_IDX_ADJUST_HEAL];
		for(list::iterator it = __list.begin(); it != __list.end();)
		{
			filter * obj = *it;
			if(!obj->IsDeleted()) 
			{
				obj->AdjustHeal(heal,heal_type);
				if(obj->_is_deleted) 
				{
					RemoveInList( ~(1 << FILTER_IDX_ADJUST_HEAL),__list,it);
					continue;
				}
			}
			++it;
		}
		return ;
	}
	inline void EF_DoEnchant(const XID & attacker,enchant_msg & msg)
	{
		call_mask_keeper<filter::FILTER_MASK_DO_ENCHANT> keeper(_call_mask,*this);
		list & __list = _filter_pool[FILTER_IDX_DO_ENCHANT];
		for(list::iterator it = __list.begin(); it != __list.end();)
		{
			filter * obj = *it;
			if(!obj->IsDeleted()) 
			{
				obj->DoEnchant(attacker,msg);
				if(obj->_is_deleted) 
				{
					RemoveInList( ~(1 << FILTER_IDX_DO_ENCHANT),__list,it);
					continue;
				}
			}
			++it;
		}
		return ;
	}
	bool Save(archive & ar);
	bool Load(archive & ar);

	bool SaveSpecFilters(archive & ar, int mask);		//用于存盘，存储的方式和Save保持一致

	int RandomRemoveSpecFilters(archive & ar, int mask, int count);
	bool AddSpecFilters(archive & ar, int count, gactive_imp * imp);
};

enum
{	
	FILTER_INDEX_SKILL_SESSION 	= 13,
	FILTER_INDEX_HEALING 		= 14,
	FILTER_INDEX_MANA_REGEN 	= 15,
	FILTER_FLY_EFFECT		= 16,	//这个id会对应几种不同的filter ，类型应该是unique
	FILTER_RESISTANCE_RUNE		= 17,
	FILTER_DEFENSE_RUNE		= 18,
	FILTER_INVINCIBLE		= 19,
	FILTER_INDEX_SITDOWN		= 20,
	FILTER_INDEX_PVPLIMIT		= 21,
	FILTER_INDEX_TRAVEL		= 22,
	FILTER_INDEX_GATHER_SESSION 	= 23,
	FILTER_INDEX_PASSIVE_WAIT	= 24,
	FILTER_INVINCIBLE_SPEC_ID	= 25,
	FILTER_INDEX_FACEPILL		= 26,
	FILTER_INVINCIBLE_BANISH	= 27,
	FILTER_CHECK_INSTANCE_KEY	= 28,
	FILTER_INDEX_DUEL_FILTER	= 29,
	FILTER_INDEX_MOUNT_FILTER	= 30,
	FILTER_INDEX_TOWERBUILD		= 31,
	FILTER_INDEX_PET_DAMAGE		= 32,
	FILTER_INDEX_INVISIBLE		= 33,	
	FILTER_INDEX_SHIELDUSER		= 34,
	FILTER_INDEX_GM_INVISIBLE	= 35,
	FILTER_INDEX_WEDDING		= 36,
	FILTER_CHECK_KICKOUT		= 37,
	FILTER_INDEX_ONLINE_AWARD_EXP = 38,
	FILTER_INDEX_PK_PROTECTED	= 39,
	FILTER_INDEX_NONPENALTY_PVP	= 40,
	FILTER_INDEX_GTAWARD		= 41,
	FILTER_INDEX_MOVING_SKILL	= 42,

	FILTER_INDEX_SKILL_BASE = 4096,
	FILTER_INDEX_SKILL_END  = 8191,
};
#endif
