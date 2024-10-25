#include "world.h"
#include "player_imp.h"
#include "playerinvade.h"
#include <common/protocol.h>
#include <arandomgen.h>
#include "playertemplate.h"
#include "task/taskman.h"
#include <arandomgen.h>

#define ASSERT_STATE(z) ASSERT(pInvade->_invader_state == (z) && pInvade->_invader_state == pInvade->_imp->_invader_state)

/*装备损毁规则:
1:只有被人杀死才损毁装备
2:非安全锁下损毁装备限制在已经天人合一的装备上
3:损毁概率				kill_count		安全锁		非安全锁
				白/粉名	0				1%			0
				浅红	1-2				2%			1%
				中红	3-10			3%			2%
				深红	11+				4%			3% */

class player_ctrl_normal : public player_invade::player_invade_control
{
	private:
	virtual void BecomeInvader(player_invade *pInvade,const XID &who, int time)
	{
		if(time <= 0) time = 60;
		ASSERT_STATE(gactive_imp::INVADER_LVL_0);
		pInvade->SetInvaderState<0>(gactive_imp::INVADER_LVL_1);
		pInvade->ClearDefender();
		pInvade->InsertDefender(who);
		pInvade->_invader_time = time;
		pInvade->_imp->_runner->invader_rise();
		pInvade->_imp->NotifyMasterInfoToPet(true);
	}

	virtual void BecomePariah(player_invade *pInvade) 
	{
		ASSERT_STATE(gactive_imp::INVADER_LVL_0);
		pInvade->SetInvaderState<0>(gactive_imp::INVADER_LVL_2);
		pInvade->_kill_count = 1;
		pInvade->_pariah_time = PARIAH_TIME_PER_KILL;
		pInvade->ClearDefender();
		pInvade->_imp->_runner->pariah_rise();
		pInvade->_imp->_runner->pariah_duration(pInvade->_pariah_time);
		pInvade->_imp->NotifyMasterInfoToPet(true);
	}

	virtual void Heartbeat(player_invade *pInvade)
	{
		//do nothing
	}

	virtual void OnDeath(player_invade *pInvade,const XID & killer)
	{
		if(pInvade->_imp->OI_TestSafeLock())
		{
			//安全锁状态下只会损毁物品
			if(killer.IsPlayerClass())
			{
				float damage_prob = 0.01f;						
				if(abase::RandUniform() < damage_prob) pInvade->_imp->DamageItemOnDeath(false,killer);
			}
			return;
		}
		//非安全锁状态下会掉落物品
		int drop_count1 = 0; 
		int drop_count2 = 0;
		if(killer.IsPlayerClass())
		{
			float * pPropInv = NULL;
			float * pPropEquip = NULL;

			pPropInv = player_template::GetNormalInventoryDropRate();
			pPropEquip = player_template::GetNormalEquipmentDropRate();
			drop_count1 = abase::RandSelect(pPropInv,10);
			drop_count2 = abase::RandSelect(pPropEquip,10);
		}
		else
		{
			float * pPropInv = NULL;
			pPropInv = player_template::GetMobNormalInventoryDropRate();
			drop_count1 = abase::RandSelect(pPropInv,10);
		}
		if(drop_count1 + drop_count2 > 0) pInvade->_imp->DropItemOnDeath(drop_count1,drop_count2,killer);
	}

};

class player_ctrl_invade : public player_invade::player_invade_control
{
	virtual void BecomeInvader(player_invade *pInvade,const XID &who, int time)
	{
		if(time <= 0) time = 60;
		ASSERT_STATE(gactive_imp::INVADER_LVL_1);
		pInvade->InsertDefender(who);
		pInvade->_invader_time = time;
	}

	virtual void BecomePariah(player_invade *pInvade) 
	{
		ASSERT_STATE(gactive_imp::INVADER_LVL_1);
		pInvade->SetInvaderState<0>(gactive_imp::INVADER_LVL_2);
		pInvade->_kill_count ++;
		pInvade->_pariah_time = PARIAH_TIME_PER_KILL;
		pInvade->ClearDefender();
		pInvade->_imp->_runner->pariah_rise();
		pInvade->_imp->_runner->pariah_duration(pInvade->_pariah_time);
		pInvade->_imp->NotifyMasterInfoToPet(true);
	}

	virtual void Heartbeat(player_invade *pInvade)
	{
		ASSERT_STATE(gactive_imp::INVADER_LVL_1);
		if((--pInvade->_invader_time) <= 0)
		{
			pInvade->SetInvaderState<0>(gactive_imp::INVADER_LVL_0);
			pInvade->_invader_time = 0;
			pInvade->_imp->_runner->invader_fade();
			//粉名恢复啦 
			__PRINTF("粉色名字变成白色拉\n");
		}
	}

	virtual void OnDeath(player_invade *pInvade,const XID & killer)
	{
		if(pInvade->_imp->OI_TestSafeLock())
		{
			//安全锁状态下只会损毁物品
			if(killer.IsPlayerClass())
			{
				float damage_prob = 0.01f;						
				if(abase::RandUniform() < damage_prob) pInvade->_imp->DamageItemOnDeath(false,killer);
			}
			return;
		}
		//非安全锁状态下会掉落物品
		int drop_count1 = 0; 
		int drop_count2 = 0;
		if(killer.IsPlayerClass())
		{
			float * pPropInv = NULL;
			float * pPropEquip = NULL;

			pPropInv = player_template::GetInvaderInventoryDropRate();
			pPropEquip = player_template::GetInvaderEquipmentDropRate();
			drop_count1 = abase::RandSelect(pPropInv,10);
			drop_count2 = abase::RandSelect(pPropEquip,10);
		}
		else
		{
			float * pPropInv = NULL;
			pPropInv = player_template::GetMobInvaderInventoryDropRate();
			drop_count1 = abase::RandSelect(pPropInv,10);
		}
		if(drop_count1 + drop_count2 > 0) pInvade->_imp->DropItemOnDeath(drop_count1,drop_count2,killer);
	}
};

class player_ctrl_pariah : public player_invade::player_invade_control
{
	virtual void BecomeInvader(player_invade *pInvade,const XID &who, int time)
	{
		//do nothing
	}

	virtual void BecomePariah(player_invade *pInvade) 
	{
		ASSERT_STATE(gactive_imp::INVADER_LVL_2);
		int t = (pInvade->_pariah_time += PARIAH_TIME_PER_KILL);
		if(pInvade->_pariah_time > MAX_PARIAH_TIME) pInvade->_pariah_time = MAX_PARIAH_TIME;
		pInvade->_kill_count ++;

		int nps = player_template::IncPariahState(pInvade->_pariah_state,t);
		if( nps != pInvade->_pariah_state)
		{
			pInvade->SetParentPariahState<0>(nps);
		}
		pInvade->_imp->_runner->pariah_duration(pInvade->_pariah_time);
	}

	virtual void Heartbeat(player_invade *pInvade)
	{
		ReducePariah(pInvade,1);
	}

	virtual void ReducePariah(player_invade * pInvade,int time)
	{
		ASSERT_STATE(gactive_imp::INVADER_LVL_2);
		int t = (pInvade->_pariah_time -= time);
		if( t <= 0)
		{
			pInvade->SetInvaderState<0>(gactive_imp::INVADER_LVL_0);
			pInvade->_pariah_time = 0;
			pInvade->_imp->_runner->invader_fade();
			pInvade->ClearDefender();
			pInvade->_kill_count = 0;
			__PRINTF("红色色名字变成白色拉\n");
			pInvade->_imp->NotifyMasterInfoToPet(true);
		}
		else
		{
			int nps = player_template::DecPariahState(pInvade->_pariah_state,t);
			if( nps != pInvade->_pariah_state)
			{
				pInvade->SetParentPariahState<0>(nps);
			}
		}
		if(time > 1) pInvade->_imp->_runner->pariah_duration(pInvade->_pariah_time);
	}

	virtual void OnDeath(player_invade *pInvade,const XID & killer)
	{
		if(pInvade->_imp->OI_TestSafeLock())
		{
			//安全锁状态下只会损毁物品
			if(killer.IsPlayerClass())
			{
				float damage_prob = 0.02f;
				if(pInvade->_pariah_state == 1) damage_prob = 0.03f;
				else if(pInvade->_pariah_state == 2) damage_prob = 0.04f;
				if(abase::RandUniform() < damage_prob) pInvade->_imp->DamageItemOnDeath(false,killer);
			}
			return;
		}
		//非安全锁状态下会掉落物品和损毁物品
		int drop_count1 = 0; 
		int drop_count2 = 0;
		if(killer.IsPlayerClass())
		{
			float * pPropInv = NULL;
			float * pPropEquip = NULL;

			pPropInv = player_template::GetPariahInventoryDropRate(pInvade->_pariah_state);
			pPropEquip = player_template::GetPariahEquipmentDropRate(pInvade->_pariah_state);
			drop_count1 = abase::RandSelect(pPropInv,10);
			drop_count2 = abase::RandSelect(pPropEquip,10);
		}
		else
		{
			float * pPropInv = NULL;
			pPropInv = player_template::GetMobPariahInventoryDropRate(pInvade->_pariah_state);
			drop_count1 = abase::RandSelect(pPropInv,10);
		}
		if(drop_count1 + drop_count2 > 0) pInvade->_imp->DropItemOnDeath(drop_count1,drop_count2,killer);
		
		if(killer.IsPlayerClass())
		{
			float damage_prob = 0.01f;
			if(pInvade->_pariah_state == 1) damage_prob = 0.02f;
			else if(pInvade->_pariah_state == 2) damage_prob = 0.03f;
			if(abase::RandUniform() < damage_prob) pInvade->_imp->DamageItemOnDeath(true,killer);
		}
	}
};

player_invade::player_invade_control * player_invade::_invade_ctrl[4] =
{
	new player_ctrl_normal,
	new player_ctrl_invade,
	new player_ctrl_pariah,
	NULL,
};

void 
player_invade::UpdatePariahState()
{
	_pariah_state = player_template::UpdatePariahState(_pariah_time); 
}

void 
player_invade::ClientGetPariahTime()
{
	if(_invader_state == gactive_imp::INVADER_LVL_2)
	{
		_imp->_runner->pariah_duration(_pariah_time);
	}
}

