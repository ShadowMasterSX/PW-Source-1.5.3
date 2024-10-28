#include "countrybattle_manager.h"
#include "../world.h"
#include "../player_imp.h"
#include "../usermsg.h"
#include "countrybattle_ctrl.h"

/*
 *	����������Ϣ
 */
int
countrybattle_world_message_handler::RecvExternMessage(int msg_tag, const MSG & msg)
{
	//���ڸ�����ֻ���ܸ���ҵ���Ϣ
	if(msg.target.type != GM_TYPE_PLAYER && msg.target.type != GM_TYPE_SERVER ) return 0;
	if(msg_tag != world_manager::GetWorldTag())
	{
		//����ĳЩ��Ϣ.......
	}

	//����ҪҪ����ĳЩ��Ϣ
	//���ﻹӦ��ֱ�Ӵ���ĳЩ��Ϣ
	//����ת����Ҫ�����ж�

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

		case GM_MSG_CREATE_COUNTRYBATTLE:
		{
			country_battle_param & param = *(country_battle_param*) msg.content;
			_manager->CreateCountryBattle(param);
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
countrybattle_world_message_handler::HandleMessage(world * pPlane,const MSG & msg)
{
	//��Щ��Ϣ�������ܻ�ȽϷ�ʱ�䣬�Ƿ���Կ���Task��ɣ��������̵߳Ļ���Ҫ����msg�����������ˡ�

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
countrybattle_world_message_handler::PlayerPreEnterServer(gplayer * pPlayer, gplayer_imp * pImp,instance_key & ikey)
{	
	world * pPlane = pImp->_plane;
	
	//������ҵİ����趨�������ط�
	//���������Ĳ����� ��ҵ�EnterServer���������
	countrybattle_ctrl * pCtrl = (countrybattle_ctrl*)(pPlane->w_ctrl);

	int id = pPlayer->GetCountryId();
	if(id)
	{
		if(id == pCtrl->_data.country_attacker)
		{
			//����
			pPlayer->SetBattleOffense();
			//����������� ע�����������������player_battle���PlayerLeaveWorld������
			if(!pCtrl->AddAttacker())
			{
				//��������,�������Ĺ�������
				ikey.target.key_level5 = 0;

				//���ս���ı�־(��������)
				pPlayer->ClrBattleMode();
			}
		}
		else
		if(id == pCtrl->_data.country_defender)
		{	
			//�ط�
			pPlayer->SetBattleDefence();
			//����������� ע�����������������player_battle���PlayerLeaveWorld������
			if(!pCtrl->AddDefender())
			{
				//��������,�������Ĺ�������
				ikey.target.key_level5 = 0;

				//���ս���ı�־(��������)
				pPlayer->ClrBattleMode();
			}
		}
	}
}

