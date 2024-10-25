
#ifndef __GNET_C2SGAMEDATASEND_HPP
#define __GNET_C2SGAMEDATASEND_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#ifdef _TESTCODE
#include "s2cgamedatasend.hpp"
#include "s2cmulticast.hpp"
#include "gproviderclient.hpp"
#endif


void handle_user_cmd(int cs_index, int sid,int uid, const void * buf, size_t size);

namespace GMSV{
	void queue_user_cmd(int cs_index, int sid, int uid,Octets & data);
}
namespace GNET
{

#ifdef __USE_SPEC_GAMEDATASEND__
enum 
{
	GDS_PRIOR = 0 
};
#else
enum 
{
	GDS_PRIOR = 100
};
#endif

class C2SGamedataSend : public GNET::Protocol
{
	public:
		int roleid;
		unsigned int localsid;
		Octets data;
		enum { PROTOCOL_TYPE = PROTOCOL_C2SGAMEDATASEND };
	public:
        C2SGamedataSend() { type = PROTOCOL_C2SGAMEDATASEND; }
		C2SGamedataSend(void*) : Protocol(PROTOCOL_C2SGAMEDATASEND) { }
		C2SGamedataSend (int l_roleid,unsigned int l_localsid,const Octets& l_data)
			 : roleid(l_roleid),localsid(l_localsid),data(l_data)
		{
			type = PROTOCOL_C2SGAMEDATASEND;
		}


		C2SGamedataSend(const C2SGamedataSend &rhs)
			: Protocol(rhs),roleid(rhs.roleid),localsid(rhs.localsid),data(rhs.data) { }

		GNET::Protocol *Clone() const { return new C2SGamedataSend(*this); }

		OctetsStream& marshal(OctetsStream & os) const
		{
			os << roleid;
			os << localsid;
			os << data;
			return os;
		}

		const OctetsStream& unmarshal(const OctetsStream &os)
		{
			os >> roleid;
			os >> localsid;
			os >> data;
			return os;
		}

		int PriorPolicy( ) const { return GDS_PRIOR; }

		bool SizePolicy(size_t size) const { return size <= 10240; }

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
#ifdef _TESTCODE //liping
		DEBUG_PRINT ("gamed(%d)::receive data from linkd(roleid=%d,localsid=%d): %.*s\n",GProviderClient::GetGameServerID(),roleid,localsid,data.size(),(char*)data.begin());
		/*char p[256];
		for (int i=0;i<50;i++)
		{
			sprintf(p,"%d",2*i);
			Octets os(p,strlen(p));
			manager->Send(sid,S2CGamedataSend(roleid,localsid,os));
			sprintf(p,"%d",2*i+1);
			os.replace(p,strlen(p));
			S2CMulticast s2cmulticast(PROTOCOL_S2CMULTICAST);
			s2cmulticast.data=os;
			s2cmulticast.playerlist.add(Player(roleid,localsid));
			manager->Send(sid,s2cmulticast);
		}*/
		/*
		S2CMulticast s2cmulticast(PROTOCOL_S2CMULTICAST);
		s2cmulticast.data=data;
		s2cmulticast.playerlist.add(Player(roleid,localsid));
		s2cmulticast.playerlist.add(Player(roleid,localsid));
		s2cmulticast.playerlist.add(Player(roleid,localsid));
		s2cmulticast.playerlist.add(Player(roleid,localsid));
		manager->Send(sid,s2cmulticast);
		*/
#endif

#ifdef __USE_SPEC_GAMEDATASEND__
		GMSV::queue_user_cmd(((GProviderClient*)manager)->GetProviderServerID(),localsid,roleid,data);
#else
		handle_user_cmd(((GProviderClient*)manager)->GetProviderServerID(),localsid,roleid,data.begin(),data.size());
#endif
	}
};

};

#endif

