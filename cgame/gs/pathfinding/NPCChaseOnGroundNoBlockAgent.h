/*
 * FILE: NPCChaseOnGroundNoBlockAgent.h
 *
 * DESCRIPTION:   A  class derived from the class CNPCChaseAgent which 
 *			      realizes on-ground NPCs' chasing movement and this agent
 *				  differs from CNPCChaseOnGroundAgent for it won't be blocked.
 *
 * CREATED BY: He Wenfeng, 2005/12/20
 *
 * HISTORY: 
 *
 * Copyright (c) 2005--2008 Archosaur Studio, All Rights Reserved.
 */

#ifndef _NPCCHASEONGROUNDAGENT_NOBLOCK_H_
#define _NPCCHASEONGROUNDAGENT_NOBLOCK_H_

#include "NPCMove.h"
#include <vector.h>

class CPathFollowing;
class CPathFinding2D;

class CNPCChaseOnGroundNoBlockAgent : public CNPCChaseAgent
{

	typedef abase::vector<POS2D> VecPOS2D;

public:
	
	CNPCChaseOnGroundNoBlockAgent(CMap * pMap)
		:CNPCChaseAgent(pMap)
	{
		m_pFollowPath = NULL;
		m_pPathFinding = NULL;
		m_bGoStraightForward = false;
	}
	
	virtual ~CNPCChaseOnGroundNoBlockAgent()
	{
		Release();
	}

	virtual void Release();

	virtual void MoveOneStep();

	virtual bool StartPathFinding(int SearchPixels = 10);

protected:

	// override, so m_vMoveDir has 0.0f of Y coordinate!
	virtual void SetMoveDir(const A3DVECTOR3& vGoal)
	{
		m_vMoveDir = vGoal - m_vCurPos;
		m_vMoveDir.y = 0.0f;
		m_vMoveDir.Normalize();
	}

	// override, so m_vCurPos clings to the terrain or building surface
	virtual void AdjustCurPos()
	{
		//************************************
		// revised by wenfeng, 06-1-13
		// now we set the reference of DeltaHeight as the pixelcenter's terrain height
		
		//POS2D curPos = g_NPCMoveMap.GetMapPos(m_vCurPos);
		POS2D curPos = m_pMap->GetGroundMapPos(m_vCurPos);
		//float fDeltaHeight = g_NPCMoveMap.GetPosDeltaHeight(curPos);
		float fDeltaHeight = m_pMap->GetGroundPosDeltaHeight(curPos);
		if( fDeltaHeight == 0)
		{
			//m_vCurPos.y = NPCMove::GetTerrainHeight(m_vCurPos.x, m_vCurPos.z);
			m_vCurPos.y = m_pMap->GetTerrainHeight(m_vCurPos.x, m_vCurPos.z);
		}
		else
		{
			//A3DVECTOR3 vPixelCenter = g_NPCMoveMap.GetPixelCenter(curPos);
			A3DVECTOR3 vPixelCenter = m_pMap->GetGroundPixelCenter(curPos);
			//m_vCurPos.y = NPCMove::GetTerrainHeight(vPixelCenter.x, vPixelCenter.z) + fDeltaHeight;
			m_vCurPos.y = m_pMap->GetTerrainHeight(vPixelCenter.x, vPixelCenter.z) + fDeltaHeight;
		}
	}

	//**********************************
	// Note, by wenfeng, 05-12-26
	// this method is really not used in this class, so maybe there's sth. wrong in it

	// Override, so we only consider the distance on the XOZ plane	
	virtual bool CurPosGetToGoal()
	{

		A3DVECTOR3 vDelta = m_vGoal-m_vCurPos;
		vDelta.y  = 0.0f;

		//************************** Revised by wenfeng, 05-12-24
		// the following condition is wrong since maybe the agent move to the goal over!

		// here, we increase the expected distance to goal by RELAX_ERROR
		bool bCloseToGoal = vDelta.SquaredMagnitude() <= (m_fSqrMinDist2Goal + 2*RELAX_ERROR*m_fMinDist2Goal + SQR_RELAX_ERROR);
		bool bOverGo = m_bGoStraightForward? (DotProduct(vDelta, m_vMoveDir) < 0) : false;
		return bCloseToGoal || bOverGo;
	}

	virtual bool PosGetToGoal(const A3DVECTOR3& vPos)
	{
		A3DVECTOR3 vDelta = m_vGoal-m_vCurPos;
		vDelta.y  = 0.0f;
		return vDelta.SquaredMagnitude() <= (m_fSqrMinDist2Goal + 2*RELAX_ERROR*m_fMinDist2Goal + SQR_RELAX_ERROR);
	}

	inline static A3DVECTOR3 Proj2XOZ(const A3DVECTOR3& vPos)
	{
		A3DVECTOR3 vXZPos(vPos);
		vXZPos.y = 0;
		return vXZPos;
	}

	void UpdateFollowPath(const VecPOS2D& lastSearchedPath);

	// When we get to the goal, maybe the m_vCurPos is not an ideal pos.
	// An ideal pos which get to goal has just distance of m_fMinDist2Goal from m_vGoal
	// So we should adjust current pos to this ideal pos.
	void AdjustCurPosGetToGoal();

// data members
protected:

	POS2D m_p2Start;
	POS2D m_p2Goal;

	bool m_bGoStraightForward;
	
	bool m_bNeedGenPath;
		
	// keep track of current searched path which would be used to trace back.
	VecPOS2D m_CurSearchedPath;

	CPathFinding2D * m_pPathFinding;
	CPathFollowing * m_pFollowPath;

	int m_iBlockCounter;
};


#endif
