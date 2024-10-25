  /*
 * FILE: SortVectorSpatialPathNode.h
 *
 * DESCRIPTION:		a set of classes which implement a sorted vector of 3D Path Node, which
 *					can be used as the prototype of Open list of BFS algorithm
 *
 * CREATED BY:		He wenfeng, 2005/10/11
 *
 * HISTORY: 
 *
 * Copyright (c) 2005 Archosaur Studio, All Rights Reserved.
 */

#ifndef	_SORTVECTORSPATIALPATHNODE_H_
#define _SORTVECTORSPATIALPATHNODE_H_

#define MAX_NODE_NUM 60

#include "CompactSpacePassableOctree.h"
using namespace SPOctree;

#include <vector.h>
using namespace abase;

struct SpatialPathNode
{
	SpatialPathNode()
	{
		pOctrTravNode = NULL;
		pLastPathNode = NULL;
	}
	
	SpatialPathNode(const Pos3DInt& posPara, int hPara,CSPOctreeTravNode* pOctrTravNodePara,SpatialPathNode *pLastPathNodePara)
		:pos(posPara),h(hPara)
	{
		pOctrTravNode = pOctrTravNodePara;
		pLastPathNode = pLastPathNodePara;
	}

	~SpatialPathNode()
	{
		if(pOctrTravNode) 
			delete pOctrTravNode;
	}

// data members
	
	Pos3DInt pos;		// position
	int h;				// heuristic cost to the terminal
	
	// position's corresponding traversal node in the octree
	CSPOctreeTravNode * pOctrTravNode;
	
	// last path node
	SpatialPathNode * pLastPathNode;	
};
typedef SpatialPathNode* PtrSpatialPathNode;
typedef vector<PtrSpatialPathNode> VecPtrSpatialPathNode;

// Sorted vector of 3D spatial path-finding node.
// Class prototype of Open list in BFS alogrithm
class CSortVectorSpatialPathNode: public VecPtrSpatialPathNode
{
public:
	
	// Push in order: the PathNode with least "h" value will 
	// be laid in the front of the vector
	bool SortPush(const Pos3DInt& pos, int h,CSPOctreeTravNode* pOctrTravNode,SpatialPathNode *pLastPathNode)
	{
		PtrSpatialPathNode pNewPathNode;

		for(VecPtrSpatialPathNode::iterator it = begin();it!=end();it++)
		{
			if( h < (*it)->h)
			{
				pNewPathNode = new SpatialPathNode(pos,h,pOctrTravNode,pLastPathNode);
				insert(it, pNewPathNode);
				// exceeds the max number, so we pop the last node
				if(size() > MAX_NODE_NUM)
				{
					PtrSpatialPathNode pBackPathNode = back();
					pop_back();
					delete pBackPathNode;
				}

				return true;
			}
		}
		
		if(size() == MAX_NODE_NUM) return false;
		
		pNewPathNode = new SpatialPathNode(pos,h,pOctrTravNode,pLastPathNode);
		push_back(pNewPathNode);
		return true;
	}
	
	PtrSpatialPathNode PopFront()
	{
		if(empty()) return NULL;
		PtrSpatialPathNode pPathNode = front();
		VecPtrSpatialPathNode::iterator it = begin();
		erase(it);
		return pPathNode;
	}

	PtrSpatialPathNode FindByPos(const Pos3DInt& pos)
	{
		for(VecPtrSpatialPathNode::iterator it = begin(); it!= end(); it++)
		{
			if((*it)->pos == pos)
				return *it;
		}
		return NULL;
	}

	void Release()
	{
		for(VecPtrSpatialPathNode::iterator it = begin(); it!= end(); it++)
		{
			delete (*it);
		}
		clear();
	}
};

#endif
