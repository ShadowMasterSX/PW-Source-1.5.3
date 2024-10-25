 /*
 * FILE: NPCRambleOnGroundAgent.h
 *
 * DESCRIPTION:   A  class derived from the class CNPCChaseAgent 
 *							which realizes on-ground NPCs' ramble movement.
 *
 * CREATED BY: He wenfeng, 2005/5/13
 *
 * HISTORY: 
 *
 * Copyright (c) 2004 Archosaur Studio, All Rights Reserved.
 */

#ifndef _NPCRAMBLEONGROUNDAGENT_H_
#define _NPCRAMBLEONGROUNDAGENT_H_

#include "NPCMove.h"
#include "NPCMoveAgent.h"

class CNPCRambleOnGroundAgent: public CNPCRambleAgent
{

public:
	CNPCRambleOnGroundAgent(CMap * pMap)
		:CNPCRambleAgent(pMap)
	{ 
	}
	virtual ~CNPCRambleOnGroundAgent() { }

	// Override to set a large search pixels!
	virtual void StartRamble();

protected:
	
	virtual CNPCChaseAgent* CreateChaseAgent()
	{
		return CreateNPCChaseAgent(m_pMap,NPC_MOVE_ENV_ON_GROUND,NPC_MOVE_BEHAVIOR_CHASE_NORMAL);
	}
	
	// Override to implement a easily-approaching goal
	// So we prefer approaching the goal in straight line!
	virtual void GenerateTmpGoal();

	// Override to generate a 2D pos
	virtual A3DVECTOR3 GeneratePosInMoveRange();

};


#endif
