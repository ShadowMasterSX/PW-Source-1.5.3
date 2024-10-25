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
��ǰ�㷨��
	������(m_vFleePos, m_fSafeDist)����һ��Բ��ֱ��(m_vFleePos, m_vCurPos)
���Բ�ཻ�ڵ�P����һ��ģ�PӦ������ѵ�Ŀ��㡣Ȼ�����뿼�ǿɴ�����
��Ѱ���Ŀ��������ԣ����ǿ�ʼ�ж�P�Ƿ�ֱ�߿ɴ����ǣ���P��Ϊ����
���� ������Բ����������P�����ҵ�P�Ļ���ÿ�ε���һ�������ĵ㣬����Щ��
��������ͬ���жϣ�������趨��Χ�����ҵ���õ㼴Ϊ����Ŀ��㡣����ѡȡ
��һ���ɴ��Ϊ����
	���������У����ǲ������ǹ�ʽ�еĺͽǡ���ǹ�ʽ�Ӷ�ʵ�ֿ��ٵļ��㣡
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
