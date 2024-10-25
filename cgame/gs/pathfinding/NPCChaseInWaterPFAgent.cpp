/*
 * FILE: NPCChaseInWaterPFAgent.cpp
 *
 * DESCRIPTION:   A  class derived from the class CNPCChaseInWaterAgent 
 *							which realizes In-Wate NPCs' chasing movement considering 
 *							avoiding obstructs! 
 *
 * CREATED BY: He wenfeng, 2005/10/21
 *
 * HISTORY: 
 *
 * Copyright (c) 2005 Archosaur Studio, All Rights Reserved.
 */

#include "NPCChaseInWaterPFAgent.h"
#include "NPCChaseInWaterAgent.h"
#include "SpatialPathFinding.h"

CNPCChaseAgent* CNPCChaseInWaterPFAgent::CreateChaseStraightAgent()
{
	CNPCChaseAgent * pChaseStraightAgent = new CNPCChaseInWaterStraightAgent(m_pMap);
	return pChaseStraightAgent;
}

CSpatialPathFinding* CNPCChaseInWaterPFAgent::CreateSpatialPathFinding()
{
	CSpatialPathFinding* pSPF = new CUnderwaterPathFinding(m_pMap);
	return pSPF;
}

