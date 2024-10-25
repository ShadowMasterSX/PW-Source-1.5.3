 /*
 * FILE: NPCFleeOnAirAgent.cpp
 *
 * DESCRIPTION:   A  class derived from the class CNPCFleeAgent 
 *							which realizes On-Air NPCs' flee movement.
 *							For now, we don't realize the avoidance of any obstructs!
 *
 * CREATED BY: He wenfeng, 2005/5/16
 *
 * HISTORY: 
 *
 * Copyright (c) 2004 Archosaur Studio, All Rights Reserved.
 */

#include "NPCFleeOnAirAgent.h"

#include "GlobalWaterAreaMap.h"

CNPCFleeOnAirAgent::CNPCFleeOnAirAgent(CMap * pMap):CNPCFleeAgent(pMap)
{

}

CNPCFleeOnAirAgent::~CNPCFleeOnAirAgent()
{

}

bool CNPCFleeOnAirAgent::IsPosBeyondEnv(const A3DVECTOR3& vPos)
{
	/*
	// old version
	float TerrainHeight = NPCMove::GetTerrainHeight(vPos.x, vPos.z);
	float WaterHeight = WaterAreaMap::GetWaterHeight(vPos);
	TerrainHeight += RELAX_ABOVE_DIST;
	WaterHeight += RELAX_ABOVE_DIST;
	return (vPos.y < TerrainHeight || vPos.y < WaterHeight );
	*/

	return !CNPCMoveAgent::IsPosOnAir(vPos, m_pMap);

}
