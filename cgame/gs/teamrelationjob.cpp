#include "world.h"
#include "worldmanager.h"
#include "player_imp.h"
#include "teamrelationjob.h"
#include "cooldowncfg.h"
#include <gsp_if.h>

TeamRelationJob::TeamRelationJob(gplayer_imp * pImp, const XID * list, size_t count) : error_msg(0)
{
	_plane = pImp->_plane;
	_leader = pImp->_parent->ID.id;
	_list.reserve(count);
	for(size_t i = 0; i < count ; i ++) 
	{       
		_list.push_back(list[i].id);
	}       
}

bool TeamRelationJob::OnRun()
{
	//首先根据所有人的ID得到所有的pPlayer
	abase::vector<gplayer *, abase::fast_alloc<> > plist;
	abase::vector<int *, abase::fast_alloc<> > llist;
	gplayer * pLeader = NULL; 
	plist.reserve(_list.size());
	llist.reserve(_list.size());
	world_manager * pManager = _plane->GetWorldManager();
	for(size_t i = 0; i < _list.size();i ++)
	{       
		int foo;
		gplayer *pPlayer = pManager->FindPlayer(_list[i],foo);
		if(pPlayer == NULL) return false;       //找不到指定的队友
		plist.push_back(pPlayer);
		llist.push_back(&pPlayer->spinlock);
		if(_list[i] == _leader) pLeader = pPlayer;
	}       
	if(pLeader == NULL) return false;       //找不到发起者（队长）

	std::sort(llist.begin(), llist.end());

	//锁定所有玩家 
	for(size_t i = 0; i < llist.size();i ++)
	{
		mutex_spinlock(llist[i]);
	}

	try
	{
		//检查所有人的状态位置是否正确
		A3DVECTOR pos = pLeader->pos;
		for(size_t i = 0; i < _list.size();i ++)
		{
			gplayer * pPlayer = plist[i];
			if(pPlayer->ID.id != _list[i])  throw -3;
			if(!pPlayer->IsActived() ||
					pPlayer->IsZombie() ||
					pPlayer->login_state != gplayer::LOGIN_OK) throw -4;
			if(pPlayer->pos.squared_distance(pos) > 30.f*30.f) throw -5;
			if(!pPlayer->imp) throw -6;
			if(!pPlayer->imp->CanTeamRelation()) throw -7;
			if(!pPlayer->imp->GetRunTimeClass()->IsDerivedFrom(gplayer_imp::GetClass()))
				throw -8;
		}

		//检查队伍状态是否符合
		gplayer_imp * pLmp = (gplayer_imp *) pLeader->imp;
		if(!pLmp->IsInTeam() || !pLmp->IsTeamLeader()) throw -9;
		for(size_t i = 0; i < _list.size();i ++)
		{
			gplayer * pPlayer = plist[i];
			if(pPlayer == pLeader) continue;
			if(!pLmp->IsMember(pPlayer->ID)) throw -10;
		}

		//对整个队伍进行检查
		if(!OnTeamCheck(plist.begin(), plist.size()))
			throw -11;

		//对队长和队员进行检查
		for(size_t i = 0; i < _list.size();i ++)
		{
			gplayer * pPlayer = plist[i];
			gplayer_imp * pImp = (gplayer_imp *) pPlayer->imp;
			if(pPlayer == pLeader)
			{
				if(!OnLeaderCheck(pImp))
					throw -12;
			}
			else
			{
				if(!OnMemberCheck(pImp))
					throw -13;
			}				
		}

		//最终执行队伍的逻辑关系
		OnExecute(plist.begin(), plist.size());

	}catch(int e)
	{
		__PRINTF("组队关系工作检查失败，错误号:%d\n", e);
		if(error_msg > 0)
		{
			MSG msg;
			BuildMessage(msg, GM_MSG_ERROR_MESSAGE, XID(GM_TYPE_PLAYER,_leader), XID(-1,-1), A3DVECTOR(0,0,0),error_msg);
			_plane->PostLazyMessage(msg);
		}
	}


	//解锁所有玩家
	for(size_t i = 0; i < llist.size();i ++)
	{
		mutex_spinunlock(llist[i]);
	}
	return true;
}

bool WeddingBookJob::OnTeamCheck(gplayer ** list, size_t count)
{
	if(count != 2) return false;
	gplayer_imp* pImp1 = (gplayer_imp*)list[0]->imp;
	gplayer_imp* pImp2 = (gplayer_imp*)list[1]->imp;
	bool female1 = pImp1->IsPlayerFemale();	
	bool female2 = pImp2->IsPlayerFemale();	
	if(female1 == female2) return false;
	bool married1 = pImp1->IsMarried();
	bool married2 = pImp2->IsMarried();
	if(!married1 || !married2) return false;
	if(list[0]->spouse_id != list[1]->ID.id) return false;
	if(list[1]->spouse_id != list[0]->ID.id) return false;
	return true;
}

bool WeddingBookJob::OnLeaderCheck(gplayer_imp * pImp)
{
	if(pImp->GetInventory().GetEmptySlotCount() < 2)
	{
		pImp->_runner->error_message(S2C::ERR_INVENTORY_IS_FULL);
		return false;	
	}
	if(!pImp->CheckItemExist(WEDDING_BOOK_TICKET_ID, 1)) return false;

	int year = 0, month = 0, day = 0;
	if(bookcard_index >= 0)
	{
		item_list & inv = pImp->GetInventory();
		if((size_t)bookcard_index >= inv.Size()) return false;	
		item & it = inv[bookcard_index];
		if(it.type <= 0 || it.body == NULL || it.body->GetItemType() != item_body::ITEM_TYPE_WEDDING_BOOKCARD) return false;
		if(!it.GetBookCardData(year,month,day)) return false;
	}
	if(!world_manager::GetInstance()->WeddingCheckBook(wedding_start_time,wedding_end_time,wedding_scene,year,month,day))
	{
		pImp->_runner->error_message(S2C::ERR_WEDDING_CANNOT_BOOK);
		return false;	
	}
	return true;
}

bool WeddingBookJob::OnMemberCheck(gplayer_imp * pImp)
{
	if(pImp->GetInventory().GetEmptySlotCount() < 1)
	{
		pImp->_runner->error_message(S2C::ERR_INVENTORY_IS_FULL);
		return false;	
	}
	if(!pImp->CheckItemExist(WEDDING_BOOK_TICKET_ID, 1)) return false;
	return true;
}

bool WeddingBookJob::OnExecute(gplayer ** list, size_t count)
{
	ASSERT(count == 2);
	gplayer_imp *pLeader, *pMember;
	if(list[0]->ID.id == _leader)
	{
		pLeader = (gplayer_imp*)list[0]->imp;
		pMember = (gplayer_imp*)list[1]->imp;
	}
	else
	{
		pLeader = (gplayer_imp*)list[1]->imp;
		pMember = (gplayer_imp*)list[0]->imp;
	}
	int groom, bride;
	if(pLeader->IsPlayerFemale())
	{
		groom = pMember->_parent->ID.id;
		bride = pLeader->_parent->ID.id;
	}
	else
	{
		groom = pLeader->_parent->ID.id;
		bride = pMember->_parent->ID.id;
	}
	
	//生成空的结婚者使用的请柬
	element_data::item_tag_t tag = {element_data::IMT_NULL, 0};
	item_data * invitecard_data = world_manager::GetDataMan().generate_item_from_player(WEDDING_INVITECARD_ID1, &tag, sizeof(tag)); 
	if(!invitecard_data)
	{
		GLog::log(GLOG_ERR,"生成结婚者使用的请柬失败");
		return false;
	}
	//请柬于婚礼结婚后24小时消失
	invitecard_data->expire_date = wedding_end_time + 86400;
	//生成准婚证明
	item_data * weddingapprove_data = world_manager::GetDataMan().generate_item_from_player(WEDDING_APPROVE_TICKET_ID, &tag, sizeof(tag)); 
	if(!weddingapprove_data) 
	{
		GLog::log(GLOG_ERR,"生成准婚证明失败");
		FreeItem(invitecard_data);
		return false;
	}
	
	//虽然前面check过了,但是还是可能失败
	if(!world_manager::GetInstance()->WeddingTryBook(wedding_start_time,wedding_end_time,groom,bride,wedding_scene))
	{
		pLeader->_runner->error_message(S2C::ERR_WEDDING_CANNOT_BOOK);
		pMember->_runner->error_message(S2C::ERR_WEDDING_CANNOT_BOOK);
		FreeItem(invitecard_data);
		FreeItem(weddingapprove_data);
		return false;	
	}
	
	item_list & leader_inv = pLeader->GetInventory();
	item_list & member_inv = pMember->GetInventory();
	//队长获得请柬
	int rst = leader_inv.PushInEmpty(0, *invitecard_data, 1);
	if(rst >= 0)
	{
		item & it = leader_inv[rst];
		it.SetInviteCardData(wedding_start_time,wedding_end_time,groom,bride,wedding_scene,pLeader->_parent->ID.id);
		pLeader->_runner->obtain_item(invitecard_data->type, invitecard_data->expire_date, 1, it.count, gplayer_imp::IL_INVENTORY, rst);
	}
	else
	{
		ASSERT(false);
		FreeItem(invitecard_data);
		FreeItem(weddingapprove_data);
		return false;
	}
	//队长获得准婚证明
	rst = leader_inv.PushInEmpty(0, *weddingapprove_data, 1);
	if(rst >= 0)
	{
		item & it = leader_inv[rst];
		pLeader->_runner->obtain_item(weddingapprove_data->type, weddingapprove_data->expire_date, 1, it.count, gplayer_imp::IL_INVENTORY, rst);
	}
	else
	{
		ASSERT(false);
		FreeItem(invitecard_data);
		FreeItem(weddingapprove_data);
		return false;
	}
	//队员获得请柬
	rst = member_inv.PushInEmpty(0, *invitecard_data, 1);
	if(rst >= 0)
	{
		item & it = member_inv[rst];
		it.SetInviteCardData(wedding_start_time,wedding_end_time,groom,bride,wedding_scene,pMember->_parent->ID.id);
		pMember->_runner->obtain_item(invitecard_data->type, invitecard_data->expire_date, 1, it.count, gplayer_imp::IL_INVENTORY, rst);
	}
	else
	{
		ASSERT(false);
		FreeItem(invitecard_data);
		FreeItem(weddingapprove_data);
		return false;
	}
	//队长删婚礼预约道具及特殊日期预约凭证
	pLeader->RemoveItems(WEDDING_BOOK_TICKET_ID, 1, S2C::DROP_TYPE_USE, true);
	if(bookcard_index >= 0)
	{
		item& it = leader_inv[bookcard_index];
		pLeader->UpdateMallConsumptionDestroying(it.type, it.proc_type, 1);
		
		pLeader->_runner->player_drop_item(gplayer_imp::IL_INVENTORY, bookcard_index, leader_inv[bookcard_index].type, 1, S2C::DROP_TYPE_USE);
		leader_inv.DecAmount(bookcard_index, 1);
	}
	//队员删婚礼预约道具
	pMember->RemoveItems(WEDDING_BOOK_TICKET_ID, 1, S2C::DROP_TYPE_USE, true);
	
	pLeader->_runner->wedding_book_success(1);
	pMember->_runner->wedding_book_success(1);
	FreeItem(invitecard_data);
	FreeItem(weddingapprove_data);
	GLog::formatlog("wedding:time(%d) player(%d) book wedding(start_time=%d,end_time=%d,groom=%d,bride=%d)",(int)time(NULL),pLeader->_parent->ID.id,wedding_start_time,wedding_end_time,groom,bride);
	return true;
}

bool WeddingCancelBookJob::OnTeamCheck(gplayer ** list, size_t count)
{
	if(count != 2) return false;
	gplayer_imp* pImp1 = (gplayer_imp*)list[0]->imp;
	gplayer_imp* pImp2 = (gplayer_imp*)list[1]->imp;
	bool female1 = pImp1->IsPlayerFemale();	
	bool female2 = pImp2->IsPlayerFemale();	
	if(female1 == female2) return false;
	int groom, bride;
	if(female1)
	{
		groom = pImp2->_parent->ID.id;
		bride = pImp1->_parent->ID.id;
	}
	else
	{
		groom = pImp1->_parent->ID.id;
		bride = pImp2->_parent->ID.id;
	}
	if(!world_manager::GetInstance()->WeddingCheckCancelBook(wedding_start_time,wedding_end_time,groom,bride,wedding_scene))
	{
		pImp1->_runner->error_message(S2C::ERR_WEDDING_CANNOT_CANCELBOOK);
		pImp2->_runner->error_message(S2C::ERR_WEDDING_CANNOT_CANCELBOOK);
		return false;	
	}
	return true;
}

bool WeddingCancelBookJob::OnLeaderCheck(gplayer_imp * pImp)
{
	if(pImp->GetInventory().GetEmptySlotCount() < 1)
	{
		pImp->_runner->error_message(S2C::ERR_INVENTORY_IS_FULL);
		return false;	
	}
	int self = pImp->_parent->ID.id;
	item_list & inv = pImp->GetInventory();
	int start_time, end_time, groom, bride, scene, invitee;
	int rst = 0;
	while( (rst = inv.Find(rst,WEDDING_INVITECARD_ID1)) >= 0)
	{
		item & it = inv[rst];
		if(it.GetInviteCardData(start_time, end_time, groom, bride, scene, invitee))
		{
			if(start_time == wedding_start_time
					&& end_time == wedding_end_time
					&& scene == wedding_scene
					&& (groom == self || bride == self)
					&& invitee == self)
				break;
		}
		rst++;
	}
	if(rst < 0) return false;
	leader_invitecard_index = rst;
	if(pImp->GetMoney() < WEDDING_CANCELBOOK_FEE)
	{
		pImp->_runner->error_message(S2C::ERR_OUT_OF_FUND);
		return false;	
	}
	return true;
}

bool WeddingCancelBookJob::OnMemberCheck(gplayer_imp * pImp)
{
	if(pImp->GetInventory().GetEmptySlotCount() < 1)
	{
		pImp->_runner->error_message(S2C::ERR_INVENTORY_IS_FULL);
		return false;	
	}
	int self = pImp->_parent->ID.id;
	item_list & inv = pImp->GetInventory();
	int start_time, end_time, groom, bride, scene, invitee;
	int rst = 0;
	while( (rst = inv.Find(rst,WEDDING_INVITECARD_ID1)) >= 0)
	{
		item & it = inv[rst];
		if(it.GetInviteCardData(start_time, end_time, groom, bride, scene, invitee))
		{
			if(start_time == wedding_start_time
					&& end_time == wedding_end_time
					&& scene == wedding_scene
					&& (groom == self || bride == self)
					&& invitee == self)
				break;
		}
		rst++;
	}
	if(rst < 0) return false;
	member_invitecard_index = rst;
	return true;
}

bool WeddingCancelBookJob::OnExecute(gplayer ** list, size_t count)
{
	ASSERT(count == 2);
	gplayer_imp *pLeader, *pMember;
	if(list[0]->ID.id == _leader)
	{
		pLeader = (gplayer_imp*)list[0]->imp;
		pMember = (gplayer_imp*)list[1]->imp;
	}
	else
	{
		pLeader = (gplayer_imp*)list[1]->imp;
		pMember = (gplayer_imp*)list[0]->imp;
	}
	int groom, bride;
	if(pLeader->IsPlayerFemale())
	{
		groom = pMember->_parent->ID.id;
		bride = pLeader->_parent->ID.id;
	}
	else
	{
		groom = pLeader->_parent->ID.id;
		bride = pMember->_parent->ID.id;
	}
	
	//生成需要返还给玩家的预约婚礼使用的道具
	element_data::item_tag_t tag = {element_data::IMT_NULL, 0};
	item_data * weddingbook_data = world_manager::GetDataMan().generate_item_from_player(WEDDING_BOOK_TICKET_ID, &tag, sizeof(tag)); 
	if(!weddingbook_data)
	{
		GLog::log(GLOG_ERR,"生成预约婚礼使用的道具失败");
		return false;
	}
	if(weddingbook_data->pile_limit == 1)
		weddingbook_data->expire_date = g_timer.get_systime() + 86400;

	//虽然前面check过了,但是还是可能失败
	if(!world_manager::GetInstance()->WeddingTryCancelBook(wedding_start_time,wedding_end_time,groom,bride,wedding_scene))
	{
		pLeader->_runner->error_message(S2C::ERR_WEDDING_CANNOT_CANCELBOOK);
		pMember->_runner->error_message(S2C::ERR_WEDDING_CANNOT_CANCELBOOK);
		FreeItem(weddingbook_data);
		return false;	
	}

	item_list & leader_inv = pLeader->GetInventory();
	item_list & member_inv = pMember->GetInventory();
	//队长返还预约婚礼使用的道具
	int rst = leader_inv.PushInEmpty(0, *weddingbook_data, 1);
	if(rst >= 0)
	{
		item & it = leader_inv[rst];
		pLeader->_runner->obtain_item(weddingbook_data->type, weddingbook_data->expire_date, 1, it.count, gplayer_imp::IL_INVENTORY, rst);
	}
	else
	{
		ASSERT(false);
		FreeItem(weddingbook_data);
		return false;
	}
	//队员返还预约婚礼使用的道具
	rst = member_inv.PushInEmpty(0, *weddingbook_data, 1);
	if(rst >= 0)
	{
		item & it = member_inv[rst];
		pMember->_runner->obtain_item(weddingbook_data->type, weddingbook_data->expire_date, 1, it.count, gplayer_imp::IL_INVENTORY, rst);
	}
	else
	{
		ASSERT(false);
		FreeItem(weddingbook_data);
		return false;
	}
	//队长删准婚证明及请柬及钱
	item& leader_invitecard = leader_inv[leader_invitecard_index];
	pLeader->UpdateMallConsumptionDestroying(leader_invitecard.type, leader_invitecard.proc_type, 1);
	pLeader->_runner->player_drop_item(gplayer_imp::IL_INVENTORY, leader_invitecard_index, leader_inv[leader_invitecard_index].type, 1, S2C::DROP_TYPE_USE);
	leader_inv.DecAmount(leader_invitecard_index,1);
	pLeader->SpendMoney(WEDDING_CANCELBOOK_FEE);
	pLeader->_runner->spend_money(WEDDING_CANCELBOOK_FEE);
	//队员删请柬
	item& member_invitecard = member_inv[member_invitecard_index];
	pMember->UpdateMallConsumptionDestroying(member_invitecard.type, member_invitecard.proc_type, 1);
	pMember->_runner->player_drop_item(gplayer_imp::IL_INVENTORY, member_invitecard_index, member_inv[member_invitecard_index].type, 1, S2C::DROP_TYPE_USE);
	member_inv.DecAmount(member_invitecard_index,1);

	pLeader->_runner->wedding_book_success(2);
	pMember->_runner->wedding_book_success(2);
	FreeItem(weddingbook_data);
	GLog::formatlog("wedding:time(%d) player(%d) cancel book wedding(start_time=%d,end_time=%d,groom=%d,bride=%d)",(int)time(NULL),pLeader->_parent->ID.id,wedding_start_time,wedding_end_time,groom,bride);
	return true;
}

bool CountryJoinApplyJob::OnTeamCheck(gplayer ** list, size_t count)
{
	return true;
}

bool CountryJoinApplyJob::OnLeaderCheck(gplayer_imp * pImp)
{
	//和队员检查条件一样
	return OnMemberCheck(pImp);
}

bool CountryJoinApplyJob::OnMemberCheck(gplayer_imp * pImp)
{
	if(!pImp->CheckCoolDown(COOLDOWN_INDEX_COUNTRY_JOIN_APPLY))
	{
		SetErrorMsg(S2C::ERR_OBJECT_IS_COOLING); 
		return false; 
	}
	//检查是否已经属于某个国家
	if(pImp->GetCountryId())
	{
		SetErrorMsg(S2C::ERR_ALREADY_JOIN_COUNTRY);
		return false;
	}
	if(pImp->_basic.level < level) return false;
	if(!pImp->IsItemExist(ticket))
	{
		SetErrorMsg(S2C::ERR_ITEM_NOT_IN_INVENTORY);
		return false;
	}
	return true;
}

bool CountryJoinApplyJob::OnExecute(gplayer ** list, size_t count)
{
	GMSV::CBApplyEntry entry_list[count];

	for(size_t i=0; i<count; i++)
	{
		gplayer_imp * pImp = (gplayer_imp*)(list[i]->imp);
		pImp->SetCoolDown(COOLDOWN_INDEX_COUNTRY_JOIN_APPLY,COUNTRY_JOIN_APPLY_COOLDOWN_TIME);

		entry_list[i].roleid = pImp->_parent->ID.id;
		entry_list[i].major_strength = pImp->Get16Por9JWeapon();
		entry_list[i].minor_strength = pImp->GetSoulPower();
	}
	GMSV::CountryBattleApply(entry_list, count);
	return true;
}

