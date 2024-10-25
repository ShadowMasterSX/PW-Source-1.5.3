
#include <gforceglobaldatabrief>
#include "dbputforce.hrp"
#include "dbforceload.hrp"
#include "syncforceglobaldata.hpp"
#include "gproviderserver.hpp"
#include "forcemanager.h"

namespace GNET
{

static void ConvertForceGlobalDataToBrief(const GForceGlobalData& data, GForceGlobalDataBrief& brief)
{
	brief.force_id 		= data.force_id;
	brief.player_count 	= data.player_count; 
	brief.development 	= data.development; 
	brief.construction 	= data.construction; 
	brief.activity 		= data.activity; 
	brief.activity_level = data.activity_level; 
}
bool ForceManager::Initialize()
{
	IntervalTimer::Attach(this,UPDATE_INTERVAL/IntervalTimer::Resolution());
	return true;
}
void ForceManager::OnDBConnect(Protocol::Manager * manager, int sid)
{
	if(_status == ST_INIT)
		manager->Send(sid, Rpc::Call(RPC_DBFORCELOAD, DBForceLoadArg()));
}
void ForceManager::OnGSConnect(Protocol::Manager * manager, int sid)
{
	if(!IsOpen()) return;
	SyncAllToGS(sid);
}
void ForceManager::OnDBLoad(const GForceGlobalDataList & list)
{
	if(_status == ST_INIT)
	{
		_force_list = list.list;
		_update_time = list.update_time;
		//根据forcelist来设置syncflag列表
		_syncgs_flag_list.insert(_syncgs_flag_list.end(), _force_list.size(), false);
		
		_status = ST_OPEN;
		SyncAllToGS();
	}
}
bool ForceManager::Update()
{
	if(!IsOpen()) return true;

	time_t now = Timer::GetTime(); 
	if(now - _update_time > 86400)
	{
		struct tm * tm1 = localtime(&now);
		_update_time = now - tm1->tm_hour*3600 - tm1->tm_min*60 - tm1->tm_sec;
		UpdateForceData();
		_dirty_flag = true;
		for(size_t i=0; i<_syncgs_flag_list.size(); i++)
			_syncgs_flag_list[i]  = true;
	}
	
	//向gs同步
	_syncgs_timer += UPDATE_INTERVAL;
	if(_syncgs_timer >= SYNCGS_CHECK_INTERVAL)
	{
		_syncgs_timer -= SYNCGS_CHECK_INTERVAL;

		bool need_sync = false;
		for(size_t i=0; i<_syncgs_flag_list.size(); i++)
		{
			if(_syncgs_flag_list[i])
			{
				need_sync = true;
				break;
			}
		}
		if(need_sync)
		{
			SyncForceGlobalData data;
			for(size_t i=0; i<_syncgs_flag_list.size(); i++)
			{
				if(_syncgs_flag_list[i])
				{
					GForceGlobalDataBrief brief;
					ConvertForceGlobalDataToBrief(_force_list[i], brief);
					data.list.push_back(brief);
					_syncgs_flag_list[i] = false;
				}
			}
			GProviderServer::GetInstance()->BroadcastProtocol(data);
		}
	}
	
	//数据写盘
	_writedb_timer += UPDATE_INTERVAL;
	if(_writedb_timer >= SYNCGS_CHECK_INTERVAL)
	{
		_writedb_timer -= SYNCGS_CHECK_INTERVAL;
		
		if(_dirty_flag && !_writeback_flag)
		{
			WriteToDB();
			_dirty_flag = false;
			_writeback_flag = true;
		}
	}
	return true;
}
void ForceManager::OnDBPutForce(int retcode)
{
	if(!IsOpen()) return;
	_writeback_flag = false;
	if(retcode != ERR_SUCCESS)
		_dirty_flag = true;
}
void ForceManager::PlayerJoinOrLeave(int force_id, bool is_join)
{
	if(!IsOpen()) return;
	int index = GetForceIndex(force_id);
	if(index < 0)
	{
		if(is_join)
		{
			GForceGlobalData data;
			data.force_id = force_id;
			_force_list.push_back(data);
			_syncgs_flag_list.push_back(false);
			index = _force_list.size() - 1;
		}
		else
		{
			Log::log(LOG_ERR, "ForceManager: PlayerJoinOrLeave. force_id(%d) not exist", force_id);
			return;
		}
	}
	GForceGlobalData & data = _force_list[index];
	data.player_count += is_join ? 1 : -1;
	_dirty_flag = true;
	_syncgs_flag_list[index] = true;
}
void ForceManager::IncreaseActivity(int force_id, int activity)
{
	if(!IsOpen()) return;
	int index = GetForceIndex(force_id);
	if(index < 0)
	{
		Log::log(LOG_ERR, "ForceManager: IncreaseActivity. force_id(%d) not exist", force_id);
		return;
	}
	GForceGlobalData & data = _force_list[index];
	data.activity += activity;
	_dirty_flag = true;
	_syncgs_flag_list[index] = true;
}
int ForceManager::GetForceIndex(int force_id)
{
	for(size_t i=0; i<_force_list.size(); i++)
		if(_force_list[i].force_id == force_id) return (int)i;
	return -1;	
}
void ForceManager::UpdateForceData()
{
	if(!_force_list.size()) return;
	int min_activity = INT_MAX, max_activity = 0;
	for(size_t i=0; i<_force_list.size(); i++)
	{
		GForceGlobalData & data = _force_list[i];
		if(data.activity > max_activity) max_activity = data.activity;
		if(data.activity < min_activity) min_activity = data.activity;
	}
	for(size_t i=0; i<_force_list.size(); i++)
	{
		GForceGlobalData & data = _force_list[i];
		if(data.activity == max_activity)
		{
			//活跃度最高
			if(data.activity_level == ACTIVITY_LEVEL_2 || data.activity_level == ACTIVITY_LEVEL_3)
				data.activity_level = ACTIVITY_LEVEL_3;
			else
				data.activity_level = ACTIVITY_LEVEL_2;
		}
		else if(data.activity == min_activity)
		{
			//活跃度最低
			data.activity_level = ACTIVITY_LEVEL_1;	
		}
		else
		{
			data.activity_level = ACTIVITY_LEVEL_0;
		}
		data.activity = 0;
	}
}
void ForceManager::WriteToDB()
{
	DBPutForce * rpc = (DBPutForce *)Rpc::Call(RPC_DBPUTFORCE, DBPutForceArg(GForceGlobalDataList(_force_list,_update_time)));
	GameDBClient::GetInstance()->SendProtocol(rpc);
}
void ForceManager::SyncAllToGS(int sid)
{
	SyncForceGlobalData data;
	for(size_t i=0; i<_force_list.size(); i++)
	{
		GForceGlobalDataBrief brief;
		ConvertForceGlobalDataToBrief(_force_list[i], brief);
		data.list.push_back(brief);
	}
	if(sid == -1)
		GProviderServer::GetInstance()->BroadcastProtocol(data);
	else
		GProviderServer::GetInstance()->Send(sid, data);
}
}
