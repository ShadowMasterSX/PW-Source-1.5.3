#ifndef __GNET_MEMPATTERNITEM_HPP
#define __GNET_MEMPATTERNITEM_HPP

#include <string>

namespace GNET
{

struct MemPatternItem
{
	enum MemPatternType { MPT_LOCAL = 0, MPT_FOREIGN = 1 };

	int m_iID;
	int m_iType;
	int m_iSize;
	int m_iAddr;
	std::string m_name;
	std::string m_pattern;

	MemPatternItem() : m_iID(0), m_iType(MPT_LOCAL), m_iSize(0), m_iAddr(0) { }

	bool IsLocal() const { return m_iType == MPT_LOCAL; }
	bool IsForeign() const { return m_iType == MPT_FOREIGN; }

	bool IsEmpty() const { return m_iID == 0; }
	void Clear() { m_iID = 0; }

};

};

#endif
