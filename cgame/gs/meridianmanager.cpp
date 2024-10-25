#include "string.h"
#include "meridianmanager.h"
#include "world.h"
#include "player_imp.h"

int meridian_manager::MERIDIAN_COST_LIST[80][3] = {	25,1,5,
													20,1,10,
													15,1,15,
													10,1,20,
													9,1,25,
													9,1,30,
													8,1,35,
													8,1,40,
													7,1,45,
													7,1,50,
													6,1,56,
													6,1,61,
													5,1,67,
													5,1,72,
													5,1,78,
													4,1,83,
													4,1,89,
													4,1,94,
													4,1,100,
													10,2,110,
													8,1,115,
													8,1,120,
													7,1,125,
													7,1,130,
													6,1,136,
													6,1,141,
													5,1,147,
													5,1,152,
													4,1,158,
													4,1,163,
													3,1,169,
													3,1,175,
													2,1,182,
													2,1,188,
													2,1,195,
													1,1,205,
													1,1,215,
													1,1,225,
													1,1,235,
													7,2,254,
													5,1,259,
													4,1,265,
													3,1,271,
													2,1,277,
													1,1,287,
													1,1,297,
													1,1,307,
													10,2,318,
													10,2,329,
													10,2,339,
													10,2,350,
													9,2,363,
													9,2,376,
													9,2,389,
													9,2,402,
													8,2,417,
													8,2,432,
													8,2,447,
													8,2,462,
													12,3,494,
													1,1,504,
													1,1,514,
													1,1,524,
													10,2,534,
													10,2,545,
													9,2,558,
													9,2,571,
													8,2,586,
													8,2,601,
													7,2,620,
													7,2,639,
													6,2,664,
													6,2,689,
													5,2,722,
													5,2,755,
													5,2,788,
													4,2,828,
													4,2,868,
													10,3,912,
													8,3,1000,
};

void
meridian_manager::Save(archive & ar)
{
	ar << _meridian_level << _lifegate_times << _deathgate_times << _free_refine_times << _paid_refine_times << _player_login_time << _continu_login_days;
	ar.push_back(_trigrams_map,sizeof(int)*3);
}

void
meridian_manager::Load(archive & ar)
{
	ar >> _meridian_level >> _lifegate_times >> _deathgate_times >> _free_refine_times >> _paid_refine_times >> _player_login_time >> _continu_login_days;
	ar.pop_back(_trigrams_map,sizeof(int)*3);
}

void
meridian_manager::Swap(meridian_manager & rhs)
{
	abase::swap(_meridian_level,rhs._meridian_level);
	abase::swap(_lifegate_times,rhs._lifegate_times);
	abase::swap(_deathgate_times,rhs._deathgate_times);
	abase::swap(_free_refine_times,rhs._free_refine_times);
	abase::swap(_paid_refine_times,rhs._paid_refine_times);
	abase::swap(_player_login_time,rhs._player_login_time);
	abase::swap(_continu_login_days,rhs._continu_login_days);
	for(int i=0;i<3;i++)
		abase::swap(_trigrams_map[i],rhs._trigrams_map[i]);
}

int
meridian_manager::TryRefineMeridian(gplayer_imp *pImp,int index)
{
	//检查index是否已经被打开过
	int serial = (index * 2)/32;
	int offset = (index * 2)%32;
	//是否两位都为0，即为翻开过
	if((_trigrams_map[serial] >> offset) & 0x01 || (_trigrams_map[serial] >> offset) & 0x02) 
		return MERIDIAN_FATAL_ERR;
	int rst = 0;
	int level = _meridian_level;
	int lifegate_total = meridian_manager::MERIDIAN_COST_LIST[level][0];
	int prob = (int)(100*((float)(lifegate_total - _lifegate_times)/(48 - _lifegate_times - _deathgate_times)));
	//减少次数
	if(_free_refine_times > 0)
		_free_refine_times -= 1;
	else if(_paid_refine_times > 0)
	{
		_paid_refine_times -= 1;
	}
	else
	{
		return MERIDIAN_FATAL_ERR;
	}
	if(abase::Rand(0,100) > prob)  //冲击到死门
	{
		//是否有连续成功要求，且之前已经冲到过生门
		if( meridian_manager::MERIDIAN_COST_LIST[level][1] > 1 && _lifegate_times > 0)
		{
			//失败所有数据归0
			memset(_trigrams_map,0,sizeof(int)*3);
			_lifegate_times = 0;
			_deathgate_times = 0;
			rst = MERIDIAN_DEATH_FAIL;
		}	
		else
		{
			_deathgate_times += 1;
			_trigrams_map[serial] |= 1 << offset;
			rst = MERIDIAN_DEATH_NOFAIL;
		}
	}
	else
	{
		_lifegate_times += 1;
		//判断是否成功
		if(meridian_manager::MERIDIAN_COST_LIST[level][1] == _lifegate_times)
		{
			_meridian_level += 1;
			memset(_trigrams_map,0,sizeof(int)*3);
			_lifegate_times = 0;
			_deathgate_times = 0;
			rst = MERIDIAN_LIFE_REFINE;
		}
		else
		{
			_trigrams_map[serial] |= 1 << (offset + 1);
			rst = MERIDIAN_LIFE_NOTREFINE;
		}
	}
	//通知客户端
	NotifyMeridianData(pImp);
	return rst;
}

void
meridian_manager::Heartbeat(gplayer_imp *pImp,int cur_time)
{
	RefreshRefineTimes(pImp,cur_time);
}

void
meridian_manager::RefreshRefineTimes(gplayer_imp *pImp,int cur_time)
{
	if(cur_time/86400 > _player_login_time)
	{
		_paid_refine_times += MERIDIAN_INC_PAID_REFINE_TIMES;
		if(_paid_refine_times > MERIDIAN_MAX_PAID_REFINE_TIMES)
			_paid_refine_times = MERIDIAN_MAX_PAID_REFINE_TIMES;
		//免费次数计算
		if(cur_time/86400 > (_player_login_time + 1))
		{
			_continu_login_days = 1;
		}
		else
		{
			_continu_login_days ++;
		}
		int cld = _continu_login_days;
		if(cld > 5)
			cld = 5;
		if(cld > _free_refine_times)
		{
			_free_refine_times = cld;
		}
		_player_login_time = cur_time/86400;
		NotifyMeridianData(pImp);
	}
}

void
meridian_manager::InitFromDBData(const GDB::meridian_data &meridian)
{
	_meridian_level = meridian.meridian_level;
	_lifegate_times = meridian.lifegate_times;
	_deathgate_times = meridian.deathgate_times;
	_free_refine_times = meridian.free_refine_times;
	_paid_refine_times = meridian.paid_refine_times;
	_player_login_time = meridian.player_login_time;
	_continu_login_days = meridian.continu_login_days;
	memcpy(_trigrams_map,meridian.trigrams_map,sizeof(int)*3);
}

void
meridian_manager::MakeDBData(GDB::meridian_data &meridian)
{
	meridian.meridian_level = _meridian_level;
	meridian.lifegate_times = _lifegate_times;
	meridian.deathgate_times = _deathgate_times;
	meridian.free_refine_times = _free_refine_times;
	meridian.paid_refine_times = _paid_refine_times;
	meridian.player_login_time = _player_login_time;
	meridian.continu_login_days = _continu_login_days;
	memcpy(meridian.trigrams_map,_trigrams_map,sizeof(int)*3);
}

void
meridian_manager::NotifyMeridianData(gplayer_imp *pImp)
{
	pImp->_runner->notify_meridian_data(_meridian_level,_lifegate_times,_deathgate_times,_free_refine_times,_paid_refine_times,_continu_login_days,_trigrams_map);
}

bool
meridian_manager::CheckMeridianCondition(int plevel)
{
	return (_meridian_level <= MERIDIAN_MAX_REFINE_LEVEL) &&  (plevel >= 100 || plevel >= _meridian_level * 2 + 40) && (_free_refine_times >= 0 || _paid_refine_times >=0);
}

bool
meridian_manager::CheckMeridianFreeRefineTimes()
{
	return _free_refine_times > 0;
}

void
meridian_manager::AddFreeRefineTime(int num)
{
	if(num <= 0)
		return;
	_free_refine_times += num;
	return;
}

void 
meridian_manager::Deactivate(gplayer_imp *pImp)
{
	int level = _meridian_level;
	if(level < 1) return;
	int hp_enh = 0;
	int dmg_enh = 0;
	int magic_enh = 0;
	int def_enh = 0;
	int resis_enh = 0;
	int cls = pImp->GetPlayerClass();
	DATA_TYPE dt;
	const MERIDIAN_CONFIG *pcfg = (MERIDIAN_CONFIG *)world_manager::GetDataMan().get_data_ptr(1064,ID_SPACE_CONFIG,dt);
	if(dt != DT_MERIDIAN_CONFIG || !pcfg) 
		return ;
	
	hp_enh = pcfg->prof_para[cls].hp;
	dmg_enh = pcfg->prof_para[cls].phy_damage;
	magic_enh = pcfg->prof_para[cls].magic_damage;
	def_enh = pcfg->prof_para[cls].phy_defence;
	resis_enh = pcfg->prof_para[cls].magic_defence;
	int enhance_prop = meridian_manager::MERIDIAN_COST_LIST[level-1][2]; //获取当前的系数
	//扣除
	pImp->_en_point.max_hp -= enhance_prop * hp_enh / 100;
	pImp->_en_point.damage_low -= enhance_prop * dmg_enh / 100;
	pImp->_en_point.damage_high -= enhance_prop * dmg_enh / 100;
	pImp->_en_point.magic_dmg_low -= enhance_prop * magic_enh / 100;
	pImp->_en_point.magic_dmg_high -= enhance_prop * magic_enh / 100;
	pImp->_en_point.defense -= enhance_prop * def_enh / 100;
	for(int i = 0; i < 5 ; i++)
	{
		pImp->_en_point.resistance[i] -= enhance_prop * resis_enh / 100;
	}
	return;	
}

void
meridian_manager::Activate(gplayer_imp *pImp)
{
	int level = _meridian_level;
	if(level < 1) return;
	int hp_enh = 0;
	int dmg_enh = 0;
	int magic_enh = 0;
	int def_enh = 0;
	int resis_enh = 0;
	int cls = pImp->GetPlayerClass();
	DATA_TYPE dt;
	const MERIDIAN_CONFIG *pcfg = (MERIDIAN_CONFIG *)world_manager::GetDataMan().get_data_ptr(1064,ID_SPACE_CONFIG,dt);
	if(dt != DT_MERIDIAN_CONFIG || !pcfg) 
		return ;
	
	hp_enh = pcfg->prof_para[cls].hp;
	dmg_enh = pcfg->prof_para[cls].phy_damage;
	magic_enh = pcfg->prof_para[cls].magic_damage;
	def_enh = pcfg->prof_para[cls].phy_defence;
	resis_enh = pcfg->prof_para[cls].magic_defence;
	int enhance_prop = meridian_manager::MERIDIAN_COST_LIST[level-1][2]; //获取当前的系数
	pImp->_en_point.max_hp += enhance_prop * hp_enh / 100;
	pImp->_en_point.damage_low += enhance_prop * dmg_enh / 100;
	pImp->_en_point.damage_high += enhance_prop * dmg_enh / 100;
	pImp->_en_point.magic_dmg_low += enhance_prop * magic_enh / 100;
	pImp->_en_point.magic_dmg_high += enhance_prop * magic_enh / 100;
	pImp->_en_point.defense += enhance_prop * def_enh / 100;
	for(int i = 0; i < 5 ; i++)
	{
		pImp->_en_point.resistance[i] += enhance_prop * resis_enh / 100;
	}
	return;
}

void
meridian_manager::InitEnhance(gplayer_imp *pImp)
{
	int cur_time = g_timer.get_systime();
	RefreshRefineTimes(pImp,cur_time);
	Activate(pImp);
	return;
}
