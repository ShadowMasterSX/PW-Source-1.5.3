#ifndef __GNET_CPUINFO
#define __GNET_CPUINFO

namespace GNET
{

struct CPUInfo
{
	unsigned short arch;
	unsigned short level;
    unsigned int ct;
    CPUInfo() : ct(0) { }
    void Clear() { ct = 0; }
    bool IsEmpty() const { return ct == 0; }
	bool operator==(const CPUInfo &info) const { return this == &info ||
		arch == info.arch && level == info.level && ct == info.ct; }
	bool operator<(const CPUInfo &info) const { if( this == &info ) return false;
		if( arch < info.arch ) return true;
		if( arch == info.arch && level < info.level ) return true;
		if( arch == info.arch && level == info.level && ct < info.ct ) return true;
		return false; }
	void SetCT(unsigned int _ct)
    {
        ct = _ct/(50*1000*1000);
    }
};

};

#endif
