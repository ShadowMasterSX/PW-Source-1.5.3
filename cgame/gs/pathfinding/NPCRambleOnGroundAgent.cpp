/*
 * FILE: NPCRambleOnGroundAgent.cpp
 *
 * DESCRIPTION:   A  class derived from the class CNPCChaseAgent 
 *							which realizes on-ground NPCs' ramble movement.
 *
 * CREATED BY: He wenfeng, 2005/5/13
 *
 * HISTORY: 
 *
 * Copyright (c) 2004 Archosaur Studio, All Rights Reserved.
 */

#include "NPCRambleOnGroundAgent.h"
#include "NPCChaseOnGroundAgent.h"

// #define MAX_GENERATE_GOAL_TIMES 10
#define PATHFINDING_SEARCH_PIXELS 200

void CNPCRambleOnGroundAgent::StartRamble()
{
	CNPCRambleAgent::StartRamble();
	// here, we set a large search pixels so as to find the path quickly.
	((CNPCChaseOnGroundAgent *)m_pNPCChaseAgent)->SetPFPixels(PATHFINDING_SEARCH_PIXELS);
}

void CNPCRambleOnGroundAgent::GenerateTmpGoal()
{
	POS2D p2Cur, p2Goal;
	//p2Cur = g_NPCMoveMap.GetMapPos(m_vCurPos);
	p2Cur = m_pMap->GetGroundMapPos(m_vCurPos);
	int iGenCounter = 0;
	bool bFoundFitGoal;

	do 
	{
		m_vTmpGoal = GeneratePosInMoveRange();
		//p2Goal = g_NPCMoveMap.GetMapPos(m_vTmpGoal);
		p2Goal = m_pMap->GetGroundMapPos(m_vTmpGoal);
		//bFoundFitGoal = g_NPCMoveMap.IsPosReachable(p2Goal) 
		bFoundFitGoal = m_pMap->IsGroundPosReachable(p2Goal) 
								&& ((CNPCChaseOnGroundAgent *)m_pNPCChaseAgent)->CanGoStraightForward(p2Cur, p2Goal);
		iGenCounter ++;
	} while( iGenCounter <= MAX_GENERATE_GOAL_TIMES && !bFoundFitGoal );

	if(bFoundFitGoal)
		return;
	else
	{
		iGenCounter = 0;
		bool bFoundReachableGoal;
		do
		{
			m_vTmpGoal = GeneratePosInMoveRange();
			//p2Goal = g_NPCMoveMap.GetMapPos(m_vTmpGoal);
			p2Goal = m_pMap->GetGroundMapPos(m_vTmpGoal);
			//bFoundReachableGoal = g_NPCMoveMap.IsPosReachable(p2Goal);
			bFoundReachableGoal = m_pMap->IsGroundPosReachable(p2Goal);
			iGenCounter ++;	
		} while(iGenCounter <= MAX_GENERATE_GOAL_TIMES && !bFoundReachableGoal);

		if(bFoundReachableGoal)
			return;
		else
		{
			// Even find no reachable pos! So we just return the cur pos as the tmp goal.
			// When program go here, it often means that the environment of this npc is not appropriate!
			m_vTmpGoal = m_vCurPos;
			return;
		}
	}
}

A3DVECTOR3 CNPCRambleOnGroundAgent::GeneratePosInMoveRange()
{
	/*
	// Circle range: now it's discarded.

	A3DVECTOR3 vOffset(0.0f);
	float r = RAND(m_fMoveRangeRadius);
	float theta = RAND( 2*PI );				// (0, 2*PI)
	vOffset.x = r * cos(theta);
	vOffset.z = r * sin(theta);
	
	// Note: we don't fill the right Y coordinate of the pos here
	return ( vOffset + m_vMoveRangeCenter );
	*/

	// now we use the AABB as the range of ramble
	A3DVECTOR3 vOffset(0.0f);
	float dx = RAND(2*m_fMoveRangeRadius);
	float dz = RAND(2*m_fMoveRangeRadius);
	vOffset.x = - m_fMoveRangeRadius + dx;
	vOffset.z = - m_fMoveRangeRadius + dz;
	
	// Note: we don't fill the right Y coordinate of the pos here
	return ( vOffset + m_vMoveRangeCenter );

}
