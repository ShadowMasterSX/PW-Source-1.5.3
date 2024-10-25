#ifndef __MY_DB
#define __MY_DB

#include "stdlib.h"
#include <string>
#include <vector>

struct CheatItem
{
	time_t date;
	int roleid;
	int type;
	std::string reason;
	std::string XmlString() const;
};

class DB
{
	static DB instance;
public:
	static DB *GetInstance() { return &instance; }
	bool Write(const std::vector<CheatItem> &cheats);
};

#endif
