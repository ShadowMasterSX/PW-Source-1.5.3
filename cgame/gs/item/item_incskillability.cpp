#include "../clstab.h"
#include "../world.h"
#include "../player_imp.h"
#include "item_incskillability.h"

DEFINE_SUBSTANCE(incskillability_item, item_body, CLS_ITEM_INCSKILLABILITY)		//CLS在clstab.h中定义

int incskillability_item::OnUse(item::LOCATION l, gactive_imp * imp, size_t count)
{
	DATA_TYPE dt;
	INC_SKILL_ABILITY_ESSENCE* ess = (INC_SKILL_ABILITY_ESSENCE*)world_manager::GetDataMan().get_data_ptr(_tid,ID_SPACE_ESSENCE,dt);
	if(!ess || dt != DT_INC_SKILL_ABILITY_ESSENCE) return -1;
	//验证生产技能等级
	int level = imp->GetProduceSkillLevel(ess->id_skill);
	if(level <= 0 || level != ess->level_required) return -1;
	//熟练度是否满
	if(imp->GetSkillAbilityRatio(ess->id_skill) > 0.999999f) return -1;

	imp->IncSkillAbilityRatio(ess->id_skill, ess->inc_ratio);
	return 1;
}

