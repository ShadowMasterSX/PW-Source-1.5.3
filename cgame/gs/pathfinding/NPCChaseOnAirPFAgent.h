/*
 * FILE: NPCChaseOnAirPFAgent.h
 *
 * DESCRIPTION:   A  class derived from the class CNPCChaseOnAirAgent 
 *							which realizes On-Air NPCs' chasing movement considering 
 *							avoiding obstructs! 
 *
 * CREATED BY: He wenfeng, 2005/10/21
 *
 * HISTORY: 
 *
 * Copyright (c) 2005 Archosaur Studio, All Rights Reserved.
 */

#ifndef _NPCCHASEONAIRPFAGENT_H_
#define _NPCCHASEONAIRPFAGENT_H_

#include "NPCChaseSpatiallyPFAgent.h"

class CNPCChaseOnAirPFAgent: public CNPCChaseSpatiallyPFAgent
{
public:

	CNPCChaseOnAirPFAgent(CMap * pMap):CNPCChaseSpatiallyPFAgent(pMap)
	{
		m_ptrFuncExtraPosPassableTest = CNPCChaseSpatiallyPFAgent::IsPosOnAir;
	}
	

protected:
	virtual CNPCChaseAgent* CreateChaseStraightAgent();
	virtual CSpatialPathFinding* CreateSpatialPathFinding();

};

#endif
