
#ifndef __GNET_DBMNFACTIONCACHEUPDATE_HPP
#define __GNET_DBMNFACTIONCACHEUPDATE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gamedbmanager.h"
#include "gamedbserver.hpp"
#include "mnfactioninfoupdate.hpp"

#include "mnfactioninfo"


namespace GNET
{

class DBMNFactionCacheUpdate : public GNET::Protocol
{
	#include "dbmnfactioncacheupdate"

    void Process(Manager *manager, Manager::Session::ID sid)
    {
        try
        {
            StorageEnv::Storage* pmnfaction = StorageEnv::GetStorage("mnfactioninfo");
            StorageEnv::Storage* pfaction = StorageEnv::GetStorage("factioninfo");
            StorageEnv::Storage* puser = StorageEnv::GetStorage("userfaction");
            StorageEnv::AtomTransaction txn;

            try
            {
                MNFactionInfoUpdate proto;
                MNFactionInfoVector& update_list = proto.factioninfo_list;
                proto.zoneid = GameDBManager::Zoneid();

                MNFactionInfoVector::const_iterator iter = factioninfo_list.begin(), iter_end = factioninfo_list.end();
                for (; iter != iter_end; ++iter)
                {
                    Marshal::OctetsStream mnfid_key, mninfo_value;
                    mnfid_key << (iter->unifid);

                    if (pmnfaction->find(mnfid_key, mninfo_value, txn))
                    {
                        MNFactionInfo mninfo;
                        mninfo_value >> mninfo;
                        if (mninfo.version != iter->version)
                        {
                            Octets faction_name, master_name;
                            faction_name = iter->faction_name;
                            master_name = iter->master_name;

                            Marshal::OctetsStream fid_key, info_value;
                            fid_key << (iter->fid);
                            if (pfaction->find(fid_key, info_value, txn))
                            {
                                GFactionInfo info;
                                info_value >> info;
                                faction_name = info.name;

                                Marshal::OctetsStream user_key, user_value;
                                user_key << (info.master.rid);
                                if (puser->find(user_key, user_value, txn))
                                {
                                    GUserFaction user;
                                    user_value >> user;
                                    master_name = user.name;
                                }
                            }

                            mninfo.fid 				= iter->fid;
                            mninfo.faction_name 	= faction_name;
                            mninfo.master_name		= master_name;
                            mninfo.zoneid			= GameDBManager::Zoneid();
							mninfo.domain_num		= iter->domain_num;
							mninfo.credit			= iter->credit;
							mninfo.credit_this_week = iter->credit_this_week;
							mninfo.credit_get_time	= iter->credit_get_time;
							mninfo.invite_count 	= iter->invite_count;
							mninfo.accept_sn		= iter->accept_sn;
							mninfo.bonus_sn			= iter->bonus_sn;
                            update_list.push_back(mninfo);
                        }

                        mninfo.domain_num		= iter->domain_num;
                        mninfo.credit			= iter->credit;
						mninfo.credit_this_week = iter->credit_this_week;
						mninfo.credit_get_time	= iter->credit_get_time;
                        mninfo.invite_count		= iter->invite_count;
                        mninfo.accept_sn		= iter->accept_sn;
                        mninfo.bonus_sn			= iter->bonus_sn;
                        pmnfaction->insert(mnfid_key, (Marshal::OctetsStream() << mninfo), txn);
                    }
                }

                GameDBServer::GetInstance()->Send2Delivery(proto);
            }
            catch (DbException e) {throw;}
            catch (...)
            {
                DbException e(DB_OLD_VERSION);
                txn.abort(e);
                throw e;
            }
        }
        catch (DbException e)
        {
            Log::log(LOG_ERR, "DBMNFactionCacheUpdate, what=%s.\n", e.what());
        }
    }
};

};

#endif
