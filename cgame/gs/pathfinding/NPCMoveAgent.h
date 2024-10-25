 /*
 * FILE: NPCMoveAgent.h
 *
 * DESCRIPTION:   A general interface to create different movement strategy
 *							of NPCs
 *
 * CREATED BY: He wenfeng, 2005/4/21
 *
 * HISTORY: 
 *
 * Copyright (c) 2004 Archosaur Studio, All Rights Reserved.
 */
#ifndef _NPCMOVEAGENT_H
#define _NPCMOVEAGENT_H

#include "NPCMove.h"

#define NPC_MOVE_ENV_ON_GROUND 0
#define NPC_MOVE_ENV_ON_AIR 1
#define NPC_MOVE_ENV_IN_WATER 2

#define NPC_MOVE_BEHAVIOR_CHASE 0
#define NPC_MOVE_BEHAVIOR_FLEE 1
#define NPC_MOVE_BEHAVIOR_RAMBLE 2

//***********************************************
// Added by wenfeng, 05-11-5
// Provide more chasing behaviors for need.

// general chasing strategy without any dispersing. This 
// maybe cause that the agents chasing a common goal continually
// overlap each other.
#define NPC_MOVE_BEHAVIOR_CHASE_NORMAL 0			

// to avoid the above case, we disperse the stop position of each agent
// around the goal(each stop position lies in the circle around goal with radius
// is m_fMinDist2Goal )
#define NPC_MOVE_BEHAVIOR_CHASE_DISPERSE 1

// while, if we disperse the spot each time when chasing, the agent can
// generate a path of Z-form, it's not good in vision. So we provide
// this agent to dispers the stop position only once when the agent first
// chase the goal. After that, it will remember this dispersed information
// and follow according it.
#define NPC_MOVE_BEHAVIOR_CHASE_DISPERSE_ONCE 2

// new chase strategy, chase without block
#define NPC_MOVE_BEHAVIOR_CHASE_NO_BLOCK 3
//***********************************************

//***********************************************
// for future scalability
#define NPC_MOVE_BEHAVIOR_RAMBLE_NORMAL 0

#define NPC_MOVE_BEHAVIOR_FLEE_NORMAL 0
//***********************************************

CNPCChaseAgent* CreateNPCChaseAgent(CMap * pMap, int iMoveEnv,int iBehaviorMode = NPC_MOVE_BEHAVIOR_CHASE_DISPERSE_ONCE);
CNPCRambleAgent* CreateNPCRambleAgent(CMap * pMap, int iMoveEnv, int iBehaviorMode = NPC_MOVE_BEHAVIOR_RAMBLE_NORMAL);
CNPCFleeAgent* CreateNPCFleeAgent(CMap * pMap, int iMoveEnv, int iBehaviorMode =NPC_MOVE_BEHAVIOR_FLEE_NORMAL);

#endif

