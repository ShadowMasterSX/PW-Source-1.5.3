
#ifndef __GNET_ACSTATUSANNOUNCE_HPP
#define __GNET_ACSTATUSANNOUNCE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "aconlinestatus"
#include "userdatamanager.hpp"
#include "punishmanager.hpp"

namespace GNET
{

class ACStatusAnnounce : public GNET::Protocol
{
	#include "acstatusannounce"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		if( _STATUS_ONGAME == status )
        {
            for(size_t i=0; i< info_list.size(); ++i)
            {
                int &uid = info_list[i].roleid;
                if( false == UserDataManager::GetInstance()->UserLogin(uid, info_list[i].ip) )
                    PunishManager::GetInstance()->DeliverCheater(Cheater(uid, Cheater::CH_MULTI_LOGIN, 0));
            }
        }
        else if( _STATUS_OFFLINE == status )
        {
            for(size_t i=0; i< info_list.size(); ++i)
            {
                int &uid = info_list[i].roleid;
                if( false == UserDataManager::GetInstance()->UserLogout(uid) )
                    PunishManager::GetInstance()->DeliverCheater(Cheater(uid, Cheater::CH_MULTI_LOGOUT, 0));
            }
        }
	}
};

};

#endif
