#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "world.h"
#include "matter.h"
#include "usermsg.h"
#include "clstab.h"
#include <common/protocol.h>
#include "npcgenerator.h"
#include "pathfinding/pathfinding.h"
#include "playerfatering.h"


DEFINE_SUBSTANCE(gmatter_imp,gobject_imp,CLS_MATTER_IMP)
DEFINE_SUBSTANCE(gmatter_dispatcher,dispatcher,CLS_MATTER_DISPATCHER)
DEFINE_SUBSTANCE(gmatter_controller,controller,CLS_MATTER_CONTROLLER)
DEFINE_SUBSTANCE_ABSTRACT(gmatter_item_base_imp,gmatter_imp,CLS_MATTER_ITEM_BASE_IMP)
DEFINE_SUBSTANCE(gmatter_money_imp,gmatter_item_base_imp,CLS_MATTER_MONEY_IMP)
DEFINE_SUBSTANCE(gmatter_mine_imp,gmatter_imp,CLS_MATTER_MINE_IMP)
DEFINE_SUBSTANCE(gmatter_dyn_imp,gmatter_imp,CLS_MATTER_DYN_IMP)

void gmatter_imp::ActiveCollision(bool active)
{       
	if(active)
	{
		if(!_collision_actived)
		{
			_collision_actived = true;
			if(_parent->collision_id >0) _plane->GetTraceMan()->EnableElement(_parent->collision_id, true, &_plane->w_collision_flags);
		}
	}
	else
	{
		if(_collision_actived)
		{
			_collision_actived = false;
			if(_parent->collision_id >0) _plane->GetTraceMan()->EnableElement(_parent->collision_id, false, &_plane->w_collision_flags);
		}
	}
}

void gmatter_dispatcher::enter_world()
{
	packet_wrapper  h1(64);
	using namespace S2C;
	gmatter * pMatter = (gmatter*)_imp->_parent;
	CMD::Make<CMD::matter_enter_world>::From(h1,pMatter);
	slice * pPiece = pMatter->pPiece;
	AutoBroadcastCSMsg(_imp->_plane,pPiece,h1);

	if(pPiece->IsBorder())
	{
		extern_object_manager::SendAppearMsg<0>(_imp->_plane,pMatter,pPiece);
	}
}

void gmatter_dispatcher::disappear()
{
	packet_wrapper  h1(64);
	using namespace S2C;
	gmatter * pObj = (gmatter*)_imp->_parent;
	CMD::Make<CMD::object_disappear>::From(h1,pObj);
	slice * pPiece = pObj->pPiece;
	AutoBroadcastCSMsg(_imp->_plane,pPiece,h1);

	if(pPiece->IsBorder())
	{
		extern_object_manager::SendDisappearMsg<0>(_imp->_plane,(gmatter*)pObj,pPiece);
	}
}

void gmatter_dispatcher::matter_pickup(int id)
{
	packet_wrapper  h1(64);
	using namespace S2C;
	gmatter * pObj = (gmatter*)_imp->_parent;
	CMD::Make<CMD::matter_pickup>::From(h1,pObj,id);
	slice * pPiece = pObj->pPiece;
	AutoBroadcastCSMsg(_imp->_plane,pPiece,h1);

	if(pPiece->IsBorder())
	{
		extern_object_manager::SendDisappearMsg<0>(_imp->_plane,(gmatter*)pObj,pPiece);
	}
}

void gmatter_dispatcher::broadcast_mine_gatherd(int mid, int pid, int item_type)
{
	packet_wrapper  h1(64);
	using namespace S2C;
	gmatter * pObj = (gmatter*)_imp->_parent;
	CMD::Make<CMD::mine_gatherd>::From(h1,mid,pid,item_type);
	slice * pPiece = pObj->pPiece;
	AutoBroadcastCSMsg(_imp->_plane,pPiece,h1);
}

void gmatter_controller::Release(bool free_parent)
{
	gmatter *pMatter = (gmatter *)_imp->_parent;
	world *pPlane = _imp->_plane;
	if(pMatter->pPiece)
	{
		pPlane->RemoveMatter(pMatter);
	}

	delete _imp->_runner;
	delete _imp;
	delete this;
	pMatter->Clear();
	pPlane->FreeMatter(pMatter);
}

/**
	物品基类的处理
*/
void
gmatter_item_base_imp::Init(world * pPlane,gobject*parent)
{
	gobject_imp::Init(pPlane,parent);
}

gmatter_item_base_imp::~gmatter_item_base_imp()
{
}

int 
gmatter_item_base_imp::MessageHandler(world * pPlane ,const MSG & msg)
{
	//
	switch(msg.message)
	{
		case GM_MSG_HEARTBEAT:
			if( (_life -= MATTER_HEARTBEAT_SEC) <=0)
			{
				//物品消失
				_runner->disappear();
				_commander->Release();
			}
			else
			{
				if(_owner_time >= 0)
				{
					if((_owner_time -=MATTER_HEARTBEAT_SEC) <=0)
					{
						_owner = 0;
						_team_owner = 0;
					}
				}
				slice * pPiece = _parent->pPiece;
				if( pPiece && pPiece->IsBorder())
				{
					//在边界的物品定期发送给其他服务器的超时消息
					extern_object_manager::SendRefreshMsg<0>(_plane,_parent,0,pPiece);
				}
			}
			return 0;
		break;

		default:
		return gobject_imp::MessageHandler(pPlane,msg);
	}
}

 

/**
 *	钱的消息处理,能够正确地处理捡取得消息
 */

void 
gmatter_money_imp::OnPickup(const XID & who,int team_id, bool is_team)
{
	MSG  msg;
	if(team_id > 0)
	{
		BuildMessage(msg,GM_MSG_PICKUP_TEAM_MONEY,XID(GM_TYPE_PLAYER,team_id),_parent->ID,
				_parent->pos,_money,&_drop_user,sizeof(_drop_user));
	}
	else
	{
		BuildMessage(msg,GM_MSG_PICKUP_MONEY,who,_parent->ID,
				_parent->pos,_money,&_drop_user,sizeof(_drop_user));
	}
	_plane->PostLazyMessage(msg);
}

int 
gmatter_money_imp::MessageHandler(world * pPlane ,const MSG & msg)
{
	switch(msg.message)
	{
		case GM_MSG_PICKUP:
			if(msg.content_length == sizeof(msg_pickup_t))
			{
				msg_pickup_t * mpt = (msg_pickup_t*)msg.content;
				Pickup<0>(msg.source,msg.param,mpt->team_seq,msg.pos, mpt->who,true);
			}
			else
			{
				ASSERT(false);
			}
			return 0;
		case GM_MSG_FORCE_PICKUP:
			if(msg.content_length == sizeof(XID))
			{
				Pickup<0>(msg.source,msg.param,0,msg.pos,*(XID*)msg.content,false);
			}
			return 0;
		default:
			return gmatter_item_base_imp::MessageHandler(pPlane,msg);
	}
}


void gmatter_mine_imp::BeMined()
{
	ActiveCollision(false);
	if(_spawner)
	{
		if(_spawner->Reclaim(_plane,(gmatter*)_parent,this))
		{
			//若返回false ，则在Relaim 内部就释放了对象
		}
	}
	else
	{
		_commander->Release();
	}
}
void 
gmatter_mine_imp::SetMonsterParam(void * buf, size_t count)
{
	if(count < 4) return ;
	memcpy(produce_monster,buf,sizeof(produce_monster));
}

int gmatter_mine_imp::MessageHandler(world * pPlane ,const MSG & msg)
{
	switch(msg.message)
	{
		case GM_MSG_HEARTBEAT:
			if (_mine_life > 0 && ((_mine_life -= MATTER_HEARTBEAT_SEC) <= 0))
			{
				//矿物消失
				_runner->disappear();
				BeMined();
				return 0;
			}
			ActiveCollision(true);
			//if(_lock)
			if( _gather_players.size() > 0)
			{
				//有锁定
				if((_lock_time_out -= MATTER_HEARTBEAT_SEC) <= 0)
				{
					//_lock = false;
					//_lock_id = 0;
					_gather_players.clear();
					__PRINTF("采集超时\n");
				}
			}
			return 0;

		
		case GM_MSG_GATHER_REQUEST:
			{
				ASSERT(msg.content_length == sizeof(int)*4);
				int faction = msg.param;
				int level = *(((int*)msg.content) +0);
				int tool = *(((int*)msg.content) +1); 
				int task_id = *(((int*)msg.content) +2);
				int soul_gather_num = *(((int*)msg.content) +3);
				//if(_lock) // && _lock_id != msg.source.id)
				if ((int)_gather_players.size() >= _gather_player_max)
				{
					//已经被锁定,返回错误
					//但如果发来消息的人就是锁定的自己，那么可以进行此操作
					//即使是自己也返回错误
					//现在改成超过采集人数上限才返回锁定错误
					SendErrMessage<0>(msg.source,S2C::ERR_MINE_HAS_BEEN_LOCKED);
					return 0;
				}

				GATHER_PLAYERS_VEC::iterator it = _gather_players.begin(), eit = _gather_players.end();
				for ( ; it!=eit; ++it)
				{
					if ((*it).id == msg.source.id)
					{
						SendErrMessage<0>(msg.source,S2C::ERR_MINE_HAS_BEEN_LOCKED);
						return 0;
					}
				}

				if(tool != _produce_tool)
				{
					//工具不正确,返回错误
					SendErrMessage<0>(msg.source,S2C::ERR_MINE_HAS_INVALID_TOOL);
					return 0;
				}
				
				if(level < _level)
				{
					//级别不够
					SendErrMessage<0>(msg.source,S2C::ERR_LEVEL_NOT_MATCH);
					return 0;
				}

				if(_task_id != task_id)
				{
					//任务不匹配返回错误
					SendErrMessage<0>(msg.source,S2C::ERR_MINE_HAS_INVALID_TOOL);
					return 0;
				}

				if(msg.pos.squared_distance(_parent->pos) >= _gather_distance_max*_gather_distance_max)
				{
					//距离不正确，返回错误
					SendErrMessage<0>(msg.source,S2C::ERR_OUT_OF_RANGE);
					return 0;
				}

				if(_owner && _owner != msg.source.id)
				{
					//不是所有者，返回错误
					SendErrMessage<0>(msg.source,S2C::ERR_MINE_NOT_OWNER);
					return 0;
				}

				if (MINE_TYPE_SOUL == _mine_type && soul_gather_num >= PLAYER_FATE_RING_GAIN_PER_WEEK)
				{
					SendErrMessage<0>(msg.source,S2C::ERR_MINE_SOUL_GATHER_TOO_MUCH);
					return 0;
				}

				if(!world_manager::GetInstance()->CanBeGathered(faction, GetParent()->matter_type)) 
				{
					return 0;
				}

				//_lock = true;
				//是否考虑采集时间为0则不进行锁定操作?
				_lock_time_out = _gather_time_max + 15;
				//_lock_id = msg.source.id;
				_gather_players.push_back(msg.source);
				//发送数据
				{
					gather_reply rpy;
					rpy.can_be_interrupted = _can_be_interrupted;
					rpy.gather_time_min = _gather_time_min;
					rpy.gather_time_max = _gather_time_max;
					if(_eliminate_tool)
					{
						rpy.eliminate_tool = _produce_tool;
					}
					else
					{
						rpy.eliminate_tool = -1;
					}
					MSG message;
					BuildMessage(message,GM_MSG_GATHER_REPLY,msg.source,_parent->ID,_parent->pos,0,&rpy,sizeof(rpy));
					_plane->PostLazyMessage(message);
				}
				if(_broadcast_aggro)
				{
					MSG message;
					msg_aggro_alarm_t  alarm;
					alarm.attacker	= msg.source;
					alarm.rage	= _aggro_count;
					alarm.faction	= msg.param;
					alarm.target_faction = _ask_help_faction;
					BuildMessage(message,GM_MSG_AGGRO_ALARM,XID(GM_TYPE_NPC,-1),msg.source,_parent->pos,0,&alarm,sizeof(alarm));
					_plane->BroadcastMessage(message,_aggro_range, 0xFFFFFFFF);
				}
			}
			return 0;

		case GM_MSG_GATHER_CANCEL:
			//if(_lock && _lock_id == msg.source.id)
			if (!_gather_players.empty())
			{
				GATHER_PLAYERS_VEC::iterator it = _gather_players.begin(), eit = _gather_players.end();
				for ( ; it!=eit; ++it)
				{
					if ((*it).id == msg.source.id)
					{
						_gather_players.erase(it);
						break;
					}
				}
				//_lock = false;
				//_lock_id = 0;
			}
			return 0;

		case GM_MSG_GATHER:
			{
				//if(_lock && _lock_id == msg.source.id)
				GATHER_PLAYERS_VEC::iterator it = _gather_players.begin(), eit = _gather_players.end();
				for ( ; it!=eit; ++it)
				{
					if ((*it).id == msg.source.id)
						break;
				}
				if (it != eit)
				{
					if (abase::RandUniform() >= _gather_success_prob)
					{
						//采集失败，没有采集到东西
						//把自己从正在采集的队列里删掉，可以重新开始采集了
						_gather_players.erase(it);
						return 0;
					}
					gather_result data;
					data.amount = _produce_amount;
					data.task_id = _produce_task_id;
					data.life = _produce_life;
					data.eliminate_tool = 0;
					data.mine_tid = GetParent()->matter_type;
					data.mine_type = _mine_type;
					if(_eliminate_tool)
					{
						data.eliminate_tool = _produce_tool;
					}

					//返回数据
					MSG message;
					BuildMessage(message,GM_MSG_GATHER_RESULT,msg.source,_parent->ID,_parent->pos,
							_produce_id,&data,sizeof(data));
					_plane->PostLazyMessage(message);

					//给点经验
					if(_exp || _sp)
					{
						msg_exp_t expdata = {_level,_exp,_sp};
						message.message = GM_MSG_EXPERIENCE;
						message.param = 0;
						message.content = & expdata;
						message.content_length = sizeof(expdata);
						_plane->PostLazyMessage(message);
					}

					//试着创建怪物
					object_interface::minor_param param;
					memset(&param,0,sizeof(param));
					param.exp_factor = 1.0f;
					param.sp_factor = 1.0f;
					param.drop_rate = 1.f;
					param.money_scale = 1.f;
					param.spec_leader_id = XID(-1,-1);
					param.parent_is_leader = false;
					param.use_parent_faction = false;
					//param.die_with_leader = false;
					param.die_with_who = 0x00;
					param.owner_id = msg.source;
					for(size_t i =0; i < 4; i ++)
					{
						if(!produce_monster[i].mob_id || produce_monster[i].num <=0) continue;
						float radius = produce_monster[i].radius;
						param.mob_id = produce_monster[i].mob_id;
						param.remain_time = produce_monster[i].remain_time;
						for(int j= 0; j < produce_monster[i].num;j ++)
						{
							A3DVECTOR pos = _parent->pos;
							pos.x += abase::Rand(-radius,radius);
							pos.z += abase::Rand(-radius,radius);
							pos.y = 0;
							if(path_finding::GetValidPos(_plane, pos))
							{
								pos.y += _plane->GetHeightAt(pos.x,pos.z);
								if(pos.y < _parent->pos.y - 10.f)
								{
									pos.y = _parent->pos.y;
								}
							}
							else
							{
								pos.y = _parent->pos.y;
							} 
							object_interface::CreateMob(_plane,pos,param);
						}
						
					}

					//如果需要的话把矿物被成功采集的信息广播给周围玩家
					if (_broadcast_on_gain)	_runner->broadcast_mine_gatherd(GetParent()->ID.id,msg.source.id,_produce_id);
					
					//让自己消失
					//现在先不管
					if(!_gather_no_disappear)
					{
						_runner->disappear();
						BeMined();
					}
					else
					{
						//_lock = false;
						_gather_players.erase(it);
					}
				}
				else
				{
					//返回错误
					SendErrMessage<0>(msg.source,S2C::ERR_MINE_HAS_BEEN_LOCKED);
				}
			}
			return 0;

		case GM_MSG_SPAWN_DISAPPEAR:
		{
			ActiveCollision(false);
			_runner->disappear();
			_commander->Release();
		}
		return 0;
		
		default:
			return gobject_imp::MessageHandler(pPlane,msg);
	}
	return 0;
}

void gmatter_mine_imp::Reborn()
{
	//_lock = false;
	//_lock_id = 0;
	_gather_players.clear();
	_runner->enter_world();
}


int gmatter_dyn_imp::MessageHandler(world * pPlane ,const MSG & msg)
{
	switch(msg.message)
	{
		case GM_MSG_HEARTBEAT:
			ActiveCollision(true);
			return 0;

		case GM_MSG_SPAWN_DISAPPEAR:
			{
				ActiveCollision(false);
				_runner->disappear();
				_commander->Release();

			}
			return 0;

		default:
			return gobject_imp::MessageHandler(pPlane,msg);
	}
	return 0;
}               

void gmatter_dyn_imp::Reborn()
{       
	_runner->enter_world();
}       

void DropMoneyItem(world * pPlane, const A3DVECTOR & pos, size_t amount,const XID &owner,int owner_team, int seq,int drop_id)
{
	gmatter * matter = pPlane->AllocMatter();
	if(matter == NULL) return ;
	matter->SetActive();
	matter->pos = pos;
	matter->ID.type = GM_TYPE_MATTER;
	matter->ID.id= MERGE_ID<gmatter>(MKOBJID(world_manager::GetWorldIndex(),pPlane->GetMatterIndex(matter)));
	matter->SetDirUp(0,0,abase::Rand(0,255));
	gmatter_money_imp *imp = new gmatter_money_imp(amount);
	imp->SetOwner(owner,owner_team,seq);
	imp->SetDrop(drop_id);
	imp->Init(pPlane,matter);
	matter->imp = imp;
	imp->_runner = new gmatter_dispatcher();
	imp->_runner->init(imp);
	imp->_commander = new gmatter_controller();
	imp->_commander->Init(imp);
	
	pPlane->InsertMatter(matter);
	imp->_runner->enter_world();
	matter->Unlock();
}
