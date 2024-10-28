#ifndef __ONLINEGAME_GS_TASK_MAN_H__
#define __ONLINEGAME_GS_TASK_MAN_H__

#include "TaskServer.h"
class elementdataman;
bool InitQuestSystem(const char * filename,const char * filename2, elementdataman * pMan);
void PublicQuestRunTick();

struct gplayer;
class gplayer_imp;
class PlayerTaskInterface : public TaskInterface
{
gplayer_imp * _imp;
public:
	virtual unsigned long GetPlayerLevel();
	virtual void* GetActiveTaskList();
	virtual void* GetFinishedTaskList();
	virtual void* GetFinishedTimeList();
	virtual void* GetFinishedCntList();
	virtual void* GetStorageTaskList();
	virtual unsigned long* GetTaskMask();
	virtual void DeliverGold(unsigned long ulGoldNum);
	virtual void DeliverExperience(unsigned long ulExp);
	virtual void DeliverSP(unsigned long ulSP);
	virtual void DeliverReputation(long lReputation);
//	virtual bool HasTaskItem(unsigned long ulTaskItem); 	//xxxxx
//	virtual bool HasCommonItem(unsigned long ulCommonItem); //xxxx
	virtual int GetTaskItemCount(unsigned long ulTaskItem);
	virtual int GetCommonItemCount(unsigned long ulCommonItem);
	virtual bool IsInFaction(unsigned long);
	virtual int GetFactionRole();
	virtual unsigned long GetGoldNum();
	virtual void TakeAwayGold(unsigned long ulNum);
	virtual void TakeAwayTaskItem(unsigned long ulTemplId, unsigned long ulNum);
	virtual void TakeAwayCommonItem(unsigned long ulTemplId, unsigned long ulNum);
	virtual long GetReputation();
	virtual unsigned long GetCurPeriod();
	virtual void SetCurPeriod(unsigned long per);
	virtual unsigned long GetPlayerId();
	virtual unsigned long GetPlayerRace();
	virtual unsigned long GetPlayerOccupation();
	virtual bool CanDeliverCommonItem(unsigned long ulItemTypes);
	virtual bool CanDeliverTaskItem(unsigned long ulItemTypes);
//	virtual bool CanDeliverCommonItem(unsigned long ulItem,unsigned long count);
//	virtual bool CanDeliverTaskItem(unsigned long ulItem,unsigned long count);
	virtual void DeliverCommonItem(unsigned long ulItemId, unsigned long ulCount, long lPeriod);
	virtual void DeliverTaskItem(unsigned long ulItem,unsigned long count);
	virtual void NotifyClient(const void* pBuf, size_t sz);
	virtual float UnitRand();
	virtual int RandNormal(int low, int high);
	virtual int  RandSelect(const float * option, int size);
	virtual bool HasLivingSkill(unsigned long ulSkill); 
	virtual long GetLivingSkillProficiency(unsigned long ulSkill);
	virtual long GetLivingSkillLevel(unsigned long ulSkill);
	virtual void SetNewRelayStation(unsigned long ulStationId);
	virtual void SetStorehouseSize(unsigned long ulSize);
	virtual void SetStorehouseSize2(unsigned long ulSize);
	virtual void SetStorehouseSize3(unsigned long ulSize);
	virtual void SetAccountStorehouseSize(unsigned long ulSize);//帐号仓库设置可操作的格子数大小
	virtual void AddDividend(int nDividend); // 鸿利值
	virtual void SetFuryUpperLimit(unsigned long ulValue);
	virtual void TransportTo(unsigned long ulWorldId, const float pos[3], long lController);
	virtual void SetInventorySize(long lSize);

	virtual int GetTeamMemberNum();
	virtual void NotifyPlayer(unsigned long ulPlayerId, const void* pBuf, size_t sz);
	virtual void GetTeamMemberInfo(int nIndex, task_team_member_info* pInfo);
	virtual bool IsDeliverLegal();
	virtual unsigned long GetTeamMemberId(int nIndex);
	virtual bool IsInTeam();
	virtual bool IsCaptain();
	virtual bool IsMale();
	virtual unsigned long GetPos(float pos[3]);
	virtual unsigned long GetTeamMemberPos(int nIndex, float pos[3]);
	virtual void GetSpecailAwardInfo(special_award* p);
	virtual void SetPetInventorySize(unsigned long ulSize);
	virtual bool IsGM();
	virtual void SetMonsterController(long id, bool bTrigger);

	virtual bool IsMarried();
	virtual bool IsWeddingOwner();
	virtual void CheckTeamRelationship(int nReason);
	virtual void Divorce();
	virtual void SendMessage(unsigned long task_id, int channel, int param);

	//全局key/value
	virtual long GetGlobalValue(long lKey);
	virtual void PutGlobalValue(long lKey, long lValue);
	virtual void ModifyGlobalValue(long lKey, long lValue);

	virtual int SummonMonster(int mob_id, int count, int distance, int remain_time, bool die_with_self);
	virtual bool IsShieldUser();
	virtual void SetAwardDeath(bool bDeadWithLoss);
	virtual unsigned long GetRoleCreateTime();
	virtual unsigned long GetRoleLastLoginTime();
	virtual unsigned long GetAccountTotalCash();
	virtual unsigned long GetSpouseID();
	virtual bool CastSkill(int skill_id, int skill_level);
	virtual size_t GetInvEmptySlot();
	virtual void LockInventory(bool is_lock);
	virtual unsigned char GetShapeMask();
	virtual bool IsAtCrossServer();
	virtual bool IsKing();
	
	virtual int GetFactionContrib();
	virtual void DeliverFactionContrib(int iConsumeContrib,int iExpContrib);
	virtual int GetFactionConsumeContrib();
	virtual void TakeAwayFactionConsumeContrib(int ulNum);
	virtual int GetFactionExpContrib();
	virtual void TakeAwayFactionExpContrib(int ulNum);

	virtual int GetForce();
	virtual int GetForceContribution();
	virtual int GetForceReputation();
	virtual bool ChangeForceContribution(int iValue);
	virtual bool ChangeForceReputation(int iValue);
	virtual int GetExp();
	virtual int GetSP();
	virtual bool ReduceExp(int exp);
	virtual bool ReduceSP(int sp);
	virtual int GetForceActivityLevel();
	virtual void AddForceActivity(int activity);
// 称号相关
	virtual bool HaveGotTitle(unsigned long id_designation);
	virtual void DeliverTitle(unsigned long id_designation, long lPeriod = 0);
	virtual bool CheckRefine(unsigned long level_refine, unsigned long num_equips);
// 历史推进相关
	virtual void PutHistoryGlobalValue(int lKey, int lValue);
	virtual void ModifyHistoryGlobalValue(int lKey, int lValue);
	virtual int  GetCurHistoryStageIndex(); // 当前历史阶段的序号

	virtual bool CheckSimpleTaskFinshConditon(unsigned long task_id) const { return true; } // 简单任务只客户端检查，服务器可直接 返回ture。
	// 转生
	virtual unsigned long GetMaxHistoryLevel();
	virtual unsigned long GetReincarnationCount();
	// 境界
	virtual unsigned long GetRealmLevel();
	virtual bool IsRealmExpFull();
	virtual void DeliverRealmExp(unsigned long exp);
	virtual void ExpandRealmLevelMax();
	virtual unsigned int GetObtainedGeneralCardCount(); // 获得的卡牌数量， 卡牌图鉴中的数量
	virtual void AddLeaderShip(unsigned long leader_ship);
	virtual unsigned long GetObtainedGeneralCardCountByRank(int rank); // 身上、包裹、和卡牌仓库中某品阶卡牌的数量之和
	virtual bool CheckTaskForbid(unsigned long task_id);
	virtual int GetWorldContribution();	// 世界贡献度
	virtual void DeliverWorldContribution(int contribution);
	virtual void TakeAwayWorldContribution(int contribution);
	virtual int GetWorldContributionSpend();
	virtual bool PlayerCanSpendContributionAsWill();
public:
	PlayerTaskInterface(gplayer_imp * imp):_imp(imp)
	{}

};	

struct PlayerTaskTeamInterface : public TaskTeamInterface
{
	int marriage_op;
	int couple[2];
	PlayerTaskTeamInterface()
	{
		marriage_op = 0;
		couple[0] =  couple[1] = 0;
	}

	virtual void SetMarriage(int nPlayer);

	void Execute(gplayer ** list, size_t count);
};

//加入成员的包装函数 ;
inline void Task_OnTeamAddMember(TaskInterface* pTask, int index)
{
	task_team_member_info info;
	pTask->GetTeamMemberInfo(index,&info);
	OnTeamAddMember(pTask,&info);
}

#endif

