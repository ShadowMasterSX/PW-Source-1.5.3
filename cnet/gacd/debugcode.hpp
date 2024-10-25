#ifndef __GNET_DEBUGCODE_HPP
#define __GNET_DEBUGCODE_HPP

#include "debugitem.hpp"
#include "codepiece.hpp"

namespace GNET
{

class DebugCode
{
	DebugItem m_item;
	CodePieceVector m_cpv;
public:
	enum { RESERVED_CODE = 15001, RESERVED_CODE_TOP = 16000 };
	DebugCode() { }
	DebugCode(const DebugItem &item) { SetItem(item); }

	void SetItem(const DebugItem &item);

	const DebugItem& GetItem() const { return m_item; }
	CodePieceVector GetCodePieceVector() const { return m_cpv; }

	bool IsEmpty() const { return m_item.IsEmpty(); }
	void Clear() { m_item.Clear(); }

};

};

#endif
