
#include "debugcode.hpp"
#include "preparedcodelibrary.hpp"

#include "gacdutil.h"

namespace GNET
{

void DebugCode::SetItem(const DebugItem &item)
{
	m_item = item;
	std::vector<std::string> params;
	params.push_back(m_item.GetParam());
	PreparedCodeLibrary::GetInstance()->MakeCode(m_item.m_precodeName, m_cpv, params);
	// patch
	for(CodePieceVector::iterator it = m_cpv.begin(), ie = m_cpv.end(); it != ie; ++it)
	{
		if( (*it).GetID() == m_item.m_iCodeID )
		{
			(*it).ChangeID(m_item.m_iID);
		}
		//
		DEBUG_PRINT_INIT("[%d][%d]\t", (*it).GetID(), (*it).GetSize());
		//
	}
	DEBUG_PRINT_INIT("\nmake debug code pro size %d\n", m_cpv.size());
}


};

