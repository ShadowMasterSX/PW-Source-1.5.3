 /*
 * FILE: NPCDisperseChaseOnGroundAgent.h
 *
 * DESCRIPTION:   A  class derived from the class CNPCChaseOnGroundAgent 
 *							which adds some random factors to on-ground NPCs' chasing movement.
 *							For now, we implement it by disturbing the goal position.
 *
 * CREATED BY: He wenfeng, 2005/5/19
 *
 * HISTORY: 
 *
 * Copyright (c) 2004 Archosaur Studio, All Rights Reserved.
 */

#ifndef _NPCDISPERSECHASEONGROUNDAGENT_H_
#define _NPCDISPERSECHASEONGROUNDAGENT_H_

//************* Revised by wenfeng, 05-12-22********************
// directly change the base class to test no block chase agent

#define CHASE_WITHOUT_BLOCK 
#ifdef CHASE_WITHOUT_BLOCK
	#include "NPCChaseOnGroundNoBlockAgent.h"
	typedef CNPCChaseOnGroundNoBlockAgent CBaseChaseAgent;
#else
	#include "NPCChaseOnGroundAgent.h"
	typedef CNPCChaseOnGroundAgent CBaseChaseAgent;
#endif


class CNPCDisperseChaseOnGroundAgent: public CBaseChaseAgent
{
public:
	
	CNPCDisperseChaseOnGroundAgent(CMap * pMap)
		:CBaseChaseAgent(pMap)
	{
		// Set an initial disperse radian range.
		SetDisperseRadianRange( 2*PI / 3 );
	}

	virtual ~CNPCDisperseChaseOnGroundAgent() {}

	virtual void SetGoal(const A3DVECTOR3& vGoal, float fMinDist, CChaseInfo* pChaseInfo = NULL);

	void SetDisperseRadianRange(float fRadianRange)
	{
		m_fDisperseRadianRange = fRadianRange;
	}

protected:

	// Compute a goal dispersed by a random radian
	void GetDispersedGoal(const A3DVECTOR3& vGoal, float fMinDist, A3DVECTOR3& vDispersedGoal);

protected:
	
	float m_fDisperseRadianRange;

};

#endif

