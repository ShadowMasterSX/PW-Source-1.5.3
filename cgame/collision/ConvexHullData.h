#ifndef _CONVEX_HULL_DATA_H_
#define _CONVEX_HULL_DATA_H_

#include "ABaseDef.h"
#include <a3dvector.h>
#include "A3DGeometry.h"
#include <vector.h>

#ifdef BMAN_EDITOR
#include "A3DFlatCollector.h"
#endif

class A3DMATRIX4;

namespace SvrCD
{


// a set of DWORD flags
#define	CHDATA_BAD_FIT				0x00000001		//标志凸包对模型的近似效果不好，需要做进一步的模型碰撞，在CHEditor中用到。
#define CHDATA_NPC_CANNOT_PASS		0x00000002		//目前用于地形编辑器中，标志该凸包是否让npc可通过。
#define CHDATA_SKIP_COLLISION		0x00000004		//标志是否在碰撞检测中忽略此凸包
#define CHDATA_SKIP_RAYTRACE		0x00000008		//标志是否在射线跟踪中忽略此凸包


class CFace;

class CConvexHullData  
{
public:
	
	CConvexHullData();
	CConvexHullData(const CConvexHullData& CHData);
	~CConvexHullData();
	

	bool LoadBinaryData(FILE* InFile);
	// 对ConvexHullData进行坐标变换！变换矩阵为mtxTrans
	// 注：不能处理任意变换，只能是带有相同scale值的刚性变换
	void Transform(const A3DMATRIX4& mtxTrans);

	int GetVertexNum() const { return (int)m_arrVertices.size(); }
	A3DVECTOR3 GetVertex(int vid) const { return m_arrVertices[vid];}
	int GetFaceNum() const { return (int)m_arrFaces.size();}
	CFace* GetFacePtr(int fid)  const { return m_arrFaces[fid];}
	DWORD GetFlags() { return m_dwFlags; }

	// build the aabb at runtime.
	bool GetAABB(A3DAABB& aabb)
	{
		if ( GetVertexNum() < 2 ) return false;

		//aabb.Build( m_arrVertices.GetData(), m_arrVertices.GetSize());
		aabb.Build( m_arrVertices.begin(), (int)m_arrVertices.size());
		return true;
	}
	// get the aabb we precomputed.
	A3DAABB GetAABB() const
	{
		return m_aabb;
	}

	void AddVertex(const A3DVECTOR3& v) { m_arrVertices.push_back(v); }
	void AddFace(const CFace& f); 

#ifdef BMAN_EDITOR
	//绘制到pFC,其中，如果指定了pSpecialFace,将对其特殊显示
	void Render(A3DFlatCollector* pFC, bool bRenderV=false, CFace* pSpecialFace=NULL, DWORD dwVColor=0xffff0000, DWORD dwFColor=0xa0ffff00, const A3DVECTOR3& vRayDir=A3DVECTOR3(1.0f,1.0f,1.0f), const A3DMATRIX4* pTransMatrix=NULL)const;

#endif

private:
	bool _ReadFromBuf(char* buf);				//从buf中读出数据
	void _BuildAABB();

private:
	DWORD		m_dwFlags;
	//AArray<A3DVECTOR3,const A3DVECTOR3&> m_arrVertices;		//顶点列表，有序
	//AArray<CFace*, CFace*> m_arrFaces;						//所包含的面的列表，无序，只需提供遍历手段即可
	abase::vector<A3DVECTOR3> m_arrVertices;		//顶点列表，有序
	abase::vector<CFace*> m_arrFaces;						//所包含的面的列表，无序，只需提供遍历手段即可

	int m_iVer;			// 版本信息，用于存取操作
	A3DAABB m_aabb;		// keep record of convex hull's AABB.
};


}
#endif
