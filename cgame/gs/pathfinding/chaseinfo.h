#ifndef __ONLINEGAME_GS_NPCMOVE_CHASE_INFO_H__
#define __ONLINEGAME_GS_NPCMOVE_CHASE_INFO_H__

#include <a3dvector.h>
//***********************************************************
// Added by wenfeng, 05-11-5
// Define a new class to record some extra information for chase.
// For scalability consideration, it may be derived...

// Revised by wenfeng, 05-11-8
// For object persistence, CChaseInfo can't be derived.

class CChaseInfo
{
public:
	CChaseInfo()
	{
		m_bHaveDispersed = false;
	}

	// can't be derived!
	//virtual ~CChaseInfo();		// can be derived!

	bool& HaveDispersed()
	{
		return m_bHaveDispersed;
	}
	
	A3DVECTOR3& DispersedDir()
	{
		return m_vDispersedDir;
	}

private:
	A3DVECTOR3 m_vDispersedDir;		// the direction vector from the dispersed spot to the goal
	bool m_bHaveDispersed;			// whether we have dispersed the real reached spot?
};
#endif

