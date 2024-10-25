/*
 * FILE: A3DFuncs.h
 *
 * DESCRIPTION: some data operation routines for Angelica 3D Engine
 *
 * CREATED BY: Hedi, 2001/7/27
 *
 * HISTORY:
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.	
 */

#ifndef _A3DFUNCS_H_
#define _A3DFUNCS_H_

#include <a3dvector.h>
#include "A3DMatrix.h"
#include "ABaseDef.h"


#define min2(a, b) (((a) > (b)) ? (b) : (a))
#define min3(a, b, c) (min2(min2((a), (b)), (c)))
#define max2(a, b) (((a) > (b)) ? (a) : (b))
#define max3(a, b, c) (max2(max2((a), (b)), (c)))
#define min4(a, b, c, d) (min2(min2((a), (b)), min2((c), (d))))
#define max4(a, b, c, d) (max2(max2((a), (b)), max2((c), (d))))

//	Returns vector with same direction and unit length
inline A3DVECTOR3 a3d_Normalize(const A3DVECTOR3& v)
{
	FLOAT mag = v.Magnitude();
	if (mag < 1e-12	&& mag > -1e-12)
		return A3DVECTOR3(0.0f);
	else
		return v / mag;
}

//	Return min/max vector composed with min/max component of the input 2 vector
A3DVECTOR3 a3d_VecMin(const A3DVECTOR3& v1, const A3DVECTOR3& v2); 
A3DVECTOR3 a3d_VecMax(const A3DVECTOR3& v1, const A3DVECTOR3& v2); 

//	Matrix initialize;
A3DMATRIX4 a3d_TransformMatrix(const A3DVECTOR3& vecDir, const A3DVECTOR3& vecUp, const A3DVECTOR3& vecPos);


#endif	//	_A3DFUNCS_H_

