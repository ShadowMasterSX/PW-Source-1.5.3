    /*
 * FILE: SpatialPathFinding.cpp
 *
 * DESCRIPTION:   a class which provides a engine for full-3D spatial path finding.
 *							We use Best-First Search algorithm to implement path finding.
 *							The passable map of 3D space is described by CompactSpacePassableOctree!
 *
 * CREATED BY:	  He wenfeng, 2005/10/11
 *
 * HISTORY: 
 *
 * Copyright (c) 2005 Archosaur Studio, All Rights Reserved.
 */

#include "SpatialPathFinding.h"
#include "GlobalSPMap.h"
#include "NPCMoveMap.h"
#include "GlobalWaterAreaMap.h"
#include "NPCMove.h"
#include "NPCChaseSpatiallyPFAgent.h"

CSpatialPathFinding::CSpatialPathFinding(NPCMoveMap::CMap * pMap):m_pMap(pMap)
{

}

CSpatialPathFinding::~CSpatialPathFinding()
{
	Release();
}


void CSpatialPathFinding::Init(const Pos3DInt& posStart,const Pos3DInt& posGoal, int iVoxelSize)
{
	Release();

	m_posStart = posStart;
	m_posGoal = posGoal;
	m_iVoxelSize = iVoxelSize;

	m_bSearchOver = false;
	m_bFoundPath = false;
	
	CSPOctreeTravNode StartOctrTravNode;
	// Verify the start and goal pos are passable
	if(!(ExtraPassableTest(posStart) && ExtraPassableTest(posGoal) &&
		 //CGlobalSPMap::GetInstance()->IsPosPassable(posStart,StartOctrTravNode) &&
		 m_pMap->IsPosPassable(posStart,StartOctrTravNode) &&
		 //CGlobalSPMap::GetInstance()->IsPosPassable(posGoal,m_GoalOctrTravNode,&StartOctrTravNode)))
		 m_pMap->IsPosPassable(posGoal,m_GoalOctrTravNode,&StartOctrTravNode)))
	{
		m_bSearchOver = true;		// searching is over, but no path founded!
		m_bFoundPath = false;		
		return;
	}
	
	// Init the BFS algorithm
	CSPOctreeTravNode *pOctrTravNode = new CSPOctreeTravNode(StartOctrTravNode);
	m_Open.SortPush(posStart, GetManhDist(posStart, m_posGoal),pOctrTravNode, NULL);

	m_iVoxelsSearched = 0;
}

void CSpatialPathFinding::StepBestFirstSearch(int iSearchVoxels)
{
	if(m_bSearchOver) return;
	
	int iCounter = 0;
	PtrSpatialPathNode pPathNode;
	Pos3DInt posNeighbor;

	CSPOctreeTravNode NeighborOctrTravNode;
	int CurH, MinH;
	Pos3DInt posMinH;
	bool bPosPassable;

	while (! m_Open.empty() && iCounter < iSearchVoxels && m_iVoxelsSearched < MAX_SEARCH_VOXEL_NUM)
	{
		pPathNode = m_Open.PopFront();
		if(pPathNode->pos == m_posGoal)
		{
			m_bFoundPath = true;
			break;
		}
		
		iCounter++;
		m_iVoxelsSearched ++;
		
		// search each neighbor of the current pos
		int dx, dy, dz;
		for( int i=0; i<27; ++i)
		{
			// when i==13, we get dx=0,dy=0 and dz=0,so the neighbor pos is same to the current pos
			if(i==13) continue;

			dx = i / 9 -1;
			dy = (i %9) /3 -1;
			dz = i % 3 -1;
			
			posNeighbor.x = pPathNode->pos.x + dx * m_iVoxelSize;
			posNeighbor.y = pPathNode->pos.y + dy * m_iVoxelSize;
			posNeighbor.z = pPathNode->pos.z + dz * m_iVoxelSize;

			if(!ExtraPassableTest(posNeighbor)) continue;
			
			//////////////////////////////////////////////////////////////////////////
			// Noted by wenfeng, 05-12-2
			//  convert to Octree based spmap again!
			// Noted by wenfeng, 05-12-1
			// when converting the global passmap to hash table, we have two choice here.
			// 1. make the condition always false,so don't call straight line test here
			// 2. call the straight line test here, but it's seems time-consuming
			//////////////////////////////////////////////////////////////////////////

			// test if the posNeighbor can go straight to m_posGoal!
			if(m_GoalOctrTravNode.IsPosInside(posNeighbor) || 
			   //(bPosPassable= CGlobalSPMap::GetInstance()->IsPosPassable(posNeighbor,NeighborOctrTravNode,pPathNode->pOctrTravNode)) &&
			   (bPosPassable= m_pMap->IsPosPassable(posNeighbor,NeighborOctrTravNode,pPathNode->pOctrTravNode)) &&
			   m_GoalOctrTravNode.IsNodeNeighborSibling(NeighborOctrTravNode) )
			{
				// try to find the least-h in all neighbors
				CurH = GetManhDist(posNeighbor, m_posGoal);
				if(!m_bFoundPath)
				{
					MinH = CurH;
					posMinH = posNeighbor;
				}
				else
				{
					if(CurH < MinH)
					{
						MinH = CurH;
						posMinH = posNeighbor;
					}
				}

				m_bFoundPath = true;
				continue;	// go directly to next for-loop				
			}
			
			if(bPosPassable && ! m_Close.FindByPos(posNeighbor))
			{
				CSPOctreeTravNode * pNeighborOctrTravNode = new CSPOctreeTravNode(NeighborOctrTravNode);
				bool bPushSucceed = m_Open.SortPush(posNeighbor,GetManhDist(posNeighbor, m_posGoal), pNeighborOctrTravNode,pPathNode);
				if(!bPushSucceed) delete pNeighborOctrTravNode;
			}

		}	// end of for-loop	
		
		if(m_bFoundPath)
			break;
		else
			m_Close.push_back(pPathNode);

	}	// end of while-loop

	if(m_bFoundPath)
	{
		BuildPath(pPathNode);
		
		if(!(m_PathFound.back() == m_posGoal))
		{
			m_PathFound.push_back(posMinH);
			m_PathFound.push_back(m_posGoal);
		}

		// Not to forget delete the pPathNode! 
		// Because pPathNode has been poped up from the m_Open and not to be pushed in m_Close
		delete pPathNode;

		m_bSearchOver = true;
	}
}

void CSpatialPathFinding::BuildPath(PtrSpatialPathNode pPathNode)
{
	VecPos3DInt InvPath;
	while(pPathNode)
	{
		InvPath.push_back(pPathNode->pos);
		pPathNode = pPathNode->pLastPathNode;
	}

	while(!InvPath.empty())
	{

		m_PathFound.push_back(InvPath.back());
		InvPath.pop_back();
	}
}

bool CAerialPathFinding::ExtraPassableTest(const Pos3DInt& pos)
{
	// Verify the pos must be above the terrain and water
	return CNPCChaseSpatiallyPFAgent::IsPosOnAir(A3DVECTOR3(pos.x,pos.y,pos.z), m_pMap);
}

bool CUnderwaterPathFinding::ExtraPassableTest(const Pos3DInt& pos)
{
	// Verify the pos must be above the terrain while under the water
	return CNPCChaseSpatiallyPFAgent::IsPosInWater(A3DVECTOR3(pos.x,pos.y,pos.z), m_pMap);
}
