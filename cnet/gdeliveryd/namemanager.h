#ifndef __GNET_NAME_MANAGER_H
#define __GNET_NAME_MANAGER_H

#include <map>

#include "octets.h"
#include "groleidandname"
#include "thread.h"
#include "grolenamehis"

namespace GNET
{

class NameManager
{
	Thread::Mutex   locker_namemanager;
	typedef std::map<int, Octets> NameMap;
	NameManager() {}
public:
	static NameManager * GetInstance() {static NameManager instance; return &instance; }

	bool AddName(int id, const Octets &name);
	bool DelName(int id);
	bool FindName(int id, Octets &name);

	int GetDBSid() {return m_dbsid;}

	bool IsInit();
	void OnDBConnect(Protocol::Manager *manager, int sid);
	void InitName(std::vector<GRoleIDandName> & rolenamelist, char finish);
	// 用来检测新的名字是否已经存在和是否合法
	int CheckNewName(Octets &name);
private:
	bool		m_init;
	int 		m_dbsid;
	NameMap		m_namemap;
};

}

#endif
