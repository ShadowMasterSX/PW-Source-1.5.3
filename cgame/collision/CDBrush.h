/*
 * FILE: aabbcd.h
 *
 * DESCRIPTION: 
 *
 * CREATED BY: wang kuiwu, 2005/8/6
 *
 * HISTORY: 
 *
 * Copyright (c) 2005 Archosaur Studio, All Rights Reserved.
 */
#ifndef  _CD_BRUSH_H_
#define _CD_BRUSH_H_


#include "ABaseDef.h"
#include <a3dvector.h>
#include "A3DGeometry.h"
#include "HalfSpace.h"

namespace SvrCD
{
const int    MAX_FACE_IN_HULL = 200;


class BrushTraceInfo 
{
public:
	//////////////////////////////////////////////////////////////////////////
	//	In
	//////////////////////////////////////////////////////////////////////////
	// aabb's info
	A3DVECTOR3	vStart;			//	Start point
	A3DVECTOR3	vDelta;			//	Move delta
  A3DVECTOR3  vExtents; 	
	A3DAABB     BoundAABB;  
	// Dist Epsilon
	float		fDistEpsilon;		//	Dist Epsilon
	bool        bIsPoint;          //raytrace
	//////////////////////////////////////////////////////////////////////////
	//	Out
	//////////////////////////////////////////////////////////////////////////
	bool		bStartSolid;	//	Collide something at start point
	bool		bAllSolid;		//	All in something
	CHalfSpace	ClipPlane;		//	Clip plane
	int			iClipPlane;		//	Clip plane's index
	float		fFraction;		//	Fraction

	void Init(const A3DVECTOR3& start, const A3DVECTOR3& delta,  const A3DVECTOR3& ext,  bool bRay = false, float epsilon = 0.03125f ){
		vStart = start;
		vDelta = delta;
		vExtents = ext;
		fDistEpsilon = epsilon;
		fFraction = 100.0f;
		bIsPoint = bRay;
		bStartSolid = false;
		bAllSolid = false;
		if (!bIsPoint) {
			BoundAABB.Clear();
			BoundAABB.AddVertex(vStart);
			BoundAABB.AddVertex(vStart + vExtents);
			BoundAABB.AddVertex(vStart - vExtents);
			BoundAABB.AddVertex(vStart+ vDelta);
			BoundAABB.AddVertex(vStart + vDelta + vExtents);
			BoundAABB.AddVertex(vStart + vDelta - vExtents);
			BoundAABB.CompleteCenterExts();
		}
		else{
			//@note : it cheats the caller. By Kuiwu[25/8/2005]
			//@todo : refine me. By Kuiwu[25/8/2005]
			fDistEpsilon = 1E-5f;
		}
	}
};

class CCDSide
{
public:
	CHalfSpace  plane;
	bool        bevel;

//operation
public:
	CCDSide(){};
	CCDSide(CHalfSpace& hs, bool bBevel): plane(hs), bevel(bBevel){
	}
	void Init(CHalfSpace& hs, bool bBevel){
		plane = hs;
		bevel = bBevel;
	}
	void Init(A3DVECTOR3& vNormal, float fDist, bool bBevel){
		plane.SetNormal(vNormal);
		plane.SetD(fDist);
		bevel = bBevel;
	}

};

class CCDBrush
{
	friend class CQBrush;

public:
	
	// constructor, deconstructor and releaser
	CCDBrush():m_pSides(NULL),m_nSides(0),m_dwReserved(0),m_Id(0){}
	~CCDBrush(){ Release();}

	void Release() 
	{
		if(m_pSides)
		{
			delete[] m_pSides;
			m_pSides = NULL;
		}
		m_nSides = 0;
	}
	
	CCDBrush * Clone();

	A3DAABB GetAABB()const
	{
		return m_aabb;
	}

	DWORD GetReservedDWORD()
	{
		return m_dwReserved;
	}

	void SetReservedDWORD(DWORD dwReserved)
	{
		m_dwReserved = dwReserved;	
	}

	int GetId()
	{
		return m_Id;
	}
	
	void SetId(int id)
	{
		m_Id = id;
	}

	// Load and save method
	bool Load(FILE * pFileToLoad);
	/*
	 *	trace with aabb or ray
	 *  @param  pInfo: the trace info (both input and output) @ref class BrushTraceInfo
	 *  @return  false if no collision 
	 */
	bool Trace(BrushTraceInfo * pInfo);
	
	int  GetSlideCount() const
	{
		return m_nSides;
	}
	const CCDSide * GetSlide(int index) const
	{
		return &m_pSides[index];
	}
private:

	CCDSide             * m_pSides;
	int					m_nSides;
	
	A3DAABB				m_aabb;

	DWORD				m_dwReserved;
	//add by liuguichen
	int					m_Id;
};

class CConvexHullData;

class  CQBrush
{
public:
	CQBrush();
	~CQBrush();
	void Release();
	void AddBrushBevels(CConvexHullData * pCHData);
	CConvexHullData * GetCHData()
	{
		return m_pCHData;
	}
	int GetSideNum()
	{
		return m_nQPlane;
	}
	
	CHalfSpace * GetSide(int i)
	{
		return m_pQPlane[i].pHS;
	}

	bool IsBevel(int i)
	{
		return m_pQPlane[i].bBevel;
	}

	void Transform(const A3DMATRIX4& mtxTrans);

	void Export(CCDBrush *pCDBrush);
private:
	CConvexHullData * m_pCHData;
	
	struct QPlane 
	{
		CHalfSpace * pHS;
		int          iCHIndex;
		bool         bBevel;
		//////////////////////////////////////////////////////////////////////////
		//debug
		A3DVECTOR3    vert;
		//////////////////////////////////////////////////////////////////////////
	};
	
	QPlane  m_pQPlane[MAX_FACE_IN_HULL];
	int    m_nQPlane;

	bool   m_bCDReady;
	CCDBrush * m_pCDBrush;


	void _FlushCH();


};



}

#endif
