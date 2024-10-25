
#include "patternmanager.hpp"

namespace GNET
{

PatternManager PatternManager::instance;

void PatternManager::AppendPattern(const PatternMap &pm)
{
	for(PatternMap::const_iterator it = pm.begin(), ie = pm.end(); it != ie; ++it)
		pattern_map[(*it).first] += (*it).second;
}

};

