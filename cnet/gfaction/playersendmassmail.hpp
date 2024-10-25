
#ifndef __GNET_PLAYERSENDMASSMAIL_HPP
#define __GNET_PLAYERSENDMASSMAIL_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "operations/ids.hxx"
#include "gfactionserver.hpp"
#include "factiondb.h"

namespace GNET
{

class PlayerSendMassMail : public GNET::Protocol
{
	#include "playersendmassmail"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		struct ERR
		{
			ERR(int t) : type(t) {}
			int type;
		};

		try
		{
			if(mass_type != MMT_FACTION) throw ERR(1);
		
			GFactionServer::Player player;
			GFactionServer *gfs = GFactionServer::GetInstance();

			if( !gfs->GetPlayer(roleid,player)) throw ERR(2);

			if( !player.faction_id) throw ERR(3);

			if( player.froleid > _R_BODYGUARD ) throw ERR(4);

			if( !Factiondb::GetInstance()->CheckMemberList(player.faction_id,roleid,receiver_list)) throw ERR(5); 

			if( !gfs->DispatchProtocol(0,this)) throw ERR(6);
		}
		catch(ERR e)
		{
			Log::log(LOG_ERR, "Player%d MassMail Send failed%d." ,roleid, e.type);		
		}	

	}
};

};

#endif
