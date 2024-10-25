/************************************************************************\
 *	SortVectorPathNode.h
 *		
 *			Created by He wenfeng ,  2005-04-07
 
 *			Revised by He wenfeng ,   05-04-18: Revised for only BFS algorithm
 *
 *			Define a class which implement a sorted vector and the 
 *		element type is PathNode
 \************************************************************************/

#ifndef _SORTVECTORPATHNODE_H_
#define _SORTVECTORPATHNODE_H_

#define MAX_NODES 30

struct  PathNode
{
	int u;
	int v;
	int h;								// heuristic cost to the terminal
	PathNode* pLastNode;
};

typedef PathNode* LPPathNode;

//*********************************************************
// Noted by wenfeng, 05-12-19
// After the test, it surprises me that the efficiency isn't
// improved at all and even more time-consuming (about 5 times
// as the case implemented using vector). So we can see Vector
// has obvious advantage when we need access a section of data
// frequently.
//*********************************************************
// Revised by wenfeng, 05-12-19
// Try to implement CSortVectorPathNode in list mode, since
// PopFront() method is a little time-consuming in vector mode.
//*********************************************************

// #define IMPL_BY_LIST

#ifdef IMPL_BY_LIST

	// implemented using list
	#include <list>
	typedef std::list<LPPathNode> vecLPPN;

#else

	// implemented using vector
	#include <vector.h>
	using namespace abase;
	
	typedef vector<LPPathNode> vecLPPN;

#endif	// end of #ifdef IMPL_BY_LIST

class CSortVectorPathNode:public vecLPPN
{
public:
	
	CSortVectorPathNode()
	{
		//---------------------------------------------
		// try to pre-malloc a block memery 
		// but according to the test results, it seems no need to do :(
// #define PRE_MALLOC
#if !defined(IMPL_BY_LIST) && defined (PRE_MALLOC)
		_data = _M_allocate(MAX_NODES);
		_finish		= _data;
		_cur_size	= 0;
		_max_size	= MAX_NODES;
#endif	// end of #ifdef PRE_MALLOC
	}

	~CSortVectorPathNode(){}
	
	// push in order: the least-h lpPN will be laid in the front of the vector
	bool SortPush(LPPathNode lpPN)
	{
		for(vecLPPN::iterator it = begin(); it!= end(); it++)
		{
			if(lpPN->h < (*it)->h)
			{
				insert(it, lpPN);
				if(size()>MAX_NODES)
				{
					LPPathNode lpBack = back();
					pop_back();
					delete lpBack;
				}
				return true;
			}
		}
		if(size()<MAX_NODES)
		{
			push_back(lpPN);
			return true;
		}
		else
			return false;
	}

	LPPathNode PopFront()
	{
		if(empty()) return NULL;
		LPPathNode lpPN = front();
		vecLPPN::iterator it = begin();
		erase(it);
		return lpPN;
	}

	LPPathNode FindByPos(int u, int v)
	{
		for(vecLPPN::iterator it = begin(); it!= end(); it++)
		{
			if((*it)->u == u && (*it)->v == v)
				return *it;
		}
		return NULL;
	}

	void Release()
	{
		for(vecLPPN::iterator it = begin(); it!= end(); it++)
		{
			delete (*it);
		}
		clear();
	}
};

#endif	// end of #ifndef _SORTVECTORPATHNODE_H_

