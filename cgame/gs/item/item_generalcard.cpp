#include "../clstab.h"
#include "../world.h"
#include "../player_imp.h"
#include "set_addon.h"
#include "item_addon.h"
#include "item_generalcard.h"

DEFINE_SUBSTANCE(generalcard_item, item_body, CLS_ITEM_GENERALCARD)

namespace
{
	//六个命轮位置在放入不匹配的卡牌时候激活的属性
	const static int genaralcard_column_extend[6][12] = {
		//{抗性,抗性,抗性,抗性,抗性,最大生命,物伤下限,物伤上限,法伤下限,法伤上限,防御,气魄}
		{0,0,0,0,0,0,200,200,200,200,0,0},
		{0,0,0,0,0,0,200,200,200,200,0,0},
		{0,0,0,0,0,330,0,0,0,0,700,0},
		{700,700,700,700,700,330,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,50},
		{0,0,0,0,0,0,0,0,0,0,0,50},
	};

	//卡牌品级对应的属性系数
	const static float rank_ratio[5] = {0.0f,0.65f,1.0f,1.35f,1.7f};

	float __rebirth_enhance(int times)
	{
		ASSERT(times >= 0 && times <= GENERALCARD_MAX_REBIRTH_TIMES);
		static const float table[GENERALCARD_MAX_REBIRTH_TIMES + 1] = {0.f, 0.3f, 0.6f};
		return table[times];
	}
}

bool generalcard_item::Load(archive & ar)
{
	ar.pop_back(&_ess, sizeof(_ess));
	
	ClearData();
	UpdateEssence();	
	_extra_addon = set_addon_manager::GetAddonList(_tid);

	CalcCRC();
	return true;
}

void generalcard_item::ClearData()
{
	memset(&_extend, 0, sizeof(_extend));
}

void generalcard_item::UpdateEssence()
{
	DATA_TYPE dt;
	const POKER_ESSENCE & card = *(const POKER_ESSENCE *)world_manager::GetDataMan().get_data_ptr(_tid, ID_SPACE_ESSENCE, dt);
	ASSERT(&card || dt == DT_POKER_ESSENCE);

	_extend.max_hp				= card.hp + card.inc_hp * (_ess.level-1);
	_extend.damage_low			= card.damage + card.inc_damage * (_ess.level-1);
	_extend.damage_high			= card.damage + card.inc_damage * (_ess.level-1);
	_extend.damage_magic_low	= card.magic_damage + card.inc_magic_damage * (_ess.level-1);
	_extend.damage_magic_high	= card.magic_damage + card.inc_magic_damage * (_ess.level-1);
	_extend.defense				= card.defence + card.inc_defence * (_ess.level-1);  
	_extend.resistance[0]		= card.magic_defence[0] + card.inc_magic_defence[0] * (_ess.level-1);
	_extend.resistance[1]		= card.magic_defence[1] + card.inc_magic_defence[1] * (_ess.level-1);
	_extend.resistance[2]		= card.magic_defence[2] + card.inc_magic_defence[2] * (_ess.level-1);
	_extend.resistance[3]		= card.magic_defence[3] + card.inc_magic_defence[3] * (_ess.level-1);
	_extend.resistance[4]		= card.magic_defence[4] + card.inc_magic_defence[4] * (_ess.level-1);
	_extend.vigour				= card.vigour + card.inc_vigour * (_ess.level-1);
}

void generalcard_item::OnRefreshItem()
{
	ClearData();
	UpdateEssence(); 

	CalcCRC();
}

bool generalcard_item::VerifyRequirement(item_list & list, gactive_imp* imp)
{
	if(list.GetLocation() == item::BODY)
	{
		return imp->GetHistoricalMaxLevel() >= _ess.require_level
			&& imp->GetAvailLeadership() >= _ess.require_leadership;
	}
	return false;
}

void generalcard_item::OnActivate(item::LOCATION, size_t pos, size_t count, gactive_imp* imp)
{
	int card_column_index = GetGeneralCardColumnIndex(pos);
	ASSERT(card_column_index >= 0 && card_column_index < 6);
	
	float adjust = player_template::GetGeneralCardClsAdjust(imp->GetObjectClass(), card_column_index);
	if(IsSecActive()) adjust *= 1.f + GetSecActiveParam<float>();
	if(_ess.rebirth_times) adjust *= 1.f + __rebirth_enhance(_ess.rebirth_times);

	if (card_column_index == _ess.type)	//卡牌类型和卡牌位置匹配 
	{
		if (adjust > 1e-5)	//系数为0的时候就不增加了，提高效率
		{
			int res[MAGIC_CLASS];
			res[0] = (int)(_extend.resistance[0] * adjust + 0.5f);
			res[1] = (int)(_extend.resistance[1] * adjust + 0.5f);
			res[2] = (int)(_extend.resistance[2] * adjust + 0.5f);
			res[3] = (int)(_extend.resistance[3] * adjust + 0.5f);
			res[4] = (int)(_extend.resistance[4] * adjust + 0.5f);
			imp->GeneralCardEnhance((int)(_extend.max_hp * adjust + 0.5f), 
									(int)(_extend.damage_low * adjust + 0.5f), 
									(int)(_extend.damage_high * adjust + 0.5f), 
									(int)(_extend.damage_magic_low * adjust + 0.5f), 
									(int)(_extend.damage_magic_high * adjust + 0.5f), 
									(int)(_extend.defense * adjust + 0.5f), 
									res, 
									(int)(_extend.vigour * adjust + 0.5f));
		}
		if(_extra_addon)
		{
			for(size_t i = 0;i < _extra_addon->size(); i ++)
			{
				addon_manager::Activate((*_extra_addon)[i],NULL,imp);
			}
		}
	}
	else	//卡牌类型和卡牌位置不匹配
	{
		adjust *= rank_ratio[_ess.rank];

		if (adjust > 1e-5)	//系数为0的话就不增加了，提高效率
		{
			int res[MAGIC_CLASS];
			res[0] = (int)(genaralcard_column_extend[card_column_index][0] * adjust + 0.5f);
			res[1] = (int)(genaralcard_column_extend[card_column_index][1] * adjust + 0.5f);
			res[2] = (int)(genaralcard_column_extend[card_column_index][2] * adjust + 0.5f);
			res[3] = (int)(genaralcard_column_extend[card_column_index][3] * adjust + 0.5f);
			res[4] = (int)(genaralcard_column_extend[card_column_index][4] * adjust + 0.5f);
			imp->GeneralCardEnhance((int)(genaralcard_column_extend[card_column_index][5] * adjust + 0.5f), 
									(int)(genaralcard_column_extend[card_column_index][6] * adjust + 0.5f), 
									(int)(genaralcard_column_extend[card_column_index][7] * adjust + 0.5f), 
									(int)(genaralcard_column_extend[card_column_index][8] * adjust + 0.5f), 
									(int)(genaralcard_column_extend[card_column_index][9] * adjust + 0.5f), 
									(int)(genaralcard_column_extend[card_column_index][10] * adjust + 0.5f), 
									res, 
									(int)(genaralcard_column_extend[card_column_index][11] * adjust + 0.5f));
		}
	}

	imp->OccupyLeadership(_ess.require_leadership);
}

void generalcard_item::OnDeactivate(item::LOCATION, size_t pos, size_t count,gactive_imp* imp)
{
	int card_column_index = GetGeneralCardColumnIndex(pos);
	ASSERT(card_column_index >= 0 && card_column_index < 6);
	
	float adjust = player_template::GetGeneralCardClsAdjust(imp->GetObjectClass(), card_column_index);
	if(IsSecActive()) adjust *= 1.f + GetSecActiveParam<float>();
	if(_ess.rebirth_times) adjust *= 1.f + __rebirth_enhance(_ess.rebirth_times);
	
	if (card_column_index == _ess.type)	//卡牌类型和卡牌位置匹配 
	{
		if (adjust > 1e-5)
		{
			int res[MAGIC_CLASS];
			res[0] = (int)(_extend.resistance[0] * adjust + 0.5f);
			res[1] = (int)(_extend.resistance[1] * adjust + 0.5f);
			res[2] = (int)(_extend.resistance[2] * adjust + 0.5f);
			res[3] = (int)(_extend.resistance[3] * adjust + 0.5f);
			res[4] = (int)(_extend.resistance[4] * adjust + 0.5f);
			imp->GeneralCardImpair((int)(_extend.max_hp * adjust + 0.5f), 
									(int)(_extend.damage_low * adjust + 0.5f), 
									(int)(_extend.damage_high * adjust + 0.5f), 
									(int)(_extend.damage_magic_low * adjust + 0.5f), 
									(int)(_extend.damage_magic_high * adjust + 0.5f), 
									(int)(_extend.defense * adjust + 0.5f), 
									res, 
									(int)(_extend.vigour * adjust + 0.5f));
		}
		if(_extra_addon)
		{
			for(size_t i = 0;i < _extra_addon->size(); i ++)
			{       
				addon_manager::Deactivate((*_extra_addon)[i],NULL,imp);
			}       
		}
	}
	else
	{
		adjust *= rank_ratio[_ess.rank];

		if (adjust > 1e-5)
		{
			int res[MAGIC_CLASS];
			res[0] = (int)(genaralcard_column_extend[card_column_index][0] * adjust + 0.5f);
			res[1] = (int)(genaralcard_column_extend[card_column_index][1] * adjust + 0.5f);
			res[2] = (int)(genaralcard_column_extend[card_column_index][2] * adjust + 0.5f);
			res[3] = (int)(genaralcard_column_extend[card_column_index][3] * adjust + 0.5f);
			res[4] = (int)(genaralcard_column_extend[card_column_index][4] * adjust + 0.5f);
			imp->GeneralCardImpair((int)(genaralcard_column_extend[card_column_index][5] * adjust + 0.5f), 
									(int)(genaralcard_column_extend[card_column_index][6] * adjust + 0.5f), 
									(int)(genaralcard_column_extend[card_column_index][7] * adjust + 0.5f), 
									(int)(genaralcard_column_extend[card_column_index][8] * adjust + 0.5f), 
									(int)(genaralcard_column_extend[card_column_index][9] * adjust + 0.5f), 
									(int)(genaralcard_column_extend[card_column_index][10] * adjust + 0.5f), 
									res, 
									(int)(genaralcard_column_extend[card_column_index][11] * adjust + 0.5f));
		}
	}
	
	imp->RestoreLeadership(_ess.require_leadership);
}

void generalcard_item::OnPutIn(item::LOCATION l,item_list & list,size_t pos,size_t count,gactive_imp* obj)
{
	//do nothing  因为激活由外面的RefreshEquip控制
}

void generalcard_item::OnTakeOut(item::LOCATION l,size_t pos,size_t count, gactive_imp* obj)
{
	if(l == item::BODY)
	{
		Deactivate(l, pos, count,obj);
	}
}

bool generalcard_item::CheckRebirthCondition(int material_rebirth_times)
{
	return (_ess.level >= _ess.max_level
			&& _ess.rebirth_times < GENERALCARD_MAX_REBIRTH_TIMES
			&& _ess.rebirth_times == material_rebirth_times);
}

bool generalcard_item::DoRebirth(int arg)
{
	ASSERT(!_is_active);
	_ess.level = 1;
	_ess.exp = 0;
	_ess.rebirth_times ++;

	OnRefreshItem();
	return true;
}

bool generalcard_item::InsertExp(int& exp, bool ischeck)
{
	if(_ess.level >= _ess.max_level) return false;
	int tmp = _ess.exp + exp;
	if(tmp <= _ess.exp) return false;
	if(ischeck) return true;

	ASSERT(!_is_active);
	_ess.exp = tmp;
	do
	{
		int lvlup_exp = player_template::GetGeneralCardLvlupExp(_ess.rank, _ess.level);
		if(_ess.exp < lvlup_exp) break;
		
		_ess.exp -= lvlup_exp;
		_ess.level ++;
		
		if(_ess.level >= _ess.max_level)
		{
			_ess.exp = 0;
			break;
		}
	}while(1);

	OnRefreshItem(); 
	return true;
}

int generalcard_item::GetSwallowExp()
{
	int exp = 0;
	//计算转生消耗的经验
	if(_ess.rebirth_times > 0)
	{
		int exp2 = 0;
		for(int i=1; i<_ess.max_level; i++)
		{
			exp2 += player_template::GetGeneralCardLvlupExp(_ess.rank, i);
		}
		exp += exp2 * _ess.rebirth_times;
	}
	//计算当前等级消耗的经验
	for(int i=1; i<_ess.level; i++)
	{
		exp += player_template::GetGeneralCardLvlupExp(_ess.rank, i);
	}
	exp += _ess.exp;
	
	return exp;
}

