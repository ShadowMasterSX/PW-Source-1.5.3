#include "world.h"
#include "worldmanager.h"
#include "player_imp.h"
#include "online_award_filter.h"
#include "online_award.h"

void online_award::Update(gplayer_imp* imp, int cur_time)
{
	const static int vip_award[7] = {0,0,7200,14400,21600,32400,43200};
	if(cur_time - _refresh_timestamp >= 86400)
	{
		int vip = imp->GetCashVipLevel(); 
		//每日中午12点刷新
		struct tm * tm1 = localtime((const time_t *)&cur_time);
		_refresh_timestamp = cur_time - (tm1->tm_hour>=12 ? tm1->tm_hour-12 : tm1->tm_hour+12)*3600 - tm1->tm_min*60 - tm1->tm_sec;
		DATA_TYPE dt;
		ONLINE_AWARDS_CONFIG * ess = (ONLINE_AWARDS_CONFIG *)world_manager::GetDataMan().get_data_ptr(ONLINE_AWARD_CONFIG_ID, ID_SPACE_CONFIG, dt);
		if(ess && dt == DT_ONLINE_AWARDS_CONFIG)
			_total_award_time = ess->max_time + ((vip >= 0 && vip <= 6) ? vip_award[vip] : 0);
		NotifyClientData(imp, -1);
	}
}

bool online_award::ActivateAward(gplayer_imp* imp, int type)
{
	if(type < 0 || type >= MAX_AWARD_TYPE) return false;
	if(imp->InCentralServer() || (!world_manager::GetWorldLimit().online_award && !imp->CheckVipService(CVS_ONLINEAWARD)))
	{
		imp->_runner->error_message(S2C::ERR_OPERTION_DENYED_IN_CUR_SCENE);
		return false;
	}
	if(imp->_basic.level < MIN_LEVEL_REQUIRED) return false;

	if(_total_award_time <= 0)
	{
		imp->_runner->error_message(S2C::ERR_NOT_ENOUGH_AWARD_TIME);
		return false;
	}
	award_data & data = _award_list[type];		
	if(data.type == -1 || data.time <= 0)
	{
		imp->_runner->error_message(S2C::ERR_NOT_ENOUGH_AWARD_TIME);
		return false;
	}
	
	DATA_TYPE dt;
	ONLINE_AWARDS_CONFIG * ess = (ONLINE_AWARDS_CONFIG *)world_manager::GetDataMan().get_data_ptr(ONLINE_AWARD_CONFIG_ID, ID_SPACE_CONFIG, dt);
	if(!ess || dt != DT_ONLINE_AWARDS_CONFIG) return false;
	
	switch(type)
	{
		case AWARD_TYPE_EXP1:
		case AWARD_TYPE_EXP2:
		case AWARD_TYPE_EXP3:
		case AWARD_TYPE_EXP4:
		{
			int interval = ess->choice[type].interval;
			int exp = ess->choice[type].exp[imp->_basic.level - 1];
			if(interval <= 0 || exp <= 0) return false;
			imp->_filters.AddFilter(new online_award_exp_filter(imp, type, interval, exp));
		}
			break;
		default:
			return false;
	}
	return true;
}

bool online_award::DeactivateAward(gplayer_imp* imp, int type)
{
	if(type < 0 || type >= MAX_AWARD_TYPE) return false;

	switch(type)
	{
		case AWARD_TYPE_EXP1:
		case AWARD_TYPE_EXP2:
		case AWARD_TYPE_EXP3:
		case AWARD_TYPE_EXP4:
			imp->_filters.RemoveFilter(FILTER_INDEX_ONLINE_AWARD_EXP);
			break;
		default:
			return false;
	}
	return true;
}

bool online_award::IncAwardTime(gplayer_imp* imp, int type, int time)
{
	if(type < 0 || type >= MAX_AWARD_TYPE) return false;

	award_data & data = _award_list[type];
	int newtime = data.time + time;
	if(newtime <= data.time) return false;
	if(newtime > MAX_AWARD_TIME) return false;
	data.type = type;
	data.time = newtime;
	NotifyClientData(imp, type);
	return true;
}

int online_award::SpendAwardTime(gplayer_imp* imp, int type, int time)
{
	if(type < 0 || type >= MAX_AWARD_TYPE) return -1;
	if(imp->InCentralServer() || (!world_manager::GetWorldLimit().online_award && !imp->CheckVipService(CVS_ONLINEAWARD))) return -2;

	if(_total_award_time <= time)
		_total_award_time = 0;
	else
		_total_award_time -= time;
	award_data & data = _award_list[type];
	if(data.time <= time)
		data.time = 0;
	else
		data.time -= time;
	NotifyClientData(imp, type);
	return _total_award_time < data.time ? _total_award_time : data.time;
}

void online_award::NotifyClientAllData(gplayer_imp* imp)
{
	imp->_runner->online_award_data(_total_award_time, _award_list.size(), _award_list.begin(), _award_list.size()*sizeof(award_data));
}

void online_award::NotifyClientData(gplayer_imp* imp, int type)
{
	if(type >= 0 && type < MAX_AWARD_TYPE)
		imp->_runner->online_award_data(_total_award_time, 1, &_award_list[type], sizeof(award_data));
	else
		imp->_runner->online_award_data(_total_award_time, 0, NULL, 0);
}

bool online_award::DBLoadData(archive& ar)
{
	try
	{
		int ver;
		ar >> ver;
		if(ver == CUR_VERSION)
		{
			ar >> _refresh_timestamp >> _total_award_time;
			int size = 0;
			ar >> size;
			for(int i=0; i<size; i++)
			{
				int type, time, reserved;
				ar >> type >> time >> reserved;
				if(type >= 0 && type < MAX_AWARD_TYPE)
				{
					award_data & data = _award_list[type];
					data.type = type;
					data.time = time;
					data.reserved = reserved;
				}
			}
		}
		else
		{
			//log
			return false;
		}

		int cur_time = g_timer.get_systime();
		if(cur_time - _refresh_timestamp >= 86400)
		{
			struct tm * tm1 = localtime((const time_t *)&cur_time);
			_refresh_timestamp = cur_time - (tm1->tm_hour>=12 ? tm1->tm_hour-12 : tm1->tm_hour+12)*3600 - tm1->tm_min*60 - tm1->tm_sec;
			DATA_TYPE dt;
			ONLINE_AWARDS_CONFIG * ess = (ONLINE_AWARDS_CONFIG *)world_manager::GetDataMan().get_data_ptr(ONLINE_AWARD_CONFIG_ID, ID_SPACE_CONFIG, dt);
			if(ess && dt == DT_ONLINE_AWARDS_CONFIG)
				_total_award_time = ess->max_time;
		}
	}
	catch(...)
	{
		//log
		return false;
	}
	return true;
}

bool online_award::DBSaveData(archive& ar)
{
	ar << _version << _refresh_timestamp << _total_award_time;
	int size = 0;	
	for(size_t i=0; i<_award_list.size(); i++)
	{
		if(_award_list[i].type != -1)
			++size;
	}
	ar << size;
	if(size)
	{
		for(size_t i=0; i<_award_list.size(); i++)
		{
			if(_award_list[i].type != -1)
			{
				award_data & data = _award_list[i];
				ar << data.type << data.time << data.reserved;
				--size;
			}
		}
		ASSERT(size == 0);
	}
	return true;
}

bool online_award::Save(archive& ar)
{
	ar << _version << _refresh_timestamp << _total_award_time;
	ar << _award_list.size();
	ar.push_back(_award_list.begin(), _award_list.size() * sizeof(award_data));
	return true;
}

bool online_award::Load(archive& ar)
{
	ar >> _version >> _refresh_timestamp >> _total_award_time;
	size_t size = 0;
	ar >> size;
	ASSERT(size <= _award_list.size());
	ar.pop_back(_award_list.begin(), size * sizeof(award_data));
	return true;
}

void online_award::Swap(online_award & rhs)
{
	abase::swap(_version, rhs._version);
	abase::swap(_refresh_timestamp, rhs._refresh_timestamp);
	abase::swap(_total_award_time, rhs._total_award_time);
	_award_list.swap(rhs._award_list);
}


