#ifndef __GNET_PREPAREDCODEFRAGMENT_HPP
#define __GNET_PREPAREDCODEFRAGMENT_HPP

#include <string>

namespace GNET
{

class PreparedCodeFragment
{
	short m_sCodeID;
	short m_sType;
	std::string m_codeName;
public:
	enum FragmentType { FT_INVALID = -1, FT_FIXED = 0, FT_ALL = 1, FT_PARAM = 2, FT_RANDOM = 3 };

	PreparedCodeFragment() { m_sType = FT_INVALID; }
	PreparedCodeFragment(short sCodeID, std::string codeName = "") 
		: m_sCodeID(sCodeID), m_codeName(codeName)
	{
		if( codeName == "" ) m_sType = FT_ALL;
		else if( codeName == "?" ) m_sType = FT_PARAM;
		else if( codeName == "*" ) m_sType = FT_RANDOM;
		else m_sType = FT_FIXED;
	}
	PreparedCodeFragment(short sCodeID, short sType, std::string codeName = "")
		: m_sCodeID(sCodeID), m_sType(sType), m_codeName(codeName) { }

	bool operator==(const PreparedCodeFragment &pcf) const
	{
		if( &pcf == this ) return true;
		if( m_sCodeID != pcf.m_sCodeID || m_sType != pcf.m_sType ) return false;
		if( m_sType == FT_FIXED && m_codeName != pcf.m_codeName ) return false;
		return true;
	}

	short GetCodeID() const { return m_sCodeID; }
	short GetType() const { return m_sType; }
	std::string GetCodeName() const { return m_codeName; }

	bool IsValid() const { return m_sType != FT_INVALID; }

};

};

#endif
