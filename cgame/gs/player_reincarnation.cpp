
#include "player_reincarnation.h"
#include "world.h"
#include "worldmanager.h"
#include "player_imp.h"

namespace
{
	//[LEVEL_REQUIRED, LEVEL_REQUIRED+1, ...]对应的额外属性点
	static const int __extra_ppoint_table[6] = {20,5,7,10,14,20};
	
	int __extra_ppoint(int level)
	{
		int delta = level - REINCARNATION_LEVEL_REQUIRED;
		if(delta < 0) return 0;
		if((size_t)delta > sizeof(__extra_ppoint_table)/sizeof(int)-1) return 0;
		return __extra_ppoint_table[delta];
	}
	
	int __extra_ppoint_total(int level)
	{
		int point = 0;
		for(int i=REINCARNATION_LEVEL_REQUIRED; i<=level; i++)
			point += __extra_ppoint(i);
		return point;
	}

}

int player_reincarnation::MaxExtraPPoint()
{
	//一次转生增加的属性点最大值
	int point = 0;
	for(size_t i=0; i<sizeof(__extra_ppoint_table)/sizeof(int); i++)
		point += __extra_ppoint_table[i];
	return point;	
}

bool player_reincarnation::InitFromDBData(const GDB::reincarnation_data & data)
{
	_tome_exp = data.tome_exp;
	_tome_active = (data.tome_active != 0);
	for(size_t i=0; i<data.count; i++)
	{
		Record r;
		r.level = data.records[i].level;
		r.timestamp = data.records[i].timestamp;
		r.exp = data.records[i].exp;
		_records.push_back(r);
	}
	if(_records.size() > REINCARNATION_TIMES_HARD_LIMIT)
	{
		GLog::log(GLOG_ERR,"玩家(%d)转生记录数(%d)超过上限，删除多余记录",_imp->_parent->ID.id, _records.size());
		_records.erase(_records.begin()+REINCARNATION_TIMES_HARD_LIMIT, _records.end());
	}

	CalcHistoricalMaxLevel();
	CalcExpBonus();
	_imp->UpdateReincarnation(GetTimes(), false);
	return true;
}

bool player_reincarnation::MakeDBData(GDB::reincarnation_data & data)
{
	data.tome_exp  = _tome_exp;
	data.tome_active = _tome_active ? 1 : 0;
	data.count = _records.size();
	data.records = NULL;
	if(data.count == 0) return true;
	data.records = (GDB::reincarnation_record *)abase::fast_allocator::alloc(data.count * sizeof(GDB::reincarnation_record));
	for(size_t i=0; i<data.count; i++)
	{
		data.records[i].level = _records[i].level;
		data.records[i].timestamp = _records[i].timestamp;
		data.records[i].exp = _records[i].exp;
	}
	return true;
}

void player_reincarnation::ReleaseDBData(GDB::reincarnation_data & data)
{
	if(data.count && data.records)
	{
		abase::fast_allocator::free(data.records, data.count*sizeof(GDB::reincarnation_record));
		data.count = 0;
		data.records = NULL;
	}
}

bool player_reincarnation::CheckCondition()
{
	DATA_TYPE dt;
	PLAYER_REINCARNATION_CONFIG * pCfg = (PLAYER_REINCARNATION_CONFIG *) world_manager::GetDataMan().get_data_ptr(REINCARNATION_CONFIG_ID, ID_SPACE_CONFIG, dt);
	if(dt != DT_PLAYER_REINCARNATION_CONFIG || pCfg == NULL) return false;
	//转生次数限制
	if(GetTimes() >= REINCARNATION_TIMES_LIMIT) return false;
	//转生等级
	if(_imp->_basic.level < REINCARNATION_LEVEL_REQUIRED || _imp->_basic.sec_level < REINCARNATION_SEC_LEVEL_REQUIRED) return false;
	//检查物品
	if(!_imp->IsItemExist(pCfg->level[GetTimes()].require_item)) return false;
	return true;
}

void player_reincarnation::DoReincarnation()
{
	DATA_TYPE dt;
	PLAYER_REINCARNATION_CONFIG * pCfg = (PLAYER_REINCARNATION_CONFIG *) world_manager::GetDataMan().get_data_ptr(REINCARNATION_CONFIG_ID, ID_SPACE_CONFIG, dt);
	if(dt != DT_PLAYER_REINCARNATION_CONFIG || pCfg == NULL)
	{
		ASSERT(false);
		return;
	}
	int rst = _imp->TakeOutItem(pCfg->level[GetTimes()].require_item);
	ASSERT(rst >= 0);
	
	int cls = _imp->GetPlayerClass();
	int oldlevel = _imp->_basic.level;
	//洗点
	_imp->_basic.status_point += player_template::Rollback(cls,_imp->_base_prop);
	//level降至1	
	player_template::LevelRollback(cls,oldlevel,_imp->_base_prop);
	_imp->_basic.status_point -= player_template::GetStatusPointPerLevel() * (oldlevel - 1);
	_imp->_basic.level = 1;
	//获取额外属性点
	_imp->_basic.status_point += __extra_ppoint_total(oldlevel);
	//经验储存
	int remain_exp = _imp->_basic.exp;
	_imp->_basic.exp = 0;
	//增加记录
	Record r;
	r.level  = oldlevel;
	r.timestamp = g_timer.get_systime();
	r.exp = remain_exp;
	_records.push_back(r);
	//计算转生经验加成
	CalcExpBonus();
	DeactivateTome();
	
	_imp->UpdateReincarnation(GetTimes(), true);
	ClientGetTome();

	GLog::formatlog("player reincarnation:roleid=%d:level=%d:times=%d", _imp->_parent->ID.id, oldlevel, GetTimes());
}

bool player_reincarnation::RewriteTome(size_t record_index, int record_level)
{
	if(record_index >= _records.size()) return false;
	Record & record = _records[record_index];
	if(record_level != record.level) return false;

	int cls = _imp->GetPlayerClass();
	int next_exp = player_template::GetLvlupExp(cls, record.level);
	if(record.exp >= next_exp)
	{
		GLog::log(GLOG_ERR, "玩家%d转生经验%d超过升级经验%d,数据错误",_imp->_parent->ID.id, record.exp, next_exp);
		return false;
	}
	
	int need_exp = next_exp - record.exp;
	if(_tome_exp < need_exp) return false;	

	_tome_exp -= need_exp;
	record.exp = 0;
	record.level ++;
	_imp->_basic.status_point += __extra_ppoint(record.level);

	CalcHistoricalMaxLevel();
	if(IsTomeActive())
	{
		if(!CheckActivateTome()) 
			DeactivateTome();
	}
	//通知客户端
	_imp->PlayerGetProperty();
	ClientGetTome();

	GLog::formatlog("player rewrite reincarnation record:roleid=%d:level=%d", _imp->_parent->ID.id, record.level);
	return true;
}

void player_reincarnation::ClientGetTome()
{
	packet_wrapper h1(64);
	using namespace S2C;
	CMD::Make<CMD::reincarnation_tome_info>::From(h1, _tome_exp, _tome_active?1:0, (int)_records.size());
	for(size_t i=0; i<_records.size(); i++)
	{
		const Record & record = _records[i];
		CMD::Make<CMD::reincarnation_tome_info>::Add(h1, record.level, record.timestamp, record.exp);
	}
	send_ls_msg(_imp->GetParent(), h1);
}

bool player_reincarnation::TryActivateTome()
{
	if(!_tome_active)
	{
		if(!CheckActivateTome()) return false;

		_tome_active = true;
		_imp->_runner->activate_reincarnation_tome(1);
	}
	return true;
}

void player_reincarnation::DeactivateTome()
{
	if(_tome_active)
	{
		//任何时候都可以取消激活
		_tome_active = false;
		_imp->_runner->activate_reincarnation_tome(0);
	}
}

bool player_reincarnation::CheckActivateTome()
{
	if(_imp->_basic.level < REINCARNATION_LEVEL_REQUIRED) return false;
	if(_records.size() == 0) return false;
	if(_records.size() == REINCARNATION_TIMES_LIMIT)
	{
		bool level_max = true;
		for(size_t i=0; i<_records.size(); i++)
		{
			if(_records[i].level < player_template::GetMaxLevel())
			{
				level_max = false;
				break;
			}
		}
		if(level_max) return false;
	}
	if(_tome_exp >= REINCARNATION_TOME_EXP_MAX) return false;
	return true;
}

void player_reincarnation::IncTomeExp(int exp)
{
	ASSERT(IsTomeActive());
	if(exp >= REINCARNATION_TOME_EXP_MAX - _tome_exp)
	{
		_tome_exp = REINCARNATION_TOME_EXP_MAX;
		DeactivateTome();
	}
	else
	{
		_tome_exp += exp;
	}
}

void player_reincarnation::CalcHistoricalMaxLevel()
{
	int max = _imp->_basic.level;
	for(size_t i=0; i<_records.size(); i++)
	{
		if(_records[i].level > max)
			max = _records[i].level;
	}
	_historical_max_level = max;
}

void player_reincarnation::CalcExpBonus()
{
	_exp_bonus = 0.f;
	if(GetTimes() == 0 || _imp->_basic.level >= REINCARNATION_LEVEL_REQUIRED) return;

	DATA_TYPE dt;
	PLAYER_REINCARNATION_CONFIG * pCfg = (PLAYER_REINCARNATION_CONFIG *) world_manager::GetDataMan().get_data_ptr(REINCARNATION_CONFIG_ID, ID_SPACE_CONFIG, dt);
	if(dt != DT_PLAYER_REINCARNATION_CONFIG || pCfg == NULL) return;

	_exp_bonus = pCfg->level[GetTimes()-1].exp_premote;
	if(_exp_bonus < 0) _exp_bonus = 0;
	if(_exp_bonus > 10.f) _exp_bonus = 10.f;
}


