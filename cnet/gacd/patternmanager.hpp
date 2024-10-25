#ifndef __GNET_PATTERNMANAGER_HPP
#define __GNET_PATTERNMANAGER_HPP


#include "stackpattern.h"

namespace GNET
{

class PatternManager
{
	PatternMap pattern_map;
	static PatternManager instance;
public:
	~PatternManager() { }

	static PatternManager *GetInstance() { return &instance; }

	void AppendPattern(const PatternMap &pm);
	const PatternMap& GetPatterns() const { return pattern_map; }

private:
	PatternManager() { }
	PatternManager(const PatternManager &cpl);
	PatternManager& operator=(const PatternManager &cpl);
	bool operator==(const PatternManager &cpl) const;
};

};

#endif
