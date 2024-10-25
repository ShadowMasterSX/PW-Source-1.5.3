#ifndef __GNET_UNIQUENAMESERVER_HPP
#define __GNET_UNIQUENAMESERVER_HPP

#include "thread.h"
#include "protocol.h"
#include "conv_charset.h"
#include "storage.h"
#include "dbbuffer.h"
#include "localmacro.h"

namespace GNET
{

#define	UNIQUENAME_ENGAGED	1
#define UNIQUENAME_USED		2
#define UNIQUENAME_OBSOLETE 3

#define ROLELIST_DEFAULT	0x80000000
#define MAX_ROLE_COUNT		16

class RoleList
{
	unsigned int rolelist;
	int cursor;
public:
	RoleList() : rolelist(ROLELIST_DEFAULT),cursor(0) { }
	RoleList(unsigned int r) : rolelist(r),cursor(0) { }
	~RoleList() { }
	void operator=(const RoleList& rhs) 
	{ 
		rolelist = rhs.rolelist; 
		cursor   = rhs.cursor;
	}
	bool ValidList()
	{
		return (rolelist & ROLELIST_DEFAULT) != 0;
	}
	bool IsRoleExist(int roleid)
	{
		return (rolelist & (1<<(roleid % MAX_ROLE_COUNT))) != 0;
	}
	void SetRoleList(unsigned int r)
	{
		rolelist = r;
		cursor = 0;
	}
	unsigned int GetRoleList() 
	{
		return rolelist;
	}
	int GetRoleCount()
	{
		int count=0;
		for (int id=0;id<MAX_ROLE_COUNT;id++)
			if (IsRoleExist(id)) count++;
		return count;
	}
	int AddRole()
	{
		if (!ValidList())  
			return -1;
		for (int id=0;id<MAX_ROLE_COUNT;id++)
		{
			if(!IsRoleExist(id))
			{
				rolelist += (1<<id);
				return id;	
			}
		}
		return -1;
	}		
	int AddRole(int roleid)
	{
		if (!ValidList() || roleid<0 || roleid>MAX_ROLE_COUNT-1)  
			return -1;
		if (IsRoleExist(roleid)) 
			return roleid;  //the role will be overwritten
		rolelist += (1<<roleid);  
		return roleid;
	}
	bool DelRole(int roleid)
	{
		if (!ValidList() || !IsRoleExist(roleid)) 
			return false;
		return (rolelist -= 1<<(roleid % MAX_ROLE_COUNT)) != 0;
	}
	int GetNextRole()
	{
		while (cursor<MAX_ROLE_COUNT)
			if (IsRoleExist(cursor++)) return cursor-1;
		return -1;
	}
	void SeekToBegin()
	{
		cursor = 0;
	}	
};

class LogicuidSeeker : public Thread::Runnable
{
public:
	LogicuidSeeker(int prior=1) : Runnable(prior) { }
	void Run();
};

class LogicuidManager
{
	static LogicuidManager instance;
	Thread::Mutex   locker;
	std::vector<unsigned int>   idset;

	unsigned int  startid;
	bool busy;
public:
	static LogicuidManager& Instance() { return instance; }
	LogicuidManager() : startid(0), busy(false) { }
	size_t  FindFreeLogicuid();
	unsigned int     AllocLogicuid();
};

class UniqueNameServer : public Protocol::Manager
{
	static UniqueNameServer instance;
	size_t		accumulate_limit;
	bool case_insensitive;
	const Session::State *GetInitState() const;
	bool OnCheckAccumulate(size_t size) const { return accumulate_limit == 0 || size < accumulate_limit; }
	void OnAddSession(Session::ID sid);
	void OnDelSession(Session::ID sid);
	void OnSetTransport(Session::ID sid, const SockAddr& local, const SockAddr& peer);
public:
	static UniqueNameServer *GetInstance() { return &instance; }
	std::string Identification() const { return "UniqueNameServer"; }
	void SetAccumulate(size_t size) { accumulate_limit = size; }
	void SetSensitivity(bool flag) { case_insensitive = flag; }
	UniqueNameServer() : accumulate_limit(0),case_insensitive(0) { }
	void TransformName(Octets& name);
};

};
#endif
