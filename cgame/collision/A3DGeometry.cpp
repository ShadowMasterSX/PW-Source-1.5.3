/*
 * FILE: A3DGeometry.cpp
 *
 * DESCRIPTION: 
 *
 * CREATED BY: Duyuxin, 2004/4/9
 *
 * HISTORY: kuiwu, 2008/4/21, transfer to server(linux env)
 *
 * Copyright (c) 2004 Archosaur Studio, All Rights Reserved.
 */


#include "A3DGeometry.h"
#include "A3DFuncs.h"
///////////////////////////////////////////////////////////////////////////
//	
//	Define and Macro
//	
///////////////////////////////////////////////////////////////////////////


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
//	Implement A3DAABB
//	
///////////////////////////////////////////////////////////////////////////

//	Add a vertex to aabb
void A3DAABB::AddVertex(const A3DVECTOR3& v)
{
	for (int i=0; i < 3; i++)
	{
		if (v.m[i] < Mins.m[i])
			Mins.m[i] = v.m[i];
	
		if (v.m[i] > Maxs.m[i])
			Maxs.m[i] = v.m[i];
	}
}

//	Check whether another aabb is in this aabb
bool A3DAABB::IsAABBIn(const A3DAABB& aabb) const
{
	A3DVECTOR3 vDelta = aabb.Center - Center;

	vDelta.x = (float)fabs(vDelta.x);
	if (vDelta.x + aabb.Extents.x > aabb.Extents.x)
		return false;

	vDelta.y = (float)fabs(vDelta.y);
	if (vDelta.y + aabb.Extents.y > aabb.Extents.y)
		return false;

	vDelta.z = (float)fabs(vDelta.z);
	if (vDelta.z + aabb.Extents.z > aabb.Extents.z)
		return false;

	return true;
}

//	Build AABB from vertices
void A3DAABB::Build(const A3DVECTOR3* aVertPos, int iNumVert)
{
	Clear();

	if (iNumVert <= 0)
		return;

	for (int i=0; i < iNumVert; i++)
	{
		Maxs = a3d_VecMax(Maxs, aVertPos[i]);
		Mins = a3d_VecMin(Mins, aVertPos[i]);
	}

	CompleteCenterExts();
}

///////////////////////////////////////////////////////////////////////////
//	
//	Implement A3DOBB
//	
///////////////////////////////////////////////////////////////////////////

