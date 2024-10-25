#ifndef __GNET_MEMPATTERNCODE_HPP
#define __GNET_MEMPATTERNCODE_HPP

#include "mempatternitem.hpp"
#include "codepiece.hpp"

namespace GNET
{

class MemPatternCode
{
	MemPatternItem m_item;
	CodePieceVector m_cpv;
	enum { 
		BCI_LOCAL_PATTERN_CODE = 1997,
		BCI_FOREIGN_PATTERN_CODE = 1999,
		BCI_LOCAL_PATTERN = 5997,
		BCI_FOREIGN_PATTERN = 5999
	};
public:
	enum { RESERVED_CODE = 20000, RESERVED_CODE_TOP = 25000 };

	MemPatternCode() { }
	MemPatternCode(const MemPatternItem &item) { SetItem(item); }

	void SetItem(const MemPatternItem &item);

	void PatchRes(int iRes);

	const MemPatternItem& GetItem() const { return m_item; }

	CodePieceVector GetCodePieceVector() const { return m_cpv; }

	bool IsEmpty() const { return m_item.IsEmpty(); }
	int GetType() const { return m_item.m_iType; }
	void Clear() { m_item.Clear(); }

};

};

#endif
