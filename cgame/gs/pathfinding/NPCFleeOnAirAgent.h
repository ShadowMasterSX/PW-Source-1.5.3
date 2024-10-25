 /*
 * FILE: NPCFleeOnAirAgent.h
 *
 * DESCRIPTION:   A  class derived from the class CNPCFleeAgent 
 *							which realizes On-Air NPCs' flee movement.
 *							For now, we don't realize the avoidance of any obstructs!
 *
 * CREATED BY: He wenfeng, 2005/5/16
 *
 * HISTORY: 
 *
 * Copyright (c) 2004 Archosaur Studio, All Rights Reserved.
 */
#ifndef _NPCFLEEONAIRAGENT_H_
#define _NPCFLEEONAIRAGENT_H_

#include "NPCMove.h"
#include "NPCMoveAgent.h"

class CNPCFleeOnAirAgent: public CNPCFleeAgent
{
public:
	CNPCFleeOnAirAgent(CMap * pMap);
	virtual ~CNPCFleeOnAirAgent();

protected:
	virtual CNPCChaseAgent* CreateChaseAgent()
	{
		return CreateNPCChaseAgent(m_pMap, NPC_MOVE_ENV_ON_AIR);
	}
	
	virtual bool IsPosBeyondEnv(const A3DVECTOR3& vPos);
};

#endif

