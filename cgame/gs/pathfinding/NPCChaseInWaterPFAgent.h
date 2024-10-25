/*
 * FILE: NPCChaseInWaterPFAgent.h
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

#ifndef _NPCCHASEINWATERPFAGENT_H_
#define _NPCCHASEINWATERPFAGENT_H_

#include "NPCChaseSpatiallyPFAgent.h"

class CNPCChaseInWaterPFAgent: public CNPCChaseSpatiallyPFAgent
{
public:

	CNPCChaseInWaterPFAgent(CMap * pMap):CNPCChaseSpatiallyPFAgent(pMap)
	{
		m_ptrFuncExtraPosPassableTest = CNPCChaseSpatiallyPFAgent::IsPosInWater;
	}
	

protected:
	virtual CNPCChaseAgent* CreateChaseStraightAgent();
	virtual CSpatialPathFinding* CreateSpatialPathFinding();

};

#endif
