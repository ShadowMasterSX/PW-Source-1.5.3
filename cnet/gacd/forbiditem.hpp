#ifndef __GNET_FORBIDITEM_HPP
#define __GNET_FORBIDITEM_HPP

#include <string>

namespace GNET
{

struct ForbidItem
{
	enum ForbidType { FT_MODULE = 6, FT_PROCESS = 8, FT_WINDOW = 9 };
	int m_iID;
	int m_iType;
	int m_iDllSize;
	std::string m_exeName;
	std::string m_dllName;
	bool operator==(const ForbidItem &fi);
public:
	ForbidItem() : m_iID(0), m_iDllSize(0) { }

	void SetDllName(const std::string &str) { if( str == "empty" ) m_dllName = ""; else m_dllName = str; }

	std::string GetForbidName() const { if( m_iType == FT_PROCESS ) return m_exeName; return m_dllName; }
	std::string GetCodeName() const { if( m_iType == FT_PROCESS ) return "findprocess";
		if( m_iType == FT_MODULE ) return "findmodule"; if( m_iType == FT_WINDOW ) return "findwindow"; return ""; }

	bool IsEmpty() const { return m_iID == 0; }
	void Clear() { m_iID = 0; }

};

};

#endif
