#ifndef _BRUSH_TREE_H_
#define _BRUSH_TREE_H_

#include <vector.h>
#include <a3dvector.h>
#include "A3DGeometry.h"

namespace SvrCD
{
class CCDBrush;
class BrushTraceInfo;

typedef abase::vector<CCDBrush*>    BrushVec;


//struct BrushNode
struct  BrushNode
{
	A3DAABB    aabb;
	BrushNode *   children[4];
	abase::vector<CCDBrush*>   brushes;
	bool					  RoI;  //obsolete

	BrushNode()
	{
		children[0]  = NULL;
		RoI = true;
	}
	~BrushNode()
	{
		if (children[0])
		{
			for (int i = 0; i < 4; ++i)
			{
				delete children[i];
				children[i] = NULL;
			}
		}
		brushes.clear();
	}
	bool IsLeaf() const
	{
		return (children[0] == NULL);
	}
	bool IsRoI() const
	{
		return RoI;
	}
	bool IsIn(float x, float z)
	{
		A3DVECTOR3  pos(x, aabb.Center.y, z);
		return aabb.IsPointIn(pos);
	}

};


#ifdef BMAN_EDITOR
struct VerboseStat
{
	int nCheckBrushes;

	VerboseStat()
	{
		nCheckBrushes = 0;
	}
};
#endif

class CBrushTree
{
public:
	CBrushTree();
	~CBrushTree();
	//load the tree from file
	bool Load(const char * szName);
	//build the tree on the fly
	void Build(const A3DVECTOR3& ext);
	void Release();
	bool Trace(BrushTraceInfo * trcInfo,const abase::vector<char>* element_flags);
	void AddBrushVec(BrushVec& brushVec);
	
#ifdef BMAN_EDITOR
	const  VerboseStat * GetVerboseStat() 
	{
		return &m_VerboseStat;
	}
	int			GetBrushesCount() const 
	{
		if (m_pRootNode)
		{
			return  (int)m_pRootNode->brushes.size();
		}
		else
		{
			return 0;
		}
	}
	BrushNode * Pickup(const A3DVECTOR3& pos);

#endif
	BrushNode * GetRootNode() const
	{
		return m_pRootNode;
	}


private:
	void _ImportNode(FILE * fp, BrushNode * node, bool& isLeaf);
	void _AddBrush(BrushNode * node, CCDBrush * brush);
	bool _TestOverlap(BrushNode * node, CCDBrush * brush);
	void _Split(BrushNode* node, bool recursive);
	bool _Trace(BrushNode* node, BrushTraceInfo * trcInfo,const abase::vector<char>* element_flags);
#ifdef BMAN_EDITOR
	BrushNode * _Pickup(BrushNode * node, const A3DVECTOR3& pos);
#endif

private:
	BrushNode * m_pRootNode;
	BrushVec    m_aBrushes;
#ifdef BMAN_EDITOR
	VerboseStat              m_VerboseStat;
#endif	
};

}


#endif

