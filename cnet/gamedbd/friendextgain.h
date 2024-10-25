#ifndef __GNET_FRIENDEXTGAIN_H
#define __GNET_FRIENDEXTGAIN_H

#include "log.h"
#include "dbfriendextlist_re.hpp"
#include "thread.h"
#include <iostream>
#include "greincarnationdata"

namespace GNET
{
	typedef struct
	{
		int roleid;
		int friend_roleid;
	}FriendExt_Need_Gain;

	class FriendExtGainManager
	{
		private:
			std::vector<FriendExt_Need_Gain> FriendExtGain_vec;
			Thread::Mutex f_lock;
		public:
			~FriendExtGainManager() {}
			static FriendExtGainManager* GetInstance()
			{
				static FriendExtGainManager instance;
				return &instance;
			}
		public:
			void InsertFriendExtGain2Vec(const int rid,const std::vector<int> roleidlist)
			{
				Thread::Mutex::Scoped   lock(f_lock);
				FriendExt_Need_Gain ws;
				for(size_t i=0;i<roleidlist.size();i++)
				{
					ws.roleid = rid;
					ws.friend_roleid = roleidlist[i];
					FriendExtGain_vec.push_back(ws);		
				}
				ws.roleid = rid;
				ws.friend_roleid = -1;
				FriendExtGain_vec.push_back(ws);
				return;
			}

			void GetFriendExtGainVec(std::vector<FriendExt_Need_Gain> & fngv)
			{
				Thread::Mutex::Scoped   lock(f_lock);
				if(FriendExtGain_vec.empty())
					return;
				fngv.reserve(FriendExtGain_vec.size());
				fngv.swap(FriendExtGain_vec);
				return;
			}

			bool IsFriendExtGainVecEmpty()
			{
				return FriendExtGain_vec.empty();
			}

			static void * UpdateLoginTime(void *tmp)
			{
				pthread_detach( pthread_self() );
				sigset_t sigs;
				sigfillset(&sigs);
				pthread_sigmask(SIG_BLOCK, &sigs, NULL);

				std::vector<FriendExt_Need_Gain> tmpList;

				while(1)
				{
					while(FriendExtGainManager::GetInstance()->IsFriendExtGainVecEmpty())
					{
						usleep(10000);
					}
					tmpList.clear();
					FriendExtGainManager::GetInstance()->GetFriendExtGainVec(tmpList);

					DBFriendExtList_Re pro;
					for(size_t i=0;i<tmpList.size();i++)
					{
						usleep(2000);
						if(tmpList[i].friend_roleid == -1)
						{
							pro.retcode = 0;
							pro.rid = tmpList[i].roleid;
							DEBUG_PRINT("FriendExt role %d Send FriendExtlist to Gdeliveryd,num = %d",pro.rid,i);
							GameDBServer::GetInstance()->Send2Delivery(pro);
							pro.friendext.clear();
						}
						else
						{
							try
							{
								StorageEnv::Storage * pbase = StorageEnv::GetStorage("base");
								StorageEnv::Storage * pstatus = StorageEnv::GetStorage("status");
								StorageEnv::AtomTransaction txn;
								Marshal::OctetsStream key;
								GRoleBase base;
								GRoleStatus status;
								GFriendExtInfo info;
								try
								{
									key << tmpList[i].friend_roleid;
									Marshal::OctetsStream(pbase->find(key,txn)) >> base;
									Marshal::OctetsStream(pstatus->find(key,txn)) >> status;

									if(base.userid)
										info.uid = base.userid;
									else
										info.uid = LOGICUID(tmpList[i].friend_roleid);
							
									info.rid 			= tmpList[i].friend_roleid;
									info.last_logintime = base.lastlogin_time;
									info.level 			= status.level;
									
									GReincarnationData tmpdata;
									if(status.reincarnation_data.size() > 0) Marshal::OctetsStream(status.reincarnation_data) >> tmpdata;
									info.reincarnation_times = tmpdata.records.size();
									pro.friendext.push_back(info);
								}
								catch ( DbException e ) { throw; }
								catch ( ... )
								{
									DbException e( DB_OLD_VERSION );
									txn.abort(e);
									throw e;
								}
							}
							catch ( DbException e )
							{
								Log::log( LOG_ERR,"FriendExtGainManager::UpdateLoginTime,roleid = %d,friendid=%d,what =%s\n",tmpList[i].roleid,tmpList[i].friend_roleid,e.what());
								if(e.get_errno() != DB_LOCK_DEADLOCK)
								{
									GFriendExtInfo info;
									info.uid = 0;
									info.rid = tmpList[i].friend_roleid;
									info.last_logintime = 0;
									info.level = 1;
									info.reincarnation_times = 0;
									pro.friendext.push_back(info);
								}
							}
						}
					}
				}
				return NULL;
			}
	};

}

#endif
