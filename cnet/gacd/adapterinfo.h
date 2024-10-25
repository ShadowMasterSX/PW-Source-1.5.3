#ifndef __GNET_ADAPTERINFO
#define __GNET_ADAPTERINFO

#include <string>

namespace GNET
{

struct AdapterInfo
{
	std::string des;
    void Clear() { des.clear(); }
    bool IsEmpty() const { return des.empty(); }
	bool operator==(const AdapterInfo &info) const { return this == &info ||
		des == info.des; }
	bool operator<(const AdapterInfo &info) const { if( this == &info ) return false;
		return des < info.des; }
	void SetDes(const std::string &str);
};

};

#endif
