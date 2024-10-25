 /*
 * FILE: NPCFleeOnGroundAgent.cpp
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

#include "NPCFleeOnGroundAgent.h"
#include "NPCChaseOnGroundAgent.h"

#ifndef PI
	#define PI 3.1416f
#endif

// #define STEP_RADIAN (PI/36)    // 5 degree for now

// Reduce the blocked case!
#define STEP_RADIAN (PI/18)    // 10 degree for now


CNPCFleeOnGroundAgent::CNPCFleeOnGroundAgent(CMap * pMap)
 :CNPCFleeAgent(pMap)
{

}

CNPCFleeOnGroundAgent::~CNPCFleeOnGroundAgent()
{

}


/************************************************************************
当前算法：
	假设以(m_vFleePos, m_fSafeDist)构成一个圆，直线(m_vFleePos, m_vCurPos)
与该圆相交于点P，则一般的，P应该是最佳的目标点。然而必须考虑可达因素
和寻径的开销。所以，我们开始判断P是否直线可达，如果是，则P即为所求。
否则， 我们在圆周上依次找P两侧且到P的弧度每次递增一个步长的点，对这些点
我们作相同的判断，如果在设定范围内能找到则该点即为所求目标点。否则，选取
第一个可达点为所求。
	递增过程中，我们采用三角公式中的和角、差角公式从而实现快速的计算！
************************************************************************/
void CNPCFleeOnGroundAgent::FindRightGoal()
{
	A3DVECTOR3 vDir = m_vCurPos - m_vFleePos;
	vDir.y = 0.0f;
	float fMag = vDir.Normalize();
	if(fMag < 1e-6)
	{
		vDir.x = RAND(1.0f);
		if(vDir.x >=1 ) vDir.x = 1;
		vDir.z = sqrt(1 - vDir.x * vDir.x);
	}

	float cosDir = vDir.x;
	float sinDir = vDir.z;

	POS2D p2Cur, p2Goal;
	//p2Cur = g_NPCMoveMap.GetMapPos(m_vCurPos);
	p2Cur = m_pMap->GetGroundMapPos(m_vCurPos);
	
	A3DVECTOR3 vPos,vFirstReachablePos(0.0f);
	
	// First, we test the best pos
	vPos.x = m_vFleePos.x + m_fSafeDist * cosDir;
	vPos.y = 0.0f;
	vPos.z = m_vFleePos.z + m_fSafeDist * sinDir;
	//p2Goal = g_NPCMoveMap.GetMapPos(vPos);
	p2Goal = m_pMap->GetGroundMapPos(vPos);
	
	//if(g_NPCMoveMap.IsPosReachable(p2Goal) && 
	if(m_pMap->IsGroundPosReachable(p2Goal) && 
		((CNPCChaseOnGroundAgent *)m_pNPCChaseAgent)->CanGoStraightForward(p2Cur, p2Goal) )
	{
		// Find it!
		m_vRightGoal = vPos;
		return;
	}
	//else if (g_NPCMoveMap.IsPosReachable(p2Goal))
	else if (m_pMap->IsGroundPosReachable(p2Goal))
		vFirstReachablePos = vPos;

	// Start the loop
	float cosStepRadian = cos(STEP_RADIAN);
	float sinStepRadian = sin(STEP_RADIAN);
	float cosClockwiseCur = cosDir;
	float sinClockwiseCur = sinDir;
	float cosAntiClockwiseCur = cosDir;
	float sinAntiClockwiseCur = sinDir;
	float fOffsetRadian = 0.0f;
	//***********************************************
	// Revised by wenfeng, 05-11-12
	// Widen the range of real goal from flee pos.
	while (fOffsetRadian < PI /* PI/2 is old value! */)
	{
		float tmpCos,tmpSin;
		fOffsetRadian += STEP_RADIAN;
		
		// Anticlockwise Pos: (A+B)
		
		// cos(A+B) = cosAcosB - sinAsinB
		// sin(A+B) = sinAcosB + cosAsinB
		tmpCos = cosAntiClockwiseCur*cosStepRadian-sinAntiClockwiseCur*sinStepRadian;
		tmpSin = sinAntiClockwiseCur * cosStepRadian + cosAntiClockwiseCur * sinStepRadian;
		cosAntiClockwiseCur = tmpCos;
		sinAntiClockwiseCur = tmpSin;
		
		vPos.x = m_vFleePos.x + m_fSafeDist * cosAntiClockwiseCur;
		vPos.y = 0.0f;
		vPos.z = m_vFleePos.z + m_fSafeDist * sinAntiClockwiseCur;
		//p2Goal = g_NPCMoveMap.GetMapPos(vPos);
		p2Goal = m_pMap->GetGroundMapPos(vPos);

		//if(g_NPCMoveMap.IsPosReachable(p2Goal) && 
		if(m_pMap->IsGroundPosReachable(p2Goal) && 
			((CNPCChaseOnGroundAgent *)m_pNPCChaseAgent)->CanGoStraightForward(p2Cur, p2Goal) )
		{
			// Find it!
			m_vRightGoal = vPos;
			return;
		}
		//else if (g_NPCMoveMap.IsPosReachable(p2Goal) && vFirstReachablePos.IsZero())
		else if (m_pMap->IsGroundPosReachable(p2Goal) && vFirstReachablePos.IsZero())
			vFirstReachablePos = vPos;

		// Clockwise Pos: (A-B)
		// cos(A-B) = cosAcosB + sinAsinB
		// sin(A-B) = sinAcosB - cosAsinB
		tmpCos = cosClockwiseCur*cosStepRadian + sinClockwiseCur*sinStepRadian;
		tmpSin = sinClockwiseCur * cosStepRadian - cosClockwiseCur * sinStepRadian;
		cosClockwiseCur = tmpCos;
		sinClockwiseCur = tmpSin;
		
		vPos.x = m_vFleePos.x + m_fSafeDist * cosClockwiseCur;
		vPos.y = 0.0f;
		vPos.z = m_vFleePos.z + m_fSafeDist * sinClockwiseCur;
		//p2Goal = g_NPCMoveMap.GetMapPos(vPos);
		p2Goal = m_pMap->GetGroundMapPos(vPos);

		//if(g_NPCMoveMap.IsPosReachable(p2Goal) && 
		if(m_pMap->IsGroundPosReachable(p2Goal) && 
			((CNPCChaseOnGroundAgent *)m_pNPCChaseAgent)->CanGoStraightForward(p2Cur, p2Goal) )
		{
			// Find it!
			m_vRightGoal = vPos;
			return;
		}
		//else if (g_NPCMoveMap.IsPosReachable(p2Goal) && vFirstReachablePos.IsZero())
		else if (m_pMap->IsGroundPosReachable(p2Goal) && vFirstReachablePos.IsZero())
			vFirstReachablePos = vPos;
	}

	if(vFirstReachablePos.IsZero())
	{
		// if the program goes here, it always means that we even find no reachable pos meeting the flee condition!
		// so we return m_vCurPos as the m_vRightGoal!
		m_vRightGoal = m_vCurPos;
	}
	else
		m_vRightGoal = vFirstReachablePos;
}
