#ifndef __GNET_STRINFO
#define __GNET_STRINFO

#include <string>
#include <vector>
#include "strings.h"

namespace GNET
{

typedef std::vector<std::string> VecStr;
typedef std::vector<std::pair<std::string, int> > VecStrI;

struct stringcasecmp
{
    bool operator() (const std::string &x, const std::string &y) const { return strcasecmp(x.c_str(), y.c_str()) < 0;}
};

struct stringintcasecmp
{
	bool operator() (const std::pair<std::string, int> &x, const std::pair<std::string, int> &y) const
	{
		int r1 = strcasecmp(x.first.c_str(), y.first.c_str());
		if( r1 < 0 ) return true;
		if( r1 > 0 ) return false;
		return x.second < y.second;
	}
};

struct stringcasepred
{
    bool operator() (const std::string &x, const std::string &y) const { return strcasecmp(x.c_str(), y.c_str()) == 0;}
};
struct stringintcasepred
{
	bool operator() (const std::pair<std::string, int> &x, const std::pair<std::string, int> &y) const
	{
		return 0 == strcasecmp(x.first.c_str(), y.first.c_str())
		&& x.second == y.second;
	}
};

struct StrInfo
{
    VecStr process_str;
    VecStr window_str;
    VecStrI module_str;
	void Clear()
	{
		process_str.clear(); window_str.clear(); module_str.clear();
	}
	bool operator==(const StrInfo &info) const
	{
		return &info == this || process_str == info.process_str
			&& window_str == info.window_str && module_str == info.module_str;
	}
	StrInfo& Unique();
	bool IsEmpty() const { return process_str.empty() && window_str.empty() && module_str.empty(); }
	void Dump() const;
};

StrInfo operator+(const StrInfo &info1, const StrInfo &info2);


};

#endif
