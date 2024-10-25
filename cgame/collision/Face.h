#ifndef _FACE_H_
#define _FACE_H_

#include <a3dvector.h>
#include "HalfSpace.h"
#include <vector.h>

class A3DMATRIX4;

namespace SvrCD
{

class CFace:public CHalfSpace
{
public:
	CFace();
	CFace(const CFace& face);	

	virtual ~CFace();

	//添加一个元素，必须同时添加一个vid和该vid与下一个id构成的边所对应的HalfSpace
	void AddElement(int vid,const CHalfSpace& hs){ m_arrVIDs.push_back(vid);m_arrEdgeHSs.push_back(hs);}
	void AddExtraHS(const CHalfSpace& hs) { m_arrExtraHSs.push_back(hs); }
	// 对面片进行变换！变换矩阵为mtxTrans
	virtual	void Transform(const A3DMATRIX4& mtxTrans);
	int GetVNum() const { return  (int)m_arrVIDs.size();}
	int GetVID(int i) const { return m_arrVIDs[i];}
	CHalfSpace GetEdgeHalfSpace(int eid) const { return m_arrEdgeHSs[eid];}
	CHalfSpace GetExtraHalfSpace(int id) const { return m_arrExtraHSs[id];}
	int GetExtraHSNum() const { return (int)m_arrExtraHSs.size(); }

private:
	//AArray<int, int> m_arrVIDs;						//按顺序记录顶点的索引id；
	//AArray<CHalfSpace,const CHalfSpace&> m_arrEdgeHSs;		//按顺序（同上）记录一条边和该面的法向决定的halfspace
	//AArray<CHalfSpace,const CHalfSpace&> m_arrExtraHSs;		//处理尖锐夹角情况时附加的一组Hs!

	abase::vector<int> m_arrVIDs;						//按顺序记录顶点的索引id；
	abase::vector<CHalfSpace> m_arrEdgeHSs;		//按顺序（同上）记录一条边和该面的法向决定的halfspace
	abase::vector<CHalfSpace> m_arrExtraHSs;		//处理尖锐夹角情况时附加的一组Hs!
};


}
#endif

