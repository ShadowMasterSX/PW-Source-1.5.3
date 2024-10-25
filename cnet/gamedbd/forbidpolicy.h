#ifndef _FORBID_POLICY_H_
#define _FORBID_POLICY_H_

#include <vector>
#include <utility>
#include <map>
#include <string>

using std::map;
using std::vector;
using std::pair;
using std::string;

namespace GNET
{
	class ForbidPolicy
	{
	private:
		typedef pair<int, int> Region;

		struct Policy
		{
			Region charge_region;	
			map<Region, int> time_map;		
		};
		vector<Policy> policy_set;
		Policy cur_policy;
		static ForbidPolicy instance;

		bool ParseChargeRegion(const string &line);
		bool ParseTimeMap(const string &line);

	public:
		static ForbidPolicy * GetInstance() { return &instance;}
		bool LoadPolicy(const string &file);
		int GetForbidTime(int cash, int origin_forbidtime);

#ifdef _DEBUGFORBIDPOLICY
		void DumpPolicy();
#endif
	};
};

#endif
