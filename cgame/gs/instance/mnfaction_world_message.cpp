#include "mnfaction_manager.h"
#include "../world.h"
#include "../player_imp.h"
#include "../usermsg.h"
#include "mnfaction_ctrl.h"


void mnfaction_world_message_handler::PlayerPreEnterServer(gplayer * pPlayer, gplayer_imp * pImp,instance_key & ikey)
{	
	world * pPlane = pImp->_plane;
	
	//������ҵİ����趨�������ط�
	//���������Ĳ����� ��ҵ�EnterServer���������
	mnfaction_ctrl * pCtrl = (mnfaction_ctrl*)(pPlane->w_ctrl);

	int64_t faction_id = 0;
	pImp->GetDBMNFactionInfo(faction_id);
	if(faction_id)
	{
		if(faction_id == pCtrl->_attacker_faction_id)
		{
			//����
			pPlayer->SetBattleOffense();
			//����������� ע�����������������player_battle���PlayerLeaveWorld������
			if(!pCtrl->AddAttacker(pPlayer))
			{
				//��������,�������İ�������
				ikey.target.key_level3 = 0;

				//���ս���ı�־(��������)
				pPlayer->ClrBattleMode();
			}
		}
		else
		if(faction_id == pCtrl->_defender_faction_id)
		{	
			//�ط�
			pPlayer->SetBattleDefence();
			//����������� ע�����������������player_battle���PlayerLeaveWorld������
			if(!pCtrl->AddDefender(pPlayer))
			{
				//��������,�������İ�������
				ikey.target.key_level3 = 0;

				//���ս���ı�־(��������)
				pPlayer->ClrBattleMode();
			}
		}
	}
}

int mnfaction_world_message_handler::RecvExternMessage(int msg_tag, const MSG & msg)
{
	//���ڸ�����ֻ���ܸ���ҵ���Ϣ
	if(msg.target.type != GM_TYPE_PLAYER && msg.target.type != GM_TYPE_SERVER ) return 0;
	if(msg_tag != world_manager::GetWorldTag())
	{
		//����ĳЩ��Ϣ.......
	
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
