 /*
 * FILE: NPCRambleOnAirAgent.h
 *
 * DESCRIPTION:   A  class derived from the class CNPCChaseAgent 
 *							which realizes On-Air NPCs' ramble movement.
 *
 * CREATED BY: He wenfeng, 2005/5/13
 *
 * HISTORY: 
 *
 * Copyright (c) 2004 Archosaur Studio, All Rights Reserved.
 */

#ifndef _NPCRAMBLEONAIRAGENT_H_
#define _NPCRAMBLEONAIRAGENT_H_

#include "NPCMove.h"
#include "NPCMoveAgent.h"

class CNPCRambleOnAirAgent: public CNPCRambleAgent
{

public:
	CNPCRambleOnAirAgent(CMap * pMap):CNPCRambleAgent(pMap) { }
	virtual ~CNPCRambleOnAirAgent() { }

protected:
	virtual CNPCChaseAgent* CreateChaseAgent()
	{
		return CreateNPCChaseAgent(m_pMap, NPC_MOVE_ENV_ON_AIR);
	}
};


#endif
