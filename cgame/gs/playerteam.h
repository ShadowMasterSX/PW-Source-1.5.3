#ifndef __ONLINEGAME_GS_PLAYERTEAM_H__
#define __ONLINEGAME_GS_PLAYERTEAM_H__

#include <common/types.h>
#include <common/base_wrapper.h>
#include <glog.h>
#include "config.h"
#include "staticmap.h"
#include "property.h"

class gplayer_imp;
class world;
class player_team  // :: public abase::ASmallObject
{
public:
	struct invite_t
	{
		long timestamp;
		XID id;
	};
	typedef abase::static_map<int , invite_t, TEAM_MEMBER_CAPACITY - 1> INVITE_MAP;

	struct team_entry{
		int time_out;			//某个状态的超时时间
		int self_seq;			//自身的序号（每组成一次队伍，加一次一,初值考虑用系统时间)
		int pickup_flag;		//开始建立队伍时捡取物品的类型
		long last_invite_time;		//最后一次被邀请的时间戳
		int race_count;			//队伍中职业数目
		int change_leader_timeout;	//更改leader的timeout
		int cur_max_level;		//当前队伍中的最大级别
		int cur_wallow_level;		//当前队伍中的最大沉迷等级
		int min_timer_counter;		//计算最小级别的counter
		INVITE_MAP invite_map;		//邀请别人的名单
		INVITE_MAP apply_map;		//申请人名单
		gplayer_imp * imp;

	};

	struct agree_invite_entry
	{
		int race;
		int cs_index;
		int cs_sid;
		team_mutable_prop data;
	};

	struct member_entry
	{
		XID id;
		team_mutable_prop data;
		int race;			//职业和性别
		int cs_index;			//队员的cs index
		int cs_sid;			//队员的sid
		A3DVECTOR pos;			//队员的位置
		int timeout;			//队员的时间戳
		bool is_changed;		//这一项是否发生变化
	};

	struct team_t
	{
		enum
		{
			TEAM_NO_CHANGE,
			TEAM_PART_CHANGED,
			TEAM_ALL_CHANGED,
		};
		
		XID 		leader;		//队长
		int 		team_seq;	//队伍序号
		size_t		member_count;
		int 		change_flag;
		int 		pickup_flag;		//开始建立队伍时捡取物品的类型
		int64_t 	team_uid; //队伍的唯一id号，不随着队长改变而改变，在创建队伍时生成，在队伍解散时失效，用于gametalk
			
		member_entry 	member_list[TEAM_MEMBER_CAPACITY]; 
	};
	
	class team_control
	{
	protected:
		void ChangeState(player_team * pTeam, int state)
		{
			pTeam->_team_state = state;
		}
		
		team_entry & GetData(player_team * pTeam) { return pTeam->_data;}
		team_t & GetTeam(player_team * pTeam) { return pTeam->_team;}
		
		template <int foo>
		static inline void SendMessage(world *pPlane,const MSG & msg)
		{
			player_team::SendMessage<0>(pPlane,msg);
		}
		
	private:
		friend class player_team;

//心跳		
		virtual void OnHeartbeat(player_team * pTeam) = 0;

//客户端操作
		virtual void CliInviteOther(player_team * pTeam,const XID & member)  = 0;
		virtual void CliAgreeInvite(player_team * pTeam,const XID & leader,int seq)  = 0;
		virtual void CliRejectInvite(player_team * pTeam, const XID & leader) = 0;
		virtual bool CliLeaveParty(player_team * pTeam) = 0;
		virtual bool CliDismissParty(player_team * pTeam) { return true;}
		virtual void CliKickMember(player_team * pTeam,const XID & member) = 0;
		virtual void CliAgreeApply(player_team *, int id, bool result) = 0;
		virtual void CliChangeLeader(player_team *, const XID & new_leader) = 0;

//消息
		virtual void MsgInvite(player_team *pTeam, const XID & leader,int seq,int pickup_flag) = 0;
		virtual void MsgAgreeInvite(player_team *,const XID & member,const A3DVECTOR &pos,const agree_invite_entry&,int seq) = 0;
		virtual void MsgRejectInvite(player_team *,const XID & member) = 0;
		virtual void MsgJoinTeam(player_team *,const XID & leader,const member_entry*, size_t count,int pickup_flag,int64_t team_uid,const void * ins_key , size_t ik_size) = 0;
		virtual void MsgJoinTeamFailed(player_team *pTeam, const XID & leader) = 0;
		virtual void MsgMemberUpdateData(player_team *, const XID & member,const A3DVECTOR &,const team_mutable_prop &) = 0;
		virtual void MsgLeaderUpdateData(player_team *, const XID & leader,const member_entry *,size_t) = 0;
		virtual void MsgMemberLeaveRequest(player_team * pTeam, const XID & member) = 0;
		virtual void MsgNotifyMemberLeave(player_team*,const XID& leader,const XID& member,int type) = 0;
		virtual void MsgNewMember(player_team*,const XID& leader,const member_entry * list, size_t count) =0;
		virtual void MsgLeaderCancelParty(player_team *,const XID & leader,int seq) = 0;
		virtual void MsgApplyParty(player_team*, const XID & who) = 0;
		virtual void MsgApplyPartyReply(player_team*, const XID & leader, int seq) = 0;
		virtual void MsgChangeToLeader(player_team*, const XID & leader) = 0;
		virtual void MsgLeaderChanged(player_team*, const XID & new_leader) = 0;
		virtual bool PickupTeamMoney(player_team * pTeam, const A3DVECTOR & pos, int amount) =0;
		virtual void LostConnection(player_team * pTeam) = 0;
		virtual void Logout(player_team * pTeam) = 0;

	};



	enum
	{
		TEAM_NORMAL,
		TEAM_WAIT,
		TEAM_MEMBER,
		TEAM_LEADER,
		TEAM_STATE_NUM,
	};

	struct auto_team_info_t
	{
		enum 
		{
			GOAL_TYPE_INVALID =0,
			GOAL_TYPE_TASK,
			GOAL_TYPE_ACTIVITY,
		};
		
		enum
		{
			TIMEOUT_COMPOSING = 30,
		};
		
		char composing_timeout; //队员已匹配，正在组队状态的超时
		XID candicate_leader;	
	};
	
protected:
	static team_control * _team_ctrl[TEAM_STATE_NUM];

	int 		_team_state;		//组队的状态
	auto_team_info_t _auto_team_info;  //自动组队
	team_entry 	_data;	
	team_t		_team;
	friend class team_control;
	friend class team_control_normal;
	friend class team_control_wait;
	friend class team_control_member;
	friend class team_control_leader;

	member_entry * FindMember(const XID & id)
	{
		for(size_t i = 0; i < _team.member_count; i ++)
		{
			if(_team.member_list[i].id == id) return _team.member_list + i;
		}
		return NULL;
	}


	template <int foo>
	inline static void SendMessage(world *pPlane,const MSG & msg)
	{
		pPlane->PostLazyMessage(msg);
	}
	

protected:

	void CalcRaceCount();
	void CalcMinLevel();
	void CalcExactMaxLevel();

	template <int foo>
	bool BecomeLeader(const XID & first_member, const agree_invite_entry & prop,const A3DVECTOR &pos)
	{
		if(_team_state != TEAM_NORMAL)
		{
			ASSERT(false);
			return false;
		}
		
		memset(&_auto_team_info, 0, sizeof(_auto_team_info));
		_team.leader = _data.imp->_parent->ID;
		_team.member_count = 1;
		_team.member_list[0].data = team_mutable_prop(_data.imp);
		_team.member_list[0].id = _team.leader;
		_team.member_list[0].is_changed = false;
		_team.member_list[0].timeout  = 0;
		_team.member_list[0].pos = _data.imp->_parent->pos;
		gplayer * pPlayer = (gplayer*)(_data.imp->_parent);
		_team.member_list[0].race = pPlayer->base_info.race;
		_team.member_list[0].cs_index = pPlayer->cs_index;
		_team.member_list[0].cs_sid  = pPlayer->cs_sid;

		_data.change_leader_timeout = 0;
		_data.cur_max_level = _data.imp->_basic.level;
		_data.cur_wallow_level = _data.imp->_wallow_level;
		_team_state = TEAM_LEADER;
		LeaderAddMember<foo>(first_member,prop,pos);
		_team.change_flag = team_t::TEAM_ALL_CHANGED;
		pPlayer->object_state |= gactive_object::STATE_TEAM | gactive_object::STATE_TEAMLEADER;
		pPlayer->team_id = _team.leader.id;
		_data.imp->_runner->player_in_team(1);
		//使用自己的个人key作为组队副本列表
		_data.imp->_team_ins_key_list = _data.imp->_cur_ins_key_list;

		GLog::log(GLOG_INFO,"用户%d建立了队伍(%d,%d)", _team.leader.id, _team.leader.id,_team.team_seq);
		return true;
	}

	template <int foo>
	bool FromMemberToLeader(const XID & leader)
	{
		if(_team_state != TEAM_MEMBER)
		{
			ASSERT(false);
			return false;
		}
		XID self = _data.imp->_parent->ID;

		_data.change_leader_timeout = 0;
		for(size_t i = 1; i < _team.member_count; i ++)
		{
			if(_team.member_list[i].id == self)
			{
				member_entry ent = _team.member_list[0];
				_team.member_list[0] = _team.member_list[i];
				ent.timeout = TEAM_MEMBER_TIMEOUT;
				_team.member_list[i] = ent;
				_team.leader = self;
				_team_state = TEAM_LEADER;
				_team.change_flag = team_t::TEAM_ALL_CHANGED;
				CalcRaceCount();
				CalcMinLevel();
				gplayer * pPlayer = (gplayer*)(_data.imp->_parent);
				pPlayer->object_state |= gactive_object::STATE_TEAM | gactive_object::STATE_TEAMLEADER;
				pPlayer->team_id = _team.leader.id;
				//用组队列表覆盖个人列表 , 这样这些列表就彻底属于本玩家了
				_data.imp->_cur_ins_key_list = _data.imp->_team_ins_key_list;
				_data.imp->_runner->player_in_team(1);

				GLog::log(GLOG_INFO,"用户%d升为队长(%d,%d)", self.id, leader.id, _team.team_seq);
				return true;
			}
		}

		return false;
	}

	template <int foo>
	bool ChangeLeader(const XID & new_leader)
	{
		if(_team_state != TEAM_MEMBER)
		{
			ASSERT(false);
			return false;
		}
		bool bRst = false;
		for(size_t i = 1; i < _team.member_count; i ++)
		{
			if(_team.member_list[i].id == new_leader)
			{
				member_entry ent = _team.member_list[0];
				_team.member_list[0] = _team.member_list[i];
				_team.member_list[i] = ent;
				_team.leader = new_leader;
				_team.change_flag = team_t::TEAM_ALL_CHANGED;
				gplayer * pPlayer = (gplayer*)(_data.imp->_parent);
				pPlayer->team_id = _team.leader.id;
				bRst = true;
			}
			_team.member_list[i].timeout =TEAM_MEMBER_TIMEOUT;
		}
		return bRst;
	}

	template <int foo>
	bool FromLeaderToMember(const XID & new_leader)
	{
		if(_team_state != TEAM_LEADER)
		{
			ASSERT(false);
			return false;
		}
		for(size_t i = 1; i < _team.member_count; i ++)
		{
			if(_team.member_list[i].id == new_leader)
			{
				member_entry ent = _team.member_list[0];
				_team.member_list[0] = _team.member_list[i];
				_team.member_list[i] = ent;
				_team.leader = new_leader;
				_team_state = TEAM_MEMBER;
				_team.change_flag = team_t::TEAM_ALL_CHANGED;
				gplayer * pPlayer = (gplayer*)(_data.imp->_parent);
				pPlayer->object_state &= ~(gactive_object::STATE_TEAM | gactive_object::STATE_TEAMLEADER);
				pPlayer->object_state |= gactive_object::STATE_TEAM;
				pPlayer->team_id = _team.leader.id;
				_data.imp->_runner->player_in_team(2);

				return true;
			}
		}
		return false;
	}

	template<typename __XID__>
	bool BecomeMember(const __XID__ & leader, const member_entry * list, size_t count)
	{
		if(_team_state != TEAM_WAIT)
		{
			ASSERT(false);
			return false;
		}
		ASSERT(leader == _team.leader);

		memset(&_auto_team_info, 0, sizeof(_auto_team_info));
		_team.member_count = 0;
		_team_state = TEAM_MEMBER;
		_data.cur_max_level = _data.imp->_basic.level;
		_data.cur_wallow_level = _data.imp->_wallow_level;
		LeaderUpdateMembers(leader,list,count);

		gplayer * pPlayer = (gplayer*)(_data.imp->_parent);
		pPlayer->object_state &= ~gactive_object::STATE_TEAMLEADER;
		pPlayer->object_state |= gactive_object::STATE_TEAM;
		pPlayer->team_id = _team.leader.id;
		_data.imp->_runner->player_in_team(2);
		GLog::log(GLOG_INFO,"用户%d成为队员(%d,%d)",pPlayer->ID.id,leader.id,_team.team_seq);
		
		//如果当前出于隐身状态则在进入队伍后通知队友自己appear
		if(pPlayer->IsInvisible()) _data.imp->SendAppearToTeam();
		
		return true;
	}

	template <int foo>
	void BecomeNormal()
	{
		gplayer * pPlayer = (gplayer*)(_data.imp->_parent);
		GLog::log(GLOG_INFO,"用户%d脱离队伍(%d,%d)",pPlayer->ID.id,_team.leader.id,_team.team_seq);
		
		//如果当前处于隐身状态则在离队前通知队友自己disappear
		if(pPlayer->IsInvisible()) _data.imp->SendDisappearToTeam();
		
		_team_state = TEAM_NORMAL;
		_team.leader = XID(-1,-1);
		_team.member_count = 0;
		_team.team_seq = 0;
		_team.team_uid = 0;
		_data.time_out = 0;
		_data.invite_map.clear();
		_data.apply_map.clear();
		pPlayer->object_state &= ~(gactive_object::STATE_TEAMLEADER | gactive_object::STATE_TEAM);
		pPlayer->team_id = 0;
		_data.imp->_runner->player_in_team(0);
	}

	template <int foo>
	bool LeaderAddMember(const XID & member,const agree_invite_entry  &prop,const A3DVECTOR &pos)
	{
		if(_team_state != TEAM_LEADER) return false;
		if(_team.member_count < TEAM_MEMBER_CAPACITY)
		{
			//如果当前处于隐身状态通知新加入的队员自己appear
			gplayer* player = (gplayer*)_data.imp->_parent;
			if(player->IsInvisible()) _data.imp->SendAppearToTeamMember(member.id,prop.cs_index,prop.cs_sid);
			
			int index = _team.member_count;
			_team.member_list[index].id = member;
			_team.member_list[index].is_changed = true;
			_team.member_list[index].timeout =TEAM_MEMBER_TIMEOUT;
			_team.member_list[index].data = prop.data;
			_team.member_list[index].pos = pos ;
			_team.member_list[index].race = prop.race ;
			_team.member_list[index].cs_index = prop.cs_index ;
			_team.member_list[index].cs_sid = prop.cs_sid ;
			_team.member_count ++;
			if(_data.cur_max_level < prop.data.level) _data.cur_max_level = prop.data.level;
			if(_data.cur_wallow_level < prop.data.wallow_level) _data.cur_wallow_level = prop.data.wallow_level;
			_team.change_flag = team_t::TEAM_PART_CHANGED;
			CalcRaceCount();
			CalcMinLevel();
			return true;
		}
		return false;
	}

	member_entry * UpdateMemberData(const XID & id, const A3DVECTOR &pos, const team_mutable_prop & data)
	{
		if(!IsInTeam()) return NULL;
		member_entry * pEntry = FindMember(id);
		if(pEntry)
		{
			if(pEntry->data != data)
			{
				pEntry->data = data;
				pEntry->is_changed = true;
				_team.change_flag = team_t::TEAM_PART_CHANGED;
			}
			//位置数据强制更新
			pEntry->pos = pos;
		}
		return pEntry;
	}

	bool LeaderUpdateMembers(const XID & source,const member_entry * list , size_t count)
	{
		if(source == _team.leader)
		{
			if(count > TEAM_MEMBER_CAPACITY)
			{
				return false;
			}
			for(size_t i = 0; i < count ; i++)
			{
				if(_team.member_count >= i
					|| _team.member_list[i].data != list[i].data
					|| !(_team.member_list[i].id == list[i].id))
				{
					//只有当数据改变才修改数据，这是为了减少带宽占用
					_team.member_list[i] = list[i];
					_team.member_list[i].is_changed = true;
				}
				else
				{
					//什么都不修改 ，当然原来is_changed也可能为true
					//只更新位置信息
					_team.member_list[i].pos = list[i].pos;
				}
			}
			_team.member_count = count;
			_team.change_flag = team_t::TEAM_ALL_CHANGED;
			CalcRaceCount();
			CalcMinLevel();
			return true;
		}
		else
		{
			return false;
		}
	}

	void LeaderRemoveMember(member_entry * pEntry);
	void MemberRemoveMember(member_entry * pEntry);

	template <int foo>
	void SendSelfDataToMember(const team_mutable_prop & data)
	{
		if(!IsInTeam()) return;
		MSG msg;
		BuildMessage(msg,GM_MSG_MEMBER_NOTIFY_DATA,XID(-1,-1),_data.imp->_parent->ID,_data.imp->_parent->pos,0,&data,sizeof(data));
		SendGroupMessage(msg);
	}

	template < int foo>
	void SendTeamDataToClient()
	{
		switch(_team.change_flag)
		{
			case team_t::TEAM_NO_CHANGE:
			return;

			case team_t::TEAM_PART_CHANGED:
			{
				size_t member_count = _team.member_count;
				abase::vector<const member_entry *,abase::fast_alloc<> > list;
				list.reserve(member_count);
				for(size_t i = 0; i < member_count ; i++)
				{
					if(_team.member_list[i].is_changed)
					{
						list.push_back(_team.member_list + i);
						_team.member_list[i].is_changed = false;
					}
				}
				if(list.size())
				{
					_data.imp->SendTeamData(_team.leader,member_count,list.size(),list.begin());
				}
				_team.change_flag = team_t::TEAM_NO_CHANGE;
			}
			return;

			case team_t::TEAM_ALL_CHANGED:
			{
				size_t member_count = _team.member_count;
				if(member_count)
				{
					_data.imp->SendTeamData(_team.leader,member_count,_team.member_list);
				}

				for(size_t i = 0; i < member_count ; i++)
				{
					_team.member_list[i].is_changed = false;
				}
				_team.change_flag = team_t::TEAM_NO_CHANGE;
			}
			return;

			default:
			ASSERT(false);
			break;
		}
		
	}


public:
	void SendGroupMessage(const MSG & msg);
	void SendAllMessage(const MSG & msg, int ex_id);
	void SendAllMessage(const MSG & msg);
	void SendMemberMessage(int idx, MSG & msg);
	void SendMessage(const MSG & msg, float range);		//发送消息到队伍
	void SendAllMessage(const MSG & msg, float range);	//发送消息到队伍 包括自己
	template <int>
	void SendGroupData(const void * buf, size_t size)
	{
		size_t count = _team.member_count;
		int self = _data.imp->_parent->ID.id;
		for(size_t i = 0;i < count; i ++)
		{
			const member_entry &  ent = _team.member_list[i];
			if(self == ent.id.id) continue;
			send_ls_msg(ent.cs_index, ent.id.id, ent.cs_sid,buf,size);
		}
	}

	player_team()
	{
		memset(&_team,0,sizeof(_team));
		memset(&_data,0,sizeof(_data));
		_team_state = TEAM_NORMAL;
		_data.time_out = 0;
		_data.last_invite_time = 0;
		memset(&_auto_team_info, 0, sizeof(_auto_team_info));
	}

	void Init(gplayer_imp * pPlayer);
	
	bool Save(archive & ar);
	bool Load(archive & ar);
	void Swap(player_team & rhs);
	
	const member_entry &GetMember(size_t index)  const
	{ 
		ASSERT(index < _team.member_count);
		return _team.member_list[index];
	}

	int GetMemberNum() const 
	{ 
		if(_team_state != TEAM_MEMBER && _team_state != TEAM_LEADER) return 0;
		return _team.member_count;
	}
	bool IsInTeam() const { return _team_state == TEAM_MEMBER || _team_state == TEAM_LEADER; } 
	bool IsLeader() const { return _team_state == TEAM_LEADER; } 
	bool IsAutoComposingTeam() { return _auto_team_info.composing_timeout > 0; }
	const XID & GetLeader() const  {ASSERT(IsInTeam()); return _team.leader;}
	bool IsMember(const XID & member) const
	{
		ASSERT(IsInTeam());
		for(size_t i = 0; i < _team.member_count; i ++)
		{
			if(_team.member_list[i].id == member)
			{
				return true;
			}
		}
		return false;
	}

	int GetMemberList(XID * list) 
	{ 
		if(_team_state != TEAM_MEMBER && _team_state != TEAM_LEADER) return 0;
		for(size_t i = 0; i < _team.member_count; i ++)
		{
			list[i] = _team.member_list[i].id;
		}
		return _team.member_count;
	}

	int GetMember(const XID & member, A3DVECTOR & pos, int & level,int & tag, int & plane_index)
	{
		if(!IsInTeam()) return -1;
		for(int i = 0; i < (int)_team.member_count; i ++)
		{
			if(_team.member_list[i].id == member)
			{
				pos = _team.member_list[i].pos;
				level = _team.member_list[i].data.level;
				tag = _team.member_list[i].data.world_tag;
				plane_index = _team.member_list[i].data.plane_index;
				return i;
			}
		}
		return -1;
	}
	
	bool GetMemberPos(const XID & member,A3DVECTOR &pos,int & tag, int& plane_index)
	{
		ASSERT(IsInTeam());
		for(size_t i = 0; i < _team.member_count; i ++)
		{
			if(_team.member_list[i].id == member)
			{
				pos = _team.member_list[i].pos;
				tag = _team.member_list[i].data.world_tag;
				plane_index = _team.member_list[i].data.plane_index;
				return true;
			}
		}
		return false;
	}
	
	int GetMemberCountInSpecWorld(int world_tag)
	{
		if(!IsInTeam()) return 0;
		int count = 0;
		for(size_t i = 0; i < _team.member_count; i ++)
			if(_team.member_list[i].data.world_tag == world_tag)
				++ count;
		return count;
	}
	
	void GetTeamID(int & id, int & seq)
	{
		if(IsInTeam())
		{
			id = _team.leader.id;
			seq = _team.team_seq;
		}
	}

	int GetEffLevel()
	{
		if(IsInTeam())
		{
			return _data.cur_max_level;
		}
		return 0;
	}

	int GetWallowLevel();
	int GetTeamSeq()
	{
		ASSERT(IsInTeam());
		return _team.team_seq;
	}

	int GetTeamID()
	{
		if(IsInTeam()) 
			return _team.leader.id;
		else
			return -1;
	}
	
	int64_t GetTeamUID()
	{
		if(IsInTeam())
			return _team.team_uid;
		return 0;
	}

	void OnHeartbeat()
	{
		_team_ctrl[_team_state]->OnHeartbeat(this);
	}

	
	
//玩家的操作和交互消息

	//客户端要求发出邀请				客户端
	bool CliInviteOther(const XID & member)
	{
		if(!(_team_state == TEAM_LEADER) && !(_team_state == TEAM_NORMAL))
		{	
			return false;
		}

		if(IsAutoComposingTeam()) return false; //处于等待自动组队状态的玩家，不能再邀请他人加入
		
		_team_ctrl[_team_state]->CliInviteOther(this,member);
		return true;
	}

	//客户端点了同意加入				客户端
	bool CliAgreeInvite(const XID & leader,int seq)	
	{
		if(_team_state != TEAM_NORMAL)
		{
			return false;
		}

		_team_ctrl[_team_state]->CliAgreeInvite(this, leader,seq);
		return true;
	}

	//客户端点击了拒绝邀请				客户端
	void CliRejectInvite(const XID & leader)
	{
		if(_team_state == TEAM_NORMAL)
		{
			_team_ctrl[_team_state]->CliRejectInvite(this,leader);
		}
	}

	//客户端发出了离开队伍				客户端
	bool CliLeaveParty()
	{
		if(!(_team_state == TEAM_LEADER) && !(_team_state == TEAM_MEMBER))
		{
			return false;
		}

		return _team_ctrl[_team_state]->CliLeaveParty(this);
	}

	//客户端发出了解开队伍                          客户端
	bool CliDismissParty()
	{
		if(_team_state != TEAM_LEADER)
		{
			return false;
		}

		return _team_ctrl[_team_state]->CliDismissParty(this);
	}       

	//客户端要求踢出一个用户			客户端
	bool CliKickMember(const XID & member)	
	{
		if(_team_state != TEAM_LEADER) return false;
		_team_ctrl[TEAM_LEADER]->CliKickMember(this,member);
		return false;
	}

	//加入一个一个队员（该队员同意 邀请）	消息
	void MsgAgreeInvite(const XID & member,const A3DVECTOR &pos,const agree_invite_entry& prop,int seq)
	{
		_team_ctrl[_team_state]->MsgAgreeInvite(this,member,pos,prop,seq);
	}

	//被邀请方发来了拒绝邀请的消息			消息
	void MsgRejectInvite(const XID & member)
	{
		_team_ctrl[_team_state]->MsgRejectInvite(this,member);
	}

	//leader发来了invite消息			消息
	bool MsgInvite(const XID & leader,int seq,int pickup_flag)
	{
		if(_team_state == TEAM_NORMAL)
		{
			if(IsAutoComposingTeam()) //处于等待自动组队的状态
			{
				if(leader != _auto_team_info.candicate_leader) return false;
				_data.last_invite_time  = g_timer.get_systime() + 10;
				_team_ctrl[TEAM_NORMAL]->CliAgreeInvite(this, leader, seq);	
				return true;
			}
			else 
			{
				_team_ctrl[TEAM_NORMAL]->MsgInvite(this,leader,seq,pickup_flag);
				return true;
			}
		}
		else
		{
			return false;
		}
	}

	//leader发来了同意加入的消息  消息
	void MsgJoinTeam(const XID & leader,const member_entry *list,size_t count, int pickup_flag,int64_t team_uid,const void * ins_key , size_t ik_size)
	{
		_team_ctrl[_team_state]->MsgJoinTeam(this,leader,list,count,pickup_flag,team_uid,ins_key,ik_size);
	}
	
	//成员发出了无法加入的消息			消息
	void MsgJoinTeamFailed(const XID & member)
	{
		if(_team_state == TEAM_LEADER)
		{
			_team_ctrl[_team_state]->MsgJoinTeamFailed(this,member);
		}
	}

	//成员发来了更新自身数据的消息			消息
	void MsgMemberUpdateData(const XID & member , const A3DVECTOR &pos, const team_mutable_prop & data)
	{
		if(IsInTeam())
		{
			_team_ctrl[_team_state]->MsgMemberUpdateData(this,member,pos,data);
		}
	}

	//leader发来了更新所有成员数据的消息		消息
	void MsgLeaderUpdateData(const XID & leader, const member_entry * list, size_t count)
	{
		if(_team_state == TEAM_MEMBER)
		{
			_team_ctrl[_team_state]->MsgLeaderUpdateData(this,leader,list,count);
		}
	}

	//leader收到了成员要求离开的消息		消息
	void MsgMemberLeaveRequest(const XID & member)
	{
		if(_team_state == TEAM_LEADER)
		{
			_team_ctrl[TEAM_LEADER]->MsgMemberLeaveRequest(this, member);
		}
	}

	//leader 踢出其他队员				消息 
	void MsgLeaderKickMember(const XID & leader, const XID & member)
	{
		if(_team_state == TEAM_MEMBER)
		{
			_team_ctrl[TEAM_MEMBER]->MsgNotifyMemberLeave(this,leader,member,1);
		}
	}

	//leader  通知告知其他队员的离开消息
	void MsgNotifyMemberLeave(const XID & leader,const XID & member,int type = 0)
	{
		if(_team_state == TEAM_MEMBER)
		{
			_team_ctrl[TEAM_MEMBER]->MsgNotifyMemberLeave(this,leader,member,type);
		}
	}


	//leader 通知有其他队员进入队伍
	void MsgNewMember(const XID & leader, const member_entry * list, size_t count)
	{
		if(_team_state == TEAM_MEMBER)
		{
			_team_ctrl[TEAM_MEMBER]->MsgNewMember(this,leader,list,count);
		}
	}

	//leader传来了取消组队的消息
	void MsgLeaderCancelParty(const XID & leader,int seq)
	{
		if(_team_state == TEAM_MEMBER)
		{
			_team_ctrl[TEAM_MEMBER]->MsgLeaderCancelParty(this,leader,seq);
		}
	}

	//玩家进行下线操作
	void PlayerLostConnection()
	{
		return _team_ctrl[_team_state]->LostConnection(this);
	}

	void PlayerLogout()
	{
		return _team_ctrl[_team_state]->Logout(this);
	}

	//传来了有人申请加入的消息
	void ApplyParty(const XID & who)
	{
		if(IsAutoComposingTeam()) return; //处于自动组队状态，不接受他人申请
		return _team_ctrl[_team_state]->MsgApplyParty(this,who);
	}

	void AgreeApply(int id, bool result)
	{
		return _team_ctrl[_team_state]->CliAgreeApply(this,id,result);
	
	}
	void ApplyPartyReply(const XID & leader,int seq)
	{
		_team_ctrl[_team_state]->MsgApplyPartyReply(this,leader,seq);
	}

	void CliChangeLeader(const XID & new_leader)
	{
		_team_ctrl[_team_state]->CliChangeLeader(this,new_leader);
	}

	void ChangeToLeader(const XID & leader)
	{
		_team_ctrl[_team_state]->MsgChangeToLeader(this,leader);
	}

	void LeaderChanged(const XID & newleader)
	{
		_team_ctrl[_team_state]->MsgLeaderChanged(this,newleader);
	}

	//void DispatchExp(const A3DVECTOR & pos, int exp,int sp,int level);
	void DispatchExp(const A3DVECTOR &pos,int * list ,size_t count, int exp,int sp,int level,int total_level, int max_level ,int min_level, int npcid,float r);
	bool PickupTeamMoney(const A3DVECTOR & pos,int amount)
	{
		if(!IsInTeam() || amount <=0) return false;
		return _team_ctrl[TEAM_LEADER]->PickupTeamMoney(this, pos,amount);
	}

	bool IsRandomPickup()
	{
		ASSERT(IsInTeam());
		return _team.pickup_flag == 0;
	}
	
	void SetPickupFlag(int pickup_flag)
	{
		_data.pickup_flag = pickup_flag;
	}

	XID GetLuckyBoy(XID  self, const A3DVECTOR & pos);
	void NotifyTeamPickup(const A3DVECTOR & pos, int type, int count);
	void TeamChat(char channel, char emote_id, const void * buf, size_t len, int srcid, const void * aux_data, size_t dsize);
	void SendTeamMessage(MSG & msg, float range, bool exclude_self);

	//-------------AUTO TEAM---------------------
	int OnAutoTeamPlayerReady(int leader_id);
	void OnAutoTeamComposeFailed(int leader_id);
	void OnAutoTeamComposeStart(int member_list[], unsigned int cnt);
};

#endif

