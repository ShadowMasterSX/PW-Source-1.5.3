#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <common/packetwrapper.h>
#include "world.h"
#include "config.h"
#include "usermsg.h"
#include "userlogin.h"
#include <common/protocol_imp.h>

bool send_ls_msg(const gplayer * pPlayer, const void * buf, size_t size)
{
	return send_ls_msg(pPlayer->cs_index,pPlayer->ID.id,pPlayer->cs_sid,buf,size);
}

static int slice_send_player_info(slice * pPiece, packet_wrapper & wrapper)
{
	gplayer *pPlayer = pPiece->GetPlayer();
	int count = 0;
	while(pPlayer)
	{
		using namespace S2C;
		if(CMD::Make<INFO::player_info_1>::From(wrapper,pPlayer))
		{
			count ++;
		}
		pPlayer = pPlayer->get_next();
	}
	return count;
}

static int slice_send_player_info_visible(slice * pPiece, packet_wrapper & wrapper, int a_invi_degree, int min_a_invi_degree, int include_team_id)
{
	gplayer *pPlayer = pPiece->GetPlayer();
	int count = 0;
	while(pPlayer)
	{
		if(a_invi_degree >= pPlayer->invisible_degree && min_a_invi_degree < pPlayer->invisible_degree 
			|| include_team_id > 0 && include_team_id == pPlayer->team_id)
		{
			using namespace S2C;
			if(CMD::Make<INFO::player_info_1>::From(wrapper,pPlayer))
			{
				count ++;
			}
		}
		pPlayer = pPlayer->get_next();
	}
	return count;
}

static int slice_send_matter_info(slice * pPiece, packet_wrapper &wrapper)
{
	gmatter *pMatter = pPiece->GetMatter();
	int count = 0;
	while(pMatter && count <= 1024) //避免扔钱攻击
	{
		using namespace S2C;
		CMD::Make<INFO::matter_info_1>::From(wrapper,pMatter);
		pMatter = pMatter->get_next();
		count ++;
	}
	return count;
}

static int slice_send_npc_info(slice * pPiece, packet_wrapper &wrapper)
{
	gnpc *pNPC = pPiece->GetNPC();
	int count = 0;
	while(pNPC)
	{
		using namespace S2C;
		CMD::Make<INFO::npc_info>::From(wrapper,pNPC);
		pNPC = pNPC->get_next();
		count ++;
	}
	return count;
}

static int slice_send_npc_info_visible(slice * pPiece, packet_wrapper &wrapper, int a_invi_degree, int min_a_invi_degree)
{
	gnpc *pNPC = pPiece->GetNPC();
	int count = 0;
	while(pNPC)
	{
		if(a_invi_degree >= pNPC->invisible_degree && min_a_invi_degree < pNPC->invisible_degree)
		{
			using namespace S2C;
			CMD::Make<INFO::npc_info>::From(wrapper,pNPC);
			count ++;
		}
		pNPC = pNPC->get_next();
	}
	return count;
}

static inline void wrapper_send(packet_wrapper & wrapper, link_sid & id,int cmd_type)
{
	if(wrapper.get_counter() > 0)
	{
		using namespace S2C;
		packet_wrapper os;
		CMD::Make<multi_data_header>::From(os,cmd_type,wrapper.get_counter());

		os << wrapper;
		send_ls_msg(id,os.data(),os.size());
		wrapper.clear();
	}
}

enum 
{
	MIN_SEND_COUNT = 256
};

int	get_player_near_info(world *pPlane,gplayer* dest) 
{
	slice * pPiece = dest->pPiece;
	if(pPiece == NULL) return -1;
	link_sid header;
	if(!make_link_sid(dest,header)) return -2;
	float player_x,player_z;
	player_x = dest->pos.x;
	player_z = dest->pos.z;

	packet_wrapper mw(4096),pw(4096),nw(4096);

	//first  get 
	get_slice_info(pPiece,nw,mw,pw);

	//slice around
	int i;
	int total = pPlane->w_far_vision;
	int index = pPlane->GetGrid().GetSliceIndex(pPiece);
	int slice_x,slice_z;
	pPlane->GetGrid().Index2Pos(index,slice_x,slice_z);
	for(i = 0; i <total; i ++)
	{
		world::off_node_t &node = pPlane->w_off_list[i]; 
		int nx = slice_x + node.x_off; 
		int nz = slice_z + node.z_off; 
		if(nx < 0 || nz < 0 || nx >= pPlane->GetGrid().reg_column || nz >= pPlane->GetGrid().reg_row) continue;
		slice * pNewPiece = pPiece + node.idx_off;
		if(i <= pPlane->w_true_vision)
		{
			get_slice_info(pNewPiece,nw,mw,pw);
		}
		else
		{
			get_slice_info(pNewPiece,nw,mw,pw);
		}
		wrapper_test<MIN_SEND_COUNT>(pw,header,S2C::PLAYER_INFO_1_LIST);
		wrapper_test<MIN_SEND_COUNT>(mw,header,S2C::MATTER_INFO_LIST);
		wrapper_test<MIN_SEND_COUNT>(nw,header,S2C::NPC_INFO_LIST);
	}

	wrapper_test<0>(pw,header,S2C::PLAYER_INFO_1_LIST);
	wrapper_test<0>(mw,header,S2C::MATTER_INFO_LIST);
	wrapper_test<0>(nw,header,S2C::NPC_INFO_LIST);
	return 0;
}

void 	get_slice_info(slice* pPiece,packet_wrapper & npc_wrapper,packet_wrapper & matter_wrapper,packet_wrapper &player_wrapper)
{
	//first  get 
	pPiece->Lock();
	player_wrapper._counter += slice_send_player_info(pPiece,player_wrapper);
	matter_wrapper._counter += slice_send_matter_info(pPiece,matter_wrapper);
	npc_wrapper._counter += slice_send_npc_info(pPiece,npc_wrapper);
	pPiece->Unlock();
}

void 	get_slice_info_visible(slice* pPiece,packet_wrapper & npc_wrapper,packet_wrapper & matter_wrapper,packet_wrapper &player_wrapper, int a_invi_degree, int min_a_invi_degree,int include_team_id)
{
	//first  get 
	pPiece->Lock();
	player_wrapper._counter += slice_send_player_info_visible(pPiece,player_wrapper,a_invi_degree,min_a_invi_degree,include_team_id);
	if(min_a_invi_degree < 0)	//<0 表示进入slice 否则表示反隐等级上升
		matter_wrapper._counter += slice_send_matter_info(pPiece,matter_wrapper);
	npc_wrapper._counter += slice_send_npc_info_visible(pPiece,npc_wrapper,a_invi_degree,min_a_invi_degree);
	pPiece->Unlock();
}

void	send_slice_object_info(gplayer* dest,packet_wrapper&npc_wrapper,packet_wrapper&matter_wrapper,packet_wrapper &player_wrapper)
{
	link_sid id;
	if(!make_link_sid(dest,id)) return ;

	wrapper_send(player_wrapper,id,S2C::PLAYER_INFO_1_LIST);
	wrapper_send(matter_wrapper,id,S2C::MATTER_INFO_LIST);
	wrapper_send(npc_wrapper,id,S2C::NPC_INFO_LIST);
}

void 	gather_slice_cs_user(slice * pPiece, cs_user_map & map)
{
	gplayer *pPlayer = pPiece->GetPlayer();
	while(pPlayer)
	{
		int cs_index = pPlayer->cs_index;
		std::pair<int,int> val(pPlayer->ID.id,pPlayer->cs_sid);
		if(cs_index >= 0 && val.first >= 0)
		{
			map[cs_index].push_back(val);
		}
		pPlayer = pPlayer->get_next();
	}
}

void 	gather_slice_cs_user(slice * pPiece, cs_user_map & map,int eid1,int eid2)
{
	gplayer *pPlayer = pPiece->GetPlayer();
	for(;pPlayer; pPlayer = pPlayer->get_next())
	{
		if(pPlayer->ID.id == eid1 || pPlayer->ID.id == eid2) continue;
		int cs_index = pPlayer->cs_index;
		std::pair<int,int> val(pPlayer->ID.id,pPlayer->cs_sid);
		if(cs_index >= 0 && val.first >= 0)
		{
			map[cs_index].push_back(val);
		}
	}
}

void 	gather_slice_cs_user_in_invisible(slice * pPiece, cs_user_map & map,int invi_degree,int include_team_id,int include_id)
{
	gplayer *pPlayer = pPiece->GetPlayer();
	while(pPlayer)
	{
		//反隐等级够或在同一队伍才能看见
		if(pPlayer->anti_invisible_degree >= invi_degree || (include_team_id>0 && include_team_id == pPlayer->team_id) || pPlayer->ID.id == include_id || pPlayer->gm_invisible)
		{
			int cs_index = pPlayer->cs_index;
			std::pair<int,int> val(pPlayer->ID.id,pPlayer->cs_sid);
			if(cs_index >= 0 && val.first >= 0)
			{
				map[cs_index].push_back(val);
			}
		}
		pPlayer = pPlayer->get_next();
	}
}

void 	gather_slice_cs_user_to_spec(slice * pPiece, cs_user_map & map,int invi_degree,int min_invi_degree,int exclude_team_id)
{
	gplayer *pPlayer = pPiece->GetPlayer();
	while(pPlayer)
	{
		if(pPlayer->anti_invisible_degree < invi_degree 
			&& pPlayer->anti_invisible_degree >= min_invi_degree
			&& (exclude_team_id<=0 || exclude_team_id != pPlayer->team_id)
			&& !pPlayer->gm_invisible)
		{
			int cs_index = pPlayer->cs_index;
			std::pair<int,int> val(pPlayer->ID.id,pPlayer->cs_sid);
			if(cs_index >= 0 && val.first >= 0)
			{
				map[cs_index].push_back(val);
			}
		}
		pPlayer = pPlayer->get_next();
	}
}

void 	gather_slice_object(slice * pPiece, abase::vector<int,abase::fast_alloc<> > & list)
{	
	gobject *pObj = pPiece->player_list;
	while(pObj)
	{
		list.push_back(pObj->ID.id);
		pObj = pObj->pNext;
	}
	
	pObj = pPiece->npc_list;
	while(pObj)
	{
		list.push_back(pObj->ID.id);
		pObj = pObj->pNext;
	}
	
	pObj = pPiece->matter_list;
	while(pObj)
	{
		list.push_back(pObj->ID.id);
		pObj = pObj->pNext;
	}
}
	
void 	gather_slice_object_visible(slice * pPiece, abase::vector<int,abase::fast_alloc<> > & list, int a_invi_degree, int min_a_invi_degree, int exclude_team_id)
{	
	gplayer * pPlayer = (gplayer *)pPiece->player_list;
	while(pPlayer)
	{
		if(a_invi_degree >= pPlayer->invisible_degree && min_a_invi_degree < pPlayer->invisible_degree
			&& (exclude_team_id <= 0 || exclude_team_id != pPlayer->team_id))
			list.push_back(pPlayer->ID.id);
		pPlayer = (gplayer *)pPlayer->pNext;
	}
	
	gnpc * pNPC = (gnpc *)pPiece->npc_list;
	while(pNPC)
	{
		if(a_invi_degree >= pNPC->invisible_degree && min_a_invi_degree < pNPC->invisible_degree)
			list.push_back(pNPC->ID.id);
		pNPC = (gnpc *)pNPC->pNext;
	}
	
	gobject* pObj = pPiece->matter_list;
	while(pObj)
	{
		if(min_a_invi_degree < 0)  //<0 表示离开slice 否则表示反隐等级下降
			list.push_back(pObj->ID.id);
		pObj = pObj->pNext;
	}
}

namespace
{
	class SendSliceMsg
	{
		world * _plane;
		cs_user_map &_map;
		public:
			SendSliceMsg(world *plane,cs_user_map &map)
					:_plane(plane),_map(map){}
			inline void operator()(int index,slice * pPiece)
			{
				if(pPiece->player_list == NULL || !pPiece->IsInWorld()) return;
				if(index < _plane->w_true_vision)
				{
					pPiece->Lock();
					gather_slice_cs_user(pPiece,_map);
					pPiece->Unlock();
				}
				else
				{
					pPiece->Lock();
					gather_slice_cs_user(pPiece,_map);
					pPiece->Unlock();
				}
			}
			
	};

	class SendSliceMsg2
	{
		world * _plane;
		cs_user_map &_map;
		int _eid1;
		int _eid2;
		public:
			SendSliceMsg2(world *plane,cs_user_map &map,int eid1,int eid2)
					:_plane(plane),_map(map),_eid1(eid1),_eid2(eid2){}
			inline void operator()(int index,slice * pPiece)
			{
				if(pPiece->player_list == NULL || !pPiece->IsInWorld()) return;
				if(index < _plane->w_true_vision)
				{
					pPiece->Lock();
					gather_slice_cs_user(pPiece,_map,_eid1,_eid2);
					pPiece->Unlock();
				}
				else
				{
					pPiece->Lock();
					gather_slice_cs_user(pPiece,_map,_eid1,_eid2);
					pPiece->Unlock();
				}
			}
			
	};
	
	class SendSliceMsgInInvisible
	{
		world * _plane;
		cs_user_map &_map;
		int _invi_degree;
		int _include_team_id;
		public:
			SendSliceMsgInInvisible(world *plane,cs_user_map &map,int invi_degree,int include_team_id)
					:_plane(plane),_map(map),_invi_degree(invi_degree),_include_team_id(include_team_id){}
			inline void operator()(int index,slice * pPiece)
			{
				if(pPiece->player_list == NULL || !pPiece->IsInWorld()) return;
				if(index < _plane->w_true_vision)
				{
					pPiece->Lock();
					gather_slice_cs_user_in_invisible(pPiece,_map,_invi_degree,_include_team_id);
					pPiece->Unlock();
				}
				else
				{
					pPiece->Lock();
					gather_slice_cs_user_in_invisible(pPiece,_map,_invi_degree,_include_team_id);
					pPiece->Unlock();
				}
			}
			
	};
	
	class SendSliceMsgToSpec
	{
		world * _plane;
		cs_user_map &_map;
		int _invi_degree;
		int _min_invi_degree;
		int _exclude_team_id;
		public:
			SendSliceMsgToSpec(world *plane,cs_user_map &map,int invi_degree,int min_invi_degree,int exclude_team_id)
					:_plane(plane),_map(map),_invi_degree(invi_degree),_min_invi_degree(min_invi_degree),_exclude_team_id(exclude_team_id){}
			inline void operator()(int index,slice * pPiece)
			{
				if(pPiece->player_list == NULL || !pPiece->IsInWorld()) return;
				if(index < _plane->w_true_vision)
				{
					pPiece->Lock();
					gather_slice_cs_user_to_spec(pPiece,_map,_invi_degree,_min_invi_degree,_exclude_team_id);
					pPiece->Unlock();
				}
				else
				{
					pPiece->Lock();
					gather_slice_cs_user_to_spec(pPiece,_map,_invi_degree,_min_invi_degree,_exclude_team_id);
					pPiece->Unlock();
				}
			}
			
	};
}

void	broadcast_chat_msg(int who, const void * buf, size_t size, char channel,char emote_id, const void * aux_data, size_t len)
{
	GMSV::chat_msg target;
	target.speaker = who;
	target.msg = buf;
	target.size = size;
	target.data = aux_data;
	target.dsize = len;
	target.channel = channel;
	target.emote_id = emote_id;
	target.speaker_level = 0;
	GMSV::BroadChatMsg(target);
}

void country_chat_msg(int who, const void * buf, size_t size, char channel,char emote_id, const void * aux_data, size_t len)
{
	GMSV::chat_msg target;
	target.speaker = who;
	target.msg = buf;
	target.size = size;
	target.data = aux_data;
	target.dsize = len;
	target.channel = channel;
	target.emote_id = emote_id;
	target.speaker_level = 0;
	GMSV::CountryChatMsg(target);
}

void	broadcast_cs_msg(world *pPlane,slice * pStart,const void * buf, size_t size,int except_id,int rlevel)
{
	if(pStart == NULL) return;

	cs_user_map map;
	pStart->Lock();
	gather_slice_cs_user(pStart,map);
	pStart->Unlock();

	pPlane->ForEachSlice(pStart,SendSliceMsg(pPlane,map),rlevel);

	multi_send_ls_msg(map,buf,size,except_id);
	map.clear();
}

void	broadcast_cs_msg(world *pPlane,slice * pStart,const void * buf, size_t size,int eid1,int eid2,int rlevel)
{
	if(pStart == NULL) return;

	cs_user_map map;
	pStart->Lock();
	gather_slice_cs_user(pStart,map,eid1,eid2);
	pStart->Unlock();

	pPlane->ForEachSlice(pStart,SendSliceMsg2(pPlane,map,eid1,eid2),rlevel);

	multi_send_ls_msg(map,buf,size);
	map.clear();
}

void	broadcast_cs_msg_in_invisible(world *pPlane,slice * pStart,const void * buf, size_t size,int invi_degree, int include_team_id,int except_id,int rlevel)
{
	if(pStart == NULL) return;

	cs_user_map map;
	pStart->Lock();
	gather_slice_cs_user_in_invisible(pStart,map,invi_degree,include_team_id);
	pStart->Unlock();

	pPlane->ForEachSlice(pStart,SendSliceMsgInInvisible(pPlane,map,invi_degree,include_team_id),rlevel);

	multi_send_ls_msg(map,buf,size,except_id);
	map.clear();
}

void	broadcast_cs_msg_to_spec(world *pPlane,slice * pStart,const void * buf, size_t size,int invi_degree, int min_invi_degree, int exclude_team_id,int except_id,int rlevel)
{
	if(pStart == NULL) return;

	cs_user_map map;
	pStart->Lock();
	gather_slice_cs_user_to_spec(pStart,map,invi_degree,min_invi_degree,exclude_team_id);
	pStart->Unlock();

	pPlane->ForEachSlice(pStart,SendSliceMsgToSpec(pPlane,map,invi_degree,min_invi_degree,exclude_team_id),rlevel);

	multi_send_ls_msg(map,buf,size,except_id);
	map.clear();
}

void 	broadcast_chat_msg(world *pPlane, slice * pStart, const void * msg, size_t size, char channel,char emote_id,const void * aux_data, size_t dsize, int self_id, int self_level, int rlevel)
{
	if(pStart == NULL) return;
	cs_user_map map;
	pStart->Lock();
	gather_slice_cs_user(pStart,map);
	pStart->Unlock();

	pPlane->ForEachSlice(pStart,SendSliceMsg(pPlane,map),rlevel);
	multi_send_chat_msg(map,msg,size,channel,emote_id, aux_data, dsize, self_id, self_level);
}

void SaySomething(world *pPlane, slice * pStart,const char * str , char channel, int self_id)
{
	char buf[2048];
	size_t index = 0;
	while(*str && index < sizeof(buf))
	{
		buf[index++] = *str++;
		buf[index++] = 0;
	}
	
	AutoBroadcastChatMsg(pPlane, pStart, buf, index, channel,0,0,0,self_id,0);
}

void SaySomething(world *pPlane, slice * pStart,const void * msg, size_t size , char channel, int self_id)
{
	AutoBroadcastChatMsg(pPlane, pStart, msg, size, channel,0,0,0,self_id,0);
}

void AutoBroadcastChatMsg(world *pPlane, slice * pStart, const void * msg, size_t size, char channel,char emote_id,const void * aux_data, size_t dsize, int self_id, int self_level, int rlevel)
{
	broadcast_chat_msg(pPlane,pStart,msg,size,channel,emote_id,aux_data,dsize,self_id,self_level,rlevel);
	/*	 现在不再跨越边界，所以本消息不转发了
	if (pStart->IsBorder())
	{
		MSG ex_msg;
		BuildMessage(ex_msg,GM_MSG_FORWARD_CHAT_MSG,XID(GM_TYPE_BROADCAST,-1),XID(-channel,self_id),
						A3DVECTOR(pStart->slice_range.left,0.f,pStart->slice_range.top),
						rlevel,msg,size);
		pPlane->BroadcastSvrMessage(pStart->slice_range,ex_msg,GRID_SIGHT_RANGE);
	}
	*/
}

void AutoBroadcastCSMsg(world * pPlane,slice * pStart, const void * buf,size_t size,int except_id,int rlevel)
{
	broadcast_cs_msg(pPlane,pStart,buf,size,except_id,rlevel);
	if (pStart && pStart->IsBorder())
	{
		MSG msg;
		BuildMessage(msg,GM_MSG_FORWARD_USERBC,XID(GM_TYPE_BROADCAST,-1),XID(0,world_manager::GetWorldIndex()),
						A3DVECTOR(pStart->slice_range.left,0.f,pStart->slice_range.top),
						rlevel,buf,size);
		pPlane->BroadcastSvrMessage(pStart->slice_range,msg,GRID_SIGHT_RANGE);
	}
}

void AutoBroadcastCSMsg(world * pPlane,slice * pStart, const void * buf,size_t size,int eid1,int eid2,int rlevel)
{
	broadcast_cs_msg(pPlane,pStart,buf,size,eid1,eid2,rlevel);
	if (pStart && pStart->IsBorder())
	{
		MSG msg;
		BuildMessage(msg,GM_MSG_FORWARD_USERBC,XID(GM_TYPE_BROADCAST,-1),XID(0,world_manager::GetWorldIndex()),
						A3DVECTOR(pStart->slice_range.left,0.f,pStart->slice_range.top),
						rlevel,buf,size);
		pPlane->BroadcastSvrMessage(pStart->slice_range,msg,GRID_SIGHT_RANGE);
	}
}

void AutoBroadcastCSMsgInInvisible(world * pPlane,slice * pStart, const void * buf,size_t size,int invi_degree, int include_team_id,int except_id,int rlevel)
{
	broadcast_cs_msg_in_invisible(pPlane,pStart,buf,size,invi_degree,include_team_id,except_id,rlevel);
	//暂时先不考虑在边界隐身的情况
	/*if (pStart->IsBorder())
	{
		MSG msg;
		BuildMessage(msg,GM_MSG_FORWARD_USERBC,XID(GM_TYPE_BROADCAST,-1),XID(0,world_manager::GetWorldIndex()),
						A3DVECTOR(pStart->slice_range.left,0.f,pStart->slice_range.top),
						rlevel,buf,size);
		pPlane->BroadcastSvrMessage(pStart->slice_range,msg,GRID_SIGHT_RANGE);
	}*/
}

void AutoBroadcastCSMsgToSpec(world * pPlane,slice * pStart, const void * buf,size_t size,int invi_degree, int min_invi_degree, int exclude_team_id,int except_id,int rlevel)
{
	broadcast_cs_msg_to_spec(pPlane,pStart,buf,size,invi_degree,min_invi_degree,exclude_team_id,except_id,rlevel);
	//暂时先不考虑在边界隐身的情况
	/*if (pStart->IsBorder())
	{
		MSG msg;
		BuildMessage(msg,GM_MSG_FORWARD_USERBC,XID(GM_TYPE_BROADCAST,-1),XID(0,world_manager::GetWorldIndex()),
						A3DVECTOR(pStart->slice_range.left,0.f,pStart->slice_range.top),
						rlevel,buf,size);
		pPlane->BroadcastSvrMessage(pStart->slice_range,msg,GRID_SIGHT_RANGE);
	}*/
}
