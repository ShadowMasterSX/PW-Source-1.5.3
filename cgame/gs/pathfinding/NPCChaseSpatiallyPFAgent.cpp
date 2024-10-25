/*
 * FILE: NPCChaseSpatiallyPFAgent.cpp
 *
 * DESCRIPTION:   A  class derived from the class CNPCChaseAgent 
 *						which is a base class for spatial NPCs(including On-air and in-water NPC)'
 *						 chasing movement considering avoiding obstructs! 
 *
 * CREATED BY: He wenfeng, 2005/11/3
 *
 * HISTORY: 
 *
 * Copyright (c) 2005 Archosaur Studio, All Rights Reserved.
 */

#include "NPCChaseSpatiallyPFAgent.h"

#include "SpatialPathFinding.h"
#include "PathFollowing.h"

#include "HalfSpace.h"
using namespace CHBasedCD;

/*
bool CNPCChaseSpatiallyPFAgent::IsPosAboveTerrain(const A3DVECTOR3& vPos)
{
	float fTerrainHeight = NPCMove::GetTerrainHeight(vPos.x, vPos.z);
	return vPos.y > fTerrainHeight + ABOVE_DIST;
}

bool CNPCChaseSpatiallyPFAgent::IsPosOnAir(const A3DVECTOR3& vPos)
{
	// Verify the pos must be above the terrain and water
	float fTerrainHeight = NPCMove::GetTerrainHeight(vPos.x, vPos.z);
	float fWaterHeight = WaterAreaMap::GetWaterHeight(vPos.x,vPos.z);

	if(vPos.y > fTerrainHeight + ABOVE_DIST && vPos.y > fWaterHeight + ABOVE_DIST)
		return true;
	else
		return false;
}

bool CNPCChaseSpatiallyPFAgent::IsPosInWater(const A3DVECTOR3& vPos)
{
	// Verify the pos must be above the terrain while under the water
	float fTerrainHeight = NPCMove::GetTerrainHeight(vPos.x, vPos.z);
	float fWaterHeight = WaterAreaMap::GetWaterHeight(vPos.x,vPos.z);

	if(vPos.y > fTerrainHeight + ABOVE_DIST && vPos.y < fWaterHeight - BELOW_DIST)
		return true;
	else
		return false;
}
*/

void CNPCChaseSpatiallyPFAgent::Release()
{
	if(m_pChaseStraightAgent)
	{
		delete m_pChaseStraightAgent;
		m_pChaseStraightAgent = NULL;
	}

	if(m_pSpatialPathFinding)
	{
		delete m_pSpatialPathFinding;
		m_pSpatialPathFinding = NULL;
	}

	if(m_pCurPath)
	{
		delete m_pCurPath;
		m_pCurPath = NULL;
	}
	
}


bool CNPCChaseSpatiallyPFAgent::StartPathFinding(int SearchPixels)
{
	if(GetToGoal()) return false;		// I've got to the goal!

	Release();
	m_PFPixels = SearchPixels;
	m_bGoStraightForward = false;

	//if(!CGlobalSPMap::GetInstance()->IsPosPassable(m_vCurPos,m_ptrFuncExtraPosPassableTest))
	if(!m_pMap->IsPosPassable(m_vCurPos,m_ptrFuncExtraPosPassableTest))
	{
		// current pos is an invalid pos!

		// ******* Noted by wenfeng, 05-10-29
		// maybe when the start pos is blocked, we'd better employ some rescue strategy,
		// such as go straight forward to the goal.

		m_bBlocked = true;
		m_bBlockedBeyondEnv = true;
		m_bFoundPath = false;
		return false;
	}
	
	Pos3DInt posBlocked;
	// ******** Revised and noted by wenfeng,05-11-5
	// When we test if we can get to the goal in straight line, we ignore 
	// the terrain and water factors and only consider building obstructs!
	// And this revision is for speed consideration.
	// **********************************************
	// if(CGlobalSPMap::GetInstance()->CanGoStraightForward(m_vCurPos,m_vGoal,posBlocked,m_ptrFuncExtraPosPassableTest) ||	// Yeah! I can go straightforward now
	//if(CGlobalSPMap::GetInstance()->CanGoStraightForward(m_vCurPos,m_vGoal,posBlocked,NULL) ||	// Yeah! I can go straightforward now
	if(m_pMap->CanGoStraightForward(m_vCurPos,m_vGoal,posBlocked,NULL) ||	// Yeah! I can go straightforward now
		PosGetToGoal(A3DVECTOR3(posBlocked.x,posBlocked.y,posBlocked.z)) ||		// I'll be blocked, while the blocked pos is regarded being in the range of the goal
		!GeneratePathFindingGoal(m_vPFGoal) )		// Failing when generate a passable goal for pathfinding,so we go straight forward as well.
	{
		// Yeah! I can go straightforward now :) 
		// Init my own states
		CNPCChaseAgent::StartPathFinding(SearchPixels);
		// employ m_pChaseStraightAgent to go
		m_pChaseStraightAgent = CreateChaseStraightAgent();
		m_pChaseStraightAgent->Init(m_vCurPos, m_fMoveStep);
		m_pChaseStraightAgent->SetGoal(m_vGoal,m_fMinDist2Goal);
		m_pChaseStraightAgent->StartPathFinding(SearchPixels);
		
		m_bGoStraightForward = true;
	}
	else
	{
		//Pos3DInt posPFGoal = CGlobalSPMap::GetInstance()->GetVoxelCenter(m_vPFGoal);
		Pos3DInt posPFGoal = m_pMap->GetVoxelCenter(m_vPFGoal);

		// Init path finding.
		m_pSpatialPathFinding = CreateSpatialPathFinding();
		//m_pSpatialPathFinding->Init(posBlocked,posPFGoal,CGlobalSPMap::GetInstance()->GetVoxelSize());
		m_pSpatialPathFinding->Init(posBlocked,posPFGoal,m_pMap->GetVoxelSize());

		// I will be blocked at posBlocked when going straight to the goal.
		m_pCurPath = new CPathFollowing;
		m_pCurPath->AddPathNode(m_vCurPos);
		m_pCurPath->AddPathNode(A3DVECTOR3(posBlocked.x,posBlocked.y,posBlocked.z));
		m_pCurPath->Start();
		
		m_bBlocked = false;
		m_bFoundPath = false;
		
		m_bIsGoalReachable = true;			// ???, this may be wrong! But when we set m_bIsGoalReachable's value?
	}

	return true;
}

void CNPCChaseSpatiallyPFAgent::MoveOneStep()
{
	if(GetToGoal() || (m_bBlocked && !m_pSpatialPathFinding)) return;
	
	// go straight forward
	if(m_bGoStraightForward && m_pChaseStraightAgent)
	{
		m_pChaseStraightAgent->MoveOneStep();
		// clone the states of m_pChaseStraightAgent
		m_vCurPos = m_pChaseStraightAgent->GetCurPos();
		m_bGetToGoal = m_pChaseStraightAgent->GetToGoal();
		m_bBlockedBeyondEnv = m_pChaseStraightAgent->BlockedBeyondEnv();
		return;
	}

	// moving one step
	if(!m_bBlocked && m_pCurPath)
	{
		m_pCurPath->MoveOneStep(m_fMoveStep);
		m_vCurPos = m_pCurPath->GetCurPos();
		m_bGetToGoal = PosGetToGoal(m_vCurPos);
		m_bBlocked = m_pCurPath->GetToEnd();
	}

	// path finding
	if(m_pSpatialPathFinding && !m_pSpatialPathFinding->SearchOver())
	{
		m_pSpatialPathFinding->StepBestFirstSearch(m_PFPixels);
		if(m_pSpatialPathFinding->FoundPath())
		{
			// found a path, add the path nodes to m_pCurPath
			VecPos3DInt& PathFound = m_pSpatialPathFinding->GetPathFound();
			for(size_t i=0; i< PathFound.size(); ++i)
			{
				A3DVECTOR3 vPathNode(PathFound[i].x,PathFound[i].y,PathFound[i].z);
				m_pCurPath->AddPathNode(vPathNode);
			}
			// Not to forget to add m_vPFGoal as the last node!
			m_pCurPath->AddPathNode(m_vPFGoal);

			m_bFoundPath = true;
			m_bBlocked = false;
		}
	}
		
}

bool CNPCChaseSpatiallyPFAgent::GeneratePathFindingGoal(A3DVECTOR3& vPFGoal)
{
	if(m_fMinDist2Goal < ZERO_DIST_ERROR )
	{
		vPFGoal = m_vGoal;
		return true;
	}

	vPFGoal = m_vGoal - m_fMinDist2Goal * m_vMoveDir;
	//if(CGlobalSPMap::GetInstance()->IsPosPassable(vPFGoal,m_ptrFuncExtraPosPassableTest))
	if(m_pMap->IsPosPassable(vPFGoal,m_ptrFuncExtraPosPassableTest))
	{
		return true;
	}

	// construct a plane by the Vertex: m_vGoal and the Normal: m_vMoveDir
	CHalfSpace hs;
	hs.SetNV(m_vMoveDir, m_vGoal);

	A3DVECTOR3 vOffset;
	float latitude,longitude,tmp;
	int iGenCounter =0;
	bool bPassableGoal = false;
	do 
	{
		// Generate a random position on the sphere surface.
		latitude = RAND(PI) - PI /2;
		longitude = RAND( 2*PI );
		vOffset.y = m_fMinDist2Goal * sin(latitude);
		tmp = m_fMinDist2Goal * cos(latitude);
		vOffset.x = tmp * cos(longitude);
		vOffset.z = tmp * sin(longitude);

		vPFGoal = m_vGoal + vOffset;

		// Judge whether the position in the nearer hemisphere
		if( DotProduct(m_vMoveDir, vOffset) > 0 )
		{
			hs.Mirror(vPFGoal);
		}
		
		//bPassableGoal = CGlobalSPMap::GetInstance()->IsPosPassable(vPFGoal,m_ptrFuncExtraPosPassableTest);
		bPassableGoal = m_pMap->IsPosPassable(vPFGoal,m_ptrFuncExtraPosPassableTest);

		++iGenCounter;
	} 
	while(iGenCounter<MAX_GENERATE_GOAL_TIMES && !bPassableGoal);

	if(bPassableGoal)
		return true;
	else
		return false;
	
}
