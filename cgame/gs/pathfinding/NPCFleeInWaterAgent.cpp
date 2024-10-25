 /*
 * FILE: NPCFleeInWaterAgent.cpp
 *
 * DESCRIPTION:   A  class derived from the class CNPCFleeAgent 
 *							which realizes In-Water NPCs' flee movement.
 *							For now, we don't realize the avoidance of any obstructs!
 *
 * CREATED BY: He wenfeng, 2005/5/16
 *
 * HISTORY: 
 *
 * Copyright (c) 2004 Archosaur Studio, All Rights Reserved.
 */

#include "NPCFleeInWaterAgent.h"

#include "GlobalWaterAreaMap.h"

CNPCFleeInWaterAgent::CNPCFleeInWaterAgent(CMap * pMap):CNPCFleeAgent(pMap)
{

}

CNPCFleeInWaterAgent::~CNPCFleeInWaterAgent()
{

}

bool CNPCFleeInWaterAgent::IsPosBeyondEnv(const A3DVECTOR3& vPos)
{
	/*
	// old version
	float TerrainHeight = NPCMove::GetTerrainHeight(vPos.x, vPos.z);
	float WaterHeight = WaterAreaMap::GetWaterHeight(vPos);
	TerrainHeight += RELAX_ABOVE_DIST;
	WaterHeight -= RELAX_ABOVE_DIST;
	return (vPos.y < TerrainHeight || vPos.y > WaterHeight );	
	*/
	return !CNPCMoveAgent::IsPosInWater(vPos, m_pMap);
}
