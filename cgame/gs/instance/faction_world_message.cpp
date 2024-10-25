#include "instance_manager.h"
#include "../world.h"
#include "../player_imp.h"
#include "../usermsg.h"
#include "faction_world_ctrl.h"

/*
 *	����������Ϣ
 */
int
faction_world_message_handler::RecvExternMessage(int msg_tag, const MSG & msg)
{
	//���ڸ�����ֻ���ܸ���ҵ���Ϣ
	if(msg.target.type != GM_TYPE_PLAYER && msg.target.type != GM_TYPE_SERVER ) return 0;
	if(msg_tag != world_manager::GetWorldTag())
	{
		//����ĳЩ��Ϣ.......
	}

	//����ҪҪ����ĳЩ��Ϣ
	//$$$$$$���ﻹӦ��ֱ�Ӵ���ĳЩ��Ϣ
	//$$$$$$$$����ת����Ҫ�����ж�

	//��Щ��Ϣ��Ҫ��������ת��
	switch(msg.message)
	{
		case GM_MSG_SWITCH_USER_DATA:
			{
				if(msg.content_length < sizeof(instance_key)) return 0;
				instance_key * key = (instance_key*)msg.content;
				//��Ϣ��ͷ��������instance_key
				//���渽�����������
				//ASSERT(key->target.key_level1 == msg.source.id);
				//���ﲻ�����ˣ���Ҫ��GM���д���Ϊ
				instance_hash_key hkey;
				_manager->TransformInstanceKey(key->target, hkey);
				int index = _manager->GetWorldByKey(hkey);
				if(index < 0) return 0;
				return _manager->GetWorldByIndex(index)->DispatchMessage(msg);
			}

		case GM_MSG_PLANE_SWITCH_REQUEST:
		//ȷ���л����� 
		//��鸱�������Ƿ���ڣ���������ڣ������ȴ��б�?		//����Ѿ����ڣ���ˢ��һ�·�������ʱ���־�������سɹ���־
		//�������Ҫ��������������ɾ������ 
		{
			if(msg.content_length != sizeof(instance_key)) 
			{
				ASSERT(false);
				return 0;
			}
			instance_key * key = (instance_key*)msg.content;
			int rst = _manager->CheckPlayerSwitchRequest(msg.source,key,msg.pos,msg.param);
			if(rst == 0)
			{
				//���л�����Ϣ
				MSG nmsg = msg;
				nmsg.target = msg.source;
				nmsg.source = msg.target;
				nmsg.message = GM_MSG_PLANE_SWITCH_REPLY;
				_manager->SendRemotePlayerMsg(msg.source.id, nmsg);
			}
			else if(rst > 0)
			{
				MSG nmsg;
				BuildMessage(nmsg,GM_MSG_ERROR_MESSAGE,msg.source,msg.target,msg.pos,rst);
				_manager->SendRemotePlayerMsg(msg.source.id, nmsg);
			}
			//���rstС��0��������ǰ�޷�ȷ���Ƿ��ܹ��������磬��Ҫ�ȴ� ���Է���ʲô������
		}
		return 0;

		default:
		if(msg.target.type == GM_TYPE_PLAYER)
		{
			int index = _manager->GetPlayerWorldIdx(msg.target.id);
			if(index < 0) return 0;
			return _manager->GetWorldByIndex(index)->DispatchMessage(msg);
		}
		//��������Ϣ��δ���� ..........
	}
	return 0;
}

int
faction_world_message_handler::HandleMessage(world * pPlane,const MSG & msg)
{
	//��Щ��Ϣ�������ܻ�ȽϷ�ʱ�䣬�Ƿ���Կ���Task��ɣ��������̵߳Ļ���Ҫ����msg�����������ˡ�

	//���ǽ�һЩ�����ƶ��������� $$$$$$
	switch(msg.message)
	{
		case GM_MSG_SWITCH_USER_DATA:
			return PlayerComeIn(_manager,pPlane,msg);

		default:
			world_message_handler::HandleMessage(pPlane,msg);
	}
	return 0;
}

void 
faction_world_message_handler::PlayerPreEnterServer(gplayer * pPlayer, gplayer_imp * pImp,instance_key &  ikey)
{
	world * pPlane = pImp->_plane;
	//������ҵİ����趨�������ط�
	//���������Ĳ����� ��ҵ�EnterServer���������
	faction_world_ctrl * pCtrl = (faction_world_ctrl *)pPlane->w_ctrl;

	if(pPlayer->id_mafia == pCtrl->factionid)
	{
		//�ط�
		pPlayer->SetBattleDefence();
		//����������� ע�����������������player_battle���PlayerLeaveWorld������
		if(!pCtrl->AddDefender())
		{
			//��������,�������İ�������
			ikey.target.key_level3 = -1;

			//���ս���ı�־(��������)
			pPlayer->ClrBattleMode();
		}
	}
	else if(pCtrl->inbattle && pPlayer->id_mafia == pCtrl->offense_faction)
	{
		//����
		pPlayer->SetBattleOffense();
		//����������� ע�����������������player_battle���PlayerLeaveWorld������
		if(!pCtrl->AddAttacker())
		{
			//��������,�������İ�������
			ikey.target.key_level3 = -1;

			//���ս���ı�־(��������)
			pPlayer->ClrBattleMode();
		}
	}
}
