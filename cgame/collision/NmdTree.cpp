#include <ASSERT.h>
#include "A3DMatrix.h"
#include "A3DFuncs.h"

#include "NmdTree.h"
#include "NmdChd.h"
#include "BrushTree.h"
#include "CDBrush.h"
#include "ConvexHullData.h"


namespace SvrCD
{
//////////////////////////////////////////////////////////////////////////
//local 
void EnableNmd(const BrushVec * brushVec, bool enable)
{
 	int i;
	for (i = 0;i < (int)brushVec->size(); ++i)
	{
		CCDBrush * cdBrush = brushVec->at(i);
		DWORD flag = cdBrush->GetReservedDWORD();
		if (enable)
		{
			//flag &= ((-1)^CHDATA_SKIP_COLLISION);
			flag &= ((0xFFFFFFFF)^CHDATA_SKIP_COLLISION);
		}
		else
		{
			flag |= CHDATA_SKIP_COLLISION;
		}
		cdBrush->SetReservedDWORD(flag);
	}

}


//class CNmdTree
CNmdTree::CNmdTree()
//:m_pNmdChd(NULL), m_pBrushTree(NULL),
:m_pBrushTree(NULL),
m_BrushTab(128)
#ifdef BMAN_EDITOR
,m_EcmTab(128)
#endif

{

}

CNmdTree::~CNmdTree()
{

}

void CNmdTree::Release()
{
	//m_pNmdChd = NULL;
	if (m_pBrushTree)
	{
		m_pBrushTree->Release();
		delete m_pBrushTree;
		m_pBrushTree = NULL;
	}
	
	m_BrushTab.clear();
#ifdef BMAN_EDITOR
	ReleaseEcmTab(m_EcmTab);
#endif
}

bool CNmdTree::Init(const A3DVECTOR3& ext)
{

	//ASSERT(pNmdChd != NULL);
	//m_pNmdChd = pNmdChd;
	
	m_pBrushTree = new CBrushTree;
	m_pBrushTree->Build(ext);
	return true;
}


bool CNmdTree::_AddNMD(CNmdChd * pNmdChd, int tplId, int uId, const A3DVECTOR3& pos, const A3DVECTOR3& up, const A3DVECTOR3& dir, bool dyn)
{
	ASSERT(pNmdChd);

	if (m_BrushTab.find(uId) != m_BrushTab.end())
	{
		return false;
	}

	const ChdVec *  tpl_chd_vec;
	if (dyn)
	{
		tpl_chd_vec = pNmdChd->GetDynObj(tplId);
	}
	else
	{
		tpl_chd_vec = pNmdChd->GetNpcMine(tplId);
	}

	if (!tpl_chd_vec)
	{
		return false;
	}

	//transform
	A3DMATRIX4 mat = a3d_TransformMatrix(dir, up, pos);	
	ChdVec chd_vec;
	BrushVec brush_vec;
	
	int i;
	for (i = 0; i < (int)tpl_chd_vec->size(); ++i)
	{
		CConvexHullData * tpl_chd = tpl_chd_vec->at(i);
		CConvexHullData * chd = new CConvexHullData(*tpl_chd);
		chd->Transform(mat);
		chd_vec.push_back(chd);

		CQBrush qBrush;
		qBrush.AddBrushBevels(chd);
		CCDBrush * cdBrush = new CCDBrush;
		qBrush.Export(cdBrush);
		//add by liuguichen
		cdBrush->SetId(uId);
		brush_vec.push_back(cdBrush);
	}

	m_pBrushTree->AddBrushVec(brush_vec);
	m_BrushTab.put(uId, brush_vec);
	SvrCD::EnableNmd(&brush_vec, false);

	
#ifdef BMAN_EDITOR
	m_EcmTab.put(uId, chd_vec);
#else
	ReleaseChdVec(chd_vec);
#endif

	return true;
}


bool CNmdTree::AddNpcMine(CNmdChd *pNmdChd, int tplId, int uId, const A3DVECTOR3& pos, const A3DVECTOR3& up, const A3DVECTOR3& dir)
{
	return _AddNMD(pNmdChd, tplId, uId, pos, up, dir, false);
}

bool CNmdTree::AddDynObj(CNmdChd *pNmdChd, int tplId, int uId, const A3DVECTOR3& pos, const A3DVECTOR3& up, const A3DVECTOR3& dir)
{
	return _AddNMD(pNmdChd, tplId, uId, pos, up, dir, true); 
}


void CNmdTree::EnableNmd(int uId, bool enable) const
{
	BrushTable::const_iterator it = m_BrushTab.find(uId);
	if (it == m_BrushTab.end())
	{
		return;
	}
	const	BrushVec * brush_vec = it.value();
	
	SvrCD::EnableNmd(brush_vec, enable);

}


bool CNmdTree::IsNmdEnabled(int uId)const
{
	BrushTable::const_iterator it = m_BrushTab.find(uId);
	if (it == m_BrushTab.end())
	{
		return false;
	}

	const	BrushVec * brush_vec = it.value();

	DWORD flag = brush_vec->at(0)->GetReservedDWORD();

	return ((flag& CHDATA_SKIP_COLLISION) == 0);
	
}


}

