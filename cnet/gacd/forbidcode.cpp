
#include "gacdutil.h"
#include "forbidcode.hpp"
#include "preparedcodelibrary.hpp"

namespace GNET
{

void ForbidCode::SetItem(const ForbidItem &item)
{
	m_item = item;
	std::vector<std::string> params;
	params.push_back(item.GetForbidName());
	PreparedCodeLibrary::GetInstance()->MakeCode(item.GetCodeName(), m_cpv, params);
	// patch
	//refs.clear(); refs[item.GetID()]++;
	for(CodePieceVector::iterator it = m_cpv.begin(), ie = m_cpv.end(); it != ie; ++it)
	{
		if( (*it).GetID() == BCI_MODULE || (*it).GetID() == BCI_PROCESS || (*it).GetID() == BCI_WINDOW )
			(*it).ChangeID(m_item.m_iID);
	}
	DEBUG_PRINT_INIT("make forbid pro size %d\n", m_cpv.size());
}

bool ForbidCode::Check(int iRes) const
{
	if( iRes == 0 ) return false;
	if( m_item.m_iType == BCI_PROCESS || m_item.m_iType == BCI_WINDOW )
		return true;
	if( m_item.m_iType == BCI_MODULE )
	{	
		if( m_item.m_iDllSize == 0 || m_item.m_iDllSize == iRes )
			return true;
	}
	return false;
}


};

