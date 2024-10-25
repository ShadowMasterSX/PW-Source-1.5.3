#ifndef __GNET_DEBUGITEM_HPP
#define __GNET_DEBUGITEM_HPP

#include <string>

namespace GNET
{

struct DebugItem
{
	int m_iID;
	int m_iCodeID;
	int m_iRes;
	int m_iResPos;
	int m_iRetPos;
	int m_iCheckSessionPos;

	std::string m_precodeName;
	std::string m_param;

	DebugItem() : m_iID(0), m_iCodeID(0), m_iRes(0), 
		m_iResPos(0), m_iRetPos(0), m_iCheckSessionPos(0) { }

	bool NeedPatchRes() const { return m_iResPos != 0; }
	bool NeedPatchRet() const { return m_iRetPos != 0; }
	bool NeedPatchSession() const { return m_iCheckSessionPos != 0; }

	std::string GetParam() const { return m_param; }

	bool IsEmpty() const { return m_iID == 0; }
	void Clear() { m_iID = 0; }

};

};

#endif
