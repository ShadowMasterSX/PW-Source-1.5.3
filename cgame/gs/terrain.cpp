/*
 * FILE: Terrain.cpp
 *
 * DESCRIPTION: header for terrain class on server side
 *
 * CREATED BY: Hedi, 2004/11/22 
 *
 * HISTORY:
 *
 * Copyright (c) 2004 Archosaur Studio, All Rights Reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>	//for memcpy

#include "terrain.h"

CTerrain::CTerrain()
{
	m_pHeights		= NULL;
}

CTerrain::~CTerrain()
{
	Release();
}

bool CTerrain::Init(const TERRAINCONFIG& config, float xmin, float zmin, float xmax, float zmax)
{
	m_config = config;
	m_vGridSizeInv = 1.0f / config.vGridSize;

	// first determine the origin of the entire terrain, to be convinient, we set 
	// the origin to the left top corner of the entire terrain
	float t_ox = -config.nAreaWidth * config.nNumCols * config.vGridSize * 0.5f;
	float t_oz = config.nAreaHeight * config.nNumRows * config.vGridSize * 0.5f;
	
	float areawidth = config.nAreaWidth * config.vGridSize;
	float areaheight = config.nAreaHeight * config.vGridSize;

	int nAreaHStart = int((xmin - t_ox) / areawidth);
	if( nAreaHStart < 0 ) nAreaHStart = 0;
	int nAreaHEnd = int((xmax - t_ox) / areawidth);
	if( nAreaHEnd >= config.nNumCols ) nAreaHEnd = config.nNumCols - 1;

	int nAreaVStart = int((t_oz - zmax) / areaheight);
	if( nAreaVStart < 0 ) nAreaVStart = 0;
	int nAreaVEnd = int((t_oz - zmin) / areaheight);
	if( nAreaVEnd >= config.nNumRows ) nAreaVEnd = config.nNumRows - 1;

	m_ox = t_ox + nAreaHStart * config.nAreaWidth * config.vGridSize;
	m_oz = t_oz - nAreaVStart * config.nAreaHeight * config.vGridSize;

	m_nNumVertX = (nAreaHEnd - nAreaHStart + 1) * config.nAreaWidth + 1;
	m_nNumVertZ = (nAreaVEnd - nAreaVStart + 1) * config.nAreaHeight + 1;

	// now we should alloc the buffer
	m_pHeights = new float[m_nNumVertZ * m_nNumVertX];
	if( NULL == m_pHeights )
		return false;

	// then read from the hmap one by one
	char	szMapName[256];
	for(int v=nAreaVStart; v<=nAreaVEnd; v++)
	{
		for(int h=nAreaHStart; h<=nAreaHEnd; h++)
		{
			int idArea = v * config.nNumCols + h;

			sprintf(szMapName, "%s/%d.hmap", config.szMapPath, idArea + 1);
			FILE * fpHMap = fopen(szMapName, "rb");
			if( !fpHMap )
				return false;

			float * pHeight = m_pHeights + (v - nAreaVStart) * config.nAreaHeight * m_nNumVertX + (h - nAreaHStart) * config.nAreaWidth;
			for(int i=0; i<=config.nAreaHeight; i++)
			{
				fread(pHeight, sizeof(float), config.nAreaWidth + 1, fpHMap);

				pHeight += m_nNumVertX;
			}

			fclose(fpHMap);
		}
	}

	float range = config.vHeightMax - config.vHeightMin;
	float offset = config.vHeightMin;
	// convert from 0.0~1.0 to real height range
	for(int i=0; i<m_nNumVertZ * m_nNumVertX; i++)
		m_pHeights[i] = m_pHeights[i] * range + offset;
	
	return true;
}

bool CTerrain::InitPiece(const TERRAINCONFIG& config, int piece_idx)
{
	m_config = config;
	m_config.nNumAreas = 1;
	m_config.nNumRows = 1;
	m_config.nNumCols = 1;
	m_vGridSizeInv = 1.0f / config.vGridSize;

	m_ox = -config.nAreaWidth * config.vGridSize * 0.5f;
	m_oz = config.nAreaHeight * config.vGridSize * 0.5f;

	m_nNumVertX = config.nAreaWidth + 1;
	m_nNumVertZ = config.nAreaHeight + 1;

	m_pHeights = new float[m_nNumVertZ * m_nNumVertX];
	if( NULL == m_pHeights )
		return false;

	char	szMapName[256];
	sprintf(szMapName, "%s/%d.hmap", config.szMapPath, piece_idx + 1);
	FILE * fpHMap = fopen(szMapName, "rb");
	if( !fpHMap )
		return false;
	fread(m_pHeights, sizeof(float), m_nNumVertX * m_nNumVertZ, fpHMap);
	fclose(fpHMap);

	float range = config.vHeightMax - config.vHeightMin;
	float offset = config.vHeightMin;
	// convert from 0.0~1.0 to real height range
	for(int i=0; i<m_nNumVertZ * m_nNumVertX; i++)
		m_pHeights[i] = m_pHeights[i] * range + offset;
	
	return true;
}

bool CTerrain::Init(int row, int col, const int * piece_indexes, CTerrain ** terrain_pieces)
{
	m_config = terrain_pieces[0]->m_config;
	m_config.nNumAreas = row * col;
	m_config.nNumRows = row;
	m_config.nNumCols = col;

	m_vGridSizeInv = 1.0f / m_config.vGridSize;

	m_ox = - m_config.nAreaWidth * m_config.nNumCols * m_config.vGridSize * 0.5f;
	m_oz = m_config.nAreaHeight * m_config.nNumRows * m_config.vGridSize * 0.5f;
	
	m_nNumVertX = m_config.nNumCols * m_config.nAreaWidth + 1;
	m_nNumVertZ = m_config.nNumRows * m_config.nAreaHeight + 1;

	m_pHeights = new float[m_nNumVertZ * m_nNumVertX];
	if(m_pHeights == NULL)
		return false;
	for(int v=0; v<m_config.nNumRows; v++)
	{
		for(int h=0; h<m_config.nNumCols; h++)
		{
			const CTerrain * terrain_piece = terrain_pieces[piece_indexes[v * m_config.nNumCols + h]];	
			
			float * pHeight = m_pHeights + v * m_config.nAreaHeight * m_nNumVertX + h * m_config.nAreaWidth;
			float * pHeightSrc = terrain_piece->m_pHeights; 
			for(int i=0; i<=m_config.nAreaHeight; i++)
			{
				//注意：在边界上将发生覆盖
				memcpy(pHeight, pHeightSrc, terrain_piece->m_nNumVertX * sizeof(float));
				pHeight += m_nNumVertX;
				pHeightSrc += terrain_piece->m_nNumVertX;
			}
		}
	}
	
	return true;
}

bool CTerrain::Release()
{
	delete [] m_pHeights;
	m_pHeights = NULL;
	
	return true;
}

float CTerrain::GetHeightAt(float x, float z)
{
	float		h = (x - m_ox) * m_vGridSizeInv;
	float		v = (m_oz - z) * m_vGridSizeInv;

	int		nX = (int) h;
	int		nZ = (int) v;
	float		dx = h - nX;
	float		dz = v - nZ;

	if( nX < 0 || nZ < 0 || nX >= m_nNumVertX || nZ >= m_nNumVertZ )
		return m_config.vHeightMin;
	
	int		v0 = nZ * m_nNumVertX + nX;
	float	h0, h1, h2;  
	//	0-----1
	//	| \   |
	//	|  \  |
	//	|   \ |
	//	2-----3
	if( dx <  dz )
	{
		// left-top triangle
		h0 = m_pHeights[v0 + m_nNumVertX];
		h1 = m_pHeights[v0 + m_nNumVertX + 1];
		h2 = m_pHeights[v0];
		dz = 1.0f - dz;
	}
	else
	{
		// right-bottom triangle
		h0 = m_pHeights[v0 + 1];
		h1 = m_pHeights[v0];
		h2 = m_pHeights[v0 + m_nNumVertX + 1];

		dx = 1.0f - dx;
	}

	return h0 + (h1 - h0) * dx + (h2 - h0) * dz;
}
