#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "world.h"
#include "player.h"
#include "player_imp.h"
#include "config.h"
#include "userlogin.h"
#include "usermsg.h"
#include "clstab.h"
#include "playertemplate.h"

#include <deque>
#include <db_if.h>
#include "task/taskman.h"
#include <base64.h>

void do_player_login(const A3DVECTOR & ppos, const GDB::base_info * pInfo, const GDB::vecdata * data,const userlogin_t &user, char flag)
{
	gplayer * _player = user._player;
	world * _plane = user._plane;
	
	_player->login_state = gplayer::WAITING_ENTER;
	if(!world_manager::GetInstance()->IsMobileWorld())
	{
		_player->pos = ppos;
	}
	else
	{
		//手机用户服务器,为玩家随机分配一个虚拟位置
		const rect & region = _plane->GetLocalWorld();
		_player->pos.x = abase::Rand(region.left, region.right);
		_player->pos.z = abase::Rand(region.top, region.bottom);
		_player->pos.y = 0;
	}
	//判定一下玩家的位置是否低于地面
	float hy = _plane->GetHeightAt(_player->pos.x, _player->pos.z);
	if(_player->pos.y < hy) _player->pos.y = hy;
	
	_player->pPiece = NULL;

	//这是帮派状态
	_player->id_mafia = pInfo->factionid;
	_player->rank_mafia = pInfo->factionrole;
	if(pInfo->factionid)
	{
		_player->object_state |= gactive_object::STATE_MAFIA;
	}

	//创建玩家数据，可能需要有多种创建方式
//	_player->imp = CF_Create(CLS_PLAYER_IMP,CLS_GM_DISPATCHER,CLS_PLAYER_CONTROLLER,_plane,_player);
//	_player->imp = CF_Create(CLS_PLAYER_IMP,CLS_GM_DISPATCHER,CLS_PLAYER_CONTROLLER,_plane,_player);
	world_manager::player_cid cid;
	world_manager::GetInstance()->GetPlayerCid(cid);
	_player->imp = CF_Create(cid.cid[0],cid.cid[2],cid.cid[1],_plane,_player);

	_player->msg_mask = gobject::MSG_MASK_ATTACK;	//这里还有用吗？$$$$$
	_player->body_size = PLAYER_BODYSIZE;

	
	gplayer_imp * pImp = (gplayer_imp *)_player->imp;
	//手机用户保存下真实位置
	pImp->SaveRealWorldPos(pInfo->worldtag, A3DVECTOR(pInfo->posx, pInfo->posy, pInfo->posz));
	//初始化玩家属性
	if(data->property.data)
	{
		if(data->property.size < sizeof(pImp->_base_prop))
		{
			ASSERT(data->property.size == sizeof(pImp->_base_prop) - 4);
			memcpy(&pImp->_base_prop,data->property.data,data->property.size);
			pImp->_base_prop.max_ap = 0;
		}
		else
		{
			ASSERT(data->property.size == sizeof(pImp->_base_prop));
			memcpy(&pImp->_base_prop,data->property.data,sizeof(pImp->_base_prop));
		}

	}
	else
	{
		ASSERT(false);
	}
	pImp->SetPlayerClass(pInfo->cls,pInfo->gender);
	pImp->SetBasicData(pInfo->level,pInfo->sec_level,pInfo->exp,pInfo->sp,pInfo->hp,pInfo->mp,pInfo->pp,pInfo->money,pInfo->userid);
	pImp->SetPlayerName(data->user_name.data,data->user_name.size);
	pImp->SetDBTimeStamp(pInfo->timestamp);
	pImp->SetSpouse(pInfo->spouse);
	player_template::InitPlayerData(pInfo->cls,pImp);
	//转生，由于其他系统初始化会使用历史最高等级，所以转生初始化提前
	pImp->_player_reincarnation.InitFromDBData(data->reincarnation);
	pImp->SetDBLeadership(pInfo->leadership);
	
	//设置玩家是否在跨服中央服
	pImp->SetInCentralServer(flag == GMSV::CHDS_FLAG_DS_TO_CENTRALDS || flag == GMSV::CHDS_FLAG_DIRECT_TO_CENTRALDS, pInfo->src_zoneid);

	//设置debug模式
	gplayer_controller * pCtrl = (gplayer_controller*)pImp->_commander;
	pCtrl->SetDebugMode(player_template::GetDebugMode());

	pCtrl->SetPrivilege(user._auth_data,user._auth_size);
	if(user._auth_data)
	{
		int ad[256];
		size_t len = user._auth_size;
		if(len > sizeof(ad)) len = sizeof(ad);
		memset(ad,0,sizeof(ad));
		memcpy(ad,user._auth_data,len);
		GLog::log(GLOG_INFO,"用户%d拥有GM权限%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x",user._uid,ad[0],ad[1],ad[2],ad[3],ad[4],ad[5],ad[6],ad[7],ad[8],ad[9]);
	}

      	pImp->SetRestTimeParam(pInfo->dbltime_expire,pInfo->dbltime_mode,pInfo->dbltime_begin,pInfo->dbltime_used,pInfo->dbltime_max);
	pImp->SetExtRestParam(data->dbltime_data.data,data->dbltime_data.size);
	pImp->SetMallInfo(pInfo->cash, pInfo->cash_used, pInfo->cash_delta, pInfo->cash_add, pInfo->cash_serial);
	pImp->SetMallConsumption(pInfo->mall_consumption);
	pImp->_dividend_mall_info.SetDividendMallInfo(pInfo->referrer, pInfo->bonus_add + pInfo->bonus_reward, pInfo->bonus_used);
	pImp->CalcRestTime();
	pImp->SetPlayEd(g_timer.get_systime(),pInfo->time_used);

	int tu = pInfo->time_used;
	GLog::log(GLOG_INFO,"用户%d已经游戏了%d小时%02d分%02d秒",user._uid,tu/3600, (tu/60)%60, tu % 60);
	GLog::log(GLOG_INFO,"用户%d包裹金钱%d,仓库金钱%d  sp%d, 帐号仓库(金钱%d 可操作格子数%d 物品数%d)", user._uid, pInfo->money, pInfo->trash_money,pInfo->sp, pInfo->userstoremoney, pInfo->userstoresize, data->user_store.count);
	GLog::log(GLOG_INFO,"用户%d现在有%d秒双倍经验时间,可申请双倍时间为%d秒",user._uid,pImp->GetDoubleExpTime(),pImp->GetDoubleExpAvailTime());

	pImp->SetLastLoginTime(pInfo->lastlogin_time);
	pImp->SetCreateTime(pInfo->create_time);

	//初始化玩家宠物数据
	if(data->pets.capacity)
	{
		pImp->SetPetSlotCapacity(data->pets.capacity,false);
		size_t count = data->pets.count;
		ASSERT(count <= pet_manager::MAX_PET_CAPACITY);
		for(size_t i = 0; i < count ; i ++)
		{
			size_t index = data->pets.list[i].index;
			const void * pData = data->pets.list[i].data.data;
			size_t psize = data->pets.list[i].data.size;
			pImp->DBSetPetData(index,pData,psize);
		}
	}

	//初始化玩家副本key
	pImp->InitInstanceKey(pInfo->create_time, data->instancekeylist.data,data->instancekeylist.size);

	//设置玩家路点信息
	pImp->SetWaypointList(data->waypoint_list.data,data->waypoint_list.size);

	//设置玩家红名粉名数据 
	pImp->SetInvadeState(pInfo->invader_state, pInfo->invader_time, pInfo->pariah_time);

	//设置玩家动态数据临时
	if(data->var_data.size >= sizeof(int))
	{
		player_var_data::SetData(_player,pImp,data->var_data.data,data->var_data.size);
	}

	//设置玩家角色状态
	pImp->SetPlayerCharMode(data->charactermode.data,data->charactermode.size);

	//设置玩家的冷却时间数据
	pImp->SetCoolDownData(data->coolingtime.data,data->coolingtime.size);

	//设置玩家的个性化数据版本号
	_player->custom_crc = (pInfo->custom_crc & 0xFFFF);

	//设置玩家的初始修真级别
	_player->sec_level = pInfo->sec_level;

	pImp->SetReputation(pInfo->reputation);

	//初始化技能 
	if(data->skill_data.data)
	{
		try
		{
			raw_wrapper ar(data->skill_data.data,data->skill_data.size);
			pImp->_skill.LoadDatabase(ar);
		}catch(...)
		{
			GLog::log(GLOG_ERR,"用户%d技能装载出错",_player->ID.id);
		}
	}
	pImp->_skill.EventReset(pImp);
	pImp->_skill.EventEnter(pImp,world_manager::GetWorldTag());

	//初始化物品
	pImp->GetInventory().SetSize(pInfo->bagsize);
	pImp->GetInventory().InitFromDBData(data->inventory);
	pImp->GetEquipInventory().InitFromDBData(data->equipment);
	pImp->GetTaskInventory().InitFromDBData(data->task_inventory);
	int internal_trashbox_size = player_var_data::GetTrashBoxSize(data->var_data.data,data->var_data.size);
	int db_trashbox_size = pInfo->storesize;
	if(db_trashbox_size >=TRASHBOX_BASE_SIZE && db_trashbox_size <= TRASHBOX_MAX_SIZE)
	{
		//如果数据库的仓库大小有效，则使用之
		internal_trashbox_size = db_trashbox_size;
	}
	pImp->_trashbox.SetTrashBoxSize(internal_trashbox_size);
	pImp->_trashbox.SetTrashBoxSize2(pInfo->storesize1);
	pImp->_trashbox.SetTrashBoxSize3(pInfo->storesize2);
	pImp->_trashbox.SetTrashBoxSize4(pInfo->storesize3);
	if(pInfo->trashbox_active)
	{
		pImp->GetTrashInventory(gplayer_imp::IL_TRASH_BOX).InitFromDBData(data->trash_box);
		pImp->GetTrashInventory(gplayer_imp::IL_TRASH_BOX2).InitFromDBData(data->trash_box1);
		pImp->GetTrashInventory(gplayer_imp::IL_TRASH_BOX3).InitFromDBData(data->trash_box2);
		pImp->GetTrashInventory(gplayer_imp::IL_TRASH_BOX4).InitFromDBData(data->trash_box3);
		pImp->_trashbox.GetMoney() = pInfo->trash_money;
		pImp->ClearTrahsBoxWriteFlag();
	}
	if(data->trashbox_passwd.data)
	{
		pImp->_trashbox.SetPasswordMD5((const char *)data->trashbox_passwd.data, data->trashbox_passwd.size);
	}
	//帐号仓库暂时没有材料仓库/时装仓库/密码
	//帐号仓库存储的userstoresize是可操作格子数,通过合服操作,仓库中的物品数量最高可到2*userstoresize
	pImp->_user_trashbox.SetTrashBoxSize(pInfo->userstoresize * 2);
	if(pInfo->userstore_active)
	{
		pImp->GetTrashInventory(gplayer_imp::IL_USER_TRASH_BOX).InitFromDBData(data->user_store);
		if(pInfo->userstoremoney > TRASHBOX_MONEY_CAPACITY || pInfo->userstoremoney < 0)
		{
			GLog::log(GLOG_ERR,"用户%d帐号仓库金钱%d超过允许范围,重置为0",_player->ID.id, pInfo->userstoremoney);
			pImp->_user_trashbox.GetMoney() = 0;
		}
		else
			pImp->_user_trashbox.GetMoney() = pInfo->userstoremoney;
		pImp->ClearUserTrahsBoxWriteFlag();
	}
	
	//初始化任务
	size_t buf_size = data->task_data.size;
	if(buf_size > pImp->_active_task_list.size()) buf_size = pImp->_active_task_list.size();
	memcpy(pImp->_active_task_list.begin(),data->task_data.data,buf_size);
	
	buf_size = data->finished_task_data.size;
	if(buf_size > pImp->_finished_task_list.size()) buf_size = pImp->_finished_task_list.size();
	memcpy(pImp->_finished_task_list.begin(),data->finished_task_data.data,buf_size);

	buf_size = data->finished_time_task_data.size;
	if(buf_size > pImp->_finished_time_task_list.size()) buf_size = pImp->_finished_time_task_list.size();
	memcpy(pImp->_finished_time_task_list.begin(),data->finished_time_task_data.data,buf_size);

	buf_size = data->task_counter.size;
	if(buf_size > pImp->_finish_task_count_list.size()) buf_size = pImp->_finish_task_count_list.size();
	memcpy(pImp->_finish_task_count_list.begin(),data->task_counter.data,buf_size);

	buf_size = data->storage_task.size;
	if(buf_size > pImp->_storage_task_list.size()) buf_size = pImp->_storage_task_list.size();
	memcpy(pImp->_storage_task_list.begin(),data->storage_task.data,buf_size);

	PlayerTaskInterface task_if(pImp);
	task_if.InitActiveTaskList();

	//初始化声望线
	if(data->npc_relation.data && data->npc_relation.size)
	{
		try{
			raw_wrapper rr_ar(data->npc_relation.data, data->npc_relation.size);	
			size_t rr_size;
			rr_ar >> rr_size;
			if(rr_size == ROLE_REPUTATION_UCHAR_SIZE)
			{
				ASSERT(pImp->_role_reputation_uchar.size() == ROLE_REPUTATION_UCHAR_SIZE);
				rr_ar.pop_back(pImp->_role_reputation_uchar.begin(),ROLE_REPUTATION_UCHAR_SIZE*sizeof(unsigned char));	
			}
			else if(rr_size < ROLE_REPUTATION_UCHAR_SIZE)
			{
				//这种情况是增加声望数据造成的,要进行处理	
				throw -1;
			}
			else if(rr_size > ROLE_REPUTATION_UCHAR_SIZE)
			{
				//这种情况肯定是出现了错误
				throw -2;
			}
			rr_ar >> rr_size;
			if(rr_size == ROLE_REPUTATION_USHORT_SIZE)
			{
				ASSERT(pImp->_role_reputation_ushort.size() == ROLE_REPUTATION_USHORT_SIZE);
				rr_ar.pop_back(pImp->_role_reputation_ushort.begin(),ROLE_REPUTATION_USHORT_SIZE*sizeof(unsigned short));	
			}
			else if(rr_size < ROLE_REPUTATION_USHORT_SIZE)
			{
				//这种情况是增加声望数据造成的,要进行处理	
				throw -3;
			}
			else if(rr_size > ROLE_REPUTATION_USHORT_SIZE)
			{
				//这种情况肯定是出现了错误
				throw -4;
			}
			rr_ar >> rr_size;
			if(rr_size == ROLE_REPUTATION_UINT_SIZE)
			{
				ASSERT(pImp->_role_reputation_uint.size() == ROLE_REPUTATION_UINT_SIZE);
				rr_ar.pop_back(pImp->_role_reputation_uint.begin(),ROLE_REPUTATION_UINT_SIZE*sizeof(unsigned int));	
			}
			else if(rr_size < ROLE_REPUTATION_UINT_SIZE)
			{
				//这种情况是增加声望数据造成的,要进行处理	
				throw -5;
			}
			else if(rr_size > ROLE_REPUTATION_UINT_SIZE)
			{
				//这种情况肯定是出现了错误
				throw -6;
			}
		}catch(...)
		{
			GLog::log(GLOG_ERR,"用户%d声望线数据装载出错",_player->ID.id);
		}
	}

	//重新装载需要存盘的filter
	if(data->filter_data.size)
	{
		raw_wrapper ar(data->filter_data.data,data->filter_data.size);
		pImp->_filters.Load(ar);
		pImp->_filters.DBInit(pImp);
	}

	//装载多倍经验数据
	if(data->multi_exp_ctrl.data && data->multi_exp_ctrl.size)
	{
		raw_wrapper mec_ar(data->multi_exp_ctrl.data, data->multi_exp_ctrl.size);
		pImp->_multi_exp_ctrl.DBLoadData(mec_ar);			
	}

	//帮派贡献度
	if(data->faction_contrib.data && data->faction_contrib.size)
	{
		if(data->faction_contrib.size == sizeof(pImp->_faction_contrib))
		{
			memcpy(&pImp->_faction_contrib,data->faction_contrib.data,sizeof(pImp->_faction_contrib));	
		}
		else
		{
			ASSERT(false);
		}
		//如果没有帮派就清空贡献度
		if(!_player->id_mafia) 
		{
			memset(&pImp->_faction_contrib,0,sizeof(pImp->_faction_contrib));
		}
	}

	//收益时间数据
	raw_wrapper ar(data->profit_time_data.data, data->profit_time_data.size);
	pImp->LoadProfitTime(ar);
	pImp->CalcProfitLevel();

	//帮派外交数据
	pImp->UpdateFactionRelation(data->faction_relation.alliance_list, data->faction_relation.alliance_count, data->faction_relation.hostile_list, data->faction_relation.hostile_count,false);
	
	//玩家势力数据
	pImp->_player_force.InitFromDBData(data->force_data);
	
	//玩家经脉数据
	pImp->_meridianman.InitFromDBData(data->meridian);
	pImp->_meridianman.InitEnhance(pImp);
	//玩家在线奖励数据
	if(data->online_award.data && data->online_award.size)
	{
		raw_wrapper oa_ar(data->online_award.data, data->online_award.size);
		pImp->_online_award.DBLoadData(oa_ar);
	}

	//国家数据
	pImp->SetCountryData(pInfo->country_id, pInfo->country_expire_time);
	//国王数据
	pImp->SetKingData(pInfo->is_king, pInfo->king_expire_time);
	
	//touch 交易
	pImp->SetDBTouchTrade(pInfo->touch_trade.sn,pInfo->touch_trade.state,pInfo->touch_trade.cost,
			pInfo->touch_trade.itemtype,pInfo->touch_trade.itemcount,pInfo->touch_trade.itemexpire,pInfo->touch_trade.lots);

	//title 称号
	raw_wrapper title_ar(data->title_data.data, data->title_data.size);
	pImp->InitDBTitleData(title_ar);
	pImp->InitDBTitleEnhance();

	//每日签到
	pImp->SetDBDailySignin(pInfo->daily_signin.update_time, pInfo->daily_signin.month_calendar, 
			pInfo->daily_signin.curr_year_data, pInfo->daily_signin.last_year_data, pInfo->daily_signin.awarded_times, pInfo->daily_signin.late_signin_times);
	//礼品卡
	pImp->SetDBGiftCard(pInfo->giftcard_redeem.state, pInfo->giftcard_redeem.type,
			pInfo->giftcard_redeem.parenttype, pInfo->giftcard_redeem.cardnumber);

	//命轮
	raw_wrapper fatering_data(data->fatering_data.data, data->fatering_data.size);
	pImp->InitDBFateRingData(fatering_data);
	pImp->InitDBFateRingEnhance();

	//境界
	pImp->SetDBRealmData(data->realm.level,data->realm.exp);	

	pImp->SetRankData(data->rank.point, data->rank.kill, data->rank.dead);

	//卡牌图鉴
	if(data->generalcard_collection.data && data->generalcard_collection.size)
	{
		pImp->_generalcard_collection.init((unsigned char *)data->generalcard_collection.data, data->generalcard_collection.size);
	}

	//时间重置管理
	raw_wrapper clockdata(data->clock_data.data, data->clock_data.size);
	pImp->SetDBClockData(clockdata);

	//随机商城数据
	raw_wrapper randmalldata(data->rand_mall_data.data, data->rand_mall_data.size);
	pImp->SetDBRandMallData(randmalldata);
	
	pImp->SetDBWorldContrib(pInfo->world_contribution.contrib,pInfo->world_contribution.cost);
	pImp->SetDBAstrolabeExtern(pInfo->astrolabe_extern.level,pInfo->astrolabe_extern.exp);

    pImp->SetDBSoloChallengeInfo(pInfo->solo_challenge_info);
    pImp->SetDBMNFactionInfo(pInfo->mnfaction_info.unifid);
    pImp->SetDBVisaInfo(pInfo->visa_info.type, pInfo->visa_info.stay_timestamp, pInfo->visa_info.cost_item, pInfo->visa_info.cost_item_count);

	raw_wrapper fixpositiontransmitdata(data->fix_position_transmit_data.data, data->fix_position_transmit_data.size);
	pImp->SetDBFixPositionTransmit(fixpositiontransmitdata);

    pImp->SetDBCashResurrectTimesInCoolDown(pInfo->cash_resurrect_info.times);

	pImp->SetDBCashVipInfo(pInfo->vip_level, pInfo->score_add, pInfo->score_cost, pInfo->score_consume);

	pImp->SetDBPurchaseLimitInfo(pInfo->next_day_item_clear_timestamp, pInfo->next_week_item_clear_timestamp, pInfo->next_month_item_clear_timestamp, pInfo->next_year_item_clear_timestamp);

	raw_wrapper purchase_limit_data(data->purchase_limit_data.data, data->purchase_limit_data.size);
	pImp->SetDBPurchaseLimitInfo(purchase_limit_data);

	//重新计算玩家的所有数据
	property_policy::UpdatePlayer(pInfo->cls,pImp);

	//设置玩家防沉迷数据
	pImp->SetWallowData(data->addiction_data.data,data->addiction_data.size); 

	//将玩家放入世界
	_plane->InsertPlayerToMan(_player);
	
	//发送登录成功消息给连接服务器
	GMSV::SendLoginRe(_player->cs_index,_player->ID.id,_player->cs_sid,0,flag);	// login ok
		
	GMSV::SendSynMutaData(_player->ID.id,pInfo->level, pImp->GetReincarnationTimes());
	//现在除了加入网格之外无其他操作
	//用户若干时间内如果仍然不enter 则自动将玩家踢出的操作
	//现在link会在2分钟内进行断线操作
	GLog::log(GLOG_INFO,"用户%d连接成功，等待用户发送enter_world",_player->ID.id);


	//注意，因为还没有进入世界，所以player应该还没有开始心跳和种种其他操作，这些是通过
	//Login state完成了特殊消息的转发而做到的。
}


void user_enter_world(int cs_index,int sid,int uid, int locktime, int max_locktime) //$$$$$$$$$$$$$
{
	GLog::log(GLOG_INFO,"用户%d进入世界",uid);

	int world_index;
	gplayer *pPlayer = world_manager::GetInstance()->FindPlayer(uid,world_index);
	if(!pPlayer)
	{
		GLog::log(GLOG_ERR,"用户%d进入世界时不在服务器内",uid);
		//该用户不在本服务器 这种错误不应该出现，记录日志即可
		return ;
	}
	world * pPlane = world_manager::GetInstance()->GetWorldByIndex(world_index);
	spin_autolock alock(pPlayer->spinlock);
	if(pPlayer->login_state != gplayer::WAITING_ENTER || pPlayer->pPiece
		|| !pPlayer->IsActived() || pPlayer->cs_index != cs_index 
		|| pPlayer->cs_sid != sid || pPlayer->ID.id != uid || !pPlayer->imp)
	{
		//处于错误的状态，发送日至
		GLog::log(GLOG_WARNING,"enter_world:用户%d已经不在服务器里了",uid);
		return;
	}
	if(pPlayer->imp->_plane != pPlane) 
	{
		GLog::log(GLOG_WARNING,"enter_world:用户%d已经不在服务器里了",uid);
		return ;
	}
	//将player放入世界
	pPlane->InsertPlayer(pPlayer);
	MSG msg;
	BuildMessage(msg,GM_MSG_ENTER_WORLD,pPlayer->ID,pPlayer->ID,pPlayer->pos, locktime, &max_locktime, sizeof(max_locktime));
	pPlayer->imp->DispatchMessage(pPlane,msg);
}

void    user_save_data(gplayer * pPlayer,GDB::Result * callback, int priority,int mask)
{
	//应该已经完成锁定
	ASSERT(pPlayer->spinlock);
	if(!pPlayer->IsActived() || 
			pPlayer->login_state == gplayer::WAITING_LOGIN ||
			pPlayer->login_state == gplayer::WAITING_ENTER)
	{
		GLog::log(GLOG_WARNING,"save_data:用户已经不再服务器里了,或者本次登陆尚未完成");
		return ;
	}

	gplayer_imp * pImp = (gplayer_imp *)pPlayer->imp;

	GDB::base_info info;
	GDB::vecdata data;
	memset(&info,0,sizeof(info));
	memset(&data,0,sizeof(data));
	info.id = pPlayer->ID.id;
	info.race = 0;
	
	pImp->GetBasicData(info.level,info.sec_level,info.exp,info.sp,info.hp,info.mp,info.pp,info.money,info.userid);
	pImp->GetPlayerClass(info.cls,info.gender);
	int tag = 1;
	A3DVECTOR pos;
	world_manager::GetInstance()->GetLogoutPos(pImp,tag,pos);
	info.worldtag = tag;
	info.posx = pos.x;
	info.posy = pos.y;
	info.posz = pos.z;
	info.trashbox_active = pImp->IsTrashBoxChanged();  //设置是否保存仓库数据
	info.trash_money = pImp->_trashbox.GetMoney();
	info.factionid = pPlayer->id_mafia;
	info.factionrole = pPlayer->rank_mafia;
	info.reputation = pImp->GetReputation();
	pImp->GetDBTouchTrade(info.touch_trade.sn,info.touch_trade.state,info.touch_trade.cost,
			info.touch_trade.itemtype,info.touch_trade.itemcount,info.touch_trade.itemexpire,info.touch_trade.lots);

	pImp->GetInvadeState(info.invader_state, info.invader_time, info.pariah_time);
	

	//每日签到
	pImp->GetDBDailySignin(info.daily_signin.update_time, info.daily_signin.month_calendar, 
			info.daily_signin.curr_year_data, info.daily_signin.last_year_data, info.daily_signin.awarded_times, info.daily_signin.late_signin_times);
	//礼品卡
	pImp->GetDBGiftCard(info.giftcard_redeem.state, info.giftcard_redeem.type,
			info.giftcard_redeem.parenttype, info.giftcard_redeem.cardnumber);
	
	//保存玩家双倍卡时
	pImp->CalcRestTime();
      	pImp->GetRestTimeParam(info.dbltime_expire,info.dbltime_mode,info.dbltime_begin,info.dbltime_used,info.dbltime_max);
	raw_wrapper dbltime_data(64);
	pImp->GetExtRestParam(dbltime_data);
	data.dbltime_data.data = dbltime_data.data();
	data.dbltime_data.size = dbltime_data.size();

	//计算玩家游戏时间并存盘
	info.time_used = pImp->GetPlayEd();

	//保存玩家点数
	pImp->GetMallInfo(info.cash, info.cash_used, info.cash_delta, info.cash_serial);
	info.mall_consumption = pImp->GetMallConsumption();
	info.cash_active = pImp->NeedSaveMallInfo();            //察看点卡数据是否改变并且需要存盘
	abase::vector<GDB::shoplog, abase::fast_alloc<> > mall_order_list;
	/*
	if((data.logs.count = pImp->OI_GetMallOrdersCount()))
	{
		mall_order_list.reserve(data.logs.count);
		mall_order_list.insert(mall_order_list.end(), data.logs.count, GDB::shoplog());
		size_t size = pImp->OI_GetMallOrders(&mall_order_list[0], mall_order_list.size());
		ASSERT(size == data.logs.count);
		data.logs.list = &mall_order_list[0];
	}
	*/
	
	int tmp1,tmp2;
	pImp->_dividend_mall_info.GetDividendMallInfo(tmp1, tmp2, info.bonus_used);

	//设置玩家扩展属性 
	data.property.data = &pImp->_base_prop;
	data.property.size = sizeof(pImp->_base_prop);
	
	//设置玩家冷却时间的数据
	pImp->ClearExpiredCoolDown();
	raw_wrapper cooling_data(512);
	pImp->GetCoolDownData(cooling_data);
	data.coolingtime.data = cooling_data.data();
	data.coolingtime.size = cooling_data.size();
		
	//设置包裹栏数据	
	info.bagsize = pImp->GetInventory().Size();
	bool bRst = pImp->GetInventory().MakeDBData(data.inventory);
	ASSERT(bRst);
	bRst = pImp->GetEquipInventory().MakeDBData(data.equipment);
	ASSERT(bRst);
	bRst = pImp->GetTaskInventory().MakeDBData(data.task_inventory);
	ASSERT(bRst);
	info.storesize = pImp->_trashbox.GetTrashBoxSize();
	info.storesize1 = pImp->_trashbox.GetTrashBoxSize2();
	info.storesize2 = pImp->_trashbox.GetTrashBoxSize3();
	info.storesize3 = pImp->_trashbox.GetTrashBoxSize4();
	if(info.trashbox_active)
	{
		bRst = pImp->GetTrashInventory(gplayer_imp::IL_TRASH_BOX).MakeDBData(data.trash_box);
		ASSERT(bRst);
		bRst = pImp->GetTrashInventory(gplayer_imp::IL_TRASH_BOX2).MakeDBData(data.trash_box1);
		ASSERT(bRst);
		bRst = pImp->GetTrashInventory(gplayer_imp::IL_TRASH_BOX3).MakeDBData(data.trash_box2);
		ASSERT(bRst);
		bRst = pImp->GetTrashInventory(gplayer_imp::IL_TRASH_BOX4).MakeDBData(data.trash_box3);
		ASSERT(bRst);
	}
	//帐号仓库暂时没有材料仓库/时装仓库/密码
	//帐号仓库存储的userstoresize是可操作格子数,通过合服操作,仓库中的物品数量最高可到2*userstoresize
	info.userstore_active = pImp->IsUserTrashBoxChanged();
	info.userstoresize = pImp->_user_trashbox.GetTrashBoxSize() / 2;
	if(info.userstore_active)
	{
		info.userstoremoney = pImp->_user_trashbox.GetMoney();
		bool bRst = pImp->GetTrashInventory(gplayer_imp::IL_USER_TRASH_BOX).MakeDBData(data.user_store);
		ASSERT(bRst);
	}

	//保存防沉迷数据
	/*
	void * wallow_d;
	size_t wallow_l = pImp->_wallow_obj.GetData(g_timer.get_systime(), & wallow_d);

	data.addiction_data.data = wallow_d;
	data.addiction_data.size = wallow_l;
	*/

	//设置技能数据
	raw_wrapper ar;
	pImp->_skill.StoreDatabase(ar);
	data.skill_data.data = ar.data();
	data.skill_data.size = ar.size();

	//设置任务数据
	PlayerTaskInterface task_if(pImp);
	task_if.BeforeSaveData();
	data.task_data.data = task_if.GetActiveTaskList();
	data.task_data.size = task_if.GetActLstDataSize();
	data.finished_task_data.data = task_if.GetFinishedTaskList(); 
	data.finished_task_data.size = task_if.GetFnshLstDataSize();
	data.finished_time_task_data.data = task_if.GetFinishedTimeList();
	data.finished_time_task_data.size = task_if.GetFnshTimeLstDataSize();
	data.task_counter.data = task_if.GetFinishedCntList();
	data.task_counter.size = task_if.GetFnshCntLstDataSize();
	data.storage_task.data = task_if.GetStorageTaskList();
	data.storage_task.size = task_if.GetStorageTaskLstDataSize();
	data.waypoint_list.data = pImp->GetWaypointBuffer(data.waypoint_list.size);

	//保存声望线数据
	raw_wrapper rr_ar;
	ASSERT(pImp->_role_reputation_uchar.size() == ROLE_REPUTATION_UCHAR_SIZE);
	rr_ar << pImp->_role_reputation_uchar.size();
	rr_ar.push_back(pImp->_role_reputation_uchar.begin(),pImp->_role_reputation_uchar.size()*sizeof(unsigned char));
	ASSERT(pImp->_role_reputation_ushort.size() == ROLE_REPUTATION_USHORT_SIZE);
	rr_ar << pImp->_role_reputation_ushort.size();
	rr_ar.push_back(pImp->_role_reputation_ushort.begin(),pImp->_role_reputation_ushort.size()*sizeof(unsigned short));
	ASSERT(pImp->_role_reputation_uint.size() == ROLE_REPUTATION_UINT_SIZE);
	rr_ar << pImp->_role_reputation_uint.size();
	rr_ar.push_back(pImp->_role_reputation_uint.begin(),pImp->_role_reputation_uint.size()*sizeof(unsigned int));
	data.npc_relation.data = rr_ar.data();
	data.npc_relation.size = rr_ar.size();

	//设置filter数据
	raw_wrapper filter_data;
	pImp->_filters.SaveSpecFilters(filter_data,filter::FILTER_MASK_SAVE_DB_DATA); 

	data.filter_data.data = filter_data.data();
	data.filter_data.size = filter_data.size();

	//保存多倍经验数据
	raw_wrapper mec_ar;
	pImp->_multi_exp_ctrl.DBSaveData(mec_ar);
	data.multi_exp_ctrl.data = mec_ar.data();
	data.multi_exp_ctrl.size = mec_ar.size();

	//帮派贡献度
	data.faction_contrib.data = &pImp->_faction_contrib;
	data.faction_contrib.size = sizeof(pImp->_faction_contrib);
	
	//玩家势力数据
	pImp->_player_force.MakeDBData(data.force_data);
	
	pImp->_meridianman.MakeDBData(data.meridian);
	//设置收益时间数据
	raw_wrapper profit_time_data;
	pImp->SaveProfitTime(profit_time_data);
	data.profit_time_data.data = profit_time_data.data();
	data.profit_time_data.size = profit_time_data.size();

	//玩家在线奖励数据
	raw_wrapper oa_ar;
	pImp->_online_award.DBSaveData(oa_ar);
	data.online_award.data = oa_ar.data();
	data.online_award.size = oa_ar.size();
	
	//国家数据
	info.country_id = pImp->GetCountryId();
	info.country_expire_time = pImp->GetCountryExpireTime();
	//国王数据
	info.is_king = pPlayer->IsKing() ? 1 : 0;
	info.king_expire_time = pImp->GetKingExpireTime();
	
	//设置自定义数据
	player_var_data var_data;
	var_data.MakeData(pPlayer,pImp);
	data.var_data.data = &var_data;
	data.var_data.size = sizeof(var_data);
	data.trashbox_passwd.data = (void*) pImp->_trashbox.GetPassword(data.trashbox_passwd.size);

	//设置效果数据 这个东西只写盘不用读 因为实际上是给界面上用的
	data.custom_status.data = pPlayer->effect_list;
	data.custom_status.size = pPlayer->effect_count * sizeof(short);

	//设置玩家角色状态
	raw_wrapper char_data;
	pImp->GetPlayerCharMode(char_data);
	data.charactermode.data = char_data.data();
	data.charactermode.size = char_data.size();

	//保存玩家的所有副本数据
	raw_wrapper ins_data;
	pImp->SaveInstanceKey(ins_data);
	data.instancekeylist.data = ins_data.data();
	data.instancekeylist.size = ins_data.size();

	//保存玩家称号数据
	raw_wrapper title_ar;
	pImp->GetDBTitleData(title_ar);
	data.title_data.data = title_ar.data();
	data.title_data.size = title_ar.size();

	//保存玩家命轮数据
	raw_wrapper fatering_data;
	pImp->GetDBFateRingData(fatering_data);
	data.fatering_data.data = fatering_data.data();
	data.fatering_data.size = fatering_data.size();

	//转生
	pImp->_player_reincarnation.MakeDBData(data.reincarnation);

	//境界
	pImp->GetDBRealmData(data.realm.level,data.realm.exp);

	pImp->GetRankData(data.rank.point, data.rank.kill, data.rank.dead);

	//卡牌图鉴
	info.leadership = pImp->GetDBLeadership();
	data.generalcard_collection.data = pImp->_generalcard_collection.data(data.generalcard_collection.size);
	
	//保存玩家的宠物数据
	GDB::pet pet_list[pet_manager::MAX_PET_CAPACITY];
	size_t pet_count = 0;
	for(size_t i = 0; i < pet_manager::MAX_PET_CAPACITY; i ++)
	{
		pet_data * pData = pImp->GetPetData(i);
		if(!pData)  continue;
		pet_list[pet_count].index = i;
		pet_list[pet_count].data.data = pData;
		pet_list[pet_count].data.size = sizeof(pet_data);
		pet_count ++;
	}
	data.pets.capacity = pImp->GetPetSlotCapacity();
	data.pets.count = pet_count;
	data.pets.list = pet_list;
	info.timestamp = pImp->OI_InceaseDBTimeStamp();

	// clock 数据
	raw_wrapper clockdata;
	pImp->GetDBClockData(clockdata);
	data.clock_data.data = clockdata.data();
	data.clock_data.size = clockdata.size();

	//随机商城数据
	raw_wrapper randmalldata;
	pImp->GetDBRandMallData(randmalldata);
	data.rand_mall_data.data = randmalldata.data();
	data.rand_mall_data.size = randmalldata.size();

	pImp->GetDBWorldContrib(info.world_contribution.contrib,info.world_contribution.cost);
	pImp->GetDBAstrolabeExtern(info.astrolabe_extern.level,info.astrolabe_extern.exp);

    pImp->GetDBSoloChallengeInfo(info.solo_challenge_info);
    pImp->GetDBMNFactionInfo(info.mnfaction_info.unifid);
    pImp->GetDBVisaInfo(info.visa_info.type, info.visa_info.stay_timestamp, info.visa_info.cost_item, info.visa_info.cost_item_count);

	raw_wrapper fixpositiontransmitdata;
	pImp->GetDBFixPositionTransmit(fixpositiontransmitdata);
	data.fix_position_transmit_data.data = fixpositiontransmitdata.data();
	data.fix_position_transmit_data.size = fixpositiontransmitdata.size();

    pImp->GetDBCashResurrectTimesInCoolDown(info.cash_resurrect_info.times);

	pImp->GetDBCashVipInfo(info.vip_level, info.score_add, info.score_cost, info.score_consume);

	pImp->GetDBPurchaseLimitData(info.next_day_item_clear_timestamp, info.next_week_item_clear_timestamp, info.next_month_item_clear_timestamp, info.next_year_item_clear_timestamp);
	
	raw_wrapper purchase_limit_data;
	pImp->GetDBPurchaseLimitInfo(purchase_limit_data);
	data.purchase_limit_data.data = purchase_limit_data.data();
	data.purchase_limit_data.size = purchase_limit_data.size();


/*---------------------------发出保存请求------------------------------------*/
	GDB::put_role(info.id,&info,&data,callback,priority,mask);
/*-------------------------发出保存请求完成----------------------------------*/

	//释放包裹栏分配的内存
	pImp->GetInventory().ReleaseDBData(data.inventory);
	pImp->GetEquipInventory().ReleaseDBData(data.equipment);
	pImp->GetTaskInventory().ReleaseDBData(data.task_inventory);
	if(info.trashbox_active)
	{
		pImp->GetTrashInventory(gplayer_imp::IL_TRASH_BOX).ReleaseDBData(data.trash_box);
		pImp->GetTrashInventory(gplayer_imp::IL_TRASH_BOX2).ReleaseDBData(data.trash_box1);
		pImp->GetTrashInventory(gplayer_imp::IL_TRASH_BOX3).ReleaseDBData(data.trash_box2);
		pImp->GetTrashInventory(gplayer_imp::IL_TRASH_BOX4).ReleaseDBData(data.trash_box3);
	}
	if(info.userstore_active)
	{
		pImp->GetTrashInventory(gplayer_imp::IL_USER_TRASH_BOX).ReleaseDBData(data.user_store);
	}
	pImp->_player_force.ReleaseDBData(data.force_data);
	pImp->_player_reincarnation.ReleaseDBData(data.reincarnation);

	//恢复任务数据
	task_if.InitActiveTaskList();

	GLog::log(GLOG_INFO,"save_data:%d发送保存用户%d数据请求 %s包含仓库信息 等级%d 金钱%u sp%d %s包含帐号仓库信息(金钱%d 可操作格子数%d 物品数%d)",
			world_manager::GetWorldTag(),
			pPlayer->ID.id,
			info.trashbox_active?"":"不",
			info.level,
			pImp->GetMoney() + pImp->GetTrashBox().GetMoney(),
			info.sp,
			info.userstore_active?"":"不",
			pImp->_user_trashbox.GetMoney(),
			pImp->_user_trashbox.GetTrashBoxSize()/2,
			pImp->_user_trashbox.GetBackpack1().GetItemCount()
			);
}

void	user_login(int cs_index,int cs_sid,int uid,const void * auth_data, size_t auth_size, bool isshielduser, char flag)
{
	world_manager::GetInstance()->UserLogin(cs_index,cs_sid,uid,auth_data, auth_size, isshielduser, flag);
}

#define MAX_BASE_PP 50

static bool __login_check_data(const GDB::base_info * pInfo, const GDB::vecdata * data)
{
	int level = pInfo->level;
	int pp = pInfo->pp;
	int reincarnation_times = data->reincarnation.count;
	extend_prop prop;
	memset(&prop,0,sizeof(prop));
	if(data->property.data)
	{
		if(data->property.size < sizeof(prop))
		{
			ASSERT(data->property.size == sizeof(prop) - 4);
			memcpy(&prop,data->property.data,data->property.size);
			prop.max_ap = 0;
		}
		else
		{
			ASSERT(data->property.size == sizeof(prop));
			memcpy(&prop,data->property.data,sizeof(prop));
		}

	}
	else
	{
		return false;
	}

	if( (level -1) * 5 + MAX_BASE_PP + player_reincarnation::MaxExtraPPoint()*reincarnation_times < pp + prop.vitality + prop.energy + prop.strength + prop.agility)
	{
		//点数不对
		return false;
	}

	if(!player_template::CheckData(pInfo->cls,level,prop))
	{	
		return false;
	}
	return true;
}

bool 	do_login_check_data(const GDB::base_info * pInfo, const GDB::vecdata * data)
{
	bool bRst = __login_check_data(pInfo,data);
/*	if(!bRst) GLog::log(GLOG_ERR,"[3-23更新]登录时 发现用户%d数据异常，目前不进行处理",pInfo->id);
	return true;
	if(!bRst && player_template::GetDebugMode()) 
	{
		GLog::log(GLOG_ERR,"调试状态下发现用户%d数据异常，此结果被忽略之",pInfo->id);
		//return bRst;
		return true;
		
	}
	*/
	return bRst;
}

