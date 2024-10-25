/********************************************************************
  created:	   11/4/2006  
  filename:	   Map.h
  author:      Wangkuiwu  
  description:  the npc move map(ground, water, area)
  Copyright (c) , All Rights Reserved.
*********************************************************************/

#ifndef _NPCMOVEMAP_CMAP_H
#define _NPCMOVEMAP_CMAP_H


#include <ASSERT.h>
#include "NPCMoveMap.h"
#include "GlobalWaterAreaMap.h"
#include "GlobalSPMap.h"

namespace NPCMoveMap
{

//class CNPCMoveMap;

class CMap  
{
public:
	CMap();
	virtual ~CMap();
	/**
	 * \brief init the pathfinding map
	 * \param[in] szGroundPath   the ground path dir
	 * \param[in] szWaterPath, szAreaPath the water,area path, currently ingored.
	 * \param[out]
	 * \return
	 * \note
	 * \warning
	 * \todo   
	 * \author kuiwu 
	 * \date 11/4/2006
	 * \see 
	 */
	bool Init(const char * szGroundPath, const char * szWaterMap, const char * szAreamap);
	bool InitPiece(const char * szGroundPath, const char * szWaterMap, const char * szAreamap, int piece_idx);
	bool Init(int row, int col, const int * piece_indexes, size_t piece_count, CMap** movemap_pieces);
	void Release();
	
	inline	float GetGroundPixelSize() const
	{
		ASSERT(m_pGroundMap && "no ground map");
		return m_pGroundMap->GetPixelSize();
	}
	inline POS2D  GetGroundMapPos(const A3DVECTOR3& vPos) const
	{
		ASSERT(m_pGroundMap && "no ground map");
		return m_pGroundMap->GetMapPos(vPos);
	}
	inline float GetGroundPosDeltaHeight (const POS2D& pos) const
	{
		ASSERT(m_pGroundMap && "no ground map");
		return m_pGroundMap->GetPosDeltaHeight(pos);
	}
	inline A3DVECTOR3 GetGroundPixelCenter(const POS2D& mPos) const
	{
		ASSERT(m_pGroundMap && "no ground map");
		return m_pGroundMap->GetPixelCenter(mPos);
	}
	inline bool IsGroundPosNeighborsReachable(const POS2D& pos, int AdjPixels) const
	{
		ASSERT(m_pGroundMap && "no ground map");
		return m_pGroundMap->IsPosNeighborsReachable(pos, AdjPixels);
	}
	inline bool IsGroundPosReachable(const POS2D& pos) const
	{
		ASSERT(m_pGroundMap && "no ground map");
		return m_pGroundMap->IsPosReachable(pos);
	}
	inline bool IsGroundPosReachable(int x, int z) const
	{
		ASSERT(m_pGroundMap && "no ground map");
		return m_pGroundMap->IsPosReachable(x, z);
	}
	
	bool CanGroundGoStraightForward(const POS2D& posFrom, const POS2D& posTo) const
	{
		ASSERT(m_pGroundMap && "no ground map");
		return m_pGroundMap->CanGoStraightForward(posFrom, posTo);
	}
	bool CanGroundGoStraightForward(const POS2D& posFrom, const POS2D& posTo, POS2D& posStop) const
	{
		ASSERT(m_pGroundMap && "no ground map");
		return m_pGroundMap->CanGoStraightForward(posFrom, posTo, posStop);
	}
	void GetGroundLastReachablePos(const A3DVECTOR3& vStart, const A3DVECTOR3& vEnd, A3DVECTOR3& vStopPos, float fStepDist = 1.0f /* fSteps must be <= 1.0f */) const
	{
		ASSERT(m_pGroundMap && "no ground map");
		m_pGroundMap->GetLastReachablePos(vStart, vEnd, vStopPos,fStepDist);
	}

	bool GetGroundValid3DPos(A3DVECTOR3& vPos)
	{
		ASSERT(m_pGroundMap && "no ground map");
		return m_pGroundMap->GetValid3DPos(vPos);
	}

	/**
	 * \brief  an interface to get terrain height
	 * \param[in]
	 * \param[out]
	 * \return
	 * \note
	 * \warning
	 * \todo   
	 * \author kuiwu 
	 * \date 11/4/2006
	 * \see 
	 */
	virtual float GetTerrainHeight(float x, float z) = 0;
//	{
//		return 0.0f;
//	}

	void ClampUpTerrain( A3DVECTOR3& vPos, float fMinDistToTerrain)
	{
		float fTerrainY = GetTerrainHeight(vPos.x, vPos.z) + fMinDistToTerrain;
		if(vPos.y < fTerrainY )
			vPos.y = fTerrainY;

	}

	float GetWaterHeight(const A3DVECTOR3& vPos)
	{
		return m_pWaterMap->GetWaterHeight(vPos);
	}

	float GetWaterHeight(float x, float z)
	{
		return m_pWaterMap->GetWaterHeight(A3DVECTOR3(x,0,z));
	}

	bool IsPosPassable(const Pos3DInt& pos, CSPOctreeTravNode& OctrTravNode, const CSPOctreeTravNode * pRefOctrTravNode = NULL,CGlobalSPMap::PtrFuncExtraPosPassableTest pFuncExtraPosPassableTest = NULL)
	{
		return m_pSPMap->IsPosPassable(pos, OctrTravNode, pRefOctrTravNode, pFuncExtraPosPassableTest);	
	}
	
	bool IsPosPassable(const Pos3DInt& pos,CGlobalSPMap::PtrFuncExtraPosPassableTest pFuncExtraPosPassableTest = NULL)
	{
		return m_pSPMap->IsPosPassable(pos, pFuncExtraPosPassableTest);
	}
	
	bool IsPosPassable(const A3DVECTOR3& vPos, CSPOctreeTravNode& OctrTravNode, const CSPOctreeTravNode * pRefOctrTravNode = NULL,CGlobalSPMap::PtrFuncExtraPosPassableTest pFuncExtraPosPassableTest = NULL)
	{
		return m_pSPMap->IsPosPassable(vPos, OctrTravNode, pRefOctrTravNode, pFuncExtraPosPassableTest);
	}

	bool IsPosPassable(const A3DVECTOR3& vPos,CGlobalSPMap::PtrFuncExtraPosPassableTest pFuncExtraPosPassableTest = NULL)
	{
		return m_pSPMap->IsPosPassable(vPos, pFuncExtraPosPassableTest);	
	}
	
	bool CanGoStraightForward(const A3DVECTOR3& vFrom, const A3DVECTOR3& vTo, Pos3DInt& posStop, CGlobalSPMap::PtrFuncExtraPosPassableTest pFuncExtraPosPassableTest = NULL, int * pHitCount = NULL)
	{
		return m_pSPMap->CanGoStraightForward(vFrom, vTo, posStop, pFuncExtraPosPassableTest, pHitCount);
	}
	
	Pos3DInt GetVoxelCenter(const A3DVECTOR3& vPos)
	{
		return m_pSPMap->GetVoxelCenter(vPos);
	}
	
	int GetVoxelSize()
	{
		return m_pSPMap->GetVoxelSize();
	}
	
	void GetLastReachablePos(const A3DVECTOR3& vStart, const A3DVECTOR3& vEnd, A3DVECTOR3& vStopPos, int iEnv)
	{
		return m_pSPMap->GetLastReachablePos(vStart, vEnd, vStopPos, iEnv);
	}
private:
	CNPCMoveMap * m_pGroundMap;
	CGlobalWaterAreaMap * m_pWaterMap;
	CGlobalSPMap * m_pSPMap;
};

}

#endif

