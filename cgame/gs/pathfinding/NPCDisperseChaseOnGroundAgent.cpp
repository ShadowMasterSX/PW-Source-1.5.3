 /*
 * FILE: NPCDisperseChaseOnGroundAgent.cpp
 *
 * DESCRIPTION:   A  class derived from the class CNPCChaseOnGroundAgent 
 *							which adds some random factors to on-ground NPCs' chasing movement.
 *							For now, we implement it by disturbing the goal position.
 *
 * CREATED BY: He wenfeng, 2005/5/19
 *
 * HISTORY: 
 *
 * Copyright (c) 2004 Archosaur Studio, All Rights Reserved.
 */

#include "NPCDisperseChaseOnGroundAgent.h"
#include "HalfSpace.h"
using namespace CHBasedCD;

// #define MAX_GENERATE_GOAL_TIMES 11

// #define ZERO_DIST_ERROR 0.01f

void CNPCDisperseChaseOnGroundAgent::SetGoal(const A3DVECTOR3& vGoal, float fMinDist, CChaseInfo* pChaseInfo)
{
	// Firstly, we test if now Agent has arrived?
	CNPCChaseAgent::SetGoal(vGoal, fMinDist);
	m_bGetToGoal = CurPosGetToGoal();
	if(m_bGetToGoal) return;

	// Else, we find a dispersed goal and reset goal again!
	
	//***********************************************
	// Revised by wenfeng, 05-11-7
	// When pChaseInfo is not NULL, it'll be refered to generate the goal.
	A3DVECTOR3 vDispersedGoal;
	if( pChaseInfo )
	{
		if(!pChaseInfo->HaveDispersed())
		{
			// first chase, so we generate the dispersed goal
			GetDispersedGoal(vGoal, fMinDist, vDispersedGoal);
			A3DVECTOR3 vDispersedDir = vDispersedGoal- vGoal;
			vDispersedDir.Normalize();
			pChaseInfo->DispersedDir() = vDispersedDir;
			pChaseInfo->HaveDispersed() = true;
		}
		else
		{
			// we have dispersed the real goal, so we directly set the goal as the vGoal add the offset.
			vDispersedGoal = vGoal + fMinDist * pChaseInfo->DispersedDir();
			A3DVECTOR3 vGoalDir = vGoal - m_vCurPos;
			// test whether vDispersedGoal is on the far half side of the goal circle.
			if(DotProduct(vGoalDir, pChaseInfo->DispersedDir()) > 0 )
			{
				vGoalDir.Normalize();
				CHalfSpace hs;
				hs.SetNV(vGoalDir, vGoal);
				hs.Mirror(vDispersedGoal);
			}
			

			//POS2D posDispersedGoal = g_NPCMoveMap.GetMapPos(vDispersedGoal);
			POS2D posDispersedGoal = m_pMap->GetGroundMapPos(vDispersedGoal);
			//if(!g_NPCMoveMap.IsPosReachable(posDispersedGoal))
			if(!m_pMap->IsGroundPosReachable(posDispersedGoal))
			{
				// this position isn't passable, so we should generate the dispersed goal again
				GetDispersedGoal(vGoal, fMinDist, vDispersedGoal);
			}
		}
	}
	else
	{
		// pChaseInfo is NULL, we generate the dispersed goal each time I SetGoal
		GetDispersedGoal(vGoal, fMinDist, vDispersedGoal);
	}
	
	//*****************Revised by wenfeng, 05-12-8
	// when the dispersed goal isn't straight forward from the current pos,
	// we'd better still chase the original goal! ( or not?)
	//POS2D posCur = g_NPCMoveMap.GetMapPos(m_vCurPos);
	POS2D posCur = m_pMap->GetGroundMapPos(m_vCurPos);
	//POS2D posGoal = g_NPCMoveMap.GetMapPos(vDispersedGoal);
	POS2D posGoal = m_pMap->GetGroundMapPos(vDispersedGoal);
	//if(g_NPCMoveMap.CanGoStraightForward(posCur,posGoal))
	if(m_pMap->CanGroundGoStraightForward(posCur,posGoal))
	{
		CNPCChaseAgent::SetGoal(vDispersedGoal, 0.0f);
	}
	// otherwise, we still use the original goal
	if(!m_pMap->IsGroundPosReachable(posGoal))
	{
		m_bBlockedBeyondEnv = true;
	}
}

void CNPCDisperseChaseOnGroundAgent::GetDispersedGoal(const A3DVECTOR3& vGoal, float fMinDist, A3DVECTOR3& vDispersedGoal)
{
	
	if( fMinDist < ZERO_DIST_ERROR )
	{
		vDispersedGoal = vGoal;
		return;
	}

	// Current position's direction relative to vGoal
	A3DVECTOR3 vDir = m_vCurPos - vGoal;
	vDir.y = 0.0f;
	vDir.Normalize();
	float cosDir = vDir.x;
	float sinDir = vDir.z;	

	float fRandomRadian,cosRR,sinRR;
	int iGenCounter = 0;
	POS2D posGoal;
	do
	{
		iGenCounter++;
		
		// Generate a random disturbing angle ( in radian )		
		fRandomRadian = RAND( m_fDisperseRadianRange ) - m_fDisperseRadianRange*0.5;
		cosRR = cos(fRandomRadian);
		sinRR = sin(fRandomRadian);

		// Add the disturbing angle to the direction, so we get a dispersed position
		vDispersedGoal.x = fMinDist * ( cosDir * cosRR - sinDir * sinRR );
		vDispersedGoal.z = fMinDist * ( sinDir * cosRR + cosDir * sinRR );
		vDispersedGoal.y = 0.0f;

		vDispersedGoal+= vGoal;
		
		//posGoal = g_NPCMoveMap.GetMapPos(vDispersedGoal);
		posGoal = m_pMap->GetGroundMapPos(vDispersedGoal);

	//} while( iGenCounter<=MAX_GENERATE_GOAL_TIMES && ! g_NPCMoveMap.IsPosReachable(posGoal));
	} while( iGenCounter<=MAX_GENERATE_GOAL_TIMES && !m_pMap->IsGroundPosReachable(posGoal));

}
