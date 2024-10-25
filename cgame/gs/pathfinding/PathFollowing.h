/*
 * FILE: PathFollowing.h
 *
 * DESCRIPTION:		a class which can generate the position after one step 
 *					when following a path which consists of set of line segments.
 *
 * CREATED BY:		He wenfeng, 2005/10/20
 *
 * HISTORY: 
 *
 * Copyright (c) 2005 Archosaur Studio, All Rights Reserved.
 */

#ifndef _PATHFOLLOWING_H_
#define _PATHFOLLOWING_H_

#include <a3dvector.h>
#include <vector.h>
using namespace abase;

class CPathFollowing
{

struct LineSeg
{
	A3DVECTOR3 vDir;		// must be normalized
	float fLength;
};

public:
	CPathFollowing()
	{
		m_iNodeNum = 0;
	}

	~CPathFollowing()
	{
		Release();
	}

	void Release()
	{
		m_Path.clear();
		m_iNodeNum = 0;
	}
	
	void AddPathNode(const A3DVECTOR3& vPos)
	{
		if(m_iNodeNum == 0)
		{
			m_vStartPos = vPos;
		}
		else
		{
			LineSeg newLineSeg;
			newLineSeg.vDir = vPos - m_vEndPos;
			if(newLineSeg.vDir.IsZero())	// it seems we insert a repeated position.
				return;

			newLineSeg.fLength = newLineSeg.vDir.Normalize();
			m_Path.push_back(newLineSeg);
		}
		
		m_vEndPos = vPos;
		++m_iNodeNum;
	}

	bool Start()
	{
		m_iCurLineSeg = 0;
		m_vCurPos =m_vStartPos;
		m_fDist = 0;
		
		if(m_Path.empty())		// only one node in the path
			return false;	

		return true;
	}

	void MoveOneStep(float fStepDist);

	A3DVECTOR3 GetCurPos() { return m_vCurPos;}
	
	bool GetToEnd()
	{
		return m_iCurLineSeg == (int)m_Path.size();
	}

private:

	A3DVECTOR3 m_vStartPos;				// the start pos of the path
	A3DVECTOR3 m_vEndPos;				// the end pos of the path
	int m_iNodeNum;

	vector<LineSeg> m_Path;
	
	A3DVECTOR3 m_vCurPos;
	int m_iCurLineSeg;
	float m_fDist;			// distance from m_vCurPos to pre path node's pos
};

#endif
