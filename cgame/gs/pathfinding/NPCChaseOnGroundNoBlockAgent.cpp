/*
 * FILE: NPCChaseOnGroundNoBlockAgent.cpp
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


#include "NPCChaseOnGroundNoBlockAgent.h"
#include "PathFollowing.h"
#include "Pf2DBfs.h"
#include <ASSERT.h>

void CNPCChaseOnGroundNoBlockAgent::Release()
{
	m_CurSearchedPath.clear();

	if(m_pFollowPath)
	{
		delete m_pFollowPath;
		m_pFollowPath = NULL;
	}

	if(m_pPathFinding)
	{
		delete m_pPathFinding;
		m_pPathFinding = NULL;
	}
}

bool CNPCChaseOnGroundNoBlockAgent::StartPathFinding(int SearchPixels)
{
	// has got to goal
	if(GetToGoal()) return false;
	
	// init
	Release();
	
	// compute start and goal pos in the map!
	//m_p2Start = g_NPCMoveMap.GetMapPos(m_vCurPos);
	m_p2Start = m_pMap->GetGroundMapPos(m_vCurPos);
	//m_p2Goal = g_NPCMoveMap.GetMapPos(m_vGoal);
	m_p2Goal = m_pMap->GetGroundMapPos(m_vGoal);
	
	// test whether we can go in straight line
	POS2D posStop;
	A3DVECTOR3 vStopPos;
	//m_bGoStraightForward = g_NPCMoveMap.CanGoStraightForward(m_p2Start, m_p2Goal, posStop);
	m_bGoStraightForward = m_pMap->CanGroundGoStraightForward(m_p2Start, m_p2Goal, posStop);
	//vStopPos = g_NPCMoveMap.GetPixelCenter(posStop);
	vStopPos = m_pMap->GetGroundPixelCenter(posStop);
	if(m_bGoStraightForward || PosGetToGoal(vStopPos))
	{
		m_bBlocked = false;
		//////////////////////////////////////////////////////////////////////////
		// revised by wenfeng, 05-8-16
		// when going straightforward, we regard it as that we've found a path
		m_bFoundPath = true;			
		return false;
	}

	m_PFPixels = SearchPixels;
	m_bNeedGenPath = (posStop == m_p2Start);

	// I can not go straight forward, so I start pathfinding and path following
	
	// init the pathfinding
	m_pPathFinding = new CPf2DBfs;
	m_pPathFinding->Init(m_pMap, posStop, m_p2Goal, m_fMinDist2Goal);
	
	// init the pathfollowing
	m_pFollowPath = new CPathFollowing;
	m_pFollowPath->AddPathNode(Proj2XOZ(m_vCurPos));
	m_pFollowPath->AddPathNode(vStopPos);
	m_pFollowPath->Start();

	// init some state value
	m_bFoundPath = false;
	m_bBlocked = false;
	//m_bIsGoalReachable = g_NPCMoveMap.IsPosNeighborsReachable(m_p2Goal, 1);
	m_bIsGoalReachable = m_pMap->IsGroundPosNeighborsReachable(m_p2Goal, 1);
	SetMaxBlockTimes();
	m_iBlockCounter = 0;
	
	return true;
}

void CNPCChaseOnGroundNoBlockAgent::MoveOneStep()
{
	// has got to goal
	if(GetToGoal()) return;

	if(m_bGoStraightForward)
	{
		// call parent's method
		CNPCChaseAgent::MoveOneStep();
		return;
	}
	else
	{
		// pathfinding
		ASSERT(m_pPathFinding);
		
		// verify that m_PFPixels should be larger than m_fMoveStep
		if(m_PFPixels < m_fMoveStep)
		{
			m_PFPixels = (int)m_fMoveStep + 1;
		}
		
		bool bNoPathFound = false;
		
		if(m_pPathFinding->GetStat() == PF2D_SEARCHING)
		{
			m_pPathFinding->StepSearch(m_PFPixels);
		}
		else if(m_pPathFinding->GetStat() == PF2D_FOUND)
		{
			m_bFoundPath = true;
		}
		else if (m_pPathFinding->GetStat() == PF2D_NOPATH)
		{
			bNoPathFound = true;
		}

		// flag to indicate that we've generated the final path to the goal or not
		bool bFinalPathGenerated = false;

		// need to generate a path?
		if(m_bNeedGenPath)
		{
			// save the last searched path
			VecPOS2D lastSearchedPath = m_CurSearchedPath;
			
			// generate the current searched path
			m_pPathFinding->GeneratePath(m_CurSearchedPath);

			// update follow path
			UpdateFollowPath(lastSearchedPath);

			if(m_bFoundPath) 
			{
				bFinalPathGenerated = true;
			}
			
		}
		
		// move according to the follow path
		ASSERT(m_pFollowPath);
		m_pFollowPath->MoveOneStep(m_fMoveStep);

		// test whether I got blocked
		if(m_pFollowPath->GetToEnd())
		{
			// blocked
			m_iBlockCounter++;
			m_bBlocked = true;
		}
		
		// judge whether I should generate a new path
		if(m_iBlockCounter > m_iMaxBlockTimes || m_bBlocked && m_bFoundPath)
		{
			// have blocked for Max times or found the path
			m_bNeedGenPath = true;
			m_bBlocked = false;
			m_iBlockCounter = 0; 
		}
		else
		{
			m_bNeedGenPath = false;
		}

		// adjust current pos
		m_vCurPos = m_pFollowPath->GetCurPos();
		m_bGetToGoal = bFinalPathGenerated && m_pFollowPath->GetToEnd();
		if(m_bGetToGoal) 
		{
			AdjustCurPosGetToGoal();
		}
		AdjustCurPos();

	}
}

void CNPCChaseOnGroundNoBlockAgent::UpdateFollowPath(const VecPOS2D& lastSearchedPath)
{
	// I'm now at the position of lastSearchedPath's end pos.
	
	ASSERT (!m_CurSearchedPath.empty());

	//***************** Not sure **************************
	// release the path I've walked here or not? 
	// it seems this would be helpful to release mememry.
	m_pFollowPath->Release();
	
	A3DVECTOR3 vPos;

	if(lastSearchedPath.empty())
	{
		// no last searched path, this would occur when calling MoveOneStep firstly
		for(size_t i=0; i<m_CurSearchedPath.size(); i++)
		{
			//vPos = g_NPCMoveMap.GetPixelCenter(m_CurSearchedPath[i]);
			vPos =  m_pMap->GetGroundPixelCenter(m_CurSearchedPath[i]);
			m_pFollowPath->AddPathNode(vPos);
		}
	}
	else
	{
		// test if we can go straight forward from lastSearchedPath's end pos
		// to m_CurSearchedPath's end pos. This would avoid trace back.
		POS2D posFrom = lastSearchedPath[lastSearchedPath.size()-1];
		POS2D posTo = m_CurSearchedPath[m_CurSearchedPath.size()-1];
		//if(g_NPCMoveMap.CanGoStraightForward(posFrom, posTo))
		if(m_pMap->CanGroundGoStraightForward(posFrom, posTo))
		{
			// Yeah! we can go straight...
			//vPos = g_NPCMoveMap.GetPixelCenter(posFrom);
			vPos = m_pMap->GetGroundPixelCenter(posFrom);
			m_pFollowPath->AddPathNode(vPos);
			
			//vPos = g_NPCMoveMap.GetPixelCenter(posTo);
			vPos = m_pMap->GetGroundPixelCenter(posTo);
			m_pFollowPath->AddPathNode(vPos);
		}
		else
		{
			// we should trace back...
			
			// find the consist path pos between lastSearchedPath and m_CurSearchedPath
			int iConsistCounter = 0;
			size_t i;
			for(i = 1; i<lastSearchedPath.size(); i++)
			{
				if(lastSearchedPath[i]==m_CurSearchedPath[i])
					iConsistCounter = i;
				else
					break;
			}

			// back trace in the last searched path
			for(i = lastSearchedPath.size()-1; i>(size_t)iConsistCounter; i--)
			{
				//vPos = g_NPCMoveMap.GetPixelCenter(lastSearchedPath[i]);
				vPos = m_pMap->GetGroundPixelCenter(lastSearchedPath[i]);
				m_pFollowPath->AddPathNode(vPos);
			}
			
			// link to the current searched path
			for(i = iConsistCounter; i < m_CurSearchedPath.size(); i++)
			{
				//vPos = g_NPCMoveMap.GetPixelCenter(m_CurSearchedPath[i]);
				vPos = m_pMap->GetGroundPixelCenter(m_CurSearchedPath[i]);
				m_pFollowPath->AddPathNode(vPos);
			}	// end of for
			
		}	// end of not go straight
	}		// end of !lastSearchedPath.empty()

	// start follow path
	m_pFollowPath->Start();

}

void CNPCChaseOnGroundNoBlockAgent::AdjustCurPosGetToGoal()
{
	// now, adjust in a very simple way
	// only adjust x and z...
	A3DVECTOR3 vDelta = m_vCurPos - m_vGoal;
	vDelta.y = 0;
	vDelta.Normalize();

	m_vCurPos = m_vGoal + m_fMinDist2Goal * vDelta;
}
