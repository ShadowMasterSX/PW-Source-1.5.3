 /*
 * FILE: NPCChaseOnAirAgent.h
 *
 * DESCRIPTION:   A  class derived from the class CNPCChaseAgent 
 *							which realizes On-Air NPCs' chasing movement.
 *							For now, we don't realize the avoidance of any obstructs!
 *
 * CREATED BY: He wenfeng, 2005/5/12
 *
 * HISTORY: 
 *
 * Copyright (c) 2004 Archosaur Studio, All Rights Reserved.
 */
#ifndef _NPCCHASEONAIRAGENT_H_
#define _NPCCHASEONAIRAGENT_H_

#include "NPCMove.h"
#include "GlobalWaterAreaMap.h"

class CNPCChaseOnAirStraightAgent: public CNPCChaseAgent
{
public:
	CNPCChaseOnAirStraightAgent(CMap * pMap);
	virtual ~CNPCChaseOnAirStraightAgent();

protected:
	virtual void AdjustCurPos()
	{
		// current position must not be under the water and terrain!
		//float TerrainHeight = NPCMove::GetTerrainHeight(m_vCurPos.x, m_vCurPos.z);
		//float WaterHeight = WaterAreaMap::GetWaterHeight(m_vCurPos);
		float TerrainHeight = m_pMap->GetTerrainHeight(m_vCurPos.x, m_vCurPos.z);
		float WaterHeight = m_pMap->GetWaterHeight(m_vCurPos);
		float MaxHeight = (TerrainHeight > WaterHeight)? TerrainHeight: WaterHeight;
		MaxHeight += ABOVE_DIST;
		if(m_vCurPos.y < MaxHeight) m_vCurPos.y = MaxHeight;
	}

//	virtual void AdjustGetToGoalPos()
//	{
//		// do nothing now!
//	}
	
	virtual bool IsPosBeyondEnv(const A3DVECTOR3& vPos)
	{
/*	
		// old version 1.0
		//*******************************************
		// Noted by wenfeng, 05-11-11
		// How about the case that vPos is under the terrain? Why I ignore this case ???

		// if vPos is under water surface, we regard the case as Pos beyond environment
		float WaterHeight = WaterAreaMap::GetWaterHeight(vPos);
		return (vPos.y < WaterHeight );

		// old version 1.1
		float TerrainHeight = NPCMove::GetTerrainHeight(vPos.x, vPos.z);
		float WaterHeight = WaterAreaMap::GetWaterHeight(vPos);
		TerrainHeight += RELAX_ABOVE_DIST;
		WaterHeight += RELAX_ABOVE_DIST;
		return (vPos.y < TerrainHeight || vPos.y < WaterHeight );
*/
		return !CNPCMoveAgent::IsPosOnAir(vPos, m_pMap);
	}
	
};

#endif

