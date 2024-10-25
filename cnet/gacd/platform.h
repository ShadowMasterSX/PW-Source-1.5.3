#ifndef __GNET_PLATFORM
#define __GNET_PLATFORM

#include <string>

namespace GNET
{

struct Platform
{
    unsigned int id;
    unsigned int majorversion;
    unsigned int minorversion;
    unsigned int buildnumber;
    unsigned int mem_size;
    unsigned short cpu_arc;
    unsigned short cpu_lvl;
    unsigned int cpu_ct;
	std::string adp_des;
    Platform() : id(0) { }
    void Clear() { id = 0; }
    bool IsEmpty() const { return id == 0; }
	bool operator==(const Platform &pl) const { return this == &pl ||
		id == pl.id && majorversion == pl.majorversion && minorversion == pl.minorversion
		&& buildnumber == pl.buildnumber; }
	bool operator<(const Platform &pl) const { if( this == &pl ) return false;
		if( id < pl.id ) return true;
		if( id == pl.id && majorversion < pl.majorversion ) return true;
		if( id == pl.id && majorversion == pl.majorversion && minorversion < pl.minorversion ) return true;
		if( id == pl.id && majorversion == pl.majorversion && minorversion == pl.minorversion 
			&& buildnumber < pl.buildnumber ) return true;
		return false; }
};

};

#endif
