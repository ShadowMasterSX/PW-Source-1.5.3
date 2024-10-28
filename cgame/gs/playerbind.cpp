#include "playerbind.h"
#include "world.h"
#include "player_imp.h"
#include "obj_interface.h"
#include "duel_filter.h"
#include "filter_man.h"

void
player_bind::PlayerLinkRequest(gplayer_imp * pImp, const XID & target)
{
	if(!pImp->IsPlayerFemale())
	{
		pImp->_runner->error_message(S2C::ERR_INVALID_GENDER);
		return;
	}
	
	//����Լ���״̬
	//�ܹ���������������� 1:�������� 2:���ڱ������� 3:û����Ҫ���� 4:����ˮ�� 5:����...
	if(_mode || !pImp->CheckPlayerBindRequest() || !target.IsPlayer())
	{
		pImp->_runner->error_message(S2C::ERR_INVALID_BIND_REQUEST);
		return;
	}

	if(world_manager::GetWorldLimit().nobind)
	{
		pImp->_runner->error_message(S2C::ERR_CANNOT_BIND_HERE);
		return;
	}

	//����״̬
	_mode = MODE_REQUEST;
	_request_target = target;
	_timeout = NORMAL_TIMEOUT;

	//������Ϣ
	pImp->SendTo<0>(GM_MSG_PLAYER_BIND_REQUEST,target,0);

}

void
player_bind::PlayerLinkInvite(gplayer_imp * pImp, const XID & target)
{
	if(pImp->GetPlayerLimit(PLAYER_LIMIT_NOBIND))//��ֹ��������
		return;
	
	if(pImp->IsPlayerFemale())
	{
		pImp->_runner->error_message(S2C::ERR_INVALID_GENDER);
		return;
	}

	//����Լ���״̬
	//�ܹ���������������� 1:�������� 2:���ڱ������� 3:û����Ҫ���� 4:����ˮ�� 5:����...
	if(_mode || !pImp->CheckPlayerBindInvite() || !target.IsPlayer() )
	{
		pImp->_runner->error_message(S2C::ERR_INVALID_BIND_REQUEST);
		return;
	}

	if(world_manager::GetWorldLimit().nobind)
	{
		pImp->_runner->error_message(S2C::ERR_CANNOT_BIND_HERE);
		return;
	}

	//����״̬
	_mode = MODE_INVITE;
	_invite_target = target;
	_timeout = NORMAL_TIMEOUT;

	//������Ϣ
	pImp->SendTo<0>(GM_MSG_PLAYER_BIND_INVITE,target,0);
}

void
player_bind::PlayerLinkReqReply(gplayer_imp * pImp, const XID & target,int param)
{
	if(pImp->IsPlayerFemale())
	{
		pImp->_runner->error_message(S2C::ERR_INVALID_GENDER);
		return;
	}

	//����Լ���״̬
	//�ܹ���Ӧ������������ܹ��������������
	if((_mode && _mode != MODE_INVITE ) || !pImp->CheckPlayerBindInvite() || !target.IsPlayer() || pImp->GetPlayerLimit(PLAYER_LIMIT_NOBIND))
	{
		pImp->_runner->error_message(S2C::ERR_INVALID_BIND_REQUEST);
		return;
	}

	if(world_manager::GetWorldLimit().nobind)
	{
		pImp->_runner->error_message(S2C::ERR_CANNOT_BIND_HERE);
		return;
	}

	//����״̬
	_mode = MODE_PREPARE;
	_bind_target = target;
	_timeout = PREPARE_TIMEOUT;

	//������Ϣ
	pImp->SendTo<0>(GM_MSG_PLAYER_BIND_REQUEST_REPLY,target,param);
}

void
player_bind::PlayerLinkInvReply(gplayer_imp * pImp, const XID & target,int param)
{
	if(!pImp->IsPlayerFemale())
	{
		pImp->_runner->error_message(S2C::ERR_INVALID_GENDER);
		return;
	}

	//����Լ���״̬
	//�ܹ���Ӧ������������ܹ��������������
	if((_mode && _mode != MODE_REQUEST) || !pImp->CheckPlayerBindRequest() || !target.IsPlayer() || pImp->GetPlayerLimit(PLAYER_LIMIT_NOBIND))
	{
		pImp->_runner->error_message(S2C::ERR_INVALID_BIND_REQUEST);
		return;
	}

	if(world_manager::GetWorldLimit().nobind)
	{
		pImp->_runner->error_message(S2C::ERR_CANNOT_BIND_HERE);
		return;
	}

	//����״̬
	_mode = MODE_PREPARE;
	_bind_target = target;
	_timeout = PREPARE_TIMEOUT;

	//������Ϣ
	pImp->SendTo<0>(GM_MSG_PLAYER_BIND_INVITE_REPLY,target,param);

}


void 
player_bind::MsgRequest(gplayer_imp * pImp, const XID & target)
{
	//��
	if(pImp->IsPlayerFemale()) return;

	//�յ������Ĳ����Ǽ���Ƿ��ڿ��Ա������״̬��������ܣ�ֱ�ӷ���һ���ܾ�����
	//�йؾ�����ж϶����������
	//Ŀǰ�ļ����Ӧ�����ڷǱ���͵ȴ���״̬����
	if(_mode && ( _mode == MODE_LINKED || _mode == MODE_PREPARE)
		|| ((gplayer*)pImp->_parent)->IsInvisible()
		|| pImp->GetPlayerLimit(PLAYER_LIMIT_NOBIND))
	{
		//ֱ�ӷ���һ���ܾ���Ϣ����
		pImp->SendTo<0>(GM_MSG_PLAYER_BIND_REQUEST_REPLY,target,ERR_CANNOT_BIND);
		return;
		
	}

	//֪ͨ��Ϣ���ͻ���
	pImp->_runner->player_bind_request(target);
}

void 
player_bind::MsgInvite(gplayer_imp * pImp, const XID & target)
{
	//Ů
	if(!pImp->IsPlayerFemale()) return;


	//�յ������Ĳ�������������
	if(_mode && ( _mode == MODE_LINKED || _mode == MODE_PREPARE)
		|| ((gplayer*)pImp->_parent)->IsInvisible()
		|| pImp->GetPlayerLimit(PLAYER_LIMIT_NOBIND))
	{
		//ֱ�ӷ���һ���ܾ���Ϣ����
		pImp->SendTo<0>(GM_MSG_PLAYER_BIND_INVITE_REPLY,target,ERR_CANNOT_BIND);
		return;
		
	}

	//֪ͨ��Ϣ���ͻ���
	pImp->_runner->player_bind_invite(target);

}

void 
player_bind::MsgRequestReply(gplayer_imp * pImp, const XID & target, int param)
{
	//Ů
	//�յ�����Ļ�Ӧ���ܹ������ǰ�����Լ�����������ǵ�ǰ����
	//�ҵ�ǰ��������״̬
	if(target != _request_target || _mode != MODE_REQUEST || pImp->GetPlayerLimit(PLAYER_LIMIT_NOBIND))
	{
		//����һ����Ϣ �öԷ�����ȴ�״̬
		if(!param) pImp->SendTo<0>(GM_MSG_PLAYER_BIND_STOP,target, 0);
		return;
	}
	
	//������������Ǿܾ���Ϣ���������ǰ״̬�����б�Ҫ�Ĳ���
	if(param)
	{
		//�˲���Ҫ���ظ��ͻ���
		pImp->_runner->player_bind_request_reply(target,param);

		//�����ǰ������״̬
		_mode = MODE_EMPTY;
		_request_target = XID(-1,-1);
		return;
	}
	
	_mode = MODE_PREPARE;
	_bind_target = target;
	_timeout = PREPARE_TIMEOUT;

	//���ص��ǳɹ���Ϣ,��ͨ��֮,����֪ͨ�Է�������Խ����״̬,Ȼ�����з������
	pImp->SendTo<0>(GM_MSG_PLAYER_BIND_PREPARE,target,0);
}

void 
player_bind::MsgInviteReply(gplayer_imp * pImp, const XID & target, int param)
{
	//��
	//�յ�����Ļ�Ӧ,�����������������˺ͷ��ض�Ӧ
	//�ҵ�ǰ��������״̬
	if(target != _invite_target || _mode != MODE_INVITE || !pImp->CheckPlayerBindRequest() || pImp->GetPlayerLimit(PLAYER_LIMIT_NOBIND))
	{
		//����һ����Ϣ �öԷ�����ȴ�״̬
		if(!param) pImp->SendTo<0>(GM_MSG_PLAYER_BIND_STOP,target, 0);
		return;
	}
	
	//������������Ǿܾ���Ϣ���������ǰ״̬�����б�Ҫ�Ĳ���
	if(param)
	{
		//�˲���Ҫ���ظ��ͻ���
		pImp->_runner->player_bind_invite_reply(target,param);

		//�����ǰ������״̬
		_mode = MODE_EMPTY;
		_invite_target = XID(-1,-1);
		return;
	}
	if(!pImp->EnterBindMoveState(target))
	{
		//����״̬ʧ�� ��ֱ�ӷ���
		return ;
	}
	//�ɶԷ����Ͱ󶨿�ʼ����Ϣ

	_mode = MODE_LINKED;
	_bind_target = target;
	//�Է�ͬ�⿪ʼ����,ֱ�ӽ����״̬

	//֪ͨ�Է���ʼ��
	pImp->SendTo<0>(GM_MSG_PLAYER_BIND_LINK,target,0);
}

void 
player_bind::MsgPrepare(gplayer_imp *pImp, const XID & target)
{
	//���յ�����Ϣ�ı�Ȼ������, ͬʱ��Ȼ���ڵȴ���ҵ�״̬ MODE_PREPARE
	if(_mode != MODE_PREPARE || target != _bind_target || !pImp->CheckPlayerBindRequest() || pImp->GetPlayerLimit(PLAYER_LIMIT_NOBIND))
	{
		//״̬��ƥ�����Ŀ�겻ƥ�������
		return;
	}
	//���һ���Ƿ�Ϊ����
	ASSERT(!pImp->IsPlayerFemale());

	//�Է�ͬ�⿪ʼ����,ֱ�ӽ����״̬
	if(!pImp->EnterBindMoveState(target))
	{
		//����״̬ʧ�� ��ֱ�ӷ���
		return ;
	}
	//״̬һ��,����link��״̬
	_mode = MODE_LINKED;
	//�ɶԷ����Ͱ󶨿�ʼ����Ϣ
	pImp->SendTo<0>(GM_MSG_PLAYER_BIND_LINK,target,0);
}

void 
player_bind::MsgBeLinked(gplayer_imp * pImp, const XID & target,const A3DVECTOR & pos)
{
	if(_mode != MODE_PREPARE || _bind_target != target || !pImp->CheckPlayerBindRequest() || pImp->GetPlayerLimit(PLAYER_LIMIT_NOBIND))
	{
		//���Ƿ�Ԥ�ڵ�״̬�򲻴���
		//����һ���Լ���״̬�Ƿ���� (�Ƿ��ڿ��Խ����ƶ���״̬,���״̬�������״̬һ��)
		//�Ƿ񷵻�һ����Ϣ?
		pImp->SendTo<0>(GM_MSG_PLAYER_BIND_STOP,target, 0);
		return;
	}

	if(!pImp->EnterBindFollowState(target))
	{
		//����״̬ʧ����ֱ�ӷ���
		return;
	}
	
	_mode = MODE_FOLLOW;
	A3DVECTOR tmp = pos;
	tmp -= pImp->_parent->pos;
	pImp->StepMove(tmp);

	pImp->_runner->player_bind_start(target);
	//�����״̬
	
}

void 
player_bind::MsgFollowOther(gplayer_imp * pImp, const XID & target,const A3DVECTOR & pos)
{
	//������ҷ�����Ҫ��������Ϣ
	if(_mode != MODE_FOLLOW || target != _bind_target)
	{
		//���账��
		//���߿��Ƿ���һ��ֹͣlink
		if(target != _bind_target || _mode != MODE_PREPARE)
		{
			//���ǵ�����Ϣ˳�����,�������е�״̬�����ش�����Ϣ
			pImp->SendTo<0>(GM_MSG_PLAYER_BIND_STOP,target, 0);
		}
		return ;
	}
	//��������ƶ�
	A3DVECTOR tmp = pos;
	tmp -= pImp->_parent->pos;
	pImp->StepMove(tmp);
//	pImp->_runner->stop_move(pos,0x7F00,0,0);
}

void 
player_bind::MsgStopLinked(gplayer_imp * pImp, const XID & target)
{
	//�յ�ȡ��link����Ϣ 
	if((_mode == MODE_LINKED || _mode == MODE_FOLLOW ) && target == _bind_target)
	{
		pImp->_runner->player_bind_stop();
		pImp->ReturnBindNormalState();
		_mode = MODE_EMPTY;
		_bind_target = XID(-1,-1);
	}
}


void 
player_bind::Heartbeat(gplayer_imp * pImp)
{
	if(!_mode) return ;
	switch(_mode)
	{
		case MODE_REQUEST:
		_timeout --;
		if(_timeout <=0)
		{
			//����ʱ�����Կ��Ƿ��ؿͻ���һ����Ϣ
			//ͬʱ��յ�ǰ״̬
			_mode = MODE_EMPTY;
			_request_target = XID(-1,-1);
		}
		break;
		
		case MODE_INVITE:
		_timeout --;
		if(_timeout <=0)
		{
			//����ʱ�����Կ��Ƿ��ؿͻ���һ����Ϣ
			//ͬʱ��յ�ǰ״̬
			_mode = MODE_EMPTY;
			_invite_target = XID(-1,-1);
		}
		break;

		case MODE_PREPARE:
		_timeout --;
		if(_timeout <=0)
		{
			//����ʱ�����Կ��Ƿ��ؿͻ���һ����Ϣ
			//ͬʱ��յ�ǰ״̬
			_mode = MODE_EMPTY;
			_bind_target = XID(-1,-1);
		}
		break;

		case MODE_LINKED:
		case MODE_FOLLOW:
		//������״̬��Ҫ�������Զ�� ��ͬʱ��������
		{
			//û�г�ʱ�����з�Χ��λ�õ��ж�
			world::object_info info;
			if(!pImp->_plane->QueryObject(_bind_target,info)
					|| (info.state & world::QUERY_OBJECT_STATE_ZOMBIE)
					|| (info.pos.squared_distance(pImp->_parent->pos)) >= 50.f*50.f)
			{
				//�����ѯ��������,���߾����Զ �������˳�������
				pImp->ReturnBindNormalState();
				pImp->_runner->player_bind_stop();
				_mode = MODE_EMPTY;
				_bind_target = XID(-1,-1);
			}
			else
			{
				//ÿ�볤Ԫ��5��
				pImp->ModifyAP(5);
			}
		}

		break;
	}
}

void 
player_bind::PlayerCancel(gplayer_imp *pImp)
{
	ASSERT(_mode == MODE_LINKED || _mode == MODE_FOLLOW);

	//���Է�����һ��ֹͣ�󶨵���Ϣ
	pImp->SendTo<0>(GM_MSG_PLAYER_BIND_STOP,_bind_target, 0);
	
	//ת�Ƶ�����״̬ 
	pImp->ReturnBindNormalState();
	pImp->_runner->player_bind_stop();
	_mode = MODE_EMPTY;
	_bind_target = XID(-1,-1);
}

