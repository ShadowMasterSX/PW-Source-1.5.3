#include "world.h"
#include "playerteam.h"
#include "player_imp.h"
#include <common/protocol.h>
#include <arandomgen.h>
#include "playertemplate.h"
#include "task/taskman.h"
#include <gsp_if.h>
#include "global_drop.h"

class std_team_control : public player_team::team_control
{
	private:
		//客户端操作
		virtual void CliInviteOther(player_team * pTeam,const XID & member)
		{
			return ;
		}

		virtual void CliAgreeInvite(player_team * pTeam,const XID & leader,int seq)
		{
			return ;
		}

		virtual void CliRejectInvite(player_team * pTeam, const XID & member)
		{
			ASSERT(false);
		}

		virtual bool CliLeaveParty(player_team * pTeam) 
		{
			return false;
		}

		virtual void MsgJoinTeam(player_team *pTeam, const XID& leader,const player_team::member_entry*,size_t ,int pickup_flag,int64_t team_uid,const void * ins_key , size_t ik_size)
		{
			//发送不能加入队伍的消息
			gplayer_imp * pImp = GetData(pTeam).imp;
			pImp->SendTo<0>(GM_MSG_JOIN_TEAM_FAILED,leader,0);
		}

		virtual void MsgJoinTeamFailed(player_team *pTeam, const XID & leader)
		{
			return;
		}

		virtual void MsgMemberUpdateData(player_team *,const XID & member, const A3DVECTOR & pos,const team_mutable_prop & data)
		{
			ASSERT(false);
		}

		virtual void MsgLeaderUpdateData(player_team *,const XID & leader,const player_team::member_entry * list,size_t count)
		{
			ASSERT(false);
		}

		virtual	void MsgInvite(player_team *pTeam, const XID & leader,int seq,int pickup_flag)
		{
			ASSERT(false);
		}

		virtual void CliKickMember(player_team *pTeam, const XID & member)
		{
			ASSERT(false);
		}

		virtual void MsgMemberLeaveRequest(player_team * pTeam, const XID & member)
		{
			ASSERT(false);
		}

		void MsgNotifyMemberLeave(player_team * pTeam, const XID & leader,const XID & member,int type) 
		{
			ASSERT(false);
		}

		virtual void MsgNewMember(player_team*,const XID& leader,const player_team::member_entry * list, size_t count)
		{
			ASSERT(false);
		}
		
		virtual void MsgLeaderCancelParty(player_team *,const XID & leader,int seq)
		{
			ASSERT(false);
		}

		virtual bool PickupTeamMoney(player_team * pTeam, const A3DVECTOR & pos, int amount)
		{
			ASSERT(false);
			return false;
		}
		
		virtual void LostConnection(player_team * pTeam)
		{
			// do nothing
		}
		
		virtual void Logout(player_team * pTeam)
		{
			// do nothing
		}

		virtual void CliAgreeApply(player_team *, int id, bool result)
		{
			//do nothing
		}

		virtual void MsgApplyPartyReply(player_team*, const XID & leader, int seq)
		{
			//do nothing
		}
		
		virtual void CliChangeLeader(player_team *, const XID & new_leader)
		{
			//do nothing 
		}

		virtual void MsgChangeToLeader(player_team*, const XID & leader)
		{
			//do nothing
		}
		
		virtual void MsgLeaderChanged(player_team*, const XID & new_leader)
		{
			//do nothing
		}

};

class team_control_normal : public  std_team_control
{
	private:
		virtual void OnHeartbeat(player_team * pTeam) 
		{
			//负责清除超时的邀请
			long t = g_timer.get_systime();
			gplayer_imp * pImp = GetData(pTeam).imp;
			player_team::INVITE_MAP & map = GetData(pTeam).invite_map;
			player_team::INVITE_MAP::iterator it = map.end();
			for(;it != map.begin(); )
			{
				--it;
				if(it->second.timestamp < t)
				{
					pImp->_runner->team_invite_timeout(it->second.id.id);
					map.erase(it);
				}
			}

			if(pTeam->_auto_team_info.composing_timeout > 0)
			{
				if(--pTeam->_auto_team_info.composing_timeout == 0) 
				{
					memset(&pTeam->_auto_team_info, 0, sizeof(pTeam->_auto_team_info));
				}
			}
		}

		//客户端操作
		virtual void CliInviteOther(player_team * pTeam,const XID & member)
		{
			player_team::INVITE_MAP & map = GetData(pTeam).invite_map;
			gplayer_imp * pImp = GetData(pTeam).imp;
			if(map.size() >= player_team::INVITE_MAP::CAPACITY)
			{
				//队列满 发送现在不能邀请的要求
				pImp->_runner->error_message(S2C::ERR_TEAM_CANNOT_INVITE);
				return ;
			}

			if(map.find(member.id) != map.end())
			{
				//队列满 发送现在不能邀请的要求
				pImp->_runner->error_message(S2C::ERR_TEAM_ALREADY_INVITE);
				return ;
			}

			player_team::invite_t it;
			it.timestamp = g_timer.get_systime() + TEAM_INVITE_TIMEOUT;
			it.id = member;
			map[member.id] = it;

			//发送消息 请求玩家加入队伍
			int pickup_flag = GetData(pTeam).pickup_flag;
			pImp->SendTo<0>(GM_MSG_TEAM_INVITE,member,GetData(pTeam).self_seq,&pickup_flag,sizeof(pickup_flag));
		}

		void MsgApplyPartyReply(player_team* pTeam, const XID & leader, int seq)
		{
			player_team::team_entry & data = GetData(pTeam);
			data.last_invite_time  = g_timer.get_systime() + 10;
			CliAgreeInvite(pTeam,leader,seq);
		}
		
		//只有normal有这个能力处理这个操作
		virtual void CliAgreeInvite(player_team * pTeam,const XID & leader,int seq)
		{
			player_team::team_entry & data = GetData(pTeam);
			gplayer_imp * pImp = GetData(pTeam).imp;
			if(data.last_invite_time < g_timer.get_systime())
			{
				//邀请超时了
				pImp->_runner->error_message(S2C::ERR_TEAM_INVITE_TIMEOUT);
				return;
			}

			//进入等待状态
			data.invite_map.clear();
			ChangeState(pTeam,player_team::TEAM_WAIT);
			data.time_out = TEAM_WAIT_TIMEOUT;
			GetTeam(pTeam).leader = leader;
			GetTeam(pTeam).team_seq = seq;

			player_team::agree_invite_entry prop;
			gplayer * pPlayer = (gplayer*)pImp->_parent;
			prop.data.Init(pImp);
			prop.cs_index = pPlayer->cs_index;
			prop.cs_sid = pPlayer->cs_sid;
			prop.race = pPlayer->base_info.race;

			//发出消息给leader
			pImp->SendTo<0>(GM_MSG_TEAM_AGREE_INVITE,leader,seq,&prop,sizeof(prop));

		}

		virtual void CliRejectInvite(player_team * pTeam, const XID & leader)
		{
			player_team::team_entry & data = GetData(pTeam);
			if(data.last_invite_time < g_timer.get_systime())
			{
				//邀请超时了，直接返回
				return;
			}

			//这里不能清除邀请标记和时间戳，因为同时可以有多个邀请

			//发出消息给leader
			gplayer_imp * pImp = data.imp;
			pImp->SendTo<0>(GM_MSG_TEAM_REJECT_INVITE,leader,0);
		}

		//消息
		void MsgAgreeInvite(player_team * pTeam,const XID & member,const A3DVECTOR & pos,const player_team::agree_invite_entry& prop,int seq)
		{
			//受到某个玩家发来了同意加入的消息
			//检查该玩家是否在invite列表中
			player_team::team_entry & data = GetData(pTeam);
			player_team::INVITE_MAP::iterator it = data.invite_map.find(member.id);
			if(data.invite_map.end() == it || seq != data.self_seq)
			{
				//未找到合适的 让它超时
				return;
			}
			data.invite_map.erase(it);

			//组成队伍, 进入组队模式
			//将队员加入到队伍中
			data.min_timer_counter = 0;
			pTeam->BecomeLeader<0>(member,prop,pos);
			data.time_out = TEAM_LEADER_UPDATE_INTERVAL;
			pTeam->_team.team_seq = data.self_seq;
			pTeam->_team.pickup_flag = data.pickup_flag;
			pTeam->_team.team_uid = (int64_t)(world_manager::GetWorldIndex()) << 32 |(int64_t)(world_manager::GetWorldTeamUID());
				
			//发送消息给该队员
			gplayer_imp * pImp = data.imp;
			pImp->_runner->join_team(pTeam->_team.leader,data.pickup_flag);
			int count = pTeam->_team.member_count;
			int pickup_flag = pTeam->_team.pickup_flag;
			size_t buf_size = sizeof(player_team::member_entry) * count + pImp->_cur_ins_key_list.size()*(sizeof(int)*3) + sizeof(int64_t);
			char * buf = (char*)malloc(buf_size);
			size_t buf_uid_header = sizeof(player_team::member_entry) * count;
			memcpy(buf,pTeam->_team.member_list,buf_uid_header);
			memcpy(buf+buf_uid_header,&(pTeam->_team.team_uid),sizeof(int64_t));
			size_t buf_header = buf_uid_header + sizeof(int64_t);
			bool bRst = pImp->GetInstanceKeyBuf(buf + buf_header, buf_size - buf_header);
			ASSERT(bRst);
			pImp->SendTo<0>(GM_MSG_JOIN_TEAM,member,(count & 0x7FFF) |(pickup_flag << 16),
					buf,buf_size);
			free(buf);

			//发送副本数据给该队员 注意可能会在组队消息之前到.....
			
			//进行任务相关的调用
			PlayerTaskInterface  task_if(pImp);
			OnTeamSetup(&task_if);

			//刷新组队标志，用于发送
			pImp->SendTeamVisibleStateToOther(member.id,prop.cs_index,prop.cs_sid);

			pImp->NotifyMasterInfoToPet();
				
			//发送队伍信息到gdelivery,包含了刚入队的队员
			std::vector<int> members;
			members.push_back(pImp->_parent->ID.id);
			members.push_back(member.id);
			GMSV::SendPlayerTeamOp(0,pTeam->_team.team_uid,pImp->_parent->ID.id,members);	
		}

		void MsgRejectInvite(player_team *pTeam,const XID & member)
		{
			//受到某个玩家发来了不同意加入的消息
			//检查该玩家是否在invite列表中
			player_team::team_entry & data = GetData(pTeam);
			player_team::INVITE_MAP::iterator it = data.invite_map.find(member.id);
			if(data.invite_map.end() == it)
			{
				//没有找到,直接返回
				return;
			}
			data.invite_map.erase(it);

			//发送拒绝消息
			data.imp->_runner->player_reject_invite(member);
		}

		void MsgInvite(player_team *pTeam, const XID & leader,int seq,int pickup_flag)
		{
			//收到了一个人发来的邀请消息
			player_team::team_entry & data = GetData(pTeam);

			data.last_invite_time = g_timer.get_systime() + TEAM_INVITE_TIMEOUT2;
			data.imp->_runner->leader_invite(leader,seq,pickup_flag);
		}
		
		void MsgApplyParty(player_team*, const XID & who)
		{
			//do nothing
		}
};

class team_control_wait : public  std_team_control
{
	private:
		virtual void OnHeartbeat(player_team * pTeam) 
		{
			if(--(GetData(pTeam).time_out) <= 0)
			{
				//超时了,回到normal状态
				ChangeState(pTeam,player_team::TEAM_NORMAL);
				pTeam->_data.imp->_runner->error_message(S2C::ERR_TEAM_JOIN_FAILED);
			}
		}

		//客户端操作
		virtual void CliInviteOther(player_team * pTeam,const XID & member)
		{
			ASSERT(false);
		}

		virtual void CliAgreeInvite(player_team * pTeam,const XID & leader,int seq)
		{
			ASSERT(false);
		}

		void CliRejectInvite(player_team * pTeam, const XID & member)
		{
			ASSERT(false);
		}

		//消息

		void MsgRejectInvite(player_team *pTeam,const XID & member)
		{
			//不处理,自己就要加入别的队伍了
			return ;
		}


		void MsgJoinTeam(player_team *pTeam, const XID & leader, const player_team::member_entry * list,size_t count, int pickup_flag,int64_t team_uid,const void * ins_key , size_t ik_size)
		{
			player_team::team_t & team =  GetTeam(pTeam);
			gplayer_imp * pImp = GetData(pTeam).imp;
			if(!(team.leader == leader))
			{
				//不匹配的队长
				//返回错误消息
				pImp->SendTo<0>(GM_MSG_JOIN_TEAM_FAILED,leader,0);
				return ;
			}

			//处理加入队伍的操作,现在不组织队伍列表,因为根本就没有数据
			team.member_count = 0;
			team.pickup_flag = pickup_flag;
			team.team_uid = team_uid;
			GetData(pTeam).min_timer_counter = 0;
			
			//进入队员状态 ,建立队伍结构
			pTeam->BecomeMember(leader,list,count);
			pTeam->_data.time_out = TEAM_LEADER_TIMEOUT;
			__PRINTF("enter team: timeout%d\n",pTeam->_data.time_out);

			//建立和队长一样的instance key
			pImp->SetInstanceKeyBuf(ins_key,ik_size);

			//通知player进入操作状态
			pImp->_runner->join_team(leader,pickup_flag);

			//发送所有的组队可见数据给队友
			pImp->SendTeamDataToMembers();

			pImp->NotifyMasterInfoToPet();

		}

		void MsgAgreeInvite(player_team * pTeam, const XID & member,const A3DVECTOR &,const player_team::agree_invite_entry & prop,int seq)
		{
			//什么都不作，忽略掉
			//因为只有leader处理
		}

		void MsgApplyParty(player_team*, const XID & who)
		{
			//do nothing
		}
};

class team_control_member : public  std_team_control
{
	private:
		virtual void OnHeartbeat(player_team * pTeam) 
		{
			gplayer_imp * pImp = GetData(pTeam).imp;
			//检测如果 leader长时间不发送notify的话,进行超时处理
			//这个超时可以考虑在30秒左右
			if((--GetData(pTeam).time_out) <= 0)
			{
				__PRINTF("队员发现队长超时，自动离开队伍\n");
				//超时了，执行离开队伍逻辑
				MsgNotifyMemberLeave(pTeam, pTeam->_team.leader,pImp->_parent->ID,0);
				return ;
			}
			if((GetData(pTeam).min_timer_counter++) > 30)
			{
				//每30秒计算最小级别
				pTeam->CalcExactMaxLevel();
				GetData(pTeam).min_timer_counter = 0;

			}
		
			//发送队伍数据给客户端
			pTeam->SendTeamDataToClient<0>();

			//首先需要是否要发送数据
			team_mutable_prop data(pImp);
			if(pImp->GetRefreshState())
			{
				pTeam->SendSelfDataToMember<0>(data);
			}
			else if(pTeam->_data.time_out & 0x01)	//每两秒发送一次数据
			{
				player_team::team_t & team =  GetTeam(pTeam);
				pImp->SendTo<0>(GM_MSG_MEMBER_NOTIFY_DATA,team.leader,0,&data,sizeof(data));
			}
		}


		//客户端操作
		virtual void CliInviteOther(player_team * pTeam,const XID & member)
		{
			ASSERT(false);
		}

		virtual void CliAgreeInvite(player_team * pTeam,const XID & leader,int seq)
		{
			ASSERT(false);
		}

		void CliRejectInvite(player_team * pTeam, const XID & leader)
		{
			ASSERT(false);
		}

		bool CliLeaveParty(player_team * pTeam)
		{
			gplayer_imp * pImp = GetData(pTeam).imp;
			player_team::team_t & team =  GetTeam(pTeam);
			//发出离开队伍消息 队长同意后即离开队伍
			pImp->SendTo<0>(GM_MSG_LEAVE_PARTY_REQUEST,team.leader,0);
			return true;
		}

		//消息
		void MsgAgreeInvite(player_team * pTeam, const XID & member,const A3DVECTOR &,const player_team::agree_invite_entry & prop,int seq)
		{
			//此状态不处理此操作,让它超时
			return;
		}

		void MsgRejectInvite(player_team *pTeam,const XID & member)
		{
			//不处理
			return ;
		}

		//成员更新自己的数据
		void MsgMemberUpdateData(player_team * pTeam,const XID & member, const A3DVECTOR &pos,const team_mutable_prop & data)
		{
			pTeam->UpdateMemberData(member,pos,data);
		}

		//队长更新大家的数据
		void MsgLeaderUpdateData(player_team *pTeam,const XID & leader,const player_team::member_entry * list,size_t count)
		{
			pTeam->LeaderUpdateMembers(leader,list,count);
			//更新队长的时间戳
			GetData(pTeam).time_out = TEAM_LEADER_TIMEOUT;

			GetData(pTeam).imp->NotifyMasterInfoToPet();
		}


		//收到leader发来的新人加入消息
		void MsgNewMember(player_team* pTeam,const XID& leader,const player_team::member_entry * list, size_t count)
		{
			ASSERT(count >= 2);
			if(leader != pTeam->_team.leader)  return ;
			pTeam->LeaderUpdateMembers(leader,list,count);
			const player_team::member_entry & ent = list[count-1];
			pTeam->_data.imp->_runner->new_member(ent.id);
			//更新队长的时间戳
			GetData(pTeam).time_out = TEAM_LEADER_TIMEOUT;
			pTeam->_data.imp->SendTeamVisibleStateToOther(ent.id.id,ent.cs_index,ent.cs_sid);

			//如果当前处于隐身状态则通知新加的队友自己appear
			gplayer* player = (gplayer*)pTeam->_data.imp->_parent;
			if(player->IsInvisible()) pTeam->_data.imp->SendAppearToTeamMember(ent.id.id,ent.cs_index,ent.cs_sid);
			
			return ;
		}

		void MsgNotifyMemberLeave(player_team * pTeam, const XID & leader,const XID & member,int type) 
		{
			//收到leader发出的踢出某member的消息
			if(leader != pTeam->_team.leader)  return ;
			gplayer_imp * pImp = pTeam->_data.imp;
			if(member == pImp->_parent->ID)
			{
				//是自己,直接离开队伍
				pTeam->BecomeNormal<0>();
				pImp->_runner->leave_party(leader,type);

				//不需要重建所有的副本key
				//因为自身的key依然存在

				//调用任务系统的接口
				PlayerTaskInterface task_if(pImp);
				OnTeamMemberLeave(&task_if);
				return ;
			}
			
			player_team::member_entry * pEntry = pTeam->FindMember(member);
			if(pEntry)
			{
				//删除该队员
				pTeam->MemberRemoveMember(pEntry);
				pImp->_runner->member_leave(leader,member,type);
			}
			pImp->NotifyMasterInfoToPet();
		}

		//收到leader发来来的取消组队的消息
		virtual void MsgLeaderCancelParty(player_team *pTeam,const XID & leader,int seq)
		{
			if(leader != pTeam->_team.leader || seq != pTeam->_team.team_seq)  return;
			
			pTeam->BecomeNormal<0>();
			ChangeState(pTeam,player_team::TEAM_NORMAL);
			//	pTeam->_data.imp->_runner->leader_cancel_party(leader);
			pTeam->_data.imp->_runner->leave_party(leader,2);

			//调用任务系统的接口
			PlayerTaskInterface task_if(pTeam->_data.imp);
			OnTeamMemberLeave(&task_if);

		}

		virtual void LostConnection(player_team * pTeam)
		{
			CliLeaveParty(pTeam);
		}

		virtual void Logout(player_team * pTeam)
		{
			LostConnection(pTeam);
		}

		void MsgApplyParty(player_team * pTeam, const XID & who)
		{
			//转发组队申请到队长身上
			const XID & leader = pTeam->GetLeader();
			MSG msg;
			BuildMessage(msg, GM_MSG_TEAM_APPLY_PARTY, leader, who,A3DVECTOR(0,0,0)); 
			SendMessage<0>(pTeam->_data.imp->_plane,msg);

		}

		void MsgChangeToLeader(player_team* pTeam, const XID & leader)
		{
			if(leader != GetTeam(pTeam).leader)  return ;
			//消息的发来者的确是leader
			//进行状态和逻辑的转换，将自己变成leader，并发送转换消息给所有的队友 
			//并且调用player的队长转换逻辑
			if(pTeam->FromMemberToLeader<0>(leader))
			{
				MSG msg;
				BuildMessage(msg, GM_MSG_TEAM_LEADER_CHANGED, XID(-1,-1),GetData(pTeam).imp->_parent->ID,A3DVECTOR(0,0,0));
				//考虑附加所有的队员列表
				pTeam->SendGroupMessage(msg);
				
				GetData(pTeam).time_out = 1;	//一秒钟后即进行所有的数据更新
				//给客户端发送消息
				gplayer_imp * pImp = pTeam->_data.imp;
				pImp->_runner->change_team_leader(leader,pImp->_parent->ID);

				PlayerTaskInterface task_if(pImp);
				OnTeamTransferCaptain(&task_if);
				//通知gdeliveryd队长更换
				GMSV::SendPlayerTeamMemberOp(GetTeam(pTeam).team_uid,2,pImp->_parent->ID.id);
			}
		}

		void MsgLeaderChanged(player_team* pTeam, const XID & new_leader)
		{
			if(!pTeam->IsMember(new_leader))
			{
				//该人不在队伍中
				return ;
			}
			
			//这时需广播一个消息通知周围的物品修改其组队权限
			//或者考虑最近30秒有无杀死的怪物（是否得到过组队经验）， 如果有再进行物品区域消息广播
			//这样，物品的所属权里必须加入team/seq作为判定标准
			//现在不考虑这个问题了，反正也并非主要问题
			
			XID old_leader = pTeam->_team.leader;
			pTeam->ChangeLeader<0>(new_leader);
			GetData(pTeam).time_out = TEAM_LEADER_TIMEOUT;

			//给客户端发送消息
			gplayer_imp * pImp = pTeam->_data.imp;
			pImp->_runner->change_team_leader(old_leader,new_leader);
		}
		
		bool PickupTeamMoney(player_team * pTeam, const A3DVECTOR & pos, int amount)
		{
			//虽然不是队长，但是仍然需要进行分配
			if(amount <=0) return false;
			size_t count = pTeam->_team.member_count;
			if(count <= 1) return false;
			size_t acount = (size_t)((float)amount / (float)count);
			if(acount > 200000000) acount = 200000000;

			//开始分发金钱
			amount -= acount * count;
			int ex_id = -1;
			if(amount > 0)
			{
				ex_id = abase::Rand(0,count -1);
			}
			MSG msg;
			gplayer_imp *pImp = pTeam->_data.imp;
			BuildMessage(msg,GM_MSG_RECEIVE_MONEY,XID(-1,-1),pImp->_parent->ID,pos,acount);
			if(acount >0 )
			{
				pTeam->SendAllMessage(msg,ex_id);
			}

			if(amount > 0)
			{
				//随机把无法分配的钱给某个队员
				msg.param += amount;
				pTeam->SendMemberMessage(ex_id,msg);
			}

			return true;
		}

};

class team_control_leader: public std_team_control
{
	private:

		virtual void OnHeartbeat(player_team * pTeam) 
		{
			long ts = g_timer.get_systime();
			gplayer_imp * pImp = GetData(pTeam).imp;
			XID self = pImp->_parent->ID;

			//负责清除超时的邀请
			player_team::INVITE_MAP & map = GetData(pTeam).invite_map;
			player_team::INVITE_MAP::iterator it = map.end();
			for(;it != map.begin(); )
			{
				--it;
				if(it->second.timestamp < ts)
				{
					pImp->_runner->team_invite_timeout(it->second.id.id);
					map.erase(it);
				}
			}

			//负责清除超时的申请
			player_team::INVITE_MAP & apply_map = GetData(pTeam).apply_map;
			it = apply_map.end();
			for(;it != apply_map.begin(); )
			{
				--it;
				if(it->second.timestamp < ts)
				{
					apply_map.erase(it);
				}
			}
			
			//清除超时的改变队长操作
			if(GetData(pTeam).change_leader_timeout)
			{
				if( (-- GetData(pTeam).change_leader_timeout) <= 0)
				{
					pImp->RebuildInstanceKey();
					__leave_party(pTeam);
					return;
				}
			}

			//检测如果 member 长时间不发送notify的话,进行超时处理
			//这个超时可以考虑在10秒左右(或者更加长)
			if(GetData(pTeam).time_out & 0x01)
			{
				MSG msg;
				MSG * pMsg = NULL;
				for(size_t i = 0 ; i < pTeam->_team.member_count; )
				{
					player_team::member_entry & member = pTeam->_team.member_list[i];
					if((member.timeout -= 2 ) <= 0 && member.id != self)
					{	
						//超时,删除之
						if(!pMsg)
						{
							BuildMessage(msg,GM_MSG_MEMBER_LEAVE,XID(-1,-1),
									self,pImp->_parent->pos,member.id.id);
							pMsg = &msg;
						}
						SendGroupMessage(pTeam,*pMsg);
						pImp->_runner->member_leave(self,member.id,0);
						__PRINTF("队长发现队员%d超时,更新队伍列表\n",member.id.id);
						
						int member_id = member.id.id;
						//去除队员		
						pTeam->LeaderRemoveMember(&member);
						if(pTeam->_team.member_count > 1)
						{
							//如果组队人数少于一个，后面会调用__leave_party逻辑
							//通知任务接口
							PlayerTaskInterface task_if(pImp);
							OnTeamDismissMember(&task_if,member_id);
						}


					}
					else
					{
						i ++;
					}
				}
			}

			//如果只剩下自己，则退出队伍
			if(pTeam->_team.member_count <= 1)
			{
				__leave_party(pTeam);
				return;
			}

			//试着发送队伍数据给客户端
			pTeam->SendTeamDataToClient<0>();

			//每30秒计算最小级别
			if((GetData(pTeam).min_timer_counter++) > 30)
			{
				pTeam->CalcExactMaxLevel();
				GetData(pTeam).min_timer_counter = 0;

			}

			//更新自己的位置 
			pTeam->_team.member_list[0].pos = pImp->_parent->pos;

			//是否要发送数据
			if((--GetData(pTeam).time_out) <= 0)
			{
				//发送所有member数据到所有其他玩家
				GetData(pTeam).time_out = TEAM_LEADER_UPDATE_INTERVAL;

				//发送队伍消息给所有队员
				int count = pTeam->_team.member_count;
				MSG msg;
				BuildMessage(msg,GM_MSG_LEADER_UPDATE_MEMBER,XID(-1,-1),
						self,pImp->_parent->pos,
						count,pTeam->_team.member_list,
						sizeof(player_team::member_entry)*count);
				SendGroupMessage(pTeam,msg);
				pTeam->CalcMinLevel();

				//设置全部更新标志，使得自己也可以收到所有数据
				pTeam->_team.change_flag = player_team::team_t::TEAM_ALL_CHANGED;
			}
			else
			if(pImp->GetRefreshState())
			{
				team_mutable_prop data(pImp);
				//更新自己的血值等参数
				pTeam->_team.member_list[0].pos = pImp->_parent->pos;
				pTeam->_team.member_list[0].data = data;
				pTeam->SendSelfDataToMember<0>(data);
			}
		}

		//客户端操作
		virtual void CliInviteOther(player_team * pTeam,const XID & member)
		{
			player_team::INVITE_MAP & map = GetData(pTeam).invite_map;
			gplayer_imp * pImp = GetData(pTeam).imp;
			if(map.size() + GetTeam(pTeam).member_count >= TEAM_MEMBER_CAPACITY)
			{
				//队列满 发送现在不能邀请的要求
				pImp->_runner->error_message(S2C::ERR_TEAM_CANNOT_INVITE);
				return ;
			}

			if(map.find(member.id) != map.end())
			{
				//发送过邀请了
				pImp->_runner->error_message(S2C::ERR_TEAM_ALREADY_INVITE);
				return ;
			}

			player_team::invite_t it;
			it.timestamp = g_timer.get_systime() + TEAM_INVITE_TIMEOUT;
			it.id = member;
			map[member.id] = it;

			//发送消息 请求玩家加入队伍
			int pickup_flag = GetTeam(pTeam).pickup_flag;	//从当前的队伍中取得
			pImp->SendTo<0>(GM_MSG_TEAM_INVITE,member,GetTeam(pTeam).team_seq,&pickup_flag, sizeof(pickup_flag));
		}

		virtual void CliAgreeInvite(player_team * pTeam,const XID & leader,int seq)
		{
			ASSERT(false);
		}

		void CliRejectInvite(player_team * pTeam, const XID & leader)
		{
			ASSERT(false);
		}


		inline 	void __leave_party(player_team * pTeam)
		{
			//通知任务接口 ,必须在真正队伍解散之前完成 放在这里是不是合适?
			PlayerTaskInterface task_if(pTeam->_data.imp);
			OnTeamDisband(&task_if);
			
			if(GetData(pTeam).change_leader_timeout > 0)
			{
				//如果处于等待更改队长的冷却期，则重建副本key
				pTeam->_data.imp->RebuildInstanceKey();
			}

			pTeam->_data.imp->_runner->leave_party(pTeam->_team.leader,0);
			//通知gdelivery队伍解散
			std::vector<int> tmp;
			GMSV::SendPlayerTeamOp(1,GetTeam(pTeam).team_uid,GetTeam(pTeam).leader.id,tmp);
			//转到正常状态
			GetData(pTeam).self_seq ++;
			pTeam->BecomeNormal<0>();
			
		}

		bool CliDismissParty(player_team * pTeam)
		{
			//处理客户端发来的离开队伍的命令
			//这样的要求即是解散队伍
			gplayer_imp * pImp = GetData(pTeam).imp;
			//发出队伍解散消息
			MSG msg;
			BuildMessage(msg,GM_MSG_LEADER_CANCEL_PARTY,XID(-1,-1),
					pImp->_parent->ID,pImp->_parent->pos,
					pTeam->_team.team_seq);
			SendGroupMessage(pTeam,msg);
			__leave_party(pTeam);
			return true;
		}

		bool CliLeaveParty(player_team * pTeam)
		{
			if(pTeam->_team.member_count > 2)
			{
				XID new_leader = pTeam->_team.member_list[1].id;
				CliChangeLeader(pTeam, new_leader);
				pTeam->_data.imp->LazySendTo<0>(GM_MSG_LEAVE_PARTY_REQUEST,new_leader,0,17);

				//重新生成key
				pTeam->_data.imp->RebuildInstanceKey();
			}
			else
			{
				CliDismissParty(pTeam);
			}
			return true;
		}

		void CliKickMember(player_team *pTeam, const XID & member)
		{
			player_team::member_entry * pEntry = pTeam->FindMember(member);
			if(pEntry)
			{
				//给该队员发出踢出的消息，离开的方式也存在
				MSG msg;
				gplayer_imp *pImp = pTeam->_data.imp;
				XID self = pImp->_parent->ID;
				BuildMessage(msg,GM_MSG_LEADER_KICK_MEMBER,XID(-1,-1),
						self,pImp->_parent->pos,member.id);
				SendGroupMessage(pTeam,msg);

				//删除该队员
				pTeam->LeaderRemoveMember(pEntry);
				pImp->_runner->member_leave(self,member,1);

				//考虑剩下了几个队员 
				//如果是一个，那么解散队伍
				if(pTeam->_team.member_count <= 1)
				{
					//踢掉
					__leave_party(pTeam);
					return ;
				}

				//通知任务接口
				PlayerTaskInterface task_if(pImp);
				OnTeamDismissMember(&task_if,member.id);
				
				//通知gdelivery队员离队
				GMSV::SendPlayerTeamMemberOp(GetTeam(pTeam).team_uid,1,member.id);
			}
		}

//消息
		void MsgAgreeInvite(player_team * pTeam, const XID & member,const A3DVECTOR & member_pos,const player_team::agree_invite_entry & prop,int seq)
		{
			//受到某个玩家发来了同意加入的消息
			//检查该玩家是否在invite列表中
			player_team::team_entry & data = GetData(pTeam);
			player_team::INVITE_MAP::iterator it = data.invite_map.find(member.id);
			if( pTeam->_team.member_count >= TEAM_MEMBER_CAPACITY || data.invite_map.end() == it || seq != GetTeam(pTeam).team_seq)
			{
				//未找到合适的 让它超时
				return;
			}
			data.invite_map.erase(it);

			//将队员加入到队伍中
			pTeam->LeaderAddMember<0>(member,prop,member_pos);


			player_team::member_entry * list = pTeam->_team.member_list;
			int count = pTeam->_team.member_count;
			//发送队伍消息给所有队员
			gplayer_imp * pImp = data.imp;
			const A3DVECTOR & pos = pImp->_parent->pos;
			XID self= pImp->_parent->ID;
			MSG msg;
			int pickup_flag = pTeam->_team.pickup_flag;

			size_t buf_size = sizeof(player_team::member_entry) * count + pImp->_cur_ins_key_list.size()*(sizeof(int)*3) + sizeof(int64_t);
			char * buf = (char*)malloc(buf_size);
			size_t buf_uid_header = sizeof(player_team::member_entry) * count;
			memcpy(buf,list,buf_uid_header);
			memcpy(buf+buf_uid_header,&(pTeam->_team.team_uid),sizeof(int64_t));
			size_t buf_header = buf_uid_header + sizeof(int64_t);
			bool bRst = pImp->GetInstanceKeyBuf(buf + buf_header, buf_size - buf_header);
			ASSERT(bRst);
			BuildMessage(msg,GM_MSG_JOIN_TEAM,member,self,pos,(count & 0x7FFF) |(pickup_flag << 16),
					buf,buf_size);
			SendMessage<0>(pImp->_plane,msg);
			free(buf);

			msg.content_length = buf_uid_header;
			msg.content = list;

			//最后一个队员肯定是新队员
			ASSERT(list[count-1].id == member);
			//给所有其他队员发送有新人进入的消息 
			msg.param = count;
			msg.message = GM_MSG_NEW_MEMBER;
			for(int i = 0; i < count - 1 ; i ++)
			{
				msg.target = list[i].id;
				SendMessage<0>(pImp->_plane,msg);
			}

			//重新设置更新时间（因为已经给所有人发送了更新消息）
			data.time_out = TEAM_LEADER_UPDATE_INTERVAL;
			pTeam->_data.imp->_runner->new_member(list[count-1].id);

			pImp->SendTeamVisibleStateToOther(member.id,prop.cs_index,prop.cs_sid);

			//通知任务系统
			PlayerTaskInterface task_if(pImp);
			Task_OnTeamAddMember(&task_if,count - 1);
			
			//通知gdeliveryd
			GMSV::SendPlayerTeamMemberOp(pTeam->_team.team_uid,0,member.id);	
		}


		virtual void MsgJoinTeamFailed(player_team *pTeam, const XID & leader)
		{
			//队员加入组队失败
			//将该队员删除，并通知其他队员
			return;
		}
		
		void MsgRejectInvite(player_team *pTeam,const XID & member)
		{
			//受到某个玩家发来了不同意加入的消息
			//检查该玩家是否在invite列表中
			player_team::team_entry & data = GetData(pTeam);
			player_team::INVITE_MAP::iterator it = data.invite_map.find(member.id);
			if(data.invite_map.end() == it)
			{
				//没有找到,直接返回
				return;
			}
			data.invite_map.erase(it);

			//发送拒绝消息
			data.imp->_runner->player_reject_invite(member);
		}

		void SendGroupMessage(player_team *pTeam,MSG & msg)
		{
			player_team::member_entry * list = pTeam->_team.member_list;
			int count = pTeam->_team.member_count;
			world *pPlane = pTeam->_data.imp->_plane;
			for(int i = 0; i < count; i++)
			{
				if(list[i].id == msg.source) continue;

				msg.target = list[i].id;
				pPlane->PostLazyMessage(msg);
			}
		}


		void MsgMemberLeaveRequest(player_team * pTeam, const XID & member)
		{
			player_team::member_entry * pEntry = pTeam->FindMember(member);
			if(pEntry)
			{
				gplayer_imp *pImp = pTeam->_data.imp;
				//通知所有队员该队员离开，离开者也使用这个消息来离开队伍
				MSG msg;
				BuildMessage(msg,GM_MSG_MEMBER_LEAVE,XID(-1,-1),
						pImp->_parent->ID,pImp->_parent->pos,member.id);
				SendGroupMessage(pTeam,msg);

				pImp->_runner->member_leave(pImp->_parent->ID,member,0);

				// 删除该队员
				pTeam->LeaderRemoveMember(pEntry);

				//考虑剩下了几个队员 
				//如果是一个，那么解散队伍
				if(pTeam->_team.member_count <= 1)
				{
					__leave_party(pTeam);
					return ;
				}

				//通知任务接口
				PlayerTaskInterface task_if(pImp);
				OnTeamDismissMember(&task_if,member.id);
				//通知gdeliveryd
				GMSV::SendPlayerTeamMemberOp(GetTeam(pTeam).team_uid,1,member.id);
			}
		}

		void MsgMemberUpdateData(player_team *pTeam, const XID & member,const A3DVECTOR & pos, const team_mutable_prop & prop)
		{
			//队员更新了数据
			player_team::member_entry * pEntry = pTeam->UpdateMemberData(member,pos,prop);
			if(pEntry)
			{
				//更新队员的时间戳
				pEntry->timeout = TEAM_MEMBER_TIMEOUT;
			}
			else
			{
				gplayer_imp * pImp = pTeam->_data.imp;
				//返回一个错误消息
				pImp->SendTo<0>(GM_MSG_MEMBER_NOT_IN_TEAM,member,0);
			}
		}

		void MsgLeaderUpdateData(player_team *, const XID & leader, const player_team::member_entry *, size_t count)
		{
			ASSERT(false);
			return ;
		}

		bool PickupTeamMoney(player_team * pTeam, const A3DVECTOR & pos, int amount)
		{
			if(amount <=0) return false;
			size_t count = pTeam->_team.member_count;
			if(count <= 1) return false;
			size_t acount = (size_t)((float)amount / (float)count);
			if(acount > 200000000) acount = 200000000;

			//开始分发金钱
			amount -= acount * count;
			int ex_id = -1;
			if(amount > 0)
			{
				ex_id = abase::Rand(0,count -1);
			}
			MSG msg;
			gplayer_imp *pImp = pTeam->_data.imp;
			BuildMessage(msg,GM_MSG_RECEIVE_MONEY,XID(-1,-1),pImp->_parent->ID,pos,acount);
			if(acount >0 )
			{
				pTeam->SendAllMessage(msg,ex_id);
			}

			if(amount > 0)
			{
				//随机把无法分配的钱给某个队员
				msg.param += amount;
				pTeam->SendMemberMessage(ex_id,msg);
			}

			return true;
		}

		void LostConnection(player_team * pTeam)
		{
			//断线了,需要将队长交给第二位
			if(pTeam->_team.member_count >= 2)
			{
				XID new_leader = pTeam->_team.member_list[1].id;
				CliChangeLeader(pTeam, new_leader);
				pTeam->_data.imp->LazySendTo<0>(GM_MSG_LEAVE_PARTY_REQUEST,new_leader,0,37);
			}
			//重新生成key
			pTeam->_data.imp->RebuildInstanceKey();
		}

		void Logout(player_team * pTeam)
		{
			//登出不更改玩家的instance_key,肯定属于自己的
			CliLeaveParty(pTeam);
		}
		
		void MsgApplyParty(player_team* pTeam, const XID & who)
		{
			if(pTeam->_team.member_count >= TEAM_MEMBER_CAPACITY) 
			{
				//考虑是否发出无法加入的申请
				return;
			}

			//自己处理申请
			player_team::INVITE_MAP & map = GetData(pTeam).apply_map;
			gplayer_imp * pImp = GetData(pTeam).imp;
			if(map.size() >= player_team::INVITE_MAP::CAPACITY) return;

			player_team::invite_t it;
			it.timestamp = g_timer.get_systime() + TEAM_INVITE_TIMEOUT;
			it.id = who;
			map[who.id] = it;

			pImp->_runner->send_party_apply(who.id);
		}

		void CliAgreeApply(player_team *pTeam, int id, bool result)
		{
			//自己处理申请
			player_team::INVITE_MAP & map = GetData(pTeam).apply_map;
			player_team::INVITE_MAP & invite_map = GetData(pTeam).invite_map;
			gplayer_imp * pImp = GetData(pTeam).imp;
			if(map.find(id) == map.end())
			{
				//未找到申请人 
				return;
			}

			if(!result) 
			{
				//发送拒绝数据给申请者
				pImp->SendTo<0>(GM_MSG_ERROR_MESSAGE,XID(GM_TYPE_PLAYER,id),S2C::ERR_TEAM_REFUSE_APPLY);
				map.erase(id);
				return;
			}

			if(pTeam->_team.member_count >= TEAM_MEMBER_CAPACITY 
					|| invite_map.size() >= player_team::INVITE_MAP::CAPACITY)
			{
				//考虑是否发出无法加入的申请
				return;
			}

			//发送同意申请的消息给玩家
			//在邀请列表里加入一项
			//发出邀请请求
			player_team::invite_t it;
			it.timestamp = g_timer.get_systime() + TEAM_INVITE_TIMEOUT;
			it.id = XID(GM_TYPE_PLAYER,id);
			invite_map[id] = it;
			pImp->SendTo<0>(GM_MSG_TEAM_APPLY_REPLY,XID(GM_TYPE_PLAYER,id),pTeam->_team.team_seq);
		}

		virtual void CliChangeLeader(player_team * pTeam, const XID & new_leader)
		{
			//检测该目标是否成员
			if(GetData(pTeam).change_leader_timeout) return;
			if(new_leader == pTeam->_team.leader) return;
			if(GetData(pTeam).change_leader_timeout) return;
			if(pTeam->IsMember(new_leader))
			{
				gplayer_imp * pImp = GetData(pTeam).imp;
				pImp->SendTo<0>(GM_MSG_TEAM_CHANGE_TO_LEADER,new_leader,0);
				pTeam->_data.change_leader_timeout = 25;
			}
		}

		void MsgLeaderChanged(player_team* pTeam, const XID & new_leader)
		{
			if(!pTeam->IsMember(new_leader))
			{
				//该人不在队伍中
				return ;
			}
			GetData(pTeam).change_leader_timeout = 0;
			pTeam->FromLeaderToMember<0>(new_leader);
			GetData(pTeam).time_out = TEAM_LEADER_TIMEOUT;
			//重新生成原始key 避免副本重用
			pTeam->_data.imp->RebuildInstanceKey();


			//给客户端发送消息
			gplayer_imp * pImp = pTeam->_data.imp;
			pImp->_runner->change_team_leader(pImp->_parent->ID,new_leader);
		}


};

player_team::team_control * player_team::_team_ctrl[player_team::TEAM_STATE_NUM] =
{
	new team_control_normal,
	new team_control_wait,
	new team_control_member,
	new team_control_leader
};

void
player_team::Init(gplayer_imp * pPlayer)
{
	_data.imp = pPlayer;
	_data.self_seq = g_timer.get_systime();
}



XID 
player_team::GetLuckyBoy(XID  self, const A3DVECTOR & pos)
{
	ASSERT(IsInTeam() && IsRandomPickup());
	int who[TEAM_MEMBER_CAPACITY];	
	int cand_count = 0;
	ASSERT(IsInTeam());
	size_t count = _team.member_count;
	world::object_info info;
	world *pPlane = _data.imp->_plane;
	int tag = world_manager::GetWorldTag();
	for(size_t i = 0; i < count; i ++)
	{
		const XID & member =_team.member_list[i].id;
		if(self != member && 
				(_team.member_list[i].data.world_tag != tag || 
				!pPlane->QueryObject(member,info) || 
				 info.pos.squared_distance(pos) >= TEAM_ITEM_DISTANCE*TEAM_ITEM_DISTANCE))
		{
			continue;
		}

		if(_team.member_list[i].data.hp <= 0) continue;

		who[cand_count] = i;
		cand_count ++;
	}
	if(cand_count <=1) return self;
	int max = 0;
	int index = 0;
	for(int i = 0; i < cand_count; i ++)
	{
		int val = abase::Rand(0,100);
		if(val >= max)
		{
			max = val;
			index = i;
		}
	}
	//int index = abase::Rand(0,cand_count - 1);
	return _team.member_list[who[index]].id;
}

void 
player_team::NotifyTeamPickup(const A3DVECTOR & pos, int type, int count)
{	
	//这里怎么个通知法....如果保存了所有的队友的cs_index和sid的话，可以直接发送数据，也未尝不可
	//这样效率可能比较高   另外组队聊天的话也可以考虑这种方式
	//现在先用消息系统来进行，如果要保存cs_index和sid话，需要做较大变动

	MSG msg;
	BuildMessage(msg,GM_MSG_TEAM_PICKUP,XID(-1,-1),_data.imp->_parent->ID,pos,type,&count,sizeof(count));
	SendGroupMessage(msg);
}

void 
player_team::TeamChat(char channel, char emote_id, const void * buf, size_t len,int srcid, const void * aux_data, size_t dsize)
{	
	GMSV::chat_msg msg;
	int self = srcid;
	msg.speaker = self;
	msg.msg = buf;
	msg.size = len;
	msg.data = aux_data;
	msg.dsize = dsize;
	msg.channel = channel;
	msg.emote_id = emote_id;
	msg.speaker_level = 0;
	size_t count = _team.member_count;
	for(size_t i = 0;i < count; i ++)
	{
		const member_entry & member = _team.member_list[i];
		if(member.id.id != self)
		{
			GMSV::SendChatMsg(member.cs_index, member.id.id, member.cs_sid,msg);
		}
	}
}

void 
player_team::SendGroupMessage(const MSG & msg)
{
	size_t count = _team.member_count;
	size_t index = 0;
	XID  list[TEAM_MEMBER_CAPACITY];
	for(size_t i = 0;i < count; i ++)
	{
		if(_team.member_list[i].id == msg.source) continue;
		list[index++] = _team.member_list[i].id;
	}
	if(index) _data.imp->_plane->SendMessage(list,list + index, msg);
}

void 
player_team::SendAllMessage(const MSG & msg, int ex_id)
{
	size_t count = _team.member_count;
	size_t index = 0;
	XID  list[TEAM_MEMBER_CAPACITY];
	for(size_t i = 0;i < count; i ++)
	{
		if(i == (size_t) ex_id) continue;
		list[index++] = _team.member_list[i].id;
	}
	if(index) _data.imp->_plane->SendMessage(list,list + index, msg);
}

void 
player_team::SendAllMessage(const MSG & msg)
{
	size_t count = _team.member_count;
	XID  list[TEAM_MEMBER_CAPACITY];
	for(size_t i = 0;i < count; i ++)
	{
		list[i] = _team.member_list[i].id;
	}
	_data.imp->_plane->SendMessage(list,list + count, msg);
}

void 
player_team::SendMemberMessage(int idx, MSG & msg)
{
	ASSERT((size_t)idx <_team.member_count);
	msg.target = _team.member_list[idx].id;
	_data.imp->_plane->PostLazyMessage(msg);
}
	
void 
player_team::SendMessage(const MSG & msg, float range)
{
	size_t count = _team.member_count;
	size_t index = 0;
	XID  list[TEAM_MEMBER_CAPACITY];
	ASSERT(msg.source == _data.imp->_parent->ID);
	range *= range;
	world *pPlane = _data.imp->_plane;
	for(size_t i = 0;i < count; i ++)
	{
		const XID & member =_team.member_list[i].id;
		world::object_info info;
		if(msg.source == member  || 
				!pPlane->QueryObject(member,info) || 
				info.pos.squared_distance(msg.pos) >= range)
		{
			continue;
		}
		list[index++] = member;
	}
	if(index) pPlane->SendMessage(list,list + index, msg);
}

void 
player_team::SendAllMessage(const MSG & msg, float range)
{
	size_t count = _team.member_count;
	size_t index = 0;
	XID  list[TEAM_MEMBER_CAPACITY];
	ASSERT(msg.source == _data.imp->_parent->ID);
	range *= range;
	world *pPlane = _data.imp->_plane;
	for(size_t i = 0;i < count; i ++)
	{
		const XID & member =_team.member_list[i].id;
		world::object_info info;
		if(msg.source != member  &&
				(!pPlane->QueryObject(member,info) || 
				 info.pos.squared_distance(msg.pos) >= range))
		{
			continue;
		}
		list[index++] = member;
	}
	if(index) pPlane->SendMessage(list,list + index, msg);
}


bool player_team::Save(archive & ar)
{
	ar << _team_state << _data.time_out << _data.self_seq << _data.pickup_flag << _data.cur_max_level << _data.cur_wallow_level;
	ar << _data.invite_map.size();
	INVITE_MAP::iterator it = _data.invite_map.begin();
	long t = g_timer.get_systime();
	for(; it != _data.invite_map.end(); it ++)
	{
		ar << it->first << (int)(it->second.timestamp - t) << it->second.id;	
	}

	ar << _team.leader << _team.team_seq << _team.member_count << _team.change_flag << _team.pickup_flag << _team.team_uid;
	for(size_t i = 0; i < _team.member_count; i ++)
	{
		member_entry & ent = _team.member_list[i];
		ar << ent.id << ent.data << ent.race << ent.cs_index << ent.cs_sid  << ent.pos << ent.timeout << ent.is_changed;
	}
	return true;
}

void 
player_team::Swap(player_team & rhs)
{
#define Set(var,cls) var = cls.var
	Set(_team_state , rhs);
	Set(_data.time_out,rhs);
	Set(_data.self_seq,rhs);
	Set(_data.pickup_flag,rhs);
	Set(_data.cur_max_level,rhs);
	Set(_data.cur_wallow_level,rhs);
	INVITE_MAP::iterator it = rhs._data.invite_map.begin();
	_data.invite_map.clear();
	for(; it != rhs._data.invite_map.end(); it ++)
	{
		_data.invite_map[it->first] = it->second;
	}

	Set(_team.leader ,rhs);
	Set(_team.team_seq ,rhs);
	Set(_team.member_count ,rhs);
	Set(_team.change_flag ,rhs);
	Set(_team.pickup_flag ,rhs);
	Set(_team.team_uid,rhs);

	for(size_t i = 0; i < _team.member_count; i ++)
	{
		_team.member_list[i] = rhs._team.member_list[i];
	}
#undef Set
}

bool player_team::Load(archive & ar)
{
	ar >> _team_state >> _data.time_out >> _data.self_seq >> _data.pickup_flag >> _data.cur_max_level >> _data.cur_wallow_level;
	size_t size;
	ar >> size;
	_data.invite_map.clear();
	long t = g_timer.get_systime();
	for(size_t i = 0; i < size; i ++)
	{
		int id;
		invite_t inv;
		ar >> id >> (int&)(inv.timestamp) >> inv.id;
		inv.timestamp += t;
		_data.invite_map[id] = inv;
	}

	ar >> _team.leader >> _team.team_seq >> _team.member_count >> _team.change_flag >> _team.pickup_flag >> _team.team_uid;
	for(size_t i = 0; i < _team.member_count; i ++)
	{
		member_entry & ent = _team.member_list[i];
		ar >> ent.id >> ent.data >> ent.race >> ent.cs_index >> ent.cs_sid  >> ent.pos >> ent.timeout >> ent.is_changed;
	}
	return true;
}

void 
player_team::DispatchExp(const A3DVECTOR &pos,int * list ,size_t count, int exp,int sp,int level,int total_level, int max_level ,int min_level,int npcid,float r)
{
	ASSERT(IsLeader());

	//按照最小级别进行加成
	float exp_adj = 0,sp_adj = 0;
	player_template::GetExpPunishment(max_level - level,&exp_adj,&sp_adj);
	//提供队伍加成
	//如果超过20级没有加成
	if(max_level - min_level < 20)
	{
		//这个操作会在比例的基础值上进行调整 
		player_template::SetTeamBonus(count,_data.race_count,&exp_adj,&sp_adj);
	}
	exp = (int)(exp * exp_adj + 0.5f);
	sp = (int)(sp * sp_adj + 0.5f);        

	//进行经验分配
	MSG msg;
	msg_grp_exp_t mexp;
	mexp.level = level;
	mexp.rand = r;
	BuildMessage(msg,GM_MSG_TEAM_EXPERIENCE,XID(-1,-1),_team.leader,pos,npcid,&mexp,sizeof(mexp));

	//total_level里面已经考虑了不得小于20级的限制
	float factor = 1/(float)total_level;

	int effect_player = world_manager::GetInstance()->GetEffectPlayerPerInstance();
	for(size_t i = 0;i < count; i ++)
	{
		if(effect_player && (int)i >= effect_player) break;
		int index = list[i];
		int level = _team.member_list[index].data.level;
		if(level < 20) level = 20;	
		float factor2 = level * factor;
		mexp.exp = (int)(exp * factor2 + 0.5f);
		mexp.sp = (int)(sp * factor2 + 0.5f);
		msg.target = _team.member_list[index].id;
		SendMessage<0>(_data.imp->_plane,msg);
	}
}

void player_team::CalcRaceCount()
{
	_data.race_count = 0;
	if(!IsInTeam()) return;
	size_t member_count = _team.member_count;
	int race_mask = 0;
	for(size_t i = 0; i < member_count ; i++)
	{
		int race = 1 << (_team.member_list[i].race & 0x1F);
		if((race_mask & race) == 0)
		{
			race_mask |= race;
			_data.race_count ++;
		}
	}
}

void player_team::CalcMinLevel()
{
	if(!IsInTeam()) return;
	CalcExactMaxLevel();
}

void 
player_team::CalcExactMaxLevel()
{
	int self = _data.imp->_parent->ID.id;
	A3DVECTOR pos = _data.imp->_parent->pos;
	size_t count = _team.member_count;
	world::object_info info;
	int tag = world_manager::GetWorldTag();

	int max_level = 0;
	int wallow_level = 0;
	for(size_t i = 0; i < count; i ++)
	{
		const member_entry & member = _team.member_list[i];
		if(self == member.id.id)
		{
			int level = _data.imp->_basic.level;
			if(level > max_level)  max_level = level;

			int wallow = _data.imp->_wallow_level;
			if(wallow > wallow_level) wallow_level = wallow;
			continue;
		}
		if(member.data.world_tag != tag)
		{
			continue;
		}
		if(member.pos.squared_distance(pos) >= TEAM_ITEM_DISTANCE*TEAM_ITEM_DISTANCE)
		{
			continue;
		}
		int level = member.data.level;
		if(level > max_level)  max_level = level;

		int wallow = member.data.wallow_level;
		if(wallow > wallow_level) wallow_level = wallow;
	}
	_data.cur_max_level = max_level;
	_data.cur_wallow_level = wallow_level;
}

void 
player_team::LeaderRemoveMember(member_entry * pEntry)
{
	//如果当前处于隐身状态则通知离队的队友自己disappear
	gplayer * pPlayer = (gplayer*)(_data.imp->_parent);
	if(pPlayer->IsInvisible()) _data.imp->SendDisappearToTeamMember(pEntry->id.id,pEntry->cs_index,pEntry->cs_sid);
	
	size_t offset = (_team.member_list + _team.member_count) - pEntry;
	memmove(pEntry,pEntry + 1, offset * sizeof(member_entry));
	_team.member_count --;
	CalcRaceCount();
	CalcMinLevel();
	_data.imp->NotifyMasterInfoToPet();
}

void 
player_team::MemberRemoveMember(member_entry * pEntry)
{
	//如果当前处于隐身状态则通知离队的队友自己disappear
	gplayer * pPlayer = (gplayer*)(_data.imp->_parent);
	if(pPlayer->IsInvisible()) _data.imp->SendDisappearToTeamMember(pEntry->id.id,pEntry->cs_index,pEntry->cs_sid);
	
	size_t offset = (_team.member_list + _team.member_count) - pEntry;
	memmove(pEntry,pEntry + 1, offset * sizeof(member_entry));
	_team.member_count --;
}

int 
player_team::GetWallowLevel()
{
	if(IsInTeam())
	{
		return _data.cur_wallow_level;
	}
	return _data.imp->_wallow_level;

}

int player_team::OnAutoTeamPlayerReady(int leader_id)
{
	if(_team_state != TEAM_NORMAL) return -1;

	_data.invite_map.clear();
	_data.apply_map.clear();
	_auto_team_info.candicate_leader = XID(GM_TYPE_PLAYER, leader_id);
	_auto_team_info.composing_timeout = auto_team_info_t::TIMEOUT_COMPOSING;
	
	return 0;
}

void player_team::OnAutoTeamComposeFailed(int leader_id)
{	
	if(leader_id == _auto_team_info.candicate_leader.id)
	{
		memset(&_auto_team_info, 0, sizeof(_auto_team_info));
	}
}

void player_team::OnAutoTeamComposeStart(int member_list[], unsigned int cnt)
{
	gplayer_imp* pImp = _data.imp;
	if(pImp->_parent->ID != _auto_team_info.candicate_leader || _auto_team_info.composing_timeout <= 0)
	{
		__PRINTF("自动组队将队伍列表发给了不正确的玩家%d，队长应是%d, 自动组队超时倒计时%d\n", pImp->_parent->ID.id, _auto_team_info.candicate_leader.id, _auto_team_info.composing_timeout);
		return;
	}
	if(_team_state != TEAM_NORMAL) return;	

	for(unsigned int i = 0; i < cnt; ++i) 
	{
		if(member_list[i] == pImp->_parent->ID.id) continue;
		
		XID member(GM_TYPE_PLAYER, member_list[i]);
		_team_ctrl[_team_state]->CliInviteOther(this,member);
	}
}

