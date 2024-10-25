#ifndef __GNET_CHEATERSDB
#define __GNET_CHEATERSDB

#include <vector>
#include "xmldb.h"


namespace GNET
{


class CheatersDB : public XmlDB
{
	int area_id;
	int cheat_id;
	time_t time_begin;
public:
	struct Item
	{
		int uid;
		int subid;
		time_t ctime;
		Item() { }
		Item(int _uid, int _subid, time_t _ctime) : uid(_uid), subid(_subid), ctime(_ctime) { }
	};
private:
	std::vector<Item> cheaters;
	enum { TIME_INTERVAL = 7 /* days */ };
public:
	std::string trim(std::string const& source, char const* delims = " \t\r\n") const;
	void AddItem(const Item& i) { cheaters.push_back(i); }
	static void SaveCheaters(const std::map<int, std::pair<std::string, std::vector<std::pair<int, std::pair<int, time_t> > > > > &c, int aid);
	~CheatersDB() { }
	std::string GetFileName() const; 
	XmlObject WriteObject() const; 
	void LoadObject(const XmlObject &_obj);	
};

};

#endif
