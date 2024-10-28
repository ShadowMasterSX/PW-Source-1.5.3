
#include "gfactionserver.hpp"
#include "state.hxx"
#include "onlineplayerstatus"
#include <algorithm>
#include "playerfactioninfo_re.hpp"
#include "gproviderserver.hpp"
#include "gfs_io.h"

#include "factiondb.h"
namespace GNET
{

GFactionServer GFactionServer::instance;

const Protocol::Manager::Session::State* GFactionServer::GetInitState() const
{
	return &state_GFactionServer;
}

void GFactionServer::OnAddSession(Session::ID sid)
{
	//TODO
}

void GFactionServer::OnDelSession(Session::ID sid)
{
	UnRegisterSession(sid);
}

//online user management
size_t GFactionServer::GetOnlineMember(unsigned int fid,IntVector& list)
{
	Thread::Mutex::Scoped l(locker_player);
	for ( FactionMemberMap::iterator
				itb=factionmembermap.lower_bound(fid),
				ite=factionmembermap.upper_bound(fid);
				itb!=ite; ++itb )
	{
		list.push_back( (*itb).second->roleid );
	}	
	return list.size();
}

void GFactionServer::RegisterPlayer(const OnlinePlayerStatus& ops, unsigned int fid,char froleid)
{
	Thread::Mutex::Scoped l(locker_player);
	PlayerMap::iterator it=playermap.find(ops.roleid);
	if ( it==playermap.end() )
	{
		it=playermap.insert( std::make_pair(ops.roleid,HardReference<Player>(new Player(ops.roleid,ops.gid,ops.link_id,ops.link_sid,fid,froleid))) ).first;
		if (fid!=_FACTION_ID_INVALID)
			factionmembermap.insert( std::make_pair(fid,HardReference<Player>((*it).second)) );
	}
	else
	{
		bool blFactionChanged = (*it).second->faction_id!=fid;
		(*it).second->roleid=ops.roleid;
		(*it).second->gsid=ops.gid;
		(*it).second->linkid=ops.link_id;
		(*it).second->localsid=ops.link_sid;
		//erase player from his old faction,if his faction changed
		if ( (*it).second->faction_id!=_FACTION_ID_INVALID && blFactionChanged )
		{
			unsigned int old_fid=(*it).second->faction_id;
			FactionMemberMap::iterator itfm,itend;
			itend=factionmembermap.upper_bound(old_fid);
			itfm=std::find_if(
					factionmembermap.lower_bound(old_fid),
					itend,
					Compare(old_fid,ops.roleid));
			if ( itfm!=itend)
				factionmembermap.erase(itfm);
		}
		//add player to its new facton
		(*it).second->faction_id=fid;	
		(*it).second->froleid=froleid;
		if ( blFactionChanged && fid!=_FACTION_ID_INVALID )
			factionmembermap.insert( std::make_pair(fid,HardReference<Player>((*it).second))  );
	}
}

void GFactionServer::UpdatePlayer(int roleid, int level, int reputation , unsigned char reincarn_times , unsigned char gender,GUserFaction& user, bool blSend2Game)
{
	Thread::Mutex::Scoped l(locker_player);
	PlayerMap::iterator it=playermap.find(roleid);
	if ( it!=playermap.end() )
	{
		bool blFactionChanged = (*it).second->faction_id!=user.fid;
		//erase player from his old faction,if his faction changed
		if ( (*it).second->faction_id!=_FACTION_ID_INVALID && blFactionChanged )
		{
			unsigned int old_fid=(*it).second->faction_id;
			FactionMemberMap::iterator itfm,itend;
			itend=factionmembermap.upper_bound(old_fid);
			itfm=std::find_if( factionmembermap.lower_bound(old_fid), itend, Compare(old_fid,roleid));
			if ( itfm!=itend )
				factionmembermap.erase(itfm);
		}
		//add player to its new facton
		(*it).second->faction_id = user.fid;
		(*it).second->froleid    = user.role;
		(*it).second->cls        = user.cls;
		(*it).second->level      = level;
		(*it).second->reputation = reputation;
		(*it).second->reincarn_times = reincarn_times;
		(*it).second->gender = gender;
		(*it).second->name       = user.name;
		if(user.extend.size()>0)
		{
			try{
				Marshal::OctetsStream os(user.extend);
				os >> (*it).second->ext;
			}catch(Marshal::Exception){ }
		}
		if ( blFactionChanged && user.fid!=_FACTION_ID_INVALID )
		{
			factionmembermap.insert( std::make_pair(user.fid,HardReference<Player>((*it).second)) );
		}

		if ( blSend2Game )
		{
	 		//send player's new faction info to gameserver
			PlayerFactionInfo_Re pfi_re(ERR_SUCCESS,PFactionInfoVector());
			pfi_re.faction_info.add(PFactionInfo(roleid,user.fid,user.role,Factiondb::GetInstance()->GetPvpMask(user.fid),Factiondb::GetInstance()->GetUnifid(user.fid)));
			GProviderServer::GetInstance()->DispatchProtocol((*it).second->gsid,pfi_re);
		}
	}
}

void GFactionServer::SetChatEmotion(int roleid, unsigned char emotion)
{
	Thread::Mutex::Scoped l(locker_player);
	PlayerMap::iterator it=playermap.find(roleid);
	if ( it!=playermap.end() )
		(*it).second->emotion = emotion;
}

void GFactionServer::UpdatePlayer(int roleid, unsigned int fid,char froleid,bool blSend2Game)
{
	//DEBUG_PRINT("gfactionserver::UpdatePlayer: Update received, roleid=%d,fid=%d,froleid=%d\n",
	//		roleid,fid,froleid);
	Thread::Mutex::Scoped l(locker_player);
	PlayerMap::iterator it=playermap.find(roleid);
	if ( it!=playermap.end() )
	{
		bool blFactionChanged = (*it).second->faction_id!=fid;
		//erase player from his old faction,if his faction changed
		if ( (*it).second->faction_id!=_FACTION_ID_INVALID && blFactionChanged )
		{
			unsigned int old_fid=(*it).second->faction_id;
			FactionMemberMap::iterator itfm,itend;
			itend=factionmembermap.upper_bound(old_fid);
			itfm=std::find_if(
					factionmembermap.lower_bound(old_fid),
					itend,
					Compare(old_fid,roleid));
			if ( itfm!=itend )
				factionmembermap.erase(itfm);
		}
		//add player to its new facton
		if(fid!=(*it).second->faction_id)
			(*it).second->ext.jointime = Timer::GetTime();
		(*it).second->faction_id=fid;
		(*it).second->froleid=froleid;
		if ( blFactionChanged && fid!=_FACTION_ID_INVALID )
		{
			factionmembermap.insert( std::make_pair(fid,HardReference<Player>((*it).second)) );
		}

		if ( blSend2Game )
		{
	 		//send player's new faction info to gameserver
			PlayerFactionInfo_Re pfi_re(ERR_SUCCESS,PFactionInfoVector());
			pfi_re.faction_info.add(PFactionInfo(roleid,fid,froleid/*faction role*/,Factiondb::GetInstance()->GetPvpMask(fid),Factiondb::GetInstance()->GetUnifid(fid)));
			GProviderServer::GetInstance()->DispatchProtocol((*it).second->gsid,pfi_re);
		}
	}
}
void GFactionServer::UnRegisterPlayer(int roleid)
{
	bool blFindPlayer=false;
	int           __roleid=0;
	unsigned int  __faction_id=0;
	{
		Thread::Mutex::Scoped l(locker_player);
		PlayerMap::iterator itp=playermap.find(roleid);
		if ( itp!=playermap.end() )
		{
			//TODO:delete player from factionmember map
			int fid=(*itp).second->faction_id;
			if ( fid!=_FACTION_ID_INVALID )
			{
				FactionMemberMap::iterator itfm,itend;
				itend=factionmembermap.upper_bound(fid);
				itfm=std::find_if(
						factionmembermap.lower_bound(fid),
						itend,
						Compare(fid,roleid));
				if ( itfm!=itend )
				{
					factionmembermap.erase(itfm);
				}
			}
			blFindPlayer=true;
			__roleid=(*itp).second->roleid;
			__faction_id=(*itp).second->faction_id;
			playermap.erase(itp);
		}
	}
	Factiondb::GetInstance()->OnLogout( __faction_id,__roleid );
}
bool GFactionServer::IsPlayerOnline(int roleid)
{
	Thread::Mutex::Scoped l(locker_player);
	return playermap.find(roleid)!=playermap.end();
}
bool GFactionServer::GetPlayer(int roleid,Player& player)
{
	Thread::Mutex::Scoped l(locker_player);
	PlayerMap::iterator itp=playermap.find(roleid);
	if (itp!=playermap.end())
	{
		player=*(*itp).second;
		return true;
	}
	return false;
}

//linkserver,deliveryserver management
void GFactionServer::UnRegisterSession(unsigned int sid)
{
	Thread::Mutex::Scoped l(locker_server);
	for (LinkSidMap::iterator it=linksidmap.begin();it!=linksidmap.end();it++)
	{
		if ((*it).second==sid) 
		{
		   	DEBUG_PRINT("gfactionserver::ondelsession: erase link(or delivery) %d from map.\n",(*it).first);	
			linksidmap.erase(it); 
			return; 
		}
	}
}
bool GFactionServer::RegisterLink(int glink_id,unsigned int sid)
{
	Thread::Mutex::Scoped l(locker_server);
	if (linksidmap.find(glink_id) != linksidmap.end()) return false;
	linksidmap[glink_id]=sid;
	return true;
}
void GFactionServer::UnRegisterLink(unsigned int glink_id)
{
	Thread::Mutex::Scoped l(locker_server);
	linksidmap.erase(glink_id);
}

void GFactionServer::BroadcastLink(const Protocol& p)
{
	Thread::Mutex::Scoped l(locker_server);
	for(LinkSidMap::iterator it=linksidmap.begin();it!=linksidmap.end();++it)
	{
		if(it->first)
			Send(it->second,p);
	}
}

void GFactionServer::AddMember(int fid, int superior, int roleid, int level, char cls, int reputation, unsigned char reincarn_times, unsigned char gender, const Octets& name)
{
	if(!fid)
		return;
	Thread::Mutex::Scoped l(locker_player);
	PlayerMap::iterator it=playermap.find(roleid);
	if(it!=playermap.end())
	{
		if ((*it).second->faction_id!=_FACTION_ID_INVALID)
		{
			unsigned int old_fid = (*it).second->faction_id;
			FactionMemberMap::iterator itfm,itend;
			itend = factionmembermap.upper_bound(old_fid);
			itfm = std::find_if( factionmembermap.lower_bound(old_fid), itend, Compare(old_fid,roleid));
			if ( itfm!=itend )
				factionmembermap.erase(itfm);
		}
		(*it).second->faction_id = fid;
		(*it).second->froleid    = _R_MEMBER;
		(*it).second->ext.jointime = Timer::GetTime();
		(*it).second->cls = cls;
		(*it).second->level = level;
		(*it).second->reputation = reputation;
		(*it).second->reincarn_times = reincarn_times;
		(*it).second->gender = gender;
		(*it).second->name = name;

		factionmembermap.insert( std::make_pair(fid,HardReference<Player>((*it).second)) );

		PlayerFactionInfo_Re pfi_re(ERR_SUCCESS,PFactionInfoVector());
		pfi_re.faction_info.add(PFactionInfo(roleid,fid,_R_MEMBER,Factiondb::GetInstance()->GetPvpMask(fid),Factiondb::GetInstance()->GetUnifid(fid)));
		GProviderServer::GetInstance()->DispatchProtocol((*it).second->gsid,pfi_re);

		gfs_broadcast_factionacceptjoin_re(fid,superior,roleid,level,cls,reputation,reincarn_times,gender,name);
	}
}

void GFactionServer::OnPlayerRename(int roleid, Octets& new_name)
{
	Thread::Mutex::Scoped l(locker_player);
	PlayerMap::iterator it=playermap.find(roleid);
	if(it!=playermap.end())
	{
		(*it).second->name = new_name;
	}
}

};
