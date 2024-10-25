/********************************************************************
  created:	   20/12/2005  
  filename:	   PathFinding2D.cpp
  author:      Wangkuiwu  
  description: 2d path finding interface
  Copyright (c) , All Rights Reserved.
*********************************************************************/


#include "PathFinding2D.h"


CPathFinding2D::CPathFinding2D()
:m_pMap(NULL)
{

}

CPathFinding2D::~CPathFinding2D()
{
	Release();
}

void CPathFinding2D::Init(CMap * pMap,const POS2D& posStart, const POS2D& posGoal, float fRange)
{
	m_PosStart = posStart;
	m_PosGoal  = posGoal;
	//m_fGoalRange = fRange;
	m_fGoalRange = fRange + 0.001f;
	m_iStat      = PF2D_SEARCHING;
	m_pMap		 = pMap;

}

void CPathFinding2D::Release()
{
}

