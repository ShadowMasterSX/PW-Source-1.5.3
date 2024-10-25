/*
 * 	处理连接服务器来的命令，主要是用户客户端发来的命令
 */
#ifndef __ONLINEGAME_GS_USERMSG_H__
#define __ONLINEGAME_GS_USERMSG_H__

#include <octets.h>
#include <common/packetwrapper.h>
#include <common/protocol_imp.h>
#include <sys/uio.h>
#include <hashmap.h>

#include <gsp_if.h>
#include "slice.h"

//为处理隐身增加以下两类函数:
//1:隐身状态下根据自身隐身等级以及对象的反隐等级决定是否发送此消息的广播函数 ...InInvisible(...,invi_degree) ...ToSpec()
//2:根据自身反隐等级以及对象的隐身等级来决定是否可获取周围对象信息的函数   ...visible(...,a_invi_degree)
//当前隐身暂未考虑slice在边界的情况

struct iovec;
struct gplayer;
class world;


inline bool send_ls_msg(int cs_index,int userid,int sid,const void * buf, size_t size)
{
	return GMSV::SendClientData(cs_index,userid,sid,buf,size);
}

inline bool send_ls_msg(const link_sid&  id, const void * buf, size_t size)
{
	return send_ls_msg(id.cs_id,id.user_id,id.cs_sid,buf,size);
}

inline void send_ls_msg(const link_sid * first, const link_sid * last , const void * buf, size_t size)
{
	for(;first != last; first++)
	{
		GMSV::SendClientData(first->cs_id,first->user_id,first->cs_sid,buf,size);
	}
}


//typedef std::vector<std::pair<int/*user*/,int/*sid*/> > cs_user_list;
//typedef std::map<int, cs_user_list > cs_user_map;
typedef abase::vector<std::pair<int/*user*/,int/*sid*/>, abase::fast_alloc<> > cs_user_list;
typedef abase::hash_map<int, cs_user_list, abase::_hash_function,abase::fast_alloc<> > cs_user_map;

inline bool multi_send_ls_msg(const cs_user_map & map, const void * buf,size_t size,int except_id)
{
	cs_user_map::const_iterator it = map.begin();	
	for(;it != map.end(); ++it)
	{
		int cs_index = it->first;
		const cs_user_list & list = it->second;
		GMSV::MultiSendClientData(cs_index,list.begin(),list.end(),buf,size,except_id);
	}
	return true;
}

inline bool multi_send_ls_msg(const cs_user_map & map, const void * buf,size_t size)
{
	cs_user_map::const_iterator it = map.begin();	
	for(;it != map.end(); ++it)
	{
		int cs_index = it->first;
		const cs_user_list & list = it->second;
		GMSV::MultiSendClientData(cs_index,list.begin(),list.end(),buf,size);
	}
	return true;
}

inline bool multi_send_chat_msg(const cs_user_map & map, const void * buf,size_t size,char channel,char emote_id,const void * aux_data, size_t dsize, int self_id,int self_level)
{
	cs_user_map::const_iterator it = map.begin();	
	for(;it != map.end(); ++it)
	{
		int cs_index = it->first;
		const cs_user_list & list = it->second;
		GMSV::chat_msg target;
		target.speaker = self_id;
		target.msg = buf;
		target.size = size;
		target.data = aux_data;
		target.dsize = dsize;
		target.channel = channel;
		target.emote_id = emote_id;
		target.speaker_level = self_level;
		GMSV::MultiChatMsg(cs_index,list.begin(),list.end(), target,self_id); 
	}
	return true;
}

void broadcast_chat_msg(int who, const void * buf, size_t size, char channel, char emote_id, const void * aux_data, size_t len);
//国家聊天消息
void country_chat_msg(int who, const void * buf, size_t size, char channel, char emote_id, const void * aux_data, size_t len);

void 	gather_slice_cs_user(slice * pPiece, cs_user_map & map);

//用于玩家或npc或petnpc在隐身状态下，收集能看到自己的玩家,包括队伍成员,(petnpc还包括主人)
void 	gather_slice_cs_user_in_invisible(slice * pPiece, cs_user_map & map, int invi_degree,int include_team_id,int include_id=0);

//用于玩家隐身等级发生变化时，收集是否能看到自己 发生变化的玩家，不包括队伍成员
void 	gather_slice_cs_user_to_spec(slice * pPiece, cs_user_map & map, int invi_degree,int min_invi_degree,int exclude_team_id);
void 	gather_slice_object(slice * pPiece, abase::vector<int, abase::fast_alloc<> > & list);

//用于玩家反隐等级下降或离开slice时，收集leave_list
void 	gather_slice_object_visible(slice * pPiece, abase::vector<int, abase::fast_alloc<> > & list, int a_invi_degree, int min_a_invi_degree = -1, int exclude_team_id = 0);

bool send_ls_msg(const gplayer * pPlayer, const void * buf, size_t size);



//这个函数没有使用暂时先不考虑隐身的情况
int	get_player_near_info(world *pPlane,gplayer* dest);
void 	get_slice_info(slice* pPiece,packet_wrapper & npc_wrapper,packet_wrapper & matter_wrapper,packet_wrapper&player_wrapper);

//用于玩家反隐等级变化或进入slice时，获取player_info npc_info matter_info
void 	get_slice_info_visible(slice* pPiece,packet_wrapper & npc_wrapper,packet_wrapper & matter_wrapper,packet_wrapper&player_wrapper, int a_invi_degree, int min_a_invi_degree = -1, int include_team_id = 0);
void	send_slice_object_info(gplayer* dest,packet_wrapper&npc_wrapper,packet_wrapper&matter_wrapper,packet_wrapper&player_wrapper);

void	broadcast_cs_msg(world *pPlane,slice * pStart, const void * buf, size_t size,int eid=-1,int rlevel=0);	//区域广播，一定是GS_CMD_FORWAD

void	broadcast_cs_msg(world *pPlane,slice * pStart, const void * buf, size_t size,int eid1,int eid2, int rlevel);	//区域广播，一定是GS_CMD_FORWAD

//用于玩家隐身状态下进行区域广播
void	broadcast_cs_msg_in_invisible(world *pPlane,slice * pStart, const void * buf, size_t size,int invi_degree, int include_team_id,int eid,int rlevel);	//区域广播，一定是GS_CMD_FORWAD

//用于玩家隐身等级发生变化时，对区域内 是否能看到自己发生变化 的玩家进行广播
void	broadcast_cs_msg_to_spec(world *pPlane,slice * pStart, const void * buf, size_t size,int invi_degree, int min_invi_degree,int exclude_team_id,int eid,int rlevel);	//区域广播，一定是GS_CMD_FORWAD

void 	broadcast_chat_msg(world *pPlane, slice * pStart, const void * msg, size_t size, char channel,char emote_id,const void * aux_data, size_t dsize, int self_id, int self_level, int rlevel=0);

void AutoBroadcastChatMsg(world *pPlane, slice * pStart, const void * msg, size_t size, char channel,char emote_id,const void * aux_data, size_t dsize, int self_id, int self_level, int rlevel = 0);

void AutoBroadcastCSMsg(world *pPlane,slice * pStart, const void * buf,size_t size,int e_id,int rlevel = 0);	//auto forward

void AutoBroadcastCSMsg(world *pPlane,slice * pStart, const void * buf,size_t size,int eid1,int eid2,int rlevel);	//auto forward

//用于玩家隐身状态下进行区域广播
void AutoBroadcastCSMsgInInvisible(world *pPlane,slice * pStart, const void * buf,size_t size,int invi_degree, int include_team_id,int e_id,int rlevel);	//auto forward

//用于玩家隐身等级发生变化时，对区域内 是否能看到自己发生变化 的玩家进行广播
void AutoBroadcastCSMsgToSpec(world *pPlane,slice * pStart, const void * buf,size_t size,int invi_degree, int min_invi_degree, int exclude_team_id,int e_id,int rlevel);	//auto forward

/*不再使用了
inline void 	send_msg_to_slice(slice *pPiece,const void * buf, size_t size)		//will lock and unlock
{
	cs_user_map map;
	pPiece->Lock();
	gather_slice_cs_user(pPiece,map);
	pPiece->Unlock();
	if(map.size()) multi_send_ls_msg(map,buf,size,-1);
}*/

template<int send_count>
void wrapper_test(packet_wrapper & wrapper, link_sid & id,int cmd_type)
{
	if(wrapper.get_counter() > send_count)
	{
		using namespace S2C;
		packet_wrapper h1(64);
		CMD::Make<multi_data_header>::From(h1,cmd_type,wrapper.get_counter());

		h1 << wrapper;
		send_ls_msg(id,h1.data(),h1.size());
		wrapper.clear();
	}
}

void SaySomething(world *pPlane, slice * pStart,const char * msg, char channel, int self_id);
void SaySomething(world *pPlane, slice * pStart,const void * msg,size_t size, char channel, int self_id);

template<typename WRAPPER>
inline bool send_ls_msg(int cs_index,int userid,int sid,WRAPPER & wrapper)
{
	return send_ls_msg(cs_index,userid,sid,wrapper.data(),wrapper.size());
}

template<typename WRAPPER>
inline bool send_ls_msg(const link_sid&  id,WRAPPER & wrapper)
{
	return send_ls_msg(id,wrapper.data(),wrapper.size());
}

template<typename WRAPPER>
inline bool multi_send_ls_msg(const cs_user_map & map,WRAPPER & wrapper,int except_id)
{
	return multi_send_ls_msg(map,wrapper.data(),wrapper.size(), except_id);
}

template<typename WRAPPER>
inline bool multi_send_ls_msg(const cs_user_map & map,WRAPPER & wrapper)
{
	return multi_send_ls_msg(map,wrapper.data(),wrapper.size());
}

template <typename WRAPPER>
inline bool send_ls_msg(const gplayer * pPlayer, WRAPPER & wrapper)
{
	return send_ls_msg(pPlayer, wrapper.data(),wrapper.size());
}
template <typename WRAPPER>
inline void broadcast_cs_msg(world *pPlane,slice * pStart, WRAPPER & wrapper,int eid=-1,int rlevel =0)
{
	return broadcast_cs_msg(pPlane,pStart,wrapper.data(),wrapper.size(),eid,rlevel);
}

template <typename WRAPPER>
inline void AutoBroadcastCSMsg(world *pPlane,slice * pStart, WRAPPER & wrapper,int e_id=-1,int rlevel = 0)
{
	return AutoBroadcastCSMsg(pPlane,pStart, wrapper.data(), wrapper.size(), e_id,rlevel);
}

template <typename WRAPPER>
inline void AutoBroadcastCSMsg(world *pPlane,slice * pStart, WRAPPER & wrapper,int eid1,int eid2,int rlevel)
{
	return AutoBroadcastCSMsg(pPlane,pStart, wrapper.data(), wrapper.size(), eid1,eid2,rlevel);
}

//用于玩家隐身状态下进行区域广播
template <typename WRAPPER>
inline void AutoBroadcastCSMsgInInvisible(world *pPlane,slice * pStart, WRAPPER & wrapper,int invi_degree,int include_team_id = 0, int e_id=-1,int rlevel = 0)
{
	return AutoBroadcastCSMsgInInvisible(pPlane,pStart, wrapper.data(), wrapper.size(),invi_degree,include_team_id,e_id,rlevel);	
}

//用于玩家隐身等级发生变化时，对区域内 是否能看到自己发生变化 的玩家进行广播
template <typename WRAPPER>
inline void AutoBroadcastCSMsgToSpec(world *pPlane,slice * pStart, WRAPPER & wrapper,int invi_degree, int min_invi_degree, int exclude_team_id = 0, int e_id=-1,int rlevel = 0)
{
	return AutoBroadcastCSMsgToSpec(pPlane,pStart, wrapper.data(), wrapper.size(),invi_degree,min_invi_degree,exclude_team_id,e_id,rlevel);	
}

#endif

