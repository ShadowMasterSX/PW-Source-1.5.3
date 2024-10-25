#include "gacdutil.h"
#include "strinfo.h"
#include <set>
#include <algorithm>
#include <functional>

namespace GNET
{

StrInfo operator+(const StrInfo &info1, const StrInfo &info2)
{
	StrInfo info;
	for(VecStr::const_iterator it = info1.window_str.begin(), ie = info1.window_str.end();
		it != ie; ++it)
	{
		for(VecStr::const_iterator it2 = info2.window_str.begin(), ie2 = info2.window_str.end();
			it2 != ie2; ++it2)
		{
			if( strcasecmp((*it).c_str(), (*it2).c_str()) == 0 )
			{
				info.window_str.push_back((*it));
				break;
			}
		}
	}
	for(VecStr::const_iterator it = info1.process_str.begin(), ie = info1.process_str.end();
		it != ie; ++it)
	{
		for(VecStr::const_iterator it2 = info2.process_str.begin(), ie2 = info2.process_str.end();
			it2 != ie2; ++it2)
		{
			if( strcasecmp((*it).c_str(), (*it2).c_str()) == 0 )
			{
				info.process_str.push_back((*it));
				break;
			}
		}
	}
	for(VecStrI::const_iterator it = info1.module_str.begin(), ie = info1.module_str.end();
		it != ie; ++it)
	{
		bool tr = false;
		for(VecStrI::const_iterator it2 = info2.module_str.begin(), ie2 = info2.module_str.end();
			it2 != ie2; ++it2)
		{
			if( strcasecmp((*it).first.c_str(), (*it2).first.c_str()) == 0 )
			{
				tr = true;
				info.module_str.push_back((*it2));
			}
		}
		if( tr ) info.module_str.push_back((*it));
	}
	info.Unique();
	return info;
}

StrInfo& StrInfo::Unique()
{
	VecStr newp;
	for(VecStr::const_iterator it = process_str.begin(), ie = process_str.end(); it != ie; ++it)
	{
		bool tr = true;
		for(VecStr::const_iterator it2 = newp.begin(), ie2 = newp.end(); it2 != ie2; ++it2)
		{
			if(strcasecmp((*it).c_str(), (*it2).c_str()) == 0 )
			{
				tr = false;
				break;
			}
		}
		if( tr )
			newp.push_back((*it));
	}
	process_str = newp;
	VecStr neww;
	for(VecStr::const_iterator it = window_str.begin(), ie = window_str.end(); it != ie; ++it)
	{
		bool tr = true;
		for(VecStr::const_iterator it2 = neww.begin(), ie2 = neww.end(); it2 != ie2; ++it2)
		{
			if(strcasecmp((*it).c_str(), (*it2).c_str()) == 0 )
			{
				tr = false;
				break;
			}
		}
		if( tr )
			neww.push_back((*it));
	}
	window_str = neww;
	VecStrI newm;
	for(VecStrI::const_iterator it = module_str.begin(), ie = module_str.end(); it != ie; ++it)
	{
		bool tr = true;
		for(VecStrI::const_iterator it2 = newm.begin(), ie2 = newm.end(); it2 != ie2; ++it2)
		{
			if(strcasecmp((*it).first.c_str(), (*it2).first.c_str()) == 0 && (*it).second == (*it2).second )
			{
				tr = false;
				break;
			}
		}
		if( tr )
			newm.push_back((*it));
	}
	module_str = newm;

	return *this;
}

void StrInfo::Dump() const
{
	GACD_STD_DEBUG_PRINT("\twindows:\n");
	for(VecStr::const_iterator it = window_str.begin(), ie = window_str.end(); it != ie; ++it)
		GACD_STD_DEBUG_PRINT("[%s]  ", (*it).c_str());
	GACD_STD_DEBUG_PRINT("\n");
	GACD_STD_DEBUG_PRINT("\tprocesses:\n");
	for(VecStr::const_iterator it = process_str.begin(), ie = process_str.end(); it != ie; ++it)
		GACD_STD_DEBUG_PRINT("[%s]  ", (*it).c_str());
	GACD_STD_DEBUG_PRINT("\n");
	GACD_STD_DEBUG_PRINT("\tmodules:\n");
	for(VecStrI::const_iterator it = module_str.begin(), ie = module_str.end(); it != ie; ++it)
		GACD_STD_DEBUG_PRINT("[%s][%d]  ", (*it).first.c_str(), (*it).second);
	GACD_STD_DEBUG_PRINT("\n");
}
};
