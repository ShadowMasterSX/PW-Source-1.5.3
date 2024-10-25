 /*
 * FILE: GlobalWaterAreaMap.h
 *
 * DESCRIPTION:   Describe a global water area map which is composed of some blocks.
 *							Each block is described by CWaterAreaMap. We use singleton pattern to implement it!
 *							
 * CREATED BY: He wenfeng, 2005/5/23
 *
 * HISTORY: 
 *
 * Copyright (c) 2005 Archosaur Studio, All Rights Reserved.
 */

#ifndef _GLOBALWATERAREAMAP_H_
#define _GLOBALWATERAREAMAP_H_

#include "WaterAreaMap.h"

#define SUBMAP(u,v) (*(m_subWaterAreaMaps[v*m_iWidth + u]))

// some global interface in namespace
/*namespace WaterAreaMap
{
	bool LoadData(const char* szPath);
	float GetWaterHeight(const A3DVECTOR3& vPos);
	float GetWaterHeight(float x, float z);
	bool IsUnderWater(const A3DVECTOR3& vPos);
};*/

namespace NPCMoveMap
{
	class CMap;
}

class CGlobalWaterAreaMap
{
public:
	
	void Release()
	{
		if(m_subWaterAreaMaps) 
		{
			if(!m_data_ref)
			{
				for(int i=0; i<m_iWidth * m_iLength; i++)
					delete m_subWaterAreaMaps[i]; 
			}
			delete [] m_subWaterAreaMaps;
			m_subWaterAreaMaps =NULL;
		}
	}
	
	void SetWorldOrigin(const A3DVECTOR3& vOrigin)
	{
		m_vOrigin = vOrigin;
	}
	
	bool SetMapCenterAsOrigin()
	{
		m_vOrigin.Clear();
		m_vOrigin.x = m_fSubMapWidth * m_iWidth * 0.5f;
		m_vOrigin.z = m_fSubMapLength * m_iLength * 0.5f;
		return true;
	}

	float GetWaterHeight(const A3DVECTOR3& vPos)
	{
		// compute the coordinates in the submap
		float fXInSubmap = vPos.x + m_vOrigin.x;
		float fZInSubmap = vPos.z + m_vOrigin.z;
		
		int u = ( int )  (fXInSubmap * m_fRecipSubMapWidth);
		int v = ( int )  (fZInSubmap * m_fRecipSubMapLength);
		
		// add the test whether vPos is beyond the boundary
		if(u < 0 || u >= m_iWidth || v < 0 || v >= m_iLength)
			return NO_WATER;
		
		fXInSubmap -= (u+0.5f ) * m_fSubMapWidth;
		fZInSubmap -= ( v+0.5f ) * m_fSubMapLength;

		/*
		// maybe its' rate is slower than division

		int u=0, v=0;
		while(fXInSubmap>m_fSubMapWidth)
		{
			fXInSubmap-=m_fSubMapWidth;
			u++;
		}
		while(fZInSubmap>m_fSubMapLength)
		{
			fZInSubmap-=m_fSubMapLength;
			v++;
		}
		
		fXInSubmap -= m_fSubMapWidth * 0.5f;
		fZInSubmap -= m_fSubMapLength * 0.5f;

		*/

		return SUBMAP(u,v).GetWaterHeight(fXInSubmap,fZInSubmap);
	}

	bool IsUnderWater(const A3DVECTOR3& vPos)
	{
		return vPos.y < GetWaterHeight(vPos);
	}
	
	// Load data from a directory
	bool Load(const char* szMapPath);
	bool LoadPiece(const char* szMapPath, int piece_idx);
	bool Load(int row, int col, const int * piece_indexes, size_t piece_count, CGlobalWaterAreaMap** watermap_piece);

private:
	
	bool m_data_ref;
	CWaterAreaMap ** m_subWaterAreaMaps;

	int m_iWidth;					// width in sub-maps  
	int m_iLength;					// length in sub-maps  
	
	float m_fSubMapWidth;		// sub-map's width
	float m_fSubMapLength;		// sub_map's length
	
	float m_fRecipSubMapWidth;		// reciprocal of sub-map's width, for speed the computation
	float m_fRecipSubMapLength;		// reciprocal of sub-map's length

	A3DVECTOR3 m_vOrigin;	// origin of the global water map
	
	NPCMoveMap::CMap * m_pMap;
/************************************************************************/
// following code is for singleton pattern

public:
	//static CGlobalWaterAreaMap* GetInstance();
	//static void ReleaseInstance();

	CGlobalWaterAreaMap(NPCMoveMap::CMap * pMap):m_pMap(pMap) { m_data_ref = false; m_subWaterAreaMaps=NULL;}
	~CGlobalWaterAreaMap() {  Release(); }

private:
	// make the following member function private so that 
	// the class can not be instantiated outside

	// copy constructor
	CGlobalWaterAreaMap(const CGlobalWaterAreaMap& );
	// assignment operator
	CGlobalWaterAreaMap& operator=(const CGlobalWaterAreaMap& );

private:
	//static CGlobalWaterAreaMap* s_Instance;

/************************************************************************/

};


#endif
