#include <stdlib.h>
#include "string.h"

#include "world.h"
#include "filter_man.h"
#include "arandomgen.h"


bool filter_man::Save(archive & ar)
{
	__DelayAddFilter();
	RemoveDeletedFilter();

	size_t count = 0;
	FILTER_MAP::iterator it = _filter_map.begin();
	for(; it != _filter_map.end(); ++ it)
	{
		if(!(it->second->GetMask() & filter::FILTER_MASK_NOSAVE))
		{
			count ++;
		}
	}

	ar << count;
	it = _filter_map.begin();
	for(; it != _filter_map.end(); ++ it)
	{
		if(!(it->second->GetMask() & filter::FILTER_MASK_NOSAVE))
		{
			it->second->SaveInstance(ar);
			count --;
		}
	}
	ASSERT(count == 0);
	return true;
}

bool filter_man::SaveSpecFilters(archive & ar,int mask)
{
	RemoveDeletedFilter();
	size_t count = 0;
	FILTER_MAP::iterator it = _filter_map.begin();
	for(; it != _filter_map.end(); ++ it)
	{
		if(it->second->GetMask() & mask)
		{
			count ++;
		}
	}
	ar << count;
	for(it = _filter_map.begin(); it != _filter_map.end(); ++ it)
	{
		if(it->second->GetMask() & mask)
		{
			it->second->SaveInstance(ar);
			count --; 
		}
	}
	ASSERT(count == 0);
	return true;
}

bool filter_man::Load(archive & ar)
{
	ASSERT(_filter_map.size() == 0);
	Clear();
	size_t size;
	ar >> size;
	for(size_t i = 0; i < size; i ++)
	{
		substance * pSub = substance::LoadInstance(ar);
		filter * f = substance::DynamicCast<filter>(pSub);
		if(!f)
		{
			delete pSub;
			ASSERT(false);
			continue;
		}

		__RawAddFilter(f);
	}
	ASSERT(_filter_map.size() == size);
	return true;
}

int filter_man::RandomRemoveSpecFilters(archive & ar, int mask, int count)
{
	RemoveDeletedFilter();
	std::multimap<float, filter*> candidate_map; 
	FILTER_MAP::iterator it = _filter_map.begin();
	for(; it != _filter_map.end(); ++ it)
	{
		if(it->second->GetMask() & mask)
		{
			candidate_map.insert(std::make_pair(abase::RandUniform(),it->second));
		}
	}
	if(candidate_map.size() == 0) return 0;
	if(count > (int)candidate_map.size()) count = (int)candidate_map.size();
	int old_count = count;
	std::multimap<float, filter*>::iterator it2 = candidate_map.begin();	
	while(count > 0)
	{
		it2->second->SaveInstance(ar);	
		RemoveFilter(it2->second);
		--count;
		++it2;
	}
	return old_count;
}

bool filter_man::AddSpecFilters(archive & ar, int count, gactive_imp * imp)
{
	object_interface parent(imp);
	while(count > 0)
	{
		substance * pSub = substance::LoadInstance(ar);
		filter * f = substance::DynamicCast<filter>(pSub);
		if(!f)
		{
			delete pSub;
			ASSERT(false);
			continue;
		}

		f->_parent = parent;
		AddFilter(f);
		--count;
	}
	return true;
}
