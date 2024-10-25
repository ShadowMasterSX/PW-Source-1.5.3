#include "string.h"
#include "petman.h"
#include "world.h"
#include "player_imp.h"
#include "mount_filter.h"
#include "petdataman.h"
#include "pathfinding/pathfinding.h"
#include "petnpc.h"

bool pet_gen_pos::FindGroundPos(A3DVECTOR & pos, float dis, A3DVECTOR & offset, world * plane)
{
	A3DVECTOR off(0.f,0.f,0.f);
	for(size_t i = 0; i < 10; i ++)
	{
		off += offset;
		A3DVECTOR t = pos;
		t += off;
		float offsetx = dis + abase::Rand(0.8f,1.2f);
		float offsetz = dis + abase::Rand(0.8f,1.2f);
		t.x += abase::Rand(0,1)?offsetx:-offsetx;
		t.z += abase::Rand(0,1)?offsetz:-offsetz;
		t.y = plane->GetHeightAt(t.x,t.z);
		if(!path_finding::GetValidPos(plane, t)) continue;
		
		if(fabs(t.y - pos.y) >= 6.8f)
		{
			//高度差过大，重新寻找
			continue;
		}
		pos = t;
		return true;
	}
	return false;
}

bool pet_gen_pos::FindWaterPos(A3DVECTOR & pos, float dis, A3DVECTOR & offset, world * plane)
{
	A3DVECTOR off(0.f,0.f,0.f);
	for(size_t i = 0; i < 10; i ++)
	{
		off += offset;
		A3DVECTOR t = pos;
		t += off;
		float offsetx = dis + abase::Rand(0.8f,1.2f);
		float offsetz = dis + abase::Rand(0.8f,1.2f);
		t.x += abase::Rand(0,1)?offsetx:-offsetx;
		t.z += abase::Rand(0,1)?offsetz:-offsetz;
		t.y += 1.0f;
		float height = plane->GetHeightAt(t.x,t.z);
		if(t.y < height + 1.0f) t.y = height + 1.0f;
		float waterheight = path_finding::GetWaterHeight(plane, t.x,t.z);
		if(t.y > waterheight - 1.0f)
		{
			t.y = waterheight - 1.0f;
			if(t.y < height + 1.0f) continue;
		}
		if(!path_finding::IsValidSPPos(plane, t)) continue;
		
		pos = t;
		return true;
	}
	return false;
}

bool pet_gen_pos::FindAirPos(A3DVECTOR & pos, float dis, A3DVECTOR & offset, world * plane)
{
	A3DVECTOR off(0.f,0.f,0.f);
	for(size_t i = 0; i < 10; i ++)
	{
		off += offset;
		A3DVECTOR t = pos;
		t += off;
		float offsetx = dis + abase::Rand(0.8f,1.2f);
		float offsetz = dis + abase::Rand(0.8f,1.2f);
		t.x += abase::Rand(0,1)?offsetx:-offsetx;
		t.z += abase::Rand(0,1)?offsetz:-offsetz;
		t.y += 1.0f;
		float height = plane->GetHeightAt(t.x,t.z);
		if(t.y < height + 1.5) t.y = height + 1.5f;
		float waterheight = path_finding::GetWaterHeight(plane, t.x,t.z);
		if(t.y < waterheight + 1.5f) t.y = waterheight + 1.5f;
		if(!path_finding::IsValidSPPos(plane, t)) continue;
		
		pos = t;
		return true;
	}
	return false;
}

bool pet_gen_pos::IsValidInhabit(char leader_layer, int inhabit_type)
{	
	switch(inhabit_type)
	{
	case 0:
		//陆地
		if(leader_layer != LAYER_GROUND) return false;
		break;
	case 1:
		//水下
		if(leader_layer != LAYER_WATER) return false;
		break;
	case 2:
		//空中
		if(world_manager::GetWorldLimit().nofly) return false;
		if(leader_layer == LAYER_WATER) return false;
		break;
	case 3:
		//地面加水下
		if(leader_layer == LAYER_AIR) return false;
		break;
	case 4:
		//地面加空中
		if(leader_layer == LAYER_WATER) return false;
		break;
	case 5:
		//水下加空中
		break;
	case 6:
		//海陆空
		break;
	default:
		return false;
	}
	return true;
}

int pet_gen_pos::FindValidPos(A3DVECTOR & pos, char & inhabit_mode, char leader_layer, int inhabit_type, world * plane, float dis, A3DVECTOR offset)
{
	//地区是否允许此宠物出现
	switch(inhabit_type)
	{
	case 0:
		//陆地
		break;
	case 1:
		//水下
		break;
	case 2:
		//空中
		if(world_manager::GetWorldLimit().nofly) return -2;
		break;
	case 3:
		//地面加水下
		break;
	case 4:
		//地面加空中
		break;
	case 5:
		//水下加空中
		break;
	case 6:
		//海陆空
		break;
	default:
		return -2;
	}

	//玩家位置和状态是否允许宠物出现
	switch(inhabit_type)
	{
	case 0:
		//陆地
		if(leader_layer != LAYER_GROUND) return -3;
		break;
	case 1:
		//水下
		if(leader_layer != LAYER_WATER) return -3;
		break;
	case 2:
		//空中
		if(leader_layer == LAYER_WATER) return -3;
		break;
	case 3:
		//地面加水下
		if(leader_layer == LAYER_AIR) return -3;
		break;
	case 4:
		//地面加空中
		if(leader_layer == LAYER_WATER) return -3;
		break;
	case 5:
		//水下加空中
		break;
	case 6:
		//海陆空
		break;
	default:
		return -3;
	}

	//判定此处是否能够存在能够唤出宠物的位置
	int env_chk_seq[3] = {0};  //0x01地 0x02空 0x04水
	switch(inhabit_type)
	{
	case 0:	
		//陆地
		inhabit_mode = NPC_MOVE_ENV_ON_GROUND;
		return FindGroundPos(pos,dis,offset,plane)?0:-4;
	case 1:
		//水下
		inhabit_mode = NPC_MOVE_ENV_IN_WATER;
		return FindWaterPos(pos,dis,offset,plane)?0:-4;
	case 2:
		//空中
		inhabit_mode = NPC_MOVE_ENV_ON_AIR;
		return FindAirPos(pos,dis,offset,plane)?0:-4;
	case 3:
		//地面加水下
		if(leader_layer == LAYER_WATER){
			env_chk_seq[0] = 0x04;
			env_chk_seq[1] = 0x01;
		}else{
			env_chk_seq[0] = 0x01;
			env_chk_seq[1] = 0x04;
		}
		break;
	case 4:
		//地面加空中
		if(leader_layer == LAYER_AIR){
			env_chk_seq[0] = 0x02;
			env_chk_seq[1] = 0x01;
		}else{
			env_chk_seq[0] = 0x01;
			env_chk_seq[1] = 0x02;
		}
		break;
	case 5:
		//水下加空中
		if(leader_layer == LAYER_WATER){
			env_chk_seq[0] = 0x04;
			env_chk_seq[1] = 0x02;
		}else{
			env_chk_seq[0] = 0x02;
			env_chk_seq[1] = 0x04;
		}
		break;
	case 6:
		//海陆空
		if(leader_layer == LAYER_WATER){
			env_chk_seq[0] = 0x04;
			env_chk_seq[1] = 0x01;
			env_chk_seq[2] = 0x02;
		}else if(leader_layer == LAYER_AIR){
			env_chk_seq[0] = 0x02;
			env_chk_seq[1] = 0x01;
			env_chk_seq[2] = 0x04;
		}else{ 
			env_chk_seq[0] = 0x01;
			env_chk_seq[1] = 0x02;
			env_chk_seq[2] = 0x04;
		}
		break;
	default:
		return -4;
	}
	//多栖宠物在这里找位置
	for(int i=0; i<3; i++)
	{
		if(!env_chk_seq[i]) break;
		if(env_chk_seq[i] == 0x01)	//地
		{
			if(FindGroundPos(pos,dis,offset,plane))
			{
				inhabit_mode = NPC_MOVE_ENV_ON_GROUND;
				return 0;
			}
		}
		else if(env_chk_seq[i] == 0x02)	//空
		{
			if(world_manager::GetWorldLimit().nofly) continue;
			if(FindAirPos(pos,dis,offset,plane))
			{
				inhabit_mode = NPC_MOVE_ENV_ON_AIR;
				return 0;
			}
		}
		else if(env_chk_seq[i] == 0x04)	//水
		{
			if(FindWaterPos(pos,dis,offset,plane))
			{
				inhabit_mode = NPC_MOVE_ENV_IN_WATER;
				return 0;
			}
		}
	}
	return -4;
}

namespace
{

const static int honor_level_list[] = {50,150,500,999};
inline int GetHonorLevel(int honor_point)
{
	for(int i = 0; i < pet_data::HONOR_LEVEL_COUNT; i++)
	{
		if(honor_point <= honor_level_list[i]) return i;
	}
	return pet_data::HONER_LEVEL_3;
}

class mount_petdata_imp : public petdata_imp
{
public:
	static float GetDropRate(int honor_level)
	{
		ASSERT(honor_level >=0  && honor_level < pet_data::HONOR_LEVEL_COUNT);
		const static float drop_rate[] = {1.0f,0.6f,0.3f,0.1f};
		return drop_rate[honor_level];
	}

	static float GetExpAdjust(int honor_level)
	{
		ASSERT(honor_level >=0  && honor_level < pet_data::HONOR_LEVEL_COUNT);
		const static float adjust[] = {0.1f,0.5f,1.0f,1.5f};
		return adjust[honor_level];
	}
	
	virtual bool DoActivePet(gplayer_imp * pImp, pet_manager * pMan,pet_data * pData,extend_prop * pProp)
	{
		//试图激活骑宠
		//能够激活骑宠的条件是：
		//不在水中，不在天上
		if(pImp->GetPlayerLimit(PLAYER_LIMIT_NOMOUNT))
		{
			pImp->_runner->error_message(S2C::ERR_PET_CAN_NOT_MOUNT);
			return false;
		}

		//必须在地上
		if(!pImp->_layer_ctrl.IsOnGround())
		{
			pImp->_runner->error_message(S2C::ERR_PET_CAN_NOT_MOUNT);
			return false;
		}

		//飞行不行
		if(pImp->_filters.IsFilterExist(FILTER_FLY_EFFECT))
		{
			pImp->_runner->error_message(S2C::ERR_PET_CAN_NOT_MOUNT);
			return false;
		}

		//水中不行
		if(pImp->IsUnderWater())
		{
			pImp->_runner->error_message(S2C::ERR_PET_CAN_NOT_MOUNT);
			return false;
		}

		//变身下不行
		if(pImp->GetForm())
		{
			pImp->_runner->error_message(S2C::ERR_PET_CAN_NOT_MOUNT);
			return false;
		}
		
		//隐身下不行
		if(((gplayer*)pImp->_parent)->IsInvisible())
		{
			pImp->_runner->error_message(S2C::ERR_PET_CAN_NOT_MOUNT);
			return false;
		}

		if(world_manager::GetWorldLimit().nomount)
		{
			pImp->_runner->error_message(S2C::ERR_PET_CAN_NOT_MOUNT);
			return false;
		}

		//进行召出的操作
		//首先去数据列表中查询并计算宠物的速度，然后根据忠诚度计算宠物的掉落概率
		float speedup = 3.0f;
		//查询和计算宠物速度
		if(!pet_dataman::CalcMountParam(pData->pet_tid,pData->level,speedup))
		{
			pImp->_runner->error_message(S2C::ERR_PET_CAN_NOT_MOUNT);
			//无法查询到宠物速度，表示存在不正确的因素
			return false;
		}
		int honor_level = GetHonorLevel(pData->honor_point);
		float drop_rate = GetDropRate(honor_level);

		speedup += pImp->GetMountSpeedEnhance(); 
		//假设已经查询完毕
		pImp->_filters.AddFilter(new mount_filter(pImp,FILTER_INDEX_MOUNT_FILTER,
					pData->pet_tid,pData->color,speedup,drop_rate));
		pMan->SetTestUnderWater(true);
		pMan->_cur_pet_id = XID(-1,-1);
		return true;
	}

	virtual bool DoRecallPet(gplayer_imp * pImp, pet_manager * pMan,pet_data * pData)
	{
		//骑宠的召回是无条件的
		//通过删除filter来完成消除骑乘状态
		pImp->_filters.RemoveFilter(FILTER_INDEX_MOUNT_FILTER);
		pMan->SetTestUnderWater(false);
		return true;
	}

	virtual void TestUnderWater(gplayer_imp * pImp, pet_manager * pMan,pet_data * pData,float offset)
	{
		if(offset > 1.0f)
		{
			pImp->_filters.RemoveFilter(FILTER_INDEX_MOUNT_FILTER);
			pMan->SetTestUnderWater(false);
			ClearCurPet(pImp, pMan);
		}
	}

	virtual void OnMountSpeedEnChanged(gplayer_imp *pImp,pet_manager * pMan, pet_data * pData)
	{
		if(pImp->_filters.IsFilterExist(FILTER_INDEX_MOUNT_FILTER))
		{
			float speedup = 3.0f;
			//查询和计算宠物速度
			if(!pet_dataman::CalcMountParam(pData->pet_tid,pData->level, speedup))
			{
				//注意忠诚度改变也要影响其他的数据
				//考虑忠诚度如何处理
				return ;
			}
			int honor_level = GetHonorLevel(pData->honor_point);
			float drop_rate = GetDropRate(honor_level);
			speedup += pImp->GetMountSpeedEnhance(); 
			pImp->_filters.AddFilter(new mount_filter(pImp,FILTER_INDEX_MOUNT_FILTER,
						pData->pet_tid,pData->color,speedup,drop_rate));
		}
	}
	virtual void LevelUp(gplayer_imp *pImp, pet_manager * pMan,pet_data * pData)
	{
		//骑宠升级的操作是，检查是否存在指定的filter，如果存在则进行Merge操作
		if(pImp->_filters.IsFilterExist(FILTER_INDEX_MOUNT_FILTER))
		{
			float speedup = 3.0f;
			//查询和计算宠物速度
			if(!pet_dataman::CalcMountParam(pData->pet_tid,pData->level, speedup))
			{
				//注意忠诚度改变也要影响其他的数据
				//考虑忠诚度如何处理
				return ;
			}
			int honor_level = GetHonorLevel(pData->honor_point);
			float drop_rate = GetDropRate(honor_level);
			speedup += pImp->GetMountSpeedEnhance(); 
			pImp->_filters.AddFilter(new mount_filter(pImp,FILTER_INDEX_MOUNT_FILTER,
						pData->pet_tid,pData->color,speedup,drop_rate));
		}

	}
	
	virtual void Heartbeat(gplayer_imp *pImp, pet_manager * pMan,pet_data * pData)
	{
		pMan->_cur_pet_counter ++;
		if(pMan->_cur_pet_counter >= 300)		//每5分钟一次经验
		{
			pMan->_cur_pet_counter = 0;

			int honor_level = GetHonorLevel(pData->honor_point);
			int base_exp = 10;
			base_exp = (int)((base_exp * GetExpAdjust(honor_level) + 0.1f));
			if(base_exp > 0)
			{
				pMan->RecvExp(pImp, pMan->GetCurActivePet(),base_exp);
			}
		}
	}

	virtual void OnHonorModify(gplayer_imp *pImp, pet_manager * pMan,pet_data * pData, int old_honor)
	{
		int new_honor = pData->honor_point;
		if(new_honor == old_honor) return;

		int t1 = GetHonorLevel(old_honor); 
		int t2 = GetHonorLevel(new_honor);
		if(t1 != t2)
		{
			//修正
			float speedup = 3.0f;

			//查询和计算宠物速度
			if(!pet_dataman::CalcMountParam(pData->pet_tid,pData->level,speedup))
			{
				//无法查询到宠物速度，表示存在不正确的因素
				return ;
			}

			//更新宠物移动速度和掉落概率
			int honor_level = GetHonorLevel(pData->honor_point);
			float drop_rate = GetDropRate(honor_level);
			speedup += pImp->GetMountSpeedEnhance(); 
			pImp->_filters.AddFilter(new mount_filter(pImp,FILTER_INDEX_MOUNT_FILTER,
						pData->pet_tid,pData->color,speedup,drop_rate));
		}
	}
	
	virtual bool OnPetRelocate(gplayer_imp *pImp, pet_manager * pMan,pet_data *pData,const XID &who,int stamp,bool)
	{
		return false;
	}

	virtual bool OnPetNotifyHP(gplayer_imp *pImp, pet_manager * pMan,pet_data *pData,const XID &who,int stamp,const msg_pet_hp_notify & info)
	{
		return false;
	}

	virtual bool OnPetDeath(gplayer_imp *pImp, pet_manager * pMan,pet_data *pData,const XID &who,int stamp)
	{
		return false;
	}
	
	virtual bool OnPetCtrl(gplayer_imp * pImp, pet_manager * pMan,pet_data *pData,int cur_target, int pet_cmd, const void * buf, size_t size)
	{
		return false;
	}

	virtual void OnKillMob(gplayer_imp * pImp, pet_manager * pMan, pet_data * pData, int mob_level)
	{
	}

	virtual void OnNotifyMasterInfo(gplayer_imp * pImp,pet_manager * pMan, pet_data * pData, bool)
	{
	}

	virtual void PreSwitchServer(gplayer_imp * pImp, pet_manager * pman, pet_data * pData)
	{
	}

	virtual void PostSwitchServer(gplayer_imp * pImp, pet_manager * pman, pet_data * pData)
	{
	}

	virtual void OnMasterBeAttacked(gplayer_imp * pImp,pet_manager *pman, pet_data *pData,const XID &who)
	{
	}

	virtual bool OnChangeName(gplayer_imp * pImp,pet_manager *pman, pet_data *pData,const char *name, size_t len)
	{
		return false;
	}

	virtual bool OnForgetSkill(gplayer_imp * pImp,pet_manager *pman, pet_data *pData, int skill_id)
	{
		return false;
	}

	virtual bool OnLearnSkill(gplayer_imp * pImp,pet_manager *pMan,pet_data *pData,int skill_id, int * level_result)
	{
		return false;
	}
	
	virtual bool OnDyePet(gplayer_imp * pImp,pet_manager *pMan,pet_data *pData,unsigned short color)
	{
		pData->color = color;
		return true;	
	}
	
	virtual bool OnEvolution(gplayer_imp *pImp,pet_manager *pMan,pet_data *pData,int evolution_pet_id,int pet_nature,int skill1,int level1,int skill2,int level2)
	{
		return false;
	}
};

class combat_petdata_imp : public petdata_imp
{
public:

	static float GetExpAdjust(int honor_level)
	{
		ASSERT(honor_level >=0  && honor_level < pet_data::HONOR_LEVEL_COUNT);
		const static float adjust[] = {0.1f,0.5f,1.0f,1.5f};
		return adjust[honor_level];
	}

	virtual bool DoActivePet(gplayer_imp * pImp, pet_manager * pMan,pet_data * pData,extend_prop * pProp)
	{
		if(pImp->_basic.level < pData->level - 35 - 5 * (int)pImp->GetReincarnationTimes())
		{
			pImp->_runner->error_message(S2C::ERR_LEVEL_NOT_MATCH);
			return false;
		}
		//宠物死亡状态下无法被召唤
		if(pData->hp_factor == 0)
		{
			pImp->_runner->error_message(S2C::ERR_CANNOT_SUMMON_DEAD_PET);
			return false;
		}

		XID  who;
		A3DVECTOR pos = pImp->_parent->pos;
		char inhabit_mode;
		const pet_data_temp * pTemp = pet_dataman::Get(pData->pet_tid);
		//招出普通宠物的条件需要满足环境 水下不能招出所有宠物 空中无法招出地面宠物 
		if(pTemp == NULL || pet_gen_pos::FindValidPos(pos,inhabit_mode,pImp->_layer_ctrl.GetLayer(), pTemp->inhabit_type, pImp->_plane) != 0)
		{
			//召唤失败， 因为没有可以立足的地点
			pImp->_runner->error_message(S2C::ERR_SUMMON_PET_INVALID_POS);
			return false;
		}

		int honor_level = GetHonorLevel(pData->honor_point);

		object_interface oi(pImp);
		bool bRst = oi.CreatePet(pos,inhabit_mode,pData,pMan->_pet_summon_stamp, honor_level,who,pMan->_cur_pet_aggro_state,pMan->_cur_pet_stay_state);
		if(!bRst) return false;
		pMan->_cur_pet_id = who;
		pMan->_cur_pet_hp = 0;
		pMan->_cur_pet_inhabit = 0;
		pMan->_cur_pet_inhabit = pTemp->inhabit_type;
		ASSERT(pMan->_cur_pet_id.IsActive());
		pImp->_runner->pet_ai_state(pMan->_cur_pet_aggro_state,pMan->_cur_pet_stay_state);
		return true;
	}

	virtual bool DoRecallPet(gplayer_imp * pImp, pet_manager * pMan,pet_data * pData)
	{
		if(!pMan->_cur_pet_id.IsActive()) 
		{
			return false;
		}

		//发出一个停止消息给目标对象
		pImp->SendTo<0>(GM_MSG_PET_DISAPPEAR,pMan->_cur_pet_id,pMan->_pet_summon_stamp);
		
		pMan->_cur_pet_id = XID(-1,-1);
		return true;
	}

	virtual void TestUnderWater(gplayer_imp * pImp, pet_manager * pMan,pet_data * pData,float offset)
	{
		//战宠不处理水下
	}

	virtual void OnMountSpeedEnChanged(gplayer_imp *pImp,pet_manager * pMan, pet_data * pData){}
	virtual void LevelUp(gplayer_imp *pImp, pet_manager * pMan,pet_data * pData)
	{
		//战宠升级的操作是 通知宠物新的级别
		int level = pData->level;
		pImp->SendTo<0>(GM_MSG_PET_LEVEL_UP,pMan->_cur_pet_id,pMan->_pet_summon_stamp,&level,sizeof(level));
	}
	
	virtual void Heartbeat(gplayer_imp *pImp, pet_manager * pMan,pet_data * pData)
	{
		pMan->_cur_pet_counter ++;
		if(pMan->_cur_pet_counter >= 15)	//每15秒同步一下自己的数据给宠物
		{
			if(pMan->_cur_pet_state <= 0)
			{
				//超过15秒没有数据到来，则将宠物自动收回
				pMan->RecallPetWithoutFree(pImp);
				pMan->_cur_pet_counter = 0;
				pMan->_cur_pet_state = 0;
				return;
			}
			pMan->_cur_pet_counter = 0;
			pMan->_cur_pet_state = 0;
		}

		if((g_timer.get_systime() & 0x0F) == 0)
		{
			//每隔16秒判断一次宠物是否能够继续存在
			if(!pet_gen_pos::IsValidInhabit(pImp->_layer_ctrl.GetLayer(), pMan->_cur_pet_inhabit))
			{
				pMan->RecallPetWithoutFree(pImp);
				pMan->_cur_pet_counter = 0;
				pMan->_cur_pet_state = 0;
				return;
			}
		}
		
		if((g_timer.get_systime() & 0x03) == 0)
		{
			//每4秒发送自己的位置给宠物
			char layer = pImp->_layer_ctrl.GetLayer();
			pImp->SendTo<0>(GM_MSG_MASTER_NOTIFY_LAYER, pMan->_cur_pet_id, pMan->_pet_summon_stamp, &layer,sizeof(layer));
		}

		pMan->_cur_pet_notify_counter ++;
		if(pMan->_cur_pet_notify_counter >= pet_manager::NOTIFY_MASTER_TIME)
		{
			//发送自己的数据给宠物
			pet_leader_prop data;
			pImp->SetPetLeaderData(data);
			pImp->SendTo<0>(GM_MSG_PET_MASTER_INFO,pMan->_cur_pet_id,pMan->_pet_summon_stamp,&data,sizeof(data));
			pMan->_cur_pet_notify_counter  = 0;
		}
	}

	virtual void OnHonorModify(gplayer_imp *pImp, pet_manager * pMan,pet_data * pData, int old_honor)
	{
		int new_honor = pData->honor_point;
		if(new_honor == old_honor) return;

		int hl = GetHonorLevel(new_honor);
		
		//通知宠物自己的honor变了
		pImp->SendTo<0>(GM_MSG_PET_HONOR_MODIFY,pMan->_cur_pet_id,pMan->_pet_summon_stamp,&hl,sizeof(hl));
	}
	
	virtual bool OnPetRelocate(gplayer_imp *pImp, pet_manager * pMan,pet_data *pData,const XID &who,int stamp,bool is_disappear)
	{
		if(pMan->_cur_pet_id != who) return false;
		if(pMan->_pet_summon_stamp != stamp) return false;
		//宠物条件满足才能继续

		msg_pet_pos_t data;
		data.pos = pImp->_parent->pos;
		const pet_data_temp * pTemp = pet_dataman::Get(pData->pet_tid);
		if(is_disappear || pTemp == NULL || pet_gen_pos::FindValidPos(data.pos,data.inhabit_mode,pImp->_layer_ctrl.GetLayer(), pTemp->inhabit_type, pImp->_plane) != 0)
		{
			//无法找到合适的内容 或者宠物要求消失
			pMan->RecallPetWithoutFree(pImp);
			return true;
		}
		//找到了合适的坐标发送给对方
		pImp->SendTo<0>(GM_MSG_PET_CHANGE_POS, who, stamp,&data,sizeof(data));
		return true;
	}

	virtual bool OnPetNotifyHP(gplayer_imp *pImp, pet_manager * pMan,pet_data *pData,const XID &who,int stamp,const msg_pet_hp_notify & info)
	{
		if(pMan->_cur_pet_id != who) return false;
		if(pMan->_pet_summon_stamp != stamp) return false;
		//宠物条件满足才能继续

		//宠物通知计数器加一
		pMan->_cur_pet_state ++;

		if(pData->hp_factor != info.hp_ratio || pMan->_cur_pet_hp != info.cur_hp
				|| pMan->_cur_pet_mp_factor != info.mp_ratio || pMan->_cur_pet_mp != info.cur_mp)
		{
			//更新宠物的血值
			pData->hp_factor = info.hp_ratio;
			pMan->_cur_pet_hp = info.cur_hp;
			pMan->_cur_pet_mp_factor = info.mp_ratio;
			pMan->_cur_pet_mp = info.cur_mp;
			//通知客户端
			pImp->_runner->pet_hp_notify(pMan->GetCurActivePet(), info.hp_ratio,info.cur_hp,info.mp_ratio,info.cur_mp);
		}

		if(info.combat_state)
		{
			pImp->ActiveCombatState(true);
			pImp->SetCombatTimer(6);
		}

		if(!(pImp->IsAttackMonster()) && info.attack_monster)
		{
			pImp->SetAttackMonster(true);
		}
		
		pMan->_cur_pet_combat_state = info.combat_state;
		if(pMan->_cur_pet_aggro_state != info.aggro_state ||
				pMan->_cur_pet_stay_state != info.stay_mode)
		{
			pMan->_cur_pet_aggro_state = info.aggro_state;
			pMan->_cur_pet_stay_state = info.stay_mode;
			//通知客户端
			pImp->_runner->pet_ai_state(pMan->_cur_pet_aggro_state,pMan->_cur_pet_stay_state);
		}
		return true;
	}

	virtual bool OnPetDeath(gplayer_imp *pImp, pet_manager * pMan,pet_data *pData,const XID &who,int stamp)
	{
		if(pMan->_cur_pet_id != who) return false;
		if(pMan->_pet_summon_stamp != stamp) return false;
		//宠物条件满足才能继续

		//更新宠物的血值
		pData->hp_factor = 0;
		int index = pMan->GetCurActivePet();
		pMan->_cur_pet_hp = 0;

		//召回宠物
		pMan->RecallPetWithoutFree(pImp,pet_manager::PET_DEATH);

		//减少忠诚等操作在外面完成

		//通知客户端
		pImp->_runner->pet_dead(index);
		return true;
	}
	
	virtual bool OnPetCtrl(gplayer_imp * pImp, pet_manager * pMan,pet_data *pData,int cur_target, int pet_cmd, const void * buf, size_t size)
	{
		if(size > 120) return false;
		XID pet = pMan->_cur_pet_id;
		if(pet.IsValid())
		{
			char dbuf[128];
			*(int*)dbuf = pet_cmd;
			memcpy(dbuf + sizeof(int), buf, size);

			pImp->SendTo<0>(GM_MSG_PET_CTRL_CMD,pet, cur_target,dbuf ,size + sizeof(int));	
		}
		return false;
	}

	virtual void OnKillMob(gplayer_imp * pImp, pet_manager * pMan, pet_data * pData, int mob_level)
	{
		int base_exp = 10;
		if(mob_level < pData->level)
		{
			base_exp -= (pData->level - mob_level);
			if(base_exp <= 0) return;
		}
		int honor_level = GetHonorLevel(pData->honor_point);
		base_exp = (int)((base_exp * GetExpAdjust(honor_level) + 0.1f));
		if(base_exp > 0)
		{
			pMan->RecvExp(pImp, pMan->GetCurActivePet(),base_exp);
		}
	}

	virtual void OnNotifyMasterInfo(gplayer_imp * pImp,pet_manager * pMan, pet_data *pData,bool at_once)
	{
		if(at_once)
		{
			pet_leader_prop data;
			pImp->SetPetLeaderData(data);
			pImp->SendTo<0>(GM_MSG_PET_MASTER_INFO,pMan->_cur_pet_id,pMan->_pet_summon_stamp,&data,sizeof(data));
			pMan->_cur_pet_notify_counter = 0;
		}
		else
		{
			pMan->_cur_pet_notify_counter = pet_manager::NOTIFY_MASTER_TIME;
		}
	}

	virtual void PreSwitchServer(gplayer_imp * pImp, pet_manager * pMan, pet_data * pData)
	{
		//现在数据已经发出，数据的改变不会在目的服务器生效
		if(!pMan->_cur_pet_id.IsActive()) 
		{
			return;
		}

		//这时需要通知宠物应该消失 在转移完成后再重新召唤出来
		pImp->SendTo<0>(GM_MSG_PET_DISAPPEAR,pMan->_cur_pet_id,pMan->_pet_summon_stamp);
	}

	virtual void PostSwitchServer(gplayer_imp * pImp, pet_manager * pMan, pet_data * pData)
	{
		//将宠物重新召唤出来
		XID  who;
		A3DVECTOR pos = pImp->_parent->pos;
		char inhabit_mode;
		//招出普通宠物的条件需要满足环境 水下不能招出所有宠物 空中无法招出地面宠物 
		const pet_data_temp * pTemp = pet_dataman::Get(pData->pet_tid);
		if(pTemp == NULL || pet_gen_pos::FindValidPos(pos,inhabit_mode,pImp->_layer_ctrl.GetLayer(), pTemp->inhabit_type, pImp->_plane) != 0)
		{
			//没有可以立足的地点，将宠物召回
			pMan->RecallPetWithoutFree(pImp);
			return;
		}
		int honor_level = GetHonorLevel(pData->honor_point);

		pMan->_pet_summon_stamp ++;
		object_interface oi(pImp);
		bool bRst = oi.CreatePet(pos,inhabit_mode,pData,pMan->_pet_summon_stamp, honor_level,who,pMan->_cur_pet_aggro_state,pMan->_cur_pet_stay_state);
		if(!bRst)
		{
			pMan->RecallPetWithoutFree(pImp);
			return ;
		}
		pMan->_cur_pet_id = who;
		pMan->_cur_pet_inhabit = pTemp->inhabit_type;
		ASSERT(pMan->_cur_pet_id.IsActive());
		pImp->_runner->summon_pet(pMan->GetCurActivePet(),pData->pet_tid,pMan->_cur_pet_id.id,0);
		pImp->_runner->pet_ai_state(pMan->_cur_pet_aggro_state,pMan->_cur_pet_stay_state);
		return ;
	}

	virtual void OnMasterBeAttacked(gplayer_imp * pImp,pet_manager *pMan, pet_data *pData,const XID &who)
	{
		if(!pMan->_cur_pet_id.IsActive()) return;

		//是否限制发送次数？？  现在先不管 以后可以考虑， 可以减少消息数目
		pImp->SendTo<0>(GM_MSG_MASTER_ASK_HELP, pMan->_cur_pet_id, 0, &who, sizeof(who));
	}
	virtual bool OnChangeName(gplayer_imp * pImp,pet_manager *pman, pet_data *pData,const char *name, size_t len)
	{
		if(len > 16) len = 16;
		memcpy(pData->name, name, len);
		pData->name_len = len;
		return true;
	}

	virtual bool OnForgetSkill(gplayer_imp * pImp,pet_manager *pMan, pet_data *pData, int skill_id)
	{
		if(!pMan->IsSkillNormal(pImp,pData,skill_id))
			return false;
		int i = 0;
		for(i = 0; i < pet_data::MAX_PET_SKILL_COUNT; i ++)
		{
			if(pData->skills[i].skill == skill_id)
			{
				for(int j = i; j < pet_data::MAX_PET_SKILL_COUNT - 1; j ++)
				{
					pData->skills[j].skill = pData->skills[j+1].skill;
					pData->skills[j].level = pData->skills[j+1].level;
				}
				pData->skills[pet_data::MAX_PET_SKILL_COUNT - 1].skill = 0;
				pData->skills[pet_data::MAX_PET_SKILL_COUNT - 1].level = 0;

				//更新技能
				XID id = pMan->_cur_pet_id;
				if(id.IsValid())
				{
					pImp->SendTo<0>(GM_MSG_PET_SKILL_LIST,id,pMan->_pet_summon_stamp, pData->skills, sizeof(pData->skills));
				}
				return true;
			}
		}
		return false;
	}

	virtual bool OnLearnSkill(gplayer_imp * pImp,pet_manager *pMan,pet_data *pData,int skill_id, int * level_result)
	{
		int index = -1;
		int i = 0;
		for(i = 0; i < pet_data::MAX_PET_SKILL_COUNT; i ++)
		{
			if(pData->skills[i].skill == 0)
			{
				//现在保证技能列表中不会有空洞
				//前面的删除技能不会引起空洞
				break;
			}
			if(pData->skills[i].skill == skill_id)
			{
				index = i;
			}
		}
		if(pMan->GetNormalSkillNum(pImp,pData)>= 4 && index == -1) return false;	//如果宠物技能达到4个，只能升级技能不能学习新技能
		
		int new_level = GNET::SkillWrapper::PetLearn((unsigned int)skill_id, 
				pData->level, object_interface(pImp),
				(unsigned int *)pData->skills, i*2);
		if(new_level <= 0) return false;

		if(index >= 0)
		{
			pData->skills[index].level = new_level;
		}
		else
		{
			pData->skills[i].skill = skill_id;
			pData->skills[i].level = new_level;
		}

		//更新技能
		XID id = pMan->_cur_pet_id;
		if(id.IsValid())
		{
			pImp->SendTo<0>(GM_MSG_PET_SKILL_LIST,id,pMan->_pet_summon_stamp, pData->skills, sizeof(pData->skills));
		}
		
		return true;
	}
	
	virtual bool OnDyePet(gplayer_imp * pImp,pet_manager *pMan,pet_data *pData,unsigned short color)
	{
		return false;	
	}

	virtual bool OnEvolution(gplayer_imp *pImp,pet_manager *pMan,pet_data *pData,int evolution_pet_id,int pet_nature,int skill1,int level1,int skill2,int level2)
	{
		//获得进化id
		const pet_data_temp * pTemp = pet_dataman::Get(pData->pet_tid);
		const pet_data_temp * eTemp = pet_dataman::Get(evolution_pet_id);
		if(pTemp == NULL || pTemp->evolution_id == 0 || eTemp->pet_class != pet_data::PET_CLASS_EVOLUTION)
			return false;
		//修改data
		pData->pet_tid = evolution_pet_id;
		pData->pet_egg_tid = pTemp->evolution_id;
		pData->pet_class = pet_data::PET_CLASS_EVOLUTION; 
		//随机系数
		pData->evo_prop.r_attack = abase::RandNormal(0,eTemp->max_r_attack);
		pData->evo_prop.r_defense = abase::RandNormal(0,eTemp->max_r_defense);
		pData->evo_prop.r_hp = abase::RandNormal(0,eTemp->max_r_hp);
		pData->evo_prop.r_atk_lvl = abase::RandNormal(0,eTemp->max_r_atk_lvl);
		pData->evo_prop.r_def_lvl = abase::RandNormal(0,eTemp->max_r_def_lvl);
		pData->evo_prop.nature = pet_nature;
		
		int count = 0;
		int skills[3] = { skill1,skill2,eTemp->specific_skill_id};
		int levels[3] = { level1,level2,1};
		for(int i = 0; i < pet_data::MAX_PET_SKILL_COUNT && count < 3; i ++)
		{
			if(pData->skills[i].skill == 0)
			{
				if(skills[count] == 0)
					continue;
				pData->skills[i].skill = skills[count];
				pData->skills[i].level = levels[count];
				count ++;
			}
		}
		return true;
	}
};

class follow_petdata_imp : public combat_petdata_imp
{
public:

	virtual void OnMountSpeedEnChanged(gplayer_imp *pImp,pet_manager * pMan, pet_data * pData){}
	virtual void LevelUp(gplayer_imp *pImp, pet_manager * pMan,pet_data * pData)
	{
	}
	

	virtual void OnHonorModify(gplayer_imp *pImp, pet_manager * pMan,pet_data * pData, int old_honor)
	{
	}

	virtual bool OnPetDeath(gplayer_imp *pImp, pet_manager * pMan,pet_data *pData,const XID &who,int stamp)
	{
		if(pMan->_cur_pet_id != who) return false;
		if(pMan->_pet_summon_stamp != stamp) return false;
		pData->hp_factor = 1.f;
		pMan->_cur_pet_hp = 0;
		pMan->RecallPetWithoutFree(pImp);
		return false;
	}

	virtual bool OnPetCtrl(gplayer_imp * pImp, pet_manager * pMan,pet_data *pData,int cur_target, int pet_cmd, const void * buf, size_t size)
	{
		return false;
	}

	virtual void OnKillMob(gplayer_imp * pImp, pet_manager * pMan, pet_data * pData, int mob_level)
	{
	}
	virtual void OnNotifyMasterInfo(gplayer_imp * pImp,pet_manager * pMan, pet_data *pData,bool)
	{
	}

	virtual void PreSwitchServer(gplayer_imp * pImp, pet_manager * pMan, pet_data * pData)
	{
		//现在数据已经发出，数据的改变不会在目的服务器生效
		if(!pMan->_cur_pet_id.IsActive()) return;
		pImp->SendTo<0>(GM_MSG_PET_DISAPPEAR,pMan->_cur_pet_id,pMan->_pet_summon_stamp);
	}

	virtual void PostSwitchServer(gplayer_imp * pImp, pet_manager * pMan, pet_data * pData)
	{
		pMan->RecallPetWithoutFree(pImp);
	}

	virtual void OnMasterBeAttacked(gplayer_imp * pImp,pet_manager *pman, pet_data *pData,const XID &who)
	{
	}
	virtual bool OnForgetSkill(gplayer_imp * pImp,pet_manager *pman, pet_data *pData, int skill_id)
	{
		return false;
	}

	virtual bool OnLearnSkill(gplayer_imp * pImp,pet_manager *pMan,pet_data *pData,int skill_id, int * level_result)
	{
		return false;
	}
	
	virtual bool OnDyePet(gplayer_imp * pImp,pet_manager *pMan,pet_data *pData,unsigned short color)
	{
		return false;	
	}
	
	virtual bool OnEvolution(gplayer_imp *pImp,pet_manager *pMan,pet_data *pData,int evolution_pet_id,int pet_nature,int skill1,int level1,int skill2,int level2)
	{
		return false;
	}
};

class summon_petdata_imp : public combat_petdata_imp
{
public:
	virtual bool DoActivePet(gplayer_imp * pImp, pet_manager * pMan,pet_data * pData,extend_prop * pProp)
	{
		XID  who;
		A3DVECTOR pos = pImp->_parent->pos;
		char inhabit_mode;
		const pet_data_temp * pTemp = pet_dataman::Get(pData->pet_tid);
		//招出普通宠物的条件需要满足环境 水下不能招出所有宠物 空中无法招出地面宠物 
		if(pTemp == NULL || pet_gen_pos::FindValidPos(pos,inhabit_mode,pImp->_layer_ctrl.GetLayer(), pTemp->inhabit_type, pImp->_plane) != 0)
		{
			//召唤失败， 因为没有可以立足的地点
			pImp->_runner->error_message(S2C::ERR_SUMMON_PET_INVALID_POS);
			return false;
		}

		int honor_level = GetHonorLevel(pData->honor_point);

		object_interface oi(pImp);
		bool bRst = oi.CreatePet2(pos,inhabit_mode,pData,pMan->_pet_summon_stamp, honor_level,who,pMan->_cur_pet_aggro_state,pMan->_cur_pet_stay_state,pMan->_summon_skill_level,pProp);
		if(!bRst) return false;
		pMan->_cur_pet_id = who;
		pMan->_cur_pet_hp = 0;
		pMan->_cur_pet_inhabit = 0;
		pMan->_cur_pet_inhabit = pTemp->inhabit_type;
		ASSERT(pMan->_cur_pet_id.IsActive());
		pImp->_runner->pet_ai_state(pMan->_cur_pet_aggro_state,pMan->_cur_pet_stay_state);
		return true;
	}
	virtual void OnMountSpeedEnChanged(gplayer_imp *pImp,pet_manager * pMan, pet_data * pData){}
	virtual void LevelUp(gplayer_imp *pImp, pet_manager * pMan,pet_data * pData)
	{
	}
	virtual void Heartbeat(gplayer_imp *pImp, pet_manager * pMan,pet_data * pData)
	{
		if(pMan->_cur_pet_life > 0)
		{
			if(--pMan->_cur_pet_life <= 0)
			{
				pMan->RecallPetWithoutFree(pImp,pet_manager::PET_LIFE_EXHAUST);
				return;
			}
		}
		pData->feed_period  = 0;
		combat_petdata_imp::Heartbeat(pImp,pMan,pData);
	}
	virtual void OnHonorModify(gplayer_imp *pImp, pet_manager * pMan,pet_data * pData, int old_honor)
	{
	}
	virtual void OnKillMob(gplayer_imp * pImp, pet_manager * pMan, pet_data * pData, int mob_level)
	{
	}
	virtual void PostSwitchServer(gplayer_imp * pImp, pet_manager * pMan, pet_data * pData)
	{
		//将宠物重新召唤出来
		XID  who;
		A3DVECTOR pos = pImp->_parent->pos;
		char inhabit_mode;
		//招出普通宠物的条件需要满足环境 水下不能招出所有宠物 空中无法招出地面宠物 
		const pet_data_temp * pTemp = pet_dataman::Get(pData->pet_tid);
		if(pTemp == NULL || pet_gen_pos::FindValidPos(pos,inhabit_mode,pImp->_layer_ctrl.GetLayer(), pTemp->inhabit_type, pImp->_plane) != 0)
		{
			//没有可以立足的地点，将宠物召回
			pMan->RecallPetWithoutFree(pImp);
			return;
		}
		int honor_level = GetHonorLevel(pData->honor_point);

		pMan->_pet_summon_stamp ++;
		object_interface oi(pImp);
		bool bRst = oi.CreatePet2(pos,inhabit_mode,pData,pMan->_pet_summon_stamp, honor_level,who,pMan->_cur_pet_aggro_state,pMan->_cur_pet_stay_state,pMan->_summon_skill_level);
		if(!bRst)
		{
			pMan->RecallPetWithoutFree(pImp);
			return ;
		}
		pMan->_cur_pet_id = who;
		pMan->_cur_pet_inhabit = pTemp->inhabit_type;
		ASSERT(pMan->_cur_pet_id.IsActive());
		pImp->_runner->summon_pet(pMan->GetCurActivePet(),pData->pet_tid,pMan->_cur_pet_id.id,pMan->_cur_pet_life);
		pImp->_runner->pet_ai_state(pMan->_cur_pet_aggro_state,pMan->_cur_pet_stay_state);
		return ;
	}

	virtual bool OnChangeName(gplayer_imp * pImp,pet_manager *pman, pet_data *pData,const char *name, size_t len)
	{
		return false;
	}
	virtual bool OnForgetSkill(gplayer_imp * pImp,pet_manager *pman, pet_data *pData, int skill_id)
	{
		return false;
	}
	virtual bool OnLearnSkill(gplayer_imp * pImp,pet_manager *pMan,pet_data *pData,int skill_id, int * level_result)
	{
		return false;
	}
	
	virtual bool OnDyePet(gplayer_imp * pImp,pet_manager *pMan,pet_data *pData,unsigned short color)
	{
		return false;	
	}
	
	virtual bool OnEvolution(gplayer_imp *pImp,pet_manager *pMan,pet_data *pData,int evolution_pet_id,int pet_nature,int skill1,int level1,int skill2,int level2)
	{
		return false;
	}
};

class evolution_petdata_imp : public combat_petdata_imp
{
public:
	virtual bool OnEvolution(gplayer_imp *pImp,pet_manager *pMan,pet_data *pData,int evolution_pet_id,int pet_nature,int skill1,int level1,int skill2,int level2)
	{
		return false;
	}
	
	virtual bool DoActivePet(gplayer_imp *pImp,pet_manager *pMan,pet_data *pData,extend_prop *prop)
	{
		if(pImp->_basic.level < pData->level - 35 - 5 * (int)pImp->GetReincarnationTimes())
		{
			pImp->_runner->error_message(S2C::ERR_LEVEL_NOT_MATCH);
			return false;
		}
		//宠物死亡状态下无法被召唤
		if(pData->hp_factor == 0)
		{
			pImp->_runner->error_message(S2C::ERR_CANNOT_SUMMON_DEAD_PET);
			return false;
		}

		XID  who;
		A3DVECTOR pos = pImp->_parent->pos;
		char inhabit_mode;
		const pet_data_temp * pTemp = pet_dataman::Get(pData->pet_tid);
		//招出普通宠物的条件需要满足环境 水下不能招出所有宠物 空中无法招出地面宠物 
		if(pTemp == NULL || pet_gen_pos::FindValidPos(pos,inhabit_mode,pImp->_layer_ctrl.GetLayer(), pTemp->inhabit_type, pImp->_plane) != 0)
		{
			//召唤失败， 因为没有可以立足的地点
			pImp->_runner->error_message(S2C::ERR_SUMMON_PET_INVALID_POS);
			return false;
		}

		int honor_level = GetHonorLevel(pData->honor_point);
		
		object_interface oi(pImp);
		bool bRst = oi.CreatePet3(pos,inhabit_mode,pData,pMan->_pet_summon_stamp,honor_level,who,pMan->_cur_pet_aggro_state,pMan->_cur_pet_stay_state,prop);
		if(!bRst) return false;
		pMan->_cur_pet_id = who;
		pMan->_cur_pet_hp = 0;
		pMan->_cur_pet_inhabit = 0;
		pMan->_cur_pet_inhabit = pTemp->inhabit_type;
		ASSERT(pMan->_cur_pet_id.IsActive());
		pImp->_runner->pet_ai_state(pMan->_cur_pet_aggro_state,pMan->_cur_pet_stay_state);
		return true;
	}

	virtual void PostSwitchServer(gplayer_imp *pImp,pet_manager *pMan,pet_data *pData)
	{
		XID who;
		A3DVECTOR pos = pImp->_parent->pos;
		char inhabit_mode;
		const pet_data_temp * pTemp = pet_dataman::Get(pData->pet_tid);
		//招出普通宠物的条件需要满足环境 水下不能招出所有宠物 空中无法招出地面宠物 
		if(pTemp == NULL || pet_gen_pos::FindValidPos(pos,inhabit_mode,pImp->_layer_ctrl.GetLayer(), pTemp->inhabit_type, pImp->_plane) != 0)
		{
			//没有可以立足的地点，将宠物召回
			pMan->RecallPetWithoutFree(pImp);
			return ;
		}

		pMan->_pet_summon_stamp ++;
		int honor_level = GetHonorLevel(pData->honor_point);
		object_interface oi(pImp);
		bool bRst = oi.CreatePet3(pos,inhabit_mode,pData,pMan->_pet_summon_stamp,honor_level,who,pMan->_cur_pet_aggro_state,pMan->_cur_pet_stay_state);
		if(!bRst)
		{
			pMan->RecallPetWithoutFree(pImp);
			return ;
		}
		pMan->_cur_pet_id = who;
		pMan->_cur_pet_inhabit = pTemp->inhabit_type;
		ASSERT(pMan->_cur_pet_id.IsActive());
		pImp->_runner->summon_pet(pMan->GetCurActivePet(),pData->pet_tid,pMan->_cur_pet_id.id,0);
		pImp->_runner->pet_ai_state(pMan->_cur_pet_aggro_state,pMan->_cur_pet_stay_state);
		return ;
	}
};

petdata_imp * __pet_imp[pet_data::PET_CLASS_MAX] = {	new mount_petdata_imp, 
							new combat_petdata_imp,
							new follow_petdata_imp,
							new summon_petdata_imp,
							NULL,
							new evolution_petdata_imp};

}

pet_manager::pet_manager():_active_pet_slot(1),_cur_active_pet(-1),_cur_pet_id(-1,-1),_is_on_underwater(false)
{
	_pet_summon_stamp= 0;
	_cur_pet_counter = 0;
	_cur_pet_notify_counter = 0;
	_cur_pet_state = 0;
	memset(&_pet_list,0,sizeof(_pet_list));
	_cur_pet_aggro_state = gpet_imp::PET_AGGRO_AUTO;
	_cur_pet_stay_state = gpet_imp::PET_MOVE_FOLLOW;
	_cur_pet_hp = 0;
	_cur_pet_inhabit = 0;
	_need_feed = true;
	_cur_pet_mp_factor = 0.f;
	_cur_pet_mp = 0;
	_cur_pet_life = 0;
	_summon_skill_level = 0;
}

pet_manager::~pet_manager()
{
	for(size_t i = 0; i < MAX_PET_CAPACITY + MAX_SUMMON_CAPACITY; i ++)
	{
		if(_pet_list[i]) abase::fastfree(_pet_list[i],sizeof(pet_data));
	}
}
//召唤宠物
bool 
pet_manager::ActivePet(gplayer_imp * pImp, size_t index)
{
	//player 的状态在外面完成判断
	//首先进行基础判断
	//if(IsPetActive()) return false;
	//有宠物就先召回
	if(IsPetActive())
	{
		if(RecallPetWithoutFree(pImp))
			TryFreePet(pImp);
	}
	pet_data * pData = GetPetData(index);
	if(pData == NULL) return false;
	
	//根据宠物的类型来召唤宠物
	//如果是骑宠，则让玩家进入骑乘状态
	//如果是战宠或者观赏宠， 则放出宠物NPC
	size_t cls = (size_t)pData->pet_class;
	if(cls < pet_data::PET_CLASS_MAX)
	{
		_cur_pet_id = XID(-1,-1);
		extend_prop prop;
		if(__pet_imp[cls]->DoActivePet(pImp, this, pData, &prop))
		{
			//设置激活标志
			_cur_active_pet = index;
			_cur_pet_counter = 0;

			int pet_id = 0;
			if(_cur_pet_id.IsActive())
			{
				pet_id = _cur_pet_id.id;
			}

			pImp->_runner->summon_pet(index,pData->pet_tid,pet_id,0);
			if(pData->pet_class == pet_data::PET_CLASS_EVOLUTION)
			{
				pImp->_runner->pet_property(index,prop);
			}
			return true;
		}
		else
		{
			return false;
		}
	}
	return false;
}

bool 
pet_manager::RecallPet(gplayer_imp * pImp)
{
	bool b = RecallPetWithoutFree(pImp);
	if(b) TryFreePet(pImp);
	return b;
}

bool 
pet_manager::RecallPetWithoutFree(gplayer_imp * pImp, char reason)
{
	//player 的状态在外面完成判断
	//首先进行基础判断
	if(!IsPetActive()) return false;
	pet_data * pData = GetPetData(_cur_active_pet);
	if(pData == NULL) return false;

	//根据宠物的类型来处理
	size_t cls = (size_t)pData->pet_class;
	if(cls < pet_data::PET_CLASS_MAX)
	{
		if(__pet_imp[cls]->DoRecallPet(pImp, this, pData))
		{
			//设置召回操作
			pImp->_runner->recall_pet(_cur_active_pet,pData->pet_tid,reason);
			_cur_active_pet = -1;
			_cur_pet_id = XID(-1,-1);

			//召回后修改召唤的时间戳
			_pet_summon_stamp ++;
			return true;
		}
		else
		{
			return false;
		}
	}
	return false;
}


void 
pet_manager::TestUnderWater(gplayer_imp * pImp, float offset)
{
	if(!IsPetActive()) return;
	pet_data * pData = GetPetData(_cur_active_pet);
	if(pData == NULL) return ;

	//根据宠物的类型来处理
	size_t cls = (size_t)pData->pet_class;
	if(cls < pet_data::PET_CLASS_MAX)
	{
		__pet_imp[cls]->TestUnderWater(pImp, this, pData, offset);
	}
	return;
}

void 
pet_manager::Save(archive & ar)
{
	for(size_t i = 0; i < MAX_PET_CAPACITY + MAX_SUMMON_CAPACITY; i ++)
	{
		if(_pet_list[i])
		{
			ar << sizeof(pet_data);
			ar.push_back(_pet_list[i],sizeof(pet_data));
		}
		else
		{
			ar << 0;
		}
		
	}
	ar << _active_pet_slot << _cur_active_pet << _cur_pet_id << _pet_summon_stamp << _is_on_underwater << _cur_pet_aggro_state << _need_feed << _cur_pet_stay_state << _cur_pet_counter << _cur_pet_inhabit << _cur_pet_mp_factor << _cur_pet_mp << _cur_pet_life << _summon_skill_level;
}

void 
pet_manager::Load(archive & ar)
{
	for(size_t i = 0; i < MAX_PET_CAPACITY + MAX_SUMMON_CAPACITY; i ++)
	{
		size_t s;
		ar >> s;
		if(!s) continue;
		pet_data * pData = (pet_data*)abase::fastalloc(sizeof(pet_data));
		ar.pop_back(pData,sizeof(pet_data));
		_pet_list[i] = pData;
	}
	ar >> _active_pet_slot >> _cur_active_pet >> _cur_pet_id >> _pet_summon_stamp >> _is_on_underwater >> _cur_pet_aggro_state >> _need_feed >> _cur_pet_stay_state >> _cur_pet_counter >> _cur_pet_inhabit >> _cur_pet_mp_factor >> _cur_pet_mp >> _cur_pet_life >> _summon_skill_level;
}

void 
pet_manager::Swap(pet_manager & rhs)
{
	for(size_t i = 0; i < MAX_PET_CAPACITY + MAX_SUMMON_CAPACITY; i ++)
	{
		abase::swap(_pet_list[i] , rhs. _pet_list[i]);
	}

	abase::swap(_active_pet_slot,rhs._active_pet_slot);
	abase::swap(_cur_active_pet,rhs._cur_active_pet);
	abase::swap(_cur_pet_id,rhs._cur_pet_id);
	abase::swap(_pet_summon_stamp,rhs._pet_summon_stamp);
	abase::swap(_is_on_underwater,rhs._is_on_underwater);
	abase::swap(_cur_pet_aggro_state,rhs._cur_pet_aggro_state);
	abase::swap(_cur_pet_stay_state ,rhs._cur_pet_stay_state);
	abase::swap(_cur_pet_counter,rhs._cur_pet_counter);
	abase::swap(_need_feed,rhs._need_feed);
	abase::swap(_cur_pet_mp_factor,rhs._cur_pet_mp_factor);
	abase::swap(_cur_pet_mp,rhs._cur_pet_mp);
	abase::swap(_cur_pet_life,rhs._cur_pet_life);
	abase::swap(_summon_skill_level,rhs._summon_skill_level);
}

bool 
pet_manager::DBSetPetData(size_t index, const void * data, size_t size)
{
	if(index >= _active_pet_slot) return false;
	if(size != sizeof(pet_data)) return false;	//考虑增加版本控制 
	ASSERT(_pet_list[index] == NULL);
	int pet_id = ((const pet_data*)data)->pet_tid;
	if(world_manager::IsExpireItem(pet_id)) return false; //是应该消失的宠物
	
	if(_pet_list[index])
	{	
		abase::fastfree(_pet_list[index],sizeof(pet_data));
		_pet_list[index] = NULL;
	}
	pet_data * pData = (pet_data*)abase::fastalloc(sizeof(pet_data));
	memcpy(pData,data,sizeof(pet_data));
	_pet_list[index] = pData;
	return true;
}


int pet_manager::AddPetData(const pet_data & data)
{
	if(data.pet_class == pet_data::PET_CLASS_SUMMON
			|| data.pet_class == pet_data::PET_CLASS_PLANT) return -1;	//召唤物不是这样召唤的
	for(size_t i = 0; i < _active_pet_slot; i ++)
	{
		if(!_pet_list[i])
		{
			pet_data * pData = (pet_data*)abase::fastalloc(sizeof(pet_data));
			memcpy(pData,&data,sizeof(data));
			_pet_list[i] = pData;
			return (int)i;
		}
	}
	return -1;
}

void 
pet_manager::FreePet(gplayer_imp * pImp, size_t index)
{
	if(index >= _active_pet_slot || index >= MAX_PET_CAPACITY)
	{
		return;
	}
	ASSERT(_pet_list[index]);
	ASSERT(_cur_active_pet != (int)index);

	//通知客户端 
	pImp->_runner->free_pet(index,_pet_list[index]->pet_tid);
	
	//释放
	abase::fastfree(_pet_list[index],sizeof(pet_data));
	_pet_list[index] = NULL;
}

bool pet_manager::BanishPet(gplayer_imp * pImp, size_t index)
{
	if(index >= _active_pet_slot || index >= MAX_PET_CAPACITY)
	{
		return false;
	}
	if(!_pet_list[index]) return false;
	if(_cur_active_pet == (int)index) return false;

	//通知客户端 
	pImp->_runner->free_pet(index,_pet_list[index]->pet_tid);
	GLog::log(GLOG_INFO,"用户%d放生了宠物%d", pImp->_parent->ID.id, _pet_list[index]->pet_tid);
	
	//释放
	abase::fastfree(_pet_list[index],sizeof(pet_data));
	_pet_list[index] = NULL;
	return true;
}

void pet_manager::RecvExp(gplayer_imp * pImp, size_t index, int exp)
{
	pet_data * pData = GetPetData(index);
	if(!pData) return;
	if(index != (size_t)_cur_active_pet) return;	//如果当前未激活不得经验
//	if(pData->pet_tid != pet_id) return;		//如果当前宠物不符合不得经验， 考虑加上一个XID
	int max_lvl = pet_dataman::GetMaxLevel(pData->pet_tid);
	if(pData->level >= max_lvl ) return;		//这里可以考虑缓存当前宠物的最大级别，这样就不必每次都查询了 
	
	int cur_exp = pData->exp + exp;

	//检测是否升级
	bool lp = false;
	do
	{
		int lvl_exp = pet_dataman::GetLvlupExp(pData->level);
		if(cur_exp < lvl_exp) break;
		if(pData->level >= pImp->GetHistoricalMaxLevel()) 
		{
			//如果宠物等级高于人物等级则不再升级 但依然保留有数据
			cur_exp = lvl_exp;
			break;
		}
		lp = true;
		cur_exp -= lvl_exp;
		pData->level += 1;
		if(pData->level >= max_lvl)
		{
			cur_exp = 0;
			break;
		}
	}while(1);

	exp = cur_exp - pData->exp;
	pData->exp = cur_exp;
	if(exp && !lp)
	{
		pImp->_runner->pet_recv_exp(_cur_active_pet,pData->pet_tid,exp);
	}

	if(lp)
	{
		//此次经验会引发升级，将此操作转发到宠物处理之中
		size_t cls = (size_t)pData->pet_class;
		if(cls < pet_data::PET_CLASS_MAX)
		{
			__pet_imp[cls]->LevelUp(pImp, this, pData);
		}
		pImp->_runner->pet_level_up(_cur_active_pet,pData->pet_tid,pData->level, pData->exp);
	}
}

void 
pet_manager::OnMountSpeedEnChanged(gplayer_imp *pImp)
{
	if(!IsPetActive()) return;
	pet_data * pData = GetPetData(_cur_active_pet);
	if(pData == NULL) return;

	//根据宠物的类型来处理
	size_t cls = (size_t)pData->pet_class;
	if(cls == pet_data::PET_CLASS_MOUNT)
	{
		__pet_imp[cls]->OnMountSpeedEnChanged(pImp, this, pData);
	}
}

void 
pet_manager::ClientGetPetRoom(gplayer_imp * pImp)
{
	gplayer_dispatcher * pRunner = (gplayer_dispatcher*)pImp->_runner;
	pRunner->send_pet_room(_pet_list,0, MAX_PET_CAPACITY);
}

void 
pet_manager::DoHeartbeat(gplayer_imp * pImp)
{
	pet_data * pData = GetPetData(_cur_active_pet);
	if(!pData) return;

	size_t cls = (size_t)pData->pet_class;
	if(_need_feed)
	{
		//如果不处于无需喂养状态
		//进行喂养的处理
		pData->feed_period ++;
		if(pData->feed_period >= pet_data::FEED_TIME_UNIT)
		{
			int old_honor = pData->honor_point;
			//达到一个喂养时间单位
			HandleFeedTimeTick(pImp, pData);

			//更新忠诚度所带来的影响
			__pet_imp[cls]->OnHonorModify(pImp, this, pData,old_honor);

			pData->feed_period  = 0;
		}
	}

	__pet_imp[cls]->Heartbeat(pImp, this, pData);	//此操作可能会召回宠物，所以放在最后执行
	
	TryFreePet(pImp);
}

static struct 
{
	float honor_factor;		//喂养忠诚度修正因子
	int   feed_effect;		//喂一次的饥饿度改变
	int   honor_dec;		//一个时间单位之内不喂养改变的忠诚度
	int   hunger;			//一个时间单位内不喂养减少的饥饿度
}__pet_feed_param_list[pet_data::HUNGER_LEVEL_COUNT] = 
{
	{1.0f,	0,	0,	1},
	{0.8f,	1,	-1,	1},
	{0.6f,	1,	-5,	1},
	{0.6f,	1,	-5,	1},
	{0.8f,	2,	-15,	1},
	{0.8f,	2,	-15,	1},
	{0.8f,	2,	-15,	1},
	{0.6f,	3,	-50,	1},
	{0.6f,	3,	-50,	1},
	{0.6f,	3,	-50,	1},
	{0.6f,	3,	-50,	1},
	{0.3f,	4,	-100,	1},
};

void 
pet_manager::ModifyHonor(pet_data * pData, int offset)
{
	pData->honor_point += offset;
	if(pData->honor_point < 0) 
	{
		pData->honor_point  = 0;
	}
	else if (pData->honor_point > pet_data::HONOR_POINT_MAX)
	{
		pData->honor_point  = pet_data::HONOR_POINT_MAX;
	}
}

void 
pet_manager::ModifyHungerGauge(pet_data * pData, int offset)
{
	pData->hunger_gauge += offset;
	if(pData->hunger_gauge < 0) 
	{
		pData->hunger_gauge  = 0;
	}
	else if (pData->hunger_gauge >= pet_data::HUNGER_LEVEL_COUNT)
	{
		pData->hunger_gauge  = pet_data::HUNGER_LEVEL_COUNT - 1;
	}
}


void 
pet_manager::HandleFeedTimeTick(gplayer_imp * pImp, pet_data * pData)
{
	ASSERT(pData->hunger_gauge >=0 && pData->hunger_gauge < pet_data::HUNGER_LEVEL_COUNT);
	ASSERT(pData == _pet_list[_cur_active_pet]);

	//现在达到了一个时间单位而不喂养的程度 减少饥饿度和减少忠诚度 
	//先减少饥饿度
	ModifyHungerGauge(pData, __pet_feed_param_list[pData->hunger_gauge].hunger);
	//按照新的饥饿度减少忠诚度
	ModifyHonor(pData, __pet_feed_param_list[pData->hunger_gauge].honor_dec);

	//通知客户端
	pImp->_runner->notify_pet_honor(_cur_active_pet,pData->honor_point);
	pImp->_runner->notify_pet_hunger(_cur_active_pet,pData->hunger_gauge);
}

bool 
pet_manager::FeedCurPet(gplayer_imp * pImp, int food_type, int honor)
{
	pet_data * pData = GetPetData(_cur_active_pet);
	if(!pData) 
	{
		pImp->_runner->error_message(S2C::ERR_PET_IS_NOT_ACTIVE);
		return false;
	}
	//取出宠物数据，检查宠物食物类型
	//if(pData->
	const pet_data_temp * pTmp = pet_dataman::Get(pData->pet_tid);
	if(!pTmp || !(pTmp->food_mask & food_type))
	{
		pImp->_runner->error_message(S2C::ERR_PET_FOOD_TYPE_NOT_MATCH);
		return false;
	}
	int h = pData->hunger_gauge;

	int honor_offset = (int)(__pet_feed_param_list[h].honor_factor * honor + 0.5f);
	int old_honor = pData->honor_point;

	//先增加好感度
	ModifyHonor(pData, honor_offset);
	//再增加饱食度
	ModifyHungerGauge(pData, -__pet_feed_param_list[h].feed_effect);

	//通知客户端
	pImp->_runner->notify_pet_honor(_cur_active_pet,pData->honor_point);
	pImp->_runner->notify_pet_hunger(_cur_active_pet,pData->hunger_gauge);

	//更新忠诚度所带来的影响
	__pet_imp[pData->pet_class]->OnHonorModify(pImp, this, pData,old_honor);

	//清空当前的喂食记数
	pData->feed_period  = 0;
	return true;
}

bool
pet_manager::RelocatePos(gplayer_imp * pImp,const XID & who , int stamp,bool dis)
{
	if(!IsPetActive()) return false;
	pet_data * pData = GetPetData(_cur_active_pet);
	if(pData == NULL) return false;
	
	//由实现来完成此操作
	bool b = __pet_imp[pData->pet_class]->OnPetRelocate(pImp, this, pData,who, stamp,dis);
	if(b) TryFreePet(pImp);
	return b;
}

bool 
pet_manager::NotifyPetHP(gplayer_imp * pImp,const XID & who , int stamp,const msg_pet_hp_notify & info)
{
	if(!IsPetActive()) return false;
	pet_data * pData = GetPetData(_cur_active_pet);
	if(pData == NULL) return false;
	
	//由实现来完成此操作
	return __pet_imp[pData->pet_class]->OnPetNotifyHP(pImp, this, pData,who, stamp,info);

}

bool 
pet_manager::PetDeath(gplayer_imp * pImp,const XID & who , int stamp)
{
	if(!IsPetActive()) return false;
	pet_data * pData = GetPetData(_cur_active_pet);
	if(pData == NULL) return false;

	int index = _cur_active_pet;
	
	//由实现来完成此操作
	if(__pet_imp[pData->pet_class]->OnPetDeath(pImp, this, pData,who, stamp))
	{
		//正确，清空忠诚度
		ModifyHonor(pData, -(int)(pData->honor_point*0.10 + 0.5f));

		//通知客户端
		pImp->_runner->notify_pet_honor(index,pData->honor_point);
	}
	TryFreePet(pImp);
	return true;
}

bool 
pet_manager::PlayerPetCtrl(gplayer_imp * pImp,int cur_target,int pet_cmd, const void * buf, size_t size)
{
	if(!IsPetActive()) return false;
	pet_data * pData = GetPetData(_cur_active_pet);
	if(pData == NULL) return false;
	
	//由实现来完成此操作
	return __pet_imp[pData->pet_class]->OnPetCtrl(pImp, this, pData,cur_target,pet_cmd, buf, size);
}

void 
pet_manager::KillMob(gplayer_imp * pImp, int mob_level)
{
	if(!IsPetActive()) return ;
	pet_data * pData = GetPetData(_cur_active_pet);
	if(pData == NULL) return;
	
	//由实现来完成此操作
	return __pet_imp[pData->pet_class]->OnKillMob(pImp, this, pData,mob_level);
}

int
pet_manager::ResurrectPet(gplayer_imp * pImp, size_t index)
{
	pet_data * pData = GetPetData(index);
	if(pData == NULL) return S2C::ERR_PET_IS_NOT_EXIST;

	if(pData->hp_factor >0) return S2C::ERR_PET_IS_NOT_DEAD;
	pData->hp_factor = 0.1f;

	pImp->_runner->pet_revive(index, pData->hp_factor);
	return 0;
}

void 
pet_manager::NotifyMasterInfo(gplayer_imp * pImp, bool at_once)
{
	if(!IsPetActive()) return ;
	pet_data * pData = GetPetData(_cur_active_pet);
	if(pData == NULL) return;
	
	//由实现来完成此操作
	__pet_imp[pData->pet_class]->OnNotifyMasterInfo(pImp, this, pData,at_once);
}

void 
pet_manager::PreSwitchServer(gplayer_imp * pImp)
{
	if(!IsPetActive()) return ;
	pet_data * pData = GetPetData(_cur_active_pet);
	if(pData == NULL) return;
	
	//由实现来完成此操作
	__pet_imp[pData->pet_class]->PreSwitchServer(pImp, this, pData);
}

void 
pet_manager::PostSwitchServer(gplayer_imp * pImp)
{
	if(!IsPetActive()) return ;
	pet_data * pData = GetPetData(_cur_active_pet);
	if(pData == NULL) return;
	
	//由实现来完成此操作
	__pet_imp[pData->pet_class]->PostSwitchServer(pImp, this, pData);
	TryFreePet(pImp);
}

void 
pet_manager::PlayerBeAttacked(gplayer_imp * pImp, const XID & attacker)
{
	if(!IsPetActive()) return ;
	pet_data * pData = GetPetData(_cur_active_pet);
	if(pData == NULL) return;
	
	if(_cur_pet_aggro_state != gpet_imp::PET_AGGRO_DEFENSE) return;

	//由实现来完成此操作
	__pet_imp[pData->pet_class]->OnMasterBeAttacked(pImp, this, pData,attacker);
}

void
pet_manager::PetSetCoolDown(gplayer_imp * pImp,const XID & who,  int idx, int msec)
{
	if(!IsPetActive()) return ;
	if(_cur_pet_id != who) return;

	pImp->_runner->pet_set_cooldown(_cur_active_pet, idx, msec);
}


int 
pet_manager::ResurrectPet(gplayer_imp * pImp)
{
	for(size_t index = 0; index < MAX_PET_CAPACITY;index ++)
	{
		if(_pet_list[index] == NULL) continue;
		pet_data * pData = GetPetData(index);
		if(pData == NULL) return S2C::ERR_PET_IS_NOT_EXIST;
		if(pData->hp_factor >0) continue;

		pData->hp_factor = 0.1f;
		pImp->_runner->pet_revive(index, pData->hp_factor);
		return 0;
	}
	return S2C::ERR_PET_IS_NOT_DEAD;
}

void 
pet_manager::NotifyStartAttack(gplayer_imp *pImp, const XID & target, char force_attack)
{
	if(!IsPetActive()) return ;
	if(!_cur_pet_id.IsValid()) return;
	if(_cur_pet_aggro_state != gpet_imp::PET_AGGRO_AUTO  || _cur_pet_combat_state)  return;

	//给宠物发送攻击消息
	_cur_pet_combat_state = 1;
	pImp->SendTo<0>(GM_MSG_PET_AUTO_ATTACK,_cur_pet_id, force_attack, &target, sizeof(target));
}

int
pet_manager::ChangePetName(gplayer_imp * pImp, size_t index, const char * name, size_t name_len)
{
	pet_data * pData = GetPetData(index);
	if(pData == NULL) return S2C::ERR_PET_IS_NOT_EXIST;
	if(index == (size_t)_cur_active_pet)  return S2C::ERR_PET_IS_ALEARY_ACTIVE;

	//由实现来完成此操作
	if(!__pet_imp[pData->pet_class]->OnChangeName(pImp,this, pData,name,name_len))
	{
		return S2C::ERR_SERVICE_UNAVILABLE;
	}
	gplayer_dispatcher * pRunner = (gplayer_dispatcher*)pImp->_runner;
	pRunner->send_pet_room(&pData, index ,index + 1);
	return 0;
}

int
pet_manager::ForgetPetSkill(gplayer_imp * pImp, int skill_id)
{
	if(!IsPetActive()) return S2C::ERR_PET_IS_NOT_ACTIVE;
	pet_data * pData = GetPetData(_cur_active_pet);
	if(pData == NULL) return S2C::ERR_PET_IS_NOT_ACTIVE;

	int index = _cur_active_pet;

	if(!__pet_imp[pData->pet_class]->OnForgetSkill(pImp,this, pData, skill_id))
	{
		return S2C::ERR_SKILL_NOT_AVAILABLE;
	}

	gplayer_dispatcher * pRunner = (gplayer_dispatcher*)pImp->_runner;
	pRunner->send_pet_room(&pData, index ,index + 1);
	return 0;
}

int
pet_manager::LearnSkill(gplayer_imp * pImp,int skill_id, int * level_result)
{
	if(!IsPetActive()) return S2C::ERR_PET_IS_NOT_ACTIVE;
	pet_data * pData = GetPetData(_cur_active_pet);
	if(pData == NULL) return S2C::ERR_PET_IS_NOT_ACTIVE;

	int index = _cur_active_pet;

	if(!__pet_imp[pData->pet_class]->OnLearnSkill(pImp,this, pData, skill_id, level_result))
	{
		return S2C::ERR_SERVICE_UNAVILABLE;
	}

	gplayer_dispatcher * pRunner = (gplayer_dispatcher*)pImp->_runner;
	pRunner->send_pet_room(&pData, index ,index + 1);
	return 0;
}

void 
pet_manager::NotifyInvisibleData(gplayer_imp *pImp)
{
	if(!IsPetActive()) return;
	if(!_cur_pet_id.IsValid()) return;
	
	msg_invisible_data data;
	gplayer * pPlayer = (gplayer*)pImp->_parent;
	data.invisible_degree = pPlayer->invisible_degree;
	data.anti_invisible_degree = pPlayer->anti_invisible_degree;
	pImp->SendTo<0>(GM_MSG_NOTIFY_INVISIBLE_DATA,_cur_pet_id, 0, &data, sizeof(data));
}

bool 
pet_manager::ActivePet2(gplayer_imp * pImp, pet_data & data, int life,int skill_level)
{
	if(data.pet_class != pet_data::PET_CLASS_SUMMON) return false;
	//有宠物就先召回
	if(IsPetActive())
	{
		if(RecallPetWithoutFree(pImp))
			TryFreePet(pImp);
	}

	ASSERT(!IsPetActive());
	ASSERT(_pet_list[SUMMON_SLOT] == NULL);

	pet_data * pData = (pet_data*)abase::fastalloc(sizeof(pet_data));
	memcpy(pData,&data,sizeof(data));
	_pet_list[SUMMON_SLOT] = pData;
	_summon_skill_level = skill_level;
	
	extend_prop prop;
	_cur_pet_id = XID(-1,-1);
	if(__pet_imp[pData->pet_class]->DoActivePet(pImp, this, pData, &prop))
	{
		//设置激活标志
		_cur_active_pet = SUMMON_SLOT;
		_cur_pet_counter = 0;

		int pet_id = 0;
		if(_cur_pet_id.IsActive())
		{
			pet_id = _cur_pet_id.id;
		}
		_cur_pet_life = life;
		
		pImp->_runner->gain_pet(SUMMON_SLOT, pData,sizeof(pet_data));
		pImp->_runner->summon_pet(SUMMON_SLOT,pData->pet_tid,pet_id,_cur_pet_life);
		pImp->_runner->pet_property(SUMMON_SLOT,prop);
		return true;
	}
	else
	{
		abase::fastfree(_pet_list[SUMMON_SLOT],sizeof(pet_data));
		_pet_list[SUMMON_SLOT] = NULL;
		_summon_skill_level = 0;
		return false;
	}
}

void 
pet_manager::TryFreePet(gplayer_imp * pImp)
{
	if(_pet_list[SUMMON_SLOT] != NULL && _cur_active_pet != SUMMON_SLOT)
	{
		//通知客户端 
		pImp->_runner->free_pet(SUMMON_SLOT,_pet_list[SUMMON_SLOT]->pet_tid);
		
		//释放
		abase::fastfree(_pet_list[SUMMON_SLOT],sizeof(pet_data));
		_pet_list[SUMMON_SLOT] = NULL;
	}
}

bool 
pet_manager::PetSacrifice(gplayer_imp * pImp)
{
	if(_cur_active_pet != SUMMON_SLOT) return false; 
	pet_data * pData = GetPetData(_cur_active_pet);
	if(pData == NULL) return false;

	const pet_data_temp * pTemp = pet_dataman::Get(pData->pet_tid);
	if(pTemp == NULL || pTemp->sacrifice_skill == 0) return false;

	if(pImp->CastRune(pTemp->sacrifice_skill,_summon_skill_level))
	{
		if(RecallPetWithoutFree(pImp,PET_SACRIFICE))
			TryFreePet(pImp);
		return true;
	}
	return false;
}

bool pet_manager::DyePet(gplayer_imp * pImp, size_t index, unsigned short color)
{
	pet_data * pData = GetPetData(index);
	if(pData == NULL) return false;

	size_t cls = (size_t)pData->pet_class;
	if(cls < pet_data::PET_CLASS_MAX)
	{
		if(__pet_imp[cls]->OnDyePet(pImp, this, pData, color))
		{
			gplayer_dispatcher * pRunner = (gplayer_dispatcher*)pImp->_runner;
			pRunner->send_pet_room(&pData, index ,index + 1);
			return true;
		}
		else
		{
			return false;
		}
	}

	return false;
}

bool
pet_manager::EvolutionPet(gplayer_imp *pImp, size_t index,int evolution_id,int pet_nature,int skill1,int level1,int skill2,int level2)
{
	pet_data *pData = GetPetData(index);
	if(pData == NULL) return false;
	
	size_t cls = (size_t)pData->pet_class;
	if(cls < pet_data::PET_CLASS_MAX)
	{
		if(!__pet_imp[cls]->OnEvolution(pImp,this,pData,evolution_id,pet_nature,skill1,level1,skill2,level2))
		{
			return false;
		}
		gplayer_dispatcher * pRunner = (gplayer_dispatcher*)pImp->_runner;
		pRunner->send_pet_room(&pData, index ,index + 1);
		return true;
	}
	return false;
}

bool 
pet_manager::RebuildInheritRatio(int pet_id,int &r_attack,int &r_defense,int &r_hp,int &r_atk_lvl,int &r_def_lvl)
{
	const pet_data_temp *pTemp = pet_dataman::Get(pet_id);
	if(pTemp == NULL)
		return false;
	r_attack = abase::RandNormal(0,pTemp->max_r_attack);
	r_defense = abase::RandNormal(0,pTemp->max_r_defense);
	r_hp = abase::RandNormal(0,pTemp->max_r_hp);
	r_atk_lvl = abase::RandNormal(0,pTemp->max_r_atk_lvl);
	r_def_lvl = abase::RandNormal(0,pTemp->max_r_def_lvl);
	return true;
}

void
pet_manager::PetAcceptInheritRatioResult(gplayer_imp *pImp,size_t index,int r_attack,int r_defense,int r_hp,int r_atk_lvl,int r_def_lvl)
{
	pet_data * pData = GetPetData(index);
	if(pData == NULL) return;
	if(index == (size_t)_cur_active_pet)  return;
	pData->evo_prop.r_attack = r_attack;
	pData->evo_prop.r_defense = r_defense;
	pData->evo_prop.r_hp = r_hp;
	pData->evo_prop.r_atk_lvl = r_atk_lvl;
	pData->evo_prop.r_def_lvl = r_def_lvl;
	gplayer_dispatcher * pRunner = (gplayer_dispatcher*)pImp->_runner;
	pRunner->send_pet_room(&pData, index ,index + 1);
}

void
pet_manager::PetAcceptNatureResult(gplayer_imp *pImp,size_t index,int nature,int skill1,int level1,int skill2,int level2)
{
	pet_data * pData = GetPetData(index);
	if(pData == NULL) return;
	if(index == (size_t)_cur_active_pet)  return;
	int skill[2] = {0,0};
	pImp->GetNatureSkill(pData->evo_prop.nature,skill[0],skill[1]);
	pData->evo_prop.nature = nature;
	for(int i=0;i<pet_data::MAX_PET_SKILL_COUNT;i++)
	{
		if(pData->skills[i].skill == skill[0])
		{
			pData->skills[i].skill = skill1;
			pData->skills[i].level = level1;
			continue;
		}
		if(pData->skills[i].skill == skill[1])
		{
			pData->skills[i].skill = skill2;
			pData->skills[i].level = level2;
			continue;
		}
	}
	gplayer_dispatcher * pRunner = (gplayer_dispatcher*)pImp->_runner;
	pRunner->send_pet_room(&pData, index ,index + 1);
}

bool
pet_manager::AddExp(gplayer_imp *pImp,size_t index, int exp)
{
	pet_data * pData = GetPetData(index);
	if(!pData) return false;
	if(index != (size_t)_cur_active_pet) return false;	//如果当前未激活不得经验
	int max_lvl = pet_dataman::GetMaxLevel(pData->pet_tid);
	if(pData->level >= max_lvl) return false;

	if(pData->level >= pImp->GetHistoricalMaxLevel() && (size_t)pData->exp == pet_dataman::GetLvlupExp(pData->level))
		return false;

	int cur_exp = pData->exp + exp;
	bool lp = false;
	do
	{
		int lvl_exp = pet_dataman::GetLvlupExp(pData->level);
		if(cur_exp < lvl_exp) break;
		if(pData->level >= pImp->GetHistoricalMaxLevel()) 
		{
			//如果宠物等级高于人物等级则不再升级 但依然保留有数据
			cur_exp = lvl_exp;
			break;
		}
		lp = true;
		cur_exp -= lvl_exp;
		pData->level += 1;
		if(pData->level >= max_lvl)
		{
			cur_exp = 0;
			break;
		}
	}while(1);

	exp = cur_exp - pData->exp;
	pData->exp = cur_exp;
	if(exp && !lp)
	{
		pImp->_runner->pet_recv_exp(_cur_active_pet,pData->pet_tid,exp);
	}

	if(lp)
	{
		//此次经验会引发升级，将此操作转发到宠物处理之中
		size_t cls = (size_t)pData->pet_class;
		if(cls < pet_data::PET_CLASS_MAX)
		{
			__pet_imp[cls]->LevelUp(pImp, this, pData);
		}
		pImp->_runner->pet_level_up(_cur_active_pet,pData->pet_tid,pData->level, pData->exp);
	}
	return true;
}

int 
pet_manager::GetNormalSkillNum(gplayer_imp *pImp,pet_data *pData)
{
	int skill[2] = {0,0};
	int count = 0;
	pImp->GetNatureSkill(pData->evo_prop.nature,skill[0],skill[1]);
	const pet_data_temp *pTemp = pet_dataman::Get(pData->pet_tid);
	for(int i=0;i<pet_data::MAX_PET_SKILL_COUNT;i++)
	{
		int skillid = pData->skills[i].skill;
		if(skillid == skill[0] || skillid == skill[1] || skillid == pTemp->specific_skill_id)
			continue;
		if(pData->skills[i].skill == 0)
			break;
		count ++;
	}
	return count;
}

bool
pet_manager::IsSkillNormal(gplayer_imp *pImp,pet_data *pData,int skill_id)
{
	int skill[2] = {0,0};	
	pImp->GetNatureSkill(pData->evo_prop.nature,skill[0],skill[1]);
	if(skill_id == skill[0] || skill_id == skill[1])
		return false;
	const pet_data_temp *pTemp = pet_dataman::Get(pData->pet_tid);
	if(skill_id == pTemp->specific_skill_id)
		return false;
	return true;
}

//以下是植物宠管理器
bool 
plant_pet_manager::ActivePlant(gplayer_imp * pImp, pet_data & data, int life,int skill_level, const XID & target, char force_attack)
{
	if(data.pet_class != pet_data::PET_CLASS_PLANT) return false;
	
	//将植物招在目标旁边
	A3DVECTOR pos = pImp->_parent->pos;
	float dis = 0.f;
	if(target.IsActive() && target.id != pImp->_parent->ID.id)
	{
		world::object_info info;
		if(pImp->_plane->QueryObject(target,info))
		{
			pos = info.pos;
			dis = info.body_size;
		}
	}
	
	char inhabit_mode;
	const pet_data_temp * pTemp = pet_dataman::Get(data.pet_tid);
	//招出普通宠物的条件需要满足环境 水下不能招出所有宠物 空中无法招出地面宠物 
	if(pTemp == NULL || pTemp->group_limit <= 0 || pet_gen_pos::FindValidPos(pos,inhabit_mode,pImp->_layer_ctrl.GetLayer(), pTemp->inhabit_type,pImp->_plane,dis) != 0)
	{
		//召唤失败， 因为没有可以立足的地点
		pImp->_runner->error_message(S2C::ERR_SUMMON_PET_INVALID_POS);
		return false;
	}

	//检查技能：植物至少有一个技能，第一个技能是自动施放技能，第二个的是自爆技能
	if(data.skills[0].skill <= 0) return false;
	//自动释放技能的类型决定植物是攻击还是辅助
	char aggro_state; 
	char skill_type = GNET::SkillWrapper::GetType(data.skills[0].skill);
	if(skill_type == 1 || skill_type == 3)
		aggro_state = gpet_imp::PET_AGGRO_AUTO;		//攻击类植物
	else if(skill_type == 2)
		aggro_state = gpet_imp::PET_AGGRO_PASSIVE;	//辅助类植物
	else 
		return false;
	int honor_level = GetHonorLevel(data.honor_point);
	XID  who;
	object_interface oi(pImp);
	bool bRst = oi.CreatePet2(pos,inhabit_mode,&data,0/*无效的时辍*/, honor_level,who,aggro_state,gpet_imp::PET_STAY_STAY,skill_level);
	if(!bRst) return false;
			
	//宠物超上限了就先召回一个
	int count = 0, first = -1;
	for(size_t i=0; i<_plant_list.size(); i++)
	{
		if(_plant_list[i].plant_group == pTemp->plant_group)	
		{
			if(first == -1) first = i;
			++ count;
		}
	}
	if(count >= pTemp->group_limit)
	{
		PLANT_LIST::iterator it = _plant_list.begin() + first;
		pImp->SendTo<0>(GM_MSG_PET_DISAPPEAR,it->id,0/*无效的时辍*/);
		pImp->_runner->plant_pet_disappear(it->id.id, PLANT_GROUP_LIMIT);
		_plant_list.erase(it);		
	}
	
	plant_pet_data ppd;
	memset(&ppd, 0, sizeof(ppd));
	ppd.id = who;
	ppd.tid = data.pet_tid;
	ppd.plant_group = pTemp->plant_group;
	ppd.group_limit = pTemp->group_limit;
	ppd.life = life;
	ppd.hp_factor = 0.f;
	ppd.hp = 0;
	ppd.mp_factor = 0.f;
	ppd.mp = 0;
	ppd.plant_state = 0;
	ppd.is_suicide = (data.skills[1].skill > 0);
	_plant_list.push_back(ppd);
	
	pImp->_runner->summon_plant_pet(ppd.tid, ppd.id.id, ppd.life);
	//如果是供给类植物，则将目标设为攻击对象
	if(aggro_state == gpet_imp::PET_AGGRO_AUTO)
	{
		if(target.IsActive() && target.id != pImp->_parent->ID.id)
			pImp->SendTo<0>(GM_MSG_PET_AUTO_ATTACK, who, force_attack, &target, sizeof(target));
	}
	return true;	
}

bool 
plant_pet_manager::PlantSuicide(gplayer_imp * pImp, float distance, const XID & target, char force_attack)
{
	if(!_plant_list.size()) return false;

	PLANT_LIST::iterator it = _plant_list.end();
	for( ; it!=_plant_list.begin(); )
	{
		--it;
		if(it->is_suicide)
		{
			plant_pet_data & plant = *it;
			world::object_info info;
			 if(!pImp->_plane->QueryObject(plant.id,info)
					 || !(info.state & world::QUERY_OBJECT_STATE_ACTIVE)
					 || info.pos.squared_distance(pImp->_parent->pos) > distance*distance)
				 continue;
			
			pImp->SendTo<0>(GM_MSG_PLANT_PET_SUICIDE,plant.id,force_attack,&target,sizeof(target));
			pImp->_runner->plant_pet_disappear(plant.id.id, PLANT_SUICIDE);
			it = _plant_list.erase(it);
			return true;	
		}
	}
	return false;
}

void 
plant_pet_manager::Heartbeat(gplayer_imp * pImp)
{
	if(!_plant_list.size()) return;
	
	PLANT_LIST::iterator it = _plant_list.begin();
	for( ; it!=_plant_list.end(); )
	{
		plant_pet_data & plant = *it;
		//检查植物寿命
		if(plant.life > 0)
		{
			if(--plant.life <= 0)
			{
				pImp->SendTo<0>(GM_MSG_PET_DISAPPEAR,plant.id,0/*无效的时辍*/);
				pImp->_runner->plant_pet_disappear(it->id.id, PLANT_LIFE_EXHAUST);
				it = _plant_list.erase(it);
				continue;
			}		
		}
		//检查植物状态
		if(++plant.plant_state >= PET_STATE_THRESHOLD)
		{
			pImp->SendTo<0>(GM_MSG_PET_DISAPPEAR,plant.id,0/*无效的时辍*/);
			pImp->_runner->plant_pet_disappear(it->id.id, PLANT_OUT_OF_RANGE);
			it = _plant_list.erase(it);
			continue;
		}
		++it;
	}
	
	//通知master info
	if(++_plant_notify_counter >= NOTIFY_MASTER_TIME)
	{
		NotifyMasterInfo(pImp);
	}
}

void 
plant_pet_manager::NotifyMasterInfo(gplayer_imp * pImp)
{
	if(!_plant_list.size()) return;
	
	abase::vector<XID> list;
	list.reserve(_plant_list.size());
	for(size_t i=0; i<_plant_list.size(); i++)
	{
		list.push_back(_plant_list[i].id);	
	}
	pet_leader_prop data;
	pImp->SetPetLeaderData(data);
	MSG msg;
	BuildMessage(msg,GM_MSG_PET_MASTER_INFO,XID(-1,-1),pImp->_parent->ID,pImp->_parent->pos,0,&data,sizeof(data));
	pImp->_plane->SendMessage(list.begin(), list.end(), msg);
	_plant_notify_counter = 0;	
}

void 
plant_pet_manager::PreSwitchServer(gplayer_imp * pImp)
{
	//现在数据已经发出，数据的改变不会在目的服务器生效
	if(!_plant_list.size()) return;
	
	PLANT_LIST::iterator it = _plant_list.begin();
	for( ; it!=_plant_list.end(); ++it)
	{
		pImp->SendTo<0>(GM_MSG_PET_DISAPPEAR,it->id,0/*无效的时辍*/);
		pImp->_runner->plant_pet_disappear(it->id.id, PLANT_OUT_OF_RANGE);
	}
	_plant_list.clear();
}

void 
plant_pet_manager::NotifyStartAttack(gplayer_imp *pImp, const XID & target, char force_attack)
{
	if(!_plant_list.size()) return;

	abase::vector<XID> list;
	list.reserve(_plant_list.size());
	for(size_t i=0; i<_plant_list.size(); i++)
	{
		list.push_back(_plant_list[i].id);	
	}
	MSG msg;
	BuildMessage(msg,GM_MSG_PET_AUTO_ATTACK,XID(-1,-1),pImp->_parent->ID,pImp->_parent->pos,force_attack,&target,sizeof(target));
	pImp->_plane->SendMessage(list.begin(), list.end(), msg);
}

void 
plant_pet_manager::PlayerBeAttacked(gplayer_imp * pImp, const XID & target)
{
	if(!_plant_list.size()) return;

	abase::vector<XID> list;
	list.reserve(_plant_list.size());
	for(size_t i=0; i<_plant_list.size(); i++)
	{
		list.push_back(_plant_list[i].id);	
	}
	MSG msg;
	BuildMessage(msg,GM_MSG_MASTER_ASK_HELP,XID(-1,-1),pImp->_parent->ID,pImp->_parent->pos,0,&target,sizeof(target));
	pImp->_plane->SendMessage(list.begin(), list.end(), msg);
}

bool 
plant_pet_manager::PlantDeath(gplayer_imp * pImp,const XID & who , int stamp)
{
	if(!_plant_list.size()) return false;
	PLANT_LIST::iterator it = _plant_list.begin();
	for( ; it!=_plant_list.end(); ++it)
	{
		if(it->id == who)
		{
			pImp->_runner->plant_pet_disappear(it->id.id, PLANT_DEATH);
			_plant_list.erase(it);	
			return true;
		}
	}
	return false;
}

bool 
plant_pet_manager::NotifyPlantHP(gplayer_imp * pImp,const XID & who , int stamp,const msg_plant_pet_hp_notify & info)
{
	if(!_plant_list.size()) return false;
	PLANT_LIST::iterator it = _plant_list.begin();
	for( ; it!=_plant_list.end(); ++it)
	{
		if(it->id == who)
		{
			plant_pet_data& plant = *it;
			plant.plant_state = 0;
			if(plant.hp_factor != info.hp_ratio || plant.hp != info.cur_hp || plant.mp_factor != info.mp_ratio || plant.mp != info.cur_mp)
			{
				plant.hp_factor = info.hp_ratio;
				plant.hp = info.cur_hp;
				plant.mp_factor = info.mp_ratio;
				plant.mp = info.cur_mp;
				//通知客户端
				pImp->_runner->plant_pet_hp_notify(plant.id.id, info.hp_ratio,info.cur_hp,info.mp_ratio,info.cur_mp);
			}
			return true;
		}
	}
	return false;
}

bool plant_pet_manager::PlantDisappear(gplayer_imp * pImp,const XID & who , int stamp)
{
	if(!_plant_list.size()) return false;
	PLANT_LIST::iterator it = _plant_list.begin();
	for( ; it!=_plant_list.end(); ++it)
	{
		if(it->id == who)
		{
			pImp->SendTo<0>(GM_MSG_PET_DISAPPEAR,it->id,0/*无效的时辍*/);
			pImp->_runner->plant_pet_disappear(it->id.id, PLANT_OUT_OF_RANGE);
			_plant_list.erase(it);	
			return true;
		}
	}
	return false;
}

