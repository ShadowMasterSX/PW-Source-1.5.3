#ifndef __GNET_MEMINFO
#define __GNET_MEMINFO

namespace GNET
{

struct MemInfo
{
    unsigned int ct;
    MemInfo() : ct(0) { }
    void Clear() { ct = 0; }
    bool IsEmpty() const { return ct == 0; }
	bool operator==(const MemInfo &info) const { return this == &info ||
		ct == info.ct; }
	bool operator<(const MemInfo &info) const { if( this == &info ) return false;
		return ct < info.ct; }
	void SetCT(unsigned int _ct)
	{
		ct = _ct/(64*1024*1024);
	}
};

};

#endif
