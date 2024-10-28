
#ifndef __GNET_GETENEMYLIST_HPP
#define __GNET_GETENEMYLIST_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "mapuser.h"
#include "namemanager.h"
#include "gdeliveryserver.hpp"
#include "getenemylist_re.hpp"


namespace GNET
{

class GetEnemyList : public GNET::Protocol
{
	#include "getenemylist"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
        Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
        PlayerInfo* pinfo = UserContainer::GetInstance().FindRole(roleid);
        if (pinfo == NULL) return;
        if (pinfo->friend_ver < 0) return;

        GEnemyListVector& enemylist = pinfo->enemylistinfo;
        if (enemylist.size() == 0) return;

        Octets name;
        GetEnemyList_Re re;
        re.retcode = 0;
        re.roleid = roleid;
        re.status.GetVector().reserve(enemylist.size());
        re.localsid = localsid;

        GEnemyListVector::iterator iter = enemylist.begin(), iter_end = enemylist.end();
        for (; iter != iter_end; ++iter)
        {
            if (NameManager::GetInstance()->FindName(iter->rid, name))
                iter->name = name;

            if (UserContainer::GetInstance().FindRoleOnline(iter->rid) == NULL)
                re.status.push_back(0);
            else 
                re.status.push_back(1);
        }

        re.enemylist = enemylist;
        GDeliveryServer::GetInstance()->Send(sid, re);
	}
};

};

#endif
