  /*
 * FILE: SpatialPathFinding.h
 *
 * DESCRIPTION:		a class which provides a solution for full-3D spatial path finding.
 *					We use Best-First Search algorithm to implement path finding.
 *					The passable map of 3D space is described by CompactSpacePassableOctree!
 *
 * CREATED BY:		He wenfeng, 2005/10/11
 *
 * HISTORY: 
 *
 * Copyright (c) 2005 Archosaur Studio, All Rights Reserved.
 */

#ifndef _SPATIALPATHFINDING_H_
#define _SPATIALPATHFINDING_H_



#include "SortVectorSpatialPathNode.h"

// In search algorthim, we set this threshold to control the search
// voxel's max number so as to exit when it can't find the path in time!
#define MAX_SEARCH_VOXEL_NUM 100000

namespace NPCMoveMap
{
	class CMap;
}

typedef vector<Pos3DInt> VecPos3DInt;

class CSpatialPathFinding
{

public:
	CSpatialPathFinding(NPCMoveMap::CMap * m_pMap);
	virtual ~CSpatialPathFinding();

	void Release()
	{
		m_Open.Release();
		m_Close.Release();
		m_PathFound.clear();
	}

	void Init(const Pos3DInt& posStart,const Pos3DInt& posGoal, int iVoxelSize);
	
	// BFS step by step, call to search one step
	void StepBestFirstSearch(int iSearchVoxels);
	
	// BFS one time, try to find the path in an action!
	void BestFirstSearch()
	{
		StepBestFirstSearch(MAX_SEARCH_VOXEL_NUM);
	}

	inline int GetManhDist(const Pos3DInt& pos1, const Pos3DInt& pos2)
	{
		return abs(pos1.x - pos2.x) + abs(pos1.y - pos2.y) + abs(pos1.z-pos2.z);
	}

	int GetEuclidDistSqr(const Pos3DInt& pos1, const Pos3DInt& pos2)
	{
		int dx = pos1.x - pos2.x, dy = pos1.y - pos2.y, dz = pos1.z - pos2.z;
		return dx * dx + dy * dy + dz * dz;
	}

	bool SearchOver() { return m_bSearchOver;}
	bool FoundPath() { return m_bFoundPath;}
	
	VecPos3DInt& GetPathFound() { return m_PathFound;}

private:

	// ********** Note *********
	// Add some extra passable test,such as when NPCs fly on air,
	// the pos should be above the terrain and water; while when NPCs 
	// swim in the water, the pos must under the water!
	virtual bool ExtraPassableTest(const Pos3DInt& pos)
	{
		return true;
	}

	// Build the whole path by a PtrSpatialPathNode
	void BuildPath(PtrSpatialPathNode pPathNode);

	// Simplify the path, remove some redundant path node
	// While the process is a time-consuming work, so for now we don't implement it
	void SimplifyPath();

private:
	Pos3DInt m_posStart;
	Pos3DInt m_posGoal;
	
	CSortVectorSpatialPathNode m_Open;
	CSortVectorSpatialPathNode m_Close;

	VecPos3DInt m_PathFound;		// the path is in order, from m_posStart to m_posGoal

	bool m_bSearchOver;				// search is finished?
	bool m_bFoundPath;				// have we found a path from the start to the goal?

	CSPOctreeTravNode m_GoalOctrTravNode;
	
	int m_iVoxelSize;				// the size of each voxel

	int m_iVoxelsSearched;

protected:
	NPCMoveMap::CMap * m_pMap;
};

//////////////////////////////////////////////////////////////////////////
// Derived classes from CSpatialPathFinding which 
// only override ExtraPassableTest function
//////////////////////////////////////////////////////////////////////////

class CAerialPathFinding : public CSpatialPathFinding
{
public:
	CAerialPathFinding(NPCMoveMap::CMap * pMap):CSpatialPathFinding(pMap){}		
private:
	virtual bool ExtraPassableTest(const Pos3DInt& pos);
};


class CUnderwaterPathFinding : public CSpatialPathFinding
{
public:
	CUnderwaterPathFinding(NPCMoveMap::CMap * pMap):CSpatialPathFinding(pMap){}		
private:
	virtual bool ExtraPassableTest(const Pos3DInt& pos);
};


#endif
