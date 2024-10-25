 /*
 * FILE: NPCChaseOnAirPFAgent.cpp
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


#include "NPCChaseOnAirPFAgent.h"
#include "NPCChaseOnAirAgent.h"
#include "SpatialPathFinding.h"

CNPCChaseAgent* CNPCChaseOnAirPFAgent::CreateChaseStraightAgent()
{
	CNPCChaseAgent * pChaseStraightAgent = new CNPCChaseOnAirStraightAgent(m_pMap);
	return pChaseStraightAgent;
}

CSpatialPathFinding* CNPCChaseOnAirPFAgent::CreateSpatialPathFinding()
{
	CSpatialPathFinding* pSPF = new CAerialPathFinding(m_pMap);
	return pSPF;
}
