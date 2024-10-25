/*
 * FILE: BrushMan.h
 *
 * DESCRIPTION: brush manager
 *
 * CREATED BY: wang kuiwu, 2008/4/21
 *
 * HISTORY: 
 *
 * Copyright (c) 2008 Archosaur Studio, All Rights Reserved.
 */
#include <ASSERT.h>
#include <a3dvector.h>

 
#include "BrushMan.h"
#include "NmdTree.h"
#include "SMTree.h"
#include "BrushTree.h"
#include "CDBrush.h"
 
namespace SvrCD
{
//const var

//local func



//////////////////////////////////////////////////////////////////////////
//class CBrushMan


CBrushMan::CBrushMan()
	:m_pSmTree(NULL),
	 m_pNmdTree(NULL)
{
	
}

CBrushMan::~CBrushMan()
{
	
}

void CBrushMan::Release()
{
	m_pSmTree = NULL;
	if (m_pNmdTree)
	{
		m_pNmdTree->Release();
		delete m_pNmdTree;
		m_pNmdTree = NULL;
	}
}

// void CBrushMan::SetNmdTree(CNmdTree * pNmdTree)
// {
// 	m_pNmdTree = pNmdTree;
// }

void CBrushMan::SetSmTree(CSMTree * pSmTree, bool enable_nmdtree)
{
	ASSERT(pSmTree);
	m_pSmTree = pSmTree;

	if(!enable_nmdtree) return;
	
	if (m_pNmdTree)
	{
		m_pNmdTree->Release();
		delete m_pNmdTree;
		m_pNmdTree = NULL;
	}
	
	if (pSmTree)
	{
		m_pNmdTree = new CNmdTree;
		m_pNmdTree->Init(pSmTree->GetBrushTree()->GetRootNode()->aabb.Extents);
	}
}


bool CBrushMan::AddDynObj(CNmdChd *pNmdChd, int tplId, int uId, const A3DVECTOR3& pos, const A3DVECTOR3& up, const A3DVECTOR3& dir)
{
	if (!m_pNmdTree)
	{
		return false;
	}

	return m_pNmdTree->AddDynObj(pNmdChd, tplId, uId, pos, up, dir);
}


bool CBrushMan::AddNpcMine(CNmdChd *pNmdChd, int tplId, int uId, const A3DVECTOR3& pos, const A3DVECTOR3& up, const A3DVECTOR3& dir)
{
	if (!m_pNmdTree)
	{
		return false;
	}

	return m_pNmdTree->AddNpcMine(pNmdChd, tplId, uId, pos, up, dir);
}

void CBrushMan::EnableNmd(int uId, bool enable) const
{
	if (!m_pNmdTree)
	{
		return;
	}

	m_pNmdTree->EnableNmd(uId, enable);
}

bool CBrushMan::IsNmdEnabled(int uId) const
{
	if (!m_pNmdTree)
	{
		return false;
	}

	return m_pNmdTree->IsNmdEnabled(uId);
}
bool CBrushMan::Trace(BrushTraceInfo * trcInfo,const abase::vector<char>* element_flags)
{
	bool bCollide = false;
	bool		bStartSolid ;	
	bool		bAllSolid ;		
	int			iClipPlane ;	
	A3DVECTOR3 vNormal; 
	float       fDist ;	
	float fFraction = 100.0f ;

	if (m_pNmdTree && m_pNmdTree->GetBrushTree())
	{
		if (m_pNmdTree->GetBrushTree()->Trace(trcInfo,element_flags) && trcInfo->fFraction < fFraction)
		{
			//update the saving info
			bStartSolid = trcInfo->bStartSolid;
			bAllSolid = trcInfo->bAllSolid;
			iClipPlane = trcInfo->iClipPlane;
			fFraction = trcInfo->fFraction;
			vNormal = trcInfo->ClipPlane.GetNormal();
			fDist = trcInfo->ClipPlane.GetDist();
			bCollide = true;
		}
	}

	if (m_pSmTree && m_pSmTree->GetBrushTree())
	{
		if (m_pSmTree->GetBrushTree()->Trace(trcInfo,element_flags) && trcInfo->fFraction < fFraction)
		{
			//update the saving info
			bStartSolid = trcInfo->bStartSolid;
			bAllSolid = trcInfo->bAllSolid;
			iClipPlane = trcInfo->iClipPlane;
			fFraction = trcInfo->fFraction;
			vNormal = trcInfo->ClipPlane.GetNormal();
			fDist = trcInfo->ClipPlane.GetDist();
			bCollide = true;
		}
	}


	if (bCollide)
	{
		//set back
		trcInfo->bStartSolid = bStartSolid;
		trcInfo->bAllSolid = bAllSolid;
		trcInfo->iClipPlane = iClipPlane;
		trcInfo->fFraction = fFraction;
		trcInfo->ClipPlane.SetNormal(vNormal);
		trcInfo->ClipPlane.SetD(fDist);
	}


	return bCollide;
}




}
