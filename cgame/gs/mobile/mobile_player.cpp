#include "../clstab.h"
#include "../world.h"
#include "mobile_player.h"

DEFINE_SUBSTANCE(gplayer_mobile_imp,gplayer_imp,CLS_MOBILE_PLAYER_IMP)


int gplayer_mobile_imp::DispatchCommand(int cmd_type, const void * buf,size_t size)
{
	switch(cmd_type)
	{
		case C2S::LOGOUT:
		case C2S::GET_ALL_DATA:
		case C2S::CALC_NETWORK_DELAY:
		case C2S::GET_SERVER_TIMESTAMP:
		case C2S::RESURRECT_IN_TOWN:
		case C2S::RESURRECT_AT_ONCE:
	//属性相关
		case C2S::SELF_GET_PROPERTY:
		case C2S::SET_STATUS_POINT:
	//物品相关	
		case C2S::GET_INVENTORY:
/*		case C2S::GET_ITEM_INFO:
		case C2S::GET_INVENTORY_DETAIL:
		case C2S::EXCHANGE_INVENTORY_ITEM:
		case C2S::MOVE_INVENTORY_ITEM:
		case C2S::EXCHANGE_EQUIPMENT_ITEM:
		case C2S::EQUIP_ITEM:
		case C2S::MOVE_ITEM_TO_EQUIPMENT:
		case C2S::GET_OWN_WEALTH:
		case C2S::GET_ITEM_INFO_LIST:
		case C2S::GET_TRASHBOX_INFO:
		case C2S::EXCHANGE_TRASHBOX_ITEM:
		case C2S::MOVE_TRASHBOX_ITEM:
		case C2S::EXHCANGE_TRASHBOX_INVENTORY:
		case C2S::MOVE_TRASHBOX_ITEM_TO_INVENTORY:
		case C2S::MOVE_INVENTORY_ITEM_TO_TRASHBOX:
		case C2S::EXCHANGE_TRASHBOX_MONEY:
		case C2S::EQUIP_TRASHBOX_FASHION_ITEM:
		case C2S::CHECK_SECURITY_PASSWD:
		case C2S::MULTI_EXCHANGE_ITEM:
	//任务	
		case C2S::TASK_NOTIFY:*/
	//签到相关
		case C2S::DAILY_SIGNIN:
		case C2S::LATE_SIGNIN:
		case C2S::APPLY_SIGNIN_AWARD:
		case C2S::REFRESH_SIGNIN:
			return gplayer_imp::DispatchCommand(cmd_type,buf,size);

		default:
			_runner->error_message(S2C::ERR_FATAL_ERR);
			return 0;
	}
}

const A3DVECTOR & gplayer_mobile_imp::GetLogoutPos(int & world_tag)
{
	world_tag = _real_world_tag;
	return _real_pos;
}

void gplayer_mobile_imp::SaveRealWorldPos(int world_tag, const A3DVECTOR & pos)
{
	_real_world_tag = world_tag;
	_real_pos = pos;
}

void gplayer_mobile_imp::SendAllData(bool detail_inv, bool detail_equip, bool detail_task)
{
	PlayerGetProperty();
	_runner->realm_exp_receive(_realm_exp,0);
	_runner->send_cooldown_data();
	_runner->send_timestamp();
}
