/*
 * FILE: NPCChaseSpatiallyPFAgent.h
 *
 * DESCRIPTION:   A  class derived from the class CNPCChaseAgent 
 *						which is a base class for spatial NPCs(including On-air and in-water NPC)'
 *						 chasing movement considering avoiding obstructs! 
 *
 * CREATED BY: He wenfeng, 2005/11/3
 *
 * HISTORY: 
 *
 * Copyright (c) 2005 Archosaur Studio, All Rights Reserved.
 */

#ifndef _NPCCHASESPATIALLYPFAGENT_H_
#define _NPCCHASESPATIALLYPFAGENT_H_

#include "NPCMove.h"
#include "GlobalWaterAreaMap.h"
#include "GlobalSPMap.h"

class CSpatialPathFinding;
class CPathFollowing;

class CNPCChaseSpatiallyPFAgent:public CNPCChaseAgent
{
public:

	CNPCChaseSpatiallyPFAgent(CMap * pMap):CNPCChaseAgent(pMap)
	{
		m_pChaseStraightAgent = NULL;
		m_pSpatialPathFinding = NULL;
		m_pCurPath = NULL;
	}
	
	virtual ~CNPCChaseSpatiallyPFAgent()
	{
		Release();
	}

	virtual void Release();

	virtual bool StartPathFinding(int SearchPixels = 10);
	virtual void MoveOneStep();

protected:
	bool GeneratePathFindingGoal(A3DVECTOR3& vPFGoal);
		
	virtual CNPCChaseAgent* CreateChaseStraightAgent() = 0;
	virtual CSpatialPathFinding* CreateSpatialPathFinding() = 0;

protected:

	bool m_bGoStraightForward;
	A3DVECTOR3 m_vPFGoal;				// the goal of pathfinding

	CNPCChaseAgent * m_pChaseStraightAgent;
	CSpatialPathFinding * m_pSpatialPathFinding;
	CPathFollowing * m_pCurPath;

	CGlobalSPMap::PtrFuncExtraPosPassableTest m_ptrFuncExtraPosPassableTest;
};



#endif
