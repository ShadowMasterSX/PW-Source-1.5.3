#include "string.h"
#include "world.h"
#include "common/message.h"
#include "fateringmanager.h"
#include "template/itemdataman.h"
#include "template/globaldataman.h"
#include "worldmanager.h"
#include "playerfatering.h"
#include "player_imp.h"

bool fatering_essence::EnhanceFateRing(gplayer_imp * imp, int level) const
{
	float ratio = GetFateRingRatio(level);
	ratio *= player_template::GetFateRingAdjust(imp->GetObjectClass(), type);

	//可能是0级命轮的激活，系数为0，下边就不加了
	if (ratio < 1e-5)	return false;

	int res[MAGIC_CLASS] = {
		(int)(_extend.resistance[0] * ratio + 0.5f),
		(int)(_extend.resistance[1] * ratio + 0.5f),
		(int)(_extend.resistance[2] * ratio + 0.5f),
		(int)(_extend.resistance[3] * ratio + 0.5f),
		(int)(_extend.resistance[4] * ratio + 0.5f)
	};
	//和卡牌系统加一样的属性
	imp->GeneralCardEnhance((int)(_extend.max_hp * ratio + 0.5f), 
							(int)(_extend.damage_low * ratio + 0.5f),
						   	(int)(_extend.damage_high * ratio + 0.5f), 
							(int)(_extend.magic_damage_low * ratio + 0.5f),
							(int)(_extend.magic_damage_high * ratio + 0.5f),
							(int)(_extend.defense * ratio + 0.5f),
							res,
							(int)(_extend.vigour * ratio + 0.5f)
							);
	return true;
}

bool fatering_essence::ImpairFateRing(gplayer_imp * imp, int level) const
{
	float ratio = GetFateRingRatio(level);
	ratio *= player_template::GetFateRingAdjust(imp->GetObjectClass(), type);

	//可能是0级命轮的激活，系数为0，下边就不加了
	if (ratio < 1e-5)	return false;

	int res[MAGIC_CLASS] = {
		(int)(_extend.resistance[0] * ratio + 0.5f),
		(int)(_extend.resistance[1] * ratio + 0.5f),
		(int)(_extend.resistance[2] * ratio + 0.5f),
		(int)(_extend.resistance[3] * ratio + 0.5f),
		(int)(_extend.resistance[4] * ratio + 0.5f)
	};
	//和卡牌系统加一样的属性
	imp->GeneralCardImpair((int)(_extend.max_hp * ratio + 0.5f), 
							(int)(_extend.damage_low * ratio + 0.5f),
						   	(int)(_extend.damage_high * ratio + 0.5f), 
							(int)(_extend.magic_damage_low * ratio + 0.5f),
							(int)(_extend.magic_damage_high * ratio + 0.5f),
							(int)(_extend.defense * ratio + 0.5f),
							res,
							(int)(_extend.vigour * ratio + 0.5f)
							);
	return true;
}

bool fatering_manager::Initialize(itemdataman & dataman)
{
	DATA_TYPE dt;
	size_t id = dataman.get_first_data_id(ID_SPACE_CONFIG,dt);
	for(; id != 0; id = dataman.get_next_data_id(ID_SPACE_CONFIG,dt))
	{
		if (dt == DT_PLAYER_SPIRIT_CONFIG)
		{
			const PLAYER_SPIRIT_CONFIG &psc = *(const PLAYER_SPIRIT_CONFIG*)dataman.get_data_ptr(id,ID_SPACE_CONFIG,dt); 
			ASSERT(dt == DT_PLAYER_SPIRIT_CONFIG && &psc);
			if (psc.type < 0 || psc.type >= PLAYER_FATE_RING_TOTAL)
				return false;
			if (faterings.find(psc.type) != faterings.end())
				return false;

			fatering_essence fe;
			fe.SetType(psc.type);
			fe.SetExtend(psc.hp, psc.damage, psc.magic_damage, psc.defence, psc.magic_defence, psc.vigour);
			for (int i = 0; i < PLAYER_FATE_RING_MAX_LEVEL; i++)
			{
				ASSERT(psc.list[i].require_power > 0 && psc.list[i].require_level >= 0 && psc.list[i].gain_ratio > 0.f);
				fatering_property fp;
				memset(&fp, 0, sizeof(fp));
				fp.power = psc.list[i].require_power;
				fp.require_level = psc.list[i].require_level;
				fp.ratio = psc.list[i].gain_ratio;
				
				fe.InsertEnum(fp);
			}

			faterings[psc.type] = fe;
		}
	}
	if (faterings.size() != PLAYER_FATE_RING_TOTAL)
		return false;

	return true;
}

bool fatering_manager::EnhanceFateRing(gplayer_imp * imp, int index, int level) const
{
	const fatering_essence * fe = GetFateRingEssence(index);
	if (!fe)
		return false;

	return fe->EnhanceFateRing(imp,level);
}

bool fatering_manager::ImpairFateRing(gplayer_imp * imp, int index, int level) const
{
	const fatering_essence * fe = GetFateRingEssence(index);
	if (!fe)
		return false;

	return fe->ImpairFateRing(imp,level);
}
