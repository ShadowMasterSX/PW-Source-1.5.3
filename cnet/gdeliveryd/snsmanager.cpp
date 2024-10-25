#include "snsrolebrief"
#include "snsroleskills"
#include "snsroleequipment"
#include "snsrolepetcorral"
#include "gamedataresp.hpp"
#include "snsclient.hpp"
#include "snsmanager.h"
#include "factionext"
#include "citywar"
#include "battlemanager.h"
#include "factionfortressmanager.h"
namespace GNET
{

void SNSManager::ForwardRoleBrief(int roleid, SNSRoleBrief& brief, SNSRoleSkills& skills, SNSRoleEquipment& equipment, SNSRolePetCorral& petcorral)
{
	if( SNSClient::GetInstance()->SendProtocol(GameDataResp(SNS_DTYPE_ROLEBRIEF,roleid,0,Marshal::OctetsStream()<<brief)) )
	{
		//发送成功则缓存role data等待sns server来取
		Thread::Mutex::Scoped l(lock);
		int cur_time = Timer::GetTime(); 
		role_data_map[roleid] = SNSRoleData(cur_time,
											Marshal::OctetsStream()<<skills,
											Marshal::OctetsStream()<<equipment,
											Marshal::OctetsStream()<<petcorral);
		//
		if(cur_time >= last_clearup_time + CLEARUP_INTERVAL)
		{
			RoleDataMap::iterator it = role_data_map.begin();
			for( ; it!=role_data_map.end(); )
			{
				if(cur_time >= it->second.timestamp+CLEARUP_INTERVAL)
					role_data_map.erase(it++);
				else
					++it;
			}
			last_clearup_time = cur_time;
		}
	}
}

void SNSManager::FetchRoleData(int roleid, int dtype)
{
	Thread::Mutex::Scoped l(lock);
	GameDataResp resp(dtype,roleid,SNS_ERR_DATA_NOT_EXIST,Octets());

	RoleDataMap::iterator it = role_data_map.find(roleid);
	if(it != role_data_map.end())
	{
		if(dtype == SNS_DTYPE_ROLESKILLS)
		{
			if(it->second.skills.size())
			{
				resp.retcode = SNS_ERR_SUCCESS;
				resp.data.swap(it->second.skills);
			}
		}
		else if(dtype == SNS_DTYPE_ROLEEQUIPMENT)
		{
			if(it->second.equipment.size())
			{
				resp.retcode = SNS_ERR_SUCCESS;
				resp.data.swap(it->second.equipment);
			}
		}
		else if(dtype == SNS_DTYPE_ROLEPETCORRAL)
		{
			if(it->second.petcorral.size())
			{
				resp.retcode = SNS_ERR_SUCCESS;
				resp.data.swap(it->second.petcorral);
			}
		}
	}
	SNSClient::GetInstance()->SendProtocol(resp);
}

void SNSManager::FetchFactionExt(int factionid,int dtype)
{
	GameDataResp resp(dtype,factionid,SNS_ERR_DATA_NOT_EXIST,Octets());
	
	FactionExt fe;
	if(FactionFortressMan::GetInstance().GetFactionExt(factionid,fe))
	{
		resp.retcode = SNS_ERR_SUCCESS;
		Marshal::OctetsStream value;
		value << fe;
		resp.data =  value;
	}

	SNSClient::GetInstance()->SendProtocol(resp);
}

void SNSManager::FetchCityInfo(int roleid,int dtype)
{
	GameDataResp resp(dtype,roleid,SNS_ERR_DATA_NOT_EXIST,Octets()); 
	CityWar cw;
	if(BattleManager::GetInstance()->GetCityInfo(cw))
	{
		resp.retcode = SNS_ERR_SUCCESS;
		Marshal::OctetsStream value;
		value << cw;
		resp.data = value;
	}
	SNSClient::GetInstance()->SendProtocol(resp);
}


}
