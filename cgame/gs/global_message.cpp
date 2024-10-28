#include "global_manager.h"
#include "world.h"
#include "player_imp.h"
#include "usermsg.h"
#include "clstab.h"
#include <glog.h>

/*
 *	处理世界消息
 */
int
global_world_message_handler::RecvExternMessage(int msg_tag,const MSG & msg)
{
	if(msg_tag != world_manager::GetWorldTag())
	{
		//要隔断某些消息 ，尚未完成 $$$$$$$$$$
	}
	return _plane->DispatchMessage(msg);
}

/**临时处理用户从其他服务器转移*/
int 
global_world_message_handler::PlayerComeIn(world * pPlane,const MSG &msg)
{
	gplayer * pPlayer = pPlane->GetPlayerByID(msg.source.id);
	if(!pPlayer)
	{
		//这次转移已经超时
		__PRINTF("用户移入,但是用户已经超时\n");
		GLog::log(GLOG_ERR, "Time out when player come in. worldtag:%d:roleid:%d", world_manager::GetWorldTag(), msg.source.id);
		return 0;
	}
	spin_autolock keeper(pPlayer->spinlock);
	if(pPlayer->ID != msg.source || !pPlayer->IsActived() || pPlayer->login_state != gplayer::WAITING_SWITCH)
	{
		//player的状态不正确
		__PRINTF("用户移入,但是用户对象的状态不正确襖n");
		GLog::log(GLOG_ERR, "Invalid state when player come in. worldtag:%d:roleid:%d", world_manager::GetWorldTag(), msg.source.id);
		return 0;
	}
	ASSERT(pPlayer->imp == NULL);
	g_timer.remove_timer(pPlayer->base_info.faction,(void*)pPlayer->base_info.race);

	//删除超时timer

	raw_wrapper wrapper(msg.content,msg.content_length);
	wrapper.SetLimit(raw_wrapper::LOAD_ONLY);
	instance_key ikey;
	int source_tag;
	wrapper >> ikey >> source_tag;
	if(ikey.target.key_level1 != msg.param)
	{
		//key的基础判断不满足
		GLog::log(GLOG_ERR,"invalid instance key while switch %d",pPlayer->ID.id);
		pPlane->FreePlayer(pPlayer);
		//恢复对象失败或者插入失败
		ASSERT(false);
		return 0;
	}
	
	pPlayer->Import(wrapper);
	ASSERT(msg.source == pPlayer->ID);

	int rst = RestoreObject(wrapper,pPlayer,pPlane);

	//检测原始地点是否存在于本世界中，如果不存在，那么执行另外的逻辑
	ASSERT(pPlane->PosInWorld(msg.pos));
	bool is_from_outside = false;
	if(source_tag != world_manager::GetWorldTag() || pPlane->GetGrid().IsOutsideGrid(pPlayer->pos.x,pPlayer->pos.y))
	{
		is_from_outside = true;
		pPlayer->pos = msg.pos;
		float height = pPlane->GetHeightAt(msg.pos.x,msg.pos.z);
		if(pPlayer->pos.y < height) pPlayer->pos.y = height;
	}

	
	if(rst < 0 || pPlane->InsertPlayer(pPlayer) <0)
	{       
		//需要做日志并清除Player对象
		GLog::log(GLOG_ERR,"insert player error while switching %d",pPlayer->ID.id);
		pPlane->FreePlayer(pPlayer);
		//恢复对象失败或者插入失败
		ASSERT(false);
		return 0;
	}       
	pPlane->InsertPlayerToMan(pPlayer);
	

	//试图更新该对象的类
	TrySwapPlayerData(pPlane,_manager->_cid.cid,pPlayer);
	
	gplayer_imp *pImp = (gplayer_imp*)pPlayer->imp;

	if(!is_from_outside)
	{
		//将自己的位置信息发送一下
		A3DVECTOR pos = msg.pos;
		//检查一下现在的坐标是否低于地面 
		float height = pPlane->GetHeightAt(pos.x,pos.z);
		if(pos.y < height) pos.y = height;
		pos -= pPlayer->pos;
		if(pos.squared_magnitude() > 200.f)
		{
			//差别比较大的时候才发送notify_pos
			//可能需要额外的命令,因为客户端只要收到notify_pos就重新加载地图.......
			pImp->_runner->notify_pos(msg.pos);
		}
		pImp->StepMove(pos);
	}
	else
	{
		pImp->_runner->notify_pos(pPlayer->pos);
		pImp->_runner->begin_transfer();
		pImp->_runner->enter_world();
		pImp->_runner->end_transfer();
		pImp->_runner->server_config_data();
	}
	pImp->PlayerEnterServer(source_tag);
	if(world_manager::GetWorldLimit().clearap)
	{
		pImp->ModifyAP(-100000);
	}
	//试着在这时候存盘试试？
//	pImp->_write_timer = abase::Rand(3, 30);

	//发送转移成功的命令
	GMSV::SendSwitchServerSuccess(pPlayer->cs_index,pPlayer->ID.id, pPlayer->cs_sid,world_manager::GetWorldIndex());

	world_manager::GetInstance()->PlayerAfterSwitch(pImp);
	//设置检测filter
	world_manager::GetInstance()->SetFilterWhenLogin(pImp);
	
	//在本地删除列表中的连接，如果有
	pPlane->GetExtObjMan().RemoveObject(pPlayer->ID.id);

	GLog::log(GLOG_INFO,"用户%d(%d,%d)转移到%d",pPlayer->ID.id, pPlayer->cs_index,pPlayer->cs_sid,world_manager::GetWorldTag());

	timeval tv;
	gettimeofday(&tv,NULL);
	__PRINTF("%d转移服务器完成:%u.%u\n",pPlayer->ID.id,tv.tv_sec,tv.tv_usec);
	return 0;
}


namespace
{
	class PlayerLeaveSlice
	{
		cs_user_map _map;
		const void * _buf;
		size_t _size;
		abase::vector<int, abase::fast_alloc<> > _leave_list;
		const link_sid & _lid;
	public:
		PlayerLeaveSlice(const void * buf,size_t size,const link_sid & lid):_buf(buf),_size(size),_lid(lid)
		{
			_leave_list.reserve(64);
		}
		~PlayerLeaveSlice()
		{
			Flush();
		}
		inline void operator()(slice *pPiece)
		{
			pPiece->Lock();
			gather_slice_cs_user(pPiece,_map);
			gather_slice_object(pPiece,_leave_list);
			pPiece->Unlock();
		}

		inline void Flush()
		{
			multi_send_ls_msg(_map,_buf,_size,-1);
			if(_leave_list.size())
			{
				packet_wrapper h1(_leave_list.size()*4 + 16);
				using namespace S2C;
				CMD::Make<CMD::OOS_list>::From(h1,_leave_list.size(), _leave_list.begin());
				send_ls_msg(_lid,h1);
			}

		}
	};

	class PlayerEnterSlice
	{
		cs_user_map _map;
		const void * _buf;
		size_t _size;
		packet_wrapper &_nw;
		packet_wrapper &_mw;
		packet_wrapper &_pw;
	public:
		PlayerEnterSlice(const void * buf,size_t size, 
				packet_wrapper &nw,
				packet_wrapper &mw,
				packet_wrapper &pw):_buf(buf),_size(size),_nw(nw),_mw(mw),_pw(pw){}
		~PlayerEnterSlice()
		{
			Flush();
		}
					
		inline void operator()(slice *pPiece) 
		{
			pPiece->Lock();
			gather_slice_cs_user(pPiece,_map);
			pPiece->Unlock();
			get_slice_info(pPiece,_nw,_mw,_pw);
		}

		inline void Flush()
		{
			multi_send_ls_msg(_map,_buf,_size,-1);
		}
	};
}

static  int PlayerMove(world * pPlane,const MSG & msg)
{
	//这里可能更新外部玩家列表 (现在不载这里更新了，而是使用单独的消息来进行)
	slice * pPiece = pPlane->GetGrid().Locate(msg.pos.x,msg.pos.z);
	const msg_usermove_t * mt = (const msg_usermove_t*)msg.content;
	slice * pNewPiece = pPlane->GetGrid().Locate(mt->newpos.x,mt->newpos.z);
	if(pPiece == pNewPiece || !pPiece || !pNewPiece) return 0;	//有可能
	const void * leavemsg = (const char *)msg.content + sizeof(msg_usermove_t); 
	const void * entermsg = (const char *)msg.content + sizeof(msg_usermove_t) + mt->leave_data_size; 
	ASSERT(mt->leave_data_size + mt->enter_data_size + sizeof(*mt) == msg.content_length);

	// make header
	link_sid lid;
	lid.cs_id = mt->cs_index;
	lid.cs_sid = mt->cs_sid;
	lid.user_id = mt->user_id;
	
	packet_wrapper nw(1024);
	packet_wrapper mw(1024);
	packet_wrapper pw(1024);

	pPlane->MoveBetweenSlice(pPiece,pNewPiece,
					PlayerEnterSlice(entermsg,mt->enter_data_size,nw,mw,pw),
					PlayerLeaveSlice(leavemsg,mt->leave_data_size,lid));
	
	wrapper_test<0>(nw,lid,S2C::NPC_INFO_LIST);
	wrapper_test<0>(mw,lid,S2C::MATTER_INFO_LIST);
	wrapper_test<0>(pw,lid,S2C::PLAYER_INFO_1_LIST);
	return 0;
}

static void RestoreNativeNPC(gnpc * pNativeNPC, gnpc & source)
{
	//首先是放原有内容
	delete pNativeNPC->imp->_runner;
	controller * commander = pNativeNPC->imp->_commander;
	delete pNativeNPC->imp;
	delete commander;
	*pNativeNPC = source;
	pNativeNPC->imp->_parent = pNativeNPC;

}

static int NativeNPCReturnHome(world *pPlane,const MSG &msg)
{
	//本地npc恢复的策略是，生成新的npc对象，然后交换之
	//首先查询到本地npc所在的对象，得到其指针

	size_t index = ID2IDX(msg.source.id);
	ASSERT(index < world_manager::GetMaxNPCCount());

	gnpc * pNativeNPC = pPlane->GetNPCByIndex(index);
	spin_autolock keeper(pNativeNPC->spinlock);
	if(pNativeNPC->pPiece != NULL || !pNativeNPC->IsActived() || !pNativeNPC->native_state)
	{	
		//如果对象不符合要求，那么不进行恢复处理
		//这里应该播放一个NPC消失的消息
	/*	packet_wrapper  h1(64);
		using namespace S2C;
		CMD::Make<CMD::object_disappear>::From(h1,msg.source);
		slice * pPiece = pPlane->GetGrid().Locate(msg.pos.x,msg.pos.z);
		AutoBroadcastCSMsg(pPlane,pPiece,h1);
		这个坐标由于是目标位置，所以有时会范围错误，
		但是不知道为何其他服务器上的人并不会看到怪物消失
		*/
		return -1;
	}

	gnpc npcobj = *pNativeNPC;

	raw_wrapper wrapper(msg.content,msg.content_length);
	wrapper.SetLimit(raw_wrapper::LOAD_ONLY);
	npcobj.Import(wrapper);
	ASSERT(msg.source == npcobj.ID);
	if(npcobj.ID.type != GM_TYPE_NPC || ID2WIDX(npcobj.ID.id) != world_manager::GetWorldIndex())
	{
		//不是NPC对象 或者不是本机的原生NPC，不应该调用这个函数
		GLog::log(GLOG_ERR,"NPC%d转移时导入数据失败",msg.source.id);
		ASSERT(false);
		return 0;
	}

	//设置本地标志
	npcobj.native_state = 1;

	int rst = RestoreObject(wrapper,&npcobj,pPlane);
	if(rst < 0 )
	{

		//恢复对象失败或者插入失败
		//需要做日志并清除NPC对象
		GLog::log(GLOG_ERR,"restore native npc error %d",msg.source.id);
		ASSERT(false);
		return 0;
	}

	//重新进行初始化 
	((gactive_imp*)(npcobj.imp))->ReInit();

	//进行本地npc的恢复工作
	RestoreNativeNPC(pNativeNPC,npcobj);

	slice *pPiece  = pPlane->GetGrid().Locate(pNativeNPC->pos.x,pNativeNPC->pos.z);
	if(pPiece == NULL) 
	{
		((gactive_imp*)(pNativeNPC->imp))->Die(XID(-1,-1),false,0,0);
		//if(pNPC->imp) pNPC->imp->Release();

		GLog::log(GLOG_ERR,"insert native npc error %d",msg.source.id);
		//恢复对象失败或者插入失败
		//需要做日志并清除NPC对象
		ASSERT(false);
		return 0;
	}
	pPiece->Lock();
	pPiece->InsertNPC(pNativeNPC);
	pPiece->Unlock();

	//进行一次行走操作
	A3DVECTOR pos = msg.pos;
	pos -= pNativeNPC->pos;
	pNativeNPC->imp->StepMove(pos);
	__PRINTF("nativeNPC回归家园\n");
	return 0;
}

static int RecvNPCObject(world *pPlane,const MSG &msg)
{
	if(msg.content_length > 12)
	{
		//第一步，检查转移过来的npc是否本地对象，如果是，走本地npc恢复逻辑
		if(ID2WIDX(msg.source.id)  == world_manager::GetWorldIndex()) return NativeNPCReturnHome(pPlane,msg);
		
		gnpc * pNPC = pPlane->AllocNPC();
		raw_wrapper wrapper(msg.content,msg.content_length);
		wrapper.SetLimit(raw_wrapper::LOAD_ONLY);
		pNPC->Import(wrapper);
		ASSERT(msg.source == pNPC->ID);
		if(pNPC->ID.type != GM_TYPE_NPC || ID2WIDX(pNPC->ID.id) == world_manager::GetWorldIndex())
		{
			pPlane->FreeNPC(pNPC);
			pNPC->Unlock();

			//不是NPC对象
			//是本机的原生NPC，不应该调用这个函数
			GLog::log(GLOG_ERR,"invalid native npc data while switching %d",msg.source.id);
			ASSERT(false);
			return 0;
		}
		//NPC 默认是不是原生的
		pNPC->native_state  = 0;

		int rst = RestoreObject(wrapper,pNPC,pPlane);
		if(rst < 0 || pPlane->InsertNPC(pNPC) <0)
		{
			pPlane->FreeNPC(pNPC);
			pNPC->Unlock();
			//if(pNPC->imp) pNPC->imp->Release();

			//恢复对象失败或者插入失败
			//需要做日志并清除NPC对象
			GLog::log(GLOG_ERR,"insert npc error while switching %d",msg.source.id);
			ASSERT(false);
			return 0;
		}
		((gactive_imp*)pNPC->imp)->ReInit();

		pPlane->SetNPCExtern(pNPC);
		//进行一次行走操作
		A3DVECTOR pos = msg.pos;
		pos -= pNPC->pos;
		pNPC->imp->StepMove(pos);
		pNPC->Unlock();
		__PRINTF("收到NPC转移的消息\n");
		return 0;
	}
	else
	{
		GLog::log(GLOG_ERR,"invalid message format %d",msg.source.id);
		ASSERT(false);
	}
	return 0;
}

static inline int PlayerAppear(world *pPlane,const MSG &msg)
{
	ASSERT(msg.content_length == sizeof(int) && msg.source.type == GM_TYPE_PLAYER);
	// make header
	link_sid lid;
	lid.user_id = msg.source.id; 
	lid.cs_sid = *(int*)msg.content;
	lid.cs_id =msg.param;

	int slice_x,slice_z;
	slice * pPiece = pPlane->GetGrid().Locate(msg.pos.x,msg.pos.z,slice_x,slice_z);
	if(pPiece == NULL) return 0;
	packet_wrapper nw(1024);
	packet_wrapper mw(1024);
	packet_wrapper pw(1024);
	get_slice_info(pPiece,nw,mw,pw);

	int i;
	int total = pPlane->w_far_vision;
	enum{MIN_SEND_COUNT = 128};
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
		wrapper_test<MIN_SEND_COUNT>(pw,lid,S2C::PLAYER_INFO_1_LIST);
		wrapper_test<MIN_SEND_COUNT>(mw,lid,S2C::MATTER_INFO_LIST);
		wrapper_test<MIN_SEND_COUNT>(nw,lid,S2C::NPC_INFO_LIST);
	}       
	wrapper_test<0>(pw,lid,S2C::PLAYER_INFO_1_LIST);
	wrapper_test<0>(mw,lid,S2C::MATTER_INFO_LIST);
	wrapper_test<0>(nw,lid,S2C::NPC_INFO_LIST);
	return 0;
}

int
global_world_message_handler::HandleMessage(world * pPlane,const MSG & msg)
{
	//有些消息操作可能会比较费时间，是否可以考虑Task完成，不过用线程的话就要考虑msg的数据问题了。

	//考虑将一些内容移动到基类中
	switch(msg.message)
	{
		case GM_MSG_SWITCH_USER_DATA:
			__PRINTF("收到用户%d的SWITCH_USER_DATA\n",msg.source.id);
			return PlayerComeIn(pPlane,msg);
		case GM_MSG_USER_MOVE_OUTSIDE:
			//暂时先不考虑在边界隐身的情况
			return PlayerMove(pPlane,msg);
		case GM_MSG_FORWARD_USERBC:
			{
				if(msg.content_length)
				{
					slice *pPiece = pPlane->GetGrid().Locate(msg.pos.x,msg.pos.z);
					//broadcast_cs_msg(pPlane,pPiece,msg.content,msg.content_length,msg.param);		//这里应该是写错了lgc
					broadcast_cs_msg(pPlane,pPiece,msg.content,msg.content_length,-1,msg.param);
				}
			}
			break;
		case GM_MSG_FORWARD_CHAT_MSG:
			{
				if(msg.content_length)
				{
					slice *pPiece = pPlane->GetGrid().Locate(msg.pos.x,msg.pos.z);
					broadcast_chat_msg(pPlane,pPiece,msg.content,msg.content_length,-msg.source.type,0,0,0,msg.source.id,0,msg.param);
				}
			}
			break;
		case GM_MSG_FORWARD_BROADCAST:
			{
				if(msg.content_length >= sizeof(MSG))
				{
					//注意，这里有一个const -> non const的转换，在极端情况下有可能出问题
					MSG * pMsg = (MSG *)msg.content;
					if(pMsg->content_length + sizeof(MSG) == msg.content_length)
					{
						pMsg->content = ((char*)msg.content) + sizeof(MSG);
						pPlane->BroadcastLocalMessage(*pMsg,msg.pos.x,msg.param);
					}
					else
					{
						ASSERT(false && "转发消息的格式不正确");
					}
				}
				else
				{
					ASSERT(false && "转发消息大小不正确");
				}
			}
			break;
		case GM_MSG_FORWARD_BROADCAST_CYLINDER:
			{
				if(msg.content_length >= sizeof(MSG))
				{
					//注意，这里有一个const -> non const的转换，在极端情况下有可能出问题
					MSG * pMsg = (MSG *)msg.content;
					if(pMsg->content_length + sizeof(MSG) == msg.content_length)
					{
						pMsg->content = ((char*)msg.content) + sizeof(MSG);
						float fRadius = *(float*)&msg.param;
						pPlane->BroadcastLocalCylinderMessage(*pMsg,msg.pos,fRadius);
					}
					else
					{
						ASSERT(false && "转发消息的格式不正确");
					}
				}
				else
				{
					ASSERT(false && "转发消息大小不正确");
				}
			}
			break;
		case GM_MSG_FORWARD_BROADCAST_SPHERE:
			{
				if(msg.content_length >= sizeof(MSG))
				{
					//注意，这里有一个const -> non const的转换，在极端情况下有可能出问题
					MSG * pMsg = (MSG *)msg.content;
					if(pMsg->content_length + sizeof(MSG) == msg.content_length)
					{
						pMsg->content = ((char*)msg.content) + sizeof(MSG);
						float fRadius = *(float*)&msg.param;
						pPlane->BroadcastLocalSphereMessage(*pMsg,msg.pos,fRadius);
					}
					else
					{
						ASSERT(false && "转发消息的格式不正确");
					}
				}
				else
				{
					ASSERT(false && "转发消息大小不正确");
				}
			}
			break;
		case GM_MSG_FORWARD_BROADCAST_TAPER:
			{
				if(msg.content_length >= sizeof(MSG))
				{
					//注意，这里有一个const -> non const的转换，在极端情况下有可能出问题
					MSG * pMsg = (MSG *)msg.content;
					if(pMsg->content_length + sizeof(MSG) == msg.content_length)
					{
						pMsg->content = ((char*)msg.content) + sizeof(MSG);
						float fRadius = *(float*)&msg.param;
						float cos_half= *(float*)&msg.source.id;
						pPlane->BroadcastLocalTaperMessage(*pMsg,msg.pos,fRadius,cos_half);
					}
					else
					{
						ASSERT(false && "转发消息的格式不正确");
					}
				}
				else
				{
					ASSERT(false && "转发消息大小不正确");
				}
			}
			break;
		case GM_MSG_SWITCH_NPC:
			//NPC切换服务器消息 
			return RecvNPCObject(pPlane,msg);
		case GM_MSG_EXTERN_OBJECT_APPEAR:
			ASSERT(msg.content_length == sizeof(extern_object_manager::object_appear));
			ASSERT(msg.source.type == GM_TYPE_NPC ||msg.source.type == GM_TYPE_PLAYER ||msg.source.type == GM_TYPE_MATTER);
			pPlane->ExtManRefresh(msg.source.id,msg.pos,*(extern_object_manager::object_appear*)msg.content);
			break;
		case GM_MSG_EXTERN_OBJECT_REFRESH:
			ASSERT(msg.content_length == 0);
			ASSERT(msg.source.type == GM_TYPE_NPC ||msg.source.type == GM_TYPE_PLAYER ||msg.source.type == GM_TYPE_MATTER);
			pPlane->ExtManRefreshHP(msg.source.id,msg.pos,msg.param);
			break;
		case GM_MSG_EXTERN_OBJECT_DISAPPEAR:
			ASSERT(msg.content_length == 0);
			ASSERT(msg.source.type == GM_TYPE_NPC ||msg.source.type == GM_TYPE_PLAYER ||msg.source.type == GM_TYPE_MATTER);
			pPlane->ExtManRemoveObject(msg.source.id);
			break;
		case GM_MSG_USER_APPEAR_OUTSIDE:
			//暂时先不考虑在边界隐身的情况
			return PlayerAppear(pPlane,msg);

		case GM_MSG_PLANE_SWITCH_REQUEST:
		//确定切换请求 
		//直接返回反馈 可能需要检查是否可以传送等
		//检查是否有剩余空间
		
		if((size_t)(_manager->GetPlayerAlloced() + 10) < world_manager::GetMaxPlayerCount())
		{
			MSG nmsg = msg;
			nmsg.target = msg.source;
			nmsg.source = msg.target;
			nmsg.message = GM_MSG_PLANE_SWITCH_REPLY;
			pPlane->PostLazyMessage(nmsg);
		}
		else
		{
			//通知客户端无法进入
			MSG nmsg;
			BuildMessage(nmsg,GM_MSG_ERROR_MESSAGE,msg.source,msg.target,msg.pos,S2C::ERR_TOO_MANY_PLAYER_IN_INSTANCE);
			_manager->SendRemotePlayerMsg(msg.source.id, nmsg);
		}
		break;
		case GM_MSG_MAFIA_PVP_STATUS:
		{
			size_t count = msg.content_length/sizeof(int);//同 global_world_manager::OnMafiaPvPStatusNotice	
			int* pctrl_id = (int*)msg.content;

			if(msg.param)
			{
				pPlane->TriggerSpawn(MAFIA_PVP_CTRLID);
				for(size_t n = 0; n < count; ++n,++pctrl_id)
					pPlane->TriggerSpawn(*pctrl_id);
			}
			else
			{
				pPlane->ClearSpawn(MAFIA_PVP_CTRLID);
				for(size_t n = 0; n < count; ++n,++pctrl_id)
					pPlane->ClearSpawn(*pctrl_id);
			}

			GLog::log(GLOG_INFO,"帮派pvp状态改变%d 受影响%d个控制器",msg.param, count);
		}
		break;
		case GM_MSG_MAFIA_PVP_ELEMENT:
		{
			DATA_TYPE dt;
			FACTION_PVP_CONFIG * config = (FACTION_PVP_CONFIG*) world_manager::GetDataMan().get_data_ptr(FACTION_PVP_CONFIG_ID, ID_SPACE_CONFIG, dt);
			if(config == NULL || dt != DT_FACTION_PVP_CONFIG)
			{
				GLog::log(GLOG_ERR,"帮派pvp elements%d 请求失败",FACTION_PVP_CONFIG_ID);
				return false;
			}

			abase::vector<GMSV::MPDomainConfig> domain_list;
			for(size_t n = 0; n < (sizeof(config->list)/sizeof(config->list[0])); ++n)
			{
				GMSV::MPDomainConfig entry;
				entry.domain_count = (unsigned short)config->list[n].domain_count;
				entry.minebase_count = (unsigned short)config->list[n].minebase_gen_count; 
				entry.bonus_minecar = (unsigned int)config->list[n].points_per_gen_minecar; 
				entry.bonus_base = (unsigned int)config->list[n].base_points; 
				entry.rob_minecar_limit = (unsigned short)config->list[n].minecar_count_can_rob; 
				entry.rob_minebase_limit = (unsigned short)config->list[n].minebase_count_can_rob; 
				domain_list.push_back(entry);
			}
			
			abase::vector<int> config_list;
			for(size_t n = 0; n < (sizeof(config->controller_id)/sizeof(config->controller_id[0])); ++n)
				config_list.push_back(config->controller_id[n]);

			GMSV::SendMafiaDomainConfig((GMSV::MPDomainConfig*)domain_list.begin(),domain_list.size(),(int*)config_list.begin(),config_list.size());
			GLog::log(GLOG_INFO,"帮派pvp初始化数据发送 共%d个领地配置信息%d个控制器信息",domain_list.size(),config_list.size());
		}
		break;
		default:
			world_message_handler::HandleMessage(pPlane,msg);
	}
	return 0;
}
