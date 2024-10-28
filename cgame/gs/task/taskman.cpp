#include "taskman.h"
#include "../world.h"
#include "../player_imp.h"
#include "../template/itemdataman.h"
#include "../antiwallow.h"
#include <arandomgen.h>
#include <gsp_if.h>
#include "../public_quest.h"
#include "../global_controller.h"

static int quest_timer = 0;
static void QuestTimerRoutine(int index,void *object,int remain)
{
	quest_timer ++;
	if(quest_timer > 12)
	{
		OnTaskCheckAllTimeLimits(g_timer.get_systime());
		quest_timer = 0; 
	}
}

bool InitQuestSystem(const char * filename,const char * filename2,elementdataman * pMan)
{
	bool bRst = LoadTasksFromPack(pMan,filename,filename2);
	if(!bRst) return false;
	g_timer.set_timer(5*20,10,0,QuestTimerRoutine,NULL);
	return true;
}

static int pq_tick_counter = 0;
void PublicQuestRunTick()
{
	if(++pq_tick_counter >= 100)	//5秒做一次heartbeat,这个时间不能随意改动
	{
		pq_tick_counter = 0;
		world_manager::GetPublicQuestMan().Heartbeat(5);
		OnTaskPublicQuestHeartbeat();
	}
}

unsigned long 
PlayerTaskInterface::GetPlayerLevel()
{
	return _imp->_basic.level;
}

void* 
PlayerTaskInterface::GetActiveTaskList()
{
	return _imp->_active_task_list.begin();
}

void* 
PlayerTaskInterface::GetFinishedTaskList()
{
	return _imp->_finished_task_list.begin();
}

void*
PlayerTaskInterface::GetFinishedTimeList()
{
	return _imp->_finished_time_task_list.begin();
}

void*
PlayerTaskInterface::GetFinishedCntList()
{
	return _imp->_finish_task_count_list.begin();
}

void* 
PlayerTaskInterface::GetStorageTaskList()
{
	return _imp->_storage_task_list.begin();
}

unsigned long* 
PlayerTaskInterface::GetTaskMask()
{
	return &_imp->_task_mask;
}

void 
PlayerTaskInterface::DeliverGold(unsigned long ulGoldNum)
{
	int gold = ulGoldNum;
	if(world_manager::AntiWallow())
	{
		anti_wallow::AdjustTaskMoney(_imp->_wallow_level, gold);
	}
	_imp->GainMoneyWithDrop(gold);
	_imp->_runner->task_deliver_money((int)gold,_imp->GetMoney());
}

void 
PlayerTaskInterface::DeliverExperience(unsigned long ulExp)
{
	int exp = ulExp & 0x3FFFFFFF;
	int sp = 0;
	if(world_manager::AntiWallow())
	{
		anti_wallow::AdjustTaskExpSP(_imp->_wallow_level, exp, sp);
	}
	_imp->ReceiveTaskExp(exp, sp);
}

void 
PlayerTaskInterface::DeliverSP(unsigned long ulSP)
{
	int exp = 0;
	int sp = ulSP&0x3FFFFFFF;
	if(world_manager::AntiWallow())
	{
		anti_wallow::AdjustTaskExpSP(_imp->_wallow_level, exp, sp);
	}
	_imp->ReceiveTaskExp(exp, sp);
}

void 
PlayerTaskInterface::DeliverReputation(long lReputation)
{
	int rep = _imp->GetReputation();
	rep += lReputation;
	_imp->SetReputation(rep);
	_imp->_runner->task_deliver_reputaion((int)lReputation,rep);
}


int 
PlayerTaskInterface::GetTaskItemCount(unsigned long ulTaskItem)
{
	item_list & inv = _imp->GetTaskInventory();
	int rst;
	if((rst = inv.Find(0,ulTaskItem)) >=0)
	{
		//是不是应该累计所有位置的数目？
		return inv[rst].count;
	}
	return 0;
}

int 
PlayerTaskInterface::GetCommonItemCount(unsigned long ulCommonItem)
{
	item_list & inv = _imp->GetInventory();
	int rst = 0;
	int count = 0;
	while((rst = inv.Find(rst,ulCommonItem)) >=0)
	{
		//是不是应该累计所有位置的数目？
		count += inv[rst].count;
		rst ++;
	}
	return count;
}

	
bool 
PlayerTaskInterface::IsInFaction(unsigned long)
{
	return ((gplayer*)_imp->_parent)->id_mafia;
}

int 
PlayerTaskInterface::GetFactionRole()
{
	return ((gplayer*)_imp->_parent)->rank_mafia;
}

unsigned long 
PlayerTaskInterface::GetGoldNum()
{
	return _imp->GetMoney();
}

void 
PlayerTaskInterface::TakeAwayGold(unsigned long ulNum)
{
	_imp->SpendMoney(ulNum);
	_imp->_runner->spend_money(ulNum);
}

void 
PlayerTaskInterface::TakeAwayTaskItem(unsigned long ulTemplId, unsigned long ulNum)
{
	item_list &inv = _imp->GetTaskInventory();
	int rst = inv.Find(0,ulTemplId);
	if(rst >= 0 && inv[rst].count >= ulNum)
	{
		item& it = inv[rst];
		_imp->UpdateMallConsumptionDestroying(it.type, it.proc_type, ulNum);

		inv.DecAmount(rst,ulNum);
		//$$$$发出消息
		_imp->_runner->player_drop_item(gplayer_imp::IL_TASK_INVENTORY,rst,ulTemplId,ulNum,S2C::DROP_TYPE_TASK);
	}
}

void 
PlayerTaskInterface::TakeAwayCommonItem(unsigned long ulTemplId, unsigned long ulNum)
{
	item_list &inv = _imp->GetInventory();
	int rst = 0;
	size_t num = ulNum;
	while(num && (rst = inv.Find(rst,ulTemplId)) >= 0)
	{
		size_t count = num;
		if(inv[rst].count < count) count = inv[rst].count;

		item& it = inv[rst];
		_imp->UpdateMallConsumptionDestroying(it.type, it.proc_type, count);

		inv.DecAmount(rst,count);
		_imp->_runner->player_drop_item(gplayer_imp::IL_INVENTORY,rst,ulTemplId,count,S2C::DROP_TYPE_TASK);
		num -= count;
		rst ++;
	}
}

unsigned long 
TaskInterface::GetCurTime()
{
	return g_timer.get_systime();
}

void
TaskInterface::WriteLog(int nPlayerId, int nTaskId, int nType, const char* szLog)
{
	return GLog::tasklog(nPlayerId,nTaskId,nType,szLog);
}

void
TaskInterface::WriteKeyLog(int nPlayerId, int nTaskId, int nType, const char* szLog)
{
	return GLog::task(nPlayerId,nTaskId,nType,szLog);
}

long 
PlayerTaskInterface::GetReputation()
{
	return _imp->GetReputation();
}

unsigned long 
PlayerTaskInterface::GetCurPeriod()
{
	return _imp->_basic.sec_level;
}

void 
PlayerTaskInterface::SetCurPeriod(unsigned long per)
{
	_imp->SetSecLevel(per);
}

unsigned long 
PlayerTaskInterface::GetPlayerId()
{
	return _imp->_parent->ID.id;
}

unsigned long 
PlayerTaskInterface::GetPlayerRace()
{
	return _imp->GetPlayerClass();
}

unsigned long 
PlayerTaskInterface::GetPlayerOccupation()
{
	return _imp->GetPlayerClass();
}

bool 
PlayerTaskInterface::CanDeliverCommonItem(unsigned long ulItemTypes)
{
	return _imp->GetInventory().GetEmptySlotCount() >= ulItemTypes;
}

void 
PlayerTaskInterface::DeliverCommonItem(unsigned long ulItem,unsigned long count, long lPeriod)
{
	if(count == 0) return;
	element_data::item_tag_t tag = {element_data::IMT_CREATE,0}; 
	item_data * pData = world_manager::GetDataMan().generate_item_for_drop(ulItem,&tag,sizeof(tag));
	if(pData)
	{
		if(count > pData->pile_limit) 
			pData->count = pData->pile_limit;
		else
			pData->count = count;
		if(lPeriod > 0 && pData->pile_limit == 1)
		{
			pData->expire_date = g_timer.get_systime() + lPeriod;
		}
		//返回false表明没有全部放入
		if(_imp->ObtainItem(gplayer_imp::IL_INVENTORY,pData,true)) FreeItem(pData);
	}
	else
	{
		//出错，打印错误
	}
	world_manager::TestCashItemGenerated(ulItem, count);
}

bool 
PlayerTaskInterface::CanDeliverTaskItem(unsigned long ulItemTypes)
{
	return _imp->GetTaskInventory().GetEmptySlotCount() >= ulItemTypes;
}

void 
PlayerTaskInterface::DeliverTaskItem(unsigned long ulItem,unsigned long count)
{
	if(count == 0) return;
	element_data::item_tag_t tag = {element_data::IMT_CREATE,0}; 
	//$$$$$$$ 这里是否可以用卖出方式完成?
	item_data * pData = world_manager::GetDataMan().generate_item_for_shop(ulItem,&tag,sizeof(tag));
	if(pData)
	{
		if(count > pData->pile_limit) 
			pData->count = pData->pile_limit;
		else
			pData->count = count;
		//返回false表明没有全部放入
		if(_imp->ObtainItem(gplayer_imp::IL_TASK_INVENTORY,pData,true)) FreeItem(pData);
	}
	else
	{
		//出错，打印错误
	}
}

void 
PlayerTaskInterface::NotifyClient(const void* pBuf, size_t sz)
{
	_imp->_runner->send_task_var_data(pBuf,sz);
}

float 
PlayerTaskInterface::UnitRand()
{
	return abase::Rand(0.f,1.f);
}

int 
PlayerTaskInterface::RandNormal(int low, int high)
{
	return abase::RandNormal(low,high);
}

int  
PlayerTaskInterface::RandSelect(const float * option, int size)
{
	return abase::RandSelect(option,size);
}

int 
PlayerTaskInterface::GetTeamMemberNum()
{
	return _imp->GetTeamMemberNum();
}

void 
PlayerTaskInterface::NotifyPlayer(unsigned long ulPlayerId, const void* pBuf, size_t sz)
{
	XID id(GM_TYPE_PLAYER,ulPlayerId);
	_imp->SendTo<0>(GM_MSG_PLAYER_TASK_TRANSFER,id,0,pBuf,sz);
}

void 
PlayerTaskInterface::GetTeamMemberInfo(int nIndex, task_team_member_info* pInfo)
{
	const player_team::member_entry &ent = _imp->GetTeamMember(nIndex);
	pInfo->m_ulId = ent.id.id;
	pInfo->m_ulLevel = ent.data.level;
	pInfo->m_ulOccupation = ent.race & 0x7FFFFFFF;
	pInfo->m_bMale = !(ent.race & 0x80000000);
	pInfo->m_Pos[0] = ent.pos.x;
	pInfo->m_Pos[1] = ent.pos.y;
	pInfo->m_Pos[2] = ent.pos.z;
	pInfo->m_ulWorldTag = ent.data.world_tag;
	pInfo->m_ulWorldIndex = ent.data.plane_index;
	pInfo->m_iForce = ent.data.force_id;
	__PRINTF("MemberInfo index:%d id:%d pos:(%f,%f,%f)\n",nIndex,ent.id.id,ent.pos.x,ent.pos.y,ent.pos.z);
}

unsigned long  
PlayerTaskInterface::GetTeamMemberPos(int nIndex, float pos[3])
{
	const player_team::member_entry &ent = _imp->GetTeamMember(nIndex);
	pos[0] = ent.pos.x;
	pos[1] = ent.pos.y;
	pos[2] = ent.pos.z;
	return ent.data.world_tag;
}


bool PlayerTaskInterface::IsDeliverLegal()
{
	return _imp->IsDeliverLegal();
}

bool PlayerTaskInterface::IsCaptain()
{
	return _imp->IsTeamLeader();
}

bool PlayerTaskInterface::IsInTeam()
{
	return _imp->IsInTeam();
}


unsigned long PlayerTaskInterface::GetTeamMemberId(int nIndex)
{
	const player_team::member_entry &ent = _imp->GetTeamMember(nIndex);
	return ent.id.id;
}

bool PlayerTaskInterface::IsMale()
{
	return !_imp->IsPlayerFemale();

}

void PlayerTaskInterface::SetInventorySize(long lSize)
{       
	_imp->ChangeInventorySize(lSize);
}       

unsigned long 
PlayerTaskInterface::GetPos(float pos[3])
{
	const A3DVECTOR & selfpos = _imp->_parent->pos;
	pos[0] = selfpos.x;
	pos[1] = selfpos.y;
	pos[2] = selfpos.z;
	return world_manager::GetWorldTag();
}

bool PlayerTaskInterface::HasLivingSkill(unsigned long ulSkill)
{
	return _imp->GetSkillLevel(ulSkill) > 0;
	
}

long PlayerTaskInterface::GetLivingSkillProficiency(unsigned long ulSkill)
{
	return _imp->GetSkillAbility(ulSkill);
}

	
long PlayerTaskInterface::GetLivingSkillLevel(unsigned long ulSkill)
{
	return _imp->GetSkillLevel(ulSkill); 
	
}

void PlayerTaskInterface::SetNewRelayStation(unsigned long ulStationId)
{
	return _imp->ActivateWaypoint(ulStationId & 0xFFFF);
}

void PlayerTaskInterface::SetStorehouseSize(unsigned long ulSize)
{
	((gplayer_dispatcher*)_imp->_runner)->trashbox_capacity_notify(gplayer_imp::IL_TRASH_BOX, ulSize);
	_imp->IncTrashBoxChangeCounter();
	return _imp->_trashbox.SetTrashBoxSize(ulSize);
}

void PlayerTaskInterface::SetStorehouseSize2(unsigned long ulSize)
{
	((gplayer_dispatcher*)_imp->_runner)->trashbox_capacity_notify(gplayer_imp::IL_TRASH_BOX2, ulSize);
	_imp->IncTrashBoxChangeCounter();
	return _imp->_trashbox.SetTrashBoxSize2(ulSize);
}

void PlayerTaskInterface::SetStorehouseSize3(unsigned long ulSize)
{
	((gplayer_dispatcher*)_imp->_runner)->trashbox_capacity_notify(gplayer_imp::IL_TRASH_BOX3, ulSize);
	_imp->IncTrashBoxChangeCounter();
	return _imp->_trashbox.SetTrashBoxSize3(ulSize);
}

void PlayerTaskInterface::SetAccountStorehouseSize(unsigned long ulSize)
{
	ulSize *= 2;
	((gplayer_dispatcher*)_imp->_runner)->trashbox_capacity_notify(gplayer_imp::IL_USER_TRASH_BOX, ulSize);
	_imp->IncUserTrashBoxChangeCounter();
	return _imp->_user_trashbox.SetTrashBoxSize(ulSize);
}

void PlayerTaskInterface::AddDividend(int nDividend)
{
	GMSV::SendTaskReward(_imp->_parent->ID.id, nDividend);
}

void PlayerTaskInterface::SetFuryUpperLimit(unsigned long ulValue)
{
	_imp->SetMaxAP((int)ulValue);
}

void PlayerTaskInterface::TransportTo(unsigned long ulWorldId, const float pos[3], long lController)
{
	_imp->LongJump(A3DVECTOR(pos[0],pos[1],pos[2]),ulWorldId, lController);
}

void PlayerTaskInterface::GetSpecailAwardInfo(special_award* p)
{
	unsigned long param;
	_imp->GetSpecailTaskAward(p->id1, p->id2, param, p->special_mask);
}

void PlayerTaskInterface::SetPetInventorySize(unsigned long ulSize)
{
	_imp->SetPetSlotCapacity(ulSize);
}

bool PlayerTaskInterface::IsGM()
{
	return _imp->CheckGMPrivilege();
}

void PlayerTaskInterface::SetMonsterController(long id, bool bTrigger)
{
	if(id <= 0) return;
	world * pPlane = _imp->_plane;
	
	//先来个简单实现
	if(bTrigger)
	{
		pPlane->TriggerSpawn(id);
	}
	else
	{
		pPlane->ClearSpawn(id);
	}
}


// 更新任务全局数据
void TaskUpdateGlobalData( unsigned long ulTaskId, const unsigned char pData[TASK_GLOBAL_DATA_SIZE])
{
	GMSV::SetTaskData((int)ulTaskId,pData,TASK_GLOBAL_DATA_SIZE);
}

void TaskQueryGlobalData( unsigned long ulTaskId, unsigned long ulPlayerId, const void* pPreservedData, size_t size)
{
	GMSV::GetTaskData((int)ulTaskId, (int)ulPlayerId, pPreservedData,size);
}

bool PlayerTaskInterface::IsMarried()
{
	return _imp->IsMarried();
}

bool PlayerTaskInterface::IsWeddingOwner()
{
	return _imp->_filters.IsFilterExist(FILTER_INDEX_WEDDING) && world_manager::GetInstance()->WeddingCheckOngoing(_imp->_parent->ID.id);	
}

void PlayerTaskInterface::CheckTeamRelationship(int nReason)
{
	return _imp->DoTeamRelationTask(nReason);
}

bool PlayerTaskInterface::HasIconStateID(unsigned long ulIconStateID)
{
	return _imp->IsExistTeamVisibleState(ulIconStateID);
}

void PlayerTaskInterface::OnTowerTaskDeliver(bool bSuccess)
{
	_imp->PlayerSoloChallengeStartTask(bSuccess);
}

void PlayerTaskInterface::OnTowerTaskComplete(bool bSuccess)
{
	_imp->PlayerSoloChallengeStageComplete(bSuccess);
}

void PlayerTaskInterface::DeliverSoloTowerChallengeScore(int score)
{
	_imp->PlayerDeliverSoloChallengeScore(score);
}

int PlayerTaskInterface::GetVIPLevel()
{
	return _imp->GetCashVipLevel();
}

void
PlayerTaskTeamInterface::SetMarriage(int nPlayer)
{
	marriage_op = 1;
	if(couple[0] == 0)
	{
		couple[0] = nPlayer;
	}
	else if(couple[1] == 0)
	{
		couple[1] = nPlayer;
	}
}

void PlayerTaskTeamInterface::Execute(gplayer ** list, size_t count)
{
	//$$$$$$$$$$日志        
	if(count != 2) return;
	gplayer_imp * pImp1 = (gplayer_imp*)list[0]->imp;
	gplayer_imp * pImp2 = (gplayer_imp*)list[1]->imp;
	if((list[0]->ID.id == couple[0] && list[1]->ID.id == couple[1]) ||
			(list[1]->ID.id == couple[0] && list[0]->ID.id == couple[1]))
	{
		//发送消息到数据库和delivery
		GMSV::SetCouple(couple[0],couple[1], 1);
		GDB::set_couple(couple[0],couple[1], 1);
		if(list[0]->ID.id == couple[0])
		{
			pImp1->SetSpouse(couple[1]);
			pImp2->SetSpouse(couple[0]);
		}
		else
		{
			pImp1->SetSpouse(couple[0]);
			pImp2->SetSpouse(couple[1]);
		}
		pImp1->_runner->player_change_spouse(pImp1->GetSpouse());
		pImp2->_runner->player_change_spouse(pImp2->GetSpouse());

		//这里能够存盘的原因是前面检查过了，一定处于normal状态
		ASSERT(pImp1->IsDeliverLegal());
		ASSERT(pImp2->IsDeliverLegal());
		pImp1->ExternSaveDB();
		pImp2->ExternSaveDB();
	}
}

void PlayerTaskInterface::Divorce()
{
	if(!_imp->IsMarried()) return;
	GMSV::SetCouple(_imp->_parent->ID.id, _imp->GetSpouse(),0);
	_imp->SetSpouse(0);
	_imp->_runner->player_change_spouse(_imp->GetSpouse());
	return ;
}

void
PlayerTaskInterface::SendMessage(unsigned long task_id, int channel, int param)
{
	_imp->TaskSendMessage(task_id, channel, param);
}

long PlayerTaskInterface::GetGlobalValue(long lKey)
{
	world * pPlane = _imp->_plane;
	if(!pPlane) return 0;
	return pPlane->GetCommonValue(lKey);
	
}

void PlayerTaskInterface::PutGlobalValue(long lKey, long lValue)
{
	world * pPlane = _imp->_plane;
	if(!pPlane) return;
	pPlane->SetCommonValue(lKey, lValue);
}

void PlayerTaskInterface::ModifyGlobalValue(long lKey, long lValue)
{
	world * pPlane = _imp->_plane;
	if(!pPlane) return;
	pPlane->ModifyCommonValue(lKey, lValue);
}

int PlayerTaskInterface::SummonMonster(int mob_id, int count, int distance, int remain_time, bool die_with_self)
{
	if(count > 20) count = 20;
	npc_template * pTemplate = npc_stubs_manager::Get(mob_id);
	if(!pTemplate) return -1; 
	if(pTemplate->mine_info.is_mine)
		return _imp->SummonMine(mob_id,count,_imp->_parent->ID,distance,remain_time);
	else if(pTemplate->npc_data.is_npc)
		return _imp->SummonNPC(mob_id,count,_imp->_parent->ID,distance,remain_time);
	else
		return _imp->SummonMonster(mob_id,count,_imp->_parent->ID,distance,remain_time,(die_with_self?0x02:0),0);	
}

bool PlayerTaskInterface::IsShieldUser()
{
	gplayer* player = (gplayer*)(_imp->_parent);
	return player->object_state & gactive_object::STATE_SHIELD_USER;
}

void PlayerTaskInterface::SetAwardDeath(bool bDeadWithLoss)
{
	_imp->SendTo<0>(GM_MSG_DEATH, _imp->_parent->ID, bDeadWithLoss?1:2);
	return;	
}

unsigned long PlayerTaskInterface::GetRoleCreateTime()
{
	return _imp->GetCreateTime();
}

unsigned long PlayerTaskInterface::GetRoleLastLoginTime()
{
	return _imp->GetLastLoginTime();
}

unsigned long PlayerTaskInterface::GetAccountTotalCash()
{
	return _imp->GetMallCashAdd();
}
	
unsigned long PlayerTaskInterface::GetSpouseID()
{
	return ((gplayer*)_imp->_parent)->spouse_id;	
}

bool PlayerTaskInterface::CastSkill(int skill_id, int skill_level)
{
	return _imp->CastRune(skill_id,skill_level);	
}

size_t PlayerTaskInterface::GetInvEmptySlot()
{
	return _imp->GetInventory().GetEmptySlotCount();
}

void PlayerTaskInterface::LockInventory(bool is_lock)
{
	return _imp->LockInventory(is_lock);
}

unsigned char PlayerTaskInterface::GetShapeMask()
{
	return ((gplayer*)_imp->_parent)->shape_form;
}

bool PlayerTaskInterface::IsAtCrossServer()
{
	return _imp->InCentralServer();
}

bool PlayerTaskInterface::IsKing()
{
	return ((gplayer*)_imp->_parent)->IsKing();
}

int PlayerTaskInterface::GetFactionContrib()
{
	return _imp->GetFactionCumulateContrib();
}

void PlayerTaskInterface::DeliverFactionContrib(int iConsumeContrib,int iExpContrib)
{
	if(_imp->OI_IsMafiaMember()) _imp->IncFactionContrib(iConsumeContrib,iExpContrib);
}

int PlayerTaskInterface::GetFactionConsumeContrib()
{
	return _imp->GetFactionConsumeContrib();
}

void PlayerTaskInterface::TakeAwayFactionConsumeContrib(int ulNum)
{
	if(_imp->OI_IsMafiaMember()) _imp->DecFactionContrib(ulNum, 0); 
}

int PlayerTaskInterface::GetFactionExpContrib()
{
	return _imp->GetFactionExpContrib();
}

void PlayerTaskInterface::TakeAwayFactionExpContrib(int ulNum)
{
	if(_imp->OI_IsMafiaMember()) _imp->DecFactionContrib(0, ulNum);
}
int PlayerTaskInterface::GetForce()
{
	return _imp->_player_force.GetForce();
}
int PlayerTaskInterface::GetForceContribution()
{
	return _imp->GetForceContribution();
}
int PlayerTaskInterface::GetForceReputation()
{
	return _imp->GetForceReputation();
}
bool PlayerTaskInterface::ChangeForceContribution(int iValue)
{
	if(iValue > 0) return _imp->IncForceContribution(iValue);
	else return _imp->DecForceContribution(-iValue);
}
bool PlayerTaskInterface::ChangeForceReputation(int iValue)
{
	if(iValue > 0) return _imp->IncForceReputation(iValue);
	else return _imp->DecForceReputation(-iValue);
}
int PlayerTaskInterface::GetExp()
{
	return _imp->_basic.exp;
}
int PlayerTaskInterface::GetSP()
{
	return _imp->_basic.skill_point;
}
bool PlayerTaskInterface::ReduceExp(int exp)
{
	if(exp > _imp->_basic.exp) exp = _imp->_basic.exp;
	if(exp <= 0) return false;
	_imp->_basic.exp -= exp;
	_imp->SetRefreshState();
	return true;
}
bool PlayerTaskInterface::ReduceSP(int sp)
{
	if(sp > _imp->_basic.skill_point) sp =_imp->_basic.skill_point;
	if(sp <= 0) return false;
	_imp->_basic.skill_point -= sp;
	_imp->SetRefreshState();
	return true;
}
int PlayerTaskInterface::GetForceActivityLevel()
{
	ForceGlobalDataMan & man = world_manager::GetForceGlobalDataMan();
	if(!man.IsDataReady()) return -1;
	return man.GetActivityLevel(GetForce());
}
void PlayerTaskInterface::AddForceActivity(int activity)
{
	GMSV::SendIncreaseForceActivity(GetForce(), activity);
}

bool PlayerTaskInterface::HaveGotTitle(unsigned long id_designation)
{
	return _imp->GetPlayerTitle().IsExistTitle(TITLE_ID(id_designation));
}

void PlayerTaskInterface::DeliverTitle(unsigned long id_designation, long lPeriod)
{
	_imp->GetPlayerTitle().ObtainTitle(TITLE_ID(id_designation),(int)lPeriod);
}

bool PlayerTaskInterface::CheckRefine(unsigned long level_refine, unsigned long num_equips)
{
	return _imp->CheckEquipRefineLevel((int)num_equips,(int)level_refine);
}

void PlayerTaskInterface::PutHistoryGlobalValue(int lKey, int lValue)
{
	world_manager::GetUniqueDataMan().ModifyData(lKey,lValue,true);
}

void PlayerTaskInterface::ModifyHistoryGlobalValue(int lKey, int lValue)
{
	world_manager::GetUniqueDataMan().ModifyData(lKey,lValue,false);
}

int PlayerTaskInterface::GetCurHistoryStageIndex() // 当前历史阶段的序号
{
	return world_manager::GetHistoryMan().GetStageVersion();
}

unsigned long PlayerTaskInterface::GetMaxHistoryLevel()
{
	return _imp->GetHistoricalMaxLevel();
}

unsigned long PlayerTaskInterface::GetReincarnationCount()
{
	return _imp->GetReincarnationTimes();
}

unsigned long PlayerTaskInterface::GetRealmLevel()
{
	return _imp->GetRealmLevel();
}

bool PlayerTaskInterface::IsRealmExpFull()
{
	return _imp->IsRealmExpFull();
}

void PlayerTaskInterface::DeliverRealmExp(unsigned long exp)
{
	_imp->ReceiveRealmExp(exp);
}

void PlayerTaskInterface::ExpandRealmLevelMax()
{
	_imp->ExpandRealmLevelMax();
}

unsigned int PlayerTaskInterface::GetObtainedGeneralCardCount()
{
	return _imp->_generalcard_collection.count();
}

void PlayerTaskInterface::AddLeaderShip(unsigned long leader_ship)
{
	return _imp->IncLeadership(leader_ship);
}

unsigned long PlayerTaskInterface::GetObtainedGeneralCardCountByRank(int rank)
{
	return _imp->GetObtainedGeneralCardCountByRank(rank);
}

bool PlayerTaskInterface::CheckTaskForbid(unsigned long task_id)
{
	return world_manager::GetGlobalController().CheckServerForbid(SERVER_FORBID_TASK,(int)task_id);
}
int PlayerTaskInterface::GetWorldContribution()
{
	int contrib = 0, contrib_cost = 0;
	_imp->GetDBWorldContrib(contrib, contrib_cost);
	return contrib;
}
void PlayerTaskInterface::DeliverWorldContribution(int contribution)
{
	_imp->IncWorldContrib(contribution);
}
void PlayerTaskInterface::TakeAwayWorldContribution(int contribution)
{
	_imp->IncWorldContrib(-contribution);
}
int PlayerTaskInterface::GetWorldContributionSpend()
{
	int contrib = 0, contrib_cost = 0;
	_imp->GetDBWorldContrib(contrib, contrib_cost);
	return contrib_cost;
}
bool PlayerTaskInterface::PlayerCanSpendContributionAsWill()
{
	return _imp->CheckPlayerAutoSupport();
}
/*----------------------------------公共任务接口---------------------------------*/

bool PublicQuestInterface::InitAddQuest(int task_id, int child_task_id, int * common_value, int size)
{
	return world_manager::GetPublicQuestMan().InitAddQuest(task_id,child_task_id,common_value,size);	
}

bool PublicQuestInterface::QuestSetStart(int task_id, int * common_value, int size, bool no_change_rank)
{
	return world_manager::GetPublicQuestMan().QuestSetStart(task_id,common_value,size,no_change_rank);	
}

bool PublicQuestInterface::QuestSetFinish(int task_id)
{
	return world_manager::GetPublicQuestMan().QuestSetFinish(task_id);	
}

bool PublicQuestInterface::QuestSetNextChildTask(int task_id, int child_task_id, int * common_value, int size, bool no_change_rank)
{
	return world_manager::GetPublicQuestMan().QuestSetNextChildTask(task_id,child_task_id,common_value,size,no_change_rank);	
}

bool PublicQuestInterface::QuestSetRandContrib(int task_id, int fixed_contrib, int max_rand_contrib, int lowest_contrib)
{
	return world_manager::GetPublicQuestMan().QuestSetRandContrib(task_id,fixed_contrib,max_rand_contrib,lowest_contrib);	
}

int PublicQuestInterface::GetCurSubTask(int task_id)
{
	return world_manager::GetPublicQuestMan().GetCurSubTask(task_id);	
}

int PublicQuestInterface::GetCurTaskStamp(int task_id)
{
	return world_manager::GetPublicQuestMan().GetCurTaskStamp(task_id);	
}

int PublicQuestInterface::GetCurContrib(int task_id, int role_id)
{
	return world_manager::GetPublicQuestMan().GetCurContrib(task_id, role_id);	
}

int PublicQuestInterface::GetCurAllPlace(int task_id, int role_id)
{
	return world_manager::GetPublicQuestMan().GetCurAllPlace(task_id, role_id);	
}

int PublicQuestInterface::GetCurClsPlace(int task_id, int role_id)
{
	return world_manager::GetPublicQuestMan().GetCurClsPlace(task_id, role_id);	
}

bool PublicQuestInterface::QuestAddPlayer(int task_id, int role_id)
{
	return world_manager::GetPublicQuestMan().QuestAddPlayer(task_id,role_id);	
}

bool PublicQuestInterface::QuestRemovePlayer(int task_id, int role_id)
{
	return world_manager::GetPublicQuestMan().QuestRemovePlayer(task_id,role_id);	
}

bool PublicQuestInterface::QuestUpdatePlayerContrib(int task_id, int roleid, int inc_contrib)
{
	return world_manager::GetPublicQuestMan().QuestUpdatePlayerContrib(task_id,roleid,inc_contrib);	
}

void PublicQuestInterface::QuestEnterWorldInit(int task_id, int role_id)
{
	world_manager::GetPublicQuestMan().QuestEnterWorldInit(task_id,role_id);	
}

void PublicQuestInterface::QuestLeaveWorld(int task_id, int role_id)
{
	world_manager::GetPublicQuestMan().QuestLeaveWorld(task_id,role_id);	
}

long PublicQuestInterface::QuestGetGlobalValue(long lKey)
{
	//公共任务只在BigWorld中存在,所以使用world 0中的全局变量
	return world_manager::GetInstance()->GetWorldByIndex(0)->GetCommonValue(lKey);	
}

void PublicQuestInterface::SetMonsterController(long id, bool bTrigger)
{
	//公共任务只在BigWorld中存在,所以使用world 0
	if(id <= 0) return;
	world * pPlane = world_manager::GetInstance()->GetWorldByIndex(0);
	
	//先来个简单实现
	if(bTrigger)
	{
		pPlane->TriggerSpawn(id);
	}
	else
	{
		pPlane->ClearSpawn(id);
	}
}

