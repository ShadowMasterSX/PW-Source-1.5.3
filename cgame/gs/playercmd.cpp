#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <common/protocol.h>
#include "world.h"
#include "player_imp.h"
#include "usermsg.h"
#include "clstab.h"
#include "actsession.h"
#include <common/protocol_imp.h>
#include "task/taskman.h"
#include "invincible_filter.h"
#include "sfilterdef.h"
#include "cooldowncfg.h"
#include "instance/battleground_manager.h"
#include <stocklib.h>
#include <sysauctionlib.h>
#include "invisible_filter.h"
#include <factionlib.h>
#include "dpsrankmanager.h"
#include "instance/countrybattle_ctrl.h"
#include "instance/countrybattle_manager.h"
#include "global_controller.h"
#include "uniquedataclient.h"
#include "gt_award_filter.h"
#include <mailsyslib.h>
#include <openssl/md5.h>
#include "moving_action.h"
/*
 *	这个文件里面是专门处理player命令的和controller消息的
 *
 */
#include "pathfinding/pathfinding.h"

void TestSearch(world * plane)
{
	path_finding::follow_target * _agent;
	path_finding::chase_info  _chase_info;
	for(size_t j = 0; j < 20000; j ++)
	{
		_agent = new path_finding::follow_target();
		_agent->CreateAgent(plane,0,NPC_MOVE_BEHAVIOR_CHASE);
		_agent->Start(A3DVECTOR(-34,218.858,4092.592),A3DVECTOR(-29.977,220.258,4110.783),5,0.8,300,&_chase_info);
		for(size_t i = 0; i < 10; i ++)
		{
			_agent->MoveOneStep(5);
		}

		_agent->Start(A3DVECTOR(-34,218.858,4092.592),A3DVECTOR(-29.977,220.258,4110.783),5,0.8,300,&_chase_info);
		for(size_t i = 0; i < 100; i ++)
		{
			_agent->MoveOneStep(5);
		}
		

		delete _agent;
	}
}

int
gplayer_controller::MessageHandler(world * pPlane, const MSG & msg)
{
	return 0;
}
void
gplayer_controller::ResurrectByItem(float exp_reduce, int param)
{
	gplayer_imp * pImp = ((gplayer_imp*)_imp);
	//判定是否能够进行复活操作
	if(!pImp->CanResurrect(param))
	{
		return ;
	}

	//需要判定复活物品是否存在
	int scroll_id = REVIVE_SCROLL_ID2;
	int scroll_index = pImp->_inventory.Find(0,REVIVE_SCROLL_ID2);
	if(scroll_index < 0)
	{
		scroll_id = REVIVE_SCROLL_ID;
		scroll_index = pImp->_inventory.Find(0,REVIVE_SCROLL_ID);
		if(scroll_index < 0)
		{
			error_cmd(S2C::ERR_ITEM_NOT_IN_INVENTORY);
			return;
		}
	}

	//判断冷却时间是否合适
	if(!pImp->CheckCoolDown(COOLDOWN_INDEX_SOUL_STONE))
	{
		error_cmd(S2C::ERR_OBJECT_IS_COOLING);
		return;
	}

	//从模板里取得物品的冷却时间
	int cooltime = world_manager::GetDataMan().get_cool_time(scroll_id);
	if(cooltime <0) cooltime = 0;
	
	//设置冷却时间
	pImp->SetCoolDown(COOLDOWN_INDEX_SOUL_STONE,cooltime);

	item& it = pImp->_inventory[scroll_index];
	pImp->UpdateMallConsumptionDestroying(it.type, it.proc_type, 1);
	
	pImp->_inventory.DecAmount(scroll_index,1);
	pImp->_runner->player_drop_item(gplayer_imp::IL_INVENTORY,scroll_index,scroll_id,1,S2C::DROP_TYPE_RESURRECT);

	//原地复活
	pImp->Resurrect(_imp->_parent->pos,true,exp_reduce,1,DEFAULT_RESURRECT_HP_FACTOR,DEFAULT_RESURRECT_MP_FACTOR,param);
}

void 
gplayer_controller::ResurrectInTown(float exp_reduce, int param)
{
	A3DVECTOR pos;
	gplayer_imp * pImp = ((gplayer_imp*)_imp);
	if(!pImp->CanResurrect(param))
	{
		return ;
	}
	int world_tag;
	if(!world_manager::GetInstance()->GetTownPosition(pImp,pImp->_parent->pos,pos,world_tag))
	{
		//应该使用默认复活点
		pos = _imp->_parent->pos;
		world_tag = world_manager::GetWorldTag();
	}
	//未来首先需要寻找城镇复生点的位置
	pImp->Resurrect(pos,false,exp_reduce,world_tag,DEFAULT_RESURRECT_HP_FACTOR,DEFAULT_RESURRECT_MP_FACTOR,param);

}

int 
gplayer_controller::UnLockInventoryHandler(int cmd_type,const void * buf, size_t size)
{
	switch(cmd_type)
	{
		case C2S::EXCHANGE_INVENTORY_ITEM:
		{
			C2S::CMD::exchange_inventory_item & eii = *(C2S::CMD::exchange_inventory_item*) buf;
			if(sizeof(eii) == size)
			{
				gplayer_imp * pImp = (gplayer_imp*)_imp;
				pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_INVENTORY, eii.index1);
				pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_INVENTORY, eii.index2);
			}
		}
		break;

		case C2S::MOVE_INVENTORY_ITEM:
		{
			C2S::CMD::move_inventory_item & mii = *(C2S::CMD::move_inventory_item *) buf;
			if(sizeof(mii) == size)
			{
				gplayer_imp * pImp = (gplayer_imp*)_imp;
				pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_INVENTORY, mii.src);
				pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_INVENTORY, mii.dest);
			}
		}
		break;

		case C2S::DROP_INVENTORY_ITEM:
		{
			C2S::CMD::drop_inventory_item& dii = *(C2S::CMD::drop_inventory_item*) buf;
			if(sizeof(dii) == size)
			{
				gplayer_imp *pImp = (gplayer_imp*)_imp;
				pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_INVENTORY, dii.index);
			}
		}
		break;
		
		case C2S::DROP_EQUIPMENT_ITEM:
		{
			C2S::CMD::drop_equipment_item & dei = *(C2S::CMD::drop_equipment_item*) buf;
			if(sizeof(dei) == size)
			{
				gplayer_imp *pImp = (gplayer_imp*)_imp;
				pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_EQUIPMENT, dei.index);
			}
		}
		break;

		case C2S::EXCHANGE_EQUIPMENT_ITEM:
		{
			C2S::CMD::exchange_equip_item & eei = *(C2S::CMD::exchange_equip_item*)buf;
			if(sizeof(eei) == size)
			{
				gplayer_imp * pImp = (gplayer_imp*)_imp;
				pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_EQUIPMENT, eei.idx1);
				pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_EQUIPMENT, eei.idx2);
			}
		}
		break;

		case C2S::EQUIP_ITEM:
		{
			C2S::CMD::equip_item & ei = *(C2S::CMD::equip_item*)buf;
			if(sizeof(ei) == size)
			{
				gplayer_imp * pImp = (gplayer_imp*)_imp;
				pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_INVENTORY, ei.idx_inv);
				pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_EQUIPMENT, ei.idx_eq);
			}
		}
		break;

		case C2S::MOVE_ITEM_TO_EQUIPMENT:
		{
			C2S::CMD::move_item_to_equipment & mite  = *(C2S::CMD::move_item_to_equipment*)buf;
			if(sizeof(mite) == size)
			{
				gplayer_imp * pImp = (gplayer_imp*)_imp;
				pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_INVENTORY, mite.idx_inv);
				pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_EQUIPMENT, mite.idx_eq);
			}
		}
		break;


		case C2S::RECHARGE_EQUIPPED_FLYSWORD:
		{
			C2S::CMD::recharge_equipped_flysword & ref = *(C2S::CMD::recharge_equipped_flysword *)buf;
			if(size == sizeof(C2S::CMD::recharge_equipped_flysword))
			{
				gplayer_imp * pImp = (gplayer_imp*)_imp;
				pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_INVENTORY, ref.element_index);
				pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_EQUIPMENT, item::EQUIP_INDEX_FLYSWORD);
			}
		}
		break;

		case C2S::RECHARGE_FLYSWORD:
		{
			C2S::CMD::recharge_flysword & rf = *(C2S::CMD::recharge_flysword *)buf;
			if(size == sizeof(C2S::CMD::recharge_flysword))
			{
				gplayer_imp * pImp = (gplayer_imp*)_imp;
				pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_INVENTORY, rf.element_index);
				pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_INVENTORY, rf.flysword_index);
			}
		}
		break;

		case C2S::EXCHANGE_TRASHBOX_ITEM:
		{
			C2S::CMD::exchange_trashbox_item & eti = *(C2S::CMD::exchange_trashbox_item*) buf;
			if(sizeof(eti) == size)
			{
				gplayer_imp * pImp = (gplayer_imp*)_imp;
				pImp->_runner->unlock_inventory_slot(eti.where, eti.index1);
				pImp->_runner->unlock_inventory_slot(eti.where, eti.index2);
			}
		}
		break;

		case C2S::MOVE_TRASHBOX_ITEM:
		{
			C2S::CMD::move_trashbox_item & mti = *(C2S::CMD::move_trashbox_item *) buf;
			if(sizeof(mti) == size)
			{
				gplayer_imp * pImp = (gplayer_imp*)_imp;
				pImp->_runner->unlock_inventory_slot(mti.where, mti.src);
				pImp->_runner->unlock_inventory_slot(mti.where, mti.dest);
			}
		}
		break;
		
		case C2S::EXHCANGE_TRASHBOX_INVENTORY:
		{
			C2S::CMD::exchange_trashbox_inventory & eti = *(C2S::CMD::exchange_trashbox_inventory *) buf;
			if(sizeof(eti) == size)
			{
				gplayer_imp * pImp = (gplayer_imp*)_imp;
				pImp->_runner->unlock_inventory_slot(eti.where, eti.idx_tra);
				pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_INVENTORY, eti.idx_inv);
			}
		}
		break;

		case C2S::MOVE_TRASHBOX_ITEM_TO_INVENTORY:
		{
			C2S::CMD::move_trashbox_item_to_inventory & mtiti = *(C2S::CMD::move_trashbox_item_to_inventory *) buf;
			if(sizeof(mtiti) == size)
			{
				gplayer_imp * pImp = (gplayer_imp*)_imp;
				pImp->_runner->unlock_inventory_slot(mtiti.where, mtiti.idx_tra);
				pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_INVENTORY, mtiti.idx_inv);
			}
		}
		break;
		
		case C2S::MOVE_INVENTORY_ITEM_TO_TRASHBOX:
		{
			C2S::CMD::move_inventory_item_to_trashbox & miitt  = *(C2S::CMD::move_inventory_item_to_trashbox *) buf;
			if(sizeof(miitt) == size)
			{
				gplayer_imp * pImp = (gplayer_imp*)_imp;
				pImp->_runner->unlock_inventory_slot(miitt.where, miitt.idx_tra);
				pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_INVENTORY, miitt.idx_inv);
			}
		}
		break;
		
		case C2S::DESTROY_ITEM:
		{
			C2S::CMD::destroy_item  & di   = *(C2S::CMD::destroy_item *) buf;
			if(size == sizeof(di))
			{
				gplayer_imp * pImp = (gplayer_imp*)_imp;
				pImp->_runner->unlock_inventory_slot(di.where, di.index);
			}
		}
		break;
		//lgc
		case C2S::ELF_ADD_ATTRIBUTE:
		{
			if(size == sizeof(C2S::CMD::elf_add_attribute))
			{       
				gplayer_imp * pImp = (gplayer_imp*)_imp;
				pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_EQUIPMENT, item::EQUIP_INDEX_ELF);
			}               
		}		
		break;

		case C2S::ELF_ADD_GENIUS:
		{
			if(size == sizeof(C2S::CMD::elf_add_genius))
			{       
				gplayer_imp * pImp = (gplayer_imp*)_imp;
				pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_EQUIPMENT, item::EQUIP_INDEX_ELF);
			}
		}
		break;
		
		case C2S::ELF_PLAYER_INSERT_EXP:
		{
			if(size == sizeof(C2S::CMD::elf_player_insert_exp))
			{       
				gplayer_imp * pImp = (gplayer_imp*)_imp;
				pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_EQUIPMENT, item::EQUIP_INDEX_ELF);
			}
		}
		break;

		case C2S::ELF_EQUIP_ITEM:
		{
			C2S::CMD::elf_equip_item & eei = *(C2S::CMD::elf_equip_item*)buf;
			if(size == sizeof(C2S::CMD::elf_equip_item))
			{       
				gplayer_imp * pImp = (gplayer_imp*)_imp;
				pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_INVENTORY, eei.index_inv);
				pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_EQUIPMENT, item::EQUIP_INDEX_ELF);
			}
		}
		break;

		case C2S::ELF_CHANGE_SECURE_STATUS:
		{
			if(size == sizeof(C2S::CMD::elf_change_secure_status))
			{       
				gplayer_imp * pImp = (gplayer_imp*)_imp;
				pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_EQUIPMENT, item::EQUIP_INDEX_ELF);
			}
		}
		break;
		
		case C2S::CAST_ELF_SKILL:
		{
			C2S::CMD::cast_elf_skill & ces = *(C2S::CMD::cast_elf_skill*)buf;
			if(size == sizeof(C2S::CMD::cast_elf_skill)+sizeof(int)*ces.target_count )
			{       
				gplayer_imp * pImp = (gplayer_imp*)_imp;
				pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_EQUIPMENT, item::EQUIP_INDEX_ELF);
			}
		}
		break;
		
		case C2S::RECHARGE_EQUIPPED_ELF:
		{
			C2S::CMD::recharge_equipped_elf & ree = *(C2S::CMD::recharge_equipped_elf *)buf;
			if(size == sizeof(C2S::CMD::recharge_equipped_elf))
			{
				gplayer_imp * pImp = (gplayer_imp*)_imp;
				pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_INVENTORY, ree.element_index);
				pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_EQUIPMENT, item::EQUIP_INDEX_ELF);
			}
		}
		break;
		
		case C2S::EQUIP_TRASHBOX_FASHION_ITEM:
		{
			C2S::CMD::equip_trashbox_fashion_item & etfi = *(C2S::CMD::equip_trashbox_fashion_item*)buf;
			if(size == sizeof(C2S::CMD::equip_trashbox_fashion_item))
			{
				gplayer_imp * pImp = (gplayer_imp*)_imp;
				pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_TRASH_BOX3, etfi.trash_idx_body);
				pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_TRASH_BOX3, etfi.trash_idx_leg);
				pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_TRASH_BOX3, etfi.trash_idx_foot);
				pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_TRASH_BOX3, etfi.trash_idx_wrist);
				pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_TRASH_BOX3, etfi.trash_idx_head);
				pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_TRASH_BOX3, etfi.trash_idx_weapon);
				
				pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_EQUIPMENT, item::EQUIP_INDEX_FASHION_BODY);
				pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_EQUIPMENT, item::EQUIP_INDEX_FASHION_LEG);
				pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_EQUIPMENT, item::EQUIP_INDEX_FASHION_FOOT);
				pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_EQUIPMENT, item::EQUIP_INDEX_FASHION_WRIST);
				pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_EQUIPMENT, item::EQUIP_INDEX_FASHION_HEAD);
				pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_EQUIPMENT, item::EQUIP_INDEX_FASHION_WEAPON);
			}
		}
		break;

		case C2S::EQUIP_TRASHBOX_ITEM:
		{
			C2S::CMD::equip_trashbox_item & cmd = *(C2S::CMD::equip_trashbox_item*)buf;
			if(size == sizeof(C2S::CMD::equip_trashbox_item))
			{
				gplayer_imp * pImp = (gplayer_imp*)_imp;
				pImp->_runner->unlock_inventory_slot(cmd.where, cmd.trash_idx);
				pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_EQUIPMENT, cmd.equip_idx);
			}
		}
		break;
	}
	return 0;
}



int 
gplayer_controller::MarketCommandHandler(int cmd_type,const void * buf, size_t size)
{
	if(!CheckBanish()) return 0;
	switch(cmd_type)
	{
		case C2S::GET_ITEM_INFO:
		case C2S::GET_INVENTORY:
		case C2S::GET_INVENTORY_DETAIL:
		case C2S::SELF_GET_PROPERTY:
		case C2S::TEAM_INVITE:
		case C2S::TEAM_AGREE_INVITE:
		case C2S::TEAM_REJECT_INVITE:
		case C2S::TEAM_LEAVE_PARTY:
		case C2S::TEAM_DISMISS_PARTY:
		case C2S::TEAM_KICK_MEMBER:
		case C2S::TEAM_GET_TEAMMATE_POS:
		case C2S::CHANGE_PICKUP_FLAG:
		case C2S::GET_OTHERS_EQUIPMENT:
		case C2S::GET_OWN_WEALTH:
		case C2S::GET_ALL_DATA:
		case C2S::GET_WALLOW_INFO:
		case C2S::RECHARGE_EQUIPPED_FLYSWORD:
		case C2S::RECHARGE_FLYSWORD:
		case C2S::SET_STATUS_POINT:
		case C2S::UNSELECT:
		case C2S::SELECT_TARGET:
		case C2S::EXCHANGE_INVENTORY_ITEM:
		case C2S::MOVE_INVENTORY_ITEM:
		case C2S::DROP_INVENTORY_ITEM:
		case C2S::EXCHANGE_EQUIPMENT_ITEM:
		case C2S::MOVE_ITEM_TO_EQUIPMENT:
		case C2S::ELF_ADD_ATTRIBUTE:		//lgc
		case C2S::ELF_ADD_GENIUS:
		case C2S::ELF_PLAYER_INSERT_EXP:
		case C2S::ELF_EQUIP_ITEM:
		case C2S::ELF_CHANGE_SECURE_STATUS:
		case C2S::RECHARGE_EQUIPPED_ELF:
		case C2S::GET_MALL_ITEM_PRICE:
		case C2S::CHECK_SECURITY_PASSWD:
		case C2S::GET_DIVIDEND_MALL_ITEM_PRICE:
		case C2S::CHOOSE_MULTI_EXP:
		case C2S::TOGGLE_MULTI_EXP:
		case C2S::MULTI_EXCHANGE_ITEM:
		case C2S::SYSAUCTION_OP:
		case C2S::CALC_NETWORK_DELAY:
		case C2S::GET_FACTION_FORTRESS_INFO:
		case C2S::CONGREGATE_REPLY:
		case C2S::GET_FORCE_GLOBAL_DATA:
		case C2S::COUNTRYBATTLE_GET_PERSONAL_SCORE:
		case C2S::GET_SERVER_TIMESTAMP:
		case C2S::TOGGLE_ONLINE_AWARD:
		case C2S::GET_CASH_MONEY_EXCHG_RATE:
		case C2S::COUNTRYBATTLE_GET_STRONGHOLD_STATE:
		case C2S::QUERY_PARALLEL_WORLD:
		case C2S::GET_REINCARNATION_TOME:
		case C2S::REWRITE_REINCARNATION_TOME:
		case C2S::ACTIVATE_REINCARNATION_TOME:
		case C2S::AUTO_TEAM_SET_GOAL:
		case C2S::AUTO_TEAM_JUMP_TO_GOAL:
		case C2S::EQUIP_TRASHBOX_FASHION_ITEM:
		case C2S::EQUIP_TRASHBOX_ITEM:
        case C2S::ACTIVATE_REGION_WAYPOINTS:
			return CommandHandler(cmd_type,buf,size);
			
		case C2S::EQUIP_ITEM:	//摆摊过程中不允许替换装备卸下摆摊凭证
		{
			C2S::CMD::equip_item & ei = *(C2S::CMD::equip_item*)buf;
			if(sizeof(ei) != size)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			if(ei.idx_eq == item::EQUIP_INDEX_STALLCARD)
				return UnLockInventoryHandler(cmd_type,buf,size);
			else
				return CommandHandler(cmd_type,buf,size);
		}
			
		case C2S::DROP_EQUIPMENT_ITEM:
		{
			C2S::CMD::drop_equipment_item & dei = *(C2S::CMD::drop_equipment_item*) buf;
			if(sizeof(dei) != size)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			if(dei.index == item::EQUIP_INDEX_STALLCARD)
				return UnLockInventoryHandler(cmd_type,buf,size);
			else
				return CommandHandler(cmd_type,buf,size);
		}

		case C2S::CAST_ELF_SKILL:		//lgc
			return UnLockInventoryHandler(cmd_type,buf,size);

		case C2S::CANCEL_PERSONAL_MARKET:
		{
			gplayer_imp * pImp= (gplayer_imp *) _imp;
			pImp->CancelPersonalMarket();
		}
			return 0;

		case C2S::LOGOUT:
		{
			gplayer_imp * pImp= (gplayer_imp *) _imp;
			pImp->CancelPersonalMarket();
		}
			return CommandHandler(cmd_type,buf,size);

		case C2S::EXCHANGE_WANMEI_YINPIAO:
		{
			C2S::CMD::exchange_wanmei_yinpiao & ewy = *(C2S::CMD::exchange_wanmei_yinpiao *)buf;
			if(size != sizeof(ewy))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp= (gplayer_imp *) _imp;
			pImp->PlayerExchangeWanmeiYinpiao(ewy.is_sell, ewy.count);
		}
		break;
			
		default:
		if(cmd_type == C2S::GOTO || cmd_type > C2S::GM_COMMAND_START && cmd_type < C2S::GM_COMMAND_END)
		{
			return GMCommandHandler(cmd_type,buf,size);
		}
		else
		{
			if(_debug_command_enable)
			{
				return DebugCommandHandler(cmd_type,buf,size);
			}
		}
	}
	return 0;
}

int
gplayer_controller::CosmeticCommandHandler(int cmd_type,const void * buf, size_t size)
{	
	if(!CheckBanish()) return 0;
//	gplayer_imp * pPlayer = (gplayer_imp*)_imp;
	switch(cmd_type)
	{
		case C2S::GET_ITEM_INFO:
		case C2S::GET_INVENTORY:
		case C2S::GET_INVENTORY_DETAIL:
		case C2S::SELF_GET_PROPERTY:
		case C2S::TEAM_INVITE:
		case C2S::TEAM_AGREE_INVITE:
		case C2S::TEAM_REJECT_INVITE:
		case C2S::TEAM_LEAVE_PARTY:
		case C2S::TEAM_DISMISS_PARTY:
		case C2S::TEAM_KICK_MEMBER:
		case C2S::TEAM_GET_TEAMMATE_POS:
		case C2S::CHANGE_PICKUP_FLAG:
		case C2S::GET_OTHERS_EQUIPMENT:
		case C2S::GET_OWN_WEALTH:
		case C2S::GET_ALL_DATA:
		case C2S::GET_WALLOW_INFO:
		case C2S::CANCEL_ACTION:
		case C2S::LOGOUT:
		case C2S::GET_MALL_ITEM_PRICE:
		case C2S::CHECK_SECURITY_PASSWD:
		case C2S::GET_DIVIDEND_MALL_ITEM_PRICE:
		case C2S::CHOOSE_MULTI_EXP:
		case C2S::TOGGLE_MULTI_EXP:
		case C2S::AUTO_TEAM_SET_GOAL:
        case C2S::ACTIVATE_REGION_WAYPOINTS:
			return CommandHandler(cmd_type,buf,size);

		case C2S::EXCHANGE_INVENTORY_ITEM:
		case C2S::MOVE_INVENTORY_ITEM:
		case C2S::DROP_INVENTORY_ITEM:
		case C2S::DROP_EQUIPMENT_ITEM:
		case C2S::EXCHANGE_EQUIPMENT_ITEM:
		case C2S::EQUIP_ITEM:
		case C2S::MOVE_ITEM_TO_EQUIPMENT:
		case C2S::EXCHANGE_TRASHBOX_ITEM:
		case C2S::MOVE_TRASHBOX_ITEM:
		case C2S::EXHCANGE_TRASHBOX_INVENTORY:
		case C2S::MOVE_TRASHBOX_ITEM_TO_INVENTORY:
		case C2S::MOVE_INVENTORY_ITEM_TO_TRASHBOX:
		case C2S::DESTROY_ITEM:
		case C2S::RECHARGE_EQUIPPED_FLYSWORD:
		case C2S::RECHARGE_FLYSWORD:
		case C2S::ELF_ADD_ATTRIBUTE:		//lgc
		case C2S::ELF_ADD_GENIUS:
		case C2S::ELF_PLAYER_INSERT_EXP:
		case C2S::ELF_EQUIP_ITEM:
		case C2S::ELF_CHANGE_SECURE_STATUS:
		case C2S::CAST_ELF_SKILL:
		case C2S::RECHARGE_EQUIPPED_ELF:
		case C2S::EQUIP_TRASHBOX_FASHION_ITEM:
		case C2S::EQUIP_TRASHBOX_ITEM:
			return UnLockInventoryHandler(cmd_type,buf,size);
	}
	return 0;
}

int
gplayer_controller::ZombieCommandHandler(int cmd_type,const void * buf, size_t size)
{	
	if(!CheckBanish()) return 0;
	switch(cmd_type)
	{
		case C2S::RESURRECT_IN_TOWN:
			_load_stats ++;
			{
				C2S::CMD::resurrect & cmd = *(C2S::CMD::resurrect *)buf;
				if(size != sizeof(cmd)) break;
				gplayer_imp * pImp= (gplayer_imp *) _imp;
				session_resurrect_in_town *pSession= new session_resurrect_in_town(pImp,cmd.param,39);
				float exp_reduce = player_template::GetResurrectExpReduce(pImp->_basic.sec_level);
				if(pImp->_basic.level <= LOW_PROTECT_LEVEL) exp_reduce = 0.f; 
				pSession->SetExpReduce(exp_reduce);
				if(pImp->AddSession(pSession)) pImp->StartSession();
			}
			break;

		case C2S::RESURRECT_BY_ITEM:
			_load_stats ++;
			{
				C2S::CMD::resurrect & cmd = *(C2S::CMD::resurrect *)buf;
				if(size != sizeof(cmd)) break;
				gplayer_imp * pImp= (gplayer_imp *) _imp;
				session_resurrect_by_item *pSession= new session_resurrect_by_item(pImp,cmd.param,99);
				float exp_reduce = player_template::GetResurrectExpReduce(pImp->_basic.sec_level);
				if(pImp->_basic.level <= LOW_PROTECT_LEVEL) exp_reduce = 0.f; 
				pSession->SetExpReduce(exp_reduce);
				if(pImp->AddSession(pSession)) pImp->StartSession();
			}
			break;

		case C2S::RESURRECT_AT_ONCE:
			{
				C2S::CMD::resurrect & cmd = *(C2S::CMD::resurrect *)buf;
				if(size != sizeof(cmd)) break;
				gplayer_imp * pImp= (gplayer_imp *) _imp;
				if(!pImp->CanResurrect(cmd.param))
				{
					break;
				}
				float exp_reduce, hp_factor, mp_factor;
				if(!pImp->GetResurrectState(exp_reduce, hp_factor, mp_factor))
				{
					break;
				}

				float base_exp_reduce = player_template::GetResurrectExpReduce(pImp->_basic.sec_level);

				//现在原来的exp_reduce变成比率了，但是传入的数值仍然是0.0~0.05所以进行一下换算
				exp_reduce = (exp_reduce /0.05f) * base_exp_reduce;
				if(pImp->_basic.level <= LOW_PROTECT_LEVEL) exp_reduce = 0.f; 

				//还需要检查一下世界的经验值比率
				pImp->Resurrect(pImp->_parent->pos,true,exp_reduce,1,hp_factor,mp_factor,cmd.param);
			}
			break;

		//下面是普通模式和死亡模式下都处理的命令
		case C2S::UNSELECT:
		case C2S::SELECT_TARGET:
		case C2S::GET_ITEM_INFO:
		case C2S::GET_INVENTORY:
		case C2S::GET_INVENTORY_DETAIL:
		case C2S::SELF_GET_PROPERTY:
		case C2S::TEAM_INVITE:
		case C2S::TEAM_AGREE_INVITE:
		case C2S::TEAM_REJECT_INVITE:
		case C2S::TEAM_LEAVE_PARTY:
		case C2S::TEAM_DISMISS_PARTY:
		case C2S::TEAM_KICK_MEMBER:
		case C2S::TEAM_GET_TEAMMATE_POS:
		case C2S::CHANGE_PICKUP_FLAG:
		case C2S::LOGOUT:
		case C2S::GET_OTHERS_EQUIPMENT:
		case C2S::GET_OWN_WEALTH:
		case C2S::GET_ALL_DATA:
		case C2S::GET_WALLOW_INFO:
		case C2S::GET_MALL_ITEM_PRICE:
		case C2S::CHECK_SECURITY_PASSWD:
		case C2S::GET_DIVIDEND_MALL_ITEM_PRICE:
		case C2S::CHOOSE_MULTI_EXP:
		case C2S::TOGGLE_MULTI_EXP:
		case C2S::SYSAUCTION_OP:
		case C2S::CALC_NETWORK_DELAY:
		case C2S::GET_FACTION_FORTRESS_INFO:
		case C2S::GET_FORCE_GLOBAL_DATA:
		case C2S::COUNTRYBATTLE_GET_PERSONAL_SCORE:
		case C2S::GET_SERVER_TIMESTAMP:
		case C2S::GET_CASH_MONEY_EXCHG_RATE:
		case C2S::COUNTRYBATTLE_GET_STRONGHOLD_STATE:
		case C2S::QUERY_TOUCH_POINT:	
		case C2S::COST_TOUCH_POINT:
		case C2S::QUERY_TITLE:
		case C2S::CHANGE_CURR_TITLE:
		case C2S::DAILY_SIGNIN:
		case C2S::LATE_SIGNIN:
		case C2S::APPLY_SIGNIN_AWARD:
		case C2S::REFRESH_SIGNIN:
		case C2S::QUERY_PARALLEL_WORLD:
		case C2S::QUERY_UNIQUE_DATA:
		case C2S::GET_REINCARNATION_TOME:
		case C2S::REWRITE_REINCARNATION_TOME:
		case C2S::ACTIVATE_REINCARNATION_TOME:
		case C2S::AUTO_TEAM_SET_GOAL:
		case C2S::QUERY_TRICKBATTLE_CHARIOTS:
		case C2S::RANDOM_MALL_SHOPPING:
		case C2S::QUERY_MAFIA_PVP_INFO:	
        case C2S::ACTIVATE_REGION_WAYPOINTS:
			return CommandHandler(cmd_type,buf,size);

		case C2S::EXCHANGE_INVENTORY_ITEM:
		case C2S::MOVE_INVENTORY_ITEM:
		case C2S::DROP_INVENTORY_ITEM:
		case C2S::DROP_EQUIPMENT_ITEM:
		case C2S::EXCHANGE_EQUIPMENT_ITEM:
		case C2S::EQUIP_ITEM:
		case C2S::MOVE_ITEM_TO_EQUIPMENT:
		case C2S::EXCHANGE_TRASHBOX_ITEM:
		case C2S::MOVE_TRASHBOX_ITEM:
		case C2S::EXHCANGE_TRASHBOX_INVENTORY:
		case C2S::MOVE_TRASHBOX_ITEM_TO_INVENTORY:
		case C2S::MOVE_INVENTORY_ITEM_TO_TRASHBOX:
		case C2S::DESTROY_ITEM:
		case C2S::RECHARGE_EQUIPPED_FLYSWORD:
		case C2S::RECHARGE_FLYSWORD:
		case C2S::ELF_ADD_ATTRIBUTE:		//lgc
		case C2S::ELF_ADD_GENIUS:
		case C2S::ELF_PLAYER_INSERT_EXP:
		case C2S::ELF_EQUIP_ITEM:
		case C2S::ELF_CHANGE_SECURE_STATUS:
		case C2S::CAST_ELF_SKILL:
		case C2S::RECHARGE_EQUIPPED_ELF:
		case C2S::EQUIP_TRASHBOX_FASHION_ITEM:
		case C2S::EQUIP_TRASHBOX_ITEM:
			error_cmd(S2C::ERR_COMMAND_IN_ZOMBIE);
			return UnLockInventoryHandler(cmd_type,buf,size);

		default:
		if(cmd_type == C2S::GOTO || cmd_type > C2S::GM_COMMAND_START && cmd_type < C2S::GM_COMMAND_END)
		{
			return GMCommandHandler(cmd_type,buf,size);
		}
		else
		{
			error_cmd(S2C::ERR_COMMAND_IN_ZOMBIE);
			if(_debug_command_enable)
			{
				return DebugCommandHandler(cmd_type,buf,size);
			}
		}
	}
	return 0;
}

int
gplayer_controller::StayInCommandHandler(int cmd_type,const void * buf, size_t size)
{	
	if(!CheckBanish()) return 0;
	switch(cmd_type)
	{
		case C2S::GET_ITEM_INFO:
		case C2S::GET_INVENTORY:
		case C2S::GET_INVENTORY_DETAIL:
		case C2S::SELF_GET_PROPERTY:
		case C2S::TEAM_INVITE:
		case C2S::TEAM_AGREE_INVITE:
		case C2S::TEAM_REJECT_INVITE:
		case C2S::TEAM_LEAVE_PARTY:
		case C2S::TEAM_DISMISS_PARTY:
		case C2S::TEAM_KICK_MEMBER:
		case C2S::TEAM_GET_TEAMMATE_POS:
		case C2S::CHANGE_PICKUP_FLAG:
		case C2S::LOGOUT:
		case C2S::GET_OTHERS_EQUIPMENT:
		case C2S::GET_OWN_WEALTH:
		case C2S::GET_ALL_DATA:
		case C2S::GET_WALLOW_INFO:
		case C2S::RECHARGE_EQUIPPED_FLYSWORD:
		case C2S::RECHARGE_FLYSWORD:
		case C2S::EXCHANGE_INVENTORY_ITEM:
		case C2S::MOVE_INVENTORY_ITEM:
		case C2S::SET_STATUS_POINT:
		case C2S::UNSELECT:
		case C2S::SELECT_TARGET:
		case C2S::DROP_EQUIPMENT_ITEM:
		case C2S::EXCHANGE_EQUIPMENT_ITEM:
		case C2S::DROP_INVENTORY_ITEM:
		case C2S::EQUIP_ITEM:
		case C2S::MOVE_ITEM_TO_EQUIPMENT:
		case C2S::EXCHANGE_TRASHBOX_ITEM:
		case C2S::MOVE_TRASHBOX_ITEM:
		case C2S::EXHCANGE_TRASHBOX_INVENTORY:
		case C2S::MOVE_TRASHBOX_ITEM_TO_INVENTORY:
		case C2S::MOVE_INVENTORY_ITEM_TO_TRASHBOX:
		case C2S::DESTROY_ITEM:
		case C2S::TASK_NOTIFY:
		case C2S::PET_CTRL_CMD:
		case C2S::ELF_ADD_ATTRIBUTE:		//lgc
		case C2S::ELF_ADD_GENIUS:
		case C2S::ELF_PLAYER_INSERT_EXP:
		case C2S::ELF_EQUIP_ITEM:
		case C2S::ELF_CHANGE_SECURE_STATUS:
		case C2S::RECHARGE_EQUIPPED_ELF:
		case C2S::GET_MALL_ITEM_PRICE:
		case C2S::EQUIP_TRASHBOX_FASHION_ITEM:
		case C2S::CHECK_SECURITY_PASSWD:
		case C2S::GET_DIVIDEND_MALL_ITEM_PRICE:
		case C2S::CHOOSE_MULTI_EXP:
		case C2S::TOGGLE_MULTI_EXP:
		case C2S::MULTI_EXCHANGE_ITEM:
		case C2S::SYSAUCTION_OP:
		case C2S::CALC_NETWORK_DELAY:
		case C2S::GET_FACTION_FORTRESS_INFO:
		case C2S::CONGREGATE_REPLY:
		case C2S::GET_FORCE_GLOBAL_DATA:
		case C2S::RECHARGE_ONLINE_AWARD:
		case C2S::TOGGLE_ONLINE_AWARD:
		case C2S::COUNTRYBATTLE_GET_PERSONAL_SCORE:
		case C2S::GET_SERVER_TIMESTAMP:
		case C2S::COUNTRYBATTLE_LEAVE:
		case C2S::GET_CASH_MONEY_EXCHG_RATE:
		case C2S::COUNTRYBATTLE_GET_STRONGHOLD_STATE:
		case C2S::QUERY_TOUCH_POINT:	
		case C2S::COST_TOUCH_POINT:	
		case C2S::QUERY_TITLE:
		case C2S::CHANGE_CURR_TITLE:
		case C2S::DAILY_SIGNIN:
		case C2S::LATE_SIGNIN:
		case C2S::APPLY_SIGNIN_AWARD:
		case C2S::REFRESH_SIGNIN:
		case C2S::SWITCH_IN_PARALLEL_WORLD:
		case C2S::QUERY_PARALLEL_WORLD:
		case C2S::QUERY_UNIQUE_DATA:
		case C2S::GET_REINCARNATION_TOME:
		case C2S::REWRITE_REINCARNATION_TOME:
		case C2S::ACTIVATE_REINCARNATION_TOME:
		case C2S::AUTO_TEAM_SET_GOAL:
		case C2S::AUTO_TEAM_JUMP_TO_GOAL:
		case C2S::TRICKBATTLE_LEAVE:
		case C2S::SWALLOW_GENERALCARD:
		case C2S::EQUIP_TRASHBOX_ITEM:
		case C2S::QUERY_TRICKBATTLE_CHARIOTS:
		case C2S::RANDOM_MALL_SHOPPING:
		case C2S::QUERY_MAFIA_PVP_INFO:	
        case C2S::ACTIVATE_REGION_WAYPOINTS:
		case C2S::INSTANCE_REENTER_REQUEST:	
			return CommandHandler(cmd_type,buf,size);
	
		case C2S::CAST_ELF_SKILL:			//lgc
			return UnLockInventoryHandler(cmd_type,buf,size);
			
		case C2S::USE_ITEM:
		if(CheckDeny(CMD_USE_ITEM)) return 0;
		{
			C2S::CMD::use_item & ui = *(C2S::CMD::use_item *)buf;
			if(size != sizeof(C2S::CMD::use_item)) 
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			if(world_manager::GetGlobalController().CheckServerForbid(SERVER_FORBID_ITEM,ui.item_id))
			{
				error_cmd(S2C::ERR_CANNOT_USE_ITEM);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			if(!pImp->PlayerSitDownUseItem(ui.where, ui.index, ui.item_id,ui.count))
			{
				//改在物品里面发送这个错误信息了
				//error_cmd(S2C::ERR_CANNOT_USE_ITEM);
			}
		}
		break;

		case C2S::CANCEL_ACTION:	//坐下时，cancel action等同于站起
		case C2S::STAND_UP:
		{
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->PlayerStandUp();
		}
		break;
		default:
		if(cmd_type == C2S::GOTO || cmd_type > C2S::GM_COMMAND_START && cmd_type < C2S::GM_COMMAND_END)
		{
			return GMCommandHandler(cmd_type,buf,size);
		}
		else
		{
			if(_debug_command_enable)
			{
				return DebugCommandHandler(cmd_type,buf,size);
			}
		}
	}
	return 0;
}

int
gplayer_controller::BoundCommandHandler(int cmd_type,const void * buf, size_t size)
{	
	if(!CheckBanish()) return 0;
	switch(cmd_type)
	{
		case C2S::NORMAL_ATTACK:
		case C2S::CAST_SKILL:
		case C2S::CAST_INSTANT_SKILL:
		case C2S::CAST_POS_SKILL:
		case C2S::USE_ITEM_WITH_TARGET:
//		case C2S::PICKUP: 
//		case C2S::GATHER_MATERIAL:
		case C2S::SIT_DOWN:
		case C2S::SERVICE_HELLO:
		case C2S::SERVICE_GET_CONTENT:
		case C2S::SERVICE_SERVE:
		case C2S::OPEN_PERSONAL_MARKET:
		return 0;

		case C2S::CAST_ELF_SKILL:			//lgc
			return UnLockInventoryHandler(cmd_type,buf,size);

		case C2S::BIND_PLAYER_CANCEL:
		//要求取消绑定状态
		{
			gplayer_imp * pImp = (gplayer_imp *)_imp;
			pImp->PlayerBindCancel();
		}
		return 0;
	}
	return CommandHandler(cmd_type,buf,size);
}

int
gplayer_controller::TravelCommandHandler(int cmd_type,const void * buf, size_t size)
{	
	if(!CheckBanish()) return 0;
	switch(cmd_type)
	{
		case C2S::GET_ITEM_INFO:
		case C2S::GET_INVENTORY:
		case C2S::GET_INVENTORY_DETAIL:
		case C2S::SELF_GET_PROPERTY:
		case C2S::TEAM_INVITE:
		case C2S::TEAM_AGREE_INVITE:
		case C2S::TEAM_REJECT_INVITE:
		case C2S::TEAM_LEAVE_PARTY:
		case C2S::TEAM_DISMISS_PARTY:
		case C2S::TEAM_KICK_MEMBER:
		case C2S::TEAM_GET_TEAMMATE_POS:
		case C2S::CHANGE_PICKUP_FLAG:
		case C2S::GET_OTHERS_EQUIPMENT:
		case C2S::GET_OWN_WEALTH:
		case C2S::GET_ALL_DATA:
		case C2S::GET_WALLOW_INFO:
		case C2S::RECHARGE_EQUIPPED_FLYSWORD:
		case C2S::RECHARGE_FLYSWORD:
		case C2S::EXCHANGE_INVENTORY_ITEM:
		case C2S::MOVE_INVENTORY_ITEM:
		case C2S::SET_STATUS_POINT:
		case C2S::UNSELECT:
		case C2S::SELECT_TARGET:
		case C2S::EXCHANGE_EQUIPMENT_ITEM:
		case C2S::EQUIP_ITEM:
		case C2S::MOVE_ITEM_TO_EQUIPMENT:
		case C2S::EXCHANGE_TRASHBOX_ITEM:
		case C2S::MOVE_TRASHBOX_ITEM:
		case C2S::EXHCANGE_TRASHBOX_INVENTORY:
		case C2S::MOVE_TRASHBOX_ITEM_TO_INVENTORY:
		case C2S::MOVE_INVENTORY_ITEM_TO_TRASHBOX:
		case C2S::DESTROY_ITEM:
		case C2S::AUTO_TEAM_SET_GOAL:
			return CommandHandler(cmd_type,buf,size);

		case C2S::PLAYER_MOVE:
		case C2S::STOP_MOVE:
			return CommandHandler(cmd_type,buf,size);

		case C2S::COMPLETE_TRAVEL:
		{
			gplayer_imp * pImp = (gplayer_imp *)_imp;
			session_complete_travel * pSession = new session_complete_travel(pImp);
			if(pImp->AddSession(pSession)) pImp->StartSession();
		}
		return 0;

		default:
		if(cmd_type == C2S::GOTO || cmd_type > C2S::GM_COMMAND_START && cmd_type < C2S::GM_COMMAND_END)
		{
			return GMCommandHandler(cmd_type,buf,size);
		}
	}
	return 0;
}

int
gplayer_controller::SealedCommandHandler(int cmd_type,const void * buf, size_t size)
{	
	if(!CheckBanish()) return 0;
	gplayer_imp * pPlayer = (gplayer_imp*)_imp;
	if(pPlayer->_idle_mode_flag ||
			((gactive_imp::SEAL_MODE_ROOT|gactive_imp::SEAL_MODE_SILENT) == (pPlayer->_seal_mode_flag & (gactive_imp::SEAL_MODE_ROOT|gactive_imp::SEAL_MODE_SILENT))) )
	{
		switch(cmd_type)
		{
			case C2S::UNSELECT:
			case C2S::SELECT_TARGET:
			case C2S::GET_ITEM_INFO:
			case C2S::GET_INVENTORY:
			case C2S::GET_INVENTORY_DETAIL:
			case C2S::SELF_GET_PROPERTY:
			case C2S::TEAM_INVITE:
			case C2S::TEAM_AGREE_INVITE:
			case C2S::TEAM_REJECT_INVITE:
			case C2S::TEAM_LEAVE_PARTY:
			case C2S::TEAM_DISMISS_PARTY:
			case C2S::TEAM_KICK_MEMBER:
			case C2S::TEAM_GET_TEAMMATE_POS:
			case C2S::CHANGE_PICKUP_FLAG:
			case C2S::GET_OTHERS_EQUIPMENT:
			case C2S::GET_OWN_WEALTH:
			case C2S::GET_ALL_DATA:
			case C2S::GET_WALLOW_INFO:
			case C2S::PET_CTRL_CMD:
			case C2S::CAST_ELF_SKILL:			//lgc
			case C2S::GET_MALL_ITEM_PRICE:
			case C2S::CHECK_SECURITY_PASSWD:
			case C2S::GET_DIVIDEND_MALL_ITEM_PRICE:
			case C2S::CHOOSE_MULTI_EXP:
			case C2S::TOGGLE_MULTI_EXP:
			case C2S::SYSAUCTION_OP:
			case C2S::CALC_NETWORK_DELAY:
			case C2S::GET_FACTION_FORTRESS_INFO:
			case C2S::GET_FORCE_GLOBAL_DATA:
			case C2S::RECHARGE_ONLINE_AWARD:
			case C2S::TOGGLE_ONLINE_AWARD:
			case C2S::COUNTRYBATTLE_GET_PERSONAL_SCORE:
			case C2S::GET_SERVER_TIMESTAMP:
			case C2S::COUNTRYBATTLE_LEAVE:
			case C2S::GET_CASH_MONEY_EXCHG_RATE:
			case C2S::COUNTRYBATTLE_GET_STRONGHOLD_STATE:
			case C2S::QUERY_PARALLEL_WORLD:
			case C2S::GET_REINCARNATION_TOME:
			case C2S::REWRITE_REINCARNATION_TOME:
			case C2S::ACTIVATE_REINCARNATION_TOME:
			case C2S::AUTO_TEAM_SET_GOAL:
			case C2S::TRICKBATTLE_LEAVE:
			case C2S::QUERY_TRICKBATTLE_CHARIOTS:
            case C2S::QUERY_CAN_INHERIT_ADDONS:
            case C2S::ACTIVATE_REGION_WAYPOINTS:
				return CommandHandler(cmd_type,buf,size);

			case C2S::EXCHANGE_INVENTORY_ITEM:
			case C2S::MOVE_INVENTORY_ITEM:
			case C2S::DROP_INVENTORY_ITEM:
			case C2S::DROP_EQUIPMENT_ITEM:
			case C2S::EXCHANGE_EQUIPMENT_ITEM:
			case C2S::EQUIP_ITEM:
			case C2S::MOVE_ITEM_TO_EQUIPMENT:
			case C2S::EXCHANGE_TRASHBOX_ITEM:
			case C2S::MOVE_TRASHBOX_ITEM:
			case C2S::EXHCANGE_TRASHBOX_INVENTORY:
			case C2S::MOVE_TRASHBOX_ITEM_TO_INVENTORY:
			case C2S::MOVE_INVENTORY_ITEM_TO_TRASHBOX:
			case C2S::DESTROY_ITEM:
			case C2S::RECHARGE_EQUIPPED_FLYSWORD:
			case C2S::RECHARGE_FLYSWORD:
			case C2S::ELF_ADD_ATTRIBUTE:		//lgc
			case C2S::ELF_ADD_GENIUS:
			case C2S::ELF_PLAYER_INSERT_EXP:
			case C2S::ELF_EQUIP_ITEM:
			case C2S::ELF_CHANGE_SECURE_STATUS:
			case C2S::RECHARGE_EQUIPPED_ELF:
			case C2S::EQUIP_TRASHBOX_FASHION_ITEM:
			case C2S::EQUIP_TRASHBOX_ITEM:
				return UnLockInventoryHandler(cmd_type,buf,size);
			default: 
				return 0;
		}
	}

	if(pPlayer->_seal_mode_flag & gactive_imp::SEAL_MODE_ROOT)
	{
		//定身
		switch(cmd_type)
		{
			case C2S::PLAYER_MOVE:
			case C2S::STOP_MOVE:
			case C2S::PICKUP: 
			case C2S::GOTO:
			case C2S::SERVICE_HELLO:
			case C2S::SERVICE_GET_CONTENT:
			case C2S::SERVICE_SERVE:
			case C2S::LOGOUT:
			case C2S::SIT_DOWN:
			case C2S::CONGREGATE_REPLY:
			//这些指令无法使用
			return 0;

			
			//使用物品的限制由物品本身完成

			default:
			//如果没有被沉默，那么直接调用缺省的函数 
			return CommandHandler(cmd_type,buf,size);

			break;
		}
	}
	if(pPlayer->_seal_mode_flag & gactive_imp::SEAL_MODE_SILENT)
	{
		//不能攻击
		switch(cmd_type)
		{
			case C2S::NORMAL_ATTACK:
			case C2S::LOGOUT:
			case C2S::CAST_SKILL:
			case C2S::CAST_INSTANT_SKILL:
			case C2S::CAST_POS_SKILL:
			case C2S::SIT_DOWN:
				//这些指令无法使用
				break;

			case C2S::RECHARGE_EQUIPPED_FLYSWORD:
			case C2S::RECHARGE_FLYSWORD:
				return UnLockInventoryHandler(cmd_type,buf,size);

			default:
				return CommandHandler(cmd_type,buf,size);
		}
	}
	return 0;
}

int 
gplayer_controller::CommandHandler(int cmd_type,const void * buf, size_t size)
{
	if(!CheckBanish()) return 0;
	switch(cmd_type)
	{
		case C2S::PLAYER_MOVE:
			if(CheckDeny(CMD_MOVE)) return 0;
			cmd_user_move(buf,size);
			return 0;
		case C2S::STOP_MOVE:
			if(CheckDeny(CMD_MOVE)) return 0;
			cmd_user_stop_move(buf,size);
			return 0;
			
		case C2S::UNSELECT:
			UnSelect();
			return 0;

		case C2S::SELECT_TARGET:
			{

				C2S::CMD::select_target &cmd = *(C2S::CMD::select_target *)buf;
				if(size != sizeof(cmd))
				{
					error_cmd(S2C::ERR_FATAL_ERR);
					return 0;
				}
				XID target;
				MAKE_ID(target,cmd.id);
				if(target.IsActive())
				{	
					SelectTarget(target.id);
				}
			}
			return 0;
		case C2S::NORMAL_ATTACK:
			if(CheckDeny(CMD_ATTACK)) return 0;
			if(CheckDeny(CMD_NORMAL_ATTACK)) return 0;
		{
			C2S::CMD::normal_attack & attack = *(C2S::CMD::normal_attack*)buf;
			if(sizeof(attack) != size) 
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			if(_cur_target.id == -1 || _cur_target.id == _imp->_parent->ID.id)
			{
				//当前没有选中对象，返回一个错误
				error_cmd(S2C::ERR_INVALID_TARGET);
				break;
			}

			//不检查选中对象的位置信息
			gactive_imp *pImp= (gactive_imp*)_imp;
			session_normal_attack *pAttack = new session_normal_attack(pImp);
			pAttack->SetTarget(_cur_target,attack.force_attack);
			if(pImp->AddSession(pAttack)) pImp->StartSession();
		}
		return 0;

		case C2S::PICKUP:
		if(CheckDeny(CMD_PICKUP)) return 0;
		{
			C2S::CMD::pickup_matter & pm = *(C2S::CMD::pickup_matter*)buf;
			if(sizeof(pm) != size)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			if(pImp->_cheat_punish) return 0;
			
			if(((gplayer*)pImp->_parent)->IsInvisible())
			{
				error_cmd(S2C::ERR_OPERTION_DENYED_IN_INVISIBLE);
				break;
			}
		
			//限制捡物品的速度
			int ts = g_timer.get_systime();
			_pickup_counter += (ts - _pickup_timestamp) * STD_PICKUP_PER_SECOND;
			if(_pickup_counter > MAX_PICKUP_PER_SECOND) _pickup_counter = MAX_PICKUP_PER_SECOND;
			else if(_pickup_counter < -1024) _pickup_counter = -5;
			_pickup_timestamp = ts;
			
			_pickup_counter --;
			if(_pickup_counter <= 0) break;		//超过了最大捡取速度
			
			size_t type = pm.type;
			gplayer * pPlayer = (gplayer*)(pImp->_parent);
			XID obj(GM_TYPE_MATTER,pm.mid);
			if(type != MONEY_MATTER_ID)
			{
				//检查包裹是否已经满了 
				if(!pImp->_inventory.HasSlot(type))
				{
					error_cmd(S2C::ERR_INVENTORY_IS_FULL);
					break;
				}
			}
			else
			{	
				//是金钱物品对应的id
				//检查金钱是否满了
				if(pImp->GetMoney() >= pImp->_money_capacity)
				{
					error_cmd(S2C::ERR_INVENTORY_IS_FULL);
					break;
				}
			}
			

			//检查物品是否存在和位置是否合适
			//注意物品的类型包含了16位标志，所以要去掉
			world::object_info info;
			if(!pImp->_plane->QueryObject(obj,info) 
					|| (info.race & 0x0000FFFF) != (int)type)
			{
				((gplayer_dispatcher*)pImp->_runner)->object_is_invalid(obj.id);
				break;
			}

			if(info.pos.squared_distance(pPlayer->pos)>=PICKUP_DISTANCE*PICKUP_DISTANCE)
			{
				error_cmd(S2C::ERR_ITEM_CANT_PICKUP);
				break;
			}

			//根据不同的组队状态调用捡取逻辑
			if(type != MONEY_MATTER_ID && pImp->_team.IsInTeam() && pImp->_team.IsRandomPickup())
			{
				//随机捡取 由组队策略确定由谁来捡取
				XID who = pImp->_team.GetLuckyBoy(pPlayer->ID,info.pos);
				__PRINTF("幸运儿是%d\n",who.id);


				msg_pickup_t mpt = { who,pImp->_team.GetTeamSeq()};
				//发一个捡物品的消息
				MSG msg;
				BuildMessage(msg,GM_MSG_PICKUP,obj,
						pPlayer->ID,info.pos,pImp->_team.GetTeamID(),
						&mpt, sizeof(mpt));
				pImp->_plane->PostLazyMessage(msg);
				break;
			}
			
			//是钱或者没有组队直接发出消息
			{
				msg_pickup_t self = { pPlayer->ID,0};
				if(pImp->IsInTeam()) self.team_seq = pImp->_team.GetTeamSeq();
				pImp->SendTo<0>(GM_MSG_PICKUP,obj,pImp->_team.GetTeamID(),&self,sizeof(self));
			}
		}
		break;
		case C2S::GET_ITEM_INFO:
		{
			C2S::CMD::get_item_info & gii = *(C2S::CMD::get_item_info*)buf;
			if(sizeof(gii) != size)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			
			if(gii.where > 2) 
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			if(pImp->PlayerGetItemInfo(gii.where,gii.index) < 0)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
			}
		}
		break;

		case C2S::GET_INVENTORY:
		{
			C2S::CMD::get_inventory & gi = *(C2S::CMD::get_inventory *)buf;
			if(sizeof(gi) != size)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}

			int where = gi.where;
			if(where > 2) 
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->PlayerGetInventory(where);
		}
		break;

		case C2S::GET_INVENTORY_DETAIL:
		{
			C2S::CMD::get_inventory_detail & gid = *(C2S::CMD::get_inventory_detail *)buf;
			if(sizeof(gid) != size)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			
			int where = gid.where;
			if(where > 2) 
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->PlayerGetInventoryDetail(where);
		}
		break;

		case C2S::EXCHANGE_INVENTORY_ITEM:
		{
			C2S::CMD::exchange_inventory_item & eii = *(C2S::CMD::exchange_inventory_item*) buf;
			if(sizeof(eii) != size)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->PlayerExchangeInvItem(eii.index1,eii.index2);
			pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_INVENTORY, eii.index1);
			pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_INVENTORY, eii.index2);
		}
		break;

		case C2S::MOVE_INVENTORY_ITEM:
		{
			C2S::CMD::move_inventory_item & mii = *(C2S::CMD::move_inventory_item *) buf;
			if(sizeof(mii) != size)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->PlayerMoveInvItem(mii.src,mii.dest,mii.amount);
			pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_INVENTORY, mii.src);
			pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_INVENTORY, mii.dest);
		}
		break;

		case C2S::DROP_INVENTORY_ITEM:
		{
			C2S::CMD::drop_inventory_item& dii = *(C2S::CMD::drop_inventory_item*) buf;
			if(sizeof(dii) != size)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp *pImp = (gplayer_imp*)_imp;
			pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_INVENTORY, dii.index);
			if(!TestSafeLock()) return 0;
			if(CheckDeny(CMD_PICKUP)) 
			{
				error_cmd(S2C::ERR_CAN_NOT_DROP_ITEM);
				return 0;
			}
			
			if(((gplayer*)pImp->_parent)->IsInvisible())
			{
				error_cmd(S2C::ERR_OPERTION_DENYED_IN_INVISIBLE);
				break;
			}
		
			if(!pImp->_cooldown.TestCoolDown(COOLDOWN_INDEX_DROP_ITEM))
			{
				break;
			}
			pImp->_cooldown.SetCoolDown(COOLDOWN_INDEX_DROP_ITEM,DROPITEM_COOLDOWN_TIME);
			pImp->PlayerDropInvItem(dii.index,dii.amount,true);
		}
		break;
		
		case C2S::DROP_EQUIPMENT_ITEM:
		{
			C2S::CMD::drop_equipment_item & dei = *(C2S::CMD::drop_equipment_item*) buf;
			if(sizeof(dei) != size)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp *pImp = (gplayer_imp*)_imp;
			pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_EQUIPMENT, dei.index);
			if(!TestSafeLock()) return 0;
			if(CheckDeny(CMD_PICKUP)) 
			{
				error_cmd(S2C::ERR_CAN_NOT_DROP_ITEM);
				return 0;
			}

			if(((gplayer*)pImp->_parent)->IsInvisible())
			{
				error_cmd(S2C::ERR_OPERTION_DENYED_IN_INVISIBLE);
				break;
			}
		
			if(!pImp->_cooldown.TestCoolDown(COOLDOWN_INDEX_DROP_ITEM))
			{
				break;
			}
			pImp->_cooldown.SetCoolDown(COOLDOWN_INDEX_DROP_ITEM,DROPITEM_COOLDOWN_TIME);
			pImp->PlayerDropEquipItem(dei.index,true);
		}
		break;

		case C2S::EXCHANGE_EQUIPMENT_ITEM:
		{
			C2S::CMD::exchange_equip_item & eei = *(C2S::CMD::exchange_equip_item*)buf;
			if(sizeof(eei) != size)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->PlayerExchangeEquipItem(eei.idx1,eei.idx2);
			pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_EQUIPMENT, eei.idx1);
			pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_EQUIPMENT, eei.idx2);
		}
		break;

		case C2S::EQUIP_ITEM:
		{
			C2S::CMD::equip_item & ei = *(C2S::CMD::equip_item*)buf;
			if(sizeof(ei) != size)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->PlayerEquipItem(ei.idx_inv,ei.idx_eq);
			pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_INVENTORY, ei.idx_inv);
			pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_EQUIPMENT, ei.idx_eq);
		}
		break;

		case C2S::MOVE_ITEM_TO_EQUIPMENT:
		{
			C2S::CMD::move_item_to_equipment & mite  = *(C2S::CMD::move_item_to_equipment*)buf;
			if(sizeof(mite) != size)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->PlayerMoveEquipItem(mite.idx_inv,mite.idx_eq);
			pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_INVENTORY, mite.idx_inv);
			pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_EQUIPMENT, mite.idx_eq);
		}
		break;


		case C2S::DROP_MONEY:
		{
			if(!TestSafeLock()) return 0;
			C2S::CMD::drop_money & dm = *(C2S::CMD::drop_money *)buf;
			if(sizeof(dm) != size)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			if(!pImp->_cooldown.TestCoolDown(COOLDOWN_INDEX_DROP_MONEY))
			{
				break;
			}
			pImp->_cooldown.SetCoolDown(COOLDOWN_INDEX_DROP_MONEY,DROPMONEY_COOLDOWN_TIME);
			pImp->PlayerDropMoney(dm.amount,true);
		}
		break;

		case C2S::SELF_GET_PROPERTY:
		{
			C2S::CMD::self_get_property & gsp = *(C2S::CMD::self_get_property *)buf;
			if(sizeof(gsp) != size)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->PlayerGetProperty();
		}
		break;

		case C2S::SET_STATUS_POINT:
		{
			C2S::CMD::set_status_point & ssp = *(C2S::CMD::set_status_point*)buf;
			if(sizeof(ssp) != size)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->PlayerSetStatusPoint(ssp.vit,ssp.eng,ssp.str,ssp.agi);
		}
		break;

		case C2S::GET_EXTPROP_BASE:
			_imp->_runner->get_extprop_base();
		break;
		case C2S::GET_EXTPROP_MOVE:
			_imp->_runner->get_extprop_move();
		break;
		case C2S::GET_EXTPROP_ATTACK:
			_imp->_runner->get_extprop_attack();
		break;
		case C2S::GET_EXTPROP_DEFENSE:
			_imp->_runner->get_extprop_defense();
		break;

		case C2S::TEAM_INVITE:
		{
			C2S::CMD::team_invite & ti = *(C2S::CMD::team_invite *)buf;
			if(sizeof(ti) != size)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			XID member(GM_TYPE_PLAYER,ti.id);
			if(member == pImp->_parent->ID)
			{
				error_cmd(S2C::ERR_TEAM_CANNOT_INVITE);
				break;
			}
	
			//检测该玩家是否存在
			if(!pImp->_plane->IsPlayerExist(member.id))
			{
				pImp->_runner->error_message(S2C::ERR_PLAYER_NOT_EXIST);
				break;
			}

			if(!pImp->_team.CliInviteOther(member))
			{
				error_cmd(S2C::ERR_TEAM_CANNOT_INVITE);
			}
		}
		break;

		case C2S::TEAM_AGREE_INVITE:
		{
			C2S::CMD::team_agree_invite & tai = *(C2S::CMD::team_agree_invite *)buf;
			if(sizeof(tai) != size)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			XID leader(GM_TYPE_PLAYER,tai.id);
			if(!pImp->_team.CliAgreeInvite(leader,tai.team_seq))
			{
				error_cmd(S2C::ERR_TEAM_JOIN_FAILED);
			}
		}
		break;
		
		case C2S::TEAM_REJECT_INVITE:
		{
			C2S::CMD::team_reject_invite & tri = *(C2S::CMD::team_reject_invite *)buf;
			if(sizeof(tri) != size)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			XID leader(GM_TYPE_PLAYER,tri.id);
			pImp->_team.CliRejectInvite(leader);
		}
		break;

		case C2S::TEAM_LEAVE_PARTY:
		{
			C2S::CMD::team_leave_party & tlp = *(C2S::CMD::team_leave_party *)buf;
			if(sizeof(tlp) != size)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->_team.CliLeaveParty();
		}
		break;
		
		case C2S::TEAM_DISMISS_PARTY:
		{
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->_team.CliDismissParty();
		}
		break;

		case C2S::TEAM_KICK_MEMBER:
		{
			C2S::CMD::team_kick_member & tkm = *(C2S::CMD::team_kick_member *)buf;
			if(sizeof(tkm) != size)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			XID member(GM_TYPE_PLAYER,tkm.id);
			pImp->_team.CliKickMember(member);
		}
		break;
		case C2S::TEAM_GET_TEAMMATE_POS:
		{
			C2S::CMD::team_get_teammate_pos & tgtp= *(C2S::CMD::team_get_teammate_pos*)buf;
			if(tgtp.count >= TEAM_MEMBER_CAPACITY || !tgtp.count || sizeof(tgtp) + tgtp.count * sizeof(int) != size)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			if(!pImp->_team.IsInTeam()) break;

			XID member(GM_TYPE_PLAYER,-1);
			A3DVECTOR pos;
			int tag;
			int plane_index;
			for(size_t i = 0; i< tgtp.count; i++)
			{
				member.id = tgtp.id[i];
				if( pImp->_team.GetMemberPos(member,pos,tag,plane_index))
				{
					pImp->_runner->teammate_get_pos(member,pos,tag,
							tag==world_manager::GetWorldTag() && plane_index==pImp->_plane->w_plane_index); 
				}
			}
		}
		break;

		case C2S::GET_OTHERS_EQUIPMENT:
		{
			C2S::CMD::get_others_equipment & goe = *(C2S::CMD::get_others_equipment*)buf;
			if(sizeof(goe) + sizeof(int) * goe.size != size || goe.size > 256 )
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			_load_stats += goe.size>>1;
			//注：这个是一个非常耗费资源的操作 需要限制玩家的发送速度
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			gplayer * pPlayer = (gplayer*)(pImp->_parent);
			MSG msg;
			int cs_index = pPlayer->cs_index;
			BuildMessage(msg,GM_MSG_QUERY_PLAYER_EQUIPMENT,XID(-1,-1),pPlayer->ID,
						pPlayer->pos,pPlayer->cs_sid,&cs_index, sizeof(cs_index));

			//针对player id 进行群体发送
			pImp->_plane->SendPlayerMessage(goe.size,goe.idlist,msg);
		}
		break;
		
		case C2S::CHANGE_PICKUP_FLAG:
		{
			C2S::CMD::set_pickup_flag & spf= *(C2S::CMD::set_pickup_flag *)buf;
			if(sizeof(spf) != size)	
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->_team.SetPickupFlag(spf.pickup_flag);
		}
		break;

		case C2S::SERVICE_HELLO:
		{
			C2S::CMD::service_hello & cmd = *(C2S::CMD::service_hello*)buf;
			if(size != sizeof(cmd)) 
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			XID target;
			MAKE_ID(target,cmd.id);
			session_say_hello *pSession = new session_say_hello(pImp);
			pSession->SetTarget(target);
			if(pImp->AddSession(pSession))
			{
				pImp->StartSession();
			}
		}
		break;

		case C2S::SERVICE_GET_CONTENT:
		{
			C2S::CMD::service_get_content & cmd = *(C2S::CMD::service_get_content*)buf;
			if(size != sizeof(cmd))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			if(world_manager::GetGlobalController().CheckServerForbid(SERVER_FORBID_SERVICE,cmd.service_type))
			{
				error_cmd(S2C::ERR_SERVICE_UNAVILABLE);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->QueryServiceContent(cmd.service_type);
		}
		break;
		
		case C2S::SERVICE_SERVE:
		{
			C2S::CMD::service_serve & cmd = *(C2S::CMD::service_serve*)buf;
			if(cmd.len > 4096 || size != sizeof(cmd) + cmd.len) 
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			if(world_manager::GetGlobalController().CheckServerForbid(SERVER_FORBID_SERVICE,cmd.service_type))
			{
				error_cmd(S2C::ERR_SERVICE_UNAVILABLE);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->SendServiceRequest(cmd.service_type,cmd.content,cmd.len);
		}
		break;

		case C2S::LOGOUT:
		{
			C2S::CMD::logout & cmd = *(C2S::CMD::logout *)buf;
			if(size != sizeof(cmd)) 
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			int type = cmd.logout_type;
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->PlayerLogout(type);
		}
		break;

		case C2S::GET_OWN_WEALTH:
		{
			C2S::CMD::get_own_wealth & gow = *(C2S::CMD::get_own_wealth *)buf;
			if(size != sizeof(C2S::CMD::get_own_wealth)) 
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			if(gow.detail_inv)
				pImp->PlayerGetInventoryDetail(gplayer_imp::IL_INVENTORY);
			else
				pImp->PlayerGetInventory(gplayer_imp::IL_INVENTORY);

			if(gow.detail_equip)
				pImp->PlayerGetInventoryDetail(gplayer_imp::IL_EQUIPMENT);
			else
				pImp->PlayerGetInventory(gplayer_imp::IL_EQUIPMENT);

			if(gow.detail_task)
				pImp->PlayerGetInventoryDetail(gplayer_imp::IL_TASK_INVENTORY);
			else
				pImp->PlayerGetInventory(gplayer_imp::IL_TASK_INVENTORY);

			pImp->_runner->get_player_money(pImp->GetMoney(),pImp->_money_capacity);
		}
		break;

		case C2S::GET_WALLOW_INFO:
		{
		/*
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			time_t l_time;
			time_t h_time;
			int ptime;
			pImp->_wallow_obj.GetTimeLeft(&l_time, &h_time,&ptime);
			pImp->_runner->player_wallow_info(pImp->_wallow_level,ptime, l_time, h_time);
			*/
		}
		break;

		case C2S::GET_ALL_DATA:
		{
			C2S::CMD::get_all_data & gad = *(C2S::CMD::get_all_data *)buf;
			if(size != sizeof(C2S::CMD::get_all_data)) 
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->SendAllData(gad.detail_inv, gad.detail_equip, gad.detail_task);
		}
		break;

		case C2S::USE_ITEM:
		if(CheckDeny(CMD_USE_ITEM)) return 0;
		{
			C2S::CMD::use_item & ui = *(C2S::CMD::use_item *)buf;
			if(size != sizeof(C2S::CMD::use_item)) 
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			if(world_manager::GetGlobalController().CheckServerForbid(SERVER_FORBID_ITEM,ui.item_id))
			{
				error_cmd(S2C::ERR_CANNOT_USE_ITEM);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			if(!pImp->PlayerUseItem(ui.where, ui.index, ui.item_id,ui.count))
			{	
				//改在物品里面发送这个错误信息了
				//error_cmd(S2C::ERR_CANNOT_USE_ITEM);
			}
		}
		break;

		case C2S::CAST_SKILL:
		if(CheckDeny(CMD_ATTACK)) return 0;
		{
			C2S::CMD::cast_skill & cs = *(C2S::CMD::cast_skill *)buf;
			if(size != sizeof(C2S::CMD::cast_skill) + sizeof(int)*(size_t)cs.target_count) 
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			if(world_manager::GetGlobalController().CheckServerForbid(SERVER_FORBID_SKILL,cs.skill_id))
			{
				error_cmd(S2C::ERR_SKILL_NOT_AVAILABLE);
				break;
			}
			
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			//不检查选中对象的位置信息
			if(!GNET::SkillWrapper::IsMovingSkill(cs.skill_id))
			{
				session_skill *pSkill= new session_skill(pImp);
				pSkill->SetTarget(cs.skill_id,cs.force_attack,cs.target_count,cs.targets);
				if(pImp->AddSession(pSkill)) pImp->StartSession();
			}
			else
			{
				if(pImp->GetAction())
				{
					error_cmd(S2C::ERR_OTHER_ACTION_IN_EXECUTE);
					break;
				}
				if(pImp->InNonMoveSession())
				{
					error_cmd(S2C::ERR_ACTION_DENYED_IN_NON_MOVE_SESSION);
					break;
				}
				moving_skill *pSkill= new moving_skill(pImp);
				pSkill->SetTarget(cs.skill_id,cs.force_attack,cs.target_count,cs.targets);
				pImp->StartAction(pSkill);	
			}
		}
		break;

		case C2S::CAST_INSTANT_SKILL:
		if(CheckDeny(CMD_ATTACK)) return 0;
		{
			C2S::CMD::cast_instant_skill & cs = *(C2S::CMD::cast_instant_skill *)buf;
			if(size != sizeof(C2S::CMD::cast_instant_skill) + sizeof(int)*(size_t)cs.target_count) 
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			if(world_manager::GetGlobalController().CheckServerForbid(SERVER_FORBID_SKILL,cs.skill_id))
			{
				error_cmd(S2C::ERR_SKILL_NOT_AVAILABLE);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			//不检查选中对象的位置信息
			if(!GNET::SkillWrapper::IsMovingSkill(cs.skill_id))
			{
				session_instant_skill *pSkill= new session_instant_skill(pImp);
				pSkill->SetTarget(cs.skill_id,cs.force_attack,cs.target_count,cs.targets);
				if(pImp->AddSession(pSkill)) pImp->StartSession();
			}
			else
			{
				if(pImp->GetAction())
				{
					error_cmd(S2C::ERR_OTHER_ACTION_IN_EXECUTE);
					break;
				}
				if(pImp->InNonMoveSession())
				{
					error_cmd(S2C::ERR_ACTION_DENYED_IN_NON_MOVE_SESSION);
					break;
				}
				moving_instant_skill *pSkill= new moving_instant_skill(pImp);
				pSkill->SetTarget(cs.skill_id,cs.force_attack,cs.target_count,cs.targets);
				pImp->StartAction(pSkill);	
			}
		}
		break;

		case C2S::CANCEL_ACTION:
		{
			if(size != sizeof(C2S::CMD::cancel_action))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;

			//加入一个阻止除了除了移动操作外的所有的命令的session
			session_cancel_action *pCancel= new session_cancel_action();
			pImp->AddSession(pCancel);
			pImp->StartSession();

			//停止当前的session
			pImp->TryStopCurSession();
			pImp->TryBreakAction();
		}
		break;

		case C2S::RECHARGE_EQUIPPED_FLYSWORD:
		{
			C2S::CMD::recharge_equipped_flysword & ref = *(C2S::CMD::recharge_equipped_flysword *)buf;
			if(size != sizeof(C2S::CMD::recharge_equipped_flysword))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			if(!pImp->RechargeEquippedFlySword(ref.element_index,ref.count))
			{
				error_cmd(S2C::ERR_CANNOT_RECHARGE);
			}
			pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_INVENTORY, ref.element_index);
			pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_EQUIPMENT, item::EQUIP_INDEX_FLYSWORD);
		}
		break;

		case C2S::RECHARGE_FLYSWORD:
		{
			C2S::CMD::recharge_flysword & rf = *(C2S::CMD::recharge_flysword *)buf;
			if(size != sizeof(C2S::CMD::recharge_flysword))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			if(!pImp->RechargeFlySword(rf.element_index,rf.count,rf.flysword_index,rf.flysword_id))
			{
				error_cmd(S2C::ERR_CANNOT_RECHARGE);
			}
			pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_INVENTORY, rf.element_index);
			pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_INVENTORY, rf.flysword_index);
		}
		break;

		case C2S::USE_ITEM_WITH_TARGET:
		if(CheckDeny(CMD_USE_ITEM)) return 0;
		{
			C2S::CMD::use_item_with_target & uiwt = *(C2S::CMD::use_item_with_target *)buf;
			if(size != sizeof(C2S::CMD::use_item_with_target)) 
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			if(world_manager::GetGlobalController().CheckServerForbid(SERVER_FORBID_ITEM,uiwt.item_id))
			{
				error_cmd(S2C::ERR_CANNOT_USE_ITEM);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			if(!pImp->PlayerUseItemWithTarget(uiwt.where, uiwt.index, uiwt.item_id,uiwt.force_attack))
			{
				error_cmd(S2C::ERR_CANNOT_USE_ITEM);
			}
		}
		break;

		case C2S::SIT_DOWN:
		if(CheckDeny(CMD_MOVE)) return 0;
		{
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			if(!pImp->PlayerSitDown())
			{
				error_cmd(S2C::ERR_CANNOT_SIT_DOWN);
			}
			
		}
		break;

		case C2S::EMOTE_ACTION:
		{
			C2S::CMD::emote_action & tn = *(C2S::CMD::emote_action *)buf;
			if(size != sizeof(tn))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			if(pImp->CheckCoolDown(COOLDOWN_INDEX_EMOTE))
			{
				pImp->SetCoolDown(COOLDOWN_INDEX_EMOTE,EMOTE_COOLDOWN_TIME);
				pImp->_runner->do_emote(tn.action);
			}
		}
		break;

		case C2S::TRICKS_ACTION:
		{
			C2S::CMD::tricks_action & ta = *(C2S::CMD::tricks_action *)buf;
			if(size != sizeof(ta))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			//现在先不考虑限制此操作的使用次数，如有需要， 以后可以考虑
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			if(pImp->_cur_session && pImp->_cur_session->GetGUID() == CLS_SESSION_MOVE) 
			{
				pImp->_runner->do_action(ta.action);
			}
		}
		break;

		case C2S::TASK_NOTIFY:
		{
			C2S::CMD::task_notify & tn = *(C2S::CMD::task_notify *)buf;
			if(size < sizeof(C2S::CMD::task_notify) || size != sizeof(C2S::CMD::task_notify) + tn.size) 
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			_load_stats += 10;
			PlayerTaskInterface  task_if((gplayer_imp*)_imp);
			OnClientNotify(&task_if,tn.buf,tn.size);
		}
		break;


		case C2S::ASSIST_SELECT:
		{
			C2S::CMD::assist_select & tn = *(C2S::CMD::assist_select *)buf;
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			if(size != sizeof(C2S::CMD::assist_select))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			if(pImp->IsInTeam())
			{	
				XID target(GM_TYPE_PLAYER,tn.partner);
				pImp->PlayerAssistSelect(target);
			}
		}
		break;

		case C2S::CONTINUE_ACTION:
		{
			gplayer_imp * pImp = (gplayer_imp*)_imp; 
			pImp->PlayerRestartSession();
			pImp->RestartAction();
		}
		break;

		case C2S::STOP_FALL:
		{
			gplayer_imp * pImp = (gplayer_imp*)_imp; 
			if(pImp->_layer_ctrl.IsFalling())
			{
				//pImp->_layer_ctrl.Ground();	客户端没有使用，注释掉，以免更改当前layer
			}
		}
		break;

		case C2S::GET_ITEM_INFO_LIST:
		{
			C2S::CMD::get_item_info_list & giil = *(C2S::CMD::get_item_info_list*)buf;
			if(size < sizeof(giil) || size != sizeof(giil) + giil.count)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->PlayerGetItemInfoList(giil.where,giil.count,giil.item_list);
		}
		break;

		case C2S::GATHER_MATERIAL:
		if(CheckDeny(CMD_PICKUP)) return 0;
		{
			C2S::CMD::gather_material & gm = *(C2S::CMD::gather_material*)buf;
			if(size != sizeof(gm))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			
			XID target;
			MAKE_ID(target,gm.mid);
			gplayer_imp * pImp = (gplayer_imp*)_imp;

			if(((gplayer*)pImp->_parent)->IsInvisible())
			{
				error_cmd(S2C::ERR_OPERTION_DENYED_IN_INVISIBLE);
				break;
			}
			
			if(!pImp->_cooldown.TestCoolDown(COOLDOWN_INDEX_PLAYER_GATHER))
			{
				error_cmd(S2C::ERR_MINE_GATHER_IS_COOLING);
				break;
			}
		
			if(target.type == GM_TYPE_MATTER)
			{
			/*
				if(pImp->HasSession())
				{
					error_cmd(S2C::ERR_OTHER_SESSION_IN_EXECUTE);
					break;
				}
				
				if(!pImp->IsItemExist(gm.tool_where,gm.tool_index,gm.tool_type,1))
				{
					error_cmd(S2C::ERR_MINE_HAS_INVALID_TOOL);
					break;
				}
				
				pImp->SendTo<0>(GM_MSG_GATHER_REQUEST,target,gm.tool_type);
				*/
				int task_id = gm.task_id;
				if(task_id > 0)
				{
					PlayerTaskInterface  task_if((gplayer_imp*)_imp);
					if(!task_if.CanDoMining(task_id))
					{
						break;
					}
				}
				
				//设置冷却时间
				pImp->_cooldown.SetCoolDown(COOLDOWN_INDEX_PLAYER_GATHER,PLAYER_GATHER_COOLDOWN_TIME);

				session_gather_prepare *pSession = new session_gather_prepare(pImp);
				pSession->SetTarget(target.id,gm.tool_where,gm.tool_index,gm.tool_type,task_id,pImp->_player_fatering.GetGainTimes());
				if(pImp->AddSession(pSession)) pImp->StartSession();
			}
		}
		break;

		case C2S::GET_TRASHBOX_INFO:
		{
			C2S::CMD::get_trashbox_info & gti = *(C2S::CMD::get_trashbox_info*)buf;
			if(size != sizeof(gti))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			if(gti.is_usertrashbox)
			{
				if(!pImp->_user_trash_box_open_flag)	
				{
					error_cmd(S2C::ERR_TRASH_BOX_NOT_OPEN);
					break;
				}
				pImp->PlayerGetUserTrashBoxInfo(gti.detail);
			}
			else
			{
				if(!pImp->_trash_box_open_flag && !pImp->_trash_box_open_view_only_flag)
				{
					error_cmd(S2C::ERR_TRASH_BOX_NOT_OPEN);
					break;
				}
				pImp->PlayerGetTrashBoxInfo(gti.detail);
			}
		}
		break;
		
		case C2S::EXCHANGE_TRASHBOX_ITEM:
		{
			C2S::CMD::exchange_trashbox_item & eti = *(C2S::CMD::exchange_trashbox_item*) buf;
			if(sizeof(eti) != size)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->_runner->unlock_inventory_slot(eti.where, eti.index1);
			pImp->_runner->unlock_inventory_slot(eti.where, eti.index2);
			int err = 0;	
			switch(eti.where)
			{
				case gplayer_imp::IL_TRASH_BOX:
				case gplayer_imp::IL_TRASH_BOX2:
					if(!pImp->_trash_box_open_flag) err = S2C::ERR_TRASH_BOX_NOT_OPEN;
					break;
				case gplayer_imp::IL_TRASH_BOX3:
				case gplayer_imp::IL_TRASH_BOX4:
					break;
				case gplayer_imp::IL_USER_TRASH_BOX:
					if(!pImp->_user_trash_box_open_flag) err = S2C::ERR_TRASH_BOX_NOT_OPEN;
					break;
				default:
					err = S2C::ERR_TRASH_BOX_NOT_OPEN;
					break;
			}
			if(err > 0)
			{
				error_cmd(err);
				break;
			}
			pImp->PlayerExchangeTrashItem(eti.where, eti.index1,eti.index2);
		}
		break;

		case C2S::MOVE_TRASHBOX_ITEM:
		{
			C2S::CMD::move_trashbox_item & mti = *(C2S::CMD::move_trashbox_item *) buf;
			if(sizeof(mti) != size)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->_runner->unlock_inventory_slot(mti.where, mti.src);
			pImp->_runner->unlock_inventory_slot(mti.where, mti.dest);
			int err = 0;	
			switch(mti.where)
			{
				case gplayer_imp::IL_TRASH_BOX:
				case gplayer_imp::IL_TRASH_BOX2:
					if(!pImp->_trash_box_open_flag) err = S2C::ERR_TRASH_BOX_NOT_OPEN;
					break;
				case gplayer_imp::IL_TRASH_BOX3:
				case gplayer_imp::IL_TRASH_BOX4:
					break;
				case gplayer_imp::IL_USER_TRASH_BOX:
					if(!pImp->_user_trash_box_open_flag) err = S2C::ERR_TRASH_BOX_NOT_OPEN;
					break;
				default:
					err = S2C::ERR_TRASH_BOX_NOT_OPEN;
					break;
			}
			if(err > 0)
			{
				error_cmd(err);
				break;
			}
			pImp->PlayerMoveTrashItem(mti.where, mti.src,mti.dest,mti.amount);
		}
		break;
		
		case C2S::EXHCANGE_TRASHBOX_INVENTORY:
		{
			C2S::CMD::exchange_trashbox_inventory & eti = *(C2S::CMD::exchange_trashbox_inventory *) buf;
			if(sizeof(eti) != size)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->_runner->unlock_inventory_slot(eti.where, eti.idx_tra);
			pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_INVENTORY, eti.idx_inv);
			int err = 0;	
			switch(eti.where)
			{
				case gplayer_imp::IL_TRASH_BOX:
				case gplayer_imp::IL_TRASH_BOX2:
					if(!pImp->_trash_box_open_flag) err = S2C::ERR_TRASH_BOX_NOT_OPEN;
					break;
				case gplayer_imp::IL_TRASH_BOX3:
				case gplayer_imp::IL_TRASH_BOX4:
					break;
				case gplayer_imp::IL_USER_TRASH_BOX:
					if(!pImp->_user_trash_box_open_flag) err = S2C::ERR_TRASH_BOX_NOT_OPEN;
					break;
				default:
					err = S2C::ERR_TRASH_BOX_NOT_OPEN;
					break;
			}
			if(err > 0)
			{
				error_cmd(err);
				break;
			}
			pImp->PlayerExchangeTrashInv(eti.where, eti.idx_tra,eti.idx_inv);
		}
		break;

		case C2S::MOVE_TRASHBOX_ITEM_TO_INVENTORY:
		{
			C2S::CMD::move_trashbox_item_to_inventory & mtiti = *(C2S::CMD::move_trashbox_item_to_inventory *) buf;
			if(sizeof(mtiti) != size)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->_runner->unlock_inventory_slot(mtiti.where, mtiti.idx_tra);
			pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_INVENTORY, mtiti.idx_inv);
			int err = 0;	
			switch(mtiti.where)
			{
				case gplayer_imp::IL_TRASH_BOX:
				case gplayer_imp::IL_TRASH_BOX2:
					if(!pImp->_trash_box_open_flag) err = S2C::ERR_TRASH_BOX_NOT_OPEN;
					break;
				case gplayer_imp::IL_TRASH_BOX3:
				case gplayer_imp::IL_TRASH_BOX4:
					break;
				case gplayer_imp::IL_USER_TRASH_BOX:
					if(!pImp->_user_trash_box_open_flag) err = S2C::ERR_TRASH_BOX_NOT_OPEN;
					break;
				default:
					err = S2C::ERR_TRASH_BOX_NOT_OPEN;
					break;
			}
			if(err > 0)
			{
				error_cmd(err);
				break;
			}
			pImp->PlayerTrashItemToInv(mtiti.where, mtiti.idx_tra,mtiti.idx_inv,mtiti.amount);
		}
		break;
		
		case C2S::MOVE_INVENTORY_ITEM_TO_TRASHBOX:
		{
			C2S::CMD::move_inventory_item_to_trashbox & miitt  = *(C2S::CMD::move_inventory_item_to_trashbox *) buf;
			if(sizeof(miitt) != size)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->_runner->unlock_inventory_slot(miitt.where, miitt.idx_tra);
			pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_INVENTORY, miitt.idx_inv);
			int err = 0;	
			switch(miitt.where)
			{
				case gplayer_imp::IL_TRASH_BOX:
				case gplayer_imp::IL_TRASH_BOX2:
					if(!pImp->_trash_box_open_flag) err = S2C::ERR_TRASH_BOX_NOT_OPEN;
					break;
				case gplayer_imp::IL_TRASH_BOX3:
				case gplayer_imp::IL_TRASH_BOX4:
					break;
				case gplayer_imp::IL_USER_TRASH_BOX:
					if(!pImp->_user_trash_box_open_flag) err = S2C::ERR_TRASH_BOX_NOT_OPEN;
					break;
				default:
					err = S2C::ERR_TRASH_BOX_NOT_OPEN;
					break;
			}
			if(err > 0)
			{
				error_cmd(err);
				break;
			}
			pImp->PlayerInvItemToTrash(miitt.where, miitt.idx_inv,miitt.idx_tra,miitt.amount);
		}
		break;
		
		case C2S::EXCHANGE_TRASHBOX_MONEY:
		{
			C2S::CMD::excnahge_trashbox_money & etm  = *(C2S::CMD::excnahge_trashbox_money *) buf;
			if(sizeof(etm) != size)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			if(etm.is_usertrashbox)
			{
				if(!pImp->_user_trash_box_open_flag)	
				{
					error_cmd(S2C::ERR_TRASH_BOX_NOT_OPEN);
					break;
				}
			}
			else
			{
				if(!pImp->_trash_box_open_flag)
				{
					error_cmd(S2C::ERR_TRASH_BOX_NOT_OPEN);
					break;
				}
			}
			pImp->PlayerExchangeTrashMoney(etm.is_usertrashbox, etm.inv_money, etm.trashbox_money);
		}
		break;

		case C2S::SET_ADV_DATA:
		{
			C2S::CMD::set_adv_data & sad = *(C2S::CMD::set_adv_data*) buf;
			if(sizeof(sad) != size)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}

			gplayer * pPlayer = (gplayer*)_imp->_parent;
			pPlayer->adv_data1 = sad.data1;
			pPlayer->adv_data2 = sad.data2;
			pPlayer->object_state |= gactive_object::STATE_ADV_MODE;
			_imp->_runner->set_adv_data(sad.data1,sad.data2);
		}
		break;

		case C2S::CLR_ADV_DATA:
		{
			gplayer * pPlayer = (gplayer*)_imp->_parent;
			if(pPlayer->object_state & gactive_object::STATE_ADV_MODE)
			{
				pPlayer->object_state &= ~gactive_object::STATE_ADV_MODE;
				_imp->_runner->clear_adv_data();
			}
		}
		break;

		case C2S::TEAM_LFG_REQUEST:
		{
			C2S::CMD::team_lfg_request & tlr  = *(C2S::CMD::team_lfg_request *) buf;
			if(sizeof(tlr) != size)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			if(!pImp->IsInTeam() && !pImp->IsAutoComposingTeam())
			{
				//发送特定的消息
				pImp->SendTo<0>(GM_MSG_TEAM_APPLY_PARTY,XID(GM_TYPE_PLAYER,tlr.id),0);
			}
		}
		break;

		case C2S::TEAM_LFG_REPLY:
		{
			C2S::CMD::team_lfg_reply & tlr  = *(C2S::CMD::team_lfg_reply *) buf;
			if(sizeof(tlr) != size)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}

			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->_team.AgreeApply(tlr.id,tlr.result);
		}
		break;

		case  C2S::QUERY_PLAYER_INFO_1:
		{
			C2S::CMD::query_player_info_1 & qpi = *(C2S::CMD::query_player_info_1 *) buf;
			if(size < sizeof(C2S::CMD::query_player_info_1) || 
					qpi.count >= 256 ||
					size != sizeof(C2S::CMD::query_player_info_1)+qpi.count*sizeof(int))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}

			_load_stats += qpi.count;
			//对玩家 进行特殊处理
			((gplayer_imp*)_imp)->QueryOtherPlayerInfo1(qpi.count,qpi.id);
		}
		break;

		case  C2S::QUERY_NPC_INFO_1:
		{
			C2S::CMD::query_npc_info_1 & qni = *(C2S::CMD::query_npc_info_1 *) buf;
			if(size < sizeof(C2S::CMD::query_npc_info_1) || 
					qni.count >= 256 ||
					size != sizeof(C2S::CMD::query_npc_info_1)+qni.count*sizeof(int))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			_load_stats += qni.count;
			((gplayer_imp*)_imp)->QueryNPCInfo1(qni.count,qni.id);
		}
		break;

		case C2S::SESSION_EMOTE_ACTION:
		{
			C2S::CMD::session_emote_action & sea = *(C2S::CMD::session_emote_action *) buf;
			if(size != sizeof(sea))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			if(sea.action <= 0) break;
			//加入动作的session
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			session_emote_action *pSession = new session_emote_action(pImp);
			pSession->SetAction(sea.action);
			if(pImp->AddSession(pSession)) pImp->StartSession();
		}
		break;

		case C2S::CONCURRECT_EMOTE_REQUEST:
		{
			C2S::CMD::concurrent_emote_request & cer  = *(C2S::CMD::concurrent_emote_request *) buf;
			if(size != sizeof(cer))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}

			//现在禁止了协同动作
			break;
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			//设置协同动作的对象，发出协同请求
			world::object_info info;
			XID id(GM_TYPE_PLAYER,cer.target);
			if(!pImp->_plane->QueryObject(id,info) 
				|| info.pos.squared_distance(pImp->_parent->pos) >= 2.0f*2.0f)
			{
				error_cmd(S2C::ERR_OUT_OF_RANGE);
				break;
			}
			//距离判定通过
			pImp->SetConcurrentEmote(id.id,cer.action);
			pImp->SendTo<0>(GM_MSG_CON_EMOTE_REQUEST,id,cer.action);
		}
		break;

		case C2S::CONCURRECT_EMOTE_REPLY:
		{
			C2S::CMD::concurrent_emote_reply & cer  = *(C2S::CMD::concurrent_emote_reply *) buf;
			if(size != sizeof(cer))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			//设置协同动作的对象，发出协同回应
			int target = cer.target;
			int data = cer.action;
			data <<= 16;
			data |= cer.result;
			pImp->SendTo<0>(GM_MSG_CON_EMOTE_REPLY,XID(GM_TYPE_PLAYER,target),data);
		}
		break;

		case C2S::TEAM_CHANGE_LEADER:
		{
			C2S::CMD::team_change_leader & tcl   = *(C2S::CMD::team_change_leader *) buf;
			if(size != sizeof(tcl))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			//现在转换队长的操作还很粗糙
			pImp->TeamChangeLeader(tcl.new_leader);
		}
		break;

		case C2S::ENTER_SANCTUARY:
		{
			C2S::CMD::enter_sanctuary & es = *(C2S::CMD::enter_sanctuary*) buf;
			if(size != sizeof(es))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			if(es.id == pImp->_parent->ID.id)
			{
				if(pImp->AddSession(new session_enter_sanctuary(pImp)))
				{
					pImp->StartSession();
				}
			}
			else
			{
				XID pet_id = pImp->OI_GetPetID();
				if(es.id == pet_id.id)
					pImp->SendTo<0>(GM_MSG_PET_TEST_SANCTUARY, pet_id, 0);
			}
			_load_stats ++;

		}
		break;

		case C2S::ENTER_PK_PROTECTED:
		{
			C2S::CMD::enter_pk_protected & els = *(C2S::CMD::enter_pk_protected*) buf;
			if(size != sizeof(els))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			if(pImp->AddSession(new session_enter_pk_protected(pImp)))
			{
				pImp->StartSession();
			}
			_load_stats ++;
		}
		break;

		case C2S::OPEN_PERSONAL_MARKET:
		if(CheckDeny(CMD_MARKET)) return 0;
		if(!TestSafeLock()) return 0;
		{
			typedef C2S::CMD::open_personal_market cmd_t;
			cmd_t & pmo = *(cmd_t *) buf;
			if(size < sizeof(pmo) || !pmo.count || size != sizeof(pmo) + pmo.count * sizeof(cmd_t::entry_t))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			
			if(world_manager::GetWorldLimit().no_market)
			{
				error_cmd(S2C::ERR_CANNOT_OPEN_PLAYER_MARKET);
				break;
			}
		
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			
			if(((gplayer*)pImp->_parent)->IsInvisible())
			{
				error_cmd(S2C::ERR_OPERTION_DENYED_IN_INVISIBLE);
				break;
			}
	
			//试图建立摊位
			if(!pImp->PlayerOpenPersonalMarket(pmo.count,pmo.name ,(int *)pmo.list))
			{
				error_cmd(S2C::ERR_CANNOT_OPEN_PLAYER_MARKET);
			}
		}
		break;

		case C2S::QUERY_PERSONAL_MARKET_NAME:
		{
			typedef C2S::CMD::query_personal_market_name cmd_t;
			cmd_t & qpmn = *(cmd_t *) buf;
			if(size < sizeof(qpmn) || !qpmn.count || size != sizeof(qpmn) + qpmn.count * sizeof(int))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			if(qpmn.count > 128) qpmn.count = 128;
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			_load_stats += qpmn.count;

			//注：这个是一个非常耗费资源的操作 需要限制玩家的发送速度
			gplayer * pPlayer = (gplayer*)(pImp->_parent);
			MSG msg;
			int cs_index = pPlayer->cs_index;
			BuildMessage(msg,GM_MSG_QUERY_PERSONAL_MARKET_NAME,XID(-1,-1),pPlayer->ID,
						pPlayer->pos,pPlayer->cs_sid,&cs_index, sizeof(cs_index));

			//针对player id 进行群体发送
			pImp->_plane->SendPlayerMessage(qpmn.count , qpmn.list,msg);
			
		}
		break;

		case C2S::DESTROY_ITEM:
		{
			if(!TestSafeLock()) return 0;
			C2S::CMD::destroy_item  & di   = *(C2S::CMD::destroy_item *) buf;
			if(size != sizeof(di))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			//pImp->PlayerDestroyItem(di.where,di.index,di.type);
			pImp->_runner->unlock_inventory_slot(di.where, di.index);
		}
		break;

		case C2S::ENABLE_PVP_STATE:
		{
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->PlayerEnablePVPState();
		}
		break;

		case C2S::DISABLE_PVP_STATE:
		{
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->PlayerDisablePVPState();
		}
		break;

		case C2S::TEST_PERSONAL_MARKET:
		{
			if(!TestSafeLock()) return 0;

			if(world_manager::GetWorldLimit().no_market)
			{
				error_cmd(S2C::ERR_CANNOT_OPEN_PLAYER_MARKET);
				break;
			}
		
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			
			if(((gplayer*)pImp->_parent)->IsInvisible())
			{
				error_cmd(S2C::ERR_OPERTION_DENYED_IN_INVISIBLE);
				break;
			}
		
			if(!pImp->PlayerTestPersonalMarket())
			{
				error_cmd(S2C::ERR_CANNOT_OPEN_PLAYER_MARKET);
			}
		}
		break;

		case C2S::SWITCH_FASHION_MODE:
		{
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			if(pImp->_cooldown.TestCoolDown(COOLDOWN_INDEX_SWITCH_FASHION))
			{
				pImp->_cooldown.SetCoolDown(COOLDOWN_INDEX_SWITCH_FASHION,FASHION_COOLDOWN_TIME);
				pImp->SwitchFashionMode();
			}
		}
		break;

		case C2S::REGION_TRANSPORT:
		if(CheckDeny(CMD_MOVE)) return 0;
		{
			C2S::CMD::region_transport  & rt   = *(C2S::CMD::region_transport *) buf;
			if(size != sizeof(rt))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->PlayerRegionTransport(rt.region_index, rt.target_tag);
		}
		break;

		case C2S::NOTIFY_POS_TO_MEMBER:
		{
			_load_stats += 10;
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			if(!pImp->IsInTeam()) break;
			
			int member_count = pImp->_team.GetMemberNum();
			for(int i=0; i<member_count; i++)
			{
				const player_team::member_entry & ent = pImp->_team.GetMember(i);	
				if(ent.id == pImp->_parent->ID) continue;
				packet_wrapper  h1(64);
				using namespace S2C;
				CMD::Make<CMD::teammate_pos>::From(h1,pImp->_parent->ID,pImp->_parent->pos,world_manager::GetWorldTag(),
						ent.data.world_tag==world_manager::GetWorldTag() && ent.data.plane_index==pImp->_plane->w_plane_index);
				send_ls_msg(ent.cs_index, ent.id.id, ent.cs_sid,h1.data(),h1.size());
			}			
		}
		break;

		case C2S::CAST_POS_SKILL:
		if(CheckDeny(CMD_ATTACK)) return 0;
		{
			C2S::CMD::cast_pos_skill & cs = *(C2S::CMD::cast_pos_skill *)buf;
			if(size != sizeof(C2S::CMD::cast_pos_skill) + sizeof(int)*(size_t)cs.target_count)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			if(world_manager::GetGlobalController().CheckServerForbid(SERVER_FORBID_SKILL,cs.skill_id))
			{
				error_cmd(S2C::ERR_SKILL_NOT_AVAILABLE);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			//不检查选中对象的位置信息
			session_pos_skill *pSkill= new session_pos_skill(pImp);
			pSkill->SetTarget(cs.skill_id,cs.pos,cs.force_attack,cs.target_count,cs.targets);
			if(pImp->AddSession(pSkill)) pImp->StartSession();
		}
		break;
		
		case C2S::ACTIVE_RUSH_MODE:
		{
			C2S::CMD::active_rush_mode & cs = *(C2S::CMD::active_rush_mode *)buf;
			if(size != sizeof(C2S::CMD::active_rush_mode))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			if(cs.is_active && !pImp->CheckCoolDown(COOLDOWN_INDEX_RUSH_FLY))
			{
				error_cmd(S2C::ERR_SKILL_IS_COOLING);
				break;
			}
			//不检查选中对象的位置信息
			int mid = cs.is_active?FMID_SPEEDUP_FLY:FMID_NORMAL_FLY;
			pImp->_filters.ModifyFilter(FILTER_FLY_EFFECT,mid,NULL,0);
		}
		break;
		
		case C2S::QUERY_DOUBLE_EXP_INFO:
		{
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->_runner->available_double_exp_time();
			pImp->_runner->enable_double_exp_time(pImp->_double_exp_mode, pImp->_double_exp_timeout);
		}
		break;

		case C2S::DUEL_REQUEST:
		{
			C2S::CMD::duel_request & dr= *(C2S::CMD::duel_request *)buf;
			if(size != sizeof(C2S::CMD::duel_request))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}

			if(world_manager::GetWorldLimit().noduel)
			{
				error_cmd(S2C::ERR_HERE_CAN_NOT_DUEL);
				break;
			}

			XID target;
			MAKE_ID(target,dr.target);
			if(target.IsPlayer())
			{
				gplayer_imp * pImp = (gplayer_imp*)_imp;
				pImp->PlayerDuelRequest(target);
			}

		}
		break;

		case C2S::DUEL_REPLY:
		{
			C2S::CMD::duel_reply & dr= *(C2S::CMD::duel_reply *)buf;
			if(size != sizeof(C2S::CMD::duel_reply))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			if(world_manager::GetWorldLimit().noduel)
			{
				error_cmd(S2C::ERR_HERE_CAN_NOT_DUEL);
				break;
			}
			XID target;
			MAKE_ID(target,dr.who);
			if(target.IsPlayer())
			{
				gplayer_imp * pImp = (gplayer_imp*)_imp;
				pImp->PlayerDuelReply(target,dr.param);
			}
		}
		break;

		case C2S::BIND_PLAYER_REQUEST:
		{
			C2S::CMD::bind_player_request & bpr= *(C2S::CMD::bind_player_request *)buf;
			if(size != sizeof(bpr))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			XID target;
			MAKE_ID(target,bpr.who);
			if(target.IsPlayer())
			{
				gplayer_imp * pImp = (gplayer_imp*)_imp;
				pImp->PlayerBindRequest(target);
			}

		}
		break;

		case C2S::BIND_PLAYER_INVITE:
		{
			C2S::CMD::bind_player_invite & bpi= *(C2S::CMD::bind_player_invite *)buf;
			if(size != sizeof(bpi))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			XID target;
			MAKE_ID(target,bpi.who);
			if(target.IsPlayer())
			{
				gplayer_imp * pImp = (gplayer_imp*)_imp;
				pImp->PlayerBindInvite(target);
			}
		}
		break;

		case C2S::BIND_PLAYER_REQUEST_REPLY:
		{
			C2S::CMD::bind_player_request_reply & dr= *(C2S::CMD::bind_player_request_reply *)buf;
			if(size != sizeof(dr))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			XID target;
			MAKE_ID(target,dr.who);
			if(target.IsPlayer())
			{
				gplayer_imp * pImp = (gplayer_imp*)_imp;
				pImp->PlayerBindRequestReply(target,dr.param);
			}
		}
		break;

		case C2S::BIND_PLAYER_INVITE_REPLY:
		{
			C2S::CMD::bind_player_invite_reply & dr= *(C2S::CMD::bind_player_invite_reply *)buf;
			if(size != sizeof(dr))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			XID target;
			MAKE_ID(target,dr.who);
			if(target.IsPlayer())
			{
				gplayer_imp * pImp = (gplayer_imp*)_imp;
				pImp->PlayerBindInviteReply(target,dr.param);
			}
		}
		break;

		case C2S::QUERY_OTHER_EQUIP_DETAIL:
		{
			C2S::CMD::query_other_equip_detail & goed = *(C2S::CMD::query_other_equip_detail*)buf;
			if(sizeof(goed) != size)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}

			_load_stats ++;

			XID target;
			MAKE_ID(target,goed.target);
			if(target.IsPlayer())
			{
				gplayer_imp * pImp = (gplayer_imp*)_imp;
				if(!pImp->CheckCoolDown(COOLDOWN_INDEX_QUERY_OTHER_EQUIP))
				{
					error_cmd(S2C::ERR_SKILL_IS_COOLING);
					break;
				}

				struct 
				{
					int cs_index;
					int cs_sid;
				}data;

				gplayer * pPlayer = (gplayer*)(pImp->_parent);
				data.cs_index = pPlayer->cs_index;
				data.cs_sid = pPlayer->cs_sid;

				//发送查询消息
				pImp->SendTo<0>(GM_MSG_QUERY_EQUIP_DETAIL,target,
						pImp->GetFaction(), &data,sizeof(data));

				//设置冷却时间
				pImp->SetCoolDown(COOLDOWN_INDEX_QUERY_OTHER_EQUIP,QUERY_OTHER_EQUIP_COOLDOWN_TIME);
			}
		}
		break;

		case C2S::SUMMON_PET:
		{
			if(CheckDeny(CMD_PET)) return 0;
			C2S::CMD::summon_pet & sp = *(C2S::CMD::summon_pet *)buf;
			if(size != sizeof(C2S::CMD::summon_pet))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			int rst = pImp->PlayerSummonPet(sp.pet_index);
			if(rst)
			{
				error_cmd(rst);
			}
		}
		break;

		case C2S::RECALL_PET:
		{
			if(CheckDeny(CMD_PET)) return 0;
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			int rst = pImp->PlayerRecallPet();
			if(rst)
			{
				error_cmd(rst);
			}
		}
		break;

		case C2S::BANISH_PET:
		{
			if(CheckDeny(CMD_PET)) return 0;
			C2S::CMD::banish_pet & sp = *(C2S::CMD::banish_pet *)buf;
			if(size != sizeof(C2S::CMD::banish_pet))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			int rst = pImp->PlayerBanishPet(sp.pet_index);
			if(rst)
			{
				error_cmd(rst);
			}
		}
		break;

		case C2S::PET_CTRL_CMD:
		{
			C2S::CMD::pet_ctrl_cmd & pcc = *(C2S::CMD::pet_ctrl_cmd *)buf;
			if(size < sizeof(C2S::CMD::pet_ctrl_cmd))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->SendPetCommand(pcc.target, pcc.pet_cmd, pcc.buf, size - sizeof(C2S::CMD::pet_ctrl_cmd));
		}
		break;

		case C2S::MALL_SHOPPING:
		{                       
			C2S::CMD::mall_shopping & ms = *(C2S::CMD::mall_shopping *)buf;
			if(size < sizeof(ms) || ms.count == 0 ||  ms.count > 65535 ||
					size != sizeof(ms) + ms.count *sizeof(C2S::CMD::mall_shopping::__entry))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			//每次永远只购买一件商品
			if(world_manager::GetWorldParam().korea_shop || world_manager::GetWorldParam().southamerican_shop)
			{
				pImp->ForeignDoShoppingStep1(ms.list[0].goods_id, ms.list[0].goods_index, ms.list[0].goods_slot);
			}
			else
			{
				pImp->PlayerDoShopping(1, (const int *)&ms.list);
			}
		}       
		break;          

		case C2S::PLAYER_GIVE_PRESENT:
		{
			C2S::CMD::player_give_present & pg = *(C2S::CMD::player_give_present*)buf;
			if (size != sizeof(C2S::CMD::player_give_present))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			//韩国和南美不开放这个功能
			if(world_manager::GetWorldParam().korea_shop || world_manager::GetWorldParam().southamerican_shop)
			{
				error_cmd(S2C::ERR_SHOP_NOT_OPEN);
				break;
			}
			else
			{
				pImp->PlayerGivePresent(pg.roleid, pg.mail_id, pg.goods_id, pg.goods_index, pg.goods_slot);
			}
		}
		break;

		case C2S::PLAYER_ASK_FOR_PRESENT:
		{
			C2S::CMD::player_ask_for_present & pa = *(C2S::CMD::player_ask_for_present*)buf;
			if (size != sizeof(C2S::CMD::player_ask_for_present))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			//韩国和南美不开放这个功能
			if(world_manager::GetWorldParam().korea_shop || world_manager::GetWorldParam().southamerican_shop)
			{
				error_cmd(S2C::ERR_SHOP_NOT_OPEN);
				break;
			}
			else
			{
				pImp->PlayerAskForPresent(pa.roleid, pa.goods_id, pa.goods_index, pa.goods_slot);
			}
		}
		break;

		case C2S::USE_ITEM_WITH_ARG:
		if(CheckDeny(CMD_USE_ITEM)) return 0;
		{       
			C2S::CMD::use_item_with_arg & ui = *(C2S::CMD::use_item_with_arg *)buf;
			if(size < sizeof(C2S::CMD::use_item))
			{       
				error_cmd(S2C::ERR_FATAL_ERR);
				break;  
			}               
			size_t buf_size = size - sizeof(ui);
			if(buf_size > 256) break;

			if(world_manager::GetGlobalController().CheckServerForbid(SERVER_FORBID_ITEM,ui.item_id))
			{
				error_cmd(S2C::ERR_CANNOT_USE_ITEM);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			if(!pImp->PlayerUseItemWithArg(ui.where, ui.index, ui.item_id,ui.count,ui.arg, buf_size))
			{
				//改在物品里面发送这个错误信息了
				error_cmd(S2C::ERR_CANNOT_USE_ITEM);
			}
		}
		break;

		case C2S::QUERY_CASH_INFO:
		{       
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			if(world_manager::GetWorldParam().korea_shop) GNET::SendBillingBalance(pImp->_parent->ID.id);
			else if(world_manager::GetWorldParam().southamerican_shop) GNET::SendBillingBalanceSA(pImp->_parent->ID.id);
		}
		break;
		//lgc
		case C2S::ELF_ADD_ATTRIBUTE:
		{
			C2S::CMD::elf_add_attribute & eaa  = *(C2S::CMD::elf_add_attribute *)buf;
			if(size != sizeof(C2S::CMD::elf_add_attribute))
			{       
				error_cmd(S2C::ERR_FATAL_ERR);
				break;  
			}               
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->ElfAddAttribute(eaa.str, eaa.agi, eaa.vit, eaa.eng);	
			pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_EQUIPMENT, item::EQUIP_INDEX_ELF);
		}		
		break;

		case C2S::ELF_ADD_GENIUS:
		{
			C2S::CMD::elf_add_genius & eag = *(C2S::CMD::elf_add_genius*)buf;
			if(size != sizeof(C2S::CMD::elf_add_genius))
			{       
				error_cmd(S2C::ERR_FATAL_ERR);
				break;  
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->ElfAddGenius(eag.genius[0], eag.genius[1], eag.genius[2], eag.genius[3], eag.genius[4]);	
			pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_EQUIPMENT, item::EQUIP_INDEX_ELF);
		}
		break;
		
		case C2S::ELF_PLAYER_INSERT_EXP:
		{
			C2S::CMD::elf_player_insert_exp & epie = *(C2S::CMD::elf_player_insert_exp*)buf;
			if(size != sizeof(C2S::CMD::elf_player_insert_exp))
			{       
				error_cmd(S2C::ERR_FATAL_ERR);
				break;  
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->ElfPlayerInsertExp(epie.exp, epie.use_sp);
			pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_EQUIPMENT, item::EQUIP_INDEX_ELF);
		}
		break;

		case C2S::ELF_EQUIP_ITEM:
		{
			C2S::CMD::elf_equip_item & eei = *(C2S::CMD::elf_equip_item*)buf;
			if(size != sizeof(C2S::CMD::elf_equip_item))
			{       
				error_cmd(S2C::ERR_FATAL_ERR);
				break;  
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->ElfEquipItem(eei.index_inv);
			pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_INVENTORY, eei.index_inv);
			pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_EQUIPMENT, item::EQUIP_INDEX_ELF);
		}
		break;

		case C2S::ELF_CHANGE_SECURE_STATUS:
		{
			C2S::CMD::elf_change_secure_status & ecss = *(C2S::CMD::elf_change_secure_status*)buf;
			if(size != sizeof(C2S::CMD::elf_change_secure_status))
			{       
				error_cmd(S2C::ERR_FATAL_ERR);
				break;  
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->ElfChangeSecureStatus(ecss.status);
			pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_EQUIPMENT, item::EQUIP_INDEX_ELF);
		}
		break;
		
		case C2S::CAST_ELF_SKILL:
		if(CheckDeny(CMD_ATTACK)) return 0;
		if(CheckDeny(CMD_ELF_SKILL)) return 0;
		{
			C2S::CMD::cast_elf_skill & ces = *(C2S::CMD::cast_elf_skill*)buf;
			if(size != sizeof(C2S::CMD::cast_elf_skill)+sizeof(int)*ces.target_count )
			{       
				error_cmd(S2C::ERR_FATAL_ERR);
				break;  
			}
			if(world_manager::GetGlobalController().CheckServerForbid(SERVER_FORBID_SKILL,ces.skill_id))
			{
				error_cmd(S2C::ERR_SKILL_NOT_AVAILABLE);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->CastElfSkill(ces.skill_id, ces.force_attack, ces.target_count, ces.targets);
			pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_EQUIPMENT, item::EQUIP_INDEX_ELF);
		}
		break;
		
		case C2S::RECHARGE_EQUIPPED_ELF:
		{
			C2S::CMD::recharge_equipped_elf & ree = *(C2S::CMD::recharge_equipped_elf *)buf;
			if(size != sizeof(C2S::CMD::recharge_equipped_elf))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			if(!pImp->RechargeEquippedElf(ree.element_index,ree.count))
			{
				error_cmd(S2C::ERR_CANNOT_RECHARGE);
			}
			pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_INVENTORY, ree.element_index);
			pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_EQUIPMENT, item::EQUIP_INDEX_ELF);
		}
		break;

		case C2S::GET_MALL_ITEM_PRICE:
		{
			C2S::CMD::get_mall_item_price & gmip = *(C2S::CMD::get_mall_item_price*)buf;
			if(size != sizeof(C2S::CMD::get_mall_item_price))
			{
				error_cmd(S2C::ERR_FATAL_ERR);	
				break;
			}
			//韩服不开放商城组控制，销售时间控制
			if(world_manager::GetWorldParam().korea_shop || world_manager::GetWorldParam().southamerican_shop) break;

			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->PlayerGetMallItemPrice(gmip.start_index, gmip.end_index);
		}
		break;
		
		case C2S::EQUIP_TRASHBOX_FASHION_ITEM:
		{
			C2S::CMD::equip_trashbox_fashion_item & etfi = *(C2S::CMD::equip_trashbox_fashion_item*)buf;
			if(size != sizeof(C2S::CMD::equip_trashbox_fashion_item))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}

			gplayer_imp * pImp = (gplayer_imp*)_imp;
			if(!pImp->PlayerEquipTrashboxFashionItem(etfi.trash_idx_body,etfi.trash_idx_leg,etfi.trash_idx_foot,etfi.trash_idx_wrist,etfi.trash_idx_head,etfi.trash_idx_weapon))
			{
				error_cmd(S2C::ERR_CANNOT_EQUIP_TRASHBOX_FASHION_ITEM);	
			}
			pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_TRASH_BOX3, etfi.trash_idx_body);
			pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_TRASH_BOX3, etfi.trash_idx_leg);
			pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_TRASH_BOX3, etfi.trash_idx_foot);
			pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_TRASH_BOX3, etfi.trash_idx_wrist);
			pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_TRASH_BOX3, etfi.trash_idx_head);
			pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_TRASH_BOX3, etfi.trash_idx_weapon);
			pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_EQUIPMENT, item::EQUIP_INDEX_FASHION_BODY);
			pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_EQUIPMENT, item::EQUIP_INDEX_FASHION_LEG);
			pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_EQUIPMENT, item::EQUIP_INDEX_FASHION_FOOT);
			pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_EQUIPMENT, item::EQUIP_INDEX_FASHION_WRIST);
			pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_EQUIPMENT, item::EQUIP_INDEX_FASHION_HEAD);
			pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_EQUIPMENT, item::EQUIP_INDEX_FASHION_WEAPON);
		}
		break;
	
		case C2S::CHECK_SECURITY_PASSWD:
		{
			C2S::CMD::check_security_passwd & csp = *(C2S::CMD::check_security_passwd*)buf;
			if(size < sizeof(C2S::CMD::check_security_passwd) || size != sizeof(C2S::CMD::check_security_passwd) + csp.passwd_size)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;	
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->PlayerCheckSecurityPasswd(csp.passwd, csp.passwd_size);
		}
		break;

		case C2S::NOTIFY_FORCE_ATTACK:
		{
			C2S::CMD::notify_force_attack & nfa = *(C2S::CMD::notify_force_attack*)buf;
			if(size != sizeof(C2S::CMD::notify_force_attack))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->PlayerNotifyForceAttack(nfa.force_attack,nfa.refuse_bless);
		}
		break;

		case C2S::DIVIDEND_MALL_SHOPPING:
		{                       
			C2S::CMD::dividend_mall_shopping & dms = *(C2S::CMD::dividend_mall_shopping *)buf;
			if(size < sizeof(dms) || dms.count == 0 ||  dms.count > 65535 ||
					size != sizeof(dms) + dms.count *sizeof(C2S::CMD::dividend_mall_shopping::__entry))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			if(world_manager::GetWorldParam().korea_shop || world_manager::GetWorldParam().southamerican_shop) break;
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			//每次永远只购买一件商品
			pImp->PlayerDoDividendShopping(1, (const int *)&dms.list);
		}       
		break;          

		case C2S::GET_DIVIDEND_MALL_ITEM_PRICE:
		{
			C2S::CMD::get_dividend_mall_item_price & gmip = *(C2S::CMD::get_dividend_mall_item_price*)buf;
			if(size != sizeof(C2S::CMD::get_dividend_mall_item_price))
			{
				error_cmd(S2C::ERR_FATAL_ERR);	
				break;
			}
			if(world_manager::GetWorldParam().korea_shop || world_manager::GetWorldParam().southamerican_shop) break;
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->PlayerGetDividendMallItemPrice(gmip.start_index, gmip.end_index);
		}
		break;
	
		case C2S::CHOOSE_MULTI_EXP:
		{
			C2S::CMD::choose_multi_exp & cme = *(C2S::CMD::choose_multi_exp*)buf;	
			if(size != sizeof(C2S::CMD::choose_multi_exp))
			{
				error_cmd(S2C::ERR_FATAL_ERR);	
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->PlayerChooseMultiExp(cme.index);
		}
		break;

		case C2S::TOGGLE_MULTI_EXP:
		{
			C2S::CMD::toggle_multi_exp & tme = *(C2S::CMD::toggle_multi_exp*)buf;	
			if(size != sizeof(C2S::CMD::toggle_multi_exp))
			{
				error_cmd(S2C::ERR_FATAL_ERR);	
				break;
			}
			//某些场景不允许使用多倍经验	
			if(world_manager::GetWorldLimit().no_multi_exp)
			{
				error_cmd(S2C::ERR_CANNOT_TOGGLE_MULTI_EXP);
				break;	
			}
			
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			if(tme.is_activate)
				pImp->_multi_exp_ctrl.ActivateMultiExp(pImp);
			else
				pImp->_multi_exp_ctrl.DeactivateMultiExp(pImp);
		}
		break;
	
		case C2S::MULTI_EXCHANGE_ITEM:
		{
			C2S::CMD::multi_exchange_item & mei = *(C2S::CMD::multi_exchange_item*)buf;
			if(size < sizeof(C2S::CMD::multi_exchange_item) || !mei.count 
				|| size != sizeof(C2S::CMD::multi_exchange_item) + mei.count*sizeof(C2S::CMD::multi_exchange_item::_operation))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}

			gplayer_imp * pImp = (gplayer_imp*)_imp;

			if(!pImp->CheckCoolDown(COOLDOWN_INDEX_MULTI_EXCHANGE_ITEM)) break;
			pImp->SetCoolDown(COOLDOWN_INDEX_MULTI_EXCHANGE_ITEM,MULTI_EXCHANGE_ITEM_COOLDOWN_TIME);
			
			switch(mei.location)
			{
			case gplayer_imp::IL_INVENTORY:
				for(size_t i=0; i<mei.count; i++)
					pImp->PlayerExchangeInvItem(mei.operations[i].index1,mei.operations[i].index2);
			break;
				
			case gplayer_imp::IL_TRASH_BOX:
			case gplayer_imp::IL_TRASH_BOX2:
				if(!pImp->_trash_box_open_flag) break;
			case gplayer_imp::IL_TRASH_BOX3:
			case gplayer_imp::IL_TRASH_BOX4:
				for(size_t i=0; i<mei.count; i++)
					pImp->PlayerExchangeTrashItem(mei.location,mei.operations[i].index1,mei.operations[i].index2);
			break;
			
			case gplayer_imp::IL_USER_TRASH_BOX:
				if(!pImp->_user_trash_box_open_flag) break;
				for(size_t i=0; i<mei.count; i++)
					pImp->PlayerExchangeTrashItem(mei.location,mei.operations[i].index1,mei.operations[i].index2);
			break;
			
			default:
			break;
			}
		}
		break;

		case C2S::SYSAUCTION_OP:
		{
			C2S::CMD::sysauction_op & so = *(C2S::CMD::sysauction_op*)buf;
			if(size < sizeof(C2S::CMD::sysauction_op)) break;
			if(world_manager::GetWorldParam().korea_shop || world_manager::GetWorldParam().southamerican_shop) break;
			GNET::ForwardSysAuctionOP(so.type, so.buf, size-sizeof(C2S::CMD::sysauction_op), object_interface((gplayer_imp*)_imp));
		}
		break;

		case C2S::CALC_NETWORK_DELAY:
		{
			C2S::CMD::calc_network_delay & cnd = *(C2S::CMD::calc_network_delay*)buf;
			if(size != sizeof(C2S::CMD::calc_network_delay)) break;
			if(cnd.timestamp)
			{
				_imp->_runner->calc_network_delay(cnd.timestamp);
			}
			else	//目前仅用于测试
			{
				gplayer_imp * pImp = (gplayer_imp *)_imp;	
				if(pImp->HasSession()) break;
				pImp->AddSession(new session_test(pImp));
				pImp->StartSession();
			}
		}
		break;
		
		case C2S::GET_FACTION_FORTRESS_INFO:
		{
			//C2S::CMD::get_faction_fortress_info & gffi = *(C2S::CMD::get_faction_fortress_info*)buf;
			if(size != sizeof(C2S::CMD::get_faction_fortress_info)) break;
			((gplayer_imp*)_imp)->PlayerGetFactionFortressInfo();
		}
		break;

		case C2S::CONGREGATE_REPLY:
		{
			C2S::CMD::congregate_reply & reply = *(C2S::CMD::congregate_reply*)buf;
			if(size != sizeof(C2S::CMD::congregate_reply)) break;
			((gplayer_imp*)_imp)->PlayerCongregateReply(reply.type, reply.agree, reply.sponsor);
		}
		break;
		
		case C2S::GET_FORCE_GLOBAL_DATA:
		{
			//C2S::CMD::get_force_global_data & gfgd = *(C2S::CMD::get_force_global_data*)buf;
			if(size != sizeof(C2S::CMD::get_force_global_data)) break;
			size_t count;
			const void * data;
			size_t data_size;
			bool data_ready = world_manager::GetForceGlobalDataMan().GetData(count,&data,data_size);
			_imp->_runner->send_force_global_data(data_ready,count,data,data_size);
		}
		break;

		case C2S::PRODUCE4_CHOOSE:
		{
			if(size != sizeof(C2S::CMD::produce4_choose)) break;
			C2S::CMD::produce4_choose & pc = *(C2S::CMD::produce4_choose*)buf;
			((gplayer_imp*)_imp)->PlayerProduce4Choose((bool)pc.remain);
		}
		break;
		
		case C2S::RECHARGE_ONLINE_AWARD:
		{
			C2S::CMD::recharge_online_award & roa = *(C2S::CMD::recharge_online_award*)buf;
			if(size < sizeof(C2S::CMD::recharge_online_award) || !roa.count || roa.count > 32
					|| size != sizeof(C2S::CMD::recharge_online_award) + roa.count * sizeof(C2S::CMD::recharge_online_award::entry))
				break;

			((gplayer_imp *)_imp)->PlayerRechargeOnlineAward(roa.type, roa.count, (int *)roa.list);
		}
		break;
		
		case C2S::TOGGLE_ONLINE_AWARD:
		{
			C2S::CMD::toggle_online_award & toa = *(C2S::CMD::toggle_online_award*)buf;
			if(size != sizeof(C2S::CMD::toggle_online_award)) break;

			gplayer_imp * pImp = (gplayer_imp *)_imp;
			if(!pImp->CheckCoolDown(COOLDOWN_INDEX_TOGGLE_ONLINE_AWARD)) break;
			pImp->SetCoolDown(COOLDOWN_INDEX_TOGGLE_ONLINE_AWARD,TOGGLE_ONLINE_AWARD_COOLDOWN_TIME);
			if(toa.activate)
				pImp->_online_award.ActivateAward(pImp, toa.type);
			else
				pImp->_online_award.DeactivateAward(pImp, toa.type);
		}
		break;

		case C2S::QUERY_PROFIT_TIME:
		{
			if(size != sizeof(C2S::CMD::query_profit_time)) break;
			gplayer_imp * pImp = (gplayer_imp *)_imp;
			pImp->_runner->update_profit_time(S2C::CMD::player_profit_time::PLAYER_QUERY, pImp->_profit_time, pImp->_profit_level);
		}
		break;

		case C2S::COUNTRYBATTLE_GET_PERSONAL_SCORE:
		{
			if(size != sizeof(C2S::CMD::countrybattle_get_personal_score)) break;
			gplayer_imp * pImp = (gplayer_imp *)_imp;
			pImp->PlayerGetCountryBattlePersonalScore();
		}
		break;

		case C2S::GET_SERVER_TIMESTAMP:
		{
			_imp->_runner->send_timestamp(); 
		}
		break;
		
		case C2S::COUNTRYBATTLE_LEAVE:
		{
			gplayer_imp * pImp = (gplayer_imp *)_imp;
			pImp->PlayerLeaveCountryBattle();
		}
		break;
		
		case C2S::GET_CASH_MONEY_EXCHG_RATE:
		{
			bool open = world_manager::GetGlobalController().GetCashMoneyExchangeOpen();
			int rate = world_manager::GetGlobalController().GetCashMoneyExchangeRate();
			_imp->_runner->cash_money_exchg_rate(open?1:0, rate);
		}
		break;
		
		case C2S::EVOLUTION_PET:
		{
			//return 0;
			if(CheckDeny(CMD_PET)) return 0;
			C2S::CMD::evolution_pet & ep = *(C2S::CMD::evolution_pet *)buf;
			if(size != sizeof(C2S::CMD::evolution_pet))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			if(!TestSafeLock()) return 0;
			gplayer_imp *pImp = (gplayer_imp*)_imp;
			int rst = pImp->PlayerEvolutionPet(ep.pet_index,ep.formula_index);
			if(rst)
			{
				error_cmd(rst);
			}
		}
		break;
		
		case C2S::ADD_PET_EXP:
		{
			if(CheckDeny(CMD_PET)) return 0;
			C2S::CMD::add_pet_exp &ape = *(C2S::CMD::add_pet_exp *)buf;
			if(size != sizeof(C2S::CMD::add_pet_exp))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			if(!TestSafeLock()) return 0;
			gplayer_imp *pImp = (gplayer_imp*)_imp;
			int rst = pImp->PlayerAddPetExp(ape.pet_index,ape.item_num);
			if(rst)
			{
				error_cmd(rst);
			}
		}
		break;
		
		case C2S::REBUILD_PET_NATURE:
		{
			if(CheckDeny(CMD_PET)) return 0;
			C2S::CMD::rebuild_pet_nature &rpn = *(C2S::CMD::rebuild_pet_nature *)buf;
			if(size != sizeof(C2S::CMD::rebuild_pet_nature))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			if(!TestSafeLock()) return 0;
			gplayer_imp *pImp = (gplayer_imp*)_imp;
			int rst = pImp->PlayerRebuildPetNature(rpn.pet_index,rpn.formula_index);
			if(rst)
			{
				error_cmd(rst);
			}
		}
		break;
		
		case C2S::REBUILD_PET_INHERIT_RATIO: 
		{
			if(CheckDeny(CMD_PET)) return 0;
			C2S::CMD::rebuild_pet_inherit_ratio &rpir = *(C2S::CMD::rebuild_pet_inherit_ratio *)buf;
			if(size != sizeof(C2S::CMD::rebuild_pet_inherit_ratio))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			if(!TestSafeLock()) return 0;
			gplayer_imp *pImp = (gplayer_imp*)_imp;
			int rst = pImp->PlayerRebuildPetInheritRatio(rpir.pet_index,rpir.formula_index);
			if(rst)
			{
				error_cmd(rst);
			}
		}
		break;
		
		case C2S::PET_REBUILDINHERIT_CHOOSE:
		{
			if(CheckDeny(CMD_PET)) return 0;
			C2S::CMD::pet_rebuildinherit_choose &prc = *(C2S::CMD::pet_rebuildinherit_choose *)buf;
			if(size != sizeof(C2S::CMD::pet_rebuildinherit_choose))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp *pImp = (gplayer_imp*)_imp;
			pImp->PlayerAcceptRebuildInheritResult((bool)prc.isaccept);
		}
		break;
		
		case C2S::PET_REBUILDNATURE_CHOOSE:
		{
			//return 0;
			if(CheckDeny(CMD_PET)) return 0;
			C2S::CMD::pet_rebuildnature_choose &prc = *(C2S::CMD::pet_rebuildnature_choose *)buf;
			if(size != sizeof(C2S::CMD::pet_rebuildnature_choose))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp *pImp = (gplayer_imp*)_imp;
			pImp->PlayerAcceptRebuildNatureResult((bool)prc.isaccept);
		}
		break;
		
		case C2S::TRY_REFINE_MERIDIAN:
		{
			C2S::CMD::try_refine_meridian &trm = *(C2S::CMD::try_refine_meridian *)buf;
			if(size != sizeof(trm))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			if(!TestSafeLock()) return 0;
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			int rst = pImp->PlayerTryRefineMeridian(trm.index);
			if(rst)
			{
				error_cmd(rst);
			}
		}
		break;

		case C2S::COUNTRYBATTLE_GET_STRONGHOLD_STATE:
		{
			if(size != sizeof(C2S::CMD::countrybattle_get_stronghold_state)) break;
			gplayer_imp * pImp = (gplayer_imp *)_imp;
			pImp->PlayerGetCountryBattleStrongholdState();
		}
		break;

		case C2S::QUERY_TOUCH_POINT:
		{
			if(size != sizeof(C2S::CMD::query_touch_point)) break;
			((gplayer_imp*)_imp)->PlayerTouchPointQuery();
		}
		break;

		case C2S::COST_TOUCH_POINT:
		{
			if(size != sizeof(C2S::CMD::cost_touch_point)) break;
			C2S::CMD::cost_touch_point & ct = *(C2S::CMD::cost_touch_point*)buf;

			((gplayer_imp*)_imp)->PlayerTouchPointCost(ct.index,ct.itemid,ct.count,ct.price,ct.expiretime,ct.lots);
		}
		break;

		case C2S::QUERY_TITLE:
		{
			if(size != sizeof(C2S::CMD::query_title)) break;
			C2S::CMD::query_title & ct = *(C2S::CMD::query_title*)buf;
			
			((gplayer_imp*)_imp)->PlayerQueryTitle(ct.roleid);
		}
		break;

		case C2S::CHANGE_CURR_TITLE:
		{
			if(size != sizeof(C2S::CMD::change_title)) break;
			C2S::CMD::change_title & ct = *(C2S::CMD::change_title*)buf;

			((gplayer_imp*)_imp)->PlayerChangeTitle(ct.titleid);
		}
		break;

		case C2S::DAILY_SIGNIN:
		{
			if(size != sizeof(C2S::CMD::daily_signin)) break;

			((gplayer_imp*)_imp)->PlayerDailySignin();
		}
		break;

		case C2S::LATE_SIGNIN:
		{
			if(size != sizeof(C2S::CMD::late_signin)) break;
			C2S::CMD::late_signin & ct = *(C2S::CMD::late_signin*)buf;

			((gplayer_imp*)_imp)->PlayerLateSignin(ct.type, ct.itempos, ct.desttime);
		}
		break;

		case C2S::APPLY_SIGNIN_AWARD:
		{
			if(size != sizeof(C2S::CMD::apply_signinaward)) break;
			C2S::CMD::apply_signinaward & ct = *(C2S::CMD::apply_signinaward*)buf;

			((gplayer_imp*)_imp)->PlayerApplySigninAward(ct.type, ct.mon);
		}
		break;

		case C2S::REFRESH_SIGNIN:
		{
			if(size != sizeof(C2S::CMD::refresh_signin)) break;

			((gplayer_imp*)_imp)->PlayerRefreshSignin();
		}
		break;

		case C2S::SWITCH_IN_PARALLEL_WORLD:
		{
			if(size != sizeof(C2S::CMD::switch_in_parallel_world)) break;
			C2S::CMD::switch_in_parallel_world & sw = *(C2S::CMD::switch_in_parallel_world *)buf;

			instance_hash_key hkey;
			hkey.key1 = sw.key;
			hkey.key2 = 0;
			((gplayer_imp*)_imp)->PlayerSwitchInParallelWorld(hkey);
		}
		break;

		case C2S::QUERY_PARALLEL_WORLD:
		{
			if(size != sizeof(C2S::CMD::query_parallel_world)) break;

			((gplayer_imp*)_imp)->PlayerQueryParallelWorld();
		}
		break;
		
		case C2S::QUERY_UNIQUE_DATA:
		{
			if(size < sizeof(C2S::CMD::query_unique_data)) break;
			C2S::CMD::query_unique_data & ct = *(C2S::CMD::query_unique_data*)buf;
			if(size != (sizeof(C2S::CMD::query_unique_data) + ct.count*sizeof(int)) ) break;

			world_manager::GetUniqueDataMan().OnRoleQuery((gplayer*)_imp->_parent,ct.count,ct.keys);
		}
		break;
	
		case C2S::GET_REINCARNATION_TOME:
		{
			if(size != sizeof(C2S::CMD::get_reincarnation_tome)) break;
	
			((gplayer_imp*)_imp)->PlayerGetReincarnationTome();
		}
		break;
		
		case C2S::REWRITE_REINCARNATION_TOME:
		{
			if(size != sizeof(C2S::CMD::rewrite_reincarnation_tome)) break;
			C2S::CMD::rewrite_reincarnation_tome & rrt = *(C2S::CMD::rewrite_reincarnation_tome *)buf;

			((gplayer_imp*)_imp)->PlayerRewriteReincarnationTome(rrt.record_index, rrt.record_level);
		}
		break;
	
		case C2S::ACTIVATE_REINCARNATION_TOME:
		{
			if(size != sizeof(C2S::CMD::activate_reincarnation_tome)) break;
			C2S::CMD::activate_reincarnation_tome & art = *(C2S::CMD::activate_reincarnation_tome *)buf;

			((gplayer_imp*)_imp)->PlayerActiveReincarnationTome(art.active != 0);
		}
		break;

		case C2S::AUTO_TEAM_SET_GOAL:
		{
			if(size != sizeof(C2S::CMD::auto_team_set_goal)) break;
			C2S::CMD::auto_team_set_goal& goal = *(C2S::CMD::auto_team_set_goal*)buf;

			((gplayer_imp*)_imp)->PlayerSetAutoTeamGoal(goal.goal_type, goal.op, goal.goal_id);
		}
		break;
		
		case C2S::AUTO_TEAM_JUMP_TO_GOAL:
		{
			if(size != sizeof(C2S::CMD::auto_team_jump_to_goal)) break;
			C2S::CMD::auto_team_jump_to_goal& goal = *(C2S::CMD::auto_team_jump_to_goal*)buf;

			((gplayer_imp*)_imp)->PlayerJumpToGoal(goal.goal_id);
		}

		case C2S::TRICKBATTLE_LEAVE:
		{
			gplayer_imp * pImp = (gplayer_imp *)_imp;
			pImp->PlayerLeaveTrickBattle();
		}
		break;

		case C2S::TRICKBATTLE_UPGRADE_CHARIOT:
		{
			if(size != sizeof(C2S::CMD::trickbattle_upgrade_chariot)) break;
			C2S::CMD::trickbattle_upgrade_chariot & cmd = *(C2S::CMD::trickbattle_upgrade_chariot *)buf;
			
			((gplayer_imp *)_imp)->TrickBattleUpgradeChariot(cmd.chariot);	
		}
		break;
	
		case C2S::SWALLOW_GENERALCARD:
		{
			if(size != sizeof(C2S::CMD::swallow_generalcard)) break;
			C2S::CMD::swallow_generalcard & cmd = *(C2S::CMD::swallow_generalcard *)buf;

			int rst = ((gplayer_imp *)_imp)->PlayerSwallowGeneralCard(cmd.equip_idx, cmd.is_inv, cmd.inv_idx, cmd.count);
			if(rst > 0)
			{
				error_cmd(rst);
			}
		}
		break;
		
		case C2S::EQUIP_TRASHBOX_ITEM:
		{
			if(size != sizeof(C2S::CMD::equip_trashbox_item)) break;
			C2S::CMD::equip_trashbox_item & cmd = *(C2S::CMD::equip_trashbox_item *)buf;

			//暂时只允许从卡牌仓库装备物品
			if(cmd.where != gplayer_imp::IL_TRASH_BOX4) break;
			
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			if(!pImp->PlayerEquipTrashboxItem(cmd.where, cmd.trash_idx, cmd.equip_idx))
			{
				error_cmd(S2C::ERR_CANNOT_EQUIP_TRASHBOX_ITEM);
			}
			pImp->_runner->unlock_inventory_slot(cmd.where, cmd.trash_idx);
			pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_EQUIPMENT, cmd.equip_idx);
		}
		break;

		case C2S::QUERY_TRICKBATTLE_CHARIOTS:
		{
			gplayer_imp * pImp = (gplayer_imp *)_imp;
			pImp->PlayerQueryChariots();
		}
		break;
		
		case C2S::COUNTRYBATTLE_LIVE_SHOW:
		{
			gplayer_imp * pImp = (gplayer_imp *)_imp;
			pImp->PlayerGetCountryBattleLiveShow();
		}
		break;

		case C2S::SEND_MASS_MAIL:
		{
			C2S::CMD::send_mass_mail & req = *(C2S::CMD::send_mass_mail*)buf;
			gplayer_imp * pImp = (gplayer_imp *)_imp;
			GNET::ForwardMailSysOP(req.service_id,req.data, size - sizeof(C2S::CMD::send_mass_mail),object_interface(pImp));
		}
		break;

		case C2S::RANDOM_MALL_SHOPPING:
		{
			if(world_manager::GetWorldParam().korea_shop || world_manager::GetWorldParam().southamerican_shop) break;
			if(world_manager::GetWorldParam().random_shop_limit) break;
			if(size != sizeof(C2S::CMD::random_mall_shopping)) break;
			C2S::CMD::random_mall_shopping & cmd = *(C2S::CMD::random_mall_shopping *)buf;

			gplayer_imp * pImp = (gplayer_imp *)_imp; 
			
			switch(cmd.opt)
			{
				case random_mall_info::RAND_MALL_OPT_QUERY: 
					pImp->PlayerRandMallQuery(cmd.entryid);
					break;
				case random_mall_info::RAND_MALL_OPT_ROLL: 
					pImp->PlayerRandMallRoll(cmd.entryid);
					break;
				case random_mall_info::RAND_MALL_OPT_PAY: 
					pImp->PlayerRandMallPay(cmd.entryid);
					break;			
			}			
		}
		break;
		case C2S::QUERY_MAFIA_PVP_INFO:
		{
			gplayer_imp * pImp = (gplayer_imp *)_imp;
			pImp->PlayerQueryMafiaPvPInfo();
		}
		break;
        case C2S::QUERY_CAN_INHERIT_ADDONS:
        {   
            if(size != sizeof(C2S::CMD::query_can_inherit_addons)) break;
            C2S::CMD::query_can_inherit_addons& cmd = *(C2S::CMD::query_can_inherit_addons*)buf;

            gplayer_imp* pImp = (gplayer_imp*)_imp;
            pImp->PlayerQueryCanInheritAddons(cmd.equip_id, cmd.inv_idx);
        }
        break;
        case C2S::ACTIVATE_REGION_WAYPOINTS:
        {
            C2S::CMD::activate_region_waypoints& cmd = *(C2S::CMD::activate_region_waypoints*)buf;
            if((size < sizeof(C2S::CMD::activate_region_waypoints)) || (size != sizeof(C2S::CMD::activate_region_waypoints) + cmd.num * sizeof(int))) break;

            gplayer_imp* pImp = (gplayer_imp*)_imp;
            pImp->ActivateRegionWayPoints(cmd.num, cmd.waypoints);
        }
        break;
		case C2S::INSTANCE_REENTER_REQUEST:
		{
			C2S::CMD::instance_reenter_request& cmd =  *(C2S::CMD::instance_reenter_request*)buf;
			if(size != sizeof(C2S::CMD::instance_reenter_request)) break;
			if(cmd.agree_reenter)
			{
				gplayer_imp* pImp = (gplayer_imp*)_imp;
				pImp->PlayerReenterInstance();
			}
		}
		break;

	default:
		
		if(_debug_command_enable && cmd_type == C2S::GOTO)
		{
			return DebugCommandHandler(cmd_type,buf,size);
		}
		
		if(cmd_type == C2S::GOTO || cmd_type > C2S::GM_COMMAND_START && cmd_type < C2S::GM_COMMAND_END)
		{
			return GMCommandHandler(cmd_type,buf,size);
		}

		if(_debug_command_enable)
		{
			return DebugCommandHandler(cmd_type,buf,size);
		}
		else
		{
			_load_stats += 10;
			__PRINTF("收到无法辨识的命令 %d\n",cmd_type);
		}
		break;
	}
	return 0;
}

// ---------------------分割线-----------------------------------------------

int 
gplayer_controller::GMCommandHandler(int cmd_type,const void * buf, size_t size)
{
#define DEFCMD(type) C2S::CMD::type & cmd = *(C2S::CMD::type*)buf; \
			if (size != sizeof(cmd))\
			{\
				error_cmd(S2C::ERR_FATAL_ERR);\
				break;\
			}
#define TESTGMPRIVILEGE(X) if(!_gm_auth->X()) \
			{\
				error_cmd(S2C::ERR_INVALID_PRIVILEGE); \
				break;\
			}
	ASSERT(cmd_type > C2S::GM_COMMAND_START && cmd_type < C2S::GM_COMMAND_END || cmd_type == C2S::GOTO);
	if(!_gm_auth) 
	{
		_load_stats += 3;
		return 0;
	}

	switch(cmd_type)
	{
		case C2S::GMCMD_MOVE_TO_PLAYER:
		{
			TESTGMPRIVILEGE(Has_MoveTo_Role)

			DEFCMD(gmcmd_move_to_player)
			XID target(GM_TYPE_PLAYER,cmd.id);

			//现在直接发送查询消息
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			//记录日志
			GLog::log(GLOG_INFO,"GM:%d试图移动到玩家%d处",pImp->_parent->ID.id,target.id);

			pImp->SendTo<0>(GM_MSG_GM_MQUERY_MOVE_POS,target,0);
		}
		break;

		case C2S::GMCMD_RECALL_PLAYER:
		{
			TESTGMPRIVILEGE(Has_Fetch_Role)

			DEFCMD(gmcmd_recall_player)
			XID target(GM_TYPE_PLAYER,cmd.id);
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			//记录日志
			const A3DVECTOR &pos = pImp->_parent->pos;
			GLog::log(GLOG_INFO,"GM:%d将玩家%d移动过来(%f,%f,%f,)",pImp->_parent->ID.id,target.id,pos.x,pos.y,pos.z);
			//发送一个消息过去要求玩家跳转过来
			pImp->SendTo<0>(GM_MSG_GM_RECALL,target,world_manager::GetWorldTag());
			
		}
		break;

		case C2S::GMCMD_OFFLINE:
		{
			//此命令已经被屏蔽，将在link上实现
			TESTGMPRIVILEGE(Has_Force_Offline)

			DEFCMD(gmcmd_offline)
			if(_cur_target.type  != GM_TYPE_PLAYER)
			{
				error_cmd(S2C::ERR_INVALID_TARGET);
				break;
			}
			//发送一个消息下线
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->SendTo<0>(GM_MSG_GM_OFFLINE,_cur_target,0);
		}
		break;


		case C2S::GMCMD_TOGGLE_INVISIBLE:
		{
			TESTGMPRIVILEGE(Has_Hide_BeGod)

			gactive_imp *pImp = (gactive_imp *)_imp;
			bool bRst = pImp->_filters.IsFilterExist(FILTER_INDEX_GM_INVISIBLE);
			if(bRst)
				pImp->_filters.RemoveFilter(FILTER_INDEX_GM_INVISIBLE);
			else
				pImp->_filters.AddFilter(new gm_invisible_filter(pImp));

			//记录日志
			GLog::log(GLOG_INFO,"GM:%d切换了隐身状态(%s)", _imp->_parent->ID.id,bRst?"现形":"隐身");
		}
		break;

		case C2S::GMCMD_TOGGLE_INVINCIBLE:
		{
			TESTGMPRIVILEGE(Has_Hide_BeGod)

			gactive_imp *pImp = (gactive_imp *)_imp;
			bool bRst = pImp->_filters.IsFilterExist(FILTER_INVINCIBLE);
			if(bRst)
			{
				pImp->_filters.RemoveFilter(FILTER_INVINCIBLE);
			}
			else
			{
				pImp->_filters.AddFilter(new invincible_filter(pImp,FILTER_INVINCIBLE));
			}
			_imp->_runner->toggle_invincible(bRst?0:1);
			GLog::log(GLOG_INFO,"GM:%d切换了无敌状态(%s)", pImp->_parent->ID.id,bRst?"无敌":"正常");
		}
		break;

		case C2S::GOTO:
		{
			TESTGMPRIVILEGE(Has_Move_AsWill)

			C2S::CMD::player_goto & pg = *(C2S::CMD::player_goto *)buf;
			if(sizeof(pg) != size)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			A3DVECTOR pos(pg.pos);
			GLog::log(GLOG_INFO,"GM:%d进行了空间跳跃(%f,%f,%f)", pImp->_parent->ID.id,pos.x,pos.y,pos.z);
			pos.y = pImp->_plane->GetHeightAt(pos.x,pos.z);
			pImp->PlayerGoto(pos);
		}
		break;

		case C2S::GMCMD_DROP_GENERATOR:
		{
			if(!player_template::GetDebugMode()) break;	//禁止了
			C2S::CMD::gmcmd_drop_generator & gdg = *(C2S::CMD::gmcmd_drop_generator *)buf;
			if(sizeof(gdg) != size)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			DATA_TYPE dt = world_manager::GetDataMan().get_data_type(gdg.id,ID_SPACE_ESSENCE);
			if(dt != DT_GM_GENERATOR_ESSENCE)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			GLog::log(GLOG_INFO,"GM:%d丢出了怪物生成器%d", _imp->_parent->ID.id,gdg.id);
			element_data::item_tag_t tag = {element_data::IMT_NULL,0};
			item_data * data = world_manager::GetDataMan().generate_item_from_player(gdg.id,&tag,sizeof(tag));
			if(data)
			{
				DropItemData(_imp->_plane,_imp->_parent->pos,data,_imp->_parent->ID,0,0,0);
			}
		}
		break;

		case C2S::GMCMD_ACTIVE_SPAWNER:
		{
			TESTGMPRIVILEGE(Has_ActivityManager)

			C2S::CMD::gmcmd_active_spawner & gas = *(C2S::CMD::gmcmd_active_spawner *)buf;
			if(sizeof(gas) != size)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			if(gas.is_active)
			{
				//if(!player_template::GetDebugMode()) break;	//禁止了
				GLog::log(GLOG_INFO,"GM:%d激活了生成区域%d",_imp->_parent->ID.id,gas.sp_id);
				if(!_imp->_plane->TriggerSpawn(gas.sp_id))
				{
					GLog::log(GLOG_INFO,"GM:%d激活生成区域出错",_imp->_parent->ID.id);
					error_cmd(S2C::ERR_INVALID_TARGET);
				}
			}
			else
			{
				GLog::log(GLOG_INFO,"GM:%d取消了生成区域%d",_imp->_parent->ID.id,gas.sp_id);
				if(!_imp->_plane->ClearSpawn(gas.sp_id))
				{
					GLog::log(GLOG_INFO,"GM:%d取消生成区域出错",_imp->_parent->ID.id);
					error_cmd(S2C::ERR_INVALID_TARGET);
				}
			}
		}
		break;

		case C2S::GMCMD_GENERATE_MOB:
		{
			if(!player_template::GetDebugMode()) break;	//禁止了
			TESTGMPRIVILEGE(Has_ActivityManager)
			C2S::CMD::gmcmd_generate_mob & ggm = *(C2S::CMD::gmcmd_generate_mob *)buf;
			if(size < sizeof(ggm) || size != sizeof(ggm) + ggm.name_len || ggm.name_len > 18)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			object_interface oi((gplayer_imp *)_imp);
			object_interface::minor_param prop;
			memset(&prop,0,sizeof(prop));
			
			prop.mob_id = ggm.mob_id;
			prop.remain_time = ggm.life;
			prop.exp_factor = 1.f;
			prop.sp_factor = 1.f;
			prop.drop_rate = 1.f;
			prop.money_scale = 1.f;
			prop.spec_leader_id = XID(0,0);
			prop.parent_is_leader = false;
			prop.use_parent_faction = false;
			//prop.die_with_leader = false;
			prop.die_with_who = 0x00;
			prop.vis_id = ggm.vis_id;
			prop.mob_name_size = ggm.name_len;
			if(ggm.name_len)
			{
				memcpy(prop.mob_name,ggm.name,ggm.name_len);
			}
			for(int i =0; i < ggm.count; i ++)
			{
				oi.CreateMinors(prop);
			}
			GLog::log(GLOG_INFO,"GM:%d创建了%d个怪物%d(%d)",
					_imp->_parent->ID.id, ggm.count, ggm.vis_id,ggm.mob_id);
		}
		break;

/*
//这些操作尚未经过权限验证
		case C2S::GMCMD_PLAYER_INC_EXP:
		{
			DEFCMD(gmcmd_player_inc_exp)
			if(_cur_target.type != GM_TYPE_PLAYER)
			{
				error_cmd(S2C::ERR_INVALID_TARGET);
				break;
			}
			//发送一个消息让玩家增加经验和sp
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->SendTo<0>(GM_MSG_GM_CHANGE_EXP,_cur_target,cmd.exp,&(cmd.sp),sizeof(int));
		}
		break;
		
		case C2S::GMCMD_ENDUE_ITEM:
		{
			DEFCMD(gmcmd_endue_item)
			if(_cur_target.type  != GM_TYPE_PLAYER)
			{
				error_cmd(S2C::ERR_INVALID_TARGET);
				break;
			}
			//发送一个消息给一件物品
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->SendTo<0>(GM_MSG_GM_ENDUE_ITEM,_cur_target,cmd.item_id,&(cmd.count),sizeof(int));
		}
		break;

		case C2S::GMCMD_ENDUE_SELL_ITEM:
		{
			DEFCMD(gmcmd_endue_sell_item)
			if(_cur_target.type  != GM_TYPE_PLAYER)
			{
				error_cmd(S2C::ERR_INVALID_TARGET);
				break;
			}
			//发送一个消息给一件商店里卖的物品
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->SendTo<0>(GM_MSG_GM_ENDUE_SELL_ITEM,_cur_target,cmd.item_id,&(cmd.count),sizeof(int));
		}
		break;

		case C2S::GMCMD_REMOVE_ITEM:
		{
			DEFCMD(gmcmd_remove_item)
			if(_cur_target.type  != GM_TYPE_PLAYER)
			{
				error_cmd(S2C::ERR_INVALID_TARGET);
				break;
			}
			//发送一个消息去除指定物品
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->SendTo<0>(GM_MSG_GM_REMOVE_ITEM,_cur_target,cmd.item_id,&(cmd.count),sizeof(int));
		}
		break;

		case C2S::GMCMD_ENDUE_MONEY:
		{
			DEFCMD(gmcmd_endue_money)
			if(_cur_target.type  != GM_TYPE_PLAYER)
			{
				error_cmd(S2C::ERR_INVALID_TARGET);
				break;
			}
			//发送一个消息给钱或者减钱 
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->SendTo<0>(GM_MSG_GM_ENDUE_MONEY,_cur_target,cmd.money);
		}
		break;

		case C2S::GMCMD_RESURRECT:
		{
			DEFCMD(gmcmd_resurrect)
			if(_cur_target.type != GM_TYPE_PLAYER)
			{
				error_cmd(S2C::ERR_INVALID_TARGET);
				break;
			}
			//发送一个消息复活
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->SendTo<0>(GM_MSG_GM_RESURRECT,_cur_target,0);
		}
		break;

		case C2S::GMCMD_ENABLE_DEBUG_CMD:
		{
			DEFCMD(gmcmd_enable_debug_cmd)
			if(_cur_target.type != GM_TYPE_PLAYER)
			{
				error_cmd(S2C::ERR_INVALID_TARGET);
				break;
			}
			//发送一个消息复活
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->SendTo<0>(GM_MSG_GM_DEBUG_COMMAND,_cur_target,0);
		}
		break;
		
		case C2S::GMCMD_ENABLE_DEBUG_CMD:
		{
			DEFCMD(gmcmd_enable_debug_cmd)
			if(_cur_target.type != GM_TYPE_PLAYER)
			{
				error_cmd(S2C::ERR_INVALID_TARGET);
				break;
			}
			//发送一个消息复活
			GLog::log(GLOG_INFO,"GM:%d激活了玩家%d的调试命令",_imp->_parent->ID.id,_cur_target.id);
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->SendTo<0>(GM_MSG_GM_DEBUG_COMMAND,_cur_target,0);
		}
		break;

		case C2S::GMCMD_RESET_PROP:		//洗点
		{
			if(_cur_target.type != GM_TYPE_PLAYER)
			{
				error_cmd(S2C::ERR_INVALID_TARGET);
				break;
			}
			//发送一个消息洗点
			GLog::log(GLOG_INFO,"GM:%d对玩家%d进行了洗点",_imp->_parent->ID.id,_cur_target.type);
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->SendTo<0>(GM_MSG_GM_RESET_PP,_cur_target,0);
		}
		break;
		*/
		
		case C2S::GMCMD_GET_COMMON_VALUE:
		{
			TESTGMPRIVILEGE(Has_Hide_BeGod)

			C2S::CMD::gmcmd_get_common_value & ggcv = *(C2S::CMD::gmcmd_get_common_value *)buf;
			if(sizeof(ggcv) != size)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			int value = _imp->_plane->GetCommonValue(ggcv.key);
			char buf[100];
			sprintf(buf,"Get:Var[%d] ----> Result:%d",ggcv.key, value);
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->Say(buf);
		}
		break;
		
		case C2S::GMCMD_QUERY_SPEC_ITEM:
		{
			TESTGMPRIVILEGE(Has_ActivityManager)

			DEFCMD(gmcmd_query_spec_item)
			if(cmd.roleid <= 0 || cmd.type <= 0) break;
			XID target(GM_TYPE_PLAYER,cmd.roleid);
			GLog::log(GLOG_INFO,"GM:%d查询玩家%d的物品%d",_imp->_parent->ID.id,cmd.roleid,cmd.type);
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			gplayer * pPlayer = (gplayer*)_imp->_parent;
			msg_query_spec_item_t data = {cmd.type, pPlayer->cs_index, pPlayer->cs_sid};
			pImp->SendTo<0>(GM_MSG_GM_QUERY_SPEC_ITEM,target,0,&data,sizeof(data));
		}
		break;

		case C2S::GMCMD_REMOVE_SPEC_ITEM:
		{
			TESTGMPRIVILEGE(Has_ActivityManager)

			DEFCMD(gmcmd_remove_spec_item)
			if(cmd.roleid <= 0 || cmd.type <= 0 || cmd.where >= gplayer_imp::IL_MAX || cmd.count == 0) break;
			XID target(GM_TYPE_PLAYER,cmd.roleid);
			GLog::log(GLOG_INFO,"GM:%d删除玩家%d的物品%d(%d个)",_imp->_parent->ID.id,cmd.roleid,cmd.type,cmd.count);
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			gplayer * pPlayer = (gplayer*)_imp->_parent;
			msg_remove_spec_item_t data = {cmd.type, cmd.where, cmd.index, cmd.count, pPlayer->cs_index, pPlayer->cs_sid};
			pImp->SendTo<0>(GM_MSG_GM_REMOVE_SPEC_ITEM,target,0,&data,sizeof(data));
		}
		break;

		case C2S::GMCMD_OPEN_ACTIVITY:
		{
			TESTGMPRIVILEGE(Has_ActivityManager)

			C2S::CMD::gmcmd_open_activity & goa = *(C2S::CMD::gmcmd_open_activity *)buf;
			if(sizeof(goa) != size)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			DATA_TYPE dt;
			GM_ACTIVITY_CONFIG * pCfg = (GM_ACTIVITY_CONFIG *)world_manager::GetDataMan().get_data_ptr(goa.activity_id, ID_SPACE_CONFIG, dt);
			if(pCfg == NULL || dt != DT_GM_ACTIVITY_CONFIG)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			if(pCfg->disabled)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			
			if(goa.is_open)
			{
				GLog::log(GLOG_INFO,"GM:%d开启活动%d",_imp->_parent->ID.id,goa.activity_id);
				for(size_t i=0; i<sizeof(pCfg->openlist)/sizeof(pCfg->openlist[0]); i++)
				{
					if(pCfg->openlist[i] <= 0) break;
					if(!_imp->_plane->TriggerSpawn(pCfg->openlist[i]))
					{
						GLog::log(GLOG_INFO,"GM:%d开启活动出错",_imp->_parent->ID.id);
						error_cmd(S2C::ERR_INVALID_TARGET);
					}
				}
			}
			else
			{
				GLog::log(GLOG_INFO,"GM:%d关闭活动%d",_imp->_parent->ID.id,goa.activity_id);
				for(size_t i=0; i<sizeof(pCfg->closelist)/sizeof(pCfg->closelist[0]); i++)
				{
					if(pCfg->closelist[i] <= 0) break;
					if(!_imp->_plane->ClearSpawn(pCfg->closelist[i]))
					{
						GLog::log(GLOG_INFO,"GM:%d关闭活动出错",_imp->_parent->ID.id);
						error_cmd(S2C::ERR_INVALID_TARGET);
					}
				}
			}
		}
		break;
		case C2S::GMCMD_CHANGE_DS:
		{
			TESTGMPRIVILEGE(Has_ActivityManager)
			DEFCMD(gmcmd_change_ds)
			if(cmd.flag != GMSV::CHDS_FLAG_DS_TO_CENTRALDS && cmd.flag != GMSV::CHDS_FLAG_CENTRALDS_TO_DS) break;
			GLog::log(GLOG_INFO,"GM:%d跳转DS服务器 flag=%d worldtag=%d",_imp->_parent->ID.id, cmd.flag, world_manager::GetWorldTag());
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->PlayerTryChangeDS(cmd.flag);
		}
		break;
		
		case C2S::GMCMD_QUERY_UNIQUE_DATA:
		{
			TESTGMPRIVILEGE(Has_ActivityManager)

			C2S::CMD::gmcmd_query_unqiue_data & ggcv = *(C2S::CMD::gmcmd_query_unqiue_data *)buf;
			if(sizeof(ggcv) != size)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			char sbuf[1024];
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			
			if(ggcv.key == -1)
			{
				int ukey = 0;
				int maxc = 1000;
				do
				{
					sprintf(sbuf,"Get UniqueData: Key[%d]",ukey);
					ukey = world_manager::GetUniqueDataMan().OnDump(ukey ,sbuf + strlen(sbuf), 1024 - strlen(sbuf));
					pImp->Say(sbuf);
				}while(-1 != ukey && --maxc);
			}
			else
			{	
				sprintf(sbuf,"Get UniqueData: Key[%d]",ggcv.key);
				world_manager::GetUniqueDataMan().OnDump(ggcv.key ,sbuf + strlen(sbuf), 1024 - strlen(sbuf));
				pImp->Say(sbuf);
			}
		}
		break;

	default:
		__PRINTF("收到无法辨识的命令 %d\n",cmd_type);
		break;
	}
	return 0;
#undef DEFCMD
}

int 
gplayer_controller::DebugCommandHandler(int cmd_type,const void * buf, size_t size)
{
	GLog::log(LOG_INFO,"GM:用户%d执行了内部命令%d",_imp->_parent->ID.id,cmd_type);
	switch(cmd_type)
	{

	//这里是测试用的命令

		
		case C2S::DEBUG_DELIVERY_CMD:
		{
			C2S::CMD::debug_delivery_cmd & sp = *(C2S::CMD::debug_delivery_cmd *)buf;
			if(size < sizeof(int)) break;
			GMSV::SendDebugCommand(_imp->_parent->ID.id , sp.type, sp.buf, size - sizeof(int));
		}
		break;

		case C2S::DEBUG_GS_CMD:
		{
			//C2S::CMD::debug_gs_cmd & sp = *(C2S::CMD::debug_gs_cmd *)buf;
			//if(size < sizeof(short)) break;
			//char * buf = sp.buf;
			//size_t cmd_len = size - sizeof(short);
			//$$$$ handle buf, cmd_len;
		}
		break;

		case 11800:
		{
			C2S::CMD::summon_pet & sp = *(C2S::CMD::summon_pet *)buf;
			if(size != sizeof(C2S::CMD::summon_pet))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->TestCreatePet(sp.pet_index);
		}
		break;

		case 1902:
			{
				_load_stats += 200;
			}
			break;
		case 1901:
		{
			if(size != 10)
			{
				break;
			}
			size_t index = *(size_t*)((char*)buf+2);
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->IdentifyItemAddon(index,0);
		}
		break;

		case 1988:
		{
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			size_t money = 1000000*pImp->_basic.level;
			pImp->GainMoneyWithDrop(money);
			pImp->_runner->get_player_money(pImp->GetMoney(),pImp->_money_capacity);
		}
		break;

		case 1989:
		{
			if(size != 6)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->_basic.sec_level = *(int*)((char*)buf + 2);
			pImp->SetRefreshState();
		}
		break;

		case 1990:
		{
			struct mma
			{
				unsigned short cmd;
				size_t id;
			};
			if(size != 6)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			int id = *(int*)((char*)buf+2);
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->_trashbox.SetTrashBoxSize(id);
		}
		break;

		case 1991:
		{
			struct mma
			{
				unsigned short cmd;
				size_t max_ap;
			};
			if(size != 6)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			int id = *(int*)((char*)buf+2);
			if(id < 0) id = 0;
			if(id > 399) id = 0;
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->SetMaxAP(id);
		}
		break;

		case 1992:
		{
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->ModifyAP(100);
		}
		break;	

		case 1993:
		{
			if(size != 10)
			{
				break;
			}
			int id2 = *(int*)((char*)buf+6);
			if(id2 != 1001) break;
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			int id = *(int*)((char*)buf+2);
			pImp->TestCreatePet(id);
			pImp->SummonPet(0);
		}
		break;
		
		case 1994:
		{
			if(size != 6)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			int skill = *(int*)((char*)buf+2);
			//不检查选中对象的位置信息
			if(_cur_target.IsValid())
			{
				session_skill *pSkill= new session_skill(pImp);
				int t = _cur_target.id;
				pSkill->SetTarget(skill,1,1,&t);
				if(pImp->AddSession(pSkill)) pImp->StartSession();
			}
		}
		break;

		case 1995:
		{
			if(size != 6)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			int skill = *(int*)((char*)buf+2);
			//不检查选中对象的位置信息
			session_instant_skill *pSkill= new session_instant_skill(pImp);
			int t = _cur_target.id;
			pSkill->SetTarget(skill,1,1,&t);
			if(pImp->AddSession(pSkill)) pImp->StartSession();
		}
		break;

		case 21989:
		{
			if(size != 10)
			{
				break;
			}
			int data = *(int*)((char*)buf+2);
			if(data == 8879)
			{
				int offset = *(int*)((char*)buf + 6);
				if(offset < -30 && offset> 30)
				{
					offset = 0;
				}
				gplayer_imp * pImp = (gplayer_imp*)_imp;
				pImp->_cur_prop.attack_speed += offset;
				if(pImp->_cur_prop.attack_speed <= 0) 
				{
					pImp->_cur_prop.attack_speed = 20;
				}
					
			}
			break;

		}
		
		case 2000:
		{
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			int level = pImp->_basic.level;
			if(level >= 150) level = 149;
			msg_exp_t mm={level,player_template::GetLvlupExp(pImp->GetPlayerClass(),level),level*10000};
			pImp->SendTo<0>(GM_MSG_EXPERIENCE,pImp->_parent->ID,0,&mm,sizeof(mm));
		}
		break;

		case 2016:
		{
			if(size != 6 )
			{       
				break;
			}
			int title = *(size_t*)((char*)buf + 2);
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->ChangeInventorySize(title);
		}
		break;

		case 2017:
		{
			if (size != 14)
				break;
			int index = *(size_t*)((char*)buf + 2);
			int item_id = *(size_t*)((char*)buf + 6);
			int count = *(size_t*)((char*)buf + 10);
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			if(!pImp->PlayerUseItem(gplayer_imp::IL_INVENTORY, index, item_id, count))
			{
			}
		}
		break;

		case 2018:
		{
			if (size != 10)
				break;
			int type = *(size_t*)((char*)buf + 2);
			int power = *(size_t*)((char*)buf + 6);
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->_player_fatering.AddFateRingPower(type,power);
		}
		break;

		case 2001:
		{
			struct mma
			{
				unsigned short cmd;
				size_t id;
			};
			if(size != 6)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			int t2 = g_timer.get_systime();
			element_data::item_tag_t tag = {element_data::IMT_CREATE,0};
			item_data * data = world_manager::GetDataMan().generate_item_for_drop(*(int*)((char*)buf+2),&tag,sizeof(tag));
			if(data)
			{
				data->expire_date = t2 + 300;
				DropItemData(_imp->_plane,_imp->_parent->pos,data,_imp->_parent->ID,0,0,0);
			}
		}
		break;


		case 2002:
		{
			if(size != 10)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			int id = (*(int*)((char*)buf+2));
			size_t timeout = (*(size_t*)((char*)buf+6));
			if(timeout)
			{
				if(timeout < 100) timeout = 120;
				if(timeout > 300) timeout = 300;
			}
			if(id == 8510)
			{
				class try_restart : public abase::timer_task, public ONET::Thread::Runnable
				{
					int  _timeout;
					int  _id;
					world * _plane;
					int _counter;
				public:
					try_restart(int id, size_t timeout,world * pPlane):_timeout(timeout),_id(id),_plane(pPlane)
					{
						_counter = 20;
						SetTimer(g_timer,20,timeout);
					}

					void SendTimeout()
					{
						//发送数据
						char buf[512];
						short buf2[512];
						sprintf(buf,"Server shutdown:%dsec\n",_timeout);
						char * p = buf;
						short* w = buf2;
						do
						{
							*w++ = *p++;
						}while(*p);
						broadcast_chat_msg(_id,buf2,(w - buf2)*sizeof(short),0,0,0,0);
					}

					
					virtual void Run()
					{
						_timeout --;
						if(_timeout > 150) 
						{
							_counter -= 1;
						}
						else if(_timeout > 80)
						{
							_counter -= 2;
						}
						else if(_timeout > 20)
						{
							_counter -= 4;
						}
						else 
						{
							_counter -= 20;
						}
						if(_counter <= 0)
						{
							_counter = 20;
							SendTimeout();
						}
						if(_timer_index <0)
						{
							_plane->GetWorldManager()->RestartProcess();
						}
					}

					void OnTimer(int index,int rtimes)      
					{
						//ONET::Thread::Pool::AddTask(this);
						Run();
					}

				};
				static try_restart * restart_task = NULL;
				if(restart_task)
				{
					restart_task->RemoveTimer();
					delete restart_task;
					restart_task = NULL;
				}
				if(timeout)
					(restart_task = new try_restart(_imp->_parent->ID.id,timeout,_imp->_plane))->SendTimeout();
			}
		}
		break;

		case 2013:
		{
			if (size != 22)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			int roleid = *(int*)((char*)buf+2);
			int mail_id = *(int*)((char*)buf+6);
			int goods_id = *(int*)((char*)buf+10);
			int goods_index = *(int*)((char*)buf+14);
			int goods_slot = *(int*)((char*)buf+18);
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			if(world_manager::GetWorldParam().korea_shop || world_manager::GetWorldParam().southamerican_shop)
			{
				error_cmd(S2C::ERR_SHOP_NOT_OPEN);
				break;
			}
			else
			{
				pImp->PlayerGivePresent(roleid, mail_id, goods_id, goods_index, goods_slot);
			}
		}
		break;

		case 2014:
		{
			if (size != 18)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			int roleid = *(int*)((char*)buf+2);
			int goods_id = *(int*)((char*)buf+6);
			int goods_index = *(int*)((char*)buf+10);
			int goods_slot = *(int*)((char*)buf+14);
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			if(world_manager::GetWorldParam().korea_shop || world_manager::GetWorldParam().southamerican_shop)
			{
				error_cmd(S2C::ERR_SHOP_NOT_OPEN);
				break;
			}
			else
			{
				pImp->PlayerAskForPresent(roleid, goods_id, goods_index, goods_slot);
			}
		}
		break;

		case 2015:
		{
			if (size != 14)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			int tech_index = *(int*)((char*)buf+2);
			int inv_index = *(int*)((char*)buf+6);
			int id = *(int*)((char*)buf+10);
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->FactionFortressResetTechPoint(tech_index, inv_index, id);
		}
		break;

		case 10801:
		{
			struct mma
			{
				unsigned short cmd;
				size_t id;
			};
			if(size != 6)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			int data = *(int*)((char*)buf+2);
			if(data == 197646)
			{
				gplayer_imp * pImp = (gplayer_imp *) _imp;
				pImp->_basic.status_point += 10000;
			}
			else if(data == 197647)
			{
				gplayer_imp * pImp = (gplayer_imp *) _imp;
				pImp->_immune_state_adj |= ~(0x01F);
			}
		}
		break;
		
		case 10800:
		{
			struct mma
			{
				unsigned short cmd;
				size_t id;
			};
			size_t count = 1;
			if(size != 6 && size != 10)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			if(size == 10)
			{
				count = *(size_t*)((char*)buf + 6);
				if(count == 0) count = 1;
			}
			element_data::item_tag_t tag = {element_data::IMT_CREATE,0};
			item_data * data = world_manager::GetDataMan().generate_item_for_drop(*(int*)((char*)buf+2),&tag,sizeof(tag));
			if(data)
			{
				if(data->pile_limit == 0) break;
				if(count > data->pile_limit) count = data->pile_limit;
				data->count = count;
				DropItemData(_imp->_plane,_imp->_parent->pos,data,_imp->_parent->ID,0,0,0);
			}
		}
		break;

		case 10802:
		{
#pragma pack(1)
			struct mma
			{
				unsigned short cmd;
				size_t id;
				size_t count;
				size_t remain_time;
				size_t uuu;
			};
#pragma pack()
			if(size != sizeof(mma))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			mma * ccc = (mma*)buf;
			int id = ccc->id;
			size_t count = ccc->count;
			object_interface oi((gplayer_imp *)_imp);
			object_interface::minor_param prop;
			memset(&prop,0,sizeof(prop));
			
			prop.remain_time = ccc->remain_time;
			prop.exp_factor = 1.f;
			prop.sp_factor = 1.f;
			prop.drop_rate = 1.f;
			prop.money_scale = 1.f;
			prop.spec_leader_id = _cur_target;
			prop.parent_is_leader = false;
			//prop.use_parent_faction = ccc->uuu;
			prop.use_parent_faction = false;
			//_cur_target == _imp->_parent->ID?true:false;
			//prop.die_with_leader = true;
			prop.die_with_who = 0x01;
			for(size_t i =0; i < count; i ++)
			{
				prop.mob_id = id;
				oi.CreateMinors(prop);
			}

		}
		break;

		case 10803:
		{
			struct mma
			{
				unsigned short cmd;
				size_t id;
			};
			if(size != 6)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			object_interface oi((gplayer_imp *)_imp);
			object_interface::minor_param prop;
			memset(&prop,0,sizeof(prop));
			
			prop.remain_time = 0;
			prop.exp_factor = 0.f;
			prop.sp_factor = 0.f;
			prop.drop_rate = 0.f;
			prop.money_scale = 0.f;
			prop.spec_leader_id = _cur_target;
			prop.parent_is_leader = true;
			prop.use_parent_faction = true;
			//prop.die_with_leader = true;
			prop.die_with_who = 0x01;
			for(size_t i =0; i< 10000;i ++)
			{
				prop.mob_id = i;
				oi.CreateMinors(prop);
			}
		
		}
		break;
		//lgc debug start
		case 10816:
		{
			//模拟客户端发来的cmd，这里处理cmd在逻辑线程池，而实际上是cmd处理是单独的cmd线程
			void handle_user_cmd(int cs_index,int sid,int user_id,const void * buf, size_t size);
			class test_skill_task : public ONET::Thread::Runnable
			{
				link_sid _lid;
				int _skillid;
				char _force_attack;
				int _target;
			public:
				test_skill_task(const link_sid & lid, int skillid, char force_attack, int target)
					:_lid(lid), _skillid(skillid), _force_attack(force_attack), _target(target){}
				virtual void Run()
				{
					bool target_valid = (_target != -1);
					void * cmd = NULL;
					size_t size = 0;
					if(!GNET::SkillWrapper::IsInstant(_skillid))
					{
						size = sizeof(C2S::CMD::cast_skill) + (target_valid?4:0);
						cmd = malloc(size);
						C2S::CMD::cast_skill * pCmd = (C2S::CMD::cast_skill *)cmd;
						pCmd->header.cmd = C2S::CAST_SKILL;
						pCmd->skill_id = _skillid;
						pCmd->force_attack = _force_attack;
						if(target_valid)
						{
							pCmd->target_count = 1;
							pCmd->targets[0] = _target;
						}
						else
							pCmd->target_count = 0;
					}
					else
					{
						size = sizeof(C2S::CMD::cast_instant_skill) + (target_valid?4:0);
						cmd = malloc(size);
						C2S::CMD::cast_instant_skill * pCmd = (C2S::CMD::cast_instant_skill *)cmd;
						pCmd->header.cmd = C2S::CAST_INSTANT_SKILL;
						pCmd->skill_id = _skillid;
						pCmd->force_attack = _force_attack;
						if(target_valid)
						{
							pCmd->target_count = 1;
							pCmd->targets[0] = _target;
						}
						else
							pCmd->target_count = 0;
					}
					handle_user_cmd(_lid.cs_id, _lid.cs_sid, _lid.user_id, cmd, size);
					free(cmd);
					delete this;
				}
			};
			class test_skill_timer : public abase::timer_task
			{
				enum{MAX_COUNT=10};
				link_sid _lid;
				int _target;
				char _force_attack;
				int _skill[MAX_COUNT];
				int _skill_count;
				int _cur_index;

			public:
				test_skill_timer(const link_sid & lid, int target, char force_attack, int * skill, size_t count)
					:_lid(lid), _target(target), _force_attack(force_attack), _skill_count(0), _cur_index(0)
				{
					ASSERT(skill && count >= 1);
					if(count > MAX_COUNT) count = MAX_COUNT;
					memcpy(_skill, skill, sizeof(int)*count);
					_skill_count = count;
					
					SetTimer(g_timer,20*5,count);
					__PRINTF("test_skill_timer(%p) SetTimer count=%d\n", this, count);
				}
				~test_skill_timer(){ RemoveTimer(); }
				void OnTimer(int index,int rtimes)      
				{
					ASSERT(_cur_index < _skill_count);
					ONET::Thread::Pool::AddTask(new test_skill_task(_lid, _skill[_cur_index++], _force_attack, _target));
					__PRINTF("test_skill_timer(%p) OnTimer rtimes=%d\n", this, rtimes);
					if(rtimes == 0)
					{
						__PRINTF("test_skill_timer(%p) deleted\n", this);
						delete this;
					}
				}
			};

			if(size < 2 + 4 || size > 2 + 10*4
					|| (size - 2) % 4 != 0)
			{
				break;
			}
			int * skillid = (int *)((char *)buf + 2);
			size_t skillcnt = (size - 2) / 4;

			gplayer_imp * pImp = (gplayer_imp *)_imp;
			link_sid lid;
			make_link_sid(pImp->GetParent(), lid);
			new test_skill_timer(lid, pImp->GetCurTarget().id, pImp->OI_GetForceAttack(), skillid, skillcnt);
		}
		break;
		
		case 10817:
		{
			//测试碰撞数据是否正确
#pragma pack(1)
			struct trace_result
			{
				bool ret;
				bool in_solid;
				int ratio;
			};
#pragma pack(0)
			gplayer_imp * pImp = (gplayer_imp *)_imp;
			const rect & rt = pImp->_plane->GetLocalWorld();
			float step = 10.f;
			int col = (int)(rt.Width()/step);
			int row = (int)(rt.Height()/step);
			trace_result * pData = new trace_result[row * col];
			memset(pData, 0, row * col * sizeof(trace_result));
		
			A3DVECTOR pos(rt.left + step/2, 0, rt.top + step/2);
			__PRINTF("startpos(%.2f, %.2f)\n",pos.x,pos.z);
			for(int i=0; i<row; i++)
			{
				for(int j=0; j<col; j++)
				{
					trace_result & res = pData[i*row + j];
					pos.y = pImp->_plane->GetHeightAt(pos.x,pos.z) + 5.0f;
					float ratio = 0.f;
					res.ret = pImp->_plane->GetTraceMan()->AABBTrace(pos, A3DVECTOR(0,-6.0f,0), A3DVECTOR(0.3f, 0.85f, 0.3f), res.in_solid, ratio, &pImp->_plane->w_collision_flags);
					res.ratio = int(ratio*100.f);
					
					pos.x += step;
				}
				pos.x = rt.left + step/2;
				pos.z += step;
			}
			__PRINTF("endpos(%.2f, %.2f)\n",pos.x,pos.z);
			unsigned char md5[16] = {0};
			MD5((unsigned char*)pData,row * col * sizeof(trace_result),md5);
			__PRINTF("md5=");
			for(int i=0; i<16; i++)
			{
				__PRINTF("%02X",md5[i]);
			}
			__PRINTF("\n");

			delete [] pData;
		}
		break;
		
		case 10818:
		{
			//测试寻路数据是否正确
			if(size != 6) break;
			int t = *(int*)((char*)buf + 2);	//1陆2水
			gplayer_imp * pImp = (gplayer_imp *)_imp;
			const rect & rt = pImp->_plane->GetLocalWorld();
			float step = 10.f;
			int col = (int)(rt.Width()/step);
			int row = (int)(rt.Height()/step);
			float * pData = new float[row * col];
			
			A3DVECTOR pos(rt.left + step/2, 0, rt.top + step/2);
			__PRINTF("startpos(%.2f, %.2f)\n",pos.x,pos.z);
			for(int i=0; i<row; i++)
			{
				for(int j=0; j<col; j++)
				{
					if(t == 1)
					{
						pos.y = pImp->_plane->GetHeightAt(pos.x,pos.z);
						if(path_finding::GetValidPos(pImp->_plane, pos))
							pData[i*row + j] = pos.y;
						else
							pData[i*row + j] = -1.f;
					}
					else if(t == 2)
					{
						pData[i*row + j] = path_finding::GetWaterHeight(pImp->_plane, pos.x, pos.z);	
					}
					pos.x += step;
				}
				pos.x = rt.left + step/2;
				pos.z += step;
			}
			__PRINTF("endpos(%.2f, %.2f)\n",pos.x,pos.z);
			unsigned char md5[16] = {0};
			MD5((unsigned char*)pData,row * col * sizeof(float),md5);
			__PRINTF("md5=");
			for(int i=0; i<16; i++)
			{
				__PRINTF("%02X",md5[i]);
			}
			__PRINTF("\n");

			delete [] pData;
		}
		break;
		
		case 10819:
		{
			if(size != 6) break;
			gplayer_imp * pImp = (gplayer_imp *)_imp;
			pImp->IncLeadership(*(int*)((char*)buf+2));
		}
		break;
		
		case 10820:
		{
			if(size != 6) break;
			gplayer_imp * pImp = (gplayer_imp *)_imp;
			pImp->PlayerTrickBattleApply(*(int*)((char*)buf+2));
		}
		break;

		case 10821:
		{
			gplayer_imp * pImp = (gplayer_imp *)_imp;
			pImp->PlayerLeaveTrickBattle();
		}
		break;
		
		case 10822:
		{
			gplayer_imp * pImp = (gplayer_imp *)_imp;
			pImp->PlayerReincarnation();
		}
		break;

		case 10823:
		{
			if(size != 10) break;
			gplayer_imp * pImp = (gplayer_imp *)_imp;
			pImp->PlayerRewriteReincarnationTome(*(int*)((char*)buf+2),*(int*)((char*)buf+6));
		}
		break;
		
		case 10824:
		{
			if(size != 6) break;
			gplayer_imp * pImp = (gplayer_imp *)_imp;
			pImp->PlayerActiveReincarnationTome(*(int*)((char*)buf+2));
		}
		break;
		
		case 10825:
		{
			if(size != 6)
			{
				break;
			}
			int flag = *(int*)((char*)buf+2);
			GMSV::SendTryChangeDS(_imp->_parent->ID.id, flag);
		}
		break;
		
		case 10826:
		{
			if(size != 2 && size != 6)
			{
				break;
			}
			gplayer_imp * pImp = (gplayer_imp *)_imp;
			int roleid = pImp->_parent->ID.id;
			if(size == 6)
			{
				roleid = *(int*)((char*)buf+2);
			}
			if(!world_manager::GetInstance()->IsCountryBattleWorld()) break;
			class stream2 : public countrybattle_ctrl::stream
			{
				gplayer_imp * _imp;
				public:
				stream2(gplayer_imp * imp):_imp(imp){}
				virtual void dump(const char * str)
				{
					_imp->Say(str);
				}
			};
			stream2 s(pImp);
			ASSERT(pImp->_plane->w_ctrl);
			countrybattle_ctrl * pCtrl = (countrybattle_ctrl *)pImp->_plane->w_ctrl;
			pCtrl->DumpPlayerScore(roleid, &s);
		}
		break;

		case 10827:
		{
		#pragma pack(1)
			struct mma
			{
				unsigned short cmd;
				size_t server_id;
				size_t battle_id;
				size_t attacker;
				size_t defender;
				size_t player_count;
			};
		#pragma pack()
			mma * cmd = (mma*)buf;
			if(size != sizeof(mma))
			{
				break;
			}
			country_battle_param param;
			memset(&param,0,sizeof(param));
			param.battle_id = cmd->battle_id;
			param.attacker = cmd->attacker;
			param.defender = cmd->defender;
			param.player_count = cmd->player_count;
			param.end_timestamp = g_timer.get_systime() + 30000;
			MSG msg;
			BuildMessage(msg,GM_MSG_CREATE_COUNTRYBATTLE,
					XID(GM_TYPE_SERVER,cmd->server_id),XID(GM_TYPE_SERVER,1),A3DVECTOR(0,0,0),
					0,&param,sizeof(param));
			world_manager::GetInstance()->SendRemoteMessage(cmd->server_id,msg);
		}
		break;
		
		case 10828:
		{
			if(size != 10)
			{
				break;
			}
			((gplayer_imp*)_imp)->EnterCountryBattle(*(int*)((char*)buf+2), *(int*)((char*)buf+6));
		}
		break;
		
		case 10829:
		{
			((gplayer_imp*)_imp)->_filters.ModifyFilter(FILTER_CHECK_INSTANCE_KEY,FMID_CLEAR_AECB,NULL,0);
		}
		break;

		case 10830:
		{
			((gplayer_imp*)_imp)->CountryJoinApply();
		}
		break;
		
		case 10831:
		{
			((gplayer_imp*)_imp)->CountryLeave();
		}
		break;
		
		case 10832:
		{
			if(size != 6)
			{   
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			((gplayer_imp*)_imp)->EnterNonpenaltyPVPState(*(int*)((char*)buf + 2));
		}
		break;
		
		case 10833:
		{
			if(size != 6)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			int value = *(int*)((char*)buf + 2);
			class test_task : public ONET::Thread::Runnable
			{
				int _value;
			public:
				test_task(int v):_value(v){}
				virtual void Run()
				{
					int s = 0;
					for(int i=0; i<100000000; i++)
					{
						s += (i%2 ? 1 : -1);	
					}
					if(--_value <= 0)
						delete this;
					else
						ONET::Thread::Pool::AddTask(this);

				}
			};
			ONET::Thread::Pool::AddTask(new test_task(value));
			ONET::Thread::Pool::AddTask(new test_task(value));
			ONET::Thread::Pool::AddTask(new test_task(value));
			ONET::Thread::Pool::AddTask(new test_task(value));
		}
		break;
		
		case 10834:
		{
			if(size != 10)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			int repu = *(int*)((char*)buf + 2);
			int contrib = *(int*)((char*)buf + 6);
			gplayer_imp * pImp = (gplayer_imp *)_imp;
			if(repu > 0) pImp->IncForceReputation(repu);
			else if(repu < 0) pImp->DecForceReputation(-repu);
			if(contrib > 0) pImp->IncForceContribution(contrib);
			else if(contrib < 0) pImp->DecForceContribution(-contrib);
		}
		break;
		
		case 10835:
		{
			if(size != 2)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp *)_imp;
			class stream2 : public ForceGlobalDataMan::stream
			{
				gplayer_imp * _imp;
				public:
				stream2(gplayer_imp * imp):_imp(imp){}
				virtual void dump(const char * str)
				{
					_imp->Say(str);
				}
			};
			stream2 s(pImp);
			world_manager::GetForceGlobalDataMan().Dump(&s);
		}
		break;
		
		case 10836:
		{
			if(size != 10)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			GMSV::SendIncreaseForceActivity(*(int*)((char*)buf+2), *(int*)((char*)buf+6));
		}
		break;
		
		case 10837:
		{
			if(size != 6)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			object_interface oi((gplayer_imp*)_imp);
			oi.ForbidBeSelected(*(int*)((char*)buf+2));
		}
		break;
		
		case 10838:
		{
			object_interface oi((gplayer_imp*)_imp);
			oi.SetSkillAttackTransmit(_cur_target);
		}
		break;
		
		case 10839:
		{
			object_interface oi((gplayer_imp*)_imp);
			oi.ExchangePosition(_cur_target);
		}
		break;
		
		case 10840:
		{
			if(size != 6)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			int shape = *(int*)((char*)buf+2);
			if(shape != 0) shape = shape&(~0xC0)|0x80;
			object_interface oi((gplayer_imp*)_imp);
			oi.ChangeShape(shape);
		}
		break;
		
		case 10841:
		{
			if(size != 6)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			((gplayer_imp*)_imp)->WeaponDisabled(*(int*)((char*)buf+2));
		}
		break;
		
		case 10842:
		{
			if(size != 6)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			int factionid = *(int*)((char*)buf+2);
			if(factionid <= 0) break;
			_imp->UpdateMafiaInfo(factionid,6,0);
		}
		break;
		
		case 10843:
		{
			if(size != 10)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			/*int factionid = *(int*)((char*)buf+2);
			if(factionid < 10000) break;
			int count = *(int*)((char*)buf+6);
			if(count <= 0 || count > 100) break;
			instance_hash_key _hkey;
			GNET::faction_fortress_data data;
			GNET::faction_fortress_data2 data2;
			memset(&_hkey,0,sizeof(_hkey));
			memset(&data,0,sizeof(data));
			memset(&data2,0,sizeof(data2));
			_hkey.key1 = factionid;
			_hkey.key2 = 0;
			data.factionid = _hkey.key1;
			data.level = 12;
			int tmp[8] = {1,1,1,1,1,2,2,2};
			data.technology.data = tmp;
			data.technology.size = 5*sizeof(int);
			data.material.data = tmp;
			data.material.size = 8*sizeof(int);
			int tmp2[10] = {29551,0,29560,0,29576,0,29578,0,29587,0};
			data.building.data = tmp2;
			data.building.size = 10*sizeof(int);
			data2.factionid = _hkey.key1;
			data2.health = 23;
			while(count > 0)
			{
				world_manager::GetInstance()->FactionLogin(_hkey,&data,&data2);
				--count;
				++ _hkey.key1;
				data.factionid = _hkey.key1;
				data2.factionid = _hkey.key1;
			}*/
		}
		break;
		
		case 10844:
		{
			if(size != 10)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			if(pImp->OI_IsMafiaMember())
			{
				pImp->IncFactionContrib(*(int*)((char*)buf+2),*(int*)((char*)buf+6));
			}
		}
		break;
		
		case 10845:
		{
			if(size != 10)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->change_factionfortress(*(int*)((char*)buf+2), *(int*)((char*)buf+6));
		}
		break;
		
		case 10846:
		{
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			gplayer * pPlayer = (gplayer*)_imp->_parent;
			raw_wrapper ar;
			ar << *((char*)(&pPlayer->ID.id)+3) << *((char*)(&pPlayer->ID.id)+2) << *((char*)(&pPlayer->ID.id)+1) << *((char*)(&pPlayer->ID.id))
				<< *((char*)(&pPlayer->id_mafia)+3) << *((char*)(&pPlayer->id_mafia)+2) << *((char*)(&pPlayer->id_mafia)+1) << *((char*)(&pPlayer->id_mafia))
				<< (char)0 
				<< (char)0
				<< (char)0;
			GNET::ForwardFactionFortressOP(4406,ar.data(),ar.size(),object_interface(pImp));
		}
		break;
		
		case 10847:
		{
			if(size != 6)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			if(*(int*)((char*)buf+2) != 10847) break;
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			memset(pImp->_active_task_list.begin(),0,TASK_ACTIVE_LIST_BUF_SIZE);
			memset(pImp->_finished_task_list.begin(),0,TASK_FINISHED_LIST_BUF_SIZE);
			memset(pImp->_finished_time_task_list.begin(),0,TASK_FINISH_TIME_LIST_BUF_SIZE);
			memset(pImp->_finish_task_count_list.begin(),0,TASK_FINISH_COUNT_LIST_BUF_SIZE);
			memset(pImp->_storage_task_list.begin(),0,TASK_STORAGE_LIST_BUF_SIZE);
			gplayer * pPlayer = pImp->GetParent();
			GMSV::SendDisconnect(pPlayer->cs_index,pPlayer->ID.id,pPlayer->cs_sid,0);
		}
		break;
		
		case 10848:
		{
			if(size != 14)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp *)_imp;
			pImp->SummonPlantPet(*(int*)((char*)buf+2),*(int*)((char*)buf+6),*(int*)((char*)buf+10),_cur_target,0);
		}
		break;
		
		case 10849:
		{
			if(size != 14)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp *)_imp;
			pImp->SummonPet2(*(int*)((char*)buf+2),*(int*)((char*)buf+6),*(int*)((char*)buf+10));
		}
		break;
		
		case 10850:
		{
			if(size != 6)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp *)_imp;
			int rep = *(int*)((char*)buf+2);
			if(rep < 0) break;
			pImp->SetReputation(rep);
			pImp->_runner->get_reputation(rep);
		}
		break;
		
		case 10851:
		{
			if(size != 10)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			int distance = *(int*)((char*)buf+2);
			if(distance < 1) distance = 1;
			if(distance > 20) distance = 20;
			int time = *(int*)((char*)buf+6);
			if(time < 50) time = 50;
			if(time > 1000) time = 1000;
			
			if(_cur_target.id == -1) break;
			world::object_info info;
			if(!_imp->_plane->QueryObject(_cur_target,info) ||
					info.state != world::QUERY_OBJECT_STATE_ACTIVE)
				break;
			((gplayer_imp*)_imp)->KnockBack(_cur_target,info.pos,distance,time,3);	
		}
		break;
		
		case 10852:
		{
			gplayer_imp * pImp = (gplayer_imp *)_imp;
			gplayer * pPlayer = (gplayer *)pImp->_parent;
			char buf[40] = {0};
			sprintf(buf,"RoleId: %d FactionId %d Team_uid_0:%d,Team_uid_1:%d",pPlayer->ID.id,pPlayer->id_mafia,(unsigned int)(pImp->_team.GetTeamUID() >> 32),(unsigned int)(pImp->_team.GetTeamUID() & 0x0000FFFF));
			pImp->Say(buf);
		}
		break;
		
		case 10853:
		{
			gplayer_imp * pImp = (gplayer_imp *)_imp;
			pImp->_inventory.Clear();
			pImp->PlayerGetInventoryDetail(gplayer_imp::IL_INVENTORY);
		}
		break;
		
		case 10854:
		{
			gplayer_imp * pImp = (gplayer_imp *)_imp;
			pImp->_trashbox.SetPassword(NULL,0);
			pImp->_runner->trashbox_passwd_state(pImp->_trashbox.HasPassword());
		}
		break;

		case 10855:
		{
			if(size != 6)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;	
			}
			if(*(int*)((char*)buf+2) != 10855) break;
			object_interface obj((gplayer_imp*)_imp);
			attack_msg msg;
			memset(&msg, 0, sizeof(msg));
			msg.attack_range = 5000.f;
			msg.physic_damage = 100000000;
			msg.attack_rate = 100000000;
			msg.magic_damage[0] = 20000000;
			msg.magic_damage[1] = 20000000;
			msg.magic_damage[2] = 20000000;
			msg.magic_damage[3] = 20000000;
			msg.magic_damage[4] = 20000000;
			msg.attack_attr = attack_msg::PHYSIC_ATTACK_HIT_DEFINITE;
			msg.force_attack = 1;
			obj.RegionAttack1(_imp->_parent->pos, 5000.f, msg, 0, XID(-1,-1));				
		}
		break;
		
		case 10856:
		{
			if(size != 6)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp* pImp = (gplayer_imp*)_imp;
			int pindex = pImp->_petman.GetCurActivePet();
			if(pindex < 0) break;
			pImp->_petman.RecvExp(pImp,pindex,*(int*)((char*)buf+2));
		}
		break;
		
		case 10857:
		{
#pragma pack(1)
			struct mma
			{
				unsigned short cmd;
				int id;
				int count;
				int remain_time;
			};
#pragma pack()
			if(size != sizeof(mma))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp* pImp = (gplayer_imp*)_imp;
			mma* ccc = (mma*)buf;
			XID xid = _cur_target;
			if(xid.id == -1) xid = pImp->_parent->ID;
			pImp->SummonNPC(ccc->id,ccc->count,xid,10,ccc->remain_time);
		}
		break;
		
		case 10858:
		{
#pragma pack(1)
			struct mma
			{
				unsigned short cmd;
				int id;
				int count;
				int remain_time;
			};
#pragma pack()
			if(size != sizeof(mma))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp* pImp = (gplayer_imp*)_imp;
			mma* ccc = (mma*)buf;
			pImp->SummonMine(ccc->id,ccc->count,_cur_target,10,ccc->remain_time);
		}
		break;
		
		case 10859:
		{
			if(size != 10)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp* pImp = (gplayer_imp*)_imp;
			int index = *(int*)((char*)buf+2);
			int dec_dura = *(int*)((char*)buf+6);
			dec_dura *= DURABILITY_UNIT_COUNT;
			if(dec_dura <= 0) break;
			if(index == item::EQUIP_INDEX_WEAPON)
			{
				while(dec_dura > 0)
				{
					pImp->DoWeaponOperation<0>();
					pImp->_runner->attack_once(0);
					dec_dura -= DURABILITY_DEC_PER_ATTACK;
				}
			}
			else if(index >= item::EQUIP_ARMOR_START && index < item::EQUIP_ARMOR_END)
			{
				if(pImp->_equipment.DecDurability(index, dec_dura))		
				{
					pImp->_runner->equipment_damaged(index,0);	
					pImp->RefreshEquipment();
				}
			}
		}
		break;		
		
		case 10860:
		{
			if(size != 22)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp* pImp = (gplayer_imp*)_imp;
			if(int rst = pImp->DyeSuitFashionItem(*(int*)((char*)buf+2),*(int*)((char*)buf+6),*(int*)((char*)buf+10),*(int*)((char*)buf+14),*(int*)((char*)buf+18)))
			{
				pImp->_runner->error_message(rst);	
			}
		}
		break;
		
		case 10861:
		{
#pragma pack(1)
			struct mma
			{
				unsigned short cmd;
				unsigned int which;
				unsigned int index;
			};
#pragma pack()
			if(size != sizeof(struct mma))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			mma* ccc = (mma*)buf;
			gplayer_imp* pImp = (gplayer_imp*)_imp;
			int value = 0;
			if(ccc->which == 0)
			{
				if(ccc->index >= ROLE_REPUTATION_UCHAR_SIZE) break;
				value = ++pImp->_role_reputation_uchar[ccc->index];
				
			}
			else if(ccc->which == 1)
			{
				if(ccc->index >= ROLE_REPUTATION_USHORT_SIZE) break;
				value = ++pImp->_role_reputation_ushort[ccc->index];
			}
			else if(ccc->which == 2)
			{
				if(ccc->index >= ROLE_REPUTATION_UINT_SIZE) break;
				value = ++pImp->_role_reputation_uint[ccc->index];
			}
			else
				break;
			char buf[1024];
			char * which[] = {"uchar","ushort","uint"};
			sprintf(buf,"Set:%s[%d] ----> Result:%d",which[ccc->which], ccc->index, value);
			pImp->Say(buf);
		}
		break;
		
		case 10862:
		{
#pragma pack(1)
			struct mma
			{
				unsigned short cmd;
				size_t id;
				size_t count;
				size_t target_distance;
				size_t remain_time;
				size_t die_with_who;
			};
#pragma pack()
			if(size != sizeof(mma))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			mma* ccc = (mma*)buf;
			gplayer_imp* pImp = (gplayer_imp*)_imp;
			pImp->SummonMonster(ccc->id,ccc->count,pImp->_parent->ID,ccc->target_distance,ccc->remain_time,ccc->die_with_who,0);
		}
		break;
		
		case 10863:
		{
#pragma pack(1)
			struct mma
			{
				unsigned short cmd;
				size_t id;
				size_t count;
				size_t target_distance;
				size_t remain_time;
				size_t die_with_who;
				size_t path_id;
			};
#pragma pack()
			if(size != sizeof(mma))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			mma * ccc = (mma*)buf;
			object_interface oi((gplayer_imp *)_imp);
			object_interface::minor_param prop;
			memset(&prop,0,sizeof(prop));
	
			prop.mob_id = ccc->id;
			prop.remain_time = ccc->remain_time;
			prop.exp_factor = 1.f;
			prop.sp_factor = 1.f;
			prop.drop_rate = 1.f;
			prop.money_scale = 1.f;
			prop.spec_leader_id = XID(-1,-1);
			prop.parent_is_leader = true;
			prop.use_parent_faction = true;
			prop.die_with_who = ccc->die_with_who;
			prop.target_id = _cur_target;
			prop.path_id = ccc->path_id;
			for(size_t i =0; i < ccc->count; i ++)
			{
				oi.CreateMinors(prop,_cur_target,ccc->target_distance);
			}
		}
		break;
		case 10864:
		{
			if(size != 6)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->IncSkillAbility(*(int*)((char*)buf+2));
		}
		break;
		case 10865:
		{
			if(size != 10)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->change_elf_property(*(int*)((char*)buf+2), *(int*)((char*)buf+6));
		}
		break;
		case 10869:
		{
			if(size != 6)
			{
				error_cmd(S2C::ERR_FATAL_ERR);	
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->InsertTeamVisibleState(*(int*)((char*)buf+2),NULL,0);	
		}
		break;
		case 10870:	//获取当前商城中存在销售时间控制的商品的详细信息
		{
			if(size != 2)
			{
				error_cmd(S2C::ERR_FATAL_ERR);	
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->get_mall_detail();
		}
		break;
		case 10871:
		{
			if(size != 6)
			{
				error_cmd(S2C::ERR_FATAL_ERR);	
				break;
			}
			int id = *(int*)((char*)buf+2);
			if(id >= 0)
				world_manager::GetPlayerMall().SetGroupId(id);
		}
		break;
		case 10872://获取商城中当前商品价格
		{
			if(size != 10)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}

			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->PlayerGetMallItemPrice(*(int*)((char*)buf+2), *(int*)((char*)buf+6));
		}
		break;
		case 10873://买商城中的物品
		{
			if(size != 18)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			//每次永远只购买一件商品
			if(world_manager::GetWorldParam().korea_shop || world_manager::GetWorldParam().southamerican_shop)
			{
				pImp->ForeignDoShoppingStep1(*(int *)((char*)buf+6), *(int *)((char*)buf+10),*(int *)((char*)buf+14));
			}
			else
			{
				pImp->PlayerDoShopping(1, (const int *)((char*)buf+6));
			}
		}
		break;
		case 10874:
		{
			if(size != 2)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->ElfPlayerInsertExp(pImp->_basic.exp,0);
			pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_EQUIPMENT, 23);
			
			int level = pImp->_basic.level;
			if(level >= 150) level = 149;
			int exp = player_template::GetLvlupExp(pImp->GetPlayerClass(),level)-pImp->_basic.exp-1;
			msg_exp_t mm={level,exp,0};
			if(mm.exp < 0) mm.exp = 0;
			pImp->SendTo<0>(GM_MSG_EXPERIENCE,pImp->_parent->ID,0,&mm,sizeof(mm));
		}
		break;
		case 10875:	//人物增加gfx光效
		{
			if(size != 10)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			int adddec = *(int *)((char*)buf +2);
			int state = *(int *)((char*)buf +6);
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			if(state>=0 && state< MAX_VISIBLE_STATE_COUNT)
			{
				if(adddec == 1)
					pImp->IncVisibleState((unsigned short)state);
				else if(adddec == 0)
					pImp->ClearVisibleState((unsigned short)state);
			}
		}
		break;
		case 10876://释放技能
		{
			if(size != 14+4* (*(size_t *)((char*)buf +10))  )
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->CastElfSkill(*(int *)((char*)buf +2), *(int *)((char*)buf +6),
				*(int *)((char*)buf +10), (int*)((char*)buf+14));
		}
		break;
		case 10877://给装备的小精灵冲耐力
		{
			if(size != 10)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			if(!pImp->RechargeEquippedElf(*(int *)((char*)buf +2), *(int *)((char*)buf +6)) )
			{
				error_cmd(S2C::ERR_CANNOT_RECHARGE);
			}
		}
		break;
		
		case 10878://小精灵分解
		{
			if(size != 6)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->ElfDecompose(*(int *)((char*)buf +2));
		}
		break;

		case 10879://小精灵装备拆卸
		{
			if(size != 14)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->ElfDestroyItem(*(int *)((char*)buf +2), *(int *)((char*)buf +6), *(int *)((char*)buf +10));
		}
		break;

		case 10880://小精灵精炼
		{
			if(size != 14)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->ElfRefine(*(int *)((char*)buf +2), *(int *)((char*)buf +6), *(int *)((char*)buf +10));
		}
		break;

		case 10881://小精灵精炼等级转移
		{
			if(size != 10)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->ElfRefineTransmit(*(int *)((char*)buf +2), *(int *)((char*)buf +6));
		}
		break;

		case 10882: //小精灵学习技能
		{
			if(size != 10)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->ElfLearnSkill(*(int *)((char*)buf +2), *(int *)((char*)buf +6));
		}
		break;

		case 10883://小精灵遗忘技能
		{
			if(size != 14)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->ElfForgetSkill(*(int *)((char*)buf +2), *(int *)((char*)buf +6), *(int *)((char*)buf +10));
		}
		break;

		case 10884://小精灵洗属性点
		{
			if(size != 26)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->ElfDecAttribute(*(int *)((char*)buf +2), *(int *)((char*)buf +6),*(int *)((char*)buf +10),*(int *)((char*)buf +14),*(int *)((char*)buf +18),*(int *)((char*)buf +22));
		}
		break;

		case 10885://小精灵洗天赋点
		{
			if(size != 10)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->ElfFlushGenius(*(int *)((char*)buf +2), *(int *)((char*)buf +6));
		}
		break;

		case 10886:
		{
			if(size != 2)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->dump_elf_info();
		}
		break;

		case 10887: //lgc
		{
			if(size != 6)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->TakeOutItem(*(int *)((char*)buf +2));
		}
		break;

		case 10888: //lgc
		{
			#pragma pack(1)
			struct mma
			{
				unsigned short cmd;
				size_t inv_idx;
				size_t eqi_idx;
			};
			#pragma pack()
			if(size != 10)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->PlayerEquipItem(*(int *)((char*)buf +2),*(int *)((char*)buf +6));
			pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_INVENTORY, *(int *)((char*)buf +2));
			pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_EQUIPMENT, *(int *)((char*)buf +6));
			
		}
		break;

		case 10889://lgc
		{
			if(size != 6)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			int level = pImp->_basic.level;
			if(level >= 150) level = 149;
			msg_exp_t mm={level,*(int*)((char*)buf+2),level*10000};
			if(mm.exp < 0) mm.exp = 0;
			pImp->SendTo<0>(GM_MSG_EXPERIENCE,pImp->_parent->ID,0,&mm,sizeof(mm));
		}
		break;
			
		case 10111://小精灵加属星点
		{
			if(size != 18)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->ElfAddAttribute(*(int *)((char*)buf +2), *(int *)((char*)buf +6),*(int *)((char*)buf +10),*(int *)((char*)buf +14));
			pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_EQUIPMENT, 23);
		}
		break;

		case 10112://加天赋点
		{
			if(size != 22)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->ElfAddGenius(*(int *)((char*)buf +2),*(int *)((char*)buf +6), *(int *)((char*)buf +10), *(int *)((char*)buf +14), *(int *)((char*)buf +18));
			pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_EQUIPMENT, 23);
		}
		break;

		case 10113://注入经验
		{
			if(size != 10)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->ElfPlayerInsertExp(*(int *)((char*)buf +2), (char)(*(int *)((char*)buf +6)));
			pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_EQUIPMENT, 23);
		}
		break;

		case 10114://装备小精灵的装备
		{
			if(size != 6)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->ElfEquipItem(*(int *)((char*)buf +2));
			pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_INVENTORY, *(int *)((char*)buf +2));
			pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_EQUIPMENT, 23);
		}
		break;

		case 10115://安全状态转化
		{
			if(size != 6)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->ElfChangeSecureStatus(*(int *)((char*)buf +2));
			pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_EQUIPMENT, 23);
		
		}
		break;
		//lgc debug end
		
		case 10900:
		{
			struct mma
			{
				unsigned short cmd;
				size_t key;
				size_t value;
			};
			if(size != 10)
			{
				break;
			}

			int key = *(int*)((char*)buf + 2);
			int value = *(int*)((char*)buf + 6);
			_imp->_plane->SetCommonValue(key, value);
			char buf[1024];
			sprintf(buf,"Set:Var[%d] ----> Result:%d",key, value);
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->Say(buf);
		}
		break;

		case 10901:
		{
			struct mma
			{
				unsigned short cmd;
				size_t key;
				size_t value;
			};
			if(size != 10)
			{
				break;
			}

			int key = *(int*)((char*)buf + 2);
			int value = *(int*)((char*)buf + 6);
			int value2 = _imp->_plane->ModifyCommonValue(key, value);

			char buf[1024];
			sprintf(buf,"Modify:Var[%d] +(%d) ----> Result:%d",key, value, value2);
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->Say(buf);

		}
		break;

		case 10902:
		{
			class my_stream : public common_data::stream
			{
				gplayer_imp * _imp;
			public:
				my_stream(gplayer_imp * pImp):_imp(pImp)
				{
				}
				void dump(const char * str)
				{
					_imp->Say(str);
				}
			};

			gplayer_imp * pImp = (gplayer_imp*)_imp;
			my_stream ttttt(pImp);
			_imp->_plane->w_common_data.Dump(&ttttt);

		}
		break;

		case 10903:
		{
			gplayer_imp *pImp = (gplayer_imp*)_imp;
			pImp->PlayerTouchPointQuery();
			char buf[20] = {0};
			sprintf(buf,"query touch");
			pImp->Say(buf);

		}
		break;
		case 10904:
		{
#pragma pack(1)
			struct mma
			{
				unsigned short cmd;
				unsigned int  index;  // 商品索引
				unsigned int  lots;   // 批量数
				unsigned int  itemid; // 以下3个和客户端表中值对应 冗余校检用
				unsigned int  count;
				unsigned int  price;
				unsigned int  expiretime;
			};
#pragma pack()
			if(size != sizeof(mma))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp* pImp = (gplayer_imp*)_imp;
			mma* ccc = (mma*)buf;
			pImp->PlayerTouchPointCost(ccc->index,ccc->itemid,ccc->count,ccc->price,ccc->expiretime,ccc->lots);	
		}
		break;
		case 10905:
		{
#pragma pack(1)
			struct mma
			{
				unsigned short cmd;
				unsigned int titleid;
				unsigned int expire;
				int flag;
			};
#pragma pack()
			if(size != sizeof(mma))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp* pImp = (gplayer_imp*)_imp;
			mma* ccc = (mma*)buf;

			if(ccc->flag)
				pImp->GetPlayerTitle().ObtainTitle((short)ccc->titleid,ccc->expire);
			else
				pImp->GetPlayerTitle().LoseTitle((short)ccc->titleid);
		}
		break;
		case 10906:
		{
#pragma pack(1)
			struct mma
			{
				unsigned short cmd;
				unsigned int titleid;
			};
#pragma pack()
			if(size != sizeof(mma))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp* pImp = (gplayer_imp*)_imp;
			mma* ccc = (mma*)buf;
			pImp->PlayerChangeTitle(ccc->titleid);
		}
		break;
		case 10907:
		{
#pragma pack(1)
			struct mma
			{
				 unsigned short cmd;
				 int flag;
				 int signtype;
				 int itempos;
				 int year;
				 int mon;
				 int day;
			};
#pragma pack()
			if(size != sizeof(mma))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp* pImp = (gplayer_imp*)_imp;
			mma* ccc = (mma*)buf;

			if(ccc->flag)
			{
				tm dst; memset(&dst,0,sizeof(tm));
				dst.tm_year = ccc->year - 1900;
				dst.tm_mon = ccc->mon - 1;
				dst.tm_mday = ccc->day;
				pImp->PlayerLateSignin(ccc->signtype,ccc->itempos,mktime(&dst));
			}
			else
			{
			 	pImp->PlayerDailySignin();
			}
		}
		break;
		case 10908:
		{
#pragma pack(1)
			struct mma
			{
				unsigned short cmd;
				int type;
				int mon;
			};
#pragma pack()
			if(size != sizeof(mma))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp* pImp = (gplayer_imp*)_imp;
			mma* ccc = (mma*)buf;

			pImp->PlayerApplySigninAward(ccc->type,ccc->mon);
		}
		break;
		case 10909:
		{
#pragma pack(1)
			struct mma
			{
				unsigned short cmd;
				int year;
				int mon;
				int day;
			};
#pragma pack()
			if(size != sizeof(mma))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			mma* ccc = (mma*)buf;

			player_dailysign::dbgtime.tm_year = ccc->year - 1900;
			player_dailysign::dbgtime.tm_mon = ccc->mon - 1;
			player_dailysign::dbgtime.tm_mday = ccc->day;
		}
		break;
		case 10910:
		{
#pragma pack(1)
			struct mma
			{
				unsigned short cmd;
				int key;
				int type;
				int val;
				int setflag;
			};
#pragma pack()
			if(size != sizeof(mma))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			mma* ccc = (mma*)buf;

			if(ccc->type == 1)
			{
				world_manager::GetUniqueDataMan().ModifyData(ccc->key,ccc->val,ccc->setflag > 0);
			}
			else if(ccc->type == 2)
			{
				world_manager::GetUniqueDataMan().ModifyData(ccc->key,ccc->val/100.0f,ccc->setflag > 0);
			}
			else
			{
				char temp[256];
				struct tm tm1;
				time_t upt = time(NULL);
				localtime_r(&upt, &tm1);
				sprintf(temp,"CMD:[ key=%d type=%d val=%d flag=%d ] exec at %d-%d-%d %d:%d:%d", ccc->key,ccc->type,ccc->val,ccc->setflag,
						1900 + tm1.tm_year,tm1.tm_mon,tm1.tm_mday,tm1.tm_hour,tm1.tm_min,tm1.tm_sec);
				world_manager::GetUniqueDataMan().ModifyData(ccc->key,UDOctets(temp,strlen(temp)),ccc->setflag);
			}
		}
		break;
		case 10911:
		{
#pragma pack(1)
			struct mma
			{
				unsigned short cmd;
				int key;
			};
#pragma pack()
			if(size != sizeof(mma))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			mma* ccc = (mma*)buf;

			char sbuf[1024];
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			
			if(ccc->key == -1)
			{
				ccc->key = 0;
				do
				{
					sprintf(sbuf,"Get UniqueData: Key[%d]",ccc->key);
					ccc->key = world_manager::GetUniqueDataMan().OnDump(ccc->key ,sbuf + strlen(sbuf), 1024 - strlen(sbuf));
					pImp->Say(sbuf);
				}while(-1 != ccc->key);
			}
			else
			{	
				sprintf(sbuf,"Get UniqueData: Key[%d]",ccc->key);
				world_manager::GetUniqueDataMan().OnDump(ccc->key ,sbuf + strlen(sbuf), 1024 - strlen(sbuf));
				pImp->Say(sbuf);
			}
		}
		break;
		case 10912:
		{
#pragma pack(1)
			struct mma
			{
				unsigned short cmd;
				int exp;
			};
#pragma pack()
			if(size != sizeof(mma))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			mma* ccc = (mma*)buf;

			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->ReceiveRealmExp(ccc->exp);
		}
		break;
		case 10913:
		{
#pragma pack(1)
			struct mma
			{
				unsigned short cmd;
			};
#pragma pack()
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->ExpandRealmLevelMax();
		}
		break;
		case 10915:
		{
#pragma pack(1)
			struct mma
			{
				unsigned short cmd;
				int type;
				int year;
				int month;
				int day;
				int hour;
			};
#pragma pack()
			if(size != sizeof(mma))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			mma* ccc = (mma*)buf;

			gplayer_imp * pImp = (gplayer_imp*)_imp;

			if(ccc->type == 0)
			{
				pImp->GetPlayerClock().Reset(pImp,ccc->year);
			}
			else if(ccc->type == 1)
			{			
				player_clock::dbgtime.tm_year = ccc->year - 1900;
				player_clock::dbgtime.tm_mon = ccc->month - 1;
				player_clock::dbgtime.tm_mday = ccc->day;
				player_clock::dbgtime.tm_hour = ccc->hour;
			}
			else
			{
				struct debug_report : public clock_listener
				{
					void OnClock(gplayer_imp* player,int type)
					{						
						char sbuf[1024];
						sprintf(sbuf,"clock[%d] report at %d-%d-%d %d:%d:%d",type,
								player_clock::dbgtime.tm_year+1900,player_clock::dbgtime.tm_mon+1,player_clock::dbgtime.tm_mday,
								player_clock::dbgtime.tm_hour,player_clock::dbgtime.tm_min,player_clock::dbgtime.tm_sec);
						player->Say(sbuf);
					}
					void OnPassClock(gplayer_imp* player, int type,int lastupdate,int now)
					{
						char sbuf[1024];
						tm tt;
						localtime_r((time_t *)&lastupdate, &tt);
						sprintf(sbuf,"clock[%d] pass report at %d-%d-%d %d:%d:%d from lasttime %d-%d-%d %d:%d:%d",type,
								player_clock::dbgtime.tm_year+1900,player_clock::dbgtime.tm_mon+1,player_clock::dbgtime.tm_mday,
								player_clock::dbgtime.tm_hour,player_clock::dbgtime.tm_min,player_clock::dbgtime.tm_sec,
								tt.tm_year+1900,tt.tm_mon+1,tt.tm_mday,tt.tm_hour,tt.tm_min,tt.tm_sec);
						player->Say(sbuf);					
					}
					void Init(gplayer_imp* player,int month,int week, int day, int hour)
					{
						player->GetPlayerClock().AddNotice(this,player_clock::GPC_PER_HOUR_GLOBAL,hour);
						player->GetPlayerClock().AddNotice(this,player_clock::GPC_PER_DAY_GLOBAL,day%7);
						player->GetPlayerClock().AddNotice(this,player_clock::GPC_PER_WEEK_GLOBAL,week);
						player->GetPlayerClock().AddNotice(this,player_clock::GPC_PER_MONTH_GLOBAL,month-1);
					}
				};
				
				static debug_report public_report;
				public_report.Init(pImp,ccc->year,ccc->month,ccc->day,ccc->hour);
			}
		}
		break;
		case 10916:
		{
#pragma pack(1)
			struct mma
			{
				unsigned short cmd;
				int type;
			};
#pragma pack()
			if(size != sizeof(mma))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			mma* ccc = (mma*)buf;
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			GNET::TestMassMail(ccc->type,object_interface(pImp));
		}
		break;
		case 10917:
		{
#pragma pack(1)
			struct mma
			{
				unsigned short cmd;
				int opt;
				int entryid;
			};
#pragma pack()
			if(size != sizeof(mma))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			mma* ccc = (mma*)buf;
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			
			switch(ccc->opt)
			{
				case random_mall_info::RAND_MALL_OPT_QUERY: 
					pImp->PlayerRandMallQuery(ccc->entryid);
					break;
				case random_mall_info::RAND_MALL_OPT_ROLL: 
					pImp->PlayerRandMallRoll(ccc->entryid);
					break;
				case random_mall_info::RAND_MALL_OPT_PAY: 
					pImp->PlayerRandMallPay(ccc->entryid);
					break;			
			}			

		}
		break;
		case 10918:
		{
#pragma pack(1)
			struct mma
			{
				unsigned short cmd;
				int type;
			};
#pragma pack()
			if(size != sizeof(mma))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			mma* ccc = (mma*)buf;
			gplayer_imp * pImp = (gplayer_imp*)_imp;

			pImp->OnMafiaPvPAward(ccc->type,pImp->_parent->ID,pImp->_parent->pos,pImp->OI_GetMafiaID() ,
					city_region::GetDomainID(pImp->_parent->pos.x,pImp->_parent->pos.z));
		}
		break;
		case 10919:
		{
#pragma pack(1)
			struct mma
			{
				unsigned short cmd;
				int contrib;
			};
#pragma pack()
			if(size != sizeof(mma))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			mma* ccc = (mma*)buf;
			gplayer_imp * pImp = (gplayer_imp*)_imp;

			pImp->IncWorldContrib(ccc->contrib);
		}
		break;
		case 10920:
		{
#pragma pack(1)
			struct mma
			{
				unsigned short cmd;
				int storageid;
				int useseed;
			};
#pragma pack()
			if(size != sizeof(mma))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			mma* ccc = (mma*)buf;
			PlayerTaskInterface  task_if((gplayer_imp*)_imp);
			task_if.UpdateOneStorageDebug(ccc->storageid,ccc->useseed > 0);
		}
		break;
		case 10921:
		{
#pragma pack(1)
			struct mma
			{
				unsigned short cmd;
				int type;
			};
#pragma pack()
			if(size != sizeof(mma))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			mma* ccc = (mma*)buf;
			_imp->_plane->GetWorldManager()->GetMapRes().SetType(ccc->type);	
		}
		break;
		case 10922:
		{
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->PlayerReenterInstance();
		}
		break;
	case 10807:
		{
#pragma pack(1)
			struct mma
			{
				unsigned short cmd;
				size_t id;
				size_t count;
				size_t remain_time;
				size_t uuu;
			};
#pragma pack()
			if(size != sizeof(mma))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			mma * ccc = (mma*)buf;
			int id = ccc->id;
			size_t count = ccc->count;
			object_interface oi((gplayer_imp *)_imp);
			object_interface::minor_param prop;
			memset(&prop,0,sizeof(prop));
			
			prop.remain_time = ccc->remain_time;
			prop.exp_factor = 1.f;
			prop.sp_factor = 1.f;
			prop.drop_rate = 1.f;
			prop.money_scale = 1.f;
			prop.spec_leader_id = _cur_target;
			prop.parent_is_leader = false;
			prop.use_parent_faction = ccc->uuu;
			prop.policy_classid = CLS_NPC_AI_POLICY_TURRET;
			prop.policy_aggro = 7;
			//prop.use_parent_faction = false;
			_cur_target == _imp->_parent->ID?true:false;
			//prop.die_with_leader = true;
			prop.die_with_who = 0x01;
			for(size_t i =0; i < count; i ++)
			{
				prop.mob_id = id;
				oi.CreateMinors(prop);
			}

		}
		break;

		case 10808:
		{
			for(size_t i =0; i < 20; i ++)
			{
				abase::RandUniform();
				abase::RandUniform();
				abase::RandUniform();
				abase::RandUniform();
				abase::RandUniform();
				abase::RandUniform();
				abase::RandUniform();

				__PRINTINFO("%f\n",abase::RandUniform());

			}

		}
		break;

		case 2003:
		case 10804:
		{
			struct mma
			{
				unsigned short cmd;
				size_t id;
			};
			if(size <= 6)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			float rlist[32]= {0};
			int ridxlist[32]= {10,10,0};
			int addonlist[32] = {0};
			int id = *(size_t*)((char*)buf + 2);
			for(size_t i =0;i < 32; i ++) rlist[i] = abase::RandUniform();
			for(size_t i =0;i < 32 && size > i*sizeof(int) + 6; i ++) 
			{
				addonlist[i] = *(size_t*)((char*)buf + 6 + i*sizeof(int));
			}

			item_data * data = world_manager::GetDataMan().generate_equipment(id,rlist,ridxlist,addonlist);
			if(data)
			{
				DropItemData(_imp->_plane,_imp->_parent->pos,data,_imp->_parent->ID,0,0,0);
			}
		
		}
		break;

		case 10809:
		{
			if(size != 6)
			{
				break;
			}
			int index = *(int*)((char*)buf+2);
			if(index == 73125)
			{
				abase::fast_allocator::dump(stdout);
				__PRINTINFO("----------------------------------------------------------------\n");
				TestSearch(_imp->_plane);
				abase::fast_allocator::dump(stdout);
				__PRINTINFO("******************************************************************\n");
			}
		}
		break;

		case 10810:
		{
			if(size != 6)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			size_t count = *(size_t*)((char*)buf + 2);
			for(size_t i = 0; i < count; i ++)
			{
				GLog::log(GLOG_INFO,"用户%d测试编号%3d",_imp->_parent->ID.id,i);
			}
		}
		break;

		case 10811:
		{
			if(size != 6)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			A3DVECTOR pos = _imp->_parent->pos;
			size_t count = *(size_t*)((char*)buf + 2);
			float r = 5.f;
			float l = 1.f;
			for(size_t i = 0; i < count; i ++)
			{
				A3DVECTOR pos2 = pos;
				float x = abase::Rand(-r,r);
				float z = abase::Rand(-r,r);
				if(fabs(x) < l && fabs(z) < l) 
				{
					i--;
					continue;
				}
				if(x*x + z*z > r*r) 
				{
					i --;
					continue;
				}
				pos2.x += x;
				pos2.z += z;
				DropMoneyItem(_imp->_plane,pos2,1,XID(-1,-1),0,0,0);
			}
		}
		break;

		case 10812: ///   addon change debug cmd
		{
#pragma pack(1)
			struct mma
			{
				unsigned short cmd;
				int equip_idx;  // 装备所在背包位 
			   	int equip_socket_idx;      // 需要转化的魂石所在孔位 
   				int old_stone_type;        // 旧魂石类型 (对应孔位中魂石类型冗余校检) 
       			int new_stone_type;        // 目标魂石类型 
			 	int recipe_type;	   //  配方id   (表中项与旧魂石和 目标魂石 一致)
	 			int materials_id[1];       // 材料类型 (排列顺序同 配方)
		    	int idx[1];     // 材料所在背包位
			};
#pragma pack()
			if(size != sizeof(mma))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			mma * req = (mma*)buf;

			GLog::log(GLOG_INFO, "收到GM用户魂石变更请求背包位[%d]孔位[%d]旧魂石[%d]新魂石[%d]配方[%d]材料[%d]材料位[%d]",
				req->equip_idx,
				req->equip_socket_idx,
				req->old_stone_type,
				req->new_stone_type,
				req->recipe_type,
				req->materials_id[0],
				req->idx[0]);

			((gplayer_imp*)_imp)->ChangeEquipAddon(
				(unsigned char)req->equip_idx,
				(unsigned char)req->equip_socket_idx,
				req->old_stone_type,
				req->new_stone_type,
				req->recipe_type,
				req->materials_id,
				(unsigned char*)req->idx,
				1);

		}
		break;

		case 10813: ///   addon replace debug cmd
		{
#pragma pack(1)
			struct mma
			{
				unsigned short cmd;
				int equip_idx;  // 装备所在背包位 
			   	int equip_socket_idx;      // 需要转化的魂石所在孔位 
      				int old_stone_type;        // 旧魂石类型 (对应孔位中魂石类型冗余校检) 
         			int new_stone_idx;  // 目标 魂石所在背包位
				int new_stone_type;        // 目标魂石类型 
			};
#pragma pack()
			if(size != sizeof(mma))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			mma * req = (mma*)buf;

			GLog::log(GLOG_INFO, "收到GM用户魂石替换请求背包位[%d]孔位[%d]旧魂石[%d]新魂石[%d]目标位[%d]",
				req->equip_idx,
				req->equip_socket_idx,
				req->old_stone_type,
				req->new_stone_type,
				req->new_stone_idx);

			((gplayer_imp*)_imp)->ReplaceEquipAddon(
				(unsigned char)req->equip_idx,
				(unsigned char)req->equip_socket_idx,
				req->old_stone_type,
				req->new_stone_type,
				(unsigned char)req->new_stone_idx);

		}
		break;
		case 10814:
		{
#pragma pack(1)
			struct mma
		     	{	
			  	unsigned short cmd;
			  	int wastetime;
		     	};
#pragma pack()
			if(size != sizeof(mma))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			mma * req = (mma*)buf;
			GNET::AccelerateExpelschedule(req->wastetime);
		}
		break;

		case 2006:
		{
			struct mma
			{
				unsigned short cmd;
				size_t id;
			};
			if(size != 6)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			int count = *(int*)((char*)buf+2);
			if(count <= 0) break;
			element_data::item_tag_t tag = {element_data::IMT_CREATE,0};
			for(size_t i = 0; i < 10000;i ++)
			{
				item_data * data = world_manager::GetDataMan().generate_item_from_player(i,&tag,sizeof(tag));
				if(!data) continue;
				FreeItem(data);
				for(int j = 0;  j < count -1; j ++)
				{
					data = world_manager::GetDataMan().generate_item_from_player(i,&tag,sizeof(tag));
					FreeItem(data);
				}
			}
		}
		break;

		case 2007:
		{
			struct mma
			{
				unsigned short cmd;
				size_t id;
			};
			if(size != 6)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			int count = *(int*)((char*)buf+2);
			if(count <= 0) break;
			element_data::item_tag_t tag = {element_data::IMT_CREATE,0};
			for(size_t i = 0; i < 10000;i ++)
			{
				item_data * data = world_manager::GetDataMan().generate_item_for_drop(i,&tag,sizeof(tag));
				if(!data) continue;
				FreeItem(data);
				for(int j = 0;  j < count -1; j ++)
				{
					data = world_manager::GetDataMan().generate_item_for_drop(i,&tag,sizeof(tag));
					FreeItem(data);
				}
			}
		}
		break;

		case 2010:
		{
			struct mma
			{
				unsigned short cmd;
				size_t id;
			};
			if(size != 6)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			int id = *(int*)((char*)buf+2);
			object_interface oi((gplayer_imp*)_imp);
			object_interface::mine_param param = {id, 1000};
			oi.CreateMine(_imp->_parent->pos,param);
		}
		break;

		case 2011:
		{
			struct mma
			{
				unsigned short cmd;
				size_t id;
			};
			if(size != 6)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			int id = *(int*)((char*)buf+2);
			object_interface oi((gplayer_imp*)_imp);
			int domain;
			if(oi.CheckWaypoint(id,domain))
			{
				oi.ReturnWaypoint(id);
			}
		}
		break;

		case 2012:
		{
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			gplayer * pPlayer = pImp->GetParent();
			GMSV::SendDisconnect(pPlayer->cs_index,pPlayer->ID.id,pPlayer->cs_sid,0);
		}
		break;

		case 10805:
		{
			struct mma
			{
				unsigned short cmd;
				size_t id;
			};
			if(size != 6)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			int id = *(int*)((char*)buf+2);
			if(!_imp->_plane->TriggerSpawn(id))
			{
				error_cmd(S2C::ERR_INVALID_TARGET);
			}
		}
		break;

		case 10806:
		{
			struct mma
			{
				unsigned short cmd;
				size_t id;
			};
			if(size != 6)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			int id = *(int*)((char*)buf+2);
			if(!_imp->_plane->ClearSpawn(id))
			{
				error_cmd(S2C::ERR_INVALID_TARGET);
			}
		}
		break;

		case 2008:
		{
			struct mma
			{
				unsigned short cmd;
				size_t index;
				size_t count;
				int need_level;
			};
			if(size != 14)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			size_t index = *(int*)((char*)buf+2);
			size_t count = *(int*)((char*)buf+6);
			int nlevel  = *(int*)((char*)buf+10);
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			item_list & inv = pImp->GetInventory();
			if(index >= inv.Size()) break;
			item & it = inv[index];
			if(it.type == -1 || it.body == 0)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			int total_times = 0;
			int success_counter = 0;
			for(size_t i = 0; i < count;i ++)
			{
				item it2 = it;
				it2.body = it.body->Clone();
				int level = 0;
				int times = 0;
				for(; level >=0 && level < nlevel;) 
				{
					float adjust[] = { 0,0,0,0};
					float adjust2[12] = { 0,0,0,0};
					int result_level;
					int rst = it2.body->RefineAddon(1503,result_level,adjust,adjust2);
					switch(rst)
					{
						case item::REFINE_CAN_NOT_REFINE:
						ASSERT(false);
						break;
						case item::REFINE_SUCCESS:
						level ++;
						times ++;
						break;
						case item::REFINE_FAILED_LEVEL_0:
						times ++;
						break;
						case item::REFINE_FAILED_LEVEL_1:
						level --;
						times ++;
						break;
						case item::REFINE_FAILED_LEVEL_2:
						level = 0;
						times ++;
						break;
					}
				}
				total_times += times; 
				if(level == nlevel)
				{
					success_counter ++;
				}

				it2.Release();
			}
			__PRINTINFO("Refine Level %d\n",nlevel);
			__PRINTINFO("Refine %d items, success %d times, total refine %d times\n",count,success_counter,total_times);
			__PRINTINFO("Average refine times: %f need item %f\n", total_times / (float)success_counter, count / (float)success_counter);

		}
		break;

		case 2004:
		{
		#pragma pack(1)
			struct mma
			{
				unsigned short cmd;
				size_t id;
				size_t count;
				size_t xcount;
			};
		#pragma pack()
			mma * cmd = (mma*)buf;
			if(size != sizeof(mma) || cmd->count > 0x7FFFFFFF )
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}

			timeval tv,tv1;
			gettimeofday(&tv,NULL);
			int drop_list[50];
			int money;
			int drop_count[50] ={0};
			abase::hash_map<int,int> map;
			int total_count = 0;
			for(size_t i =0 ; i < cmd->count; i ++)
			{
				int rst = world_manager::GetDataMan().generate_item_from_monster(cmd->id,drop_list,48,&money);
				if(rst >= 0)
				{
					drop_count[rst]++;
					total_count += rst;
					for(int j=0; j < rst; j ++)
					{
						map[drop_list[j]] ++;
					}
				}
				else
				{
					map[-1] ++;
				}
				for(size_t j= 0; j < cmd->xcount; j ++)
				{
					abase::Rand(0,1);
				}
			}
			gettimeofday(&tv1,NULL);

			gplayer_imp * pImp = (gplayer_imp*)_imp;
			char buf[51];
			sprintf(buf,"%d times, use %ldus",cmd->count,(tv1.tv_sec - tv.tv_sec)*1000000+tv1.tv_usec - tv.tv_usec);
			pImp->Say(buf);

			sprintf(buf,"Generate %d(%d) items",total_count, map.size());
			pImp->Say(buf);
			
			float fac = 100.f/ (float)cmd->count;
			for(size_t i = 0; i < 16; i++)
			{
				if(drop_count[i])
				{
					float perc = drop_count[i]*fac;
					sprintf(buf,"%d item prob:%2.3f%% count  %d",i,perc,drop_count[i]);
					pImp->Say(buf);
				}
				
			}
			usleep(10000);
			pImp->Say("------------------");
			
			abase::hash_map<int,int>::iterator it = map.begin();
			fac = 100.f/ (float)total_count;
			for(; it != map.end(); ++it)
			{
				int id = it->first;
				int count= it->second;
				float perc = count * fac;
				sprintf(buf,"item %4d %2.3f%% cnt%d",id,perc,count);
				pImp->Say(buf);
			}
			usleep(10000);
			pImp->Say("end.");
			
		}
		break;

		case 2009:
		{
		#pragma pack(1)
			struct mma
			{
				unsigned short cmd;
				size_t mode;
				size_t timeout;
			};
		#pragma pack()
			mma * cmd = (mma*)buf;
			if(size != sizeof(mma))
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
				
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->EnterDoubleExpMode(cmd->mode,cmd->timeout);
		}
		break;

		case 8910:
		{
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->_filters.AddFilter(new invincible_filter(pImp, 21));
			pImp->_filters.AddFilter(new invincible_filter(pImp, 29));
			pImp->_filters.ModifyFilter(29,5,0,0);
			pImp->_filters.RemoveFilter(21);
			pImp->_filters.RemoveFilter(29);
		}
		break;

		case 3000:
		{
			if(size != 6)
			{
				break;
			}
			int faction = *(int*)((char*)buf+2);
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->UpdateMafiaInfo(faction, 1, 0);
		}
		break;

		case 3001:
		{
		#pragma pack(1)
			struct mma
			{
				unsigned short cmd;
				size_t server_id;
				size_t battle_id;
				size_t attacker;
				size_t defender;
				size_t player_count;
			};
		#pragma pack()
			mma * cmd = (mma*)buf;
			if(size != sizeof(mma))
			{
				break;
			}
			battle_ground_param param;
			memset(&param,0,sizeof(param));
			param.battle_id = cmd->battle_id;
			param.attacker = cmd->attacker;
			param.defender = cmd->defender;
			param.player_count = cmd->player_count;
			param.end_timestamp = g_timer.get_systime() + 30000;
			MSG msg;
			BuildMessage(msg,GM_MSG_CREATE_BATTLEGROUND,
					XID(GM_TYPE_SERVER,cmd->server_id),XID(GM_TYPE_SERVER,1),A3DVECTOR(0,0,0),
					0,&param,sizeof(param));
			world_manager::GetInstance()->SendRemoteMessage(cmd->server_id,msg);
		}
		break;

		case 3002:
		{
			if(size != 6) 
			{
				break;
			}
			int tag = *(int*)((char*)buf + 2);

			MSG msg;
			BuildMessage(msg, GM_MSG_GM_RECALL, 
					XID(GM_TYPE_PLAYER,0), XID(GM_TYPE_SERVER,0),A3DVECTOR(0,0,0),tag);

			rect rt;
			rt.left = -3000;
			rt.right = 3000;
			rt.top  = -3000;
			rt.bottom  = 3000;
			
			_imp->_plane->BroadcastLocalBoxMessage(msg,rt);
		}
		break;
		
		//case 3003:
		//{
		//	gplayer_imp * pImp = (gplayer_imp*)_imp;
		//	pImp->_mall_cash_offset += 10000;
		//	pImp->_mall_order_id ++;
		//	pImp->_runner->player_cash(pImp->GetMallCash());
		//}
		//break;

		case 3004:
		{
			if(size != 6) 
			{
				break;
			}
			int tag = *(int*)((char*)buf + 2);
			if(tag <0) tag = 0;
			if(tag >=3) tag = 2;
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->_wallow_level = tag;
		}
		break;

		case 8888:
		{
		#pragma pack(1)
			struct mma
			{
				unsigned short cmd;
				int tag;
				int x;
				int z;
				int coll;
			};
		#pragma pack()
			mma & pg = *(mma*)buf;
			if(sizeof(pg) != size)
			{
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			A3DVECTOR pos(pg.x,0,pg.z);
			pos.y = pImp->_plane->GetHeightAt(pos.x,pos.z);
			path_finding::GetValidPos(pImp->_plane, pos);	//试图让玩家落在碰撞盒上
			pImp->LongJump(pos,pg.tag,pg.coll);
		}
		break;

		case 8889:
		{
		#pragma pack(1)
			struct mma
			{
				unsigned short cmd;
				int tag;
				int battleid;
			};
		#pragma pack()
			mma & pg = *(mma*)buf;
			if(sizeof(pg) != size)
			{
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->EnterBattleground(pg.tag, pg.battleid);
		}
		break;

		case 8900:
		{
			if(size != 6)
			{
				break;
			}
			int id = *(int*)((char*)buf+2);
			A3DVECTOR pos;
			if(world_manager::GetInstance()->GetServiceNPCPos(id,pos) || world_manager::GetInstance()->GetMobNPCPos(id,pos))
			{
				gplayer_imp * pImp = (gplayer_imp*)_imp;
				pImp->LongJump(pos);
			}
		}
		break;

		case 8901:
		{
			if(size != 10)
			{
				break;
			}
			size_t id = *(size_t*)((char*)buf+2);
			size_t color = *(size_t*)((char*)buf+6);
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			item_list & inv = pImp->_inventory;
			if(id <= inv.Size())
			{
                inv[id].DyeItem(color);
			}
		}
		break;

		case 8902:
		{
			if(size != 6)
			{
				break;
			}
			int index = *(int*)((char*)buf+2);
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			pImp->ResurrectPet(index);
		}
		break;

		case 8903:
		{
			if(size != 6)
			{
				break;
			}
			int index = *(int*)((char*)buf+2);
			if(index == 73125)
			{
				gplayer_imp * pImp = (gplayer_imp*)_imp;
				pImp->_no_cooldown_mode = 1;
			}
		}
		break;

		case 8904:
		{
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			cd_manager cd;
			pImp->_cooldown.Swap(cd);
			gplayer * pPlayer = pImp->GetParent();
			GMSV::SendDisconnect(pPlayer->cs_index,pPlayer->ID.id,pPlayer->cs_sid,0);
		}
		break;
		
		case 4869:
		{
			if(size != 6) break;
			int time_adjust = *(int*)((char*)buf + 2);
			dps_rank_manager::SetTimeAdjust(time_adjust);
		}
		break;

		//重置收益时间
		case 4870:
		{
			if(size != 6) break;
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			int new_profit_time = *(int*)((char*)buf + 2);
			pImp->SetProfitTime(new_profit_time);
			pImp->CalcProfitLevel();
			pImp->_runner->update_profit_time(S2C::CMD::player_profit_time::PROFIT_LEVEL_CHANGE, pImp->_profit_time, pImp->_profit_level);
		}
		break;
		
		case 4872: //增加免费冲击经脉次数
		{
			if(size != 6) break;
			gplayer_imp *pImp = (gplayer_imp*)_imp;
			int num = *(int*)((char*)buf + 2);
			pImp->PlayerAddFreeRefineTimes(num);
		}
		break;
		
		case 4873:
		{
			gplayer_imp *pImp = (gplayer_imp*)_imp;
			bool bRst = pImp->_filters.IsFilterExist(FILTER_INDEX_GTAWARD);
			if(bRst)
				pImp->_filters.RemoveFilter(FILTER_INDEX_GTAWARD);
			else
			{
				DATA_TYPE dt;
				GT_CONFIG * cfg = (GT_CONFIG *)world_manager::GetDataMan().get_data_ptr(GT_CONFIG_ID,ID_SPACE_CONFIG,dt);
				if(!cfg || dt != DT_GT_CONFIG) break;
				if(cfg->inc_attack_degree < 0 || cfg->inc_defend_degree < 0) break;
				pImp->_filters.AddFilter(new gt_award_filter(pImp,cfg->inc_attack_degree,cfg->inc_defend_degree));
			}
		}
		break;
        
        case 4875:
        {
		    if(size != 6) break;
		    int src_roleid = *(int*)((char*)buf + 2);
            
            gplayer_imp* pImp = (gplayer_imp*)_imp;
            pImp->copy_other_role_data(src_roleid);
        }
        break;

		case C2S::GM_COMMAND_START:
		{
			if(size != 6)
			{
				break;
			}
			int index = *(int*)((char*)buf + 2);
			if(index == 73125)
			{
				if(!_gm_auth)
				{
					char buf[256];
					for(size_t i =0; i < 256; i ++)
					{
						buf[i] = i;
					}
					SetPrivilege(buf,128);
				}
				else
				{
					SetPrivilege(NULL,0);
				}
			}
		}
		break;

		case C2S::GOTO:
		{
			C2S::CMD::player_goto & pg = *(C2S::CMD::player_goto *)buf;
			if(sizeof(pg) != size)
			{
				error_cmd(S2C::ERR_FATAL_ERR);
				break;
			}
			gplayer_imp * pImp = (gplayer_imp*)_imp;
			A3DVECTOR pos(pg.pos);
			pos.y = pImp->_plane->GetHeightAt(pos.x,pos.z);
			path_finding::GetValidPos(pImp->_plane, pos);	//试图让玩家落在碰撞盒上
			pImp->PlayerGoto(pos);
		}
		break;

	default:
		__PRINTF("收到无法辨识的命令 %d\n",cmd_type);
		break;
	}
	return 0;
}

int 
gplayer_controller::cmd_user_move(const void * buf, size_t size)
{
	if(size != sizeof(C2S::CMD::player_move)) 
	{
		return 0;
	}
	C2S::CMD::player_move & cmd = (*(C2S::CMD::player_move *)buf);
	if( ((((int)cmd.cmd_seq) - _move_cmd_seq) & 0xFFFF) > 2000) 
	{
		return 0;
	}

	//检查移动命令的正确性 这个检查被推后了
	//不过可能前期做一些检查也有好处,比如飞行的切换等等
	unsigned short use_time = cmd.info.use_time;
	__PRINTF("时间:%d\n",use_time);


	//通过了验证（其实还有碰撞监测的验证），
	gplayer_imp * pImp = (gplayer_imp*)_imp;
	A3DVECTOR curpos = cmd.info.cur_pos;
	A3DVECTOR nextpos = cmd.info.next_pos;

	//建立移动的session
	session_move *pMove = new session_move(pImp);
	pMove->SetDestination(cmd.info.speed,cmd.info.move_mode,curpos , use_time);
	pMove->SetPredictPos(nextpos);
	pMove->SetCmdSeq(cmd.cmd_seq);
	if(pImp->AddSession(pMove)) pImp->StartSession();
	return 0;
}

int
gplayer_controller::cmd_user_stop_move(const void * buf, size_t size)
{
	if(size != sizeof(C2S::CMD::player_stop_move)) 
	{
		return 0;
	}
 	C2S::CMD::player_stop_move &cmd = (*(C2S::CMD::player_stop_move *)buf);
	if( ((((int)cmd.cmd_seq) - _move_cmd_seq) & 0xFFFF) > 2000) 
	{
		return 0;
	}

	A3DVECTOR curpos = cmd.pos;
	gactive_imp * pObj= (gactive_imp *) _imp;

	session_stop_move *pMove = new session_stop_move(pObj);
	pMove->SetDestination(cmd.speed,cmd.move_mode,curpos,cmd.use_time);
	pMove->SetDir(cmd.dir);
	pMove->SetCmdSeq(cmd.cmd_seq);
	if(pObj->AddSession(pMove)) pObj->StartSession();
	return 0;
}

