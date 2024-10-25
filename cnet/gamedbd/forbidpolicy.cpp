#include "forbidpolicy.h"
#include <fstream>
using std::ifstream;

namespace GNET
{
	ForbidPolicy ForbidPolicy::instance;

	bool ForbidPolicy::ParseChargeRegion(const string &line)
	{
		int region_start, region_end;

		size_t startpos = line.find_first_not_of(" \t-]\r\n", 1), endpos;
		if (startpos==string::npos) return false;
		if (line[startpos] == '*')
		{
			region_start = -1;
			endpos = startpos+1;
		}
		else if (isdigit(line[startpos]))
		{
			endpos = line.find_first_of(" \t-]", startpos);
			if (endpos == string::npos) return false;
			region_start = atoi(line.substr(startpos, endpos-startpos).c_str());
		}
		else
			return false;

		startpos = line.find_first_not_of(" \t\r\n", endpos);
		if (startpos == string::npos) return false;
		if (line[startpos] == '-')
		{
			startpos = line.find_first_not_of(" \t\r\n", startpos+1);
			if (startpos==string::npos) return false;
			if (line[startpos] == '*')
				region_end = -1;
			else if (isdigit(line[startpos]))
			{
				endpos = line.find_first_of(" \t]", startpos);
				if (endpos == string::npos) return false;
				region_end = atoi(line.substr(startpos, endpos-startpos).c_str());
			}
			else
				return false;

		}
		else if (line[startpos] == ']')
			region_end = region_start;
		else
			return false;

		cur_policy.charge_region.first = region_start;
		cur_policy.charge_region.second = region_end;

		return true;
	}

	bool ForbidPolicy::ParseTimeMap(const string &line)
	{
		Region forbidtime_key;
		int forbidtime_value;

		size_t startpos=0, endpos;

		if (line[startpos] == '*')
		{
			forbidtime_key.first = -1;
			endpos = startpos+1;
		}
		else if (isdigit(line[startpos])) 
		{
			endpos = line.find_first_of(" \t-=", startpos);
			if (endpos == string::npos) return false;
			forbidtime_key.first = atoi(line.substr(startpos, endpos-startpos).c_str());
		}
		else
			return false;

		startpos = line.find_first_not_of(" \t", endpos);
		if (startpos == string::npos) return false;

		forbidtime_key.second = forbidtime_key.first;
		if (line[startpos] == '-')
		{
			startpos = line.find_first_not_of(" \t", startpos+1);
			if (startpos==string::npos) return false;

			if (line[startpos] == '*')
			{
				forbidtime_key.second = -1;
				endpos = startpos+1;
			}
			else if (isdigit(line[startpos]))
			{
				endpos = line.find_first_of(" \t=", startpos);
				if (endpos == string::npos) return false;

				forbidtime_key.second = atoi(line.substr(startpos, endpos-startpos).c_str());
			}
			else
				return false;

			startpos = line.find_first_not_of(" \t", endpos);
			if (startpos == string::npos) return false;
		}

		if (line[startpos]=='=' && line[startpos+1]=='>')
			startpos += 2;
		else
			return false;

		startpos = line.find_first_not_of(" \t", startpos);
		if (startpos==string::npos || !isdigit(line[startpos])) return false;

		endpos = line.find_last_not_of(" \t\r\n");
		if (endpos == string::npos) return false;

		forbidtime_value = atoi(line.substr(startpos, endpos-startpos+1).c_str());

		cur_policy.time_map[forbidtime_key]= forbidtime_value;

		return true;
	}
	
	bool ForbidPolicy::LoadPolicy(const string & file)
	{
		ifstream iof(file.c_str());
		if (iof.is_open())
		{
			policy_set.clear();

			string line;
			bool newpolicy = false;
			std::getline(iof, line);
			while (!iof.fail())
			{
				size_t linestart = line.find_first_not_of(" \t\r\n", 0);

				if (linestart != string::npos)
				{
					line = line.substr(linestart);

					if (line[0] == '[')
					{
						if (newpolicy)
						{
							policy_set.push_back(cur_policy);
							cur_policy.time_map.clear();
						}
						else
							newpolicy = true;

						if (!ParseChargeRegion(line)) return false;
					}
					else if (line[0] != ';') //comment
						if (!ParseTimeMap(line)) return false;
				}
				
				std::getline(iof, line);
			}
			policy_set.push_back(cur_policy);

			return true;
		}
		else
		{
			policy_set.clear();
			return false;
		}
	}

	int ForbidPolicy::GetForbidTime(int cash ,int origin_forbidtime)
	{
		for (size_t i = 0;  i < policy_set.size(); i++)
		{
			const Region &charge_region = policy_set[i].charge_region;
			const map<Region, int> &time_map = policy_set[i].time_map;
			if ((charge_region.first==-1 || cash>=charge_region.first) && (charge_region.second==-1 || cash<=charge_region.second))
			{
				for (map<Region, int>::const_iterator itr=time_map.begin(); itr!=time_map.end(); itr++)
				{
					if ((itr->first.first==-1 || itr->first.first<=origin_forbidtime) 
						&& (itr->first.second==-1 || itr->first.second>=origin_forbidtime))
						return itr->second;
				}
			}
		}
		return -1;
	}

#ifdef _DEBUGFORBIDPOLICY
	void ForbidPolicy::DumpPolicy()
	{
		for (size_t i = 0; i < policy_set.size(); i++)
		{
			printf("charge region: %d-%d\n", policy_set[i].charge_region.first,  policy_set[i].charge_region.second);
			for (map<Region, int>::const_iterator itr=policy_set[i].time_map.begin(); itr!=policy_set[i].time_map.end(); itr++)
			{
				printf("[%d-%d] => %d\n", itr->first.first, itr->first.second, itr->second);
			}
			printf("\n");
		}
	}
#endif
};

#ifdef _DEBUGFORBIDPOLICY
int main(void)
{
	using namespace GNET;
	if (!ForbidPolicy::GetInstance()->LoadPolicy("forbidpolicy.conf"))
	{
		printf("Load policy file error\n");
		return 1;
	}
	//ForbidPolicy::GetInstance()->DumpPolicy();

	int cash[] = {-1, 0, 31, 44, 45, 46, 200, 399, 400, 401, 600, 799, 800, 801, 802, 1000, 1999, 2000, 2001, 5000};
	int forbidtime[] = {31104000, 259200, 172800, 86400};
	for (int i = 0; i < sizeof(cash)/sizeof(cash[0]); i++)
	{
		printf("charge %d:\n", cash[i]);
		for (int j = 0; j < sizeof(forbidtime)/sizeof(forbidtime[0]); j++)
			printf("%d => %d\n", forbidtime[j], ForbidPolicy::GetInstance()->GetForbidTime(cash[i], forbidtime[j]));
		printf("\n");
	}
	return 0;
}
#endif
