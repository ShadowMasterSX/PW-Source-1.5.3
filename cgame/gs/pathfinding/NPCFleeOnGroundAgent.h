 /*
 * FILE: NPCFleeOnGroundAgent.h
 *
 * DESCRIPTION:   A  class derived from the class CNPCFleeAgent 
 *							which realizes On-Ground NPCs' flee movement.
 *							
 *
 * CREATED BY: He wenfeng, 2005/5/17
 *
 * HISTORY: 
 *
 * Copyright (c) 2004 Archosaur Studio, All Rights Reserved.
 */
#ifndef _NPCFLEEONGROUNDAGENT_H_
#define _NPCFLEEONGROUNDAGENT_H_

#include "NPCMove.h"
#include "NPCMoveAgent.h"

class CNPCFleeOnGroundAgent: public CNPCFleeAgent
{
public:
	CNPCFleeOnGroundAgent(CMap * pMap);
	virtual ~CNPCFleeOnGroundAgent();

protected:
	
	virtual CNPCChaseAgent* CreateChaseAgent()
	{
		//return CreateNPCChaseAgent(NPC_MOVE_ENV_ON_GROUND,NPC_MOVE_BEHAVIOR_CHASE_NORMAL);
		return CreateNPCChaseAgent(m_pMap, NPC_MOVE_ENV_ON_GROUND,NPC_MOVE_BEHAVIOR_CHASE_NORMAL);
	}
	
	// To be overriden!
	// Find the right goal according the m_vFleePos and m_fSafeDist
	virtual void FindRightGoal();

	// Override, only consider the distance on the XOZ plane
	// Test if we need update the right goal?
	virtual bool NeedUpdateGoal()
	{
		if(!m_bHaveGoal) return true;
		
		A3DVECTOR3 vDelta =	m_vFleePos-m_vRightGoal;
		vDelta.y = 0.0f;
		return (vDelta.SquaredMagnitude() < m_fSqrSafeDist );
	}

	virtual bool CurPosFled()
	{
		A3DVECTOR3 vDelta =	m_vFleePos-m_vCurPos;
		vDelta.y = 0.0f;

		return ( vDelta.SquaredMagnitude() >= m_fSqrSafeDist );
	}
};

#endif

