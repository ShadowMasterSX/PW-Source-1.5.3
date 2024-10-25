
#include "gacdutil.h"
#include "mempatterncode.hpp"
#include "preparedcodelibrary.hpp"

namespace GNET
{

void MemPatternCode::PatchRes(int iRes)
{
	for(CodePieceVector::iterator it = m_cpv.begin(), ie = m_cpv.end(); it != ie; ++it)
	{
		if( (*it).GetID() == m_item.m_iID && !(*it).IsRunType() )
		{
			if( m_item.IsLocal() )
				(*it).PatchInt(142, iRes);
			else
				(*it).PatchInt(439, iRes);
			break;
		}
	}
}

void MemPatternCode::SetItem(const MemPatternItem &item)
{
	m_item = item;
	std::string buffer = m_item.m_pattern;
	for(size_t i=0; i<buffer.size(); ++i) buffer[i] = '0';
	std::vector<std::string> params;
	if( m_item.IsLocal() )
		PreparedCodeLibrary::GetInstance()->MakeCode("findlocalpattern", m_cpv, params);
	else
		PreparedCodeLibrary::GetInstance()->MakeCode("findmempattern", m_cpv, params);
	// patch
	for(CodePieceVector::iterator it = m_cpv.begin(), ie = m_cpv.end(); it != ie; ++it)
	{
		if( (*it).GetID() == BCI_FOREIGN_PATTERN_CODE )
		{
			(*it).ChangeID(m_item.m_iID);
			if( !(*it).IsRunType() )
			{
				(*it).PatchInt(205, m_item.m_iAddr);
				(*it).PatchInt(60, m_item.m_iSize);
			}
		}
		else if( (*it).GetID() == BCI_LOCAL_PATTERN_CODE )
		{
			(*it).ChangeID(m_item.m_iID);
			if( !(*it).IsRunType() )
			{
				(*it).PatchInt(36, item.m_iAddr);
				(*it).PatchInt(108, item.m_iAddr);
				(*it).PatchInt(25, item.m_iSize);
			}
		}
		else if( (*it).GetID() == BCI_FOREIGN_PATTERN || (*it).GetID() == BCI_LOCAL_PATTERN )
		{
			(*it) = CodePiece((*it).GetID(), m_item.m_pattern, true);
		}
		DEBUG_PRINT_INIT("[%d][%d]\t", (*it).GetID(), (*it).GetSize());
	}
	DEBUG_PRINT_INIT("\nmake mempattern pro size %d\n", m_cpv.size());
}


};

