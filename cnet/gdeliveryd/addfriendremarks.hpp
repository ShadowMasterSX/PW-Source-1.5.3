
#ifndef __GNET_ADDFRIENDREMARKS_HPP
#define __GNET_ADDFRIENDREMARKS_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "mapuser.h"
#include "addfriendremarks_re.hpp"


#define MAX_FRIEND_REMARKS_LENGTH 40

enum AddFriendRemarksErrorCode
{
    ERR_SUCCESS             = 0,
    ERR_FRIEND_REMARKS_LEN  = 1,
};


namespace GNET
{

class AddFriendRemarks : public GNET::Protocol
{
	#include "addfriendremarks"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
        int remarks_len = friendremarks.size();
        DEBUG_PRINT("gdelivery::addfriendremarks: receive request. roleid = %d, friendroleid = %d, friendremarks_len = %d, srclsid = %d.\n", roleid, friendroleid, remarks_len, srclsid);

        GDeliveryServer* dsm = GDeliveryServer::GetInstance();
        if (dsm->IsCentralDS())
        {
            DEBUG_PRINT("gdelivery::addfriendremarks: try to add friend remarks on central delivery, refuse. roleid = %d, friendroleid = %d, friendremarks_len = %d, srclsid = %d.\n", roleid, friendroleid, remarks_len, srclsid);
            return;
        }

        Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
        PlayerInfo* pinfo = UserContainer::GetInstance().FindRole(roleid);
        if (pinfo == NULL) return;

        int ret = ERR_SUCCESS;
        if ((0 <= remarks_len) && (remarks_len <= MAX_FRIEND_REMARKS_LENGTH))
        {
            const GFriendInfoVector& friends = pinfo->friends;
            GFriendInfoVector::const_iterator it = friends.begin();
            GFriendInfoVector::const_iterator it_end = friends.end();

            for (; it != it_end; ++it)
            {
                if (it->rid == friendroleid) break;
            }

            if (it == it_end) return;

            GFriendExtInfoVector& friendextinfo = pinfo->friendextinfo;
            GFriendExtInfoVector::iterator iter = friendextinfo.begin();
            GFriendExtInfoVector::iterator iter_end = friendextinfo.end();

            for (; iter != iter_end; ++iter)
            {
                if (iter->rid == friendroleid)
                {
                    iter->remarks = friendremarks;
                    pinfo->friend_ver++;
                    break;
                }
            }

            if (iter == iter_end)
            {
                GFriendExtInfo info;
                info.rid = friendroleid;
                info.remarks = friendremarks;

                PlayerInfo* pfriend = UserContainer::GetInstance().FindRole(friendroleid);
                if (pfriend != NULL)
                {
                    info.uid = pfriend->userid;
                    info.rid = pfriend->roleid;
                    info.level = pfriend->level;
                    info.last_logintime = pfriend->user->last_login_time;
                    info.update_time = time(NULL);
                    info.reincarnation_times = pfriend->reincarnation_times;
                }

                friendextinfo.push_back(info);
                pinfo->friend_ver++;
            }
        }
        else
            ret = ERR_FRIEND_REMARKS_LEN;

        AddFriendRemarks_Re re;
        re.retcode = ret;
        re.roleid = roleid;
        re.friendroleid = friendroleid;
        re.friendremarks = friendremarks;
        re.srclsid = srclsid;

        dsm->Send(pinfo->linksid, re);
    }
};

};

#endif
