#include "mnfaction_manager.h"
#include "../world.h"
#include "../player_imp.h"
#include "../usermsg.h"
#include "mnfaction_ctrl.h"


void mnfaction_world_message_handler::PlayerPreEnterServer(gplayer * pPlayer, gplayer_imp * pImp,instance_key & ikey)
{	
	world * pPlane = pImp->_plane;
	
	//根据玩家的帮派设定攻方和守方
	//增加人数的操作在 玩家的EnterServer操作中完成
	mnfaction_ctrl * pCtrl = (mnfaction_ctrl*)(pPlane->w_ctrl);

	int64_t faction_id = 0;
	pImp->GetDBMNFactionInfo(faction_id);
	if(faction_id)
	{
		if(faction_id == pCtrl->_attacker_faction_id)
		{
			//攻方
			pPlayer->SetBattleOffense();
			//检查人数上限 注意减少人数上限是在player_battle里的PlayerLeaveWorld里做的
			if(!pCtrl->AddAttacker(pPlayer))
			{
				//人数已满,清除里面的帮派内容
				ikey.target.key_level3 = 0;

				//清除战场的标志(后面有用)
				pPlayer->ClrBattleMode();
			}
		}
		else
		if(faction_id == pCtrl->_defender_faction_id)
		{	
			//守方
			pPlayer->SetBattleDefence();
			//检查人数上限 注意减少人数上限是在player_battle里的PlayerLeaveWorld里做的
			if(!pCtrl->AddDefender(pPlayer))
			{
				//人数已满,清除里面的帮派内容
				ikey.target.key_level3 = 0;

				//清除战场的标志(后面有用)
				pPlayer->ClrBattleMode();
			}
		}
	}
}

int mnfaction_world_message_handler::RecvExternMessage(int msg_tag, const MSG & msg)
{
	//对于副本，只接受给万家的消息
	if(msg.target.type != GM_TYPE_PLAYER && msg.target.type != GM_TYPE_SERVER ) return 0;
	if(msg_tag != world_manager::GetWorldTag())
	{
		//隔断某些消息.......
	
	}
	switch(msg.message)
	{
		case GM_MSG_CREATE_MNFACTION:
		{
			mnfaction_battle_param & param = *(mnfaction_battle_param*) msg.content;
			_manager->CreateMNFactionBattle(param);
		}
		return 0;
	}
	return instance_world_message_handler::RecvExternMessage(msg_tag, msg);
}
