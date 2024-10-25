/*
 * FILE: A3DCollision.h
 *
 * DESCRIPTION: Some common routines for collision detection
 *
 * CREATED BY: duyuxin, 2001/10/13
 *
 * HISTORY: kuiwu, 2008/4/21, transfer to server(linux env)
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.	
 */

#ifndef _A3DCOLLISION_H_
#define _A3DCOLLISION_H_

#include <a3dvector.h>
#include "ABaseDef.h"

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

bool CLS_RayToAABB3(const A3DVECTOR3& vStart, const A3DVECTOR3& vDelta, const A3DVECTOR3& vMins,
					const A3DVECTOR3& vMaxs, A3DVECTOR3& vPoint, FLOAT* pfFraction, A3DVECTOR3& vNormal);

bool CLS_AABBAABBOverlap(const A3DVECTOR3& vCenter1, const A3DVECTOR3& vExt1, const A3DVECTOR3& vCenter2, const A3DVECTOR3& vExt2);



#endif	//	_A3DCOLLISION_H_

