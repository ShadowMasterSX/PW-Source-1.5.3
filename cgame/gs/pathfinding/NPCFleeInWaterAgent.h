 /*
 * FILE: NPCFleeInWaterAgent.h
 *
 * DESCRIPTION:   A  class derived from the class CNPCFleeAgent 
 *							which realizes In-Water NPCs' flee movement.
 *							For now, we don't realize the avoidance of any obstructs!
 *
 * CREATED BY: He wenfeng, 2005/5/16
 *
 * HISTORY: 
 *
 * Copyright (c) 2004 Archosaur Studio, All Rights Reserved.
 */
#ifndef _NPCFLEEINWATERAGENT_H_
#define _NPCFLEEINWATERAGENT_H_

#include "NPCMove.h"
#include "NPCMoveAgent.h"

class CNPCFleeInWaterAgent: public CNPCFleeAgent
{
public:
	CNPCFleeInWaterAgent(CMap * pMap);
	virtual ~CNPCFleeInWaterAgent();

protected:
	
	virtual CNPCChaseAgent* CreateChaseAgent()
	{
		return CreateNPCChaseAgent(m_pMap, NPC_MOVE_ENV_IN_WATER);
	}
	
	virtual bool IsPosBeyondEnv(const A3DVECTOR3& vPos);
};

#endif

