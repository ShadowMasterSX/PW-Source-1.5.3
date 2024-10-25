#include "world.h"
#include "actobject.h"
#include "actsession.h"
#include "npc.h"
#include "ainpc.h"
#include "aipolicy.h"
#include "clstab.h"
#include <common/protocol.h>
#include "npcgenerator.h"
#include "player_imp.h"
#include "skill_filter.h"
#include "sfilterdef.h"
#include <gsp_if.h>
#include "invincible_filter.h"
#include "skillwrapper.h"
#include "item.h"

DEFINE_SUBSTANCE_ABSTRACT(act_session,substance,CLS_SESSION_BASE)
DEFINE_SUBSTANCE(session_empty,act_session,CLS_SESSION_EMPTY)
DEFINE_SUBSTANCE(session_move,act_session,CLS_SESSION_MOVE)
DEFINE_SUBSTANCE(session_stop_move,act_session,CLS_SESSION_STOP_MOVE)
DEFINE_SUBSTANCE(session_normal_attack,act_session,CLS_SESSION_NORMAL_ATTACK)
//DEFINE_SUBSTANCE(session_npc_zombie,act_session,CLS_SESSION_NPC_ZOMBIE)
DEFINE_SUBSTANCE(session_skill,act_session,CLS_SESSION_SKILL)
DEFINE_SUBSTANCE(session_produce,act_session,CLS_SESSION_PRODUCE)
DEFINE_SUBSTANCE(session_produce2,act_session,CLS_SESSION_PRODUCE2)
DEFINE_SUBSTANCE(session_decompose,act_session,CLS_SESSION_DECOMPOSE)
DEFINE_SUBSTANCE(session_cancel_action,act_session,CLS_SESSION_CANCEL_ACTION)
DEFINE_SUBSTANCE(session_use_item,act_session,CLS_SESSION_USE_ITEM)
DEFINE_SUBSTANCE(session_use_item_with_target,session_use_item,CLS_SESSION_USE_ITEM_WITH_TARGET)
DEFINE_SUBSTANCE(session_sit_down,act_session,CLS_SESSION_PLAYER_SIT_DOWN)
DEFINE_SUBSTANCE(session_gather,act_session,CLS_SESSION_GATHER)
DEFINE_SUBSTANCE(session_use_trashbox,act_session,CLS_SESSION_USE_TRASHBOX)
DEFINE_SUBSTANCE(session_emote_action,act_session, CLS_SESSION_EMOTE)
DEFINE_SUBSTANCE(session_gather_prepare,act_session, CLS_SESSION_GATHER_PREPARE)
DEFINE_SUBSTANCE(session_dead_move, session_move,CLS_SESSION_DEAD_MOVE)
DEFINE_SUBSTANCE(session_dead_stop_move, session_stop_move,CLS_SESSION_DEAD_STOP_MOVE)
DEFINE_SUBSTANCE(session_resurrect, act_session ,CLS_SESSION_RESURRECT)
DEFINE_SUBSTANCE(session_resurrect_by_item, session_resurrect ,CLS_SESSION_RESURRECT_BY_ITEM)
DEFINE_SUBSTANCE(session_resurrect_in_town, session_resurrect ,CLS_SESSION_RESURRECT_IN_TOWN)
DEFINE_SUBSTANCE(session_complete_travel, act_session,CLS_SESSION_COMPLETE_TRAVEL)
DEFINE_SUBSTANCE(session_enter_sanctuary, act_session,CLS_SESSION_ENTER_SANCTUARY)
DEFINE_SUBSTANCE(session_say_hello, act_session,CLS_SESSION_SAY_HELLO)
DEFINE_SUBSTANCE(session_instant_skill, act_session,CLS_SESSION_INSTANT_SKILL)
DEFINE_SUBSTANCE(session_cosmetic, act_session,CLS_SESSION_COSMETIC)
DEFINE_SUBSTANCE(session_region_transport, act_session,CLS_SESSION_REGION_TRANSPORT)
DEFINE_SUBSTANCE(session_resurrect_protect, act_session , CLS_SESSION_RESURRECT_PROTECT)
DEFINE_SUBSTANCE(session_pos_skill, act_session , CLS_SESSION_POS_SKILL)
DEFINE_SUBSTANCE(session_summon_pet, act_session , CLS_SESSION_SUMMON_PET)
DEFINE_SUBSTANCE(session_recall_pet, act_session , CLS_SESSION_RECALL_PET)
DEFINE_SUBSTANCE(session_free_pet, act_session , CLS_SESSION_FREE_PET)
DEFINE_SUBSTANCE(session_restore_pet, act_session , CLS_SESSION_RESTORE_PET)
DEFINE_SUBSTANCE(session_rune_skill, session_skill, CLS_SESSION_RUNE_SKILL)
DEFINE_SUBSTANCE(session_rune_instant_skill, session_instant_skill, CLS_SESSION_RUNE_INSTANT_SKILL)
DEFINE_SUBSTANCE(session_produce3,act_session,CLS_SESSION_PRODUCE3)
DEFINE_SUBSTANCE(session_use_user_trashbox,act_session,CLS_SESSION_USE_USER_TRASHBOX)
DEFINE_SUBSTANCE(session_knockback,act_session,CLS_SESSION_KNOCKBACK)
DEFINE_SUBSTANCE(session_test,act_session,CLS_SESSION_TEST)
DEFINE_SUBSTANCE(session_congregate,act_session,CLS_SESSION_CONGREGATE)
DEFINE_SUBSTANCE(session_engrave,act_session,CLS_SESSION_ENGRAVE)
DEFINE_SUBSTANCE(session_addonregen,act_session,CLS_SESSION_ADDONREGEN)
DEFINE_SUBSTANCE(session_pullover,act_session,CLS_SESSION_PULLOVER)
DEFINE_SUBSTANCE(session_teleport,act_session,CLS_SESSION_TELEPORT)
DEFINE_SUBSTANCE(session_teleport2,act_session,CLS_SESSION_TELEPORT2)
DEFINE_SUBSTANCE(session_produce4,act_session,CLS_SESSION_PRODUCE4)
DEFINE_SUBSTANCE(session_enter_pk_protected, act_session,CLS_SESSION_ENTER_PK_PROTECTED)
DEFINE_SUBSTANCE(session_rebuild_pet_inheritratio, act_session,CLS_SESSION_REBUILD_PET_INHERITRATIO)
DEFINE_SUBSTANCE(session_rebuild_pet_nature, act_session,CLS_SESSION_REBUILD_PET_NATURE)
DEFINE_SUBSTANCE(session_knockup, act_session,CLS_SESSION_KNOCKUP)
DEFINE_SUBSTANCE(session_produce5,act_session,CLS_SESSION_PRODUCE5)

act_session::act_session(gactive_imp * imp):_imp(imp),_session_id(-2),_plane(0)
{
	if(_imp) _plane = _imp->_plane;
} 

void 
act_session::Restore(gactive_imp * imp,int session_id)
{
	_imp = imp;
	_plane = imp->_plane;
	_session_id = session_id;
}

bool 
act_timer_session::Save(archive & ar) 
{
	ar << _self_id;
	if(_timer_index != -1)
	{
		int interval;
		int rtimes;
		int next_interval = GetTaskData(interval,rtimes);
		ar << 0 << interval << next_interval << rtimes;
	}
	else
	{
		ar << -1;
	}
	return true;
}

bool 
act_timer_session::Load(archive & ar) 
{
	ar >> _self_id;
	int rst;
	ar >> rst;
	if(!rst)
	{
		//timer存在
		int interval;
		int rtimes;
		int next_interval;
		ar >> interval >> next_interval >> rtimes;
		if(next_interval < 0) { next_interval = 0;}
		SetTimer(g_timer,interval,rtimes,next_interval);
		return true;
	}
	return false;
}

void 
act_session::SendMsg(int message, const XID & target,const XID & source)
{
	MSG msg;
	BuildMessage(msg,message,target,source,A3DVECTOR(0.f,0.f,0.f),_session_id);
	//这里访问_im-->_plane不知道是否安全
	//考虑直接保存一个world 结构？
	//$$$$$
	_imp->_plane->PostLazyMessage(msg);
}

void 
act_session::SendRepeatMsg(const XID & self)
{
	MSG msg;
	BuildMessage(msg,GM_MSG_OBJ_SESSION_REPEAT,self,self,A3DVECTOR(0.f,0.f,0.f),_session_id);
	_plane->PostLazyMessage(msg);
}

void 
act_session::SendForceRepeat(const XID & self)
{
	MSG msg;
	BuildMessage(msg,GM_MSG_OBJ_SESSION_REPEAT_FORCE,self,self,A3DVECTOR(0.f,0.f,0.f),_session_id);
	_plane->PostLazyMessage(msg);
}
	
void 
act_session::SendEndMsg(const XID & self)
{
	MSG msg;
	BuildMessage(msg,GM_MSG_OBJ_SESSION_END,self,self,A3DVECTOR(0.f,0.f,0.f),_session_id);
	_plane->PostLazyMessage(msg);
}

void 
act_session::NPCSessionStart(int task_id)
{
	_imp->_commander->NPCSessionStart(task_id,_session_id);
}

void 
act_session::NPCSessionEnd(int task_id, int retcode)
{
	_imp->_commander->NPCSessionEnd(task_id,_session_id,retcode);
}

void 
act_session::NPCSessionUpdateChaseInfo(int task_id, const chase_info & info)
{
	_imp->_commander->NPCSessionUpdateChaseInfo(task_id,&info, sizeof(info));
}

inline bool CheckPlayerMove(gactive_imp * obj, const A3DVECTOR & target,const A3DVECTOR & offset, int mode, int use_time,int seq)
{
	gplayer_imp * pImp =  (gplayer_imp *)obj;
	ASSERT(pImp->GetRunTimeClass()->IsDerivedFrom(CLASSINFO(gplayer_imp)));
	const A3DVECTOR & pos = pImp->_parent->pos;
	int rst = -1;
	//检查一下高度信息
	float terrain_height = pImp->_plane->GetHeightAt(target.x,target.z);
	if(terrain_height <= target.y+0.3f)	 //允许有1.5 米的误差
	{
		rst = pImp->CheckPlayerMove(offset,mode,use_time);
	}
	else
	{
		GLog::log(GLOG_INFO,"用户%d的移动坐标低于地面(%f,%f,%f)" ,pImp->_parent->ID.id, target.x,target.y,target.z);
	}

	if(rst >= 0)
	{
		//CheckPlayerMove 基本通过 ，进行碰撞检测的调用
		int pcrst = pImp->PhaseControl(target,terrain_height, mode, use_time);
		if(pcrst >= 0)
		{
			if(pcrst > rst) rst = pcrst;
			if(rst == 0) 
			{
				if(pImp->DecMoveCheckerError(1) < 16)
				{
					return true;
				}
			}
			else if(rst > 0)
			{
				GLog::log(GLOG_INFO,"用户%d的可疑移动数据(%f,%f,%f) pos:(%f,%f,%f) 时间%d,模式%d"
						,pImp->_parent->ID.id, offset.x,offset.y,offset.z,pos.x,pos.y,pos.z
						,use_time,mode);
				if(pImp->IncMoveCheckerError(rst) < 16)
				{
					return true;
				}
			}
		}
		else
		{
			rst = -3;	
		}
	}

	//出现错误，记录特殊日志，
	GLog::log(GLOG_INFO,"用户%d被强行同步位置(%f,%f,%f)",pImp->_parent->ID.id,pos.x,pos.y,pos.z);

	//清除现在的移动错误计数
	pImp->ClrMoveCheckerError();
	
	//修正新的命令序号
	seq = (seq + 100) & 0xFFFF;
	pImp->_commander->SetNextMoveSeq(seq);

	//考虑重新发送玩家的速度数据
	pImp->_runner->get_extprop_move();
	//将玩家拉回原地
	pImp->_runner->trace_cur_pos(seq);
	//清空后面的所有session
	pImp->ClearNextSession();
	return false;
}

bool 
session_move::StartSession(bool hasmorecmd)
{
	if(!CheckCmdSeq<0>()) return false;

	ASSERT(_imp->_session_state == gactive_imp::STATE_SESSION_IDLE);
	_imp->_session_state = gactive_imp::STATE_SESSION_MOVE;
	_session_id = _imp->GetNextSessionID();
	_self_id = _imp->_parent->ID;

	
	A3DVECTOR predict = _target;
	_target -= _imp->_parent->pos;

	if(_use_time < 100 ) _use_time = 100;
	if(!CheckPlayerMove(_imp,predict,_target,_move_mode,_use_time,_seq))
	{	
		//超速了
		//拉回来
		return false;
	}

	_imp->UpdateMoveMode(_move_mode);

	//播放自己移动的消息，可能需要有一定的简化策略
	//比如对距离远的位置减少播放的频率等等
	
	_imp->_runner->move(predict,_use_time,_speed,_move_mode);
	_imp->StepMove(_target);
	__PRINTF("MMMM MMMMM MMM:%f %f %f ---- %f %f %f\n",_target.x,_target.y,_target.z,
	_imp->_parent->pos.x,_imp->_parent->pos.y,_imp->_parent->pos.z);

	//测试延迟速度
	int tick = (int)((_use_time * (1.0f/50.f) + 0.5f))  - 1;
	if(hasmorecmd) tick --;			//如果后面有命令则加快一点
	if(tick <= 8) tick = 8;
	SetTimer(g_timer,tick,1);
	return true;
}

bool 
session_move::RepeatSession()
{
	ASSERT(false && "移动session不能重复");
	return false;
}


bool 
session_stop_move::StartSession(bool hasmorecmd)
{
	if(!CheckCmdSeq<0>()) return false;

	ASSERT(_imp->_session_state == gactive_imp::STATE_SESSION_IDLE);
	_imp->_session_state = gactive_imp::STATE_SESSION_MOVE;
	_session_id = _imp->GetNextSessionID();
	_self_id = _imp->_parent->ID;


	A3DVECTOR pos = _target;
	_target -= _imp->_parent->pos;
	//每次stopmove 都要记录以下
	if(_use_time < 100 ) _use_time = 100;
	if(!CheckPlayerMove(_imp,pos,_target,_move_mode,_use_time,_seq))
	{	
		//超速了
		//拉回来
		return false;
	}

	_imp->UpdateStopMove(_move_mode);

	//发送停止移动的消息
	_imp->_runner->stop_move(pos,_speed,_dir,_move_mode);
	
	//进行真正的移动
	_imp->StepMove(_target);
	__PRINTF("MMMM SSSSS MMM:%f %f %f ---- %f %f %f\n",_target.x,_target.y,_target.z,
			_imp->_parent->pos.x,_imp->_parent->pos.y,_imp->_parent->pos.z);

	//进行移动的判定，如果前一个stopmove和现在的stop move 时间相差太近，则应该加入延时，
	//或者考虑如果后面跟随了攻击命令，则延时很小，否则进行正常延时
	act_session * pSession = _imp->GetNextSession();
	int t = (_use_time + 25)/50;
	if(t < 8) t = 8;
	if(!pSession || !(pSession->GetMask() & SS_MASK_ATTACK))
	{
		SetTimer(g_timer,t,1);
		return true;
	}
	else
	{
		if(t > 10)
		{
			//时间太长则还是要延迟 只是将延迟缩短一些
			t = 8;
			SetTimer(g_timer,t,1);
			return true;
		}
	}
	
	//SetTimer(g_timer,1,1);
	return false;
}

bool 
session_normal_attack::StartSession(bool hasmorecmd)
{
	ASSERT(_target.id != -1);
	ASSERT(_imp->_session_state == gactive_imp::STATE_SESSION_IDLE);
	if(!_imp->CheckAttack(_target))
	{
		return false;
	}
	
	if(_imp->CheckLevitate()) return false;	
	
	_imp->Notify_StartAttack(_target, _force_attack);

	_imp->_session_state = gactive_imp::STATE_SESSION_ATTACK;
	_session_id = _imp->GetNextSessionID();
	_self_id = _imp->_parent->ID;
	__PRINTF("user attack\n");
	_imp->_runner->start_attack(_target);
	_imp->DoAttack(_target,_force_attack);
	int interval = _imp->_cur_prop.attack_speed;
	if(interval <=0) {
		ASSERT(false);
		interval = 34;
	}
	timeval tv;
	gettimeofday(&tv,NULL);
	__PRINTF("player start attack %ld.%06ld\n",tv.tv_sec,tv.tv_usec);
	_attack_speed = interval;
	SetTimer(g_timer,interval,0);
	return true;
}

bool
session_normal_attack::EndSession()
{
	if(_session_id >= 0)
	{
		_imp->_runner->stop_attack(_stop_flag);
		_session_id = -4;
		RemoveTimer();
	}
	return true;
}

bool 
session_normal_attack::RepeatSession()
{
	float dis;
	A3DVECTOR pos;
	int flag;
	if(!_imp->CheckAttack(_target,&flag, &dis,pos))
	{	
		//现在不给客户端回馈错误原因了
		_stop_flag = flag;
		return false;
	}
	int interval = _imp->_cur_prop.attack_speed;
	if(interval <=0) {
		ASSERT(false);
		interval = 34;
	}
	if(interval != _attack_speed)
	{
		_attack_speed = interval;
		ChangeInterval(_attack_speed);
	}
	_imp->DoAttack(_target,_force_attack);
	return true;
}

/*
bool 
session_npc_zombie::StartSession(bool hasmorecmd)
{
	ASSERT(_imp->_session_state == gactive_imp::STATE_SESSION_IDLE);
	_imp->_session_state = gactive_imp::STATE_SESSION_ZOMBIE;
	_session_id = _imp->GetNextSessionID();
	_self_id = _imp->_parent->ID;
	SetTimer(g_timer,_delay_time,1);
	return true;
}

bool 
session_npc_zombie::RepeatSession()
{
	ASSERT(false);
	return false;
}

void 
session_npc_zombie::OnTimer(int index,int rtimes)
{
	SendMsg(GM_MSG_OBJ_ZOMBIE_END,_self_id,_self_id);
}
*/

void 
session_skill::SetTarget(int skill_id, char force_attack,int target_num,int * targets)
{
	_data.id = skill_id;
	_data.forceattack = force_attack;
	_force_attack = force_attack;
	if(target_num > 0)
	{
		XID id;
		_target_list.reserve(target_num);
		for(int i = 0; i < target_num; i ++,targets ++)
		{
			MAKE_ID(id,*targets);
			_target_list.push_back(id);
		}
	}
}


bool 
session_skill::StartSession(bool hasmorecmd)
{
	ASSERT(_imp->_session_state == gactive_imp::STATE_SESSION_IDLE);

	_imp->_session_state = gactive_imp::STATE_SESSION_USE_SKILL;
	_session_id = _imp->GetNextSessionID();
	_self_id = _imp->_parent->ID;
	__PRINTF("use skill................. %d\n", _data.id);

	//悬空状态不能释放技能
	if(_imp->CheckLevitate()) return false;	
	
	int first_interval = _imp->StartSkill(_data,_target_list.begin(),_target_list.size(),_next_interval);
	if(first_interval < 0)
	{
		_imp->_runner->error_message(S2C::ERR_SKILL_NOT_AVAILABLE);
		return false;
	}

	if(_target_list.size())
	{
		//检查是否负面法术
		int type = GNET::SkillWrapper::GetType(_data.id);
		if(type == 1 || type == 3)
		{
			_imp->Notify_StartAttack(_target_list[0], _force_attack);
		}
	}
	
	if(first_interval < 50)
	{
	/*
		__PRINTF("玩家瞬发技能\n");
		//瞬发技能
		int next_interval;
		_imp->RunSkill(_data,_target_list.begin(),_target_list.size(),next_interval);
		return false;
		*/
		first_interval = 50;
	}

	//将时间转换成tick
	__PRINTF("skill session start: first_interval %d, next_interval %d\n",first_interval, _next_interval);
	first_interval /= 50;
	_next_interval /= 50;
	ASSERT(first_interval > 0);

	if(_data.skippable) 
	{
		_skill_skip_time = g_timer.get_tick();
	}

	
	SetTimer(g_timer,20,0,first_interval);

	//注册一个filter
	_imp->_filters.AddFilter(new skill_interrupt_filter(_imp,_session_id,FILTER_INDEX_SKILL_SESSION));
	return true;
}

bool 
session_skill::RestartSession()
{
	if(!_data.skippable) return true;
	ASSERT(_session_id >= 0);
	//重新分配session id
	_session_id = _imp->GetNextSessionID();

	//蓄力技能重新开始
	int tick = g_timer.get_tick() - _skill_skip_time;
	if(tick <= 0) tick = 0;
	
	//停止当前定时器
	RemoveTimer();

	int next_interval;
	int interval = _imp->ContinueSkill(_data,_target_list.begin(),_target_list.size(),next_interval,tick*50);
	//将时间转换成tick
	interval /= 50;
	if(interval <= 0) return false;
	_next_interval = next_interval / 50;

	if(_data.skippable) 
	{
		_skill_skip_time = g_timer.get_tick();
	}
	
	SetTimer(g_timer,20,0,interval);
	return true;
}

bool
session_skill::EndSession()
{
	if(_session_id >= 0)
	{
		//并非强制终止技能，试图终止之
		__PRINTF("skill session end \n");
		_session_id = -5;
		RemoveTimer();
		_imp->_filters.RemoveFilter(FILTER_INDEX_SKILL_SESSION);
		_imp->_runner->stop_skill();
		timeval tv;
		gettimeofday(&tv,NULL);
		__PRINTF("player %6d stop skill at %ld.%06ld\n",_imp->_parent->ID.id,tv.tv_sec,tv.tv_usec);
	}
	return true;
}

bool 
session_skill::RepeatSession()
{
	if(_end_flag) return false;	//结束

	int new_interval = -1;
	int skill_id = _data.id;		//test
	int rst = _imp->RunSkill(_data,_target_list.begin(),_target_list.size(),new_interval);
	if(_imp->_cur_session == NULL)	//test
			GLog::log(GLOG_ERR,"FATALERROR session_skill skill_id=%d",skill_id);//test
	if(rst <= 0 || _next_interval <= 0) return false;

	__PRINTF("skill repeat , next interval %d\n",new_interval);
	if(_data.skippable) 
	{
		_skill_skip_time = g_timer.get_tick();
	}
	if(new_interval <= 0)
	{
		_next_interval  = new_interval;
		return true;
	}
	_next_interval  = new_interval / 50;
	return true;
}

bool 
session_skill::TerminateSession(bool force)
{
	if(force) 
	{
		return EndSession();
	}

	if(_session_id >= 0)
	{
		if(_imp->CancelSkill(_data))
		{
			return EndSession();
		}
		return false;
	}
	return true;
}

void 
session_skill::OnTimer(int index,int rtimes)
{
	int interval = _next_interval;
	__PRINTF("%d skill change timer %d %d\n",_self_id.id,interval,_end_flag);
	if(interval <= 0 || _end_flag)
	{
		//结束自己 
		SendForceRepeat(_self_id);
		if(_timer_index != -1) //试图结束自己的定时器
		{
			RemoveSelf();
		}
	}
	else
	{
		ChangeIntervalInCallback(interval);
		_next_interval = 20;		//这个操作理论上可能由于时序引发错误$$$$
		SendForceRepeat(_self_id);
	}
}

bool 
session_skill::OnAttacked()
{
	ASSERT(_session_id == _imp->GetCurSessionID());
	if(_imp->SkillOnAttacked(_data))
	{
		_end_flag = true;
		RemoveTimer();
		SendEndMsg(_self_id);
		return true;
	}
	return false;
}

bool 
session_produce::StartSession(bool hasmorecmd)
{
	ASSERT(_imp->_session_state == gactive_imp::STATE_SESSION_IDLE);

	_imp->_session_state = gactive_imp::STATE_SESSION_USE_SKILL;
	_session_id = _imp->GetNextSessionID();
	_self_id = _imp->_parent->ID;

	_imp->_runner->produce_start(_rt->recipe_id,_rt->use_time,_count);
	SetTimer(g_timer,_rt->use_time,0);
	return true;
}

bool 
session_produce::RepeatSession()
{
	if(_count <= 0) return false;
	gplayer_imp * pImp = ((gplayer_imp*)_imp);
	if(!pImp->ProduceItem(*_rt))
	{
		return false;
	}
	if(--_count == 0) return false;
	return true;
}

bool 
session_produce::EndSession()
{
	if(_session_id >= 0)
	{
		_session_id = -6;
		RemoveTimer();
		_imp->_runner->produce_end();
		//发送停止制造的指令
	}
	return true;
}


bool 
session_produce::TerminateSession(bool force)
{
	//随时都可以中断
	return EndSession();
}

bool 
session_produce::Save(archive & ar) 
{
	act_timer_session::Save(ar);
	ar << _rt->recipe_id << _count;
	return true;
}

bool 
session_produce::Load(archive & ar) 
{
	act_timer_session::Load(ar);
	int id;
	ar >> id >> _count;
	_rt = recipe_manager::GetRecipe(id);
	ASSERT(_rt);
	return true;
}


bool 
session_produce2::StartSession(bool hasmorecmd)
{
	ASSERT(_imp->_session_state == gactive_imp::STATE_SESSION_IDLE);

	_imp->_session_state = gactive_imp::STATE_SESSION_USE_SKILL;
	_session_id = _imp->GetNextSessionID();
	_self_id = _imp->_parent->ID;

	_imp->_runner->produce_start(_rt->recipe_id,_rt->use_time,1);
	SetTimer(g_timer,_rt->use_time,0);
	return true;
}

bool 
session_produce2::RepeatSession()
{
	gplayer_imp * pImp = ((gplayer_imp*)_imp);
	pImp->ProduceItem2(*_rt, _materials, _idxs);
	return false;
}

bool 
session_produce2::EndSession()
{
	if(_session_id >=0) 
	{
		_session_id = -7;
		RemoveTimer();
		_imp->_runner->produce_end();
		//发送停止制造的指令
	}
	return true;
}


bool 
session_produce2::TerminateSession(bool force)
{
	//随时都可以中断
	return EndSession();
}

bool 
session_produce2::Save(archive & ar) 
{
	act_timer_session::Save(ar);
	ar << _rt->recipe_id ;
	ar.push_back(_materials, sizeof(_materials));
	ar.push_back(_idxs, sizeof(_idxs));
	return true;
}

bool 
session_produce2::Load(archive & ar) 
{
	act_timer_session::Load(ar);
	int recipe_id;
	ar >> recipe_id ;
	_rt = recipe_manager::GetRecipe(recipe_id);
	ASSERT(_rt);

	ar.pop_back(_materials, sizeof(_materials));
	ar.pop_back(_idxs, sizeof(_idxs));
	return true;
}


bool 
session_decompose::StartSession(bool hasmorecmd)
{
	ASSERT(_imp->_session_state == gactive_imp::STATE_SESSION_IDLE);

	_imp->_session_state = gactive_imp::STATE_SESSION_USE_SKILL;
	_session_id = _imp->GetNextSessionID();
	_self_id = _imp->_parent->ID;

	_imp->_runner->decompose_start(_rt->decompose_time,_rt->id);
	SetTimer(g_timer,_rt->decompose_time,1);
	return true;
}

bool 
session_decompose::RepeatSession()
{
	//对物品进行拆解
	gplayer_imp * pImp = ((gplayer_imp*)_imp);
	if(pImp->GetMoney() < _rt->decompose_fee)
	{
		//报告一个中断消息
		return false;
	}
	item_list & inv= pImp->GetInventory();
	const item & it = inv[_index];
	if(it.type != _rt->id || (it.proc_type & (item::ITEM_PROC_TYPE_BIND)))
	{
		//拆解失败
		pImp->_runner->error_message(S2C::ERR_DECOMPOSE_FAILED);
		return false;
	}
	int durability;
	int max_durability;
	it.GetDurability(durability,max_durability);
	int element_num = (int)(_rt->element_num * (float)durability / (float)(max_durability));
	if(element_num <= 0) 
	{
		pImp->_runner->error_message(S2C::ERR_DECOMPOSE_FAILED);
		return false;
	}

	//进行拆解的操作
	element_data::item_tag_t tag = {element_data::IMT_NULL,0};
	item_data * data = world_manager::GetDataMan().generate_item_from_player(_rt->element_id,&tag,sizeof(tag));
	if(!data) 
	{
		//无法创建元石
		pImp->_runner->error_message(S2C::ERR_DECOMPOSE_FAILED);
		return false;
	}

	GLog::log(GLOG_INFO,"用户%d分解%d得到%d个%d",pImp->_parent->ID.id,_rt->id,element_num,_rt->element_id);
	//销毁物品
	//减少金钱
	pImp->SpendMoney(_rt->decompose_fee);
	pImp->_runner->spend_money(_rt->decompose_fee);
	pImp->_runner->player_drop_item(gplayer_imp::IL_INVENTORY,_index,it.type,it.count, S2C::DROP_TYPE_DECOMPOSE);
	pImp->UpdateMallConsumptionDestroying(it.type,it.proc_type,it.count);
	inv.Remove(_index);

	//将元石加入包裹
	size_t count = element_num;
	bool inv_isfull = false;
	while(count)
	{
		data->count = count;
		if(data->count > data->pile_limit) data->count = data->pile_limit;
		count -= data->count;
		if(!inv_isfull)
		{
			int ocount = data->count;
			int rst;
			if((rst = inv.Push(*data)) >= 0)
			{
				//发出获得制造物品的消息
				pImp->_runner->obtain_item(_rt->element_id,data->expire_date,ocount-data->count,inv[rst].count,0,rst);
				__PRINTF("物品id:%d  加入数量：%d slot数量%d 位置%d\n",_rt->element_id,ocount-data->count,inv[rst].count,rst);
			}
		}
		if(data->count)
		{
			DropItemFromData(pImp->_plane,pImp->_parent->pos,*data,_self_id,0,0,_self_id.id);
			inv_isfull = true;
		}
	
	}

	//发出拆解成功消息

	FreeItem(data);
	return false;
}

bool 
session_decompose::EndSession()
{
	if(_session_id >= 0)
	{
		_session_id = -8;
		RemoveTimer();
		_imp->_runner->decompose_end();
	}
	return true;
}

bool 
session_decompose::TerminateSession(bool force)
{
	//随时都可以中断
	return EndSession();
}

bool 
session_decompose::Save(archive & ar)
{
	act_timer_session::Save(ar);
	ar << _index << _rt->id;
	return true;
}

bool 
session_decompose::Load(archive & ar)
{
	act_timer_session::Load(ar);
	int id;
	ar >> _index >> id;
	_rt = recipe_manager::GetDecomposeRecipe(id);
	ASSERT(_rt);
	return true;
}

void 
session_decompose::OnTimer(int index,int rtimes)
{
	SendForceRepeat(_self_id);
}




bool 
session_use_item::StartSession(bool hasmorecmd)
{
	ASSERT(_imp->_session_state == gactive_imp::STATE_SESSION_IDLE);

	_imp->_session_state = gactive_imp::STATE_SESSION_USE_SKILL;
	_session_id = _imp->GetNextSessionID();
	_self_id = _imp->_parent->ID;
	if(_usetime == 0)
	{
		RepeatSession();
		//这里本来将session_id 重新赋值，但是这里会出现一个问题，就是在UseItem的时候可能会发生
		//ClearSession的操作， 比如某些特殊技能。这样这个写入操作就会写入错误的内存，如果这块内存正巧
		//被重新分配（目前正好是ai_task_patrol,因为都是56字节的内容）。那么就正巧写错了内容
		//以后应该再检查一下其他时候会不会出现这种情况
		//或许也应该进行session的嵌套检查 $$$$$$$$$$$$$$$$$
		return false;
	}
	_imp->_runner->start_use_item(_type,_usetime * 50);

	SetTimer(g_timer,_usetime,1);
	return true;
}

bool 
session_use_item::RepeatSession()
{
	gplayer_imp * pImp = ((gplayer_imp*)_imp);
	//使用指定的物品
	if(!pImp->UseItem(_where,_index,_type,_count))
	{
		//无法使用，发送终止使用的消息
		if(_usetime) 
			_imp->_runner->cancel_use_item();
		else
		{	
			//这个错误报告改在物品实现里了
			//_imp->_runner->error_message(S2C::ERR_CANNOT_USE_ITEM);
		}
	}
	return false;
}

bool 
session_use_item::EndSession()
{
	if(_session_id >= 0)
	{
		_session_id = -10;
		RemoveTimer();
	}
	return true;
}

bool 
session_use_item::TerminateSession(bool force)
{
	//随时都可以中断
	_imp->_runner->cancel_use_item();
	return EndSession();
}

bool 
session_use_item::Save(archive & ar)
{
	act_timer_session::Save(ar);
	ar << _where << _index << _type << _count << _usetime;
	return true;
}

bool 
session_use_item::Load(archive & ar)
{
	act_timer_session::Load(ar);
	ar >> _where >> _index >> _type >> _count >> _usetime;
	return true;
}

void 
session_use_item::OnTimer(int index,int rtimes)
{
	SendForceRepeat(_self_id);
}

bool 
session_use_item_with_target::StartSession(bool hasmorecmd)
{
	ASSERT(_imp->_session_state == gactive_imp::STATE_SESSION_IDLE);

	_imp->_session_state = gactive_imp::STATE_SESSION_USE_SKILL;
	_session_id = _imp->GetNextSessionID();
	_self_id = _imp->_parent->ID;
	if(_usetime == 0)
	{
		RepeatSession();
		_session_id = -11;
		return false;
	}
	_imp->_runner->start_use_item_with_target(_type,_usetime * 50,_target);

	SetTimer(g_timer,_usetime,1);
	return true;
}
bool 
session_use_item_with_target::Save(archive & ar)
{
	session_use_item::Save(ar);
	ar << _target << _force_attack;
	return true;
}

bool 
session_use_item_with_target::Load(archive & ar)
{
	session_use_item::Load(ar);
	ar >> _target >> _force_attack;
	return true;
}

bool 
session_use_item_with_target::RepeatSession()
{
	gplayer_imp * pImp = ((gplayer_imp*)_imp);
	//使用指定的物品
	if(!pImp->UseItemWithTarget(_where,_index,_type,_target,_force_attack))
	{	
		//无法使用，发送终止使用的消息
		if(_usetime) 
			_imp->_runner->cancel_use_item();
		else
			_imp->_runner->error_message(S2C::ERR_CANNOT_USE_ITEM);
	}
	return false;
}

bool
session_sit_down::StartSession(bool hasmorecmd)
{
	((gplayer_imp*)_imp)->SitDown();
	return false;
}

bool
session_sit_down::EndSession()
{
	return true;
}

bool 
session_sit_down::TerminateSession(bool force) 
{
	return true;
}

bool 
session_sit_down::RepeatSession() 
{
	return false;
}


bool 
session_gather_prepare::StartSession(bool hasmorecmd)
{
	ASSERT(_imp->_session_state == gactive_imp::STATE_SESSION_IDLE);

	gplayer_imp * pImp = (gplayer_imp *)_imp;
	if(_tool_type > 0)
	{
		if(!pImp->IsItemExist(_where,_index,_tool_type,1))
		{
			pImp->_runner->error_message(S2C::ERR_MINE_HAS_INVALID_TOOL);
			return false;
		}
	}
	if(_task_id > 0)
	{
		//检查任务条件是否满足.................
	}
	
	struct 
	{
		int level;
		int tool;
		int task;
		int soul_gather_num;
	}data;
	data.level = pImp->_basic.level;
	data.tool = _tool_type;
	data.task = _task_id;
	data.soul_gather_num = _soul_gather_num;
	
	pImp->SendTo<0>(GM_MSG_GATHER_REQUEST,XID(GM_TYPE_MATTER,_target),pImp->GetFaction(),&data,sizeof(data));
	return false;
}

bool 
session_gather::OnAttacked()
{
	ASSERT(_session_id == _imp->GetCurSessionID());
	if(_gather_flag)
	{
		RemoveTimer();
		SendEndMsg(_self_id);
		_gather_flag = false;
		return true;
	}
	return false;
}

bool session_gather::StartSession(bool hasmorecmd)
{
	ASSERT(_imp->_session_state == gactive_imp::STATE_SESSION_IDLE);

	_imp->_session_state = gactive_imp::STATE_SESSION_GATHERING;
	_session_id = _imp->GetNextSessionID();
	_self_id = _imp->_parent->ID;

	_imp->_runner->gather_start(_mine,_gather_time);
	//开始等待采集完成 
	SetTimer(g_timer,_gather_time*20,1);
	_gather_flag = true;

	if(_lock_inventory)
	{
		_imp->LockInventory(true);
	}

	if(_can_be_interruputed)
	{
		//注册一个filter
		_imp->_filters.AddFilter(new gather_interrupt_filter(_imp,_session_id,FILTER_INDEX_GATHER_SESSION));
	}
	return true;
}

bool session_gather::RepeatSession()
{
	return false;
}

bool session_gather::EndSession()
{
	if(_session_id >= 0) 
	{
		_session_id = -12;
		RemoveTimer();
		if(_gather_flag)
		{
			//这是正常采集完毕 发送采集消息
			SendMsg(GM_MSG_GATHER,XID(GM_TYPE_MATTER,_mine),_imp->_parent->ID);
		}
		else
		{
			//采集中断  发送中断采集消息
			SendMsg(GM_MSG_GATHER_CANCEL,XID(GM_TYPE_MATTER,_mine),_imp->_parent->ID);
		}
		if(_lock_inventory)
		{
			_imp->LockInventory(false);
		}
		//发送采集结束的消息
		_imp->_runner->gather_stop();
		if(_can_be_interruputed)
		{
			_imp->_filters.RemoveFilter(FILTER_INDEX_GATHER_SESSION);
		}
	}
	return true;
}

bool session_gather::TerminateSession(bool force)
{
	_gather_flag = false;
	EndSession();
	return true;
}


bool session_use_trashbox::StartSession(bool hasmorecmd)
{
	ASSERT(_imp->_session_state == gactive_imp::STATE_SESSION_IDLE);

	_imp->_session_state = gactive_imp::STATE_SESSION_TRAHSBOX;
	_session_id = _imp->GetNextSessionID();
	_self_id = _imp->_parent->ID;

	((gplayer_imp*)_imp)->TrashBoxOpen(_view_only);

	//等待仓库使用完毕
	SetTimer(g_timer,47,0);
	return true;
}

bool session_use_trashbox::RepeatSession()
{
	return true;
}

bool session_use_trashbox::EndSession()
{
	if(_session_id >= 0)
	{
		_session_id = -13;
		RemoveTimer();
		((gplayer_imp*)_imp)->TrashBoxClose(_view_only);
	}
	return true;
}

bool session_use_trashbox::TerminateSession(bool force)
{
	return EndSession();
}


bool session_emote_action::StartSession(bool hasmorecmd)
{
	ASSERT(_imp->_session_state == gactive_imp::STATE_SESSION_IDLE);
	ASSERT(_action != 0);

	_imp->_session_state = gactive_imp::STATE_SESSION_EMOTE;
	_session_id = _imp->GetNextSessionID();
	_self_id = _imp->_parent->ID;

	_imp->SetEmoteState(_action);
	_imp->_runner->do_emote(_action);

	//开始等待采集完成 
	SetTimer(g_timer,29,0);
	return true;
}

bool session_emote_action::RepeatSession()
{
	return true;
}

bool session_emote_action::EndSession()
{
	if(_session_id >= 0)
	{
		_session_id = -14;
		RemoveTimer();
		_imp->ClearEmoteState();
		_imp->_runner->do_emote_restore(_action);
	}
	return true;
}

bool session_emote_action::TerminateSession(bool force)
{
	return EndSession();
}

void 
session_dead_move::OnTimer(int index,int rtimes)
{
	SendMsg(GM_MSG_OBJ_ZOMBIE_SESSION_END,_self_id,_self_id);
}

void 
session_dead_stop_move::OnTimer(int index,int rtimes)
{
	SendMsg(GM_MSG_OBJ_ZOMBIE_SESSION_END,_self_id,_self_id);
}

bool 
session_resurrect::StartSession(bool hasmorecmd)
{
	if(!_imp->_parent->IsZombie()) return false;
	ASSERT(_imp->_session_state == gactive_imp::STATE_SESSION_IDLE);

	_imp->_session_state = gactive_imp::STATE_SESSION_EMOTE;
	_session_id = _imp->GetNextSessionID();
	_self_id = _imp->_parent->ID;

	SetTimer(g_timer,_time,1);
	return true;
}

void 
session_resurrect::OnTimer(int index,int rtimes)
{
	SendMsg(GM_MSG_OBJ_ZOMBIE_SESSION_END,_self_id,_self_id);
}

bool 
session_resurrect::EndSession()
{
	RemoveTimer();
	if(!_imp->_parent->IsZombie()) return true;
	gplayer_imp * pImp = (gplayer_imp *)_imp;
	pImp->Resurrect(pImp->_parent->pos,true,_exp_reduce,1,DEFAULT_RESURRECT_HP_FACTOR,DEFAULT_RESURRECT_MP_FACTOR,_param);
	return true;
}

bool 
session_resurrect_by_item::EndSession()
{
	RemoveTimer();
	if(!_imp->_parent->IsZombie()) return true;
	gplayer_controller * pCtrl = (gplayer_controller *)(_imp->_commander);
	pCtrl->ResurrectByItem(_exp_reduce,_param);
	return true;
}

bool 
session_resurrect_in_town::EndSession()
{
	RemoveTimer();
	if(!_imp->_parent->IsZombie()) return true;
	gplayer_controller * pCtrl = (gplayer_controller *)(_imp->_commander);
	pCtrl->ResurrectInTown(_exp_reduce,_param);
	return false;
}

bool
session_complete_travel::StartSession(bool)
{
	_imp->_filters.ModifyFilter(FILTER_INDEX_TRAVEL,FMID_COMPLETE_TRAVEL,NULL,0);
	return false;
}

bool
session_enter_sanctuary::StartSession(bool)
{
    gplayer_imp* pImpl = substance::DynamicCast<gplayer_imp>(_imp);
	if ((pImpl != NULL) && (pImpl->TestSanctuary()))
    {
        pImpl->PlayerStartEnterSanctuarySession();
    }
	return false;
}

bool
session_enter_pk_protected::StartSession(bool)
{
	if(!world_manager::GetWorldParam().pve_mode)
	{
		//只在pvp服务器生效
		((gplayer_imp*)_imp)->TestPKProtected();
	}
	return false;
}

bool
session_say_hello::StartSession(bool)
{
	((gplayer_imp*)_imp)->SayHelloToNPC(_target);
	return false;
}

void 
session_instant_skill::SetTarget(int skill_id, char force_attack,int target_num,int * targets)
{
	_data.id = skill_id;
	_data.forceattack = force_attack;
	if(target_num > 0)
	{
		XID id;
		_target_list.reserve(target_num);
		for(int i = 0; i < target_num; i ++,targets ++)
		{
			MAKE_ID(id,*targets);
			_target_list.push_back(id);
		}
	}
}

bool 
session_instant_skill::StartSession(bool hasmorecmd)
{
	ASSERT(_imp->_session_state == gactive_imp::STATE_SESSION_IDLE);

	__PRINTF("use instant skill................. %d\n", _data.id);
	if(_imp->_filters.IsFilterExist(FILTER_INDEX_MOUNT_FILTER))
	{
		//骑马状态不能使用瞬发技能
		_imp->_runner->error_message(S2C::ERR_SKILL_NOT_AVAILABLE);
		return false;
	}
	
	//悬空状态不能释放技能
	if(_imp->CheckLevitate()) return false;	
	
	int rst = _imp->CastInstantSkill(_data,_target_list.begin(),_target_list.size());
	if(rst < 0)
	{
		_imp->_runner->error_message(S2C::ERR_SKILL_NOT_AVAILABLE);
		return false;
	}
	return false;
}




bool session_cosmetic::StartSession(bool hasmorecmd)
{
	if(_imp->_parent->IsZombie()) return false;
	ASSERT(_imp->_session_state == gactive_imp::STATE_SESSION_IDLE);

	_imp->_session_state = gactive_imp::STATE_SESSION_COSMETIC;
	_session_id = _imp->GetNextSessionID();

	//发一个要求开始整容的协议
	_imp->EnterCosmeticMode(_inv_idx,_id);
	GMSV::SendCosmeticRequest(_imp->_parent->ID.id,_inv_idx,_id);
	return true;
}

bool session_cosmetic::EndSession() 
{ 
	if(_imp->_session_state != gactive_imp::STATE_SESSION_COSMETIC) return true;
	//要减去物品？ 不需要，在link发来完成消息的时候减少就可以了
	if(_id <= -1)
	{
		GMSV::CancelCosmeticRequest(_imp->_parent->ID.id);
	}
	//需要等待片刻才能离开脱离整容状态
	_imp->LazySendTo<0>(GM_MSG_LEAVE_COSMETIC_MODE,_imp->_parent->ID,_inv_idx,17);
	return true; 
}

bool
session_region_transport::StartSession(bool hasmorecmd)
{
	gplayer_imp * pImp = (gplayer_imp*)_imp;
	if(!pImp->RegionTransport(_ridx, _tag))
	{
		pImp->_runner->error_message(S2C::ERR_CANNOT_ENTER_INSTANCE);
	}
	return false;
}

bool 
session_resurrect_protect::StartSession(bool hasmorecmd)
{
	ASSERT(_imp->_session_state == gactive_imp::STATE_SESSION_IDLE);

	_imp->_session_state = gactive_imp::STATE_SESSION_EMOTE;
	_session_id = _imp->GetNextSessionID();
	_self_id = _imp->_parent->ID;

	SetTimer(g_timer,PLAYER_REBORN_PROTECT*20,1);
	_imp->_filters.AddFilter(new invincible_banish_filter(_imp,FILTER_INVINCIBLE_BANISH,PLAYER_REBORN_PROTECT));
	_imp->_runner->resurrect(1);
	return true;

}

bool 
session_resurrect_protect::EndSession()
{
	RemoveTimer();
	//发一个消息给玩家 告诉玩家可以移动啦 
	_imp->_filters.RemoveFilter(FILTER_INVINCIBLE_BANISH);
	_imp->_runner->resurrect(2);
	return true;
}

void 
session_pos_skill::SetTarget(int skill_id, const A3DVECTOR & pos, char force_attack,int target_num,int * targets)
{
	_data.id = skill_id;
	_target_pos = pos;
	_data.forceattack = force_attack;
	_force_attack = force_attack;
	if(target_num > 0)
	{
		XID id;
		_target_list.reserve(target_num);
		for(int i = 0; i < target_num; i ++,targets ++)
		{
			MAKE_ID(id,*targets);
			_target_list.push_back(id);
		}
	}
}

bool 
session_pos_skill::StartSession(bool hasmorecmd)
{
	ASSERT(_imp->_session_state == gactive_imp::STATE_SESSION_IDLE);

	_imp->_session_state = gactive_imp::STATE_SESSION_USE_SKILL;
	_session_id = _imp->GetNextSessionID();
	_self_id = _imp->_parent->ID;
	__PRINTF("use pos skill................. %d\n", _data.id);
	
	//悬空状态不能释放技能
	if(_imp->CheckLevitate()) return false;	
	
	//限制使用场景
	/*if(_data.id == 1095 || _data.id == 1145)
	{
		const int tag[] = {1,119,121,122,230,231,232,233,234,235};
		int world_tag = world_manager::GetWorldTag();
		size_t i;
		for(i=0; i<sizeof(tag)/sizeof(int); i++)
		{
			if(world_tag <= tag[i])	break;
		}
		if(i >= sizeof(tag)/sizeof(int) || world_tag != tag[i]) return false;
	}*/

	int first_interval = _imp->StartSkill(_data,_target_pos,_target_list.begin(),_target_list.size(),_next_interval);
	if(first_interval < 0)
	{
		_imp->_runner->error_message(S2C::ERR_SKILL_NOT_AVAILABLE);
		return false;
	}
	if(first_interval < 50)
	{
		__PRINTF("玩家瞬发技能\n");
		//瞬发技能
		int next_interval;
		first_interval = _imp->RunSkill(_data,_target_pos,_target_list.begin(),_target_list.size(),next_interval);
		if(first_interval > 50)
		{	
			_next_interval = next_interval / 50;
			SetTimer(g_timer,20,0,first_interval/50);
			return true;
		}
		return false;
	}

	//将时间转换成tick
	__PRINTF("skill session start: first_interval %d, next_interval %d\n",first_interval, _next_interval);
	first_interval /= 50;
	_next_interval /= 50;
	ASSERT(first_interval > 0);

	
	SetTimer(g_timer,20,0,first_interval);

	//注册一个filter
	return true;
}

bool
session_pos_skill::EndSession()
{
	if(_session_id >= 0)
	{
		//并非强制终止技能，试图终止之
		_session_id = -15;
		RemoveTimer();
		_imp->_runner->stop_skill();
		__PRINTF("pos skill end\n");
	}
	return true;
}

bool 
session_pos_skill::RepeatSession()
{
	if(_end_flag) return false;	//结束

	int new_interval = -1;
	int rst = _imp->RunSkill(_data,_target_pos,_target_list.begin(),_target_list.size(),new_interval);
	if(rst <= 0 || _next_interval <= 0) return false;

	__PRINTF("skill repeat , next interval %d\n",new_interval);
	if(new_interval <= 0)
	{
		_next_interval  = new_interval;
		return true;
	}
	_next_interval  = new_interval / 50;
	return true;
}

bool 
session_pos_skill::TerminateSession(bool force)
{
	if(force) 
	{
		return EndSession();
	}

	if(_session_id >= 0)
	{
		if(_imp->CancelSkill(_data))
		{
			return EndSession();
		}
		return false;
	}
	return true;
}

void 
session_pos_skill::OnTimer(int index,int rtimes)
{
	__PRINTF("skill change timer %d\n",_next_interval);
	if(_next_interval <= 0 || _end_flag)
	{
		//结束自己 
		SendForceRepeat(_self_id);
		if(_timer_index != -1) //试图结束自己的定时器
		{
			RemoveSelf();
		}
	}
	else
	{
		ChangeIntervalInCallback(_next_interval);
		_next_interval = 20;		//这个操作理论上可能由于时序引发错误$$$$
		SendForceRepeat(_self_id);
	}
}


bool 
session_general::StartSession(bool hasmorecmd)
{
	ASSERT(_imp->_session_state == gactive_imp::STATE_SESSION_IDLE);

	_imp->_session_state = gactive_imp::STATE_SESSION_GENERAL;
	_session_id = _imp->GetNextSessionID();
	_self_id = _imp->_parent->ID;
	if(_delay <=  0) _delay = 100;

	SetTimer(g_timer,_delay,1);
	OnStart();
	return true;
}

bool 
session_general::RepeatSession()
{
	OnRepeat();
	return false;
}

bool 
session_general::EndSession()
{
	if(_session_id >= 0)
	{
		_session_id = -16;
		RemoveTimer();
		OnEnd();
	}
	return true;
}

bool 
session_general::TerminateSession(bool force)
{
	return EndSession();
}

void 
session_general::OnTimer(int index,int rtimes)
{
	SendForceRepeat(_self_id);
}

void 
session_pet_operation::OnStart()
{
	_imp->_runner->start_pet_operation(_index,_pet_id,_delay,_op);
}

void 
session_pet_operation::OnEnd()
{
	_imp->_runner->end_pet_operation();
}

void 
session_summon_pet::OnRepeat()
{
	gplayer_imp * pImp = (gplayer_imp *)_imp;
	pImp->SummonPet(_index);
}

void 
session_recall_pet::OnRepeat()
{
	gplayer_imp * pImp = (gplayer_imp *)_imp;
	pImp->RecallPet();
}

void 
session_restore_pet::OnRepeat()
{
	gplayer_imp * pImp = (gplayer_imp *)_imp;
	
	int rst = pImp->ConvertPetToEgg(_index);
	if(rst)
	{
		pImp->_runner->error_message(rst);
	}
}

void 
session_free_pet::OnRepeat()
{
	gplayer_imp * pImp = (gplayer_imp *)_imp;
	pImp->BanishPet(_index);
}

bool
session_rune_skill::StartSession(bool hasmorecmd)
{
	ASSERT(_imp->_session_state == gactive_imp::STATE_SESSION_IDLE);

	_imp->_session_state = gactive_imp::STATE_SESSION_USE_SKILL;
	_session_id = _imp->GetNextSessionID();
	_self_id = _imp->_parent->ID;
	__PRINTF("use rune skill................. %d\n", _data.id);
	
	//检查包裹栏中的技能物品
	gplayer_imp * pImp = (gplayer_imp *)_imp;
	item_list & inv = pImp->GetInventory();
	item & it = inv[inv_index];
	if(it.type <= 0 || it.body == NULL || it.body->GetItemType() != item_body::ITEM_TYPE_SKILLTRIGGER2) return false;
	unsigned int sk_id, sk_lvl;
	if(!it.GetSkillData(sk_id, sk_lvl) || sk_id != _data.id || sk_lvl != level) return false;

	int first_interval = _imp->StartRuneSkill(_data,_target_list.begin(),_target_list.size(),_next_interval, level);
	if(first_interval < 0)
	{
		_imp->_runner->error_message(S2C::ERR_SKILL_NOT_AVAILABLE);
		return false;
	}
	//扣除技能物品
	if(consume_if_use)
	{
		pImp->UpdateMallConsumptionDestroying(it.type, it.proc_type, 1);
		
		pImp->_runner->player_drop_item(gplayer_imp::IL_INVENTORY, inv_index, it.type, 1, S2C::DROP_TYPE_USE);
		inv.DecAmount(inv_index,1);
	}
	
	if(_target_list.size())
	{
		//检查是否负面法术
		int type = GNET::SkillWrapper::GetType(_data.id);
		if(type == 1 || type == 3)
		{
			_imp->Notify_StartAttack(_target_list[0], _force_attack);
		}
	}
	
	if(first_interval < 50)
	{
	/*
		__PRINTF("玩家瞬发技能\n");
		//瞬发技能
		int next_interval;
		_imp->RunSkill(_data,_target_list.begin(),_target_list.size(),next_interval);
		return false;
		*/
		first_interval = 50;
	}

	//将时间转换成tick
	__PRINTF("skill session start: first_interval %d, next_interval %d\n",first_interval, _next_interval);
	first_interval /= 50;
	_next_interval /= 50;
	ASSERT(first_interval > 0);

	if(_data.skippable) 
	{
		_skill_skip_time = g_timer.get_tick();
	}

	
	SetTimer(g_timer,20,0,first_interval);

	//注册一个filter
	_imp->_filters.AddFilter(new skill_interrupt_filter(_imp,_session_id,FILTER_INDEX_SKILL_SESSION));
	return true;
}
bool 
session_rune_skill::RestartSession()
{
	if(!_data.skippable) return true;
	ASSERT(_session_id >= 0);
	//重新分配session id
	_session_id = _imp->GetNextSessionID();

	//蓄力技能重新开始
	int tick = g_timer.get_tick() - _skill_skip_time;
	if(tick <= 0) tick = 0;
	
	//停止当前定时器
	RemoveTimer();

	int next_interval;
	int interval = _imp->ContinueRuneSkill(_data,_target_list.begin(),_target_list.size(),next_interval,tick*50,level);
	//将时间转换成tick
	interval /= 50;
	if(interval <= 0) return false;
	_next_interval = next_interval / 50;

	if(_data.skippable) 
	{
		_skill_skip_time = g_timer.get_tick();
	}
	
	SetTimer(g_timer,20,0,interval);
	return true;
}
bool 
session_rune_skill::RepeatSession()
{
	if(_end_flag) return false;	//结束

	int new_interval = -1;
	int rst = _imp->RunRuneSkill(_data,_target_list.begin(),_target_list.size(),new_interval,level);
	if(rst <= 0 || _next_interval <= 0) return false;

	__PRINTF("skill repeat , next interval %d\n",new_interval);
	if(_data.skippable) 
	{
		_skill_skip_time = g_timer.get_tick();
	}
	if(new_interval <= 0)
	{
		_next_interval  = new_interval;
		return true;
	}
	_next_interval  = new_interval / 50;
	return true;
}
bool 
session_rune_skill::TerminateSession(bool force)
{
	if(force) 
	{
		return EndSession();
	}

	if(_session_id >= 0)
	{
		if(_imp->CancelRuneSkill(_data,level))
		{
			return EndSession();
		}
		return false;
	}
	return true;	
}
bool 
session_rune_skill::OnAttacked()
{
	ASSERT(_session_id == _imp->GetCurSessionID());
	if(_imp->RuneSkillOnAttacked(_data,level))
	{
		_end_flag = true;
		RemoveTimer();
		SendEndMsg(_self_id);
		return true;
	}
	return false;	
}
bool
session_rune_instant_skill::StartSession(bool hasmorecmd)
{
	ASSERT(_imp->_session_state == gactive_imp::STATE_SESSION_IDLE);

	__PRINTF("use rune instant skill................. %d\n", _data.id);
	if(_imp->_filters.IsFilterExist(FILTER_INDEX_MOUNT_FILTER))
	{
		//骑马状态不能使用瞬发技能
		_imp->_runner->error_message(S2C::ERR_SKILL_NOT_AVAILABLE);
		return false;
	}

	//检查包裹栏中的技能物品
	gplayer_imp * pImp = (gplayer_imp *)_imp;
	item_list & inv = pImp->GetInventory();
	item & it = inv[inv_index];
	if(it.type <= 0 || it.body == NULL || it.body->GetItemType() != item_body::ITEM_TYPE_SKILLTRIGGER2) return false;
	unsigned int sk_id, sk_lvl;
	if(!it.GetSkillData(sk_id, sk_lvl) || sk_id != _data.id || sk_lvl != level) return false;
	
	int rst = _imp->CastRuneInstantSkill(_data,_target_list.begin(),_target_list.size(),level);
	if(rst < 0)
	{
		_imp->_runner->error_message(S2C::ERR_SKILL_NOT_AVAILABLE);
		return false;
	}
	//扣除技能物品
	if(consume_if_use)
	{
		pImp->UpdateMallConsumptionDestroying(it.type, it.proc_type, 1);
		
		pImp->_runner->player_drop_item(gplayer_imp::IL_INVENTORY, inv_index, it.type, 1, S2C::DROP_TYPE_USE);
		inv.DecAmount(inv_index,1);
	}
	
	return false;
}

bool 
session_produce3::StartSession(bool hasmorecmd)
{
	ASSERT(_imp->_session_state == gactive_imp::STATE_SESSION_IDLE);

	_imp->_session_state = gactive_imp::STATE_SESSION_USE_SKILL;
	_session_id = _imp->GetNextSessionID();
	_self_id = _imp->_parent->ID;

	_imp->_runner->produce_start(_rt->recipe_id,_rt->use_time,1);
	SetTimer(g_timer,_rt->use_time,0);
	return true;
}

bool 
session_produce3::RepeatSession()
{
	gplayer_imp * pImp = ((gplayer_imp*)_imp);
	pImp->ProduceItem3(*_rt, _materials, _idxs, _equip_id, _equip_inv_idx, _inherit_type);
	return false;
}

bool 
session_produce3::EndSession()
{
	if(_session_id >=0) 
	{
		_session_id = -17;
		RemoveTimer();
		_imp->_runner->produce_end();
		//发送停止制造的指令
	}
	return true;
}


bool 
session_produce3::TerminateSession(bool force)
{
	//随时都可以中断
	return EndSession();
}

bool 
session_produce3::Save(archive & ar) 
{
	act_timer_session::Save(ar);
	ar << _rt->recipe_id ;
	ar.push_back(_materials, sizeof(_materials));
	ar.push_back(_idxs, sizeof(_idxs));
	ar << _equip_id << _equip_inv_idx << _inherit_type;
	return true;
}

bool 
session_produce3::Load(archive & ar) 
{
	act_timer_session::Load(ar);
	int recipe_id;
	ar >> recipe_id ;
	_rt = recipe_manager::GetRecipe(recipe_id);
	ASSERT(_rt);

	ar.pop_back(_materials, sizeof(_materials));
	ar.pop_back(_idxs, sizeof(_idxs));
	ar >> _equip_id >> _equip_inv_idx >> _inherit_type;
	return true;
}

bool 
session_produce4::StartSession(bool hasmorecmd)
{
	ASSERT(_imp->_session_state == gactive_imp::STATE_SESSION_IDLE);

	_imp->_session_state = gactive_imp::STATE_SESSION_USE_SKILL;
	_session_id = _imp->GetNextSessionID();
	_self_id = _imp->_parent->ID;

	_imp->_runner->produce_start(_rt->recipe_id,_rt->use_time,1);
	SetTimer(g_timer,(PRODUCE4_CHOOSE_ITEM_TIME+1)*TICK_PER_SEC,2,_rt->use_time); //比客户端长一秒，防止延迟
	return true;
}

bool 
session_produce4::RepeatSession()
{
	gplayer_imp * pImp = ((gplayer_imp*)_imp);
	return pImp->ProduceItem4(*_rt, _materials, _idxs, _equip_id, _equip_inv_idx, _inherit_type, &pItem, _crc, _eq_refine_level, _eq_socket_count, _eq_stone_type, _eq_engrave_addon_list, _eq_engrave_addon_count);
}

bool 
session_produce4::EndSession()
{
	if(_session_id >=0) 
	{
		_session_id = -18;
		if (pItem)
		{
			FreeItem((item_data*)pItem);
			pItem = NULL;
		}
		RemoveTimer();
		_imp->_runner->produce_end();
		//发送停止制造的指令
	}
	return true;
}


bool 
session_produce4::TerminateSession(bool force)
{
	//随时都可以中断
	return EndSession();
}

void
session_produce4::ChooseItem(bool remain)
{
	gplayer_imp * pImp = ((gplayer_imp*)_imp);
	if (!pItem)
	{
		return;
	}
	if (!remain)	//如果选择新装备
	{
		pImp->Produce4ChooseExec(*_rt, _equip_id, _equip_inv_idx, _inherit_type, &pItem, _crc, _eq_refine_level, _eq_socket_count, _eq_stone_type, _eq_engrave_addon_list, _eq_engrave_addon_count);
	}
	if (pItem)	// 只要指针不为空，就释放，这里释放主要是为了把指针置空，防止函数重复调用
	{
		FreeItem((item_data*)pItem);
		pItem = NULL;
	}
	RemoveTimer();
	SendEndMsg(_self_id);
}

bool 
session_produce4::Save(archive & ar) 
{
	act_timer_session::Save(ar);
	// 这个已经不用了
	ASSERT(false);
	return true;
}

bool 
session_produce4::Load(archive & ar) 
{
	act_timer_session::Load(ar);
	// 这个已经不用了
	ASSERT(false);
	return true;
}

bool 
session_produce5::StartSession(bool hasmorecmd)
{
	ASSERT(_imp->_session_state == gactive_imp::STATE_SESSION_IDLE);

	_imp->_session_state = gactive_imp::STATE_SESSION_USE_SKILL;
	_session_id = _imp->GetNextSessionID();
	_self_id = _imp->_parent->ID;

	_imp->_runner->produce_start(_rt->recipe_id, _rt->use_time, 1);
	SetTimer(g_timer, _rt->use_time, 0);
	return true;
}

bool 
session_produce5::RepeatSession()
{
	gplayer_imp* pImp = ((gplayer_imp*)_imp);
	pImp->ProduceItem5(*_rt, _materials, _idxs, _equip_id, _equip_inv_idx, _inherit_type);
	return false;
}

bool 
session_produce5::EndSession()
{
	if(_session_id >=0) 
	{
		_session_id = -19;
		RemoveTimer();
		_imp->_runner->produce_end();
		//发送停止制造的指令
	}
	return true;
}


bool 
session_produce5::TerminateSession(bool force)
{
	//随时都可以中断
	return EndSession();
}

bool 
session_produce5::Save(archive & ar) 
{
	act_timer_session::Save(ar);
	ar << _rt->recipe_id ;
	ar.push_back(_materials, sizeof(_materials));
	ar.push_back(_idxs, sizeof(_idxs));
	ar << _equip_id << _equip_inv_idx << _inherit_type;
	return true;
}

bool 
session_produce5::Load(archive & ar) 
{
	act_timer_session::Load(ar);
	int recipe_id;
	ar >> recipe_id ;
	_rt = recipe_manager::GetRecipe(recipe_id);
	ASSERT(_rt);

	ar.pop_back(_materials, sizeof(_materials));
	ar.pop_back(_idxs, sizeof(_idxs));
	ar >> _equip_id >> _equip_inv_idx >> _inherit_type;
	return true;
}

bool session_use_user_trashbox::StartSession(bool hasmorecmd)
{
	ASSERT(_imp->_session_state == gactive_imp::STATE_SESSION_IDLE);

	_imp->_session_state = gactive_imp::STATE_SESSION_TRAHSBOX;
	_session_id = _imp->GetNextSessionID();
	_self_id = _imp->_parent->ID;

	((gplayer_imp*)_imp)->UserTrashBoxOpen();

	//等待仓库使用完毕
	SetTimer(g_timer,47,0);
	return true;
}

bool session_use_user_trashbox::RepeatSession()
{
	return true;
}

bool session_use_user_trashbox::EndSession()
{
	if(_session_id >= 0)
	{
		_session_id = -30;
		RemoveTimer();
		((gplayer_imp*)_imp)->UserTrashBoxClose();
	}
	return true;
}

bool session_use_user_trashbox::TerminateSession(bool force)
{
	return EndSession();
}

bool session_knockback::StartSession(bool hasmorecmd)
{
	ASSERT(_imp->_session_state == gactive_imp::STATE_SESSION_IDLE);
	_imp->_session_state = gactive_imp::STATE_SESSION_MOVE;
	_session_id = _imp->GetNextSessionID();
	_self_id = _imp->_parent->ID;

	world::object_info info;
	if(!_imp->_plane->QueryObject(_attacker,info)) return false;
	if(info.state & world::QUERY_OBJECT_STATE_ZOMBIE) return false;
	A3DVECTOR newpos;
	int ret = ((gplayer_imp *)_imp)->GetKnockBackPos(info.pos, _distance, newpos);
	__PRINTF("GetKnockBackPos ret=%d\n", ret);
	if(ret < 0)
	{
		return false;
	}

	_imp->ClearNextSession();
	int seq = _imp->_commander->GetCurMoveSeq() + 200;
	_imp->_runner->change_move_seq(seq & 0xFFFF);
	_imp->_commander->SetNextMoveSeq(seq);
	_imp->_runner->player_knockback(newpos,_time);
	newpos -= _imp->_parent->pos;
	_imp->StepMove(newpos);
	_imp->PhaseControlInit();

	int tick = _time/50;
	if(tick < 1) tick = 1;
	SetTimer(g_timer,tick,1);	
	return true;
}

bool session_knockback::RepeatSession()
{
	ASSERT(false && "击退session不能重复");
	return false;
}

bool session_knockup::StartSession(bool hasmorecmd)
{
	ASSERT(_imp->_session_state == gactive_imp::STATE_SESSION_IDLE);
	_imp->_session_state = gactive_imp::STATE_SESSION_MOVE;
	_session_id = _imp->GetNextSessionID();
	_self_id = _imp->_parent->ID;

	A3DVECTOR sourcepos = _imp->_parent->pos;
	A3DVECTOR offsetpos(0.f,1.f,0.f);
	sourcepos -= offsetpos;
	A3DVECTOR newpos;
	int ret = ((gplayer_imp *)_imp)->GetKnockBackPos(sourcepos, _distance, newpos);
	__PRINTF("session_knockup GetKnockBackPos ret=%d\n", ret);
	if (ret < 0) return false;
	
	_imp->ClearNextSession();
	int seq = _imp->_commander->GetCurMoveSeq() + 200;
	_imp->_runner->change_move_seq(seq & 0xFFFF);
	_imp->_commander->SetNextMoveSeq(seq);
	_imp->_runner->player_teleport(newpos,_time,1);
	newpos -= _imp->_parent->pos;
	_imp->StepMove(newpos);
	_imp->PhaseControlInit();

	int tick = _time/50;
	if(tick < 1) tick = 1;
	SetTimer(g_timer,tick,1);
	return true;
}

bool session_knockup::RepeatSession()
{
	ASSERT(false && "击飞session不能重复");
	return false;
}

void session_test::OnTimer(int index,int rtimes)
{
	gettimeofday(&_end,NULL);
	__PRINTF("session_test timer end(%d,%d)\n",_end.tv_sec,_end.tv_usec);
	SendForceRepeat(_self_id);
}

bool session_test::StartSession(bool hasmorecmd)
{
	ASSERT(_imp->_session_state == gactive_imp::STATE_SESSION_IDLE);
	_imp->_session_state = gactive_imp::STATE_SESSION_MOVE;
	_session_id = _imp->GetNextSessionID();
	_self_id = _imp->_parent->ID;

	gettimeofday(&_start,NULL);
	__PRINTF("session_test timer start(%d,%d)\n",_start.tv_sec,_start.tv_usec);
	SetTimer(g_timer,TICK_PER_SEC,1);
	return true;
}

bool session_test::RepeatSession()
{
	struct timeval repeat_recv;
	gettimeofday(&repeat_recv,NULL);
	__PRINTF("session_test repeat recv(%d,%d)\n",repeat_recv.tv_sec,repeat_recv.tv_usec);

	int timer_delay = (_end.tv_sec - _start.tv_sec)*1000000 + (_end.tv_usec - _start.tv_usec) - 1000000;
	int msg_delay = (repeat_recv.tv_sec - _end.tv_sec)*1000000 + (repeat_recv.tv_usec - _end.tv_usec);
	char buf[128];
	snprintf(buf, sizeof(buf), "timer_delay %d usec, msg delay %d usec", timer_delay, msg_delay);
	__PRINTF(buf);

	((gplayer_imp*)_imp)->Say(buf);
	return false;
}

bool session_congregate::StartSession(bool hasmorecmd)
{
	ASSERT(_imp->_session_state == gactive_imp::STATE_SESSION_IDLE);
	_imp->_session_state = gactive_imp::STATE_SESSION_MOVE;
	_session_id = _imp->GetNextSessionID();
	_self_id = _imp->_parent->ID;

	_imp->_runner->congregate_start(_type,CONGREGATE_PREPARE_TICK*50);
	SetTimer(g_timer,CONGREGATE_PREPARE_TICK,1);
	return true;
}

bool session_congregate::RepeatSession()
{
	gplayer_imp * pImp = ((gplayer_imp*)_imp);
	if(!pImp->LongJump(_pos,_world_tag))
	{
		_imp->_runner->cancel_congregate(_type);
	}
	else
	{
		_imp->_filters.AddFilter(new invincible_filter(pImp,FILTER_INVINCIBLE,3));
	}
	return false;
}

bool session_congregate::EndSession()
{
	if(_session_id >= 0)
	{
		_session_id = -31;
		RemoveTimer();
	}
	return true;
}

bool session_congregate::TerminateSession(bool force)
{
	//随时都可以中断
	_imp->_runner->cancel_congregate(_type);
	return EndSession();
}

void session_congregate::OnTimer(int index,int rtimes)
{
	SendForceRepeat(_self_id);
}

bool 
session_engrave::StartSession(bool hasmorecmd)
{
	ASSERT(_imp->_session_state == gactive_imp::STATE_SESSION_IDLE);
	_imp->_session_state = gactive_imp::STATE_SESSION_USE_SKILL;
	_session_id = _imp->GetNextSessionID();
	_self_id = _imp->_parent->ID;

	_imp->_runner->engrave_start(_ert->recipe_id,_ert->use_time);
	SetTimer(g_timer,_ert->use_time,1);
	return true;
}

bool 
session_engrave::RepeatSession()
{
	gplayer_imp * pImp = ((gplayer_imp*)_imp);
	if(!pImp->EngraveItem(*_ert,_inv_index))
	{
		return false;
	}
	return false;
}

bool 
session_engrave::EndSession()
{
	if(_session_id >= 0)
	{
		_session_id = -32;
		RemoveTimer();
		_imp->_runner->engrave_end();
		//发送停止制造的指令
	}
	return true;
}

bool 
session_engrave::TerminateSession(bool force)
{
	//随时都可以中断
	return EndSession();
}

bool 
session_engrave::Save(archive & ar) 
{
	act_timer_session::Save(ar);
	ar << _ert->recipe_id << _inv_index;
	return true;
}

bool 
session_engrave::Load(archive & ar) 
{
	act_timer_session::Load(ar);
	int id;
	ar >> id >> _inv_index;
	_ert = recipe_manager::GetEngraveRecipe(id);
	ASSERT(_ert);
	return true;
}

void 
session_engrave::OnTimer(int index,int rtimes)
{
	SendForceRepeat(_self_id);
}

bool 
session_addonregen::StartSession(bool hasmorecmd)
{
	ASSERT(_imp->_session_state == gactive_imp::STATE_SESSION_IDLE);
	_imp->_session_state = gactive_imp::STATE_SESSION_USE_SKILL;
	_session_id = _imp->GetNextSessionID();
	_self_id = _imp->_parent->ID;

	_imp->_runner->addonregen_start(_arrt->recipe_id,_arrt->use_time);
	SetTimer(g_timer,_arrt->use_time,1);
	return true;
}

bool 
session_addonregen::RepeatSession()
{
	gplayer_imp * pImp = ((gplayer_imp*)_imp);
	if(!pImp->ItemAddonRegen(*_arrt,_inv_index))
	{
		return false;
	}
	return false;
}

bool 
session_addonregen::EndSession()
{
	if(_session_id >= 0)
	{
		_session_id = -32;
		RemoveTimer();
		_imp->_runner->addonregen_end();
		//发送停止制造的指令
	}
	return true;
}

bool 
session_addonregen::TerminateSession(bool force)
{
	//随时都可以中断
	return EndSession();
}

bool 
session_addonregen::Save(archive & ar) 
{
	act_timer_session::Save(ar);
	ar << _arrt->recipe_id << _inv_index;
	return true;
}

bool 
session_addonregen::Load(archive & ar) 
{
	act_timer_session::Load(ar);
	int id;
	ar >> id >> _inv_index;
	_arrt = recipe_manager::GetAddonRegenRecipe(id);
	ASSERT(_arrt);
	return true;
}

void 
session_addonregen::OnTimer(int index,int rtimes)
{
	SendForceRepeat(_self_id);
}

bool session_pullover::StartSession(bool hasmorecmd)
{
	ASSERT(_imp->_session_state == gactive_imp::STATE_SESSION_IDLE);
	_imp->_session_state = gactive_imp::STATE_SESSION_MOVE;
	_session_id = _imp->GetNextSessionID();
	_self_id = _imp->_parent->ID;

	world::object_info info;
	if(!_imp->_plane->QueryObject(_attacker,info)) return false;
	if(info.state & world::QUERY_OBJECT_STATE_ZOMBIE) return false;
	
	A3DVECTOR opposite_pos = _imp->_parent->pos;
	opposite_pos *= 2.f;
	opposite_pos -= info.pos;
	float squared_dis = info.pos.squared_distance(_imp->_parent->pos);
	if(_distance*_distance > squared_dis)
		_distance = sqrt(squared_dis);

	A3DVECTOR newpos;
	int ret = ((gplayer_imp *)_imp)->GetKnockBackPos(opposite_pos, _distance, newpos);
	__PRINTF("GetKnockBackPos ret=%d\n", ret);
	if(ret < 0)
	{
		return false;
	}

	_imp->ClearNextSession();
	int seq = _imp->_commander->GetCurMoveSeq() + 200;
	_imp->_runner->change_move_seq(seq & 0xFFFF);
	_imp->_commander->SetNextMoveSeq(seq);
	_imp->_runner->player_teleport(newpos,_time,1);
	newpos -= _imp->_parent->pos;
	_imp->StepMove(newpos);
	_imp->PhaseControlInit();

	int tick = _time/50;
	if(tick < 1) tick = 1;
	SetTimer(g_timer,tick,1);	
	return true;
}

bool session_pullover::RepeatSession()
{
	ASSERT(false && "pullover session不能重复");
	return false;
}

bool session_teleport::StartSession(bool hasmorecmd)
{
	ASSERT(_imp->_session_state == gactive_imp::STATE_SESSION_IDLE);
	_imp->_session_state = gactive_imp::STATE_SESSION_MOVE;
	_session_id = _imp->GetNextSessionID();
	_self_id = _imp->_parent->ID;

	_imp->ClearNextSession();
	int seq = _imp->_commander->GetCurMoveSeq() + 200;
	_imp->_runner->change_move_seq(seq & 0xFFFF);
	_imp->_commander->SetNextMoveSeq(seq);
	_imp->_runner->player_teleport(_pos,_time,_mode);
	_pos -= _imp->_parent->pos;
	_imp->StepMove(_pos);
	_imp->PhaseControlInit();

	int tick = _time/50;
	if(tick < 1) return false;	//不需持续时间
	SetTimer(g_timer,tick,1);	
	return true;
}

bool session_teleport::RepeatSession()
{
	ASSERT(false && "teleport session不能重复");
	return false;
}

bool session_teleport2::StartSession(bool hasmorecmd)
{
	ASSERT(_imp->_session_state == gactive_imp::STATE_SESSION_IDLE);
	_imp->_session_state = gactive_imp::STATE_SESSION_MOVE;
	_session_id = _imp->GetNextSessionID();
	_self_id = _imp->_parent->ID;

	_imp->ClearNextSession();
	int seq = _imp->_commander->GetCurMoveSeq() + 200;
	_imp->_runner->change_move_seq(seq & 0xFFFF);
	_imp->_commander->SetNextMoveSeq(seq);

	float squared_dis = _pos.squared_distance(_imp->_parent->pos);
	A3DVECTOR newpos;
	int ret = ((gplayer_imp *)_imp)->GetKnockBackPos(_pos, sqrt(squared_dis), newpos);
	__PRINTF("session_teleport2 GetKnockBackPos ret=%d\n", ret);
	if(ret < 0)	return false;

	_imp->_runner->player_teleport(newpos,_time,_mode);
	newpos -= _imp->_parent->pos;
	_imp->StepMove(newpos);
	_imp->PhaseControlInit();

	int tick = _time/50;
	if(tick < 1) return false;	//不需持续时间
	SetTimer(g_timer,tick,1);	
	return true;
}

bool session_teleport2::RepeatSession()
{
	ASSERT(false && "teleport session不能重复");
	return false;
}

bool
session_rebuild_pet_inheritratio::StartSession(bool hasmorecmd)
{
	ASSERT(_imp->_session_state == gactive_imp::STATE_SESSION_IDLE);
	_imp->_session_state = gactive_imp::STATE_SESSION_GENERAL;
	_session_id = _imp->GetNextSessionID();
	_self_id = _imp->_parent->ID;

	_imp->_runner->pet_rebuild_inherit_start(_index,PET_REBUILD_TIME);
	SetTimer(g_timer,(PET_CHOOSE_REBUILD_RESULT_TIME+1)*TICK_PER_SEC,2,PET_REBUILD_TIME * TICK_PER_SEC); //比客户端长一秒，防止延迟
	return true;
}

bool
session_rebuild_pet_inheritratio::RepeatSession()
{
	gplayer_imp * pImp = ((gplayer_imp*)_imp);
	return pImp->RebulidPetInheritRatio(_pet_id,_index,_formula_index,_r_attack,_r_defense,_r_hp,_r_atk_lvl,_r_def_lvl);
}

bool
session_rebuild_pet_inheritratio::EndSession()
{
	if(_session_id >= 0)
	{
		_session_id = -18;
		RemoveTimer();
		_imp->_runner->pet_rebuild_inherit_end(_index);
	}
	return true;
}

bool
session_rebuild_pet_inheritratio::TerminateSession(bool force)
{
	return EndSession();
}

void
session_rebuild_pet_inheritratio::AcceptResult(bool isaccept)
{
	gplayer_imp * pImp = ((gplayer_imp*)_imp);
	if(isaccept)
	{
		pImp->AcceptInheritRatioResult(_pet_id,_index,_r_attack,_r_defense,_r_hp,_r_atk_lvl,_r_def_lvl);
	}
	RemoveTimer();
	SendEndMsg(_self_id);
}

void
session_rebuild_pet_inheritratio::OnTimer(int index,int rtimes)
{
	if(rtimes)
		SendForceRepeat(_self_id);
	else
		SendEndMsg(_self_id);
}

bool 
session_rebuild_pet_inheritratio::Save(archive & ar) 
{
	act_timer_session::Save(ar);
	ar << _pet_id << _index << _formula_index << _r_attack << _r_defense << _r_hp << _r_atk_lvl << _r_def_lvl;
	return true;
}

bool 
session_rebuild_pet_inheritratio::Load(archive & ar) 
{
	act_timer_session::Load(ar);
	ar >> _pet_id >> _index >> _formula_index >> _r_attack >> _r_defense >> _r_hp >> _r_atk_lvl >> _r_def_lvl;
	return true;
}

bool
session_rebuild_pet_nature::StartSession(bool hasmorecmd)
{
	ASSERT(_imp->_session_state == gactive_imp::STATE_SESSION_IDLE);
	_imp->_session_state = gactive_imp::STATE_SESSION_GENERAL;
	_session_id = _imp->GetNextSessionID();
	_self_id = _imp->_parent->ID;
	
	_imp->_runner->pet_rebuild_nature_start(_index,PET_REBUILD_TIME);
	SetTimer(g_timer,(PET_CHOOSE_REBUILD_RESULT_TIME+1)*TICK_PER_SEC,2,PET_REBUILD_TIME * TICK_PER_SEC); //比客户端长一秒，防止延迟
	return true;
}

bool
session_rebuild_pet_nature::RepeatSession()
{
	gplayer_imp *pImp = (gplayer_imp*)_imp;
	return pImp->RebuildPetNature(_pet_id,_index,_formula_index,_nature);
}

bool
session_rebuild_pet_nature::EndSession()
{
	if(_session_id >= 0)
	{
		_session_id = -18;
		RemoveTimer();
		_imp->_runner->pet_rebuild_nature_end(_index);
	}
	return true;
}

bool
session_rebuild_pet_nature::TerminateSession(bool force)
{
	return EndSession();
}

void
session_rebuild_pet_nature::AcceptResult(bool isaccept)
{
	gplayer_imp * pImp = ((gplayer_imp*)_imp);
	if(isaccept)
	{
		pImp->AcceptNatureResult(_pet_id,_index,_nature);
	}
	RemoveTimer();
	SendEndMsg(_self_id);
}

void
session_rebuild_pet_nature::OnTimer(int index,int rtimes)
{
	if(rtimes)
		SendForceRepeat(_self_id);
	else
		SendEndMsg(_self_id);
}

bool
session_rebuild_pet_nature::Save(archive & ar)
{
	act_timer_session::Save(ar);
	ar << _pet_id << _index << _nature;
	return true;
}

bool
session_rebuild_pet_nature::Load(archive & ar)
{
	act_timer_session::Load(ar);
	ar >> _pet_id >> _index >> _nature;
	return true;
}
