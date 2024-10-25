#include "Face.h"
#include "A3DMatrix.h"



namespace SvrCD
{

CFace::CFace()
{

}

CFace::~CFace()
{

}

CFace::CFace(const CFace& face)
:CHalfSpace(face)
{
		
	int i;
	for(i=0;i<face.GetVNum();i++)
	{
		CHalfSpace hs=face.GetEdgeHalfSpace(i);
		AddElement(face.GetVID(i),face.GetEdgeHalfSpace(i));
	}
	
	for(i=0;i<face.GetExtraHSNum();i++)
	{
		AddExtraHS(face.GetExtraHalfSpace(i));
	}

}

void CFace::Transform(const A3DMATRIX4& mtxTrans)
{
	//调用父类方法变换所在平面的方程
	CHalfSpace::Transform(mtxTrans);

	int i;
	for(i=0;i<(int)m_arrEdgeHSs.size();i++)
	{
		m_arrEdgeHSs[i].Transform(mtxTrans);
	}

	for(i=0;i<(int)m_arrExtraHSs.size();i++)
	{
		m_arrExtraHSs[i].Transform(mtxTrans);
	}

}

}

