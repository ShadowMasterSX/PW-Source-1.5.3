#include "policy.h"
#include "policy_loader.h"
#include "../aitrigger.h"
#include "../world.h"
#include "../worldmanager.h"
#include <ASSERT.h>

using namespace ai_trigger;

namespace
{
	expr * ConvertExpr (void *pTree);

	condition * ConvertCondition(void * pTree)
	{
		CTriggerData::_s_tree_item * temp = (CTriggerData::_s_tree_item*)pTree;
		switch(temp->mConditon.iType) 
		{
			case CTriggerData::c_time_come:
				return new cond_timer(((C_TIME_COME*)temp->mConditon.pParam)->uID);
			case CTriggerData::c_hp_less:
				return new cond_hp_less(((C_HP_LESS*)temp->mConditon.pParam)->fPercent);
			case CTriggerData::c_random:
				return new cond_random(((C_RANDOM*)temp->mConditon.pParam)->fProbability);

			case CTriggerData::c_and:
				{
					condition * left = ConvertCondition(temp->pLeft);
					condition * right = ConvertCondition(temp->pRight);
					return new cond_and(left,right);
				}
			case CTriggerData::c_not:
				{
					condition * right = ConvertCondition(temp->pRight);
					return new cond_not(right);
				}
				
			case CTriggerData::c_or:
				{
					condition * left = ConvertCondition(temp->pLeft);
					condition * right = ConvertCondition(temp->pRight);
					return new cond_or(left,right);
				}

			case CTriggerData::c_kill_player:
				return new cond_kill_target();
			case CTriggerData::c_start_attack:
				return new cond_start_combat();

			case CTriggerData::c_died:
				return new cond_on_death();

			case CTriggerData::c_less:
				{
					expr * left = ConvertExpr(temp->pLeft);
					expr * right = ConvertExpr(temp->pRight);
					return new cond_compare_less (left, right);
				}
			case CTriggerData::c_great:
				{
					expr * left = ConvertExpr(temp->pLeft);
					expr * right = ConvertExpr(temp->pRight);
					return new cond_compare_greater (left, right);
				}
			case CTriggerData::c_equ:
				{
					expr * left = ConvertExpr(temp->pLeft);
					expr * right = ConvertExpr(temp->pRight);
					return new cond_compare_equal (left, right);
				}
			//case CTriggerData::c_time_point://lgc
				//return new cond_time_point(((C_TIME_POINT*)temp->mConditon.pParam)->uHour, ((C_TIME_POINT*)temp->mConditon.pParam)->uMinute);	

			case CTriggerData::c_be_hurt:
				return new cond_on_damage(((C_BE_HURT*)temp->mConditon.pParam)->iHurtLow,((C_BE_HURT*)temp->mConditon.pParam)->iHurtHigh);

			case CTriggerData::c_reach_end:
				return new cond_path_end(((C_REACH_END*)temp->mConditon.pParam)->iPathID);
			
			case CTriggerData::c_at_history_stage:
				return new cond_at_history_stage(((C_HISTORY_STAGE*)temp->mConditon.pParam)->iID);
			
			case CTriggerData::c_stop_fight:
				return new cond_end_combat();

			case CTriggerData::c_reach_end_2:
				return new cond_path_end_2(((C_REACH_END_2*)temp->mConditon.pParam)->iPathID,((C_REACH_END_2*)temp->mConditon.pParam)->iPathIDType); 

			case CTriggerData::c_has_filter:
				return new cond_spec_filter(((C_HAS_FILTER*)temp->mConditon.pParam)->iID) ;

			default:
			ASSERT(false);
			break;
		}
		return NULL;
	}

	expr * ConvertExpr (void *pTree)
	{
		//事实上这部分是表达式树，不是条件树
		CTriggerData::_s_tree_item * temp = (CTriggerData::_s_tree_item*)pTree;
		switch(temp->mConditon.iType)
		{
			case CTriggerData::c_plus:
				{
					expr * left = ConvertExpr(temp->pLeft);
					expr * right = ConvertExpr(temp->pRight);
					return new expr_plus (left, right);
				}
			case CTriggerData::c_minus:
				{
					expr * left = ConvertExpr(temp->pLeft);
					expr * right = ConvertExpr(temp->pRight);
					return new expr_minus (left, right);
				}
			case CTriggerData::c_multiply:
				{
					expr * left = ConvertExpr(temp->pLeft);
					expr * right = ConvertExpr(temp->pRight);
					return new expr_multiply (left, right);
				}
			case CTriggerData::c_divide:
				{
					expr * left = ConvertExpr(temp->pLeft);
					expr * right = ConvertExpr(temp->pRight);
					return new expr_divide (left, right);
				}
			case CTriggerData::c_constant:
				{
					return new expr_constant(((C_CONSTANT*)temp->mConditon.pParam)->iValue);
				}
			case CTriggerData::c_var:
				{
					return new expr_common_data(((C_VAR*)temp->mConditon.pParam)->iID);
				}
			case CTriggerData::c_history_value:
				{
					return new expr_history_value(((C_HISTORY_VALUE*)temp->mConditon.pParam)->iValue); 
				}
			case CTriggerData::c_local_var:
				{
					return new expr_local_value(((C_LOCAL_VAR*)temp->mConditon.pParam)->iID);
				}
			case CTriggerData::c_room_index:
				{
					return new expr_room_index();
				}
			default:
				ASSERT(false);
				break;
		}
		return NULL;
	}

	target * ConvertTarget(const CTriggerData::_s_target & mTarget)
	{
		//目标类型，目前只有t_occupation_list有参数
		switch(mTarget.iType)
		{
			case CTriggerData::t_hate_first:
				return new target_aggro_first();
			case CTriggerData::t_hate_second:
				return new target_aggro_second();
			case CTriggerData::t_hate_others:
				return new target_aggro_second_rand();
			case CTriggerData::t_most_hp:
				return new target_most_hp();
			case CTriggerData::t_most_mp:
				return new target_most_mp();
			case CTriggerData::t_least_hp:
				return new target_least_hp();
			case CTriggerData::t_self:
				return new target_self();
			case CTriggerData::t_occupation_list:
				{
					int bit =  ((T_OCCUPATION*)mTarget.pParam)->uBit;
					return new target_class_combo(bit);
				}
			case CTriggerData::t_monster_killer:
				return new target_monster_killer();
			case CTriggerData::t_monster_birthplace_faction:
				return new target_monster_birthplace_faction();
			case CTriggerData::t_hate_random_one:
				return new target_aggro_special(target_aggro_special::ATAS_RAND);
			case CTriggerData::t_hate_nearest:
				return new target_aggro_special(target_aggro_special::ATAS_NEAR);
			case CTriggerData::t_hate_farthest:
				return new target_aggro_special(target_aggro_special::ATAS_FAR);
			case CTriggerData::t_hate_first_redirected:
				return new target_aggro_first_redirected();
		}
		return NULL;
	}

	trigger * ConvertTrigger(CPolicyData *pPolicyData, CTriggerData *pTriggerData);
	operation * ConvertOperation(CPolicyData *pPolicyData, CTriggerData::_s_operation *pOperation)
	{
		target * tar = ConvertTarget(pOperation->mTarget);
		operation * pOP = NULL;

		#define OPARAM(x) ((x*)pOperation->pParam)

		//操作类型
		switch(pOperation->iType) 
		{
			case CTriggerData::o_attact:
				pOP = new op_attack(OPARAM(O_ATTACK_TYPE)->uType);
				break;
			case CTriggerData::o_use_skill:
				pOP = new op_skill(OPARAM(O_USE_SKILL)->uSkill,OPARAM(O_USE_SKILL)->uLevel);
				break;
			case CTriggerData::o_use_skill_2:
				pOP = new op_skill_2(OPARAM(O_USE_SKILL_2)->uSkill,OPARAM(O_USE_SKILL_2)->uSkillType,
						OPARAM(O_USE_SKILL_2)->uLevel,OPARAM(O_USE_SKILL_2)->uLevelType);
				break;
			case CTriggerData::o_talk:
				if(OPARAM(O_TALK_TEXT)->uAppendDataMask)
					pOP = new op_say_2(OPARAM(O_TALK_TEXT)->szData,OPARAM(O_TALK_TEXT)->uSize,OPARAM(O_TALK_TEXT)->uAppendDataMask);
				else	
					pOP = new op_say(OPARAM(O_TALK_TEXT)->szData,OPARAM(O_TALK_TEXT)->uSize);
				break;
			case CTriggerData::o_reset_hate_list:
				pOP = new op_reset_aggro();
				break;
			case CTriggerData::o_run_trigger:
				{
					int idx = pPolicyData->GetIndex(OPARAM(O_RUN_TRIGGER)->uID);
					CTriggerData *pTriggerData = pPolicyData->GetTriggerPtr(idx);
					trigger * pT = ConvertTrigger(pPolicyData,pTriggerData);
					pOP = new op_exec_trigger(pT);
				}
				break;
			case CTriggerData::o_stop_trigger:
				pOP = new op_enable_trigger(OPARAM(O_STOP_TRIGGER)->uID,false);
				break;
			case CTriggerData::o_active_trigger:
				pOP = new op_enable_trigger(OPARAM(O_ACTIVE_TRIGGER)->uID,true);
				break;
			case CTriggerData::o_create_timer:
				pOP = new op_create_timer(OPARAM(O_CREATE_TIMER)->uID,OPARAM(O_CREATE_TIMER)->uPeriod,OPARAM(O_CREATE_TIMER)->uCounter);
				break;
			case CTriggerData::o_kill_timer:
				pOP = new op_remove_timer(OPARAM(O_KILL_TIMER)->uID);
				break;

			case CTriggerData::o_flee:
				pOP = new op_flee();
				break;
			case CTriggerData::o_set_hate_to_first:
				pOP = new op_be_taunted();
				break;
			case CTriggerData::o_set_hate_to_last:
				pOP = new op_fade_target();
				break;
			case CTriggerData::o_set_hate_fifty_percent:
				pOP = new op_aggro_fade();
				break;
			case CTriggerData::o_skip_operation:
				pOP = new op_break();
				break;
			case CTriggerData::o_active_controller:
				pOP = new op_active_spawner(OPARAM(O_ACTIVE_CONTROLLER)->uID,OPARAM(O_ACTIVE_CONTROLLER)->bStop);
				break;
			case CTriggerData::o_active_controller_2:
				pOP = new op_active_spawner_2(OPARAM(O_ACTIVE_CONTROLLER_2)->uID,OPARAM(O_ACTIVE_CONTROLLER_2)->uIDType,OPARAM(O_ACTIVE_CONTROLLER_2)->bStop);
				break;
			case CTriggerData::o_set_global:
				{
					O_SET_GLOBAL * pSG = (O_SET_GLOBAL*)(pOperation->pParam);
					pOP = new op_set_common_data (pSG->iID, pSG->iValue, pSG->bIsValue);
				}
				break;
			case CTriggerData::o_revise_global:
				{
					O_REVISE_GLOBAL * pRG = (O_REVISE_GLOBAL*)(pOperation->pParam);
					pOP = new op_add_common_data (pRG->iID, pRG->iValue);
				}
				break;
			case CTriggerData::o_summon_monster:
				{
					O_SUMMON_MONSTER* pSM = (O_SUMMON_MONSTER *)(pOperation->pParam);
					pOP = new op_summon_monster(pSM->iMonsterID, pSM->iMonsterNum, pSM->iRange, pSM->iLife, (char)pSM->iDispearCondition, pSM->iPathID);
				}
				break;
			case CTriggerData::o_summon_monster_2:
				{
					O_SUMMON_MONSTER_2* pSM = (O_SUMMON_MONSTER_2 *)(pOperation->pParam);
					pOP = new op_summon_monster_2(pSM->iMonsterID,pSM->iMonsterIDType, pSM->iMonsterNum,pSM->iMonsterNumType, pSM->iRange, pSM->iLife, (char)pSM->iDispearCondition, pSM->iPathID,pSM->iPathIDType);
				}
				break;
			case CTriggerData::o_walk_along:
				{
					O_WALK_ALONG* pWA = (O_WALK_ALONG*)(pOperation->pParam);
					pOP = new op_change_path(pWA->iWorldID,pWA->iPathID,pWA->iPatrolType,pWA->iSpeedType);
				}
				break;
			case CTriggerData::o_walk_along_2:
				{
					O_WALK_ALONG_2* pWA = (O_WALK_ALONG_2*)(pOperation->pParam);
					pOP = new op_change_path_2(pWA->iWorldID,pWA->iPathID,pWA->iPathIDType,pWA->iPatrolType,pWA->iSpeedType);
				}
				break;
			case CTriggerData::o_play_action:
				{
					O_PLAY_ACTION* pPA= (O_PLAY_ACTION*)(pOperation->pParam);
					pOP = new op_play_action(pPA->szActionName,pPA->iLoopCount,pPA->iPlayTime,pPA->iInterval);
				}
				break;
			case CTriggerData::o_revise_history:
				{
					O_REVISE_HISTORY* pRH = (O_REVISE_HISTORY*)(pOperation->pParam);
					pOP = new op_revise_history(pRH->iID,pRH->iValue);
				}
				break;
			case CTriggerData::o_set_history:
				{
					O_SET_HISTORY* pSH = (O_SET_HISTORY*)(pOperation->pParam);
					pOP = new op_set_history(pSH->iID,pSH->iValue,pSH->bIsHistoryValue);
				}
				break;
			case CTriggerData::o_deliver_faction_pvp_points:
				{
					O_DELIVER_FACTION_PVP_POINTS* pDF = (O_DELIVER_FACTION_PVP_POINTS*)(pOperation->pParam);
					pOP = new op_deliver_faction_pvp_points(pDF->uType);
				}
				break;
			case CTriggerData::o_calc_var:
				{
					O_CALC_VAR* pCV = (O_CALC_VAR*)(pOperation->pParam);
					pOP = new op_calc_var(pCV->iDst,pCV->iDstType,pCV->iSrc1,pCV->iSrc1Type,pCV->iSrc2,pCV->iSrc2Type,pCV->iOp); 
				}
				break;
			case CTriggerData::o_deliver_task:
				{
					O_DELIVER_TASK* pDT = (O_DELIVER_TASK*)(pOperation->pParam); 
					pOP = new op_deliver_task(pDT->uID,pDT->uIDType); 
				}
				break;
			case CTriggerData::o_summon_mine:
				{
					O_SUMMON_MINE* pSM = (O_SUMMON_MINE*)(pOperation->pParam); 
					pOP = new op_summon_mine(pSM->iMineID,pSM->iMineIDType,pSM->iMineNum,pSM->iMineNumType,
							pSM->iLife,pSM->iLifeType,pSM->iRange);
				}
				break;
			case CTriggerData::o_summon_npc:
				{
					O_SUMMON_NPC* pSN = (O_SUMMON_NPC*)(pOperation->pParam);
					pOP = new op_summon_npc(pSN->iNPCID,pSN->iNPCIDType,pSN->iNPCNum,pSN->iNPCNumType,
							pSN->iRange,pSN->iLife,pSN->iLifeType,pSN->iPathID,pSN->iPathIDType); 
				}
				break;
			case CTriggerData::o_deliver_random_task_in_region:
				{
					O_DELIVER_RANDOM_TASK_IN_REGION* pDR = (O_DELIVER_RANDOM_TASK_IN_REGION*)(pOperation->pParam);	
					pOP = new op_deliver_random_task_in_region(pDR->uID,pDR->zvMin.x,pDR->zvMin.z,
							pDR->zvMax.x,pDR->zvMax.z);
				}
				break;
			case CTriggerData::o_deliver_task_in_hate_list:
				{
					O_DELIVER_TASK_IN_HATE_LIST* pDT = (O_DELIVER_TASK_IN_HATE_LIST*)(pOperation->pParam);
					pOP = new op_deliver_task_in_dmglist(pDT->uID,pDT->uIDType,pDT->iRange,pDT->iPlayerNum); 
				}
				break;
			case CTriggerData::o_clear_tower_task_in_region:
				{
					O_CLEAR_TOWER_TASK_IN_REGION* pCT =(O_CLEAR_TOWER_TASK_IN_REGION*)(pOperation->pParam);
					pOP = new op_clear_tower_task_in_region(pCT->zvMin.x,pCT->zvMin.z,pCT->zvMax.x,pCT->zvMax.z);
				}
				break;

			default://...
				ASSERT(false);
				break;
		}
		
		if(pOP->RequireTarget())
		{
			pOP->SetTarget(tar);
		}
		else
		{
			delete tar;
		}
		return pOP;
		#undef OPARAM
	}

	trigger * ConvertTrigger(CPolicyData *pPolicyData, CTriggerData *pTriggerData)
	{
		CTriggerData::_s_tree_item* root = pTriggerData->GetConditonRoot();
		condition * cond = ConvertCondition(root);

		trigger * pTri = new trigger();
		pTri->SetData(pTriggerData->GetID(),cond);

		int numOperation = pTriggerData->GetOperaionNum();
		for( int j = 0; j < numOperation; ++j)
		{
			CTriggerData::_s_operation *pOperation = pTriggerData->GetOperaion(j);
			operation *pOP = ConvertOperation(pPolicyData,pOperation);
			pTri->AddOp(pOP);
		}
		return pTri;
	}
}

bool LoadAIPolicy(const char * path)
{
	CPolicyDataManager man;
	if(!man.Load(path))
	{
		printf("Failed to Load AIPolicy '%s'\n",path);
		return false;
	}
	int numPolicy = man.GetPolicyNum();
	for(int i = 0; i < numPolicy; i ++)
	{
		CPolicyData *pPolicyData = man.GetPolicy(i);
		policy * pPolicy = new policy(pPolicyData->GetID());

		//加入各个触发器
		int numTrigger = pPolicyData->GetTriggerPtrNum();
		for(int j = 0; j < numTrigger; j ++)
		{
			CTriggerData *pTriggerData = pPolicyData->GetTriggerPtr(j);
			if(pTriggerData->IsRun()) continue;
			trigger * pTrigger = ConvertTrigger(pPolicyData,pTriggerData);
			pTrigger->SetDefaultEnable(pTriggerData->IsActive());
			pTrigger->SetBattleEnable(pTriggerData->OnlyAttackValid());
			pPolicy->AddTrigger(pTrigger);
		}

		//将策略加入到管理器中
		world_manager::GetTriggerMan().AddPolicy(pPolicy);
		//printf("加入策略%d\n",pPolicyData->GetID());
	}
	man.Release();
	return true;
}

