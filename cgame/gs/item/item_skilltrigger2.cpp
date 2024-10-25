#include "../clstab.h"
#include "../world.h"
#include "../player_imp.h"
#include "../cooldowncfg.h"
#include "../actsession.h"
#include "item_skilltrigger2.h"

DEFINE_SUBSTANCE(skilltrigger2_item, item_body, CLS_ITEM_SKILLTRIGGER2)		//CLS在clstab.h中定义

bool skilltrigger2_item::GetSkillData(unsigned int& skill_id, unsigned int& skill_level)
{
	DATA_TYPE dt;
	struct TARGET_ITEM_ESSENCE* ess = (struct TARGET_ITEM_ESSENCE*)world_manager::GetDataMan().get_data_ptr(_tid,ID_SPACE_ESSENCE,dt);
	if(ess == NULL || dt != DT_TARGET_ITEM_ESSENCE)
	{
		ASSERT(false);
		return false;
	}
	skill_id = ess->id_skill;
	skill_level = ess->skill_level;
	return true;
}

int skilltrigger2_item::OnUseWithTarget(item::LOCATION l,int index,gactive_imp * obj,const XID & target, char force_attack)
{
	DATA_TYPE dt;
	struct TARGET_ITEM_ESSENCE* ess = (struct TARGET_ITEM_ESSENCE*)world_manager::GetDataMan().get_data_ptr(_tid,ID_SPACE_ESSENCE,dt);
	if(ess == NULL || dt != DT_TARGET_ITEM_ESSENCE)
	{
		ASSERT(false);
		return -1;
	}
	gplayer_imp* pImp = (gplayer_imp*)obj;
	//物品使用冷却
	if(!pImp->CheckCoolDown(COOLDOWN_INDEX_SKILLTRIGGER2)) 
	{
		pImp->_runner->error_message(S2C::ERR_OBJECT_IS_COOLING);
		return -1;
	}
	//判断地图是否符合
	if(ess->num_area > 0)
	{
		int world_tag = world_manager::GetWorldTag();
		int i=0;
		for(;i<ess->num_area;i++)
		{
			if(ess->area_id[i] == world_tag)
				break;
		}
		if(i == ess->num_area) return -1;
	}
	//是否仅在安全区使用
	if(ess->use_in_sanctuary_only && !player_template::IsInSanctuary(pImp->_parent->pos)) 
		return -1;
	//战斗状态是否可用
	if(!ess->use_in_combat && pImp->IsCombatState())
		return -1;

	if(pImp->GetHistoricalMaxLevel() < ess->require_level)
		return -1;
	
	world::object_info info;
	if(!pImp->_plane->QueryObject(target,info))
		return -1;
	if(ess->target_faction && !(ess->target_faction & info.faction))
		return -1;
	if((ess->combined_switch & TICS_TARGET_MY_FACTION_OBJECT) && pImp->OI_GetMafiaID() != info.mafia_id)
		return -1;
	//cast
	/*if(pImp->_skill.IsPosSkill(ess->id_skill))
	{
		return -1;	
	}
	else*/ 
	if(pImp->_skill.IsInstant(ess->id_skill))
	{
		session_rune_instant_skill *pSkill= new session_rune_instant_skill(pImp);
		pSkill->SetLevel(ess->skill_level);
		pSkill->SetInvIndex(index);
		pSkill->SetConsumeIfUse(ess->num_use_pertime>0);
		int id = target.id;
		pSkill->SetTarget(ess->id_skill,force_attack,1,&id);
		if(pImp->AddSession(pSkill)) pImp->StartSession();
	}
	else
	{
		session_rune_skill *pSkill= new session_rune_skill(pImp);
		pSkill->SetLevel(ess->skill_level);
		pSkill->SetInvIndex(index);
		pSkill->SetConsumeIfUse(ess->num_use_pertime>0);
		int id = target.id;
		pSkill->SetTarget(ess->id_skill,force_attack,1,&id);
		if(pImp->AddSession(pSkill)) pImp->StartSession();
	}

	pImp->SetCoolDown(COOLDOWN_INDEX_SKILLTRIGGER2,SKILLTRIGGER2_COOLDOWN_TIME);
	return 0;//物品在session中扣除
}
