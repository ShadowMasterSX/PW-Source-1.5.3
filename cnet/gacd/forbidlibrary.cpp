#include <algorithm>

#include "gacdutil.h"
#include "forbidlibrary.hpp"
#include "stringhelper.hpp"
#include "codepiece.hpp"

namespace GNET
{

void CodeResCheckerFromForbid::DoCheck(WaitingCode *pWC, UserSessionData *pUser, int iRes, bool &bCommit)
{
    bCommit = ForbidLibrary::GetInstance()->CheckRes(pWC->m_iCodeID, iRes);
}

ForbidLibrary ForbidLibrary::s_instance;

void ForbidLibrary::UpdateSendingQueue(SendingQueue &queue) const
{
	std::copy(m_rawQueue.begin(), m_rawQueue.end(), std::back_inserter(queue));
}

bool ForbidLibrary::CheckRes(int iID, int iRes) const
{
	Library::const_iterator it = m_library.find(iID);
	if( it != m_library.end() )
		return (*it).second.Check(iRes);
	return false;
}

bool ForbidLibrary::GetCodePieceVector(int iUserID, int iID, CodePieceVector &cpv, int iSeq, CodeResChecker *&pChecker)
{
	Library::const_iterator it = m_library.find(iID);
	if( it == m_library.end() )
		return false;
	cpv = (*it).second.GetCodePieceVector();
	for(CodePieceVector::iterator it = cpv.begin(), ie = cpv.end(); it != ie; ++it)
    {
        if( (*it).GetID() == iID )
            (*it).ChangeID(iSeq);
    }
	pChecker = new CodeResCheckerFromForbid();
	return true;
}

void ForbidLibrary::OnUpdateConfig(const XmlConfig::Node *pRoot)
{
	Clear();
	const XmlConfig::Node *manager = pRoot->GetFirstChild("statmanager");
    XmlConfig::Nodes nodes = manager->GetFirstChild("forbid")->GetChildren("item");

    for(XmlConfig::Nodes::const_iterator it = nodes.begin(), ie = nodes.end(); it != ie ; ++it)
    {
		ForbidItem item;
		(*it)->GetIntAttr("id", &item.m_iID);
		(*it)->GetIntAttr("type", &item.m_iType);
		if( item.IsEmpty() ) continue;
		(*it)->GetStrAttr("dll_name", item.m_dllName);
		(*it)->GetStrAttr("exe_name", item.m_exeName);
		if( item.m_iType == ForbidItem::FT_PROCESS && item.m_exeName.empty() ) continue;
		if( item.m_iType == ForbidItem::FT_MODULE  && item.m_dllName.empty() ) continue;
		if( item.m_iType == ForbidItem::FT_WINDOW  && item.m_dllName.empty() ) continue;

		item.m_iID += ForbidCode::RESERVED_CODE;
		(*it)->GetIntAttr("dll_size", &item.m_iDllSize);

		m_library[item.m_iID] = ForbidCode(item);
		bool bBindToDebugCode = false;
		(*it)->GetBoolAttr("bind", &bBindToDebugCode);
		if( !bBindToDebugCode )
			m_rawQueue.push_back(SendingCode(item.m_iID, this) );
		DEBUG_PRINT_INIT("load forbid %d\n", item.m_iID);
    }
        
}

};

