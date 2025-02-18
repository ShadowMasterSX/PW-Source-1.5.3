#include "settings.h"

namespace GNET
{
	int FactionConfig::UpgradeCost(int level)
	{
		static int costs[MAX_LEVEL+1] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
		if(level>MAX_LEVEL || level<0)
			return -1;
		return costs[level];
	}

	int FactionConfig::MaxRole(int role)
	{
		static int max[7] = {0, 0, 1, 1, 4, 12, 200};
		if(role>6 || role<0)
			return -1;
		return max[role];
	}

	int FactionConfig::MaxMember(int level)
	{
		static int members[MAX_LEVEL+1] = {30, 35, 40, 45, 50, 55, 60, 65, 70, 75, 80};
		if(level>MAX_LEVEL || level<0)
			return -1;
		return members[level];
	}
}
