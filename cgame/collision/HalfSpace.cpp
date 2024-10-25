#include "HalfSpace.h"
#include "A3DMatrix.h"

namespace SvrCD
{
//////////////////////////////////////////////////////////////////////
// Definitions of Members of CHalfSpace
//////////////////////////////////////////////////////////////////////

void CHalfSpace::Transform(const A3DMATRIX4& mtxTrans)
{
	//从mtxTrans分解出Scale,Rotate,Translate分量
	A3DVECTOR3 vTranslate=mtxTrans.GetRow(3);
	float fScale=mtxTrans.GetCol(0).Magnitude();
	
	// 与mtxTrans一样，仍定义为右乘矩阵
	A3DMATRIX3 mtx3Rotate;		
	for(int i=0;i<3;i++)
		for(int j=0;j<3;j++)
			mtx3Rotate.m[i][j]=mtxTrans.m[i][j]/fScale;
	
	//对于平面方程N.X=D的变换(s,R,t)如下：

	// N'=NR
	SetNormal(m_vNormal*mtx3Rotate);

	//D'=s*D+N'.t
	SetD(fScale*m_d+DotProduct(m_vNormal,vTranslate));
}


}

