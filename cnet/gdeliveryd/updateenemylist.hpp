
#ifndef __GNET_UPDATEENEMYLIST_HPP
#define __GNET_UPDATEENEMYLIST_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "mapuser.h"
#include "gdeliveryserver.hpp"
#include "updateenemylist_re.hpp"


#define ENEMY_LIST_MAX_SIZE 100


enum EnemyListOpType
{
    ENEMYLIST_INSERT,
    ENEMYLIST_DELETE,
    ENEMYLIST_LOCK,
    ENEMYLIST_UNLOCK,
};

enum EnemyListErrorCode
{
    ENEMYLIST_SUCCESS,
    ENEMYLIST_ENEMY_IN_FRIENDS,
    ENEMYLIST_ENEMY_OFFLINE,
    ENEMYLIST_LIST_FULL,

    ENEMYLIST_OTHER,
};


namespace GNET
{

class UpdateEnemyList : public GNET::Protocol
{
	#include "updateenemylist"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
        if ((srcroleid < 0) || (dstroleid < 0) || (srcroleid == dstroleid)) return;
        if (GDeliveryServer::GetInstance()->IsCentralDS()) return;

        Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
        PlayerInfo* pinfo = UserContainer::GetInstance().FindRole(srcroleid);
        if (pinfo == NULL) return;
        if (pinfo->friend_ver < 0) return;

        int ret = ENEMYLIST_OTHER;
        time_t cur_time = Timer::GetTime();

        GEnemyList enemy;
        enemy.rid = dstroleid;
        enemy.add_time = cur_time;

        UpdateEnemyList_Re re;
        re.retcode = ret;
        re.optype = optype;
        re.srcroleid = srcroleid;
        re.enemy = enemy;
        re.localsid = pinfo->localsid;

        const GFriendInfoVector& friendlist = pinfo->friends;
        GFriendInfoVector::const_iterator it = friendlist.begin(), it_end = friendlist.end();

        for (; it != it_end; ++it)
        {
            if (it->rid == dstroleid)
            {
                Log::log(LOG_INFO, "UpdateEnemyList, enemy in friendlist. srcroleid=%d, dstroleid=%d.",
                    srcroleid, dstroleid);

                re.retcode = ENEMYLIST_ENEMY_IN_FRIENDS;
                GDeliveryServer::GetInstance()->Send(pinfo->linksid, re);
                return;
            }
        }

        GEnemyListVector& enemylist = pinfo->enemylistinfo;
        GEnemyListVector::iterator iter = enemylist.begin(), iter_end = enemylist.end();

        int add_time_min = INT_MAX;
        GEnemyListVector::iterator iter_min = iter_end;

        for (; iter != iter_end; ++iter)
        {
            if (iter->rid == dstroleid) break;
            if ((iter->type == 0) && (iter->add_time < add_time_min))
            {
                add_time_min = iter->add_time;
                iter_min = iter;
            }
        }

        Log::log(LOG_INFO, "UpdateEnemyList, optype=%d, srcroleid=%d, dstroleid=%d, update_time=%d, enemylist_size=%d, is_in_enemylist=%d.",
            optype, srcroleid, dstroleid, cur_time, enemylist.size(), (iter != iter_end));

        switch (optype)
        {
            case ENEMYLIST_INSERT:
            {
                PlayerInfo* penemy = UserContainer::GetInstance().FindRole(dstroleid);
                if (penemy != NULL)
                {
                    enemy.cls = penemy->cls;
                    enemy.type = 0;
                    enemy.name = penemy->name;

                    if (iter != iter_end)
                    {
                        re.enemy = enemy;
                        iter->add_time = cur_time;
                        ret = ENEMYLIST_SUCCESS;
                    }
                    else
                    {
                        if (enemylist.size() < ENEMY_LIST_MAX_SIZE)
                        {
                            re.enemy = enemy;
                            enemylist.push_back(enemy);
                            ret = ENEMYLIST_SUCCESS;
                        }
                        else if (iter_min != iter_end)
                        {
                            re.enemy = enemy;
                            enemylist.erase(iter_min);
                            enemylist.push_back(enemy);
                            ret = ENEMYLIST_SUCCESS;
                        }
                        else
                        {
                            ret = ENEMYLIST_LIST_FULL;
                        }
                    }
                }
                else
                {
                    ret = ENEMYLIST_ENEMY_OFFLINE;
                }
                Log::log(LOG_INFO, "UpdateEnemyList, insert enemylist. ret=%d.", ret);
            }
            break;

            case ENEMYLIST_DELETE:
            {
                if (iter != iter_end)
                {
                    enemylist.erase(iter);
                    ret = ENEMYLIST_SUCCESS;

                    GFriendExtInfoVector& friend_ext = pinfo->friendextinfo;
                    GFriendExtInfoVector::iterator it = friend_ext.begin(), it_end = friend_ext.end();

                    for (; it != it_end; ++it)
                    {
                        if (it->rid == dstroleid)
                        {
                            friend_ext.erase(it);
                            break;
                        }
                    }
                }
                Log::log(LOG_INFO, "UpdateEnemyList, delete enemylist. ret=%d.", ret);
            }
            break;

            case ENEMYLIST_LOCK:
            {
                if (iter != iter_end)
                {
                    iter->type = 1;
                    ret = ENEMYLIST_SUCCESS;
                }
                Log::log(LOG_INFO, "UpdateEnemyList, lock enemylist. ret=%d.", ret);
            }
            break;

            case ENEMYLIST_UNLOCK:
            {
                if (iter != iter_end)
                {
                    iter->type = 0;
                    iter->add_time = cur_time;
                    ret = ENEMYLIST_SUCCESS;
                }
                Log::log(LOG_INFO, "UpdateEnemyList, unlock enemylist. ret=%d.", ret);
            }
            break;

            default:
            break;
        }

        if (ret == ENEMYLIST_SUCCESS)
            ++pinfo->friend_ver;

        re.retcode = ret;
        GDeliveryServer::GetInstance()->Send(pinfo->linksid, re);
	}
};

};

#endif
