#include "string.h"
#include "world.h"
#include "common/message.h"
#include "worldmanager.h"
#include "template/itemdataman.h"
#include <glog.h>

class HistoryADVersionModify : public UniqueDataClient::ModifyOperater 
{
	friend bool history_manager::Initialize(itemdataman & dataman);
	HistoryADVersionModify() {}
public:
	bool  OnInit(int key,const UDOctets& val)
	{
		return world_manager::GetHistoryMan().OnSetVersion(val);	
	}
	
	bool  OnModify(int key, const UDOctets& val, int retcode, bool localflag)
	{
		return world_manager::GetHistoryMan().OnAdvance(val, localflag, retcode);
	}
	
	bool  CheckModify(int key, const UDOctets& val, bool setflag)  		
	{ 
		return setflag && (int)val <= world_manager::GetHistoryMan().GetStageLimit(); // 只能设置
	}
};

class HistoryStageValModify : public UniqueDataClient::ModifyOperater 
{
	friend bool history_manager::Initialize(itemdataman & dataman);
	HistoryStageValModify(int id,int goal) : _stageid(id),_stagegoal(goal) {}
public:
	bool  OnInit(int key, const UDOctets& val)
	{
		return world_manager::GetHistoryMan().OnSetValue(val, _stageid);
	}
	
	bool  OnModify(int key, const UDOctets& val, int retcode, bool localflag)
	{
		return world_manager::GetHistoryMan().OnStep(val, _stageid, (int)val >= _stagegoal, retcode, localflag);
	}
	
	bool  CheckModify(int key, const UDOctets& val, bool setflag)  		
	{ 
		return !setflag;  // 只能修改
	}
private:	
	int _stageid;
	int _stagegoal;
};

bool history_manager::Initialize(itemdataman & dataman)
{
	DATA_TYPE dt;
	HISTORY_ADVANCE_CONFIG * config = (HISTORY_ADVANCE_CONFIG*) dataman.get_data_ptr(HISTORY_ADVANCE_CONFIG_ID, ID_SPACE_CONFIG, dt);
	if(config == NULL || dt != DT_HISTORY_ADVANCE_CONFIG)
	{
		return false;
	}

	world_manager::GetUniqueDataMan().Register(UDI_HISTORY_VERSION, 0, new HistoryADVersionModify, true);

	int stage_count = sizeof(config->history_stage_id)/sizeof(int);

	for(; _stagelimit < stage_count; ++_stagelimit)
	{
		if(!config->history_stage_id[_stagelimit])
			break;

		HISTORY_STAGE_CONFIG * sconf = (HISTORY_STAGE_CONFIG*) dataman.get_data_ptr(config->history_stage_id[_stagelimit], ID_SPACE_CONFIG, dt);

		if(sconf == NULL || dt != DT_HISTORY_STAGE_CONFIG)
		{
			return false;
		}

		world_manager::GetUniqueDataMan().Register(sconf->progress_value_id, 0, new HistoryStageValModify(_stagelimit,sconf->progress_value_goal));		
	}

	_initialized = true;
	return true;
}

bool history_manager::OnSetVersion(int version)
{
	if(version > _stagelimit)
	{
		GLog::log(GLOG_ERR,"history:stage%d over stagelimit%d \n",version, _stagelimit);
		return false;
	}

	_stageversion = version;
	
	return true;
}

bool history_manager::OnAdvance(int version, bool localflag, int retcode)
{
	if(localflag && retcode)
	{
		GLog::log(GLOG_ERR,"history:stage%d advance fail [retcode:%d] \n",
				_stageversion, retcode);
	}
	else
	{
		GLog::log(GLOG_INFO,"history:stage%d advance [retcode:%d] \n",
				_stageversion, retcode);
	}

	if(version > _stagelimit)
	{
		GLog::log(GLOG_ERR,"history:stage%d over stagelimit%d [retcode:%d] \n",
				version, _stagelimit, retcode);
		return false;
	}

	_stageversion = version;

	return true;
}

bool history_manager::OnSetValue(int value, int stageid)
{
	if(stageid != _stageversion)
		return false;

	_stagevalue = value;

	return true;
}
	
bool history_manager::OnStep(int value,int stageid,bool advance,int retcode ,bool localflag)
{
	if(stageid != _stageversion)
	{
		//log 策划逻辑上允许新阶段下修改老阶段的值
	//	GLog::log(GLOG_ERR,"history:old stage%d still modify val%d at new stage%d [adv:%dret:%d] \n",
	//			stageid, value, _stageversion, advance ? 1 : 0, retcode);
		return false;
	}
	
	_stagevalue = value;

	if(0 == retcode)
	{
		if(localflag && advance && _stageversion < _stagelimit) // 本服修改成功才负责推进历史
		{
			world_manager::GetUniqueDataMan().ModifyDataInCallback(UDI_HISTORY_VERSION,++_stageversion,true);	
		}
	}
	else
	{
		GLog::log(GLOG_ERR,"history:stage%d modify val%d fail [adv:%dret:%d] \n",
				stageid, value, advance ? 1 : 0, retcode);
	}
	
	return true;
}

