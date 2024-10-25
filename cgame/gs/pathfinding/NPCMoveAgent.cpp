 /*
 * FILE: NPCMoveAgent.cpp
 *
 * DESCRIPTION:   Implementation of general interface to create different movement strategy
 *							of NPCs
 *
 * CREATED BY: He wenfeng, 2005/4/29
 *
 * HISTORY: 
 *
 * Copyright (c) 2004 Archosaur Studio, All Rights Reserved.
 */

#include "NPCMoveAgent.h"

// Chase Agent header
#include "NPCChaseOnGroundAgent.h"
#include "NPCDisperseChaseOnGroundAgent.h"
#include "NPCChaseOnGroundNoBlockAgent.h"

#include "NPCChaseInWaterAgent.h"
#include "NPCChaseOnAirAgent.h"

#include "NPCChaseOnAirPFAgent.h"
#include "NPCChaseInWaterPFAgent.h"

// Ramble Agent header
#include "NPCRambleOnGroundAgent.h"
#include "NPCRambleInWaterAgent.h"
#include "NPCRambleOnAirAgent.h"

// Flee Agent header
#include "NPCFleeOnGroundAgent.h"
#include "NPCFleeInWaterAgent.h"
#include "NPCFleeOnAirAgent.h"


static CNPCMoveAgent* CreateNPCMoveAgent(CMap* pMap, int iMoveEnv, int iBehavior, int iBehaviorMode)
{
	CNPCMoveAgent* pNPCMoveAgent = NULL;

	switch( iMoveEnv) 
	{
	case NPC_MOVE_ENV_ON_GROUND:
		
		if(NPC_MOVE_BEHAVIOR_CHASE==iBehavior)
		{
			if(NPC_MOVE_BEHAVIOR_CHASE_NORMAL == iBehaviorMode)
				pNPCMoveAgent = new CNPCChaseOnGroundAgent(pMap);
			else if(NPC_MOVE_BEHAVIOR_CHASE_DISPERSE == iBehaviorMode)
				//**************************************
				// hoho, tmp code!!! remember to fix!!!
				// now we use the same agent as  NPC_MOVE_BEHAVIOR_CHASE_DISPERSE_ONCE
				pNPCMoveAgent = new CNPCDisperseChaseOnGroundAgent(pMap);
			else if(NPC_MOVE_BEHAVIOR_CHASE_DISPERSE_ONCE == iBehaviorMode)
				pNPCMoveAgent = new CNPCDisperseChaseOnGroundAgent(pMap);
			else if(NPC_MOVE_BEHAVIOR_CHASE_NO_BLOCK == iBehaviorMode)
				pNPCMoveAgent = new CNPCChaseOnGroundNoBlockAgent(pMap);

		}
		else if(NPC_MOVE_BEHAVIOR_RAMBLE==iBehavior)
			pNPCMoveAgent = new CNPCRambleOnGroundAgent(pMap);
		else if (NPC_MOVE_BEHAVIOR_FLEE==iBehavior)
			pNPCMoveAgent = new CNPCFleeOnGroundAgent(pMap);

		break;

	case NPC_MOVE_ENV_ON_AIR:
		if(NPC_MOVE_BEHAVIOR_CHASE==iBehavior)
			// pNPCMoveAgent = new CNPCChaseOnAirStraightAgent;
			pNPCMoveAgent = new CNPCChaseOnAirPFAgent(pMap);

		else if(NPC_MOVE_BEHAVIOR_RAMBLE==iBehavior)
			pNPCMoveAgent = new CNPCRambleOnAirAgent(pMap);
		else if(NPC_MOVE_BEHAVIOR_FLEE==iBehavior)
			pNPCMoveAgent = new CNPCFleeOnAirAgent(pMap);
		
		break;

	case NPC_MOVE_ENV_IN_WATER:

		if(NPC_MOVE_BEHAVIOR_CHASE==iBehavior)
			// pNPCMoveAgent = new CNPCChaseInWaterStraightAgent;
			pNPCMoveAgent = new CNPCChaseInWaterPFAgent(pMap);
		else if(NPC_MOVE_BEHAVIOR_RAMBLE==iBehavior)
			pNPCMoveAgent = new CNPCRambleInWaterAgent(pMap);
		else if(NPC_MOVE_BEHAVIOR_FLEE==iBehavior)
			pNPCMoveAgent = new CNPCFleeInWaterAgent(pMap);
		
		break;

	default:
		break;
	}
	return pNPCMoveAgent;
}

CNPCChaseAgent* CreateNPCChaseAgent(CMap * pMap, int iMoveEnv, int iBehaviorMode)
{
	return (CNPCChaseAgent*)CreateNPCMoveAgent(pMap, iMoveEnv,NPC_MOVE_BEHAVIOR_CHASE, iBehaviorMode);
}

CNPCRambleAgent* CreateNPCRambleAgent(CMap * pMap, int iMoveEnv,int iBehaviorMode)
{
	return (CNPCRambleAgent*)CreateNPCMoveAgent(pMap, iMoveEnv,NPC_MOVE_BEHAVIOR_RAMBLE, iBehaviorMode);
}

CNPCFleeAgent* CreateNPCFleeAgent(CMap * pMap, int iMoveEnv,int iBehaviorMode)
{
	return (CNPCFleeAgent*)CreateNPCMoveAgent(pMap, iMoveEnv,NPC_MOVE_BEHAVIOR_FLEE, iBehaviorMode);
}
