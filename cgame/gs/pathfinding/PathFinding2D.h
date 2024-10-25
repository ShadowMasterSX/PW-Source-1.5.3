/********************************************************************
  created:	   20/12/2005  
  filename:	   PathFinding2D.h
  author:      Wangkuiwu  
  description: 2d path finding interface
  Copyright (c) , All Rights Reserved.
*********************************************************************/

#ifndef  _PATH_FINDING_2D_H_
#define  _PATH_FINDING_2D_H_

#include  <vector.h>
using namespace abase;
#include "NPCMoveMap.h"
#include "CMap.h"
using namespace NPCMoveMap;

enum  
{
	//PF2D_UNKNOWN   = 0x0,
	PF2D_SEARCHING, 
	PF2D_FOUND    , 
	PF2D_NOPATH    
};

class CPathFinding2D  
{
public:
	CPathFinding2D();
	virtual ~CPathFinding2D();
	virtual void Init(CMap * pMap, const POS2D& posStart, const POS2D& posGoal, float fRange = 0.01f);
	virtual void Release();
	virtual void StepSearch(int nSteps) = 0;
	/*
	 *
	 * @desc :
	 * @param :     
	 * @return :
	 * @note:
	 * @todo:   
	 * @author: kuiwu [20/12/2005]
	 * @ref:
	 */
	virtual void GeneratePath(vector<POS2D>& path) = 0;
	int          GetStat() const
	{
		return m_iStat;
	}

	static inline float GetManhDist(int x1, int z1, int x2, int z2)
	{
		 return (fabs(x1- x2) + fabs(z1 - z2));
	}
	static inline float GetEuclDist(int x1, int z1, int x2, int z2)
	{

		return (sqrtf((x1 - x2)*(x1 -x2) + (z1 -z2)*(z1 -z2)));
	}
protected:
	POS2D  m_PosStart;
	POS2D  m_PosGoal;
	float  m_fGoalRange;  //the dist close to goal
	int    m_iStat;	
	

	CMap  * m_pMap;
	
};

#endif
