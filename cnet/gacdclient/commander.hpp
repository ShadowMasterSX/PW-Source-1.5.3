#ifndef __GNET_COMMANDER_HPP
#define __GNET_COMMANDER_HPP

#include "protocol.h"
#include "thread.h"
#include "acconstants.h"

namespace GNET
{

class Commander
{
	static Commander instance;
	typedef std::map<std::string, int> CmdsMap;
	CmdsMap cmdsmap;
	CmdsMap parammap;
	enum { DEFAULT_TIMEOUT = 1 };
	int timeout;
	bool parsing;
	std::string cmdcache;
	int* share_p;
public:
	static Commander *GetInstance() { return &instance; }
	Commander() : timeout(0) { 
		share_p = NULL;
		cmdsmap["pat"] = ACCMD_PATTERNS;
		cmdsmap["brief"] = ACCMD_BRIEF;
		cmdsmap["patuser"] = ACCMD_PATTERN_USER;
		cmdsmap["userpat"] = ACCMD_USER_PATTERN;
		cmdsmap["cheaters"] = ACCMD_CHEATERS;

	}
	void SharePointer(int** _p) { share_p = *_p; }
	void TimerUpdate();
	void QueryCheaters();
	void QueryPatterns();
	void ParseCommandLine(std::string line);
	void split(const string &str, std::vector<std::string> &cmds) const;
	std::string replace(const std::string &str, char c1, char c2) const;
	/*
	void ParseResponse(int rescode, int cmd, const Octets &res);
	void DumpBrief(const Octets &res) const;
	void DumpMapIUI(const Octets &res, const char *s1, const char* s2, const char *p="%x");
	void DumpCheaters(const Octets &res) const;
	*/
};

};
#endif
