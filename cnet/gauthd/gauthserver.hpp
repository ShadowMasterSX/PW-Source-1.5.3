#ifndef __GNET_GAUTHSERVER_HPP
#define __GNET_GAUTHSERVER_HPP

#include "protocol.h"

namespace GNET
{
struct UserInfo
{
	unsigned int 	sid;
	unsigned int	localsid;
	char			zoneid;
	UserInfo() { }
	UserInfo(unsigned int _s,unsigned int _l,char _z) : sid(_s),localsid(_l),zoneid(_z) { }
};
class GAuthServer : public Protocol::Manager
{
	static GAuthServer instance;
	size_t		accumulate_limit;
	const Session::State *GetInitState() const;
	bool OnCheckAccumulate(size_t size) const { return accumulate_limit == 0 || size < accumulate_limit; }
	void OnAddSession(Session::ID sid);
	void OnDelSession(Session::ID sid);
public:
	static GAuthServer *GetInstance() { return &instance; }
	std::string Identification() const { return "GAuthServer"; }
	void SetAccumulate(size_t size) { accumulate_limit = size; }
	GAuthServer() : accumulate_limit(0) { }
	Octets shared_key;
	bool ValidUser(Session::ID sid,int userid);
	
	Thread::RWLock locker_map;
	typedef std::map<int,UserInfo> UserMap;
   	UserMap usermap;	

	Thread::Mutex locker_accntmap;
	typedef std::map<int/*userid*/,unsigned int/* onlinetime*/> AccountingMap;
	AccountingMap accntmap;
	
	Thread::Mutex locker_zonemap;
	typedef std::map<unsigned int/*session id*/,char/*zoneid*/> ZoneMap;
	ZoneMap zonemap;
};

};
#endif
