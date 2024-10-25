 /*
 * FILE: NPCMoveMap.h
 *
 * DESCRIPTION:   A  class to realize a map for NPCs' movement, 
 *							which mainly includes two parts: 
 *							1. Reachable map (to tell whether a tile can enter )
 *							2. Delta Height map ( if the tile is reachable, refer to 
 *								this map for the delta height from the terrain surface)
 *
 * CREATED BY: He wenfeng, 2005/4/4
 *
 * HISTORY: 
 *
 * Copyright (c) 2004 Archosaur Studio, All Rights Reserved.
 */

#ifndef _NPCMOVEMAP_H_
#define _NPCMOVEMAP_H_

#define STANDARD_SUBMAP_SIZE 1024

#include "BlockImage.h"
#include "BitImage.h"
#include "a3dvector.h"

class CNPCMoveMap;

/************************************************************************/
// some global function and variables

// here is only a func declaration which must be defined somewhere!
namespace NPCMove{
//float GetTerrainHeight(float x, float z);
};

// void ClampUpTerrain(A3DVECTOR3& vPos, float fMinDistToTerrain = 0.0f);


// class laid under the namespace NPCMoveMap
namespace NPCMoveMap
{

/************************************************************************/

// Data structure for Path Finding
struct POS2D
{
	int u;
	int v;
	POS2D() {}
	POS2D(int iu, int iv) { u = iu; v = iv;}
	bool operator ==(const POS2D& pos) const
	{
		return ( u == pos.u && v == pos.v);
	}
};

class CMap;

// ground move map.  By kuiwu. [11/4/2006]
class CNPCMoveMap
{
public:
	CNPCMoveMap(CMap * pMap) ;
	
	virtual ~CNPCMoveMap();

	void Release();

	void Init(int iWidth, int iLength, int iSubmapWidth=STANDARD_SUBMAP_SIZE, int iSubmapLength=STANDARD_SUBMAP_SIZE, float fPixelSize=1.0f, bool filldefault=true);

	float GetPixelSize() const { return m_fPixelSize; }
	
	void SetWorldOrigin(const A3DVECTOR3& vOrigin)
	{
		m_vOrigin = vOrigin;
	}

	bool SetMapCenterAsOrigin()
	{
		if( m_iSubMapWidth == -1)
			return false;						// no submaps loaded
		else
		{
			m_vOrigin.Clear();
			m_vOrigin.x = m_iSubMapWidth * m_iWidth * m_fPixelSize * 0.5f;
			m_vOrigin.z = m_iSubMapLength * m_iLength * m_fPixelSize * 0.5f;
			return true;
		}		
	}
	
	// Transform an absolute 3D position to the map position
	inline POS2D GetMapPos(const A3DVECTOR3& vPos) const
	{
		POS2D mPos;
		float fPSizeRecip=(m_fPixelSize==1.0f)?1.0f:(1.0f/m_fPixelSize);
		mPos.u = (int) ((vPos.x + m_vOrigin.x) * fPSizeRecip );
		mPos.v = (int) ((vPos.z + m_vOrigin.z) * fPSizeRecip );
		return mPos;
	}
	
	// Return a 3D Pos corresponding to a Map Pos Pixel's center
	// To note: the Y coordinate of returned 3D pos is now set to 0.0f
	inline A3DVECTOR3 GetPixelCenter(const POS2D& mPos) const
	{
		A3DVECTOR3 vPixelCenter(0.0f);
		vPixelCenter.x = ( mPos.u + 0.5f ) * m_fPixelSize - m_vOrigin.x;
		vPixelCenter.z = ( mPos.v + 0.5f ) * m_fPixelSize - m_vOrigin.z;
		return vPixelCenter;
	}

	inline bool IsPosValid(const POS2D& pos) const
	{
		int MapWidth, MapLength;
		if(m_bStandardSubmap)
		{
			MapWidth = m_iWidth << 10;
			MapLength = m_iLength << 10;
		}
		else
		{
			MapWidth = m_iWidth * m_iSubMapWidth;
			MapLength = m_iLength * m_iSubMapLength;
		}
		return ( pos.u >=0 && pos.u < MapWidth && pos.v >=0 && pos.v < MapLength);
	}

	inline bool IsPosReachable(int x, int z) const
	{
		POS2D pos(x, z);
		return IsPosReachable(pos);
	}

	inline bool IsPosReachable(const POS2D& pos) const
	{
		// Revised by wenfeng, 05-9-26
		// we add the code to test whether the pos is valid.
		if(!IsPosValid(pos)) return false;

		int su,sv,u,v;
		if(m_bStandardSubmap)
		{
			su = pos.u >> 10;
			sv = pos.v >> 10;
			u = pos.u & 0x3ff;
			v = pos.v & 0x3ff;
		}
		else
		{
			su = pos.u / m_iSubMapWidth;
			sv = pos.v / m_iSubMapLength;
			u = pos.u % m_iSubMapWidth;
			v = pos.v % m_iSubMapLength;
		}
		
		return m_ReachableMap[ sv * m_iWidth + su]->GetPixel(u,v);
	}
	
	inline bool IsPosNeighborsReachable(const POS2D& pos, int AdjPixels) const
	{
		int u = pos.u - AdjPixels;
		int v = pos.v - AdjPixels;
		int r = 2 * AdjPixels +1;
		for(int i = 0; i<= r; i++)
			for(int j = 0; j<=r; j++)
			{
				POS2D pos(u+i, v+j);
				if(IsPosValid(pos) && IsPosReachable(pos))
					return true;
			}
		return false;
	}

	inline float GetPosDeltaHeight (const POS2D& pos) const 
	{
		// Revised by wenfeng, 05-10-20
		// we add the code to test whether the pos is valid.
		if(!IsPosValid(pos)) return 0;

		int su,sv,u,v;
		if(m_bStandardSubmap)
		{
			su = pos.u >> 10;
			sv = pos.v >> 10;
			u = pos.u & 0x3ff;
			v = pos.v & 0x3ff;
		}
		else
		{
			su = pos.u / m_iSubMapWidth;
			sv = pos.v / m_iSubMapLength;
			u = pos.u % m_iSubMapWidth;
			v = pos.v % m_iSubMapLength;
		}
		FIX16 f16DeltaHeight=m_DeltaHeightMap[ sv * m_iWidth + su]->GetPixel(u, v);
		return FIX16TOFLOAT( f16DeltaHeight );		
	}

	// we assume that vPos is on the surface of the terrain!
	bool GetValid3DPos(A3DVECTOR3& vPos)
	{

		POS2D pos = GetMapPos(vPos);
		if( IsPosReachable(pos))
		{
			vPos.y += GetPosDeltaHeight(pos);
			return true;
		}
		else
			return false;
	}
	
	// ******* new version: we get a pos on the surface of the terrain or building if there exist!
	bool Get3DPosOnGround(A3DVECTOR3& vPos);


	bool Is3DPosReachable(float x, float z)
	{
		POS2D pos = GetMapPos(A3DVECTOR3(x, 0.0f, z));
		return IsPosReachable(pos);
	}

	// Added by wenfeng, 05-9-16
	// A interface try to find the last unblocked pos in a line segment!
	void GetLastReachablePos(const A3DVECTOR3& vStart, const A3DVECTOR3& vEnd, A3DVECTOR3& vStopPos, float fStepDist = 1.0f /* fSteps must be <= 1.0f */);
	
	// test whether we can go straight forward from posFrom to posTo
	bool CanGoStraightForward(const POS2D& posFrom, const POS2D& posTo);

	// similar to the above interface, one more para out, posStop which is 
	// the last reachable 2D pos in the straight line
	bool CanGoStraightForward(const POS2D& posFrom, const POS2D& posTo, POS2D& posStop);
	
	// Load and Save, using FILE
	bool Save( FILE *pFileToSave );
	bool Load( FILE *pFileToLoad );

	bool LoadSubReachableMap(int u, int v, FILE* pFileToLoad);
	bool LoadSubDeltaHeightMap(int u, int v, FILE* pFileToLoad);
	
	// Load data from a directory
	bool Load(const char* szMapPath);
	bool LoadPiece(const char* szMapPath, int piece_idx);
	bool Load(int row, int col, const int * piece_indexes, size_t piece_count, CNPCMoveMap** groundmap_piece);

protected:
	bool m_data_ref;
	CBitImage ** m_ReachableMap;								// set of sub- reachable maps
	CBlockImage<FIX16> ** m_DeltaHeightMap;			// set of sub- deltaheight maps
	
	int m_iWidth;					// sub-maps in width
	int m_iLength;					// sub-maps in length
	
	int m_iSubMapWidth;			// sub-map's width
	int m_iSubMapLength;		// sub_map's length
	
	bool m_bStandardSubmap;	// whether the submap is 1024*1024

	float m_fPixelSize;					// the pixel's size of the map, pixel is the elementary unit of the map!

	A3DVECTOR3 m_vOrigin;	// origin of the world in the map coordinate system

	CMap * m_pMap;   //currently only used to get terrain height
};

//extern CNPCMoveMap g_NPCMoveMap;

}	// end of the namespace

#endif

