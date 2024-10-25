
#ifndef __GNET_GETREMOTEROLEINFO_RE_HPP
#define __GNET_GETREMOTEROLEINFO_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "groleinfo"

namespace GNET
{

class GetRemoteRoleInfo_Re : public GNET::Protocol
{
	#include "getremoteroleinfo_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		//由于原服和跨服的roleid和remote_roleid正好相反，所以收到该协议时，要交换二者的位置
		int tmp = remote_roleid;
		remote_roleid = roleid;
		roleid = tmp;

		LOG_TRACE("Recv GetRemoteRoleInfo_Re retcode %d userid %d roleid %d remote_roleid %d", retcode, userid, roleid, remote_roleid);
		
		if (retcode != ERR_SUCCESS) {
			Log::log(LOG_ERR, "GetRemoteRoleInfo_Re errno %d userid %d roleid %d remote_roleid %d", retcode, userid, roleid, remote_roleid);
		} else {
			Thread::RWLock::WRScoped l(UserContainer::GetInstance().GetLocker());
			UserInfo* user = UserContainer::GetInstance().FindUser(userid);
			if(NULL == user) return;	
			
			GRoleInfo& role = value;
			if(role.worldtag == 0) role.worldtag = 1;
			role.status = _ROLE_STATUS_CROSS_LOCKED;
			
			user->role_status[roleid % MAX_ROLE_COUNT] = role.status;

			RoleInfo info(
					roleid, role.gender, role.race,
					role.cls, role.level, role.level2, role.name,
					role.custom_data,
					role.equipment,
					role.status,
					role.delete_time, role.create_time, role.lastlogin_time,
					role.posx, role.posy, role.posz,
					role.worldtag,
					role.custom_status,
					role.charactermode,
					role.referrer_role,
					role.cash_add,
					role.reincarnation_data,
					role.realm_data);
			
			/*user->role_pos[role.id % MAX_ROLE_COUNT] = UserInfo::point_t(role.posx,role.posy,role.posz);
			user->worldtag[role.id % MAX_ROLE_COUNT] = role.worldtag;
			user->create_time[role.id % MAX_ROLE_COUNT] = role.create_time;
			user->role_status[role.id % MAX_ROLE_COUNT] = role.status;

			if(role.lastlogin_time>user->lastlogin) {//获得帐号下所有角色最后登录时间
				user->lastlogin = role.lastlogin_time;
				LOG_TRACE("getroleinfo user %d lastlogintime %d", userid, user->lastlogin);
			}*/

			DelayRolelistTask::InsertRoleInfo(info);
		}
		
		DelayRolelistTask::OnRecvInfo(userid, roleid);
	}
};

};

#endif
