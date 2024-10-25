/********************************************************************
  created:	   20/12/2005  
  filename:	   Pf2DBfs.cpp
  author:      Wangkuiwu  
  description: path finding 2d using best first searching
  Copyright (c) , All Rights Reserved.
*********************************************************************/
#include "Pf2DBfs.h"
#include <ASSERT.h>

#define PF2D_INVALID_NODE    (-30000)


int NeighborD[] =
{
		-1,  0,   //left
		+1,  0,   //right
		0,   +1,  //top
		0,   -1,  //bottom
		+1,  +1,  //topright
		+1,  -1,  //bottomright
		-1,  +1,  //topleft
		-1,  -1   //bottomleft
};


CPf2DBfs::CPf2DBfs()
{

}

CPf2DBfs::~CPf2DBfs()
{

}

void CPf2DBfs::Init(CMap * pMap, const POS2D& posStart, const POS2D& posGoal, float fRange /* = 0.01f  */)
{
	CPathFinding2D::Init(pMap, posStart, posGoal, fRange);
	m_Open.Init(30);

	
	m_CurNode.x = m_PosStart.u;
	m_CurNode.z = m_PosStart.v;
	m_CurNode.prv_x  = PF2D_INVALID_NODE;
	m_CurNode.prv_z  = PF2D_INVALID_NODE;
	m_CurNode.cost   = CPathFinding2D::GetManhDist(m_PosStart.u, m_PosStart.v, m_PosGoal.u, m_PosGoal.v);
	m_Open.Push(m_CurNode);	
	
	m_nSumSearchSteps = 0;
}

void CPf2DBfs::StepSearch(int nSteps)
{
	ASSERT(nSteps > 0);

	if ((m_iStat == PF2D_FOUND) || (m_iStat == PF2D_NOPATH))
	{
		return;
	}

	//m_iStat = PF2D_SEARCHING;
	
	int nCounter = 0;
	float fDist2Goal;
	Pf2DNode  node;
	while ((m_iStat == PF2D_SEARCHING)&& (!m_Open.Empty()) && (nCounter < nSteps))
	{
		m_Open.PopMinCost(m_CurNode);
		fDist2Goal = CPathFinding2D::GetEuclDist(m_CurNode.x, m_CurNode.z, m_PosGoal.u, m_PosGoal.v);
		if (fDist2Goal < m_fGoalRange)
		{
			m_iStat = PF2D_FOUND;
			m_Close.Push(m_CurNode);  //to simplify the path generating
			break;
		}
		++nCounter;
		// check the 8-neighbors
		for (int i = 0; i < 8; ++i)
		{
			node.x = m_CurNode.x + NeighborD[i*2];
			node.z = m_CurNode.z + NeighborD[i*2+1];
			//if ((!g_NPCMoveMap.IsPosReachable(node.x, node.z)) 
			if ((!m_pMap->IsGroundPosReachable(node.x, node.z)) 
				|| (m_Close.Find(node.x, node.z))
				|| (m_Open.Find(node.x, node.z) != m_Open.GetSize()))
			{
				continue;
			}
			node.prv_x = m_CurNode.x;
			node.prv_z = m_CurNode.z;
			node.cost = CPathFinding2D::GetManhDist(node.x, node.z, m_PosGoal.u, m_PosGoal.v);
			m_Open.Push(node);
		}
		//add to close
		m_Close.Push(m_CurNode);
	}

	m_nSumSearchSteps += nCounter;
	
	if ((m_Open.Empty()) && (m_iStat != PF2D_FOUND))
	{
		m_iStat = PF2D_NOPATH;
	}

}


void CPf2DBfs::GeneratePath(vector<POS2D>& path)
{
//	path.clear();
//	//first pass, get len
//	int len = 1;
//	short x, z, prv_x, prv_z;
//	x = m_CurNode.prv_x;
//	z = m_CurNode.prv_z;
//
//	while (x != PF2D_INVALID_NODE && z != PF2D_INVALID_NODE)
//	{
//		m_Close.GetPrv(x, z, prv_x, prv_z);
//		x = prv_x;
//		z = prv_z;
//		++len;
//	}
//	//second pass, fill path
//	path.reserve(len);
//	--len;
	
	//space-time tradeoff
	//first pass, fill temp path
	vector<POS2D> tmpPath;
	POS2D pos;
	short x, z;
	x = m_CurNode.x;
	z = m_CurNode.z;
	while (x != PF2D_INVALID_NODE && z != PF2D_INVALID_NODE)
	{
		pos.u = x;
		pos.v = z;
		tmpPath.push_back(pos);

		m_Close.GetPrv((short)pos.u, (short)pos.v, x, z);
	}
	//second pass, reverse the path order
	path.clear();
	int i = tmpPath.size() -1;
	while (i >= 0)
	{
		path.push_back(tmpPath[i]);
		--i;
	}

}


// #define  PF2D_OPEN_RETAIN_MAX

Pf2DOpen::Pf2DOpen()
{
}

Pf2DOpen::~Pf2DOpen()
{
	Clear();
}

void Pf2DOpen::Clear()
{
	m_List.clear();
}

void Pf2DOpen::Init(int nMaxSize /* = 30 */)
{
	m_nMaxSize = nMaxSize;
	ASSERT(m_nMaxSize > 0);
	m_List.reserve(m_nMaxSize);
}

void Pf2DOpen::Push(const Pf2DNode& node)
{
#ifdef PF2D_OPEN_RETAIN_MAX
	if (m_List.size() < m_nMaxSize)
	{
		m_List.push_back(node);
	}
	else
	{
		//get the max cost  to replace
		int iMax = 0;
		for (int i = 1; i < m_List.size(); ++i)
		{
//			if (m_List[i].cost > m_List[iMax].cost)
//			{
//				iMax = i;
//			}

			iMax = (m_List[i].cost > m_List[iMax].cost)? (i) :(iMax);
		}
		m_List[iMax] = node;
	}
#else
	m_List.push_back(node);
#endif

}


void Pf2DOpen::PopMinCost(Pf2DNode& node)
{
	int iMin = 0;
	for (size_t i = 1; i < m_List.size(); ++i)
	{
//		if (m_List[i].cost < m_List[iMin].cost)
//		{
//			iMin = i;
//		}
		//eliminate branches
		iMin = (m_List[i].cost < m_List[iMin].cost)? (i):(iMin);
	}

	node = m_List[iMin];
	m_List[iMin] = m_List[m_List.size()-1];
	m_List.pop_back();
}

int  Pf2DOpen::Find(short x, short z)
{
	size_t i = 0;
	while (i < m_List.size())
	{
		if (m_List[i].x == x && m_List[i].z == z)
		{
			break;
		}
		++i;
	}

	return i;
}

//low =x, high = z
#define MAKE_KEY2D(x, z)    ((unsigned int)(((unsigned short)((x) & 0xffff)) | ((unsigned int)((unsigned short)((z) & 0xffff))) << 16))
#define GET_KEY2D_X(key)		((unsigned short)((unsigned int)(key) & 0xffff))	
#define GET_KEY2D_Z(key)		((unsigned short)((unsigned int)(key) >> 16))


bool Pf2DClose::Find(short x, short z)
{
	unsigned int key = MAKE_KEY2D(x, z);
	return (m_List.find(key) != m_List.end());
}

void Pf2DClose::Push(const Pf2DNode& node)
{
	unsigned int key = MAKE_KEY2D(node.x, node.z);
	unsigned int val = MAKE_KEY2D(node.prv_x, node.prv_z);
	ASSERT(!Find(node.x, node.z));
	m_List.put(key, val);
}

void Pf2DClose::GetPrv(short x, short z, short& prv_x, short& prv_z)
{
	unsigned int key = MAKE_KEY2D(x, z);
	CloseList::pair_type pair;
	pair = m_List.get(key);
	if (pair.second)
	{
		unsigned int val = *pair.first;
		prv_x = GET_KEY2D_X(val);
		prv_z = GET_KEY2D_Z(val);
		return;
	}
    //not found
	ASSERT(0);
	
}


void CPf2DBfs::Release()
{
	m_Open.Clear();
	m_Close.Clear();
}