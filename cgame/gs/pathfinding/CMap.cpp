/********************************************************************
  created:	   11/4/2006  
  filename:	   Map.cpp
  author:      Wangkuiwu  
  description: 
  Copyright (c) , All Rights Reserved.
*********************************************************************/
#include "CMap.h"

namespace NPCMoveMap
{

CMap::CMap()
:m_pGroundMap(NULL),m_pWaterMap(NULL),m_pSPMap(NULL)
{
	
}

CMap::~CMap()
{
	Release();
}

bool CMap::Init(const char * szGroundPath, const char * szWaterMap, const char * szAreamap)
{
	Release(); ///< realse firstly

	m_pGroundMap = new CNPCMoveMap(this);	
	if(!m_pGroundMap->Load(szGroundPath))
	{
		Release();
		return false;
	}

	m_pWaterMap = new CGlobalWaterAreaMap(this);
	if(!m_pWaterMap->Load(szWaterMap))
	{
		Release();
		return false;
	}

	m_pSPMap = new CGlobalSPMap(this);
	if(!m_pSPMap->Load(szAreamap))
	{
		Release();
		return false;
	}
	return true;
}

bool CMap::InitPiece(const char * szGroundPath, const char * szWaterMap, const char * szAreamap, int piece_idx)
{
	Release(); ///< realse firstly

	m_pGroundMap = new CNPCMoveMap(this);	
	if(!m_pGroundMap->LoadPiece(szGroundPath, piece_idx))
	{
		Release();
		return false;
	}

	m_pWaterMap = new CGlobalWaterAreaMap(this);
	if(!m_pWaterMap->LoadPiece(szWaterMap, piece_idx))
	{
		Release();
		return false;
	}

	m_pSPMap = new CGlobalSPMap(this);
	if(!m_pSPMap->LoadFreePiece(szAreamap))
	{
		Release();
		return false;
	}
	return true;
}

bool CMap::Init(int row, int col, const int * piece_indexes, size_t piece_count, CMap** movemap_pieces)
{
	Release(); ///< realse firstly

	abase::vector<CNPCMoveMap *> groundmap_pieces;
	abase::vector<CGlobalWaterAreaMap *> watermap_pieces;
	abase::vector<CGlobalSPMap *> spmap_pieces;
	groundmap_pieces.reserve(piece_count);
	watermap_pieces.reserve(piece_count);
	spmap_pieces.reserve(piece_count);
	for(size_t i=0; i<piece_count; i++)
	{
		groundmap_pieces.push_back(movemap_pieces[i]->m_pGroundMap);
		watermap_pieces.push_back(movemap_pieces[i]->m_pWaterMap);
		spmap_pieces.push_back(movemap_pieces[i]->m_pSPMap);
	}

	m_pGroundMap = new CNPCMoveMap(this);	
	if(!m_pGroundMap->Load(row, col, piece_indexes, piece_count, groundmap_pieces.begin()))
	{
		Release();
		return false;
	}

	m_pWaterMap = new CGlobalWaterAreaMap(this);
	if(!m_pWaterMap->Load(row, col, piece_indexes, piece_count, watermap_pieces.begin()))
	{
		Release();
		return false;
	}

	m_pSPMap = new CGlobalSPMap(this);
	if(!m_pSPMap->SetFreeMap(row, col, *(spmap_pieces.begin())))
	{
		Release();
		return false;
	}
	return true;
}

void CMap::Release()
{
	if (m_pGroundMap)
	{
		m_pGroundMap->Release();
		delete m_pGroundMap;
		m_pGroundMap = NULL;
	}

	if(m_pWaterMap)
	{
		m_pWaterMap->Release();
		delete m_pWaterMap;
		m_pWaterMap = NULL;
	}

	if(m_pSPMap)
	{
		m_pSPMap->Release();
		delete m_pSPMap;
		m_pSPMap = NULL;
	}
}

}

