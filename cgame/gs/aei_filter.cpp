#include "string.h"
#include "clstab.h"
#include "world.h"
#include "common/message.h"
#include "aei_filter.h"

#include "player_imp.h"
#include "sfilterdef.h"
#include "instance/faction_world_ctrl.h"

DEFINE_SUBSTANCE(aei_filter,filter,CLS_FILTER_CHECK_INSTANCE_KEY)
DEFINE_SUBSTANCE(aebf_filter,filter,CLS_FILTER_CHECK_BATTLEFIELD_KEY)
DEFINE_SUBSTANCE(aeff_filter,filter,CLS_FILTER_CHECK_FACTIONFORTRESS_KEY)
DEFINE_SUBSTANCE(aegw_filter,filter,CLS_FILTER_CHECK_KICKOUT)
DEFINE_SUBSTANCE(aect_filter,filter,CLS_FILTER_CHECK_COUNTRYKICKOUT)
DEFINE_SUBSTANCE(aecb_filter,filter,CLS_FILTER_CHECK_COUNTRYBATTLE_KEY)
DEFINE_SUBSTANCE(aetb_filter,filter,CLS_FILTER_CHECK_TRICKBATTLE_KEY)

void 
aei_filter::OnAttach()
{
	_key =_parent.GetImpl()->_plane->w_ins_key;
}

void 
aei_filter::OnRelease()
{
	//do nothing
}

void 
aei_filter::Heartbeat(int tick)
{
	int world_tag = world_manager::GetWorldTag();
	bool is_dead = _parent.IsDead();
	gplayer_imp * pImp = (gplayer_imp*)_parent.GetImpl();
	bool is_kick = pImp->_plane->w_ins_kick;
	if(_state == NORMAL && (_timeout -= tick) < 0)
	{
		instance_key key;
		pImp->GetInstanceKey(world_tag, key);
		key.target = key.essence;
		//这里的比较key是否一致是否应该由world_manager来判断？
		bool key_match = world_manager::GetInstance()->CompareInsKey(key,_key);
		bool playercount_exceed = (pImp->GetTeamCtrl().GetMemberCountInSpecWorld(world_tag) > world_manager::GetInstance()->GetPlayerLimitPerInstance());
		if((!key_match || playercount_exceed) && !is_dead && is_kick)
		{
			//需要踢出，检查是否GM
			if(world_manager::GetWorldLimit().gmfree && _parent.CheckGMPrivilege())
			{
				//GM 且本世界允许GM随意穿越 ，则不会被踢出
				_timeout = 3;
			}
			else
			{
				_state = WAIT_ESCAPE;
				_timeout = 60;
				pImp->_runner->kickout_instance((key_match ? KIR_KEY_MISMATCH : KIR_PLAYERCOUNT_EXCEED), _timeout);
			}
		}
		else
		{
			//每三秒查一次
			_timeout = 3;
			if(key_match && pImp->_plane->w_life_time == 0)
			{
				//副本已过期,需要重置
				pImp->ResetInstance(world_tag);
			}
		}
	}
	else if(_state == WAIT_ESCAPE)
	{
		if((_timeout -= tick) <= 0)
		{
			//到时间了，踢出副本
			__PRINTF("副本该踢出辣\n");
			pImp->LeaveAbnormalState();
			if(!pImp->ReturnToTown())
			{
				if(world_manager::GetSavePoint().tag > 0)
				{
					pImp->LongJump(world_manager::GetSavePoint().pos,world_manager::GetSavePoint().tag);
				}
			}
			//重置一下timeout
			_timeout = 5;
		}
		else
		{
			//再检查是否恢复了
			instance_key key;
			pImp->GetInstanceKey(world_tag, key);
			key.target = key.essence;
			bool key_match = world_manager::GetInstance()->CompareInsKey(key,_key);
			bool playercount_exceed = (pImp->GetTeamCtrl().GetMemberCountInSpecWorld(world_tag) > world_manager::GetInstance()->GetPlayerLimitPerInstance());
			if(key_match && !playercount_exceed || is_dead || !is_kick)
			{
				_state = NORMAL;
				_timeout = 3;
				pImp->_runner->kickout_instance(KIR_NULL,-1);
			}
		}
	}
}

/*--------------------------------------------------------*/
void 
aebf_filter::OnAttach()
{
	if(_attack_defend_award)
	{	
		_parent.InsertTeamVisibleState(254);
		_parent.EnhanceAttackDegree(_attack_defend_award);
		_parent.EnhanceDefendDegree(_attack_defend_award);
	}
}

void 
aebf_filter::OnRelease()
{
	if(_attack_defend_award)
	{	
		_parent.RemoveTeamVisibleState(254);
		_parent.ImpairAttackDegree(_attack_defend_award);
		_parent.ImpairDefendDegree(_attack_defend_award);
	}
}

void  
aebf_filter::OnModify(int ctrlname,void * ctrlval,size_t ctrllen)
{
	if(ctrlname == FMID_CLEAR_AEBF)
	{
		_origin_mafia = -1;
	}
}

void 
aebf_filter::Heartbeat(int tick)
{
	int cur_mafia = _parent.GetMafiaID();
	gplayer_imp * pImp = (gplayer_imp*)_parent.GetImpl();
	world * pPlane = pImp->_plane;

//计算和统计分数
	int attacker_score = pPlane->w_offense_cur_score;
	int defender_score = pPlane->w_defence_cur_score;

	if(attacker_score != _attacker_score || defender_score != _defender_score)
	{
		//攻击和防守的分数要反过来
		pImp->_runner->battle_score( defender_score,pPlane->w_defence_goal, attacker_score,pPlane->w_offense_goal);

		_attacker_score = attacker_score;
		_defender_score = defender_score;
	}

//分析结果是否出现
	if(!_battle_result)
	{
		if(pImp->_plane->w_battle_result != _battle_result)
		{
			_battle_result = pImp->_plane->w_battle_result;
			pImp->_runner->battle_result(_battle_result);

			//激活准备离开的操作
			_battle_end_timer = 4;
			_timeout = abase::Rand(55,90);
		}
	}

//准备退出倒数
	if(_battle_end_timer)
	{
		_battle_end_timer --;
		if(_battle_end_timer <= 0)
		{
			_timeout = abase::Rand(55,90);
			pImp->_runner->kickout_instance(KIR_BATTLE_END, _timeout);
		}
	}

	
	
	if(_battle_result)
	{
		if(_timeout > 0)  _timeout --;
		if(_timeout <=0)
		{
			_kickout ++;
		}
	
	}
	else
	{
		if(cur_mafia != _origin_mafia || _origin_mafia <= 0)
		if(_origin_mafia <= 0 || 
		   	cur_mafia != _origin_mafia &&
		    	!(world_manager::GetWorldLimit().gmfree && _parent.CheckGMPrivilege()) )
		{
			_origin_mafia = -1;
			if(_timeout > 0)  _timeout --;
			if(_timeout <=0)
			{
				_kickout ++;
			}
		}
	}

	if(_kickout && _timeout <=0)
	{
		if(_kickout > 5)
		{
			//如果多次都不能离开副本，则断线之
			_is_deleted = true;
			pImp->LostConnection(gplayer_imp::PLAYER_OFF_LPG_DISCONNECT);
		}
		else
		{
			pImp->LeaveAbnormalState();
			_timeout = 3;
			//使用这里的存盘点来处理
			A3DVECTOR pos;
			int tag;
			world_manager::GetInstance()->GetLogoutPos(pImp,tag,pos);
			if(tag > 0)
			{
				pImp->LongJump(pos,tag);
			}
		}
	}
}

void 
aeff_filter::Heartbeat(int tick)
{
	gplayer_imp * pImp = (gplayer_imp*)_parent.GetImpl();

	if(_state == NORMAL)
	{
		if(_timeout > 0) _timeout --;
		if(_timeout <= 0)
		{
			int cur_mafia = _parent.GetMafiaID();
			world * pPlane = pImp->_plane;
			faction_world_ctrl * pCtrl = (faction_world_ctrl *)pPlane->w_ctrl;

			if(pCtrl->iskick 
				|| cur_mafia != pCtrl->factionid 
				&& (!pCtrl->inbattle || cur_mafia != pCtrl->offense_faction)
				&& !(world_manager::GetWorldLimit().gmfree && _parent.CheckGMPrivilege()) )
			{
				_state = WAIT_ESCAPE;
				_timeout = abase::Rand(55,90);
				pImp->_runner->kickout_instance((pCtrl->iskick ? KIR_FACTION_FORTRESS_CLEAR : KIR_FACTION_MISMATCH), _timeout);
			}
			else
			{
				_timeout = 3;
			}
		}
	}
	else if(_state == WAIT_ESCAPE)
	{
		if(_timeout > 0) _timeout --;
		if(_timeout <= 0)
		{
			_kickout ++;
			if(_kickout > 5)
			{
				//如果多次都不能离开副本，则断线之
				_is_deleted = true;
				pImp->LostConnection(gplayer_imp::PLAYER_OFF_LPG_DISCONNECT);
			}
			else
			{
				pImp->LeaveAbnormalState();
				_timeout = 3;
				//使用这里的存盘点来处理
				A3DVECTOR pos;
				int tag;
				world_manager::GetInstance()->GetLogoutPos(pImp,tag,pos);
				if(tag > 0)
				{
					pImp->LongJump(pos,tag);
				}
			}
		}	
	}
}

void  
aeff_filter::OnModify(int ctrlname,void * ctrlval,size_t ctrllen)
{
	if(ctrlname == FMID_CLEAR_AEFF)
	{
		_state = WAIT_ESCAPE;
		_timeout = 0;
	}
}

void 
aegw_filter::Heartbeat(int tick)
{
	gplayer_imp * pImp = (gplayer_imp*)_parent.GetImpl();

	if(_state == NORMAL)
	{
		if(_timeout > 0) _timeout --;
		if(_timeout <= 0)
		{
			//检查踢出标志全局变量
			if(pImp->_plane->GetCommonValue(COMMON_VALUE_ID_KICKOUT) != 0)
			{
				//该踢出了
				_state = WAIT_ESCAPE;
				_timeout = 60;
				pImp->_runner->kickout_instance(KIR_GLOBALWORLD_KICKOUT, _timeout);
			}
			else
			{
				_timeout = 3;
			}
		}
	}
	else if(_state == WAIT_ESCAPE)
	{
		if(_timeout > 0) _timeout --;	
		if(_timeout <= 0)
		{
			_kickout ++;
			if(_kickout > 5)
			{
				//如果多次都不能离开副本，则断线之
				_is_deleted = true;
				pImp->LostConnection(gplayer_imp::PLAYER_OFF_LPG_DISCONNECT);
			}
			else
			{
				pImp->LeaveAbnormalState();
				_timeout = 3;
				if(world_manager::GetKickoutPoint().tag > 0)
					pImp->LongJump(world_manager::GetKickoutPoint().pos, world_manager::GetKickoutPoint().tag);			
			}
		}
	}
}

void 
aect_filter::Heartbeat(int tick)
{
	gplayer_imp * pImp = (gplayer_imp*)_parent.GetImpl();

	if(_state == NORMAL)
	{
		if(_timeout > 0) _timeout --;
		if(_timeout <= 0)
		{
			//检查踢出标志全局变量
			if(!pImp->GetCountryId()
					&& !(world_manager::GetWorldLimit().gmfree && _parent.CheckGMPrivilege()) )
			{
				//该踢出了
				_state = WAIT_ESCAPE;
				_timeout = 3;
				pImp->_runner->kickout_instance(KIR_NO_COUNTRY, _timeout);
			}
			else
			{
				_timeout = 3;
			}
		}
	}
	else if(_state == WAIT_ESCAPE)
	{
		if(_timeout > 0) _timeout --;	
		if(_timeout <= 0)
		{
			_kickout ++;
			if(_kickout > 5)
			{
				//如果多次都不能离开副本，则断线之
				_is_deleted = true;
				pImp->LostConnection(gplayer_imp::PLAYER_OFF_LPG_DISCONNECT);
			}
			else
			{
				pImp->LeaveAbnormalState();
				_timeout = 3;
				int tag;
				A3DVECTOR pos;
				pImp->GetCountryKickoutPos(tag, pos);
				pImp->LongJump(pos, tag);			
			}
		}
	}
}

void 
aecb_filter::Heartbeat(int tick)
{
	gplayer_imp* pImp = (gplayer_imp*)_parent.GetImpl();
	world * pPlane = pImp->_plane;
	int cur_country = pImp->GetCountryId();

//计算和统计分数
	int attacker_score = pPlane->w_offense_cur_score;
	int defender_score = pPlane->w_defence_cur_score;

	if(attacker_score != _attacker_score || defender_score != _defender_score)
	{
		pImp->_runner->countrybattle_score( attacker_score,pPlane->w_offense_goal, defender_score,pPlane->w_defence_goal);

		_attacker_score = attacker_score;
		_defender_score = defender_score;
	}

	int attacker_count = 0;
	int defender_count = 0;
	ASSERT(pPlane->w_ctrl);
	pPlane->w_ctrl->GetCountryBattleInfo(attacker_count,defender_count);
	if(attacker_count != _attacker_count || defender_count != _defender_count)
	{
		pImp->_runner->countrybattle_info(attacker_count,defender_count);
		_attacker_count = attacker_count;
		_defender_count = defender_count;
	}

//分析结果是否出现
	if(!_battle_result)
	{
		if(pImp->_plane->w_battle_result != _battle_result)
		{
			_battle_result = pImp->_plane->w_battle_result;
			pImp->SyncScoreToPlane();
			pImp->_runner->countrybattle_result(_battle_result);

			//激活准备离开的操作
			_battle_end_timer = 4;
			_timeout = 10;
		}
	}

//准备退出倒数
	if(_battle_end_timer)
	{
		_battle_end_timer --;
		if(_battle_end_timer <= 0)
		{
			_timeout = abase::Rand(3,10);
			pImp->_runner->kickout_instance(KIR_COUNTRYBATTLE_END, _timeout);
		}
	}

	
	
	if(_battle_result)
	{
		if(_timeout > 0)  _timeout --;
		if(_timeout <=0)
		{
			_kickout ++;
		}
	
	}
	else
	{
		if(_origin_country <= 0 
				|| cur_country != _origin_country && !(world_manager::GetWorldLimit().gmfree && _parent.CheckGMPrivilege())
				|| pImp->_parent->IsZombie() && !pImp->CanResurrect(0))
		{
			_origin_country = -1;
			if(_timeout > 0)  _timeout --;
			if(_timeout <=0)
			{
				_kickout ++;
			}
		}
	}

	if(_kickout && _timeout <=0)
	{
		if(_kickout > 3)
		{
			//如果多次都不能离开副本，则断线之
			_is_deleted = true;
			pImp->LostConnection(gplayer_imp::PLAYER_OFF_LPG_DISCONNECT);
		}
		else
		{
			pImp->LeaveAbnormalState();
			_timeout = 3;
			A3DVECTOR pos;
			int tag;
			pImp->GetLastInstanceSourcePos(tag,pos);
			if(tag > 0)
			{
				pImp->LongJump(pos,tag);
			}
		}
	}
}

void  
aecb_filter::OnModify(int ctrlname,void * ctrlval,size_t ctrllen)
{
	if(ctrlname == FMID_CLEAR_AECB)
	{
		_origin_country = -1;
	}
}

void 
aetb_filter::Heartbeat(int tick)
{
	gplayer_imp* pImp = (gplayer_imp*)_parent.GetImpl();
	gplayer * pPlayer = pImp->GetParent();

//分析结果是否出现
	if(!_battle_result)
	{
		if(pImp->_plane->w_battle_result != _battle_result)
		{
			_battle_result = pImp->_plane->w_battle_result;
			pImp->SyncScoreToPlane();

			//激活准备离开的操作
			_timeout = abase::Rand(3,15);
			pImp->_runner->kickout_instance(KIR_TRICKBATTLE_END, _timeout);
		}
	}

	if(_battle_result)
	{
		if(_timeout > 0)  _timeout --;
		if(_timeout <=0)
		{
			_kickout ++;
		}
	}
	else
	{
		if(_err_condition || 
				!pPlayer->IsBattleOffense() && !pPlayer->IsBattleDefence())
		{
			_err_condition = 1;
			if(_timeout > 0)  _timeout --;
			if(_timeout <=0)
			{
				_kickout ++;
			}
		}
	}

	if(_kickout && _timeout <=0)
	{
		if(_kickout > 3)
		{
			//如果多次都不能离开副本，则断线之
			_is_deleted = true;
			pImp->LostConnection(gplayer_imp::PLAYER_OFF_LPG_DISCONNECT);
		}
		else
		{
			pImp->LeaveAbnormalState();
			_timeout = 3;
			A3DVECTOR pos;
			int tag;
			pImp->GetLastInstanceSourcePos(tag,pos);
			if(tag > 0)
			{
				pImp->LongJump(pos,tag);
			}
		}
	}
}

void  
aetb_filter::OnModify(int ctrlname,void * ctrlval,size_t ctrllen)
{
	if(ctrlname == FMID_CLEAR_AETB)
	{
		_err_condition = 1;
	}
}
