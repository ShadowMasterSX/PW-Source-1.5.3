#include <algorithm>

#include "mempatternlibrary.hpp"
#include "stringhelper.hpp"
#include "codepiece.hpp"
#include "gacdutil.h"

namespace GNET
{

MemPatternLibrary MemPatternLibrary::s_instance;

void MemPatternLibrary::UpdateSendingQueue(SendingQueue &queue) const
{
	std::copy(m_rawQueue.begin(), m_rawQueue.end(), std::back_inserter(queue));
}

bool MemPatternLibrary::GetCodePieceVector(int iUserID, int iID, CodePieceVector &cpv, int iSeq, 
		CodeResChecker *&pChecker)
{
	//TODO
	Library::const_iterator it = m_library.find(iID);
	if( it == m_library.end() )
		return false;
	MemPatternCode tcode = (*it).second;
	int iRes = (0x73fe7451*(USERID2ACCOUNTID(iUserID)))^(0x8888f487*iID)|1;
	tcode.PatchRes(iRes);
	cpv = tcode.GetCodePieceVector();
	for(CodePieceVector::iterator it = cpv.begin(), ie = cpv.end(); it != ie; ++it)
    {
        if( (*it).GetID() == iID )
            (*it).ChangeID(iSeq);
    }
	// TODO patch rand res
	pChecker = new CodeResCheckerWithAnswer(iRes);
	return true;
}

void MemPatternLibrary::OnUpdateConfig(const XmlConfig::Node *pRoot)
{
	Clear();
	const XmlConfig::Node *manager = pRoot->GetFirstChild("statmanager");
    XmlConfig::Nodes nodes = manager->GetFirstChild("mempattern")->GetChildren("memitem");

    for(XmlConfig::Nodes::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
    {
		MemPatternItem item;
		(*it)->GetIntAttr("id", &item.m_iID);
		(*it)->GetIntAttr("size", &item.m_iSize);
		(*it)->GetIntAttr("addr", &item.m_iAddr);
		(*it)->GetStrAttr("name", item.m_name);
		(*it)->GetStrAttr("value", item.m_pattern);

		if( item.IsEmpty() ) continue;
		item.m_iID += MemPatternCode::RESERVED_CODE;

		std::string stype = (*it)->GetAttr("type");
		if( stype == "local" )
			item.m_iType = MemPatternItem::MPT_LOCAL;
		else
			item.m_iType = MemPatternItem::MPT_FOREIGN;
		
		m_library[item.m_iID] = MemPatternCode(item);
		m_rawQueue.push_back(SendingCode(item.m_iID, this));
		DEBUG_PRINT_INIT("load MemPattern %d\n", item.m_iID);
    }
        
}

};

