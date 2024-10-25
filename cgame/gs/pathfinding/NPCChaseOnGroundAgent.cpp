/*
 * FILE: NPCChaseOnGroundAgent.cpp
 *
 * DESCRIPTION:   A  class derived from the class CNPCChaseAgent 
 *							which realizes on-ground NPCs' chasing movement.
 *
 * CREATED BY: He wenfeng, 2005/4/18
 *
 * HISTORY: 
 *
 * Copyright (c) 2004 Archosaur Studio, All Rights Reserved.
 */

#include "NPCChaseOnGroundAgent.h"

// #define ZERO_DIST_ERROR 0.001f

CNPCChaseOnGroundAgent::CNPCChaseOnGroundAgent(CMap * pMap)
	:CNPCChaseAgent(pMap)
{
	m_bFoundPath = false;
	m_bGoStraightForward = false;
}

CNPCChaseOnGroundAgent::~CNPCChaseOnGroundAgent()
{
	Release();
}

void CNPCChaseOnGroundAgent::Release()
{
	m_Open.Release();
	m_Close.Release();
	m_PathFound.clear();
	m_PathPredict.clear();
}

void CNPCChaseOnGroundAgent::MoveOneStep()
{
	if(GetToGoal()) return;

	if(m_bGoStraightForward)
	{
		// Go in straight line!
		CNPCChaseAgent::MoveOneStep();
		return;
	}

	if(m_bStepTooSmall)
	{
		if(m_iCurStep == 0)
		{
			// save the current pos
			A3DVECTOR3 vLastPos = m_vCurPos;	

			// compute next grid!
			if(m_bIsGoalReachable)
			{
				// Start the path finding!
				BestFirstSearchPath();
				
				if(m_bFoundPath)
				{
					// Follow the found path
					FollowFoundPath();
					m_bBlocked = false;
				}
				else
				{
					// Predict the path and go!
					PredictPath();
				}
			}
			else
				PredictPath();
			
			// if we've get to the goal, return!
			if(m_bGetToGoal) return;

			// only consider the delta of XOZ plane!
			m_vSmallStep = m_vCurPos - vLastPos;
			m_vSmallStep.y = 0.0f;
			
			m_vCurPos = vLastPos;	// back to the last pos

			m_vSmallStep /= m_iStepsOneGrid;
		}

		m_vCurPos += m_vSmallStep;
		m_iCurStep ++ ;
		if(m_iCurStep == m_iStepsOneGrid)
			m_iCurStep = 0;
	
	}
	else
	{
		// normal step size: step size is larger than one grid size

		if(m_bIsGoalReachable)
		{
			// Start the path finding!
			BestFirstSearchPath();
			
			if(m_bFoundPath)
			{
				// Follow the found path
				FollowFoundPath();
				m_bBlocked = false;
			}
			else
			{
				// Predict the path and go!
				PredictPath();
			}
		}
		else
			PredictPath();

	}

	// We've placed the AdjustCurPos() func here!
	AdjustCurPos();
}

bool CNPCChaseOnGroundAgent::StartPathFinding(int SearchPixels)
{
	if(GetToGoal()) return false;
	
	Release();
	
	
	// compute start and goal pos in the map!
	//m_p2Start = g_NPCMoveMap.GetMapPos(m_vCurPos);
	m_p2Start = m_pMap->GetGroundMapPos(m_vCurPos);
	//m_p2Goal = g_NPCMoveMap.GetMapPos(m_vGoal);
	m_p2Goal = m_pMap->GetGroundMapPos(m_vGoal);
	
	/*
	// Now we add the test (if the start pos is close enough to the goal)
	// in the SetGoal() interface, so here, we don't do this test!

	if(PosGetToGoal(m_p2Start))
	{
		m_bGetToGoal = true;
		return false;
	}
	*/
	
	if(CanGoStraightForward())
	{
		m_bGoStraightForward = true;
		m_bBlocked = false;
		
		//////////////////////////////////////////////////////////////////////////
		// revised by wenfeng, 05-8-16
		// when going straightforward, we regard it as that we've found a path
		m_bFoundPath = true;			

		return false;
	}
	else
		m_bGoStraightForward = false;

	// compute m_du, m_dv
	if(m_p2Goal.u > m_p2Start.u)
		m_du=1;
	else if(m_p2Goal.u < m_p2Start.u)
		m_du = -1;
	else 
		m_du =0;

	if(m_p2Goal.v > m_p2Start.v)
		m_dv=1;
	else if(m_p2Goal.v < m_p2Start.v)
		m_dv = -1;
	else 
		m_dv =0;

	// Init the open list
	PathNode *pPNStart = new PathNode;
	pPNStart->u = m_p2Start.u;
	pPNStart->v = m_p2Start.v;
	pPNStart->h= GetDist(pPNStart->u,pPNStart->v,m_p2Goal);
	pPNStart->pLastNode = NULL;
	m_Open.SortPush(pPNStart);	

	m_bFoundPath = false;
	m_bGetToGoal = false;
	m_bBlocked = false;
	
	//m_bIsGoalReachable = g_NPCMoveMap.IsPosReachable(m_p2Goal);
	//m_bIsGoalReachable = g_NPCMoveMap.IsPosNeighborsReachable(m_p2Goal, 1);
	m_bIsGoalReachable = m_pMap->IsGroundPosNeighborsReachable(m_p2Goal, 1);

	m_PFPixels = SearchPixels;
	
	m_iCurInFoundPath = -1;

	return true;
}

void CNPCChaseOnGroundAgent::PredictPath()
{
	if(m_bBlocked) return;				// if blocked, keep the current pos

	POS2D posLast = (m_PathPredict.empty())? m_p2Start: m_PathPredict.back();
	POS2D posCur;
	
	for(int i=0; i < m_StepPixels; i++)
	{
		posCur.u = (posLast.u == m_p2Goal.u) ? posLast.u: (posLast.u + m_du);
		posCur.v = (posLast.v == m_p2Goal.v) ? posLast.v: (posLast.v + m_dv);

		//if(!g_NPCMoveMap.IsPosReachable(posCur))		// blocked?
		if(!m_pMap->IsGroundPosReachable(posCur))		// blocked?
		{
			m_bBlocked = true;
			return;
		}
		
		// compute the 3D position. Note that only x,z coordinates is computed!
		//m_vCurPos.x += (posCur.u - posLast.u) * g_NPCMoveMap.GetPixelSize();
		m_vCurPos.x += (posCur.u - posLast.u) * m_pMap->GetGroundPixelSize();
		//m_vCurPos.z += (posCur.v - posLast.v) * g_NPCMoveMap.GetPixelSize();
		m_vCurPos.z += (posCur.v - posLast.v) * m_pMap->GetGroundPixelSize();
		
		// No need to get the Y coordinate each time!
		// m_vCurPos.y = NPCMove::GetTerrainHeight(m_vCurPos.x, m_vCurPos.z) + g_NPCMoveMap.GetPosDeltaHeight(posCur);

		if(PosGetToGoal(posCur))
		{
			m_bGetToGoal = true;
			//m_vCurPos.y = NPCMove::GetTerrainHeight(m_vCurPos.x, m_vCurPos.z) + g_NPCMoveMap.GetPosDeltaHeight(posCur);
			//m_vCurPos.y = NPCMove::GetTerrainHeight(m_vCurPos.x, m_vCurPos.z) + m_pMap->GetGroundPosDeltaHeight(posCur);
			m_vCurPos.y = m_pMap->GetTerrainHeight(m_vCurPos.x, m_vCurPos.z) + m_pMap->GetGroundPosDeltaHeight(posCur);

			// Adjust the m_vCurPos!
			AdjustCurPosGetToGoal(posCur, posLast);

			return;
		}

		m_PathPredict.push_back(posCur);
		posLast = posCur;
	}

	// Revised by wenfeng, 05-5-18
	// We've placed the AdjustCurPos() func outside!
	//m_vCurPos.y = NPCMove::GetTerrainHeight(m_vCurPos.x, m_vCurPos.z) + g_NPCMoveMap.GetPosDeltaHeight(posCur);
}

void CNPCChaseOnGroundAgent::BestFirstSearchPath()
{
	if(m_bFoundPath) return;		// path has been found, return directly

	int SearchPixels = 0;
	LPPathNode lpPN, lpPNNeighbor;
	
	while( !m_Open.empty() && SearchPixels < m_PFPixels)
	{
		// pop up the first element
		lpPN = m_Open.PopFront();
		if(PosGetToGoal(lpPN->u,lpPN->v))
		{
			m_bFoundPath = true;
			break;		// now we've found the path!
		}
		
		SearchPixels++;

		// check the 8-neighbors
		lpPNNeighbor = new PathNode;
		lpPNNeighbor->u =  lpPN->u+1;
		lpPNNeighbor->v = lpPN ->v;
		lpPNNeighbor->h = GetDist(lpPNNeighbor->u,lpPNNeighbor->v,m_p2Goal);
		lpPNNeighbor->pLastNode = lpPN;
		if(!InsertPathNode(lpPNNeighbor))
			delete lpPNNeighbor;

		lpPNNeighbor = new PathNode;
		lpPNNeighbor->u =  lpPN->u+1;
		lpPNNeighbor->v = lpPN ->v+1;
		lpPNNeighbor->h = GetDist(lpPNNeighbor->u,lpPNNeighbor->v,m_p2Goal);
		lpPNNeighbor->pLastNode = lpPN;
		if(!InsertPathNode(lpPNNeighbor))
			delete lpPNNeighbor;

		lpPNNeighbor = new PathNode;
		lpPNNeighbor->u =  lpPN->u;
		lpPNNeighbor->v = lpPN ->v+1;
		lpPNNeighbor->h = GetDist(lpPNNeighbor->u,lpPNNeighbor->v,m_p2Goal);
		lpPNNeighbor->pLastNode = lpPN;
		if(!InsertPathNode(lpPNNeighbor))
			delete lpPNNeighbor;

		lpPNNeighbor = new PathNode;
		lpPNNeighbor->u =  lpPN->u-1;
		lpPNNeighbor->v = lpPN ->v+1;
		lpPNNeighbor->h = GetDist(lpPNNeighbor->u,lpPNNeighbor->v,m_p2Goal);
		lpPNNeighbor->pLastNode = lpPN;
		if(!InsertPathNode(lpPNNeighbor))
			delete lpPNNeighbor;

		lpPNNeighbor = new PathNode;
		lpPNNeighbor->u =  lpPN->u-1;
		lpPNNeighbor->v = lpPN ->v;
		lpPNNeighbor->h = GetDist(lpPNNeighbor->u,lpPNNeighbor->v,m_p2Goal);
		lpPNNeighbor->pLastNode = lpPN;
		if(!InsertPathNode(lpPNNeighbor))
			delete lpPNNeighbor;

		lpPNNeighbor = new PathNode;
		lpPNNeighbor->u =  lpPN->u-1;
		lpPNNeighbor->v = lpPN ->v-1;
		lpPNNeighbor->h = GetDist(lpPNNeighbor->u,lpPNNeighbor->v,m_p2Goal);
		lpPNNeighbor->pLastNode = lpPN;
		if(!InsertPathNode(lpPNNeighbor))
			delete lpPNNeighbor;

		lpPNNeighbor = new PathNode;
		lpPNNeighbor->u =  lpPN->u;
		lpPNNeighbor->v = lpPN ->v-1;
		lpPNNeighbor->h = GetDist(lpPNNeighbor->u,lpPNNeighbor->v,m_p2Goal);
		lpPNNeighbor->pLastNode = lpPN;
		if(!InsertPathNode(lpPNNeighbor))
			delete lpPNNeighbor;

		lpPNNeighbor = new PathNode;
		lpPNNeighbor->u =  lpPN->u+1;
		lpPNNeighbor->v = lpPN ->v-1;
		lpPNNeighbor->h = GetDist(lpPNNeighbor->u,lpPNNeighbor->v,m_p2Goal);
		lpPNNeighbor->pLastNode = lpPN;
		if(!InsertPathNode(lpPNNeighbor))
			delete lpPNNeighbor;

		// 插入m_Close中
		m_Close.push_back(lpPN);
	}

	// Generate the found path, store in m_PathFound
	if(m_bFoundPath)
	{
		GeneratePath(lpPN);
		
		// ************** Added by wenfeng, 05-12-10
		// forget deleting the lpPN?...
		delete lpPN;
	}

}

bool CNPCChaseOnGroundAgent::InsertPathNode(LPPathNode lpPN)
{
	POS2D pos(lpPN->u, lpPN->v);
	//if( ! g_NPCMoveMap.IsPosReachable(pos) )
	if( ! m_pMap->IsGroundPosReachable(pos) )
		return false;

	if( m_Open.FindByPos(lpPN->u, lpPN->v) || m_Close.FindByPos(lpPN->u, lpPN->v) )	
		return false;
	else
	{
		if(m_Open.SortPush(lpPN))
			return true;
		else
			return false;
	}
}

// After the function, lpPN will be changed!
void CNPCChaseOnGroundAgent::GeneratePath(LPPathNode lpPN)
{
	  // From the goal to the start
	while(lpPN)
	{
		POS2D pos(lpPN->u, lpPN->v);
		m_PathFound.push_back(pos);
		lpPN = lpPN->pLastNode;
	}
}

void CNPCChaseOnGroundAgent::FollowFoundPath()
{
	int iMovePixels = 0;
	POS2D posCur, posLast;
	bool posChanged = false;

	while(m_iCurInFoundPath != 0 && iMovePixels < (int)m_StepPixels)
	{
		if(m_iCurInFoundPath==-1)
		{
			// Search whether the current pos in Path found!
			posLast = m_PathPredict.empty()? m_p2Start:m_PathPredict.back();
			for(DWORD i =0; i< m_PathFound.size(); i++)
			{
				if(m_PathFound[i].u==posLast.u && m_PathFound[i].v == posLast.v)
				{
					//here when i ==0, it means posLast is the expected pos!
					// It always occurs when the m_p2Start  just satisfies the goal condition.
					// Although it's really a exception,but here we still handle it!
					m_iCurInFoundPath =(i==0)?0:(i-1);
					break;
				}
			}
			if(m_iCurInFoundPath==-1)
			{
				// current pos is not in the path found, so we back move a pixel
				m_PathPredict.pop_back();
				posCur = m_PathPredict.empty()? m_p2Start:m_PathPredict.back();
			}
			else
			{
				posCur = m_PathFound[m_iCurInFoundPath];
			}
		}
		else
		{
			posLast = m_PathFound[m_iCurInFoundPath];
			m_iCurInFoundPath--;
			posCur = m_PathFound[m_iCurInFoundPath];
		}

		// compute the 3D position. Note that only x,z coordinates is computed!
		iMovePixels++;
		//m_vCurPos.x += (posCur.u - posLast.u) * g_NPCMoveMap.GetPixelSize();
		m_vCurPos.x += (posCur.u - posLast.u) * m_pMap->GetGroundPixelSize();
		//m_vCurPos.z += (posCur.v - posLast.v) * g_NPCMoveMap.GetPixelSize();
		m_vCurPos.z += (posCur.v - posLast.v) *  m_pMap->GetGroundPixelSize();
		posChanged = true;
	}

	// Revised by wenfeng, 05-5-18
	// We've placed the AdjustCurPos() func outside!
	//if(posChanged)	m_vCurPos.y = NPCMove::GetTerrainHeight(m_vCurPos.x, m_vCurPos.z) + g_NPCMoveMap.GetPosDeltaHeight(posCur);
	
	if( m_iCurInFoundPath == 0 && m_PathFound.size()>1)
	{
		m_bGetToGoal = true;
		
		// Adjust the m_vCurPos!
		AdjustCurPosGetToGoal(m_PathFound[0], m_PathFound[1]);
	}
}

bool CNPCChaseOnGroundAgent::AdjustCurPosGetToGoal(const POS2D& curMapPos, const POS2D& lastMapPos)
{
	if( m_fMinDist2Goal < ZERO_DIST_ERROR && curMapPos == m_p2Goal)
	{
		// m_fMinDist2Goal	is set to 0, so we directly set the m_vCurPos to the m_vGoal
		m_vCurPos = m_vGoal;
		return true;
	}

	//A3DVECTOR3 vCurPixelCenter = g_NPCMoveMap.GetPixelCenter(curMapPos);
	A3DVECTOR3 vCurPixelCenter = m_pMap->GetGroundPixelCenter(curMapPos);
	A3DVECTOR3 vLastPixelCenter = m_pMap->GetGroundPixelCenter(lastMapPos);

	A3DVECTOR3 vDelta = m_vGoal - vCurPixelCenter;
	vDelta.y = 0.0f;
	if(vDelta.SquaredMagnitude() >= m_fSqrMinDist2Goal )
		m_vCurPos = vCurPixelCenter;		// 当前中心点到目标距离仍大于阈值，该情况应很少见
	else
	{
		vDelta = m_vGoal - vLastPixelCenter;
		vDelta.y = 0.0f;
		if(vDelta.SquaredMagnitude() <= m_fSqrMinDist2Goal )
			m_vCurPos = vLastPixelCenter;	// 上一中心点到目标距离仍小于阈值，该情况应很少见
		else
		{
			// 否则，在vCurPixelCenter和vLastPixelCenter的连线段上寻找最佳点
			// 这里需要求解一个一元二次方程
			float dx = vLastPixelCenter.x - vCurPixelCenter.x;
			float dz = vLastPixelCenter.z - vCurPixelCenter.z;
			float cx_gx = vCurPixelCenter.x - m_vGoal.x;
			float cz_gz =  vCurPixelCenter.z - m_vGoal.z;
			// 构造一元二次方程的系数
			float a = dx*dx + dz*dz;
			float b = 2*(dx*cx_gx + dz*cz_gz);
			float c = cx_gx * cx_gx + cz_gz * cz_gz - m_fSqrMinDist2Goal;
			float Determinant = b * b - 4 * a * c;
			if (Determinant < 0 )
				return false;							// Error, Adjust failed!
			
			float r = ( -b - sqrt(Determinant) ) / (2*a);		// root 1
			bool bHaveRoot = false;
			if(r>0.0f && r<1.0f) 
				bHaveRoot = true;
			else
			{
				r = (- b /a ) - r;					// root 2
				if(r>0.0f && r<1.0f) 
					bHaveRoot = true;
			}

			if(bHaveRoot)
			{	
				m_vCurPos.x = vCurPixelCenter.x + dx * r;
				m_vCurPos.z = vCurPixelCenter.z + dz * r;
				m_vCurPos.y = 0.0f;
			}
			else
				return false;
		}
	}
	
	// Fill the Y coordinate!
	//AdjustCurPos();
	return true;
}

bool CNPCChaseOnGroundAgent::CanGoStraightForward()
{
	return CanGoStraightForward(m_p2Start, m_p2Goal);
}

bool CNPCChaseOnGroundAgent::CanGoStraightForward(const POS2D& posFrom, const POS2D& posTo)
{
	//*******************************************************
	// Revised by wenfeng, 05-12-20
	// lay the following code in NPCMoveMap
	//return g_NPCMoveMap.CanGoStraightForward(posFrom, posTo);
	return  m_pMap->CanGroundGoStraightForward(posFrom, posTo);

	// As general, we use x and y here, while they are corresponding to u, v in Map Pos2D denotation respectively!
	/*
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
	POS2D pos;
	for(int i=0; i<dx; i++)
	{
		pos.u = x;
		pos.v = y;
		if( !g_NPCMoveMap.IsPosReachable(pos) )			
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
	*/
}
