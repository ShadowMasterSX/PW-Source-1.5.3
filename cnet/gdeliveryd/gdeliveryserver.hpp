#ifndef __GNET_GDELIVERYSERVER_HPP
#define __GNET_GDELIVERYSERVER_HPP

#include "protocol.h"
#include "macros.h"
#include "localmacro.h"

#include "cache.h"
#include "groleforbid"
#include "grolebase"
#include "hashstring.h"
#include <map>
#include <set>
#include <vector>
#include "serverattr.h"
namespace GNET
{
class GDeliveryServer : public Protocol::Manager
{
	static GDeliveryServer instance;
	size_t accumulate_limit;
	int debugmode;
	const Session::State *GetInitState() const;
	bool OnCheckAccumulate(size_t size) const { return accumulate_limit == 0 || size < accumulate_limit; }
	void OnAddSession(Session::ID sid);
	void OnDelSession(Session::ID sid);
	
	//cross server related
	bool is_central_ds;
	unsigned int link_version;
	Octets gs_edition;	

	typedef std::map<Session::ID, unsigned char> LinkTypes;
	LinkTypes link_types;

public:
	static GDeliveryServer *GetInstance() { return &instance; }
	std::string Identification() const { return "GDeliveryServer"; }
	void SetAccumulate(size_t size) { accumulate_limit = size; }
	GDeliveryServer() : accumulate_limit(0), challenge_algo(0), token_login_support(false), max_name_len(18)
	{
		rbcache.init(102400,-1,900);	
		iweb_sid = _SID_INVALID;
		link_version = 0;
		is_central_ds = false;
	}

	bool IsFreeZone() { return aid==freeaid; }
	//rolebase info cache
	typedef Cache<int/*roleid*/,GRoleBase> RoleBaseCache;
	RoleBaseCache rbcache;	

	char	zoneid; //game zone id
	char	aid;	//accounting area id
	char    freeaid;
	int	freecreatime;
	char	challenge_algo;
	bool	token_login_support;
	unsigned int max_name_len;
	int district_id;  //服务器所在区

	int GetZoneid(){ return (unsigned char)zoneid; }
	char ChallengeAlgo() const { return challenge_algo;}
	bool IsFreeForAll() const { return freecreatime == 0 ; }
	bool IsFreeForNone() const { return freecreatime == ~0 ; }
	bool IsFreeForUser(int creatime ) const { 
		return freecreatime == 0 || freecreatime != ~0 && freecreatime < creatime;
	}
	unsigned int iweb_sid;	//sid of the webservice server
	ServerAttr serverAttr;
	void SetDebugmode(int mode) { debugmode = mode; }
	int  GetDebugmode() { return debugmode; }

	bool IsCentralDS() { return is_central_ds; }
	void SetCentralDS(bool b) { is_central_ds = b; }
	void SetVersion(unsigned int ver) { link_version = ver; }
	unsigned int GetVersion() { return link_version; }
	void SetEdition(const Octets & edi) { gs_edition = edi; }
	Octets GetEdition() { return gs_edition; }
	void BroadcastStatus();

	inline void InsertLinkType(Session::ID sid, unsigned char type) { link_types[sid] = type; }
	inline void RemoveLinkType(Session::ID sid) {
		LinkTypes::iterator it = link_types.find(sid);
	   	if (it != link_types.end())	link_types.erase(it);
	}
	inline bool IsPhoneLink(Session::ID sid) {
		LinkTypes::iterator it = link_types.find(sid);
		if (it != link_types.end())
			return LINK_TYPE_PHONE == it->second;
		return false;
	}
};

class StatusAnnouncer : public Thread::Runnable
{
        int update_time;

public:
        StatusAnnouncer(int _time,int _proir=1) : Runnable(_proir),update_time(_time) { }
        void Run();
};

};
#endif
