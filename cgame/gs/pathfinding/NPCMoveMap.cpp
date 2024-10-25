 /*
 * FILE: NPCMoveMap.cpp
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

#include "NPCMoveMap.h"
#include "CMap.h"

#include <string.h>

#define SIGN(x) ( (x>0)? 1 : ((x<0)?-1:0) )
#ifndef ABS
	#define ABS(x) ((x>0)?x:(-x))
#endif

/************************************************************************/

// Version Info (DWORD)

// Current Version
#define NPCMOVEMAP_VER 0x00000001	

// Old Version


/************************************************************************

  Global functions' implement!

//************************************************************************/
//void ClampUpTerrain(A3DVECTOR3& vPos, float fMinDistToTerrain)
//{
//	float fTerrainY = NPCMove::GetTerrainHeight(vPos.x, vPos.z) + fMinDistToTerrain;
//	if(vPos.y < fTerrainY )
//		vPos.y = fTerrainY;
//}

namespace NPCMoveMap
{
	
// the CNPCMoveMap class has one global instance!
//CNPCMoveMap g_NPCMoveMap;

CNPCMoveMap::CNPCMoveMap(CMap * pMap) 
{ 
	m_data_ref = false;
	m_ReachableMap = NULL; 
	m_DeltaHeightMap = NULL;

	m_pMap = pMap;
}

CNPCMoveMap::~CNPCMoveMap() 
{ 
	Release(); 
}



void CNPCMoveMap::Release()
{
	if(m_ReachableMap)
	{
		if(!m_data_ref)
		{
			for(int i=0; i<m_iWidth*m_iLength; i++)
				delete m_ReachableMap[i];
		}
		delete [] m_ReachableMap;
		m_ReachableMap = NULL;
	}
	if(m_DeltaHeightMap)
	{
		if(!m_data_ref)
		{
			for(int i=0; i<m_iWidth*m_iLength; i++)
				delete m_DeltaHeightMap[i];
		}
		delete [] m_DeltaHeightMap;
		m_DeltaHeightMap = NULL;
	}
}

void CNPCMoveMap::Init(int iWidth, int iLength, int iSubmapWidth, int iSubmapLength, float fPixelSize, bool filldefault)
{
	m_iWidth = iWidth;
	m_iLength = iLength;
	
	m_iSubMapWidth = iSubmapWidth;
	m_iSubMapLength = iSubmapLength;
	m_fPixelSize = fPixelSize;

	m_bStandardSubmap=(m_iSubMapWidth == STANDARD_SUBMAP_SIZE && m_iSubMapLength == STANDARD_SUBMAP_SIZE);

	//By default, we set the map center as origin!
	SetMapCenterAsOrigin();
	
	if(!filldefault) return;

	int iSize = iWidth * iLength;
	m_ReachableMap = new CBitImage * [iSize];
	for(int i=0; i<iSize; i++)
		m_ReachableMap[i] = new CBitImage;
	m_DeltaHeightMap = new CBlockImage<FIX16> * [iSize];
	for(int i=0; i<iSize; i++)
		m_DeltaHeightMap[i] = new CBlockImage<FIX16>;
	// Init each submap to an everywhere reachable submap
	for(int i=0; i< iSize; i++)
	{
		m_ReachableMap[i]->InitNoneZero(m_iSubMapWidth, m_iSubMapLength, fPixelSize);
		m_DeltaHeightMap[i]->InitZero(m_iSubMapWidth, m_iSubMapLength, fPixelSize);
	}
}

bool CNPCMoveMap::LoadSubReachableMap(int u, int v, FILE* pFileToLoad)
{
	if( u >= m_iWidth || u < 0 || v>= m_iLength || v < 0 )
		return false;
	
	int iSubMapID = v*m_iWidth + u;

	if(!m_ReachableMap[ iSubMapID ]->Load(pFileToLoad))
		return false;

	if(m_iSubMapWidth == -1)
	{
		m_ReachableMap[ iSubMapID ]->GetImageSize(m_iSubMapWidth, m_iSubMapLength);
		m_fPixelSize = m_ReachableMap[ iSubMapID ]->GetPixelSize();
		m_bStandardSubmap = ( m_iSubMapWidth==STANDARD_SUBMAP_SIZE && m_iSubMapLength== STANDARD_SUBMAP_SIZE );
		return true;
	}
	else
	{
		int w, l;
		m_ReachableMap[ iSubMapID ]->GetImageSize(w, l);
		if( w != m_iSubMapWidth || l != m_iSubMapLength || m_fPixelSize!=m_ReachableMap[ iSubMapID ]->GetPixelSize() )
			return false;
		else
			return true;
	}

}

bool CNPCMoveMap::Get3DPosOnGround(A3DVECTOR3& vPos)
{
	//vPos.y = NPCMove::GetTerrainHeight(vPos.x,vPos.z);
	vPos.y = m_pMap->GetTerrainHeight(vPos.x,vPos.z);
	POS2D pos = GetMapPos(vPos);
	if( IsPosReachable(pos))
	{
		vPos.y += GetPosDeltaHeight(pos);
		return true;
	}
	else
		return false;
}


bool CNPCMoveMap::LoadSubDeltaHeightMap(int u, int v, FILE* pFileToLoad)
{
	if( u >= m_iWidth || u < 0 || v>= m_iLength || v < 0 )
		return false;
	
	int iSubMapID = v*m_iWidth + u;

	if(!m_DeltaHeightMap[iSubMapID ]->Load(pFileToLoad))
		return false;

	if(m_iSubMapWidth == -1)
	{
		m_DeltaHeightMap[ iSubMapID ]->GetImageSize(m_iSubMapWidth, m_iSubMapLength);
		m_fPixelSize = m_DeltaHeightMap[ iSubMapID ]->GetPixelSize();
		m_bStandardSubmap = ( m_iSubMapWidth==STANDARD_SUBMAP_SIZE && m_iSubMapLength== STANDARD_SUBMAP_SIZE );
		return true;
	}
	else
	{
		int w, l;
		m_DeltaHeightMap[ iSubMapID ]->GetImageSize(w, l);
		if( w != m_iSubMapWidth || l != m_iSubMapLength || m_fPixelSize!=m_ReachableMap[ iSubMapID ]->GetPixelSize())
			return false;
		else
			return true;
	}
}

bool CNPCMoveMap::Save( FILE *pFileToSave )
{
	if(!pFileToSave) return false;
	
	DWORD dwWrite, dwWriteLen;
	
	// write the Version info
	dwWrite=NPCMOVEMAP_VER;
	dwWriteLen = fwrite(&dwWrite, 1, sizeof(DWORD), pFileToSave);
	if(dwWriteLen != sizeof(DWORD))
		return false;

	// write the buf size. To note is this buf size isn't include m_ReachableMap and m_DeltaHeightMap data
	DWORD BufSize= 4*sizeof(int)+4*sizeof(float) ;
	dwWrite= BufSize;
	dwWriteLen = fwrite(&dwWrite, 1, sizeof(DWORD), pFileToSave);
	if(dwWriteLen != sizeof(DWORD))
		return false;
	
	// write the following info in order
	// 1. m_iWidth
	// 2. m_iLength
	// 3. m_iSubMapWidth
	// 4. m_iSubMapLength
	// 5. m_fPixelSize
	// 6. m_vOrigin
	
	UCHAR *buf=new UCHAR[BufSize];
	int cur=0;
	* (int *) (buf+cur) = m_iWidth;
	cur+=sizeof(int);
	
	* (int *) (buf+cur) = m_iLength;
	cur+=sizeof(int);

	* (int *) (buf+cur) = m_iSubMapWidth;
	cur+=sizeof(int);

	* (int *) (buf+cur) = m_iSubMapLength;
	cur+=sizeof(int);

	* (float *) (buf+cur) = m_fPixelSize;
	cur+=sizeof(float);	

	* (float *) (buf+cur) = m_vOrigin.x;
	cur+=sizeof(float);	
	* (float *) (buf+cur) = m_vOrigin.y;
	cur+=sizeof(float);	
	* (float *) (buf+cur) = m_vOrigin.z;
	cur+=sizeof(float);	

	dwWriteLen = fwrite(buf, 1, BufSize, pFileToSave);
	if(dwWriteLen != BufSize)
	{
		delete [] buf;
		return false;
	}
	
	// write the map data : m_ReachableMap and m_DeltaHeightMap
	for(int i=0; i< m_iWidth * m_iLength; i++)
	{
		if(!m_ReachableMap[i]->Save(pFileToSave) || ! m_DeltaHeightMap[i]->Save(pFileToSave))
			return false;
	}
	
	return true;
	
}

bool CNPCMoveMap::Load( FILE *pFileToLoad )
{
	if(!pFileToLoad) return false;
	
	DWORD dwRead, dwReadLen;

	// Read the Version
	dwReadLen = fread(&dwRead, 1, sizeof(DWORD), pFileToLoad);
	if(dwReadLen != sizeof(DWORD))
		return false;

	if(dwRead==NPCMOVEMAP_VER)
	{
		// Current Version
		
		// Read the buf size
		dwReadLen = fread(&dwRead, 1, sizeof(DWORD), pFileToLoad);
		if(dwReadLen != sizeof(DWORD))
			return false;
		int BufSize=dwRead;
		UCHAR * buf = new UCHAR[BufSize];

		// Read the data
		dwReadLen = fread(buf, 1, BufSize, pFileToLoad);
		if( dwReadLen != (DWORD)BufSize )
		{
			delete [] buf;
			return false;
		}
			
		Release();			// Release the old data

		int cur=0;
		m_iWidth=* (int *) (buf+cur);
		cur+=sizeof(int);
		m_iLength=* (int *) (buf+cur);
		cur+=sizeof(int);
		
		m_iSubMapWidth=* (int *) (buf+cur);
		cur+=sizeof(int);
		m_iSubMapLength=* (int *) (buf+cur);
		cur+=sizeof(int);

		m_fPixelSize=* (float *) (buf+cur);
		cur+=sizeof(float);

		m_vOrigin.x=* (float *) (buf+cur);
		cur+=sizeof(float);
		m_vOrigin.y=* (float *) (buf+cur);
		cur+=sizeof(float);
		m_vOrigin.z=* (float *) (buf+cur);
		cur+=sizeof(float);
		
		delete [] buf;

		int iSize = m_iWidth * m_iLength;
		m_ReachableMap = new CBitImage * [iSize];
		for(int i=0; i<iSize; i++)
			m_ReachableMap[i] = new CBitImage;
		m_DeltaHeightMap = new CBlockImage<FIX16> * [iSize];
		for(int i=0; i<iSize; i++)
			m_DeltaHeightMap[i] = new CBlockImage<FIX16>;

		for(int i = 0; i < iSize; i++)
		{
			if( !m_ReachableMap[i]->Load(pFileToLoad) || !m_DeltaHeightMap[i]->Load(pFileToLoad))
				return false;
		}
	
		return true;
	}		// Version is current version!
	else
		return false;
	
}

bool CNPCMoveMap::Load(const char* szMapPath)
{
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
	strcat(szMapParaPath, "movemap.conf");
	
	FILE* fMapPara = fopen(szMapParaPath, "rt");
	if(!fMapPara) return false;
	int iWidth, iLength, iSubmapWidth, iSubmapLength;
	float fPixelSize;
	// Need some error control!
	fscanf(fMapPara, "Map Width =%d\n", &iWidth);
	fscanf(fMapPara, "Map Length =%d\n", &iLength);
	fscanf(fMapPara, "Submap Width =%d\n", &iSubmapWidth);
	fscanf(fMapPara, "Submap Length =%d\n", &iSubmapLength);
	fscanf(fMapPara, "Pixel Size =%f\n", &fPixelSize);
	fclose(fMapPara);

	Init(iWidth, iLength, iSubmapWidth, iSubmapLength, fPixelSize);
	
	// now, we start reading the submaps' data
	FILE *fRMap, *fDHMap;
	int iFileID;
	char szFileName[20];
	for(int i=0; i< iLength; i++)
		for(int j=0; j<iWidth; j++)
		{
			iFileID = (iLength-i-1)*iWidth+j+1;
			// Reachable Map
			sprintf(szFileName, "%d.rmap", iFileID);
			strcpy(szSubMapPath, szPathWithSlash);
			strcat(szSubMapPath, szFileName);
			fRMap = fopen(szSubMapPath, "rb");

			// Delta Height Map
			sprintf(szFileName, "%d.dhmap", iFileID);
			strcpy(szSubMapPath, szPathWithSlash);
			strcat(szSubMapPath, szFileName);
			fDHMap = fopen(szSubMapPath, "rb");
			
			if(fRMap && fDHMap)
			{
				if (!LoadSubReachableMap(j, i, fRMap) ||
					!LoadSubDeltaHeightMap(j, i, fDHMap))
				{
					// Load Map failed, we still set the map to default state
					m_ReachableMap[i*iWidth+j]->InitNoneZero(iSubmapWidth, iSubmapLength);
					m_DeltaHeightMap[i*iWidth+j]->InitZero(iSubmapWidth, iSubmapLength);
				}
			}

			// close the opened files
			if(fRMap) fclose(fRMap);
			if(fDHMap) fclose(fDHMap);
		}
	
	return true;

}

bool CNPCMoveMap::LoadPiece(const char* szMapPath, int piece_idx)
{
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
	strcat(szMapParaPath, "movemap.conf");
	
	FILE* fMapPara = fopen(szMapParaPath, "rt");
	if(!fMapPara) return false;
	int iWidth, iLength, iSubmapWidth, iSubmapLength;
	float fPixelSize;
	// Need some error control!
	iWidth = 1;
	iLength = 1;
	int tmp;
	fscanf(fMapPara, "Map Width =%d\n", &tmp);
	fscanf(fMapPara, "Map Length =%d\n", &tmp);
	fscanf(fMapPara, "Submap Width =%d\n", &iSubmapWidth);
	fscanf(fMapPara, "Submap Length =%d\n", &iSubmapLength);
	fscanf(fMapPara, "Pixel Size =%f\n", &fPixelSize);
	fclose(fMapPara);

	Init(iWidth, iLength, iSubmapWidth, iSubmapLength, fPixelSize);
	
	// now, we start reading the submaps' data
	FILE *fRMap, *fDHMap;
	int iFileID;
	char szFileName[20];

	iFileID = piece_idx + 1;
	// Reachable Map
	sprintf(szFileName, "%d.rmap", iFileID);
	strcpy(szSubMapPath, szPathWithSlash);
	strcat(szSubMapPath, szFileName);
	fRMap = fopen(szSubMapPath, "rb");

	// Delta Height Map
	sprintf(szFileName, "%d.dhmap", iFileID);
	strcpy(szSubMapPath, szPathWithSlash);
	strcat(szSubMapPath, szFileName);
	fDHMap = fopen(szSubMapPath, "rb");

	if(fRMap && fDHMap)
	{
		if (!LoadSubReachableMap(0, 0, fRMap) ||
				!LoadSubDeltaHeightMap(0, 0, fDHMap))
		{
			// Load Map failed, we still set the map to default state
			m_ReachableMap[0]->InitNoneZero(iSubmapWidth, iSubmapLength);
			m_DeltaHeightMap[0]->InitZero(iSubmapWidth, iSubmapLength);
		}
	}

	// close the opened files
	if(fRMap) fclose(fRMap);
	if(fDHMap) fclose(fDHMap);
	
	return true;

}

bool CNPCMoveMap::Load(int row, int col, const int * piece_indexes, size_t piece_count, CNPCMoveMap** groundmap_piece)
{
	int iWidth, iLength, iSubmapWidth, iSubmapLength;
	float fPixelSize;
	// Need some error control!
	iWidth = col;
	iLength = row;
	iSubmapWidth = groundmap_piece[0]->m_iSubMapWidth;
	iSubmapLength = groundmap_piece[0]->m_iSubMapLength;
	fPixelSize = groundmap_piece[0]->m_fPixelSize;

	Init(iWidth, iLength, iSubmapWidth, iSubmapLength, fPixelSize, false);
	
	int iSize = iWidth * iLength;
	m_ReachableMap = new CBitImage * [iSize];
	m_DeltaHeightMap = new CBlockImage<FIX16> * [iSize];

	// now, we start reading the submaps' data
	for(int i=0; i< iLength; i++)
	{
		for(int j=0; j<iWidth; j++)
		{
			//iFileID = (iLength-i-1)*iWidth+j+1;
			m_ReachableMap[i * iLength + j] = groundmap_piece[piece_indexes[(iLength-i-1) * iWidth + j]]->m_ReachableMap[0];
			m_DeltaHeightMap[i * iLength + j] = groundmap_piece[piece_indexes[(iLength-i-1) * iWidth + j]]->m_DeltaHeightMap[0]; 
		}
	}
	
	m_data_ref = true;
	
	return true;
}

void CNPCMoveMap::GetLastReachablePos(const A3DVECTOR3& vStart, const A3DVECTOR3& vEnd, A3DVECTOR3& vStopPos, float fStepDist)
{
	float fDeltaX = vEnd.x - vStart.x;
	float fDeltaZ = vEnd.z - vStart.z;
	float fAbsDeltaX = fabs(fDeltaX);
	float fAbsDeltaZ = fabs(fDeltaZ);

	float fStepX, fStepZ;
	if(fAbsDeltaX > fAbsDeltaZ)
	{
		fStepX = fStepDist;
		fStepZ = fStepDist * fAbsDeltaZ / fAbsDeltaX;
	}
	else
	{
		fStepZ = fStepDist;
		fStepX = fStepDist * fAbsDeltaX / fAbsDeltaZ;
	}
	
	if(fDeltaX < 0) fStepX = - fStepX;
	if(fDeltaZ < 0) fStepZ = - fStepZ;

	A3DVECTOR3 vCurPos(vStart),vLastPos(vStart);
	
	bool bStopAtEnd = true;

	if(fDeltaX > 0)
	{
		while(vCurPos.x < vEnd.x)
		{
			if(!Is3DPosReachable(vCurPos.x, vCurPos.z))
			{
				bStopAtEnd = false;
				break;
			}
			
			vLastPos= vCurPos;

			vCurPos.x += fStepX;
			vCurPos.z += fStepZ;
		}
	}
	else
	{
		while(vCurPos.x > vEnd.x)
		{
			if(!Is3DPosReachable(vCurPos.x, vCurPos.z))
			{
				bStopAtEnd = false;
				break;
			}
			
			vLastPos= vCurPos;

			vCurPos.x += fStepX;
			vCurPos.z += fStepZ;
		}

	}
	
	if(bStopAtEnd)
		vStopPos = vEnd;
	else
		vStopPos = vLastPos;

	Get3DPosOnGround(vStopPos);
}


// using the Integer-Bresenham Line-Raster Algorithm
bool CNPCMoveMap::CanGoStraightForward(const POS2D& posFrom, const POS2D& posTo, POS2D& posStop)
{
	int x,y,dx,dy,s1,s2;	
	
	x = posFrom.u;
	y = posFrom.v;
	dx = posTo.u - posFrom.u;
	dy = posTo.v - posFrom.v;
	s1 = SIGN(dx);
	s2 = SIGN(dy);
	dx = ABS(dx);
	dy = ABS(dy);

	// Interchange ?
	bool bInterchange = false;
	if( dy > dx )
	{
		int tmp = dx;
		dx = dy;
		dy = tmp;
		bInterchange = true;
	}

	// Init the error term
	int e = 2*dy-dx;

	// Start loop, now dx >= dy
	POS2D pos = posFrom;
	for(int i=0; i<dx; i++)
	{
		// save the last reachable pos
		posStop = pos;

		pos.u = x;
		pos.v = y;
		if( !IsPosReachable(pos) )			
			return false;			// The pixel can't be reached. So we return false.
		while( e > 0 )
		{
			if(bInterchange)
				x += s1;
			else
				y += s2;

			e -= 2*dx;
		}

		if(bInterchange)
			y += s2;
		else
			x += s1;

		e += 2*dy;
	}

	return true;

}

bool CNPCMoveMap::CanGoStraightForward(const POS2D& posFrom, const POS2D& posTo)
{
	POS2D posStop;
	
	return CanGoStraightForward(posFrom, posTo, posStop);
}

}	// end of the namespace
