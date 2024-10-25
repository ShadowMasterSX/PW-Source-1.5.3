 /*
 * FILE: NPCMove.h
 *
 * DESCRIPTION:   A set of classes to realize several types of NPCs' movement, 
 *							Which have a abstract base interface: CNPCMoveAgent.
 *
 * CREATED BY: He wenfeng, 2005/4/18
 *
 * HISTORY: 
 *
 * Copyright (c) 2004 Archosaur Studio, All Rights Reserved.
 */

#ifndef _NPCMOVE_H_
#define _NPCMOVE_H_

#include "a3dvector.h"
#include "chaseinfo.h"
#include "HalfSpace.h"
#include "GlobalWaterAreaMap.h"

#include "NPCMoveMap.h"
#include "CMap.h"
using namespace NPCMoveMap;

#define RELAX_ERROR 0.1f
#define SQR_RELAX_ERROR 0.01f


//******************************************************************************************
// Revise these two macro parameters again, by wenfeng, 05-12-31

// These two parameters are only for AdjustCurPos() function to 
// make sure to confine the position to the region it belongs to more strictly!
// take care of these two parameters
#define ABOVE_DIST  0.2f  /* old value: 0.2f */
#define BELOW_DIST  0.2f  /* old value: 0.8f */

// while, we add new two similar macro parameters for the function IsPosBeyondEnv(),
// so that we can avoid the statical standing of some NPCs whose birthplace is 
// not so appropriate.
#define RELAX_ABOVE_DIST -0.001f
#define RELAX_BELOW_DIST -0.001f
//******************************************************************************************

#ifndef PI
	#define PI 3.1416f
#endif

#define RAND(x) ( (float)(x) * rand() / RAND_MAX )

#define MAX_GENERATE_GOAL_TIMES 11		// max try times of generating a temporary goal

#define ZERO_DIST_ERROR 0.01f			// dist we regard as Zero!

#define MAX_BLOCK_TIMES 3				// max block times we can tolerate

/************************************************************************
 * the Base Class: general interfaces describe the movement
 * It is an abstract class
 ************************************************************************/
class CNPCMoveAgent
{
public:
	CNPCMoveAgent(CMap * pMap): m_pMap(pMap){}
	virtual ~CNPCMoveAgent(){}

	// The first function should be called after the construction
	// Do some initialization, so can be overridden
	// here, set the initial pos and length of one step
	virtual void Init(const A3DVECTOR3& vCurPos, float fStep)
	{
		m_vCurPos = vCurPos;
		SetMoveStep(fStep);
	}
	
	const A3DVECTOR3& GetCurPos() const
	{
		return m_vCurPos;
	}

	virtual void SetMoveStep(float fStep) { m_fMoveStep = fStep; }
	virtual void MoveOneStep() = 0;
	virtual void Release(){}

	static A3DVECTOR3 GeneratePosOnSphere(const A3DVECTOR3& vCenter, float fRadius)
	{
		A3DVECTOR3 vOffset;
		float latitude = RAND(PI) - PI /2;			// (-PI/2, PI/2)
		float longitude = RAND( 2*PI );				// (0, 2PI)
		vOffset.y = fRadius * sin(latitude);
		float tmp = fRadius * cos(latitude);
		vOffset.x = tmp * cos(longitude);
		vOffset.z = tmp * sin(longitude);
		return ( vOffset + vCenter );			
	}

	static A3DVECTOR3 GeneratePosInsideSphere(const A3DVECTOR3& vCenter, float fRadius)
	{
		float r = RAND(fRadius);
		return GeneratePosOnSphere(vCenter,r);
	}

	//////////////////////////////////////////////////////////////////////////
	// some extra passable test function type declaration
	
	// only test whether the pos is above terrain
	//inline static bool IsPosAboveTerrain(const A3DVECTOR3& vPos)
	inline  bool IsPosAboveTerrain(const A3DVECTOR3& vPos)
	{
		//float fTerrainHeight = NPCMove::GetTerrainHeight(vPos.x, vPos.z);
		float fTerrainHeight = m_pMap->GetTerrainHeight(vPos.x, vPos.z);
		return vPos.y > fTerrainHeight + RELAX_ABOVE_DIST;		
	}
	
	// test whether the pos is above terrain and water
	inline static bool IsPosOnAir(const A3DVECTOR3& vPos, CMap * pMap)
	{
		// Verify the pos must be above the terrain and water
		//float fTerrainHeight = NPCMove::GetTerrainHeight(vPos.x, vPos.z);
		float fTerrainHeight = pMap->GetTerrainHeight(vPos.x, vPos.z);
		//float fWaterHeight = WaterAreaMap::GetWaterHeight(vPos.x,vPos.z);
		float fWaterHeight = pMap->GetWaterHeight(vPos.x,vPos.z);

		if(vPos.y > fTerrainHeight + RELAX_ABOVE_DIST && vPos.y > fWaterHeight + RELAX_ABOVE_DIST)
			return true;
		else
			return false;
	}
	
	// test whether the pos is above terrain and below water
	inline static bool IsPosInWater(const A3DVECTOR3& vPos, CMap * pMap)
	{
		// Verify the pos must be above the terrain while under the water
		//float fTerrainHeight = NPCMove::GetTerrainHeight(vPos.x, vPos.z);
		float fTerrainHeight = pMap->GetTerrainHeight(vPos.x, vPos.z);
		//float fWaterHeight = WaterAreaMap::GetWaterHeight(vPos.x,vPos.z);
		float fWaterHeight = pMap->GetWaterHeight(vPos.x,vPos.z);

		if(vPos.y > fTerrainHeight + RELAX_ABOVE_DIST && vPos.y < fWaterHeight - RELAX_BELOW_DIST)
			return true;
		else
			return false;
	}
	//////////////////////////////////////////////////////////////////////////

protected:

	// On moving a step, maybe the m_vCurPos will turn to be an invalid pos
	// Then, call this func to adjust m_vCurPos
	// To be overridden
	virtual void AdjustCurPos()
	{
		// Here, only test if m_vCurPos are below the terrain?
		// Avoid moving under the terrain surface
		//ClampUpTerrain(m_vCurPos, ABOVE_DIST);
		m_pMap->ClampUpTerrain(m_vCurPos, ABOVE_DIST);
	}

protected:

	A3DVECTOR3 m_vCurPos;		// current pos
	float m_fMoveStep;					// the length of one step

	CMap   * m_pMap;
};


//***********************************************************

/************************************************************************
 * the Class derived from CNPCMoveAgent which describes 
 * a chase-goal movement. 
 ************************************************************************/
class CNPCChaseAgent: public CNPCMoveAgent
{
public:
	CNPCChaseAgent(CMap * pMap)
		:CNPCMoveAgent(pMap)
	{
	}
	virtual ~CNPCChaseAgent() {}

	//**************************************************
	// Revised by wenfeng ,05-11-7
	// Change the interface!
	// Add the last para CChaseInfo* pChaseInfo, which is for recording the dispersed infomation
	// when chasing a goal continually.
	//**************************************************
	virtual void SetGoal(const A3DVECTOR3& vGoal, float fMinDist, CChaseInfo* pChaseInfo = NULL )
	{
		m_vGoal = vGoal;
		m_fMinDist2Goal = fMinDist;
		m_fSqrMinDist2Goal = fMinDist * fMinDist;
		SetMoveDir(vGoal);
		m_bGetToGoal = CurPosGetToGoal();
		m_bBlockedBeyondEnv = false;
	}
	
	// Default implement: Go in straight line!
	virtual void MoveOneStep()	
	{
		if(!GetToGoal() && ! BlockedBeyondEnv())
		{
			A3DVECTOR3 vNextPos = m_vCurPos + m_fMoveStep * m_vMoveDir;
			if(!IsPosBeyondEnv(vNextPos))
			{
				m_vCurPos = vNextPos;
				AdjustCurPos();
				
				// Get to goal?
				m_bGetToGoal = CurPosGetToGoal();
				if(m_bGetToGoal) 
					AdjustGetToGoalPos();
				else
				{
					// update the m_vMoveDir
					SetMoveDir(m_vGoal);
				}
			}
			else
			{
				m_bBlockedBeyondEnv = true;
			}
		}
	}

	bool GetToGoal()
	{
		return m_bGetToGoal;
	}
	
	bool BlockedBeyondEnv()
	{
		return m_bBlockedBeyondEnv;
	}
	
	/************************************************************************
	 The following are some Interfaces used only by CNPCChaseOnGroundAgent 
	 but placed here for  general interfaces!

	 Now, the following interfaces are useful for all chase agent! by wenfeng, 05-11-5.
	************************************************************************/
	virtual bool StartPathFinding(int SearchPixels=10) 
	{ 
		// following code is for non-PF ChaseOnAirAgent and ChaseInWaterAgent
		m_bFoundPath = true;
		m_PFPixels = SearchPixels;
		m_bBlocked = false;
		m_bIsGoalReachable = true;
		SetMaxBlockTimes();
		
		return true; 
	}

	virtual bool FoundPath() { return m_bFoundPath; }
	virtual void SetPFPixels(int SearchPixels)  { m_PFPixels = SearchPixels; }
	virtual int  GetPFPixels() { return m_PFPixels;}

	virtual bool IsBlocked() { return m_bBlocked; }

	virtual bool IsGoalReachable() { return m_bIsGoalReachable; }

	virtual void SetMaxBlockTimes(int iMaxBlockTimes = MAX_BLOCK_TIMES){ m_iMaxBlockTimes = iMaxBlockTimes; }

protected:

	// To be overridden
	virtual void SetMoveDir(const A3DVECTOR3& vGoal)
	{
		m_vMoveDir = vGoal - m_vCurPos;
		m_vMoveDir.Normalize();
	}

	// Can be overridden!	
	virtual bool CurPosGetToGoal()
	{
		A3DVECTOR3 vDelta = m_vGoal-m_vCurPos;
		// here, we increase the expected distance to goal by RELAX_ERROR
		return ( vDelta.SquaredMagnitude() <= (m_fSqrMinDist2Goal + 2*RELAX_ERROR*m_fMinDist2Goal + SQR_RELAX_ERROR)
		//*********************************
		// Note, by wenfeng, 05-12-26
		// the following test condition is for move in straight line
			||	DotProduct(vDelta, m_vMoveDir) < 0 );
	}

	virtual bool PosGetToGoal(const A3DVECTOR3& vPos)
	{
		return (m_vGoal - vPos).SquaredMagnitude() <= (m_fSqrMinDist2Goal + 2*RELAX_ERROR*m_fMinDist2Goal + SQR_RELAX_ERROR);
	}

	// To be overridden
	virtual bool IsPosBeyondEnv(const A3DVECTOR3& vPos)
	{
		return false;
	}

	// when current position gets to the goal, call this function to adjust the position!
	// Can be overridden
	virtual void AdjustGetToGoalPos()
	{
		// Adjust the final pos ( which we regard as the pos get to goal)
		A3DVECTOR3 vCurPosBak = m_vCurPos;			// back up the old current pos
		
		// Adjust the m_vCurPos to be the position which most satisfies the distance range to the goal
		m_vCurPos = m_vGoal - m_fMinDist2Goal * m_vMoveDir;
		AdjustCurPos();
		
		// if the position after adjustment doesn't meet the condition of CurPosGetToGoal condition,
		// we restore the old m_vCurPos
		if(!CurPosGetToGoal())
			m_vCurPos=vCurPosBak;
	}

protected:

	A3DVECTOR3 m_vGoal;
	float m_fMinDist2Goal;				  // holds a minimum distance to the goal (>=0.0f)
	float m_fSqrMinDist2Goal;			// holds a minimum distance to the goal (>=0.0f), here we keep the square of the min dist as well

	A3DVECTOR3 m_vMoveDir;		// Moving direction!
	
	bool m_bGetToGoal;
	
	// whether the agent is blocked due to next position's 
	// beyond the current environment!
	bool m_bBlockedBeyondEnv;	
	
	bool m_bFoundPath;			// has the path found?
	bool m_bBlocked;			// Is the NPC blocked?
	bool m_bIsGoalReachable;	// Is the goal reachable?
	int m_PFPixels;				// how many pixels will be searching each time call a path finding function

	int m_iMaxBlockTimes;		// the max block times we can tolerate
};

/************************************************************************
 * the Class derived from CNPCMoveAgent which describes 
 * a flee movement. 
 ************************************************************************/
class CNPCFleeAgent: public CNPCMoveAgent
{
public:
	CNPCFleeAgent(CMap * pMap)
		:CNPCMoveAgent(pMap), m_pNPCChaseAgent(NULL)
	{ 
		m_pNPCChaseAgent = NULL; 
	}
	virtual ~CNPCFleeAgent() {}

	void SetFleePos(const A3DVECTOR3& vFleePos, float fSafeDist)
	{
		m_vFleePos = vFleePos;
		m_fSafeDist = fSafeDist;
		m_fSqrSafeDist = m_fSafeDist * m_fSafeDist;
	}
	
	virtual void StartFlee()
	{
		m_bFleeSuccess = CurPosFled();
		if(m_bFleeSuccess) return;

		if(NeedUpdateGoal())
		{
			FindRightGoal();
			m_pNPCChaseAgent ->SetGoal(m_vRightGoal, 0.0f);
			m_pNPCChaseAgent -> StartPathFinding();
			m_bHaveGoal = true;
		}
	}

	bool FleeSuccess()
	{
		return m_bFleeSuccess;
	}

	virtual void Init(const A3DVECTOR3& vCurPos, float fStep)
	{
		CNPCMoveAgent::Init(vCurPos, fStep);
		
		if( ! m_pNPCChaseAgent )
			m_pNPCChaseAgent = CreateChaseAgent();
		
		m_pNPCChaseAgent ->Init(vCurPos, fStep);

		m_bHaveGoal = false;
	}

	virtual void Release()
	{
		if(m_pNPCChaseAgent)
		{
			m_pNPCChaseAgent->Release();
			delete m_pNPCChaseAgent;
			m_pNPCChaseAgent = NULL;
		}
	}

	virtual void MoveOneStep()
	{
		if(FleeSuccess()) return;

		m_pNPCChaseAgent -> MoveOneStep();
		m_vCurPos = m_pNPCChaseAgent ->GetCurPos();
		
		m_bFleeSuccess = CurPosFled() || m_pNPCChaseAgent->GetToGoal();
		if(m_bFleeSuccess) m_bHaveGoal = false;
	}

	virtual void SetMoveStep(float fStep) 
	{ 
		CNPCMoveAgent::SetMoveStep(fStep);
		if(m_pNPCChaseAgent)
			m_pNPCChaseAgent->SetMoveStep(fStep);
	}

protected:
	// This is a Factory-Pattern method!
	// It will delay creating the concrete instance in its' derived class. 
	virtual CNPCChaseAgent* CreateChaseAgent() = 0;

	// To be overriden!
	// Find the right goal according the m_vFleePos and m_fSafeDist
	virtual void FindRightGoal()
	{
		A3DVECTOR3 vDir = m_vCurPos - m_vFleePos;
		vDir.Normalize();
		if(vDir.Magnitude() < 1e-6) vDir.x = 1.f;
		m_vRightGoal = m_vFleePos + m_fSafeDist * vDir;

		if(IsPosBeyondEnv(m_vRightGoal))
		{
			// m_vRightGoal is not a valid pos! So we try to generate one!
			int iTryCounter=0;

			//***********************************************
			// Revised by wenfeng, 05-11-12
			// the previous half try times we only try the half circle region
			// on the same side of m_vCurPos! 
			
			const int iHalfMaxGenGoalTimes = MAX_GENERATE_GOAL_TIMES >> 1;
			do 
			{
				m_vRightGoal = CNPCMoveAgent::GeneratePosOnSphere(m_vFleePos, m_fSafeDist);
				if(iTryCounter < iHalfMaxGenGoalTimes && DotProduct(m_vRightGoal-m_vFleePos,vDir) < 0)
				{
					// the temporary goal m_vRightGoal lies on the other hemisphere
					// so, we flip it
					CHBasedCD::CHalfSpace hs;
					hs.SetNV(vDir,m_vFleePos);
					hs.Mirror(m_vRightGoal);
				}

				iTryCounter++;

			} while(IsPosBeyondEnv(m_vRightGoal) && iTryCounter < MAX_GENERATE_GOAL_TIMES);

		}
	}

	// Test if we need update the right goal?
	virtual bool NeedUpdateGoal()
	{
		if(!m_bHaveGoal) return true;
		
		return ((m_vFleePos-m_vRightGoal).SquaredMagnitude() < m_fSqrSafeDist );
	}

	virtual bool CurPosFled()
	{
		return ( (m_vFleePos-m_vCurPos).SquaredMagnitude() >= m_fSqrSafeDist );
	}
	
	// test whether a pos is beyond current environment, if so, 
	// the pos is an invalid pos.
	virtual bool IsPosBeyondEnv(const A3DVECTOR3& vPos)
	{
		return false;
	}

protected:
	
	bool m_bFleeSuccess;				// Finish the flee successfully?

	bool m_bHaveGoal;					// whether the agent has a goal
	A3DVECTOR3 m_vRightGoal;	// The right goal which meets the flee condition

	A3DVECTOR3 m_vFleePos;
	float m_fSafeDist;						// holds a minimum distance to the flee pos (>=0.0f)
	float m_fSqrSafeDist;				  // holds the square of the minimum distance to the goal (>=0.0f)
	CNPCChaseAgent* m_pNPCChaseAgent;		// A chase agent to handle the chase-goal process
};


/************************************************************************
 * the Class derived from CNPCMoveAgent which describes 
 * a Ramble movement. 
 ************************************************************************/
class CNPCRambleAgent: public CNPCMoveAgent
{
public:
	CNPCRambleAgent(CMap * pMap)
		:CNPCMoveAgent(pMap), m_pNPCChaseAgent(NULL)
	{
		
	}
	virtual ~CNPCRambleAgent() {}

	void SetRambleRange(const A3DVECTOR3& vCenter, float fRadius)
	{
		m_vMoveRangeCenter = vCenter;
		m_fMoveRangeRadius = fRadius;
	}

	virtual void Init(const A3DVECTOR3& vCurPos, float fStep )
	{
		CNPCMoveAgent::Init(vCurPos, fStep);
		
		if( ! m_pNPCChaseAgent )
			m_pNPCChaseAgent = CreateChaseAgent();
		
		m_pNPCChaseAgent ->Init(vCurPos, fStep);
	}

	virtual void Release()
	{
		if(m_pNPCChaseAgent)
		{
			m_pNPCChaseAgent->Release();
			delete m_pNPCChaseAgent;
			m_pNPCChaseAgent = NULL;
		}
	}

	virtual void MoveOneStep()
	{
		if(Stopped()) return;

		m_pNPCChaseAgent -> MoveOneStep();
		m_vCurPos = m_pNPCChaseAgent ->GetCurPos();
	}

	virtual void StartRamble()
	{
		GenerateTmpGoal();
		m_pNPCChaseAgent -> SetGoal(m_vTmpGoal, 0.0f);
		m_pNPCChaseAgent -> StartPathFinding();
	}

	// Whether it finishes a ramble and arrive at the tmp goal
	virtual bool Stopped()
	{
		return m_pNPCChaseAgent->GetToGoal();
	}

	A3DVECTOR3 GetTmpGoal()
	{
		return m_vTmpGoal;
	}

	virtual void SetMoveStep(float fStep) 
	{ 
		CNPCMoveAgent::SetMoveStep(fStep);
		if(m_pNPCChaseAgent)
			m_pNPCChaseAgent->SetMoveStep(fStep);
	}

protected:
	
	virtual void GenerateTmpGoal()
	{
		m_vTmpGoal = GeneratePosInMoveRange();
		//ClampUpTerrain(m_vTmpGoal, 2.0f);
		m_pMap->ClampUpTerrain(m_vTmpGoal, 2.0f);
	}
	
	// Generate a position in the range of ramble
	// By default, implement it for 3D case
	// Override this func to realize 2D case
	virtual A3DVECTOR3 GeneratePosInMoveRange()
	{
		/*
		// Sphere range: now it's discarded.
		
		A3DVECTOR3 vOffset;
		float r = RAND(m_fMoveRangeRadius);
		float latitude = RAND(PI) - PI /2;			// (-PI/2, PI/2)
		float longitude = RAND( 2*PI );				// (0, 2PI)
		vOffset.y = r * sin(latitude);
		float tmp = r * cos(latitude);
		vOffset.x = tmp * cos(longitude);
		vOffset.z = tmp * sin(longitude);
		return ( vOffset + m_vMoveRangeCenter );
		 */
		
		A3DVECTOR3 vOffset;
		float dx = RAND(2*m_fMoveRangeRadius);
		float dy = RAND(2*m_fMoveRangeRadius);
		float dz = RAND(2*m_fMoveRangeRadius);
		vOffset.x = - m_fMoveRangeRadius + dx;
		vOffset.y = - m_fMoveRangeRadius + dy;
		vOffset.z = - m_fMoveRangeRadius + dz;
		return ( vOffset + m_vMoveRangeCenter );
	}

	// This is a Factory-Pattern method!
	// It will delay creating the concrete instance in its' derived class. 
	virtual CNPCChaseAgent* CreateChaseAgent() = 0;

protected:
	
	// The range of ramble is defined as a sphere space
	A3DVECTOR3 m_vMoveRangeCenter;
	float m_fMoveRangeRadius;

	// A temp goal for ramble
	A3DVECTOR3 m_vTmpGoal;

	CNPCChaseAgent* m_pNPCChaseAgent;		// A chase agent to handle the chase-goal process

};

#endif

