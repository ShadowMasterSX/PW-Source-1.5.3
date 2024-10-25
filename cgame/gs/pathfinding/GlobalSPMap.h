 /*
 * FILE: GlobalSPMap.h
 *
 * DESCRIPTION:   Describe a global space passable map which is composed of some blocks(that is submaps).
 *				  Each block is described by CCompactSpacePassableOctree. 
 *				  Hehe, we now use singleton pattern to implement it!
 *							
 * CREATED BY: He wenfeng, 2005/9/29
 *
 * HISTORY: 
 *
 * Copyright (c) 2005 Archosaur Studio, All Rights Reserved.
 */

#ifndef _GLOBALSPMAP_H_
#define _GLOBALSPMAP_H_

#include <a3dvector.h>
#include <ASSERT.h>

#include "CompactSpacePassableOctree.h"
using namespace SPOctree;

// class CGlobalSPMapHash;

namespace NPCMoveMap
{
	class CMap;
}

class CGlobalSPMap
{
public:
	
	enum
	{
		Env_OnAir,
		Env_UnderWater
	};
	
//////////////////////////////////////////////////////////////////////////
// Extra passable test function type declaration
	typedef bool (*PtrFuncExtraPosPassableTest)(const A3DVECTOR3& vPos, NPCMoveMap::CMap * pMap);
//
//////////////////////////////////////////////////////////////////////////

	void Release()
	{
		if(m_SPMaps) 
		{
			delete [] m_SPMaps;
			m_SPMaps = NULL;
		}
	}

	int GetVoxelSize()
	{
		return m_iVoxelSize;
	}

	inline CCompactSpacePassableOctree* GetSPMap(int row, int col)
	{
		// valid verification
		if( row < 0 || row >= m_iLength || col < 0 || col >= m_iWidth)
			return NULL;

		return m_SPMaps + row * m_iWidth + col;
	}

	// This interface may be called high-frequently,so we must optimize it!
	inline CCompactSpacePassableOctree* LocateSPMap(const Pos3DInt& pos)
	{
		if( pos.y < 0 || pos.y > m_iSubMapSize) return NULL;
		
		int x = pos.x - m_SPMaps[0].GetCenter().x + (m_iSubMapSize>>1);;
		int z = pos.z - m_SPMaps[0].GetCenter().z + (m_iSubMapSize>>1);;
		
		int row, col;
		if(m_bStandardSubmap)
		{
			row = z >> 10;	// equals z / 1024
			col = x >> 10;  // equals x / 1024
		}
		else
		{
			row = z / m_iSubMapSize;
			col = x / m_iSubMapSize;
		}

		return GetSPMap(row, col);
	}

	inline CCompactSpacePassableOctree* LocateSPMap(const A3DVECTOR3& vPos)
	{
		if( vPos.y < 0 || vPos.y > m_iSubMapSize) return NULL;

		// refer to the left-bottom position of global map as the origin
		float x = vPos.x - m_SPMaps[0].GetCenter().x + (m_iSubMapSize>>1);
		float z = vPos.z - m_SPMaps[0].GetCenter().z + (m_iSubMapSize>>1);
		
		// float fRecipSize = 1.0f/m_iSubMapSize;
		int row = (int) (z * m_fRecipSubMapSize);
		int col = (int) (x * m_fRecipSubMapSize);

		return GetSPMap(row,col);
	}

	inline bool IsPosValid(const Pos3DInt& pos)
	{
		return (LocateSPMap(pos) != NULL);
	}

	inline bool IsPosValid(const A3DVECTOR3& vPos)
	{
		return (LocateSPMap(vPos) != NULL);
	}

	inline Pos3DInt GetVoxelCenter(const A3DVECTOR3& vPos)
	{
		Pos3DInt pos(vPos);
		CCompactSpacePassableOctree * pSPMap=LocateSPMap(vPos);
		if(!pSPMap) return pos;

		// compute the coordinates in the local map
		float x,y,z;
		x = vPos.x - pSPMap->GetCenter().x;
		y = vPos.y - pSPMap->GetCenter().y;
		z = vPos.z - pSPMap->GetCenter().z;

		// compute the voxel coordinate
		// float fRecipVSize = 1.0f / m_iVoxelSize;
		pos.x = (int)(x * m_fRecipVoxelSize);
		pos.y = (int)(y * m_fRecipVoxelSize);
		pos.z = (int)(z * m_fRecipVoxelSize);

		// compute the voxel's center
		pos.x *= m_iVoxelSize;
		pos.y *= m_iVoxelSize;
		pos.z *= m_iVoxelSize;

		int iHalfVoxelSize = m_iVoxelSize>>1;
		pos.x += (x>0)?iHalfVoxelSize:-iHalfVoxelSize;
		pos.y += (y>0)?iHalfVoxelSize:-iHalfVoxelSize;
		pos.z += (z>0)?iHalfVoxelSize:-iHalfVoxelSize;

		// back to global coordinates!
		pos.x += pSPMap->GetCenter().x;
		pos.y += pSPMap->GetCenter().y;
		pos.z += pSPMap->GetCenter().z;

		return pos;
	}

	inline bool GetTraversalNode(const Pos3DInt& pos, CSPOctreeTravNode& OctrTravNode, const CSPOctreeTravNode * pRefOctrTravNode = NULL)
	{
		CCompactSpacePassableOctree * pSPMap = LocateSPMap(pos);
		if(!pSPMap) return false;		// outside of boundary
		
		pSPMap->GetTraversalNode(pos, OctrTravNode, pRefOctrTravNode);
		return true;
	}

	inline bool GetTraversalNode(const A3DVECTOR3& vPos, CSPOctreeTravNode& OctrTravNode, const CSPOctreeTravNode * pRefOctrTravNode = NULL)
	{
		return GetTraversalNode(GetVoxelCenter(vPos), OctrTravNode, pRefOctrTravNode);
	}

	inline bool IsPosPassable(const Pos3DInt& pos, CSPOctreeTravNode& OctrTravNode, const CSPOctreeTravNode * pRefOctrTravNode = NULL,PtrFuncExtraPosPassableTest pFuncExtraPosPassableTest = NULL)
	{
		if(pFuncExtraPosPassableTest && !pFuncExtraPosPassableTest(A3DVECTOR3(pos.x,pos.y,pos.z), m_pMap))
		{
			return false;
		}

		if(!GetTraversalNode(pos, OctrTravNode,pRefOctrTravNode))
			return false;
		else
			return OctrTravNode.State == CCompactSpacePassableOctree::Free;
	}

	inline bool IsPosPassable(const Pos3DInt& pos,PtrFuncExtraPosPassableTest pFuncExtraPosPassableTest = NULL)
	{
		CSPOctreeTravNode OctrTravNode;
		return IsPosPassable(pos, OctrTravNode,NULL,pFuncExtraPosPassableTest);
	}
	
	inline bool IsPosPassable(const A3DVECTOR3& vPos, CSPOctreeTravNode& OctrTravNode, const CSPOctreeTravNode * pRefOctrTravNode = NULL,PtrFuncExtraPosPassableTest pFuncExtraPosPassableTest = NULL)
	{
		if(pFuncExtraPosPassableTest && !pFuncExtraPosPassableTest(vPos, m_pMap))
		{
			return false;
		}

		if(!GetTraversalNode(vPos, OctrTravNode,pRefOctrTravNode))
			return false;
		else
			return OctrTravNode.State == CCompactSpacePassableOctree::Free;
	}

	inline bool IsPosPassable(const A3DVECTOR3& vPos,PtrFuncExtraPosPassableTest pFuncExtraPosPassableTest = NULL)
	{
		CSPOctreeTravNode OctrTravNode;
		return IsPosPassable(vPos, OctrTravNode,NULL,pFuncExtraPosPassableTest);
	}

	// **************** Added by wenfeng, 05-11-19
	// the interface for generating the hash table form of spatial passable map
	// a little logical diffrence from the interface IsPosPassable
	// return: true if vPos is Free or Blocked
	//		   false if vPos is CHBorder or beyond the current range of map
	inline bool CanPosPass(const A3DVECTOR3& vPos)
	{
		CSPOctreeTravNode OctrTravNode;
		if(!GetTraversalNode(vPos, OctrTravNode))
			return false;
		
		return OctrTravNode.State != CCompactSpacePassableOctree::CHBorder;
	}
	
	// Note: this interface do not consider the terrain and water information!
	bool CanGoStraightForward(const A3DVECTOR3& vFrom, const A3DVECTOR3& vTo, Pos3DInt& posStop, PtrFuncExtraPosPassableTest pFuncExtraPosPassableTest = NULL, int * pHitCount = NULL);

	// For now, it's not implemented! 05-10-22
	bool CanGoStraightForward(const Pos3DInt& posFrom, const Pos3DInt& posTo, Pos3DInt& posStop);
	
	void GetLastReachablePos(const A3DVECTOR3& vStart, const A3DVECTOR3& vEnd, A3DVECTOR3& vStopPos, int iEnv);

	// Load data from a directory
	bool Load(const char* szMapPath);
	bool LoadFreePiece(const char* szMapPath);
	bool SetFreeMap(int row, int col, CGlobalSPMap * spmap_template);

	int GetRows() const { return m_iLength;}
	int GetColumns() const { return m_iWidth;}
	A3DVECTOR3 GetSubmapCenter(int row, int col)
	{
		CCompactSpacePassableOctree * pSPMap = GetSPMap(row, col);
		ASSERT(pSPMap);
		A3DVECTOR3 vCenter(pSPMap->GetCenter().x, pSPMap->GetCenter().y, pSPMap->GetCenter().z);
		return vCenter;
	}

	// void Export(CGlobalSPMapHash* pGlobalSPMapHash);

private:
	
	// Set the submap(col,row) to as a free map
	void SetFreeSPMap(int row, int col, int MapID,const Pos3DInt& posGlobalOrigin);

// --------------------------------------------------------
// The following code is for singleton pattern!
public:
	/*static CGlobalSPMap* GetInstance()
	{
		if(!s_GlobalSPMapInstance)
		{
			s_GlobalSPMapInstance = new CGlobalSPMap;
		}
		
		return s_GlobalSPMapInstance;
	}

	static void ReleaseInstance()
	{
		if(s_GlobalSPMapInstance)
		{
			s_GlobalSPMapInstance->Release();
			s_GlobalSPMapInstance = NULL;
		}
	}*/

	CGlobalSPMap(NPCMoveMap::CMap * pMap):m_pMap(pMap)
	{
		m_SPMaps = NULL;
	}
	
	~CGlobalSPMap()
	{
		Release();
	}

private:
	CGlobalSPMap(const CGlobalSPMap& );
	const CGlobalSPMap& operator=(const CGlobalSPMap& );

	//static CGlobalSPMap* s_GlobalSPMapInstance;

// --------------------------------------------------------

// data members
private:
	
	// Each SPMap is an instance of class CCompactSpacePassableOctree.
	// Stored order: row-prior
	CCompactSpacePassableOctree* m_SPMaps;

	int m_iWidth;					// width in sub-maps
	int m_iLength;					// length in sub-maps

	int m_iSubMapSize;				// submap's size, which equals the cube's size of each CCompactSpacePassableOctree
	float m_fRecipSubMapSize;		// Reciprocal of m_iSubMapSize, for speed computing

	bool m_bStandardSubmap;			// whether the submap is 1024*1024 (x * z)

	int m_iVoxelSize;				// Voxel is the minimum cube unit of 3D space ( which should be >= 2 )
	float m_fRecipVoxelSize;		// Reciprocal of m_iVoxelSize, for speed computing

	NPCMoveMap::CMap * m_pMap;
};


#endif
