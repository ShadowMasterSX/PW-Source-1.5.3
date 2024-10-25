/*
 * FILE: A3DGeometry.h
 *
 * DESCRIPTION: 
 *
 * CREATED BY: Duyuxin, 2004/4/9
 *
 * HISTORY: kuiwu, 2008/4/21, transfer to server(linux env)
 *
 * Copyright (c) 2004 Archosaur Studio, All Rights Reserved.
 */

#ifndef _A3DGEOMETRY_H_
#define _A3DGEOMETRY_H_

#include "ABaseDef.h"
#include <a3dvector.h>

///////////////////////////////////////////////////////////////////////////
//	
//	Define and Macro
//	
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
//	
//	Types and Global variables
//	
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
//	
//	Declare of Global functions
//	
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
//	
//	Class A3DAABB
//	
///////////////////////////////////////////////////////////////////////////

//	Axis-Aligned Bounding Box
class A3DAABB
{
public:		//	Constructors and Destructors

	A3DAABB() {}

	A3DAABB(const A3DAABB& aabb) : 
	Center(aabb.Center),
	Extents(aabb.Extents),
	Mins(aabb.Mins),
	Maxs(aabb.Maxs) {}

	A3DAABB(const A3DVECTOR3& vMins, const A3DVECTOR3& vMaxs) :
	Center((vMins + vMaxs) * 0.5f),
	Extents((vMaxs - vMins)* 0.5f),
	Mins(vMins),
	Maxs(vMaxs)
	{
		
	}

public:		//	Attributes

	A3DVECTOR3	Center;
	A3DVECTOR3	Extents;
	A3DVECTOR3	Mins;
	A3DVECTOR3	Maxs;

public:		//	Operations

	//	Clear aabb
	void Clear()
	{
		Mins.Set(999999.0f, 999999.0f, 999999.0f);
		Maxs.Set(-999999.0f, -999999.0f, -999999.0f);
	}

	//	Add a vertex to aabb
	void AddVertex(const A3DVECTOR3& v);


	//	Compute Mins and Maxs
	void CompleteMinsMaxs()
	{
		Mins = Center - Extents;
		Maxs = Center + Extents;
	}

	//	Compute Center and Extents
	void CompleteCenterExts()
	{
		Center  = (Mins + Maxs) * 0.5f;
		Extents = Maxs - Center;
	}

	//	Check whether a point is in this aabb
	bool IsPointIn(const A3DVECTOR3& v) const
	{
		if (v.x > Maxs.x || v.x < Mins.x ||
			v.y > Maxs.y || v.y < Mins.y ||
			v.z > Maxs.z || v.z < Mins.z)
			return false;

		return true;
	}

	//	Check whether another aabb is in this aabb
	bool IsAABBIn(const A3DAABB& aabb) const;

	void Build(const A3DVECTOR3* aVertPos, int iNumVert);

};





///////////////////////////////////////////////////////////////////////////
//	
//	Inline functions
//	
///////////////////////////////////////////////////////////////////////////


#endif	//	_A3DGEOMETRY_H_
