 /*
 * FILE: NPCRambleInWaterAgent.h
 *
 * DESCRIPTION:   A  class derived from the class CNPCChaseAgent 
 *							which realizes In-Water NPCs' ramble movement.
 *
 * CREATED BY: He wenfeng, 2005/5/13
 *
 * HISTORY: 
 *
 * Copyright (c) 2004 Archosaur Studio, All Rights Reserved.
 */

#ifndef _NPCRAMBLEINWATERAGENT_H_
#define _NPCRAMBLEINWATERAGENT_H_

#include "NPCMove.h"
#include "NPCMoveAgent.h"

class CNPCRambleInWaterAgent: public CNPCRambleAgent
{

public:
	CNPCRambleInWaterAgent(CMap * pMap):CNPCRambleAgent(pMap) { }
	virtual ~CNPCRambleInWaterAgent() { }

protected:
	
	virtual CNPCChaseAgent* CreateChaseAgent()
	{
		return CreateNPCChaseAgent(m_pMap,NPC_MOVE_ENV_IN_WATER);
	}
};


#endif
