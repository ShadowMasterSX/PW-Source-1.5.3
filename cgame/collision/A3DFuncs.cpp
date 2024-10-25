/*
 * FILE: A3DTypes.cpp
 * 
 * DESCRIPTION: Implements the overloads of some structures
 *
 * CREATED BY: Hedi, 2001/7/27
 *
 * HISTORY:
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.	
 */


#include "A3DFuncs.h"



// Return min/max vector composed with min/max component of the input 2 vector
A3DVECTOR3 a3d_VecMin(const A3DVECTOR3& v1, const A3DVECTOR3& v2)
{
	return A3DVECTOR3(min2(v1.x, v2.x), min2(v1.y, v2.y), min2(v1.z, v2.z));
}

A3DVECTOR3 a3d_VecMax(const A3DVECTOR3& v1, const A3DVECTOR3& v2)
{
	return A3DVECTOR3(max2(v1.x, v2.x), max2(v1.y, v2.y), max2(v1.z, v2.z));
}


A3DMATRIX4 a3d_TransformMatrix(const A3DVECTOR3& vecDir, const A3DVECTOR3& vecUp, const A3DVECTOR3& vecPos)
{
    A3DMATRIX4   mat;
    A3DVECTOR3   vecXAxis, vecYAxis, vecZAxis;

    vecZAxis = a3d_Normalize(vecDir);
    vecYAxis = a3d_Normalize(vecUp);
    vecXAxis = a3d_Normalize(CrossProduct(vecYAxis, vecZAxis));

    memset(&mat, 0, sizeof(mat));
    mat.m[0][0] = vecXAxis.x;
    mat.m[0][1] = vecXAxis.y;
    mat.m[0][2] = vecXAxis.z;

    mat.m[1][0] = vecYAxis.x;
    mat.m[1][1] = vecYAxis.y;
    mat.m[1][2] = vecYAxis.z;

    mat.m[2][0] = vecZAxis.x;
    mat.m[2][1] = vecZAxis.y;
    mat.m[2][2] = vecZAxis.z;

    mat.m[3][0] = vecPos.x;
    mat.m[3][1] = vecPos.y;
    mat.m[3][2] = vecPos.z;
    mat.m[3][3] = 1.0f;

    return mat;
}



