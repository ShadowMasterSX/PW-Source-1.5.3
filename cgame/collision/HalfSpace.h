#ifndef _HALF_SPACE_H_
#define _HALF_SPACE_H_

#include <a3dvector.h>

class A3DMATRIX4;

namespace SvrCD
{

//class halfspace
////////////////////////////////////////////////////////////////////////////
// 
// 平面方程的定义为N.X=D，在本类中的形式为 m_vNormal.X=m_d
// 注意：这与一般的平面方程Ax+By+Cz+D=0不一样，区别在于D的正负！
// 
////////////////////////////////////////////////////////////////////////////

class CHalfSpace
{
public:

	CHalfSpace(){}
	virtual ~CHalfSpace() {}
	CHalfSpace(const CHalfSpace& hs):m_vNormal(hs.m_vNormal) { m_d=hs.m_d; }

	A3DVECTOR3 GetNormal() const {return m_vNormal;}
	float GetDist() const { return m_d;}
	
	
	//直接设置法向
	void SetNormal(const A3DVECTOR3& n) { m_vNormal=n;m_vNormal.Normalize();}
	//直接设置距离d
	void SetD(float d) { m_d=d;}

	// 对决定Halfspace的平面进行变换！变换矩阵为mtxTrans
	virtual void Transform(const A3DMATRIX4& mtxTrans);

//Attributes
protected:
	A3DVECTOR3 m_vNormal;
	float m_d;

};

}
#endif
