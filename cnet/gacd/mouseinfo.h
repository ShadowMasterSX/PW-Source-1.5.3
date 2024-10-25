#ifndef __GNET_MOUSEINFO
#define __GNET_MOUSEINFO

namespace GNET
{

struct Mouse
{
    enum { RegionCount = 9 };
    unsigned int     lbuttondownfreq;
    unsigned int     rbuttondownfreq;
    unsigned int     mousemovefreq;
    float  mousemovevelo;
    unsigned int  g_counts[RegionCount];
    unsigned int  g_times[RegionCount];

	int GetAllCounts() const
	{
		int r = 0;
		for(unsigned int i = 0; i < RegionCount; ++i)
			r += g_counts[i];
		return r;
	}
};

};

#endif
