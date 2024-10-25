#include "string.h"

#include "aggrolist.h"

int 
aggro_list::AddRage(const XID & __id, int __rage)
{	
	if(__rage == 0) return -1;
	AGGROLIST::iterator it = __Find(__id);
	AGGROLIST::iterator it2;
	if(it != _alist.end())
	{
		int newrage = it->rage + __rage;
		if(newrage < 0) newrage = 0;
		if(newrage)
		{
			it2 = it;
			if(__rage > 0)
			{	
				while(it2 != _alist.begin())
				{
					--it2;
					if(it2->rage > newrage) break;
				}

				aggro_node node = *it;
				if(it2->rage > newrage) 
				{
					it2 ++;
				}
				else
				{
					ASSERT(it2 == _alist.begin());
				}
				for(;it != it2;--it)
				{
					*it = *(it-1);
				}
				*it2 = node;
			}
			else
			{
				++it2;
				while(it2 != _alist.end())
				{
					if(it2->rage < newrage) break;
					++it2; 
				}

				aggro_node node = *it;
				--it2;
				for(;it!=it2; ++it)
				{
					*it = *(it+1);
				}
				*it2 = node;
			}
			it2->rage = newrage;
			return it2 - _alist.begin();
		}
		else
		{
			_alist.erase(it);
			return it - _alist.begin();
		}
	}
	else
	{	
		//新加入的条目
		if(__rage < 0) return 0;
		if(_alist.size() >= _max_size)
		{
			_alist.pop_back();
		}
		it2  = __Find(__rage);
		_alist.insert(it2,aggro_node(__id,__rage));
		return it2 - _alist.begin();
	}
}

int
aggro_list::AddRage(const XID & __id, int __rage,int __max_rage)
{
	if(__rage == 0) return -1;
	AGGROLIST::iterator it = __Find(__id);
	if(it == _alist.end() || it->rage < __max_rage)
	{
		return AddRage(__id,__rage);
	}
	return -1;
}


int
aggro_list::AddToFrist(const XID & __id,int __addon_rage)
{
	ASSERT(__addon_rage >= 0);
	AGGROLIST::iterator it = __Find(__id);
	if(it == _alist.end())
	{
		//没有此人，执行特殊逻辑
		if(_alist.size() == 0) 
		{
			if(!__addon_rage) __addon_rage = 3;
			return AddRage(__id,__addon_rage);
		}
		else
		{
			return AddRage(__id,_alist[0].rage + __addon_rage);
		}
	}
	else
	{
		//此人存在，
		return AddRage(__id, _alist[0].rage - it->rage + __addon_rage);
	}
	return -1;
}

int
aggro_list::Remove(const XID & __id)
{
	AGGROLIST::iterator it = __Find(__id);
	if(it == _alist.end()) return -1;
	return _alist.erase(it) - _alist.begin();
}

int
aggro_list::RemoveRage(const XID& __src, const XID& __dest, float __ratio)
{
    AGGROLIST::iterator it = __Find(__src);
    if (it == _alist.end()) return -1;

    int removed_rage = (int)(it->rage * __ratio);
    if (removed_rage == 0) return -1;
    AddRage(__src, -removed_rage);

    if (__src != __dest)
    {
        AddRage(__dest, removed_rage);
    }

    return 0;
}

void 	
aggro_list::RegroupAggro()
{
	AGGROLIST::iterator it = _alist.begin();
	for(;it != _alist.end(); ++it)
	{
		it->rage = 1;
	}
}

void 	
aggro_list::SwapAggro(size_t index1,size_t index2)
{
	if(index1 >= _alist.size() || index2 >= _alist.size()) return;
	abase::swap(_alist[index1].id,_alist[index2].id);
}

void 	
aggro_list::AddToLast(const XID & __id)
{
	AGGROLIST::iterator it = __Find(__id);
	if(it == _alist.end())
	{
		//没有此人则加入此人
		AddRage(__id,1);
	}
	else
	{
		//此人存在，将其仇恨设成1
		AddRage(__id, 1 - it->rage);
	}
}

void 	
aggro_list::Fade()
{
	AGGROLIST::iterator it = _alist.begin();
	for(;it != _alist.end(); ++it)
	{
		int rage = it->rage >> 1;
		if(rage <=0 ) rage = 1;
		it->rage = rage;
	}
}

