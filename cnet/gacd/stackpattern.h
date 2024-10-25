#ifndef __GNET_STACKPATTERN
#define __GNET_STACKPATTERN

#include <map>

namespace GNET
{

struct StackPattern
{
	unsigned int m_uCaller;
	unsigned int m_uSize;
	unsigned int m_uPattern;

    StackPattern() : m_uSize(0) { }
	StackPattern(unsigned int uCaller, unsigned int uSize, unsigned int uPattern)
		: m_uCaller(uCaller), m_uSize(uSize), m_uPattern(uPattern) { }
	StackPattern(unsigned int uPattern) : m_uCaller(0), m_uSize(1), m_uPattern(uPattern) { }
    void Clear() { m_uSize = 0; }
    bool IsEmpty() const { return m_uSize == 0; }

	bool operator<(const StackPattern &sp) const { if( this == &sp ) return false;
		if( m_uCaller < sp.m_uCaller ) return true;
		if( m_uCaller == sp.m_uCaller && m_uSize < sp.m_uSize ) return true;
		if( m_uCaller == sp.m_uCaller && m_uSize == sp.m_uSize && m_uPattern < sp.m_uPattern ) 
			return true;
		return false; 
	}
};

typedef std::map<StackPattern, unsigned long long> PatternMap;

};

#endif
