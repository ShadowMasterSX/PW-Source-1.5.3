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
 
#ifndef _BRUSH_MAN_H_
#define _BRUSH_MAN_H_

#include <a3dvector.h>
#include <vector.h>

namespace SvrCD
{
class CSMTree;
class CNmdTree;
class BrushTraceInfo;
class CNmdChd;
class CBrushMan
{
public:
	CBrushMan();
	~CBrushMan();
	//void SetNmdTree(CNmdTree * pNmdTree);
	void SetSmTree(CSMTree * pSmTree, bool enable_nmdtree = true);
	void Release();

	CSMTree * GetSmTree() const
	{
		return m_pSmTree;
	}
	CNmdTree * GetNmdTree() const
	{
		return m_pNmdTree;
	}
	bool Trace(BrushTraceInfo * pInfo,const abase::vector<char>* element_flags);

	bool AddNpcMine(CNmdChd *pNmdChd, int tplId, int uId, const A3DVECTOR3& pos, const A3DVECTOR3& up, const A3DVECTOR3& dir);
	bool AddDynObj(CNmdChd *pNmdChd, int tplId, int uId, const A3DVECTOR3& pos, const A3DVECTOR3& up, const A3DVECTOR3& dir);
	void EnableNmd(int uId, bool enable) const;
	bool IsNmdEnabled(int uId) const;

private:
	CSMTree * m_pSmTree;
	CNmdTree * m_pNmdTree;
};



}
#endif
