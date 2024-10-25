    /*
 * FILE: GlobalWaterAreaMap.cpp
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

#include "GlobalWaterAreaMap.h"

#include <string.h>

/*namespace WaterAreaMap
{
	bool LoadData(const char* szPath)
	{
		return CGlobalWaterAreaMap::GetInstance()->Load(szPath);
	}
	
	float GetWaterHeight(const A3DVECTOR3& vPos)
	{
		return CGlobalWaterAreaMap::GetInstance()->GetWaterHeight(vPos);
	}
	
	float GetWaterHeight(float x, float z)
	{
		return CGlobalWaterAreaMap::GetInstance()->GetWaterHeight(A3DVECTOR3(x,0,z));
	}

	bool IsUnderWater(const A3DVECTOR3& vPos)
	{
		return CGlobalWaterAreaMap::GetInstance()->IsUnderWater(vPos);
	}
};*/

/************************************************************************/
// following code is for singleton pattern

// define a class which is only used to call ReleaseInstance()
/*class CDestructGlobalWaterAreaMap
{
public:
	~CDestructGlobalWaterAreaMap()
	{
		CGlobalWaterAreaMap::ReleaseInstance();
	}
};

static CDestructGlobalWaterAreaMap DestructGlobalWaterAreaMap;*/

// initialize the static member
/*CGlobalWaterAreaMap* CGlobalWaterAreaMap::s_Instance=NULL;

CGlobalWaterAreaMap* CGlobalWaterAreaMap::GetInstance()
{
	if(!s_Instance)
		s_Instance = new CGlobalWaterAreaMap;

	return s_Instance;
}

void CGlobalWaterAreaMap::ReleaseInstance()
{
	if(s_Instance)	delete s_Instance;
	s_Instance = NULL;
}*/
/************************************************************************/

bool CGlobalWaterAreaMap::Load(const char* szMapPath)
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
	strcat(szMapParaPath, "watermap.conf");	

	FILE* fMapPara = fopen(szMapParaPath, "rt");
	if(!fMapPara) return false;

	// Need some error control!
	fscanf(fMapPara, "Map Width =%d\n", &m_iWidth);
	fscanf(fMapPara, "Map Length =%d\n", &m_iLength);
	fscanf(fMapPara, "Submap Width =%f\n", &m_fSubMapWidth);
	fscanf(fMapPara, "Submap Length =%f\n", &m_fSubMapLength);
	fclose(fMapPara);

	m_fRecipSubMapWidth = 1.0f / m_fSubMapWidth;
	m_fRecipSubMapLength = 1.0f / m_fSubMapLength;

	m_subWaterAreaMaps = new CWaterAreaMap * [ m_iWidth * m_iLength ];
	for(int i=0; i<m_iWidth * m_iLength; i++)
		m_subWaterAreaMaps[i] = new CWaterAreaMap;

	// now, we start reading the submaps' data
	FILE *fWaterMap;
	int iFileID;
	char szFileName[20];
	for(int i=0; i< m_iLength; i++)
		for(int j=0; j<m_iWidth; j++)
		{
			iFileID = (m_iLength-i-1)*m_iWidth+j+1;
			sprintf(szFileName, "%d.wmap", iFileID);
			strcpy(szSubMapPath, szPathWithSlash);
			strcat(szSubMapPath, szFileName);

			fWaterMap = fopen(szSubMapPath, "rb");
			if(fWaterMap)
			{
				if(SUBMAP(j,i).Load(fWaterMap))
				{
					if(!(SUBMAP(j,i).GetMapWidth()==m_fSubMapWidth && SUBMAP(j,i).GetMapLength()==m_fSubMapLength) )
						SUBMAP(j,i).Release();
				}
				
				fclose(fWaterMap);
			}

		}
	
	// set the origin of the global coordinate system
	SetMapCenterAsOrigin();

	return true;
}

bool CGlobalWaterAreaMap::LoadPiece(const char* szMapPath, int piece_idx)
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
	strcat(szMapParaPath, "watermap.conf");	

	FILE* fMapPara = fopen(szMapParaPath, "rt");
	if(!fMapPara) return false;

	// Need some error control!
	m_iWidth = 1;
	m_iLength = 1;
	int tmp;
	fscanf(fMapPara, "Map Width =%d\n", &tmp);
	fscanf(fMapPara, "Map Length =%d\n", &tmp);
	fscanf(fMapPara, "Submap Width =%f\n", &m_fSubMapWidth);
	fscanf(fMapPara, "Submap Length =%f\n", &m_fSubMapLength);
	fclose(fMapPara);

	m_fRecipSubMapWidth = 1.0f / m_fSubMapWidth;
	m_fRecipSubMapLength = 1.0f / m_fSubMapLength;

	m_subWaterAreaMaps = new CWaterAreaMap * [ m_iWidth * m_iLength ];
	for(int i=0; i<m_iWidth * m_iLength; i++)
		m_subWaterAreaMaps[i] = new CWaterAreaMap;

	// now, we start reading the submaps' data
	FILE *fWaterMap;
	int iFileID;
	char szFileName[20];

	iFileID = piece_idx + 1;
	sprintf(szFileName, "%d.wmap", iFileID);
	strcpy(szSubMapPath, szPathWithSlash);
	strcat(szSubMapPath, szFileName);

	fWaterMap = fopen(szSubMapPath, "rb");
	if(fWaterMap)
	{
		if(SUBMAP(0,0).Load(fWaterMap))
		{
			if(!(SUBMAP(0,0).GetMapWidth()==m_fSubMapWidth && SUBMAP(0,0).GetMapLength()==m_fSubMapLength) )
				SUBMAP(0,0).Release();
		}

		fclose(fWaterMap);
	}

	
	// set the origin of the global coordinate system
	SetMapCenterAsOrigin();

	return true;
}

bool CGlobalWaterAreaMap::Load(int row, int col, const int * piece_indexes, size_t piece_count, CGlobalWaterAreaMap** watermap_piece)
{
	// Need some error control!
	m_iWidth = col;
	m_iLength = row;
	m_fSubMapWidth = watermap_piece[0]->m_fSubMapWidth;
	m_fSubMapLength = watermap_piece[0]->m_fSubMapLength;

	m_fRecipSubMapWidth = 1.0f / m_fSubMapWidth;
	m_fRecipSubMapLength = 1.0f / m_fSubMapLength;

	m_subWaterAreaMaps = new CWaterAreaMap * [ m_iWidth * m_iLength ];

	// now, we start reading the submaps' data
	for(int i=0; i< m_iLength; i++)
	{
		for(int j=0; j<m_iWidth; j++)
		{
			//iFileID = (m_iLength-i-1)*m_iWidth+j+1;
			m_subWaterAreaMaps[i * m_iWidth + j] = watermap_piece[piece_indexes[(m_iLength-i-1) * m_iWidth + j]]->m_subWaterAreaMaps[0];
		}
	}

	m_data_ref = true;
	
	// set the origin of the global coordinate system
	SetMapCenterAsOrigin();

	return true;
}
