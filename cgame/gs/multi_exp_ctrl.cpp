#include "world.h"
#include "worldmanager.h"
#include "player_imp.h"
#include "multi_exp_ctrl.h"

void multi_exp_ctrl::Update(gactive_imp* imp, int cur_time)
{
	if(cur_time/86400 != refresh_timestamp/86400)
	{
		activate_times = 0;
		refresh_timestamp = cur_time;
		NotifyClientState(imp);
	}
	if(state == STATE_NORMAL || cur_time < timeout) return;
	switch(state)
	{
	case STATE_ENHANCE:
		enhance_factor = 0.f;
		enhance_time = 0;
		if(buffer_time > 0)
		{
			state = STATE_BUFFER;
			timeout = cur_time + buffer_time;
		}
		else if(impair_time > 0)
		{
			state = STATE_IMPAIR;
			timeout = cur_time + impair_time;
		}
		else
		{
			state = STATE_NORMAL;
			timeout = 0;
		}
	break;
	case STATE_BUFFER:
		buffer_time = 0;
		if(impair_time > 0)
		{
			state = STATE_IMPAIR;
			timeout = cur_time + impair_time;
		}
		else
		{
			state = STATE_NORMAL;
			timeout = 0;
		}
	break;
	case STATE_IMPAIR:
		impair_time = 0;
		state = STATE_NORMAL;
		timeout = 0;
	break;
	
	default:
	break;
	}
	NotifyClientState(imp);
}

bool multi_exp_ctrl::ActivateMultiExp(gactive_imp* imp)
{
	if(state == STATE_ENHANCE) return false;
	if(enhance_time <= 0 || enhance_factor <= 0.f) return false;
	if(activate_times >= MAX_DEACTIVATE_TIMES_PER_DAY) return false;
	activate_times ++;
	
	int cur_time = g_timer.get_systime();
	StateReset(cur_time);
	
	state = STATE_ENHANCE;
	timeout = cur_time + enhance_time;
	
	NotifyClientState(imp);
	return true;
};

bool multi_exp_ctrl::DeactivateMultiExp(gactive_imp* imp)
{
	if(state != STATE_ENHANCE) return false;
	int cur_time = g_timer.get_systime();
	if(timeout - cur_time <= 0) return false;
	
	enhance_time = timeout - cur_time;
	StateInit(cur_time);
	
	NotifyClientState(imp);
	return true;
};

bool multi_exp_ctrl::IncMultiExpTime(gactive_imp* imp, float e_factor, int e_time, int b_time, int i_time)
{
	int cur_time = g_timer.get_systime();
	if(cur_time/86400 <= last_timestamp/86400) return false;
	if(enhance_time > 0) return false;
	
	StateReset(cur_time);
	
	last_timestamp = cur_time;
	enhance_factor = e_factor;
	enhance_time = e_time;
	buffer_time += b_time;
	if(buffer_time > 256*3600) buffer_time = 256*3600;
	impair_time += i_time;
	if(impair_time > 1024*3600) impair_time = 1024*3600;
	activate_times = 0;
	
	StateInit(cur_time);

	NotifyClientInfo(imp);
	NotifyClientState(imp);
	return true;
}

bool multi_exp_ctrl::DBLoadData(archive& ar)
{
	int cur_time = g_timer.get_systime();
	int impair_timeout;
	try{
		ar >> last_timestamp >> enhance_factor >> enhance_time >> buffer_time >> impair_timeout;
		if(!ar.is_eof()) ar >> activate_times;
		if(!ar.is_eof()) ar >> refresh_timestamp;
		if(impair_timeout > cur_time)
			impair_time = impair_timeout - cur_time;
		else 
			impair_time = 0;
		if(cur_time/86400 != refresh_timestamp/86400)
		{
			activate_times = 0;
			refresh_timestamp = cur_time;
		}
	}
	catch(...)
	{
		GLog::log(GLOG_ERR,"玩家多倍经验数据装载出错");
		return false;	
	}
	ASSERT(ar.is_eof());
	StateInit(cur_time);		
	return true;
}

bool multi_exp_ctrl::DBSaveData(archive& ar)
{
	float e_factor; 
	int e_time; 
	int b_time;
	int i_timeout;
	int cur_time = g_timer.get_systime();
	if(state == STATE_ENHANCE)
	{
		e_factor = enhance_factor;
		e_time = timeout - cur_time;
		if(e_time <= 0)
		{
			e_factor = 0.f;
			e_time = 0;
		}
	}
	else
	{
		e_factor = enhance_factor;
		e_time = enhance_time;
	}

	if(state == STATE_BUFFER)
	{
		b_time = timeout - cur_time;
		if(b_time < 0) b_time = 0;
	}
	else
		b_time = buffer_time;	
		
	if(state == STATE_IMPAIR)
		i_timeout = timeout;
	else
		i_timeout = cur_time + impair_time;	

	ar << last_timestamp << e_factor << e_time << b_time << i_timeout << activate_times << refresh_timestamp;
	return true;
}
//切换服务器
bool multi_exp_ctrl::Save(archive& ar)
{
	ar << state << timeout << last_timestamp << enhance_factor << enhance_time << buffer_time << impair_time << activate_times << refresh_timestamp;
	return true;
}

bool multi_exp_ctrl::Load(archive& ar)
{
	ar >> state >> timeout >> last_timestamp >> enhance_factor >> enhance_time >> buffer_time >> impair_time >> activate_times >> refresh_timestamp;
	return true;	
}

void multi_exp_ctrl::NotifyClientInfo(gactive_imp* imp)
{
	imp->_runner->multi_exp_info(last_timestamp,enhance_factor);
}

void multi_exp_ctrl::NotifyClientState(gactive_imp* imp)
{
	int cur_time = g_timer.get_systime();
	imp->_runner->change_multi_exp_state(state,
						state == STATE_ENHANCE ? timeout - cur_time : enhance_time,
						state == STATE_BUFFER  ? timeout - cur_time : buffer_time,
						state == STATE_IMPAIR  ? timeout - cur_time : impair_time,
						MAX_DEACTIVATE_TIMES_PER_DAY-activate_times);
}

void multi_exp_ctrl::StateInit(int cur_time)
{
	if(buffer_time > 0)
	{
		state = STATE_BUFFER;	
		timeout = cur_time + buffer_time;
	}
	else if(impair_time > 0)
	{
		state = STATE_IMPAIR;
		timeout = cur_time + impair_time;
	}
	else
	{
		state = STATE_NORMAL;
		timeout = 0;
	}
}

void multi_exp_ctrl::StateReset(int cur_time)
{
	switch(state)
	{
	case STATE_NORMAL:
	break;
	case STATE_ENHANCE:
		enhance_time = timeout - cur_time;
		if(enhance_time <= 0)
		{
			enhance_time = 0;
			enhance_factor = 0.f;
		}
	break;
	case STATE_BUFFER:
		buffer_time = timeout - cur_time;
		if(buffer_time < 0) buffer_time = 0;
	break;
	case STATE_IMPAIR:
		impair_time = timeout - cur_time;
		if(impair_time < 0) impair_time = 0;
	break;
	default:
	break;
	}
	state = STATE_NORMAL;
	timeout = 0;
}
