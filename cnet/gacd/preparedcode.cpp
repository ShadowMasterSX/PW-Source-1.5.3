
#include "gacdutil.h"
#include "preparedcode.hpp"
#include "codepiecelibrary.hpp"
#include "stringhelper.hpp"

namespace GNET
{

void PreparedCode::Parse(const std::string &str)
{
	std::vector<PreparedCodeFragment> pcfs;
	StringHelper::string_pair_vec p = StringHelper::parse_string_pair(str);
	for(StringHelper::string_pair_vec::const_iterator it = p.begin(), ie = p.end(); it != ie; ++it)
	{
		DEBUG_PRINT_INIT("[%s],[%s]\t", (*it).first.c_str(), (*it).second.c_str());
		pcfs.push_back(PreparedCodeFragment(atoi((*it).first.c_str()), (*it).second));
	}
	DEBUG_PRINT_INIT("\n");
	PreBuild(pcfs);
}

void PreparedCode::PreBuild(const std::vector<PreparedCodeFragment> &pcfs)
{
	Clear();
	bool bWaiting = false;
	for(std::vector<PreparedCodeFragment>::const_iterator it = pcfs.begin(), ie = pcfs.end(); it != ie ; ++it)
	{
		if( !(*it).IsValid() ) continue;
		if( (*it).GetType() == PreparedCodeFragment::FT_FIXED
			|| (*it).GetType() == PreparedCodeFragment::FT_ALL )
		{
			CodePieceLibrary::GetInstance()->ParseCode((*it), m_finalCode);
		}
		else
		{
			bWaiting = true;
			if( m_finalCode.size() > 0 )
			{
				Node node;
				node.m_sType = Node::NT_COMPLETE;
				node.m_cpv = m_finalCode;
				m_parsingList.push_back(node);	
				m_finalCode.clear();
			}
			if( (*it).GetType() == PreparedCodeFragment::FT_RANDOM )
			{
				Node node;
				node.m_sType = Node::NT_RANDOM;
				node.m_pcf = (*it);
				m_parsingList.push_back(node);	
			}
			else if( (*it).GetType() == PreparedCodeFragment::FT_PARAM )
			{
				Node node;
				node.m_sType = Node::NT_PARAM;
				node.m_sCodeID = (*it).GetCodeID();
				m_parsingList.push_back(node);	
			}
		}
	}
	if( bWaiting ) 
	{
		if( m_finalCode.size() > 0 )
		{
			Node node;
			node.m_sType = Node::NT_COMPLETE;
			node.m_cpv = m_finalCode;
			m_parsingList.push_back(node);	
			m_finalCode.clear();
		}
		m_finalCode.clear();
		DEBUG_PRINT_INIT("parsing list size %d\n", m_parsingList.size());
	}
	else DEBUG_PRINT_INIT("got final code\n");
}
void PreparedCode::MakeCode(CodePieceVector &cpv, std::vector<std::string> params) const
{
	//_refs = refs;
	if( m_finalCode.size() > 0 )
	{
		cpv = m_finalCode;
		return;
	}
	cpv.clear();
	size_t index = 0, n = params.size();
	for(ParsingList::const_iterator it = m_parsingList.begin(), ie = m_parsingList.end(); it != ie; ++it)
	{
		const Node &node = (*it);
		if( node.m_sType == Node::NT_COMPLETE )
		{
			std::copy( node.m_cpv.begin(), node.m_cpv.end(), std::back_inserter(cpv));
		}
		else if( node.m_sType == Node::NT_RANDOM )
		{
			CodePieceLibrary::GetInstance()->ParseCode(node.m_pcf, cpv);
		}
		else if( node.m_sType == Node::NT_PARAM )
		{
			if( index < n)
			{
				cpv.push_back(CodePiece(node.m_sCodeID, params[index++]));
			}
		}
	}
}

};

