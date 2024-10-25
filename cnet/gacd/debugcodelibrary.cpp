#include <algorithm>

#include "gacdutil.h"
#include "debugcodelibrary.hpp"
#include "asmcodelibrary.hpp"
#include "stringhelper.hpp"
#include "codepiece.hpp"
#include "usersessiondata.hpp"

namespace GNET
{

DebugCodeLibrary DebugCodeLibrary::s_instance;

/*
void DebugCode::PatchRes(int _id, int _res)
{
    for(OctetsVector::iterator it = pro.content.begin(), ie = pro.content.end(); it != ie; ++it)
    {
        CodePiece piece((*it));
        if( piece.GetID() == _id && !piece.IsRunType() )
        {
            piece.PatchInt(item.GetPatchPos(), _res);
            (*it) = piece.GetData();
        }
    }
}
*/

void DebugCodeLibrary::UpdateSendingQueue(SendingQueue &queue) const
{
	std::copy(m_rawQueue.begin(), m_rawQueue.end(), std::back_inserter(queue));
}

bool DebugCodeLibrary::GetCodePieceVector(int iUserID, int iID, CodePieceVector &cpv, int iSeq, 
		CodeResChecker *&pChecker)
{
	Library::iterator it = m_library.find(iID);
	if( it == m_library.end() )
		return false;
	cpv = (*it).second.GetCodePieceVector();

	//if( (*it).second.GetItem().IsNeedPatch() || (*it).second.GetItem().IsNeedPatchRes() )
	//{
		int iRes = (*it).second.GetItem().m_iRes;
		DEBUG_PRINT_CODE("code %d patch res to %d\n", iID, iRes);
		//
    	for(CodePieceVector::iterator it2 = cpv.begin(), ie2 = cpv.end(); it2 != ie2; ++it2)
	    {
			if( (*it2).GetID() == BCI_SESSION )
			{
				DEBUG_PRINT_CODE("prpare sessionid\n");
				(*it2).MakeIntDataPiece(BCI_SESSION, MakeSessionID(iUserID)); 
			}
			else if( (*it2).GetID() == iID )
			{
				(*it2).ChangeID(iSeq);
        		if( !(*it2).IsRunType() )
	        	{
					if( (*it).second.GetItem().NeedPatchRes() )
					{
						iRes = (0x73fe7451*(USERID2ACCOUNTID(iUserID)))^(0x8878f487*iID)|1;
	    	        	(*it2).PatchInt((*it).second.GetItem().m_iResPos, iRes);
						DEBUG_PRINT_CODE("patch res %d\n", iID);
					}
					if( (*it).second.GetItem().NeedPatchSession() )
					{
						DEBUG_PRINT_CODE("patch session %d\n", iID);
	    	        	(*it2).PatchInt((*it).second.GetItem().m_iCheckSessionPos, MakeSessionID(iUserID));
					}
					if( (*it).second.GetItem().NeedPatchRet() )
					{
						DEBUG_PRINT_CODE("before patch code ret code %d, res %d\n", iID, iRes);
						Octets os;
						AsmCodeLibrary::GetInstance()->GetCode(os, iRes);
						(*it2).PatchData((*it).second.GetItem().m_iRetPos, os.begin(), os.size());
						DEBUG_PRINT_CODE("after patch code ret code %d, res %d\n", iID, iRes);
						/*
						AsmCode ac; //ac.SetFunc(func);
						//if( AsmCodeLibrary::GetInstance()->MakeSimpleCode(ac) )
						if( AsmCodeLibrary::GetInstance()->MakeRandomCode(ac) )
						{
							(*it2).PatchData((*it).second.GetItem().m_iRetPos, ac.begin(), ac.GetSize());
							iRes += ac.GetFunc();
						}
						*/
					}
	       		}
			}
    	}
		//
	DEBUG_PRINT_CODE("res with answer checker %d\n", iRes);
	pChecker = new CodeResCheckerWithAnswer(iRes);
	return true;
}

void DebugCodeLibrary::OnUpdateConfig(const XmlConfig::Node *pRoot)
{
	Clear();
	const XmlConfig::Node *manager = pRoot->GetFirstChild("codemanager");
    XmlConfig::Nodes nodes = manager->GetChildren("debugcodes");

    for(XmlConfig::Nodes::const_iterator it = nodes.begin(), ie = nodes.end(); it != ie ; ++it)
    {
		DebugItem item;
		(*it)->GetIntAttr("id", &item.m_iID);
		(*it)->GetIntAttr("codeid", &item.m_iCodeID);	
		(*it)->GetIntAttr("res", &item.m_iRes);
		(*it)->GetStrAttr("param", item.m_param);
		if( item.IsEmpty() ) continue;

		(*it)->GetIntAttr("patch_pos", &item.m_iResPos);		
		(*it)->GetIntAttr("res_pos", &item.m_iRetPos);		
		(*it)->GetIntAttr("session_pos", &item.m_iCheckSessionPos);		
		(*it)->GetStrAttr("precodename", item.m_precodeName);

		item.m_iID += DebugCode::RESERVED_CODE;

		m_library[item.m_iID] = DebugCode(item);
		m_rawQueue.push_back(SendingCode(item.m_iID,this));
		DEBUG_PRINT_INIT("load debug code %d, pr %d, pt %d, ps %d\n", item.m_iID
			,item.m_iResPos, item.m_iRetPos, item.m_iCheckSessionPos);
    }
        
}

};

