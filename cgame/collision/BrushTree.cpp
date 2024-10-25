#include <ASSERT.h>


#include "BrushTree.h"
#include "CDBrush.h"
#include "ConvexHullData.h"
#include "A3DCollision.h"



namespace SvrCD
{
//const var
const DWORD BRUSH_TREE_FILE_MAGIC = (DWORD)(('b'<<24)| ('t'<<16)|('r'<<8)|('f'));
const DWORD BRUSH_TREE_FILE_VERSION = 1;
const float EXTENDS_Y = 5000.0f;
const float MIN_NODE_SIZE = 16.0f;


//local func
CCDBrush ** linear_find(BrushVec& vec, CCDBrush * target)
{
	BrushVec::iterator  it;
	for (it = vec.begin(); it != vec.end(); ++it)
	{
		if (*it == target)
		{
			break;
		}
	}

	return it;
}

void ReleaseBrushVec(BrushVec& brushVec)
{
	BrushVec::iterator it;
	for (it = brushVec.begin(); it != brushVec.end(); ++it)
	{
		  (*it)->Release();
		  delete *it;
	}
	brushVec.clear();
}



//class CBrushTree
CBrushTree::CBrushTree()
:m_pRootNode(NULL)
{

}

CBrushTree::~CBrushTree()
{

}


void CBrushTree::Release()
{
	if(m_pRootNode)
	{
		delete m_pRootNode;
		m_pRootNode = NULL;
	}
	
	ReleaseBrushVec(m_aBrushes);
}

void CBrushTree::Build(const A3DVECTOR3& ext)
{
	ASSERT(m_pRootNode == NULL);

	m_pRootNode = new BrushNode;
	A3DVECTOR3 vExtents(ext);
	vExtents.y =  EXTENDS_Y;
	m_pRootNode->aabb.Center = A3DVECTOR3(0.0f);
	m_pRootNode->aabb.Extents = vExtents;
	m_pRootNode->aabb.CompleteMinsMaxs();

	_Split(m_pRootNode, true);
}


bool CBrushTree::Load(const char * szName)
{
	Release();

	FILE * fp = fopen(szName, "rb");
	if(!fp)
	{
		return false;
	}
	DWORD flag;
	fread(&flag, sizeof(DWORD), 1, fp);
	if( flag != BRUSH_TREE_FILE_MAGIC)
	{
		//note: log it?
		fclose(fp);
		return false;	
	}
	fread(&flag, sizeof(DWORD), 1, fp);
	if( flag != BRUSH_TREE_FILE_VERSION)
	{
		//note: log it?
		fclose(fp);
		return false;	
	}
	//read brush
	int count;
	fread(&count, sizeof(int), 1, fp);
	
	int i;
	for (i = 0; i < count; ++i)
	{
		CCDBrush * pBrush = new CCDBrush;
		pBrush->Load(fp);
		m_aBrushes.push_back(pBrush);
	}

	//tree
	m_pRootNode = new BrushNode;
	abase::vector<BrushNode *>  nodes;
	nodes.push_back(m_pRootNode);
	while (!nodes.empty())
	{
		abase::vector<BrushNode *> children;

		for (i =0; i < (int)nodes.size(); ++i)
		{
			BrushNode * cur_node = nodes[i];
			bool is_leaf;
			_ImportNode(fp, cur_node, is_leaf);
			
			if (!is_leaf)
			{
				int j;
				if (cur_node->IsLeaf())
				{
					for (j = 0; j < 4; ++j)
					{
						cur_node->children[j] = new BrushNode;
					}
				}


				for (j = 0; j < 4; ++j)
				{
					BrushNode * child = cur_node->children[j];
					children.push_back(child);	
				}
			}
		}
		nodes = children;
	}
	fclose(fp);	
	
	//add brush
	count = (int)m_aBrushes.size();
	for(i = 0; i < count; ++i )
	{
		_AddBrush(m_pRootNode, m_aBrushes[i]);
	}
	return true;
}


void CBrushTree::_ImportNode(FILE * fp, BrushNode * node, bool& isLeaf)
{
	fread(&node->aabb.Center.x, sizeof(float), 1, fp);
	fread(&node->aabb.Center.z, sizeof(float), 1, fp);
	fread(&node->aabb.Extents.x, sizeof(float),1, fp);
	fread(&node->aabb.Extents.z, sizeof(float),1, fp);

	node->aabb.Center.y = 0;
	node->aabb.Extents.y = EXTENDS_Y;
	node->aabb.CompleteMinsMaxs();
	

	bool b;
	fread(&b, sizeof(bool), 1, fp);
	node->RoI = b;
	fread(&b, sizeof(bool), 1, fp);
	isLeaf = b;
}

void CBrushTree::_AddBrush(BrushNode * node, CCDBrush * brush)
{
	if (!_TestOverlap(node, brush))
	{
		return;
	}
#ifdef BMAN_EDITOR
//#if 0
	BrushVec::iterator it = linear_find(node->brushes, brush);
	if (it != node->brushes.end())
	{
		return;
	}
#endif
	node->brushes.push_back(brush);
	if (node->IsLeaf())
	{
		return;
	}
	int j;
	for (j = 0; j < 4; ++j)
	{
		_AddBrush(node->children[j], brush);
	}
}


bool CBrushTree::_TestOverlap(BrushNode * node, CCDBrush * brush)
{
	if (!CLS_AABBAABBOverlap(node->aabb.Center, node->aabb.Extents, brush->GetAABB().Center, brush->GetAABB().Extents))
	{
		return false;
	}

	BrushTraceInfo trc_info;
	trc_info.Init(node->aabb.Center, A3DVECTOR3(0.0f), node->aabb.Extents);
	//trc_info.Init(node->aabb.Center, A3DVECTOR3(0.0f, 0.1f, 0.0f), node->aabb.Extents);
	return brush->Trace(&trc_info);
}

void CBrushTree::AddBrushVec(BrushVec& brushVec)
{

	if (!m_pRootNode)
	{
		ASSERT(0);
		return;
	}

	int count = (int)brushVec.size();
	int i;
	for (i = 0; i < count; ++i)
	{
		m_aBrushes.push_back(brushVec[i]);
		_AddBrush(m_pRootNode, brushVec[i]);
	}
}

//   ----------------------
//	 |          |         |
//   |     1    |     2   |
//   |          |         |
//   ----------------------
//	 |          |         |
//   |     0    |     3   |
//   |          |         |
//   ----------------------
void CBrushTree::_Split(BrushNode* node, bool recursive)
{
	ASSERT(node->IsLeaf());

	if (node->aabb.Extents.x * 2 < MIN_NODE_SIZE + 0.1f)
	{
		return;
	}

	
	ASSERT(node->children[0] == NULL);

	A3DVECTOR3 child_ext = node->aabb.Extents * 0.5f;
	child_ext.y = EXTENDS_Y;
	
	node->children[0] = new BrushNode;
	node->children[0]->aabb.Center.x   = node->aabb.Center.x - child_ext.x;
	node->children[0]->aabb.Center.y   = 0.0f;
	node->children[0]->aabb.Center.z   = node->aabb.Center.z - child_ext.z;
	node->children[0]->aabb.Extents = child_ext;
	node->children[0]->aabb.CompleteMinsMaxs();
	node->children[0]->RoI = node->RoI;

	node->children[1] = new BrushNode;
	node->children[1]->aabb.Center.x   = node->aabb.Center.x - child_ext.x;
	node->children[1]->aabb.Center.y   = 0.0f;
	node->children[1]->aabb.Center.z   = node->aabb.Center.z + child_ext.z;
	node->children[1]->aabb.Extents = child_ext;
	node->children[1]->aabb.CompleteMinsMaxs();
	node->children[1]->RoI = node->RoI;

	node->children[2] = new BrushNode;
	node->children[2]->aabb.Center.x   = node->aabb.Center.x + child_ext.x;
	node->children[2]->aabb.Center.y   = 0.0f;
	node->children[2]->aabb.Center.z   = node->aabb.Center.z + child_ext.z;
	node->children[2]->aabb.Extents = child_ext;
	node->children[2]->aabb.CompleteMinsMaxs();
	node->children[2]->RoI = node->RoI;

	node->children[3] = new BrushNode;
	node->children[3]->aabb.Center.x   = node->aabb.Center.x + child_ext.x;
	node->children[3]->aabb.Center.y   = 0.0f;
	node->children[3]->aabb.Center.z   = node->aabb.Center.z - child_ext.z;
	node->children[3]->aabb.Extents = child_ext;
	node->children[3]->aabb.CompleteMinsMaxs();
	node->children[3]->RoI = node->RoI;

	if (recursive)
	{
		int j;
		for (j = 0; j < 4; ++j)
		{
			BrushNode * child = node->children[j];
			_Split(child, recursive);
		}
	}
}


bool CBrushTree::Trace(BrushTraceInfo * trcInfo,const abase::vector<char>* element_flags)
{
#ifdef BMAN_EDITOR
	m_VerboseStat.nCheckBrushes = 0;
#endif
	if (!m_pRootNode)
	{
		return false;
	}

	return _Trace(m_pRootNode, trcInfo, element_flags);
}


bool CBrushTree::_Trace(BrushNode* node, BrushTraceInfo * trcInfo, const abase::vector<char>* element_flags)
{
	if (m_aBrushes.empty())
	{
		return false;
	}
	
	A3DVECTOR3 vDummyPoint, vDummyNormal;
	float fDummyFraction;

	if(trcInfo->bIsPoint && !CLS_RayToAABB3(trcInfo->vStart, trcInfo->vDelta, 
		node->aabb.Mins, node->aabb.Maxs, vDummyPoint, &fDummyFraction, vDummyNormal))
	{
		return false;
	}
	
	if(!trcInfo->bIsPoint && !CLS_AABBAABBOverlap(node->aabb.Center, node->aabb.Extents, 
		trcInfo->BoundAABB.Center, trcInfo->BoundAABB.Extents))
	{
		return false;
	}
	bool bCollide = false;
	bool		bStartSolid ;	
	bool		bAllSolid ;		
	int			iClipPlane ;	
	A3DVECTOR3 vNormal; 
	float       fDist ;	
	float fFraction = 100.0f ;

	
	if(node->IsLeaf())
	{
		int i;
		for (i = 0; i < (int)node->brushes.size(); ++i)
		{
				CCDBrush * pBrush = node->brushes[i];
				DWORD dwFlags = pBrush->GetReservedDWORD();
				//add by liuguichen
				if(element_flags && pBrush->GetId() > 0)
				{
					if((*element_flags)[pBrush->GetId()])
						dwFlags &= ~CHDATA_SKIP_COLLISION;	
					else
						dwFlags |= CHDATA_SKIP_COLLISION;
				}
				if((trcInfo->bIsPoint && (dwFlags & CHDATA_SKIP_RAYTRACE)) ||
		       (!trcInfo->bIsPoint && (dwFlags & CHDATA_SKIP_COLLISION) ))
				{
					continue;
				}
				#ifdef BMAN_EDITOR
					m_VerboseStat.nCheckBrushes++;
				#endif				
				
				if (pBrush->Trace(trcInfo) && (trcInfo->fFraction < fFraction)) 
				{
						//update the saving info
						bStartSolid = trcInfo->bStartSolid;
						bAllSolid = trcInfo->bAllSolid;
						iClipPlane = trcInfo->iClipPlane;
						fFraction = trcInfo->fFraction;
						vNormal = trcInfo->ClipPlane.GetNormal();
						fDist = trcInfo->ClipPlane.GetDist();
						bCollide = true;
						//if (trcInfo->fFraction == 0.0f)
						if (bStartSolid)
						{
							break;
						}
					}
		 }
	}
	else
	{
		int j;
		for (j = 0; j < 4; ++j)
		{
			 if (_Trace(node->children[j], trcInfo, element_flags) && trcInfo->fFraction < fFraction)
			 	{
			 			bStartSolid = trcInfo->bStartSolid;
						bAllSolid = trcInfo->bAllSolid;
						iClipPlane = trcInfo->iClipPlane;
						fFraction = trcInfo->fFraction;
						vNormal = trcInfo->ClipPlane.GetNormal();
						fDist = trcInfo->ClipPlane.GetDist();
						bCollide = true;
						//if (trcInfo->fFraction == 0.0f)
						if (bStartSolid)
						{
							break;
						}
			 	}
		}
	}


	if (bCollide)
	{
		//set back
		trcInfo->bStartSolid = bStartSolid;
		trcInfo->bAllSolid = bAllSolid;
		trcInfo->iClipPlane = iClipPlane;
		trcInfo->fFraction = fFraction;
		trcInfo->ClipPlane.SetNormal(vNormal);
		trcInfo->ClipPlane.SetD(fDist);
	}
	return bCollide;	
}


#ifdef BMAN_EDITOR
BrushNode * CBrushTree::Pickup(const A3DVECTOR3& pos)
{
	if (!m_pRootNode)
	{
		return NULL;
	}
	return _Pickup(m_pRootNode, pos);	
}

BrushNode * CBrushTree::_Pickup(BrushNode * node, const A3DVECTOR3& pos)
{
	if (node->IsLeaf() )
	{
		if (node->IsIn(pos.x, pos.z))
		{
			return node;
		}
		else
		{
			return NULL;
		}
	}

	int j;
	for (j = 0; j < 4; ++j)
	{
		BrushNode * child = node->children[j];

		BrushNode * pick = _Pickup(child, pos);
		if (pick != NULL)
		{
			return pick;
		}
	}

	return NULL;
}
#endif


}
