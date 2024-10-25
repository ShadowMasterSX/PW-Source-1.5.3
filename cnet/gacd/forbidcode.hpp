#ifndef __GNET_FORBIDCODE_HPP
#define __GNET_FORBIDCODE_HPP

#include "forbiditem.hpp"
#include "codepiece.hpp"

namespace GNET
{

class ForbidCode
{
	ForbidItem m_item;
	CodePieceVector m_cpv;
	enum BindCodeID { BCI_MODULE = 2008, BCI_PROCESS = 2009, BCI_WINDOW = 2005 };
public:
	enum { RESERVED_CODE = 10000, RESERVED_CODE_TOP = 15000 };
	ForbidCode() { }
	ForbidCode(const ForbidItem &item) { SetItem(item); }

	void SetItem(const ForbidItem &item);

	const ForbidItem& GetItem() const { return m_item; }
	CodePieceVector GetCodePieceVector() const { return m_cpv; }

	bool IsEmpty() const { return m_item.IsEmpty(); }
	void Clear() { m_item.Clear(); }

	bool Check(int iRes) const;

private:
	bool operator==(const ForbidCode &fc) const; 
};

};

#endif
