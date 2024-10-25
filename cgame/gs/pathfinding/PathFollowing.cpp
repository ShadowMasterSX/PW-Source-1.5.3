 /*
 * FILE: PathFollowing.cpp
 *
 * DESCRIPTION:		a class which can generate the position after one step 
 *					when following a path which consists of set of line segments.
 *
 * CREATED BY:		He wenfeng, 2005/10/20
 *
 * HISTORY: 
 *
 * Copyright (c) 2005 Archosaur Studio, All Rights Reserved.
 */

#include "PathFollowing.h"

void CPathFollowing::MoveOneStep(float fStepDist)
{
	if(m_iCurLineSeg >= (int)m_Path.size()) return;	// get to the end

	float fDist2NextNode = m_Path[m_iCurLineSeg].fLength - m_fDist;
	
	while(fStepDist > fDist2NextNode)
	{
		fStepDist -= fDist2NextNode;
		m_vCurPos += fDist2NextNode * m_Path[m_iCurLineSeg].vDir;
		m_fDist = 0;

		++m_iCurLineSeg;
		if((size_t)m_iCurLineSeg < m_Path.size())
			fDist2NextNode =m_Path[m_iCurLineSeg].fLength;
		else
			return;
	}
	

	m_fDist += fStepDist;
	m_vCurPos += fStepDist * m_Path[m_iCurLineSeg].vDir;

}
