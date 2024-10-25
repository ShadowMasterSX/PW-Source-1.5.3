 /*
 * FILE: GlobalSPMap.cpp
 *
 * DESCRIPTION:   Describe a global space passable map which is composed of some blocks(that is submaps).
 *				  Each block is described by CCompactSpacePassableOctree. We use singleton pattern to implement it!
 *							
 * CREATED BY: He wenfeng, 2005/9/29
 *
 * HISTORY: 
 *
 * Copyright (c) 2005 Archosaur Studio, All Rights Reserved.
 */

#include "GlobalSPMap.h"
#include <string.h>
#include "NPCChaseSpatiallyPFAgent.h"

// #include "GlobalSPMapHash.h"

#define STANDARD_SUBMAP_SIZE 1024

// --------------------------------------------------------
// The following code is for singleton pattern!

// Init the global instance
//CGlobalSPMap* CGlobalSPMap::s_GlobalSPMapInstance = NULL;


// Define a class which is only used to call ReleaseInstance()
// After defined a global instance of this class, we need not call
// CGlobalSPMap::ReleaseInstance() after we use it!
/*class CDestructGlobalSPMap
{
public:
	~CDestructGlobalSPMap()
	{
		CGlobalSPMap::ReleaseInstance();
	}
};

static CDestructGlobalSPMap DestructGlobalSPMap;*/

// --------------------------------------------------------

void CGlobalSPMap::SetFreeSPMap(int row, int col, int MapID,const Pos3DInt& posGlobalOrigin)
{
	CCompactSpacePassableOctree* pSPMap = GetSPMap(row,col);
	if(!pSPMap) return;

	Pos3DInt posSPMapCenter;
	int iHalfMapSize = m_iSubMapSize >> 1;
	posSPMapCenter.x = col * m_iSubMapSize + iHalfMapSize - posGlobalOrigin.x;
	posSPMapCenter.y = iHalfMapSize;
	posSPMapCenter.z = row * m_iSubMapSize + iHalfMapSize - posGlobalOrigin.z;

	pSPMap->InitFree(posSPMapCenter, m_iSubMapSize, m_iVoxelSize,(SPOctree::UCHAR)MapID);
}

bool CGlobalSPMap::Load(const char* szMapPath)
{
	// Firstly, we read the config file.
	char szPathWithSlash[256];
	char szMapParaPath[256];
	char szSubMapPath[256];

	size_t ilen = strlen(szMapPath);
	if(ilen ==0 || ilen >= sizeof(szPathWithSlash)) return false;

	strcpy(szPathWithSlash, szMapPath);
	if(szMapPath[ilen-1]!='\\' && szMapPath[ilen-1]!='/')
		strcat(szPathWithSlash, "/");
	
	// Read Map's parameter from the file MapPara.ini
	strcpy(szMapParaPath, szPathWithSlash);
	strcat(szMapParaPath, "spmap.conf");
	
	FILE* fMapPara = fopen(szMapParaPath, "rt");
	if(!fMapPara) return false;
	// Need some error control!
	fscanf(fMapPara, "Map Width =%d\n", &m_iWidth);
	fscanf(fMapPara, "Map Length =%d\n", &m_iLength);
	fscanf(fMapPara, "Submap Size =%d\n", &m_iSubMapSize);
	fscanf(fMapPara, "Voxel Size =%d\n", &m_iVoxelSize);
	fclose(fMapPara);
	
	m_bStandardSubmap=(m_iSubMapSize == STANDARD_SUBMAP_SIZE);
	m_fRecipSubMapSize = 1.0f / m_iSubMapSize;
	m_fRecipVoxelSize = 1.0f / m_iVoxelSize;

	// compute the global origin (refer to the left-bottom!)
	Pos3DInt posGlobalOrigin;
	posGlobalOrigin.x = (m_iSubMapSize * m_iWidth) >> 1;
	posGlobalOrigin.y = 0;
	posGlobalOrigin.z = (m_iSubMapSize * m_iLength) >> 1;

	m_SPMaps = new  CCompactSpacePassableOctree[ m_iWidth * m_iLength ];

	// Then, we read the submap(Octree file) in the following order
	/*  Assume the global map is conmposed of 8 (m_iWidth) * 11 (m_iLength)
	 *
	 *	80 81 82... 87
	 *    .......
	 *    .......
	 *  8 9 10 ... 15
	 *  0 1 2  ... 7
	 */

	FILE *fSPMap;
	int iFileID;
	char szFileName[20];
	for(int i=0; i< m_iLength; i++)				// i is corresponding to row
		for(int j=0; j<m_iWidth; j++)			// j is corresponding to col
		{
			iFileID = (m_iLength-i-1)*m_iWidth+j+1;
			sprintf(szFileName, "%d.octr", iFileID);
			strcpy(szSubMapPath, szPathWithSlash);
			strcat(szSubMapPath, szFileName);

			fSPMap = fopen(szSubMapPath, "rb");
			
			if(fSPMap)
			{
				CCompactSpacePassableOctree* pCurSPMap = GetSPMap(i,j);
				
				// ******* Revised by wenfeng, 05-10-14
				// If loading fails, we set current SPMap to free!
				if(!pCurSPMap->Load(fSPMap) || pCurSPMap->GetSize() != m_iSubMapSize )
				{
					SetFreeSPMap(i,j,iFileID,posGlobalOrigin);
				}
				
				fclose(fSPMap);
			}
			else
			{
				// ******* Revised by wenfeng, 05-10-14
				// If loading fails, we set current SPMap to free!
				SetFreeSPMap(i,j,iFileID,posGlobalOrigin);
			}

		}
	

	return true;
}

bool CGlobalSPMap::LoadFreePiece(const char* szMapPath)
{
	// Firstly, we read the config file.
	char szPathWithSlash[256];
	char szMapParaPath[256];
	//char szSubMapPath[256];

	size_t ilen = strlen(szMapPath);
	if(ilen ==0 || ilen >= sizeof(szPathWithSlash)) return false;

	strcpy(szPathWithSlash, szMapPath);
	if(szMapPath[ilen-1]!='\\' && szMapPath[ilen-1]!='/')
		strcat(szPathWithSlash, "/");
	
	// Read Map's parameter from the file MapPara.ini
	strcpy(szMapParaPath, szPathWithSlash);
	strcat(szMapParaPath, "spmap.conf");
	
	FILE* fMapPara = fopen(szMapParaPath, "rt");
	if(!fMapPara) return false;
	// Need some error control!
	m_iWidth = 1;
	m_iLength = 1;
	int tmp;
	fscanf(fMapPara, "Map Width =%d\n", &tmp);
	fscanf(fMapPara, "Map Length =%d\n", &tmp);
	fscanf(fMapPara, "Submap Size =%d\n", &m_iSubMapSize);
	fscanf(fMapPara, "Voxel Size =%d\n", &m_iVoxelSize);
	fclose(fMapPara);
	
	m_bStandardSubmap=(m_iSubMapSize == STANDARD_SUBMAP_SIZE);
	m_fRecipSubMapSize = 1.0f / m_iSubMapSize;
	m_fRecipVoxelSize = 1.0f / m_iVoxelSize;

	// compute the global origin (refer to the left-bottom!)
	Pos3DInt posGlobalOrigin;
	posGlobalOrigin.x = (m_iSubMapSize * m_iWidth) >> 1;
	posGlobalOrigin.y = 0;
	posGlobalOrigin.z = (m_iSubMapSize * m_iLength) >> 1;

	m_SPMaps = new  CCompactSpacePassableOctree[ m_iWidth * m_iLength ];

	// Then, we read the submap(Octree file) in the following order
	/*  Assume the global map is conmposed of 8 (m_iWidth) * 11 (m_iLength)
	 *
	 *	80 81 82... 87
	 *    .......
	 *    .......
	 *  8 9 10 ... 15
	 *  0 1 2  ... 7
	 */

	SetFreeSPMap(0,0,1,posGlobalOrigin);
	

	return true;
}

bool CGlobalSPMap::SetFreeMap(int row, int col, CGlobalSPMap * spmap_template)
{
	// Need some error control!
	m_iWidth = col;
	m_iLength = row;
	m_iSubMapSize = spmap_template->m_iSubMapSize;
	m_iVoxelSize = spmap_template->m_iVoxelSize;
	
	m_bStandardSubmap=(m_iSubMapSize == STANDARD_SUBMAP_SIZE);
	m_fRecipSubMapSize = 1.0f / m_iSubMapSize;
	m_fRecipVoxelSize = 1.0f / m_iVoxelSize;

	// compute the global origin (refer to the left-bottom!)
	Pos3DInt posGlobalOrigin;
	posGlobalOrigin.x = (m_iSubMapSize * m_iWidth) >> 1;
	posGlobalOrigin.y = 0;
	posGlobalOrigin.z = (m_iSubMapSize * m_iLength) >> 1;

	m_SPMaps = new  CCompactSpacePassableOctree[ m_iWidth * m_iLength ];

	// Then, we read the submap(Octree file) in the following order
	/*  Assume the global map is conmposed of 8 (m_iWidth) * 11 (m_iLength)
	 *
	 *	80 81 82... 87
	 *    .......
	 *    .......
	 *  8 9 10 ... 15
	 *  0 1 2  ... 7
	 */

	int iFileID;
	for(int i=0; i< m_iLength; i++)				// i is corresponding to row
		for(int j=0; j<m_iWidth; j++)			// j is corresponding to col
		{
			iFileID = (m_iLength-i-1)*m_iWidth+j+1;
			SetFreeSPMap(i,j,iFileID,posGlobalOrigin);
		}
	

	return true;
}
	

//////////////////////////////////////////////////////////////////////////
// Revised by wenfeng, 05-10-29
//		Since we add pFuncExtraPosPassableTest for each position on the 
// straight line, when we confirm that current pos lies in the same node
// of the goal, we can't make sure that the line is unblocked.
//////////////////////////////////////////////////////////////////////////
bool CGlobalSPMap::CanGoStraightForward(const A3DVECTOR3& vFrom, const A3DVECTOR3& vTo, Pos3DInt& posStop,PtrFuncExtraPosPassableTest pFuncExtraPosPassableTest,int * pHitCount )
{
	CSPOctreeTravNode FromOctrTravNode, ToOctrTravNode;
	Pos3DInt posFrom,posTo;
	
	// The start pos is not passable
	posFrom = GetVoxelCenter(vFrom);
	if(! IsPosPassable(posFrom, FromOctrTravNode,NULL,pFuncExtraPosPassableTest))
	{
		posStop = posFrom;
		return false;
	}

	// The goal pos is passable
	posTo = GetVoxelCenter(vTo);
	bool bGoalPassable =IsPosPassable(posTo, ToOctrTravNode, &FromOctrTravNode, pFuncExtraPosPassableTest);
	if(bGoalPassable && !pFuncExtraPosPassableTest)
	{
		// when pFuncExtraPosPassableTest == NULL, we can use the following code to 
		// speed this function. 
		if(ToOctrTravNode== FromOctrTravNode ||
			ToOctrTravNode.IsNodeNeighborSibling(FromOctrTravNode))
		{
			posStop = posTo;
			if(pHitCount)
				(*pHitCount) ++;
			return true;
		}
		
	}

	// now we start 3D DDA alogrithm to find a straight line!
	float dx,dy,dz,adx,ady,adz,maxd;
	dx = vTo.x - vFrom.x;
	dy = vTo.y - vFrom.y;
	dz = vTo.z - vFrom.z;
	adx = (float)fabs(dx);
	ady = (float)fabs(dy);
	adz = (float)fabs(dz);
	maxd = (adx>ady)?adx:ady;
	maxd = (maxd>adz)?maxd:adz;	
	
	dx /= maxd;
	dy /= maxd;
	dz /= maxd;

	A3DVECTOR3 vCur(vFrom);
	Pos3DInt posCur(posFrom),posLast;

	int iLoops =(int)maxd;
	CSPOctreeTravNode CurOctrTravNode,RefOctrTravNode(FromOctrTravNode);
	for(int i=0;i<iLoops;++i)
	{
		vCur.x += dx;
		vCur.y += dy;
		vCur.z += dz;
		posLast = posCur;
		posCur = GetVoxelCenter(vCur);
		if(bGoalPassable && !pFuncExtraPosPassableTest && ToOctrTravNode.IsPosInside(posCur))
		{
			posStop = posTo;
			return true;
		}
		if(IsPosPassable(posCur,CurOctrTravNode,&RefOctrTravNode,pFuncExtraPosPassableTest))
		{
			RefOctrTravNode = CurOctrTravNode;
		}
		else
		{
			posStop = posLast;
			return false;
		}
	}
	
	if(bGoalPassable)
	{
		posStop = posTo;
	}
	else
	{
		posStop = posCur;
	}

	return bGoalPassable;

}

//////////////////////////////////////////////////////////////////////////
// Added by wenfeng, 05-12-1
// push the CHBorder pos to CGlobalSPMapHash
// called by CCompactSpacePassableOctree's Traverse interface
/*
static void PushCHBorderPos2GSPMapHash(void *pPara, const CubeInt& Cube, SPOctree::UCHAR state)
{
	CGlobalSPMapHash* pGlobalSPMapHash = (CGlobalSPMapHash*)pPara;
	if(state == CCompactSpacePassableOctree::CHBorder)
	{
		pGlobalSPMapHash->AddBorderVoxel(A3DVECTOR3(Cube.Center.x, Cube.Center.y, Cube.Center.z));
	}
}

//////////////////////////////////////////////////////////////////////////
// Added by wenfeng, 05-12-1
// traverse the each CHBorder pos and export to pGlobalSPMapHash!
// using PushCHBorderPos2GSPMapHash and CCompactSpacePassableOctree's Traverse method
void CGlobalSPMap::Export(CGlobalSPMapHash* pGlobalSPMapHash)
{
	if(!m_SPMaps) return;		// no data
	
	// for each submap
	for(int row = 0; row < m_iLength; row++)
		for(int col = 0; col < m_iWidth; col++)
		{
			GetSPMap(row,col)->Traverse(PushCHBorderPos2GSPMapHash,(void *)pGlobalSPMapHash);
		}
}
*/

void CGlobalSPMap::GetLastReachablePos(const A3DVECTOR3& vStart, const A3DVECTOR3& vEnd, A3DVECTOR3& vStopPos, int iEnv)
{
	Pos3DInt posStop;
	if(iEnv == Env_OnAir)
	{
		CanGoStraightForward(vStart,vEnd,posStop,CNPCChaseSpatiallyPFAgent::IsPosOnAir);
	}
	else if(iEnv == Env_UnderWater)
	{
		CanGoStraightForward(vStart,vEnd,posStop,CNPCChaseSpatiallyPFAgent::IsPosInWater);
	}
	vStopPos.Set(posStop.x, posStop.y,posStop.z);
}

/*

// No implement! So, when you call this interface, it'll present you a compile error.

bool CGlobalSPMap::CanGoStraightForward(const Pos3DInt& posFrom, const Pos3DInt& posTo, Pos3DInt& posStop)
{
	return false;
}
*/
