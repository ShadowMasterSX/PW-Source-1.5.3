#ifndef __GNET_GFACTIONSERVER_HPP
#define __GNET_GFACTIONSERVER_HPP

#include "protocol.h"
#include "../common/reference.h"
#include "rpcdefs.h"
#include "gfactionextend"
#include "guserfaction"
namespace GNET
{
// faction server is the server for glinkd and gdelivery, to handle all faction operations
// coming from client. It established a map to manage all linkserver's ID and delivery's ID
class OnlinePlayerStatus;	
class GFactionServer : public Protocol::Manager
{
	static GFactionServer instance;
	size_t		accumulate_limit;
	const Session::State *GetInitState() const;
	bool OnCheckAccumulate(size_t size) const { return accumulate_limit == 0 || size < accumulate_limit; }
	void OnAddSession(Session::ID sid);
	void OnDelSession(Session::ID sid);
	
public:
	//cross server related
	bool is_central_faction;

	//define a link(delivery) server manager
	Thread::Mutex locker_server;
	typedef std::map< unsigned int/* link or deliver id */,unsigned int /* session id */> LinkSidMap;
	LinkSidMap linksidmap;
	//define a online user manager
	struct Player
	{
		int            roleid;
		int            gsid;
		int            linkid;
		unsigned int   localsid;
		unsigned int   faction_id;
		char           froleid;
		char           cls;
		unsigned char  emotion;
		int            level;
		int			   reputation;
		unsigned char  reincarn_times;
		unsigned char  gender;
		Octets         name;
		GFactionExtend ext;

		Player(int _roleid=-1,int _gsid=-1,int _linkid=-1,unsigned int _localsid=0,unsigned int _fid=0,
			char _froleid=-1)
			: roleid(_roleid),gsid(_gsid),linkid(_linkid),localsid(_localsid),faction_id(_fid),
			froleid(_froleid),cls(0),level(1),reputation(0),reincarn_times(0),gender(0),name(),ext()
		{ 
			emotion = 0;
		}
	};
	Thread::Mutex locker_player;
	typedef std::map<unsigned int /*roleid*/,HardReference<Player> > PlayerMap;
	PlayerMap playermap;
	
	//this structure is used in std::find_if for multimap
	struct Compare
	{
		unsigned int faction_id;
		int roleid;
		Compare(unsigned int _faction_id,int _roleid) : faction_id(_faction_id),roleid(_roleid) { }
		bool operator() (const std::pair<unsigned int,HardReference<Player> >& rhs ) {
			return faction_id==rhs.first && roleid==rhs.second->roleid;
		}
	};
	typedef std::multimap<unsigned int /*faction id*/,HardReference<Player> > FactionMemberMap;
	FactionMemberMap factionmembermap;
	char zoneid;  // game zone id, acquired from delivery server
public:
	static GFactionServer *GetInstance() { return &instance; }
	std::string Identification() const { return "GFactionServer"; }
	void SetAccumulate(size_t size) { accumulate_limit = size; }
	GFactionServer() : accumulate_limit(0), is_central_faction(false),
					   locker_server("GFactionServer::Locker_Server"),
   					   locker_player("GFactionServer::Locker_Player")	{ }

	//online user management
	bool IsPlayerOnline(int roleid);
	void RegisterPlayer(const OnlinePlayerStatus& ops, unsigned int fid, char froleid);
	void UpdatePlayer(int roleid, unsigned int fid,char froleid,bool blSend2Game=true);
	void UpdatePlayer(int roleid, int level, int reputation, unsigned char reincarn_times, unsigned char gender, GUserFaction& user, bool blSend2Game=true);
	void SetChatEmotion(int roleid, unsigned char emotion);
	void UnRegisterPlayer(int roleid);
	bool GetPlayer(int roleid,Player& player);
	void AddMember(int fid, int superior, int roleid, int level, char cls, int reputation, unsigned char reincarn_times, unsigned char gender, const Octets& name);
	void OnPlayerRename(int roleid, Octets& new_name);
	
	size_t GetOnlineMember(unsigned int fid,IntVector& list);
	//linkserver,deliveryserver management
	bool RegisterLink(int glink_id,unsigned int sid);
	void UnRegisterLink(unsigned int glink_id);
	void UnRegisterSession(unsigned int sid);

	//linkserver,deliveryserver send 
	bool DispatchProtocol(unsigned int server_id/* link or deliver id */,const Protocol* p) {
		Thread::Mutex::Scoped l(locker_server);
		LinkSidMap::const_iterator it=linksidmap.find(server_id);
		if (it!=linksidmap.end())
			return Send((*it).second,p);
		else
			return false;
	}
	bool DispatchProtocol(unsigned int server_id/* link or deliver id */,const Protocol& p) {
		return DispatchProtocol(server_id,&p);
	}
	bool DispatchProtocol(unsigned int server_id/* link or deliver id */,      Protocol* p) {
		Thread::Mutex::Scoped l(locker_server);
		LinkSidMap::const_iterator it=linksidmap.find(server_id);
		if (it!=linksidmap.end())
			return Send((*it).second,p);
		else
			return false;
	}
	bool DispatchProtocol(unsigned int server_id/* link or deliver id */,       Protocol& p) {
		return DispatchProtocol(server_id,&p);
	}

	bool SendPlayer(int roleid, const Protocol& p) 
	{
		Player player;
		if(!GetPlayer(roleid, player))
			return false;
		return DispatchProtocol(player.linkid, &p);
	}

	void BroadcastLink(const Protocol& p);

	bool IsCentralFaction() { return is_central_faction; }
	void SetCentralFaction(bool b) { is_central_faction = b; }
};

};
#endif
