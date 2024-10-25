 /*
 * FILE: NPCChaseInWaterAgent.h
 *
 * DESCRIPTION:   A  class derived from the class CNPCChaseAgent 
 *							which realizes In-Water NPCs' chasing movement.
 *							For now, we don't realize the avoidance of any obstructs!
 *
 * CREATED BY: He wenfeng, 2005/5/4
 *
 * HISTORY: 
 *
 * Copyright (c) 2004 Archosaur Studio, All Rights Reserved.
 */
#ifndef _NPCCHASEINWATERAGENT_H_
#define _NPCCHASEINWATERAGENT_H_

#include "NPCMove.h"
#include "GlobalWaterAreaMap.h"

class CNPCChaseInWaterStraightAgent: public CNPCChaseAgent
{
public:
	CNPCChaseInWaterStraightAgent(CMap * pMap);
	virtual ~CNPCChaseInWaterStraightAgent();

protected:
	virtual void AdjustCurPos()
	{
		// current position must not be under the water and above the terrain!
		//float TerrainHeight = NPCMove::GetTerrainHeight(m_vCurPos.x, m_vCurPos.z);
		//float WaterHeight = WaterAreaMap::GetWaterHeight(m_vCurPos);
		float TerrainHeight = m_pMap->GetTerrainHeight(m_vCurPos.x, m_vCurPos.z);
		float WaterHeight = m_pMap->GetWaterHeight(m_vCurPos);
		TerrainHeight += ABOVE_DIST;
		WaterHeight -= BELOW_DIST;
		if(TerrainHeight > WaterHeight)
		{
			// this is a exceptional case for the terrain at this position 
			// is higher than the water height at it! In this case, we only
			// guarantee we must be above the terrain!
			m_vCurPos.y = TerrainHeight;	
		}
		else
		{
			if(m_vCurPos.y< TerrainHeight) 
				m_vCurPos.y = TerrainHeight;
			else if(m_vCurPos.y > WaterHeight) 
				m_vCurPos.y = WaterHeight;
		}
	}

	virtual bool IsPosBeyondEnv(const A3DVECTOR3& vPos)
	{

/*
		// old version 1.0
		float TerrainHeight = NPCMove::GetTerrainHeight(vPos.x, vPos.z);
		float WaterHeight = WaterAreaMap::GetWaterHeight(vPos);
		TerrainHeight += ABOVE_DIST;
		WaterHeight -= BELOW_DIST;
		return (TerrainHeight > WaterHeight || vPos.y > WaterHeight );

		// old version 1.1
		float TerrainHeight = NPCMove::GetTerrainHeight(vPos.x, vPos.z);
		float WaterHeight = WaterAreaMap::GetWaterHeight(vPos);
		TerrainHeight += RELAX_ABOVE_DIST;
		WaterHeight -= RELAX_ABOVE_DIST;
		return (vPos.y < TerrainHeight || vPos.y > WaterHeight );
*/

		return !CNPCMoveAgent::IsPosInWater(vPos, m_pMap);
	}

//	virtual void AdjustGetToGoalPos()
//	{
//		// do nothing now!
//	}
};

#endif

