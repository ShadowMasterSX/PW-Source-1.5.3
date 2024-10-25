/*
 * FILE: A3DCollision.cpp
 *
 * DESCRIPTION: Some common routines for collision detection
 *
 * CREATED BY: duyuxin, 2001/10/13
 *
 * HISTORY: kuiwu, 2008/4/21, transfer to server(linux env) 
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.	
 */

#include "A3DCollision.h"


///////////////////////////////////////////////////////////////////////////
//
//	Define and Macro
//
///////////////////////////////////////////////////////////////////////////

//	Epsilon for collision detection
#define EPSILON_COLLISION	0.0001f
#define	EPSILON_DISTANCE	0.01f

///////////////////////////////////////////////////////////////////////////
//
//	Reference to External variables and functions
//
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
//
//	Local Types and Variables and Global variables
//
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
//
//	Local functions
//
///////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////
//
//	Implement
//
///////////////////////////////////////////////////////////////////////////



/*	Check whether a ray collision with a 3D AABB, if true, calcualte the hit point.

	Return true if ray collision with AABB, otherwise return false.

	vStart: ray's start point.
	vDelta: ray's moving delta (just is vEnd-vStart, not to be normalized)
	vMins, vMaxs: 3D Axis-Aligned Bounding Box
	vPoint (out): receive collision point.
	pfFraction (out): hit fraction.
	vNormal (out): hit plane's normal if true is returned. If start point is laid in
					specified AABB,	vNormal will be (0, 0, 0);
*/
bool CLS_RayToAABB3(const A3DVECTOR3& vStart, const A3DVECTOR3& vDelta, const A3DVECTOR3& vMins,
					const A3DVECTOR3& vMaxs, A3DVECTOR3& vPoint, FLOAT* pfFraction, A3DVECTOR3& vNormal)
{
	A3DVECTOR3 vN, vMaxT(-1.0f, -1.0f, -1.0f);
	bool bInside = true;
	int i;
	float aSign[3];

	//	Search candidate plane
	for (i=0; i < 3; i++)
	{
		if (vStart.m[i] < vMins.m[i])
		{
			vPoint.m[i]	= vMins.m[i];
			bInside		= false;
			aSign[i]	= 1.0f;

			//	Calcualte T distance to candidate plane
			if (vDelta.m[i] != 0.0f)
				vMaxT.m[i] = (vMins.m[i] - vStart.m[i]) / vDelta.m[i];
		}
		else if (vStart.m[i] > vMaxs.m[i])
		{
			vPoint.m[i]	= vMaxs.m[i];
			bInside		= false;
			aSign[i]	= -1.0f;

			//	Calcualte T distance to candidate plane
			if (vDelta.m[i] != 0.0f)
				vMaxT.m[i] = (vMaxs.m[i] - vStart.m[i]) / vDelta.m[i];
		}
	}

	if (bInside)
	{
		vPoint		= vStart;
		*pfFraction = 0;
		vNormal		= A3DVECTOR3(0.0f);
		return true;
	}

	//	Get largest of the maxT's for final choice of intersection
	int iWhichPlane = 0;
	vN = A3DVECTOR3(-aSign[0], 0.0f, 0.0f);

	if (vMaxT.m[1] > vMaxT.m[iWhichPlane])
	{
		iWhichPlane = 1;
		vN = A3DVECTOR3(0.0f, -aSign[1], 0.0f);
	}

	if (vMaxT.m[2] > vMaxT.m[iWhichPlane])
	{
		iWhichPlane = 2;
		vN = A3DVECTOR3(0.0f, 0.0f, -aSign[2]);
	}

	//	Check final candidate actually inside box
	if (vMaxT.m[iWhichPlane] < 0)
		return false;

	for (i=0; i < 3; i++)
	{
		if (i != iWhichPlane)
		{
			vPoint.m[i] = vStart.m[i] + vMaxT.m[iWhichPlane] * vDelta.m[i];

			if (vPoint.m[i] < vMins.m[i] - EPSILON_COLLISION ||
				vPoint.m[i] > vMaxs.m[i] + EPSILON_COLLISION)
				return false;
		}
	}

	*pfFraction = vMaxT.m[iWhichPlane];
	vNormal		= vN;

	return true;
}



/*	AABB-AABB intersection routine.

	Return true if two AABB collision, otherwise return false.

	vCenter1: first AABB's center.
	vExt1: first AABB's extents.
	vCenter1: second AABB's center.
	vExt1: second AABB's extents.
*/	
bool CLS_AABBAABBOverlap(const A3DVECTOR3& vCenter1, const A3DVECTOR3& vExt1, 
						 const A3DVECTOR3& vCenter2, const A3DVECTOR3& vExt2)
{
	float fDist;

	//	X axis
	fDist = vCenter1.x - vCenter2.x;
	if (fDist < 0)
		fDist = -fDist;
	
	if (vExt1.x + vExt2.x < fDist)
		return false;

	//	Y axis
	fDist = vCenter1.y - vCenter2.y;
	if (fDist < 0)
		fDist = -fDist;
	
	if (vExt1.y + vExt2.y < fDist)
		return false;

	//	Z axis
	fDist = vCenter1.z - vCenter2.z;
	if (fDist < 0)
		fDist = -fDist;
	
	if (vExt1.z + vExt2.z < fDist)
		return false;

	return true;
}

