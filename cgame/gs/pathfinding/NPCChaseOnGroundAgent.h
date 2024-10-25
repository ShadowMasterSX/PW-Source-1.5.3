 /*
 * FILE: NPCChaseOnGroundAgent.h
 *
 * DESCRIPTION:   A  class derived from the class CNPCChaseAgent 
 *							which realizes on-ground NPCs' chasing movement.
 *
 * CREATED BY: He wenfeng, 2005/4/18
 *
 * HISTORY: 
 *
 * Copyright (c) 2004 Archosaur Studio, All Rights Reserved.
 */

#ifndef _NPCCHASEONGROUNDAGENT_H_
#define _NPCCHASEONGROUNDAGENT_H_

#include <math.h>
#include "NPCMove.h"
#include "NPCMoveMap.h"
#include "SortVectorPathNode.h"

typedef vector<POS2D> vecPOS2D;

class CNPCChaseOnGroundAgent: public CNPCChaseAgent
{
public:
	CNPCChaseOnGroundAgent(CMap * pMap);
	virtual ~CNPCChaseOnGroundAgent();

	virtual void Release();
	virtual void MoveOneStep();
	
	virtual void SetMoveStep(float fStep)
	{
		CNPCMoveAgent::SetMoveStep(fStep);
		//m_StepPixels = (int) (m_fMoveStep / g_NPCMoveMap.GetPixelSize() + 0.5f );
		m_StepPixels = (int) (m_fMoveStep / m_pMap->GetGroundPixelSize() + 0.5f );

		if(m_StepPixels == 0)
		{
			// too small steps
			m_bStepTooSmall = true;
			//m_iStepsOneGrid = (int) (g_NPCMoveMap.GetPixelSize() / m_fMoveStep +0.5f);
			m_iStepsOneGrid = (int) (m_pMap->GetGroundPixelSize() / m_fMoveStep +0.5f);
			m_iCurStep = 0;
			
			// for now, we set the m_StepPixels to 1
			m_StepPixels = 1;

		}
		else
			m_bStepTooSmall = false;
	}
	
	// false if path finding is no need, m_vCurPos is close enough to m_vGoal
	virtual bool StartPathFinding(int SearchPixels = 10);

	//////////////////////////////////////////////////////////////////////////
	// After StartPathFinding, call this func to test if I can go to the goal straight forward?
	// We use the Integer-Bresenham Line-Raster Algorithm to explore whether the pixels on the 
	// straight line from m_p2Start to m_p2Goal are reachable!
	//////////////////////////////////////////////////////////////////////////
	bool CanGoStraightForward();
	
	bool CanGoStraightForward(const POS2D& posFrom, const POS2D& posTo);

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
/*
		// here, we increase the expected distance to goal by RELAX_ERROR
		return ( vDelta.SquaredMagnitude() <= (m_fSqrMinDist2Goal + 2*RELAX_ERROR*m_fMinDist2Goal + SQR_RELAX_ERROR)
			|| m_fMinDist2Goal < ZERO_DIST_ERROR && DotProduct(vDelta, m_vMoveDir) < 0  );
*/
	}
	
	//////////////////////////////////////////////////////////////////////////
	// When curMapPos satisfies the PosGetToGoal condition, it doesn't mean
	// the 3D pos m_vCurPos meets the GetToGoal condition due to the error between int and float!
	// So we adjust the m_vCurPos to fit the condition through this function!
	//////////////////////////////////////////////////////////////////////////
	bool AdjustCurPosGetToGoal(const POS2D& curMapPos, const POS2D& lastMapPos);
	
	void FollowFoundPath();

	void GeneratePath(LPPathNode lpPN);

	bool InsertPathNode(LPPathNode lpPN);
	
	void PredictPath();

	void BestFirstSearchPath();				// using BFS algorithm!

	int GetDist(int u, int v, const POS2D& pos)		
	{
		// here we use du+dv as the distance
		return abs(u-pos.u) + abs(v-pos.v);
	}

	int GetEuclidDistSqr(int u, int v, const POS2D& pos)
	{
		int du=u-pos.u,dv=v-pos.v;
		return ( du*du + dv*dv);
	}

	int GetEuclidDistSqr(const POS2D& pos1, const POS2D& pos2)
	{
		return GetEuclidDistSqr(pos1.u,pos1.v,pos2);
	}

	bool PosGetToGoal(const POS2D& pos)
	{
		return ( GetEuclidDistSqr(pos, m_p2Goal) <= m_fSqrMinDist2Goal) ;
	}

	bool PosGetToGoal(int u, int v)
	{
		return ( GetEuclidDistSqr(u,v, m_p2Goal) <= m_fSqrMinDist2Goal ) ;
	}

protected:

	bool m_bGoStraightForward;		// Go in straight line to the goal


	// Path finding data
	CSortVectorPathNode m_Open;
	CSortVectorPathNode m_Close;			// only use the FindByPos function
	vecPOS2D m_PathFound;					
	vecPOS2D m_PathPredict;

	POS2D m_p2Start;
	POS2D m_p2Goal;
	
	char m_du;
	char m_dv;
	
	char m_StepPixels;				// the pixels in one step

	int m_iCurInFoundPath;		// use for following the found path
	
	// members for the case of too small move step
	bool m_bStepTooSmall;		// use for the move step is less then One grid!
	int m_iStepsOneGrid;
	int m_iCurStep;
	A3DVECTOR3 m_vSmallStep;
};


#endif

