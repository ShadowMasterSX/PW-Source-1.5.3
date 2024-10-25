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
			//�߶Ȳ��������Ѱ��
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
		//½��
		if(leader_layer != LAYER_GROUND) return false;
		break;
	case 1:
		//ˮ��
		if(leader_layer != LAYER_WATER) return false;
		break;
	case 2:
		//����
		if(world_manager::GetWorldLimit().nofly) return false;
		if(leader_layer == LAYER_WATER) return false;
		break;
	case 3:
		//�����ˮ��
		if(leader_layer == LAYER_AIR) return false;
		break;
	case 4:
		//����ӿ���
		if(leader_layer == LAYER_WATER) return false;
		break;
	case 5:
		//ˮ�¼ӿ���
		break;
	case 6:
		//��½��
		break;
	default:
		return false;
	}
	return true;
}

int pet_gen_pos::FindValidPos(A3DVECTOR & pos, char & inhabit_mode, char leader_layer, int inhabit_type, world * plane, float dis, A3DVECTOR offset)
{
	//�����Ƿ�����˳������
	switch(inhabit_type)
	{
	case 0:
		//½��
		break;
	case 1:
		//ˮ��
		break;
	case 2:
		//����
		if(world_manager::GetWorldLimit().nofly) return -2;
		break;
	case 3:
		//�����ˮ��
		break;
	case 4:
		//����ӿ���
		break;
	case 5:
		//ˮ�¼ӿ���
		break;
	case 6:
		//��½��
		break;
	default:
		return -2;
	}

	//���λ�ú�״̬�Ƿ�����������
	switch(inhabit_type)
	{
	case 0:
		//½��
		if(leader_layer != LAYER_GROUND) return -3;
		break;
	case 1:
		//ˮ��
		if(leader_layer != LAYER_WATER) return -3;
		break;
	case 2:
		//����
		if(leader_layer == LAYER_WATER) return -3;
		break;
	case 3:
		//�����ˮ��
		if(leader_layer == LAYER_AIR) return -3;
		break;
	case 4:
		//����ӿ���
		if(leader_layer == LAYER_WATER) return -3;
		break;
	case 5:
		//ˮ�¼ӿ���
		break;
	case 6:
		//��½��
		break;
	default:
		return -3;
	}

	//�ж��˴��Ƿ��ܹ������ܹ����������λ��
	int env_chk_seq[3] = {0};  //0x01�� 0x02�� 0x04ˮ
	switch(inhabit_type)
	{
	case 0:	
		//½��
		inhabit_mode = NPC_MOVE_ENV_ON_GROUND;
		return FindGroundPos(pos,dis,offset,plane)?0:-4;
	case 1:
		//ˮ��
		inhabit_mode = NPC_MOVE_ENV_IN_WATER;
		return FindWaterPos(pos,dis,offset,plane)?0:-4;
	case 2:
		//����
		inhabit_mode = NPC_MOVE_ENV_ON_AIR;
		return FindAirPos(pos,dis,offset,plane)?0:-4;
	case 3:
		//�����ˮ��
		if(leader_layer == LAYER_WATER){
			env_chk_seq[0] = 0x04;
			env_chk_seq[1] = 0x01;
		}else{
			env_chk_seq[0] = 0x01;
			env_chk_seq[1] = 0x04;
		}
		break;
	case 4:
		//����ӿ���
		if(leader_layer == LAYER_AIR){
			env_chk_seq[0] = 0x02;
			env_chk_seq[1] = 0x01;
		}else{
			env_chk_seq[0] = 0x01;
			env_chk_seq[1] = 0x02;
		}
		break;
	case 5:
		//ˮ�¼ӿ���
		if(leader_layer == LAYER_WATER){
			env_chk_seq[0] = 0x04;
			env_chk_seq[1] = 0x02;
		}else{
			env_chk_seq[0] = 0x02;
			env_chk_seq[1] = 0x04;
		}
		break;
	case 6:
		//��½��
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
	//���ܳ�����������λ��
	for(int i=0; i<3; i++)
	{
		if(!env_chk_seq[i]) break;
		if(env_chk_seq[i] == 0x01)	//��
		{
			if(FindGroundPos(pos,dis,offset,plane))
			{
				inhabit_mode = NPC_MOVE_ENV_ON_GROUND;
				return 0;
			}
		}
		else if(env_chk_seq[i] == 0x02)	//��
		{
			if(world_manager::GetWorldLimit().nofly) continue;
			if(FindAirPos(pos,dis,offset,plane))
			{
				inhabit_mode = NPC_MOVE_ENV_ON_AIR;
				return 0;
			}
		}
		else if(env_chk_seq[i] == 0x04)	//ˮ
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
		//��ͼ�������
		//�ܹ��������������ǣ�
		//����ˮ�У���������
		if(pImp->GetPlayerLimit(PLAYER_LIMIT_NOMOUNT))
		{
			pImp->_runner->error_message(S2C::ERR_PET_CAN_NOT_MOUNT);
			return false;
		}

		//�����ڵ���
		if(!pImp->_layer_ctrl.IsOnGround())
		{
			pImp->_runner->error_message(S2C::ERR_PET_CAN_NOT_MOUNT);
			return false;
		}

		//���в���
		if(pImp->_filters.IsFilterExist(FILTER_FLY_EFFECT))
		{
			pImp->_runner->error_message(S2C::ERR_PET_CAN_NOT_MOUNT);
			return false;
		}

		//ˮ�в���
		if(pImp->IsUnderWater())
		{
			pImp->_runner->error_message(S2C::ERR_PET_CAN_NOT_MOUNT);
			return false;
		}

		//�����²���
		if(pImp->GetForm())
		{
			pImp->_runner->error_message(S2C::ERR_PET_CAN_NOT_MOUNT);
			return false;
		}
		
		//�����²���
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

		//�����ٳ��Ĳ���
		//����ȥ�����б��в�ѯ�����������ٶȣ�Ȼ������ҳ϶ȼ������ĵ������
		float speedup = 3.0f;
		//��ѯ�ͼ�������ٶ�
		if(!pet_dataman::CalcMountParam(pData->pet_tid,pData->level,speedup))
		{
			pImp->_runner->error_message(S2C::ERR_PET_CAN_NOT_MOUNT);
			//�޷���ѯ�������ٶȣ���ʾ���ڲ���ȷ������
			return false;
		}
		int honor_level = GetHonorLevel(pData->honor_point);
		float drop_rate = GetDropRate(honor_level);

		speedup += pImp->GetMountSpeedEnhance(); 
		//�����Ѿ���ѯ���
		pImp->_filters.AddFilter(new mount_filter(pImp,FILTER_INDEX_MOUNT_FILTER,
					pData->pet_tid,pData->color,speedup,drop_rate));
		pMan->SetTestUnderWater(true);
		pMan->_cur_pet_id = XID(-1,-1);
		return true;
	}

	virtual bool DoRecallPet(gplayer_imp * pImp, pet_manager * pMan,pet_data * pData)
	{
		//�����ٻ�����������
		//ͨ��ɾ��filter������������״̬
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
			//��ѯ�ͼ�������ٶ�
			if(!pet_dataman::CalcMountParam(pData->pet_tid,pData->level, speedup))
			{
				//ע���ҳ϶ȸı�ҲҪӰ������������
				//�����ҳ϶���δ���
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
		//��������Ĳ����ǣ�����Ƿ����ָ����filter��������������Merge����
		if(pImp->_filters.IsFilterExist(FILTER_INDEX_MOUNT_FILTER))
		{
			float speedup = 3.0f;
			//��ѯ�ͼ�������ٶ�
			if(!pet_dataman::CalcMountParam(pData->pet_tid,pData->level, speedup))
			{
				//ע���ҳ϶ȸı�ҲҪӰ������������
				//�����ҳ϶���δ���
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
		if(pMan->_cur_pet_counter >= 300)		//ÿ5����һ�ξ���
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
			//����
			float speedup = 3.0f;

			//��ѯ�ͼ�������ٶ�
			if(!pet_dataman::CalcMountParam(pData->pet_tid,pData->level,speedup))
			{
				//�޷���ѯ�������ٶȣ���ʾ���ڲ���ȷ������
				return ;
			}

			//���³����ƶ��ٶȺ͵������
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
		//��������״̬���޷����ٻ�
		if(pData->hp_factor == 0)
		{
			pImp->_runner->error_message(S2C::ERR_CANNOT_SUMMON_DEAD_PET);
			return false;
		}

		XID  who;
		A3DVECTOR pos = pImp->_parent->pos;
		char inhabit_mode;
		const pet_data_temp * pTemp = pet_dataman::Get(pData->pet_tid);
		//�г���ͨ�����������Ҫ���㻷�� ˮ�²����г����г��� �����޷��г�������� 
		if(pTemp == NULL || pet_gen_pos::FindValidPos(pos,inhabit_mode,pImp->_layer_ctrl.GetLayer(), pTemp->inhabit_type, pImp->_plane) != 0)
		{
			//�ٻ�ʧ�ܣ� ��Ϊû�п�������ĵص�
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

		//����һ��ֹͣ��Ϣ��Ŀ�����
		pImp->SendTo<0>(GM_MSG_PET_DISAPPEAR,pMan->_cur_pet_id,pMan->_pet_summon_stamp);
		
		pMan->_cur_pet_id = XID(-1,-1);
		return true;
	}

	virtual void TestUnderWater(gplayer_imp * pImp, pet_manager * pMan,pet_data * pData,float offset)
	{
		//ս�費����ˮ��
	}

	virtual void OnMountSpeedEnChanged(gplayer_imp *pImp,pet_manager * pMan, pet_data * pData){}
	virtual void LevelUp(gplayer_imp *pImp, pet_manager * pMan,pet_data * pData)
	{
		//ս�������Ĳ����� ֪ͨ�����µļ���
		int level = pData->level;
		pImp->SendTo<0>(GM_MSG_PET_LEVEL_UP,pMan->_cur_pet_id,pMan->_pet_summon_stamp,&level,sizeof(level));
	}
	
	virtual void Heartbeat(gplayer_imp *pImp, pet_manager * pMan,pet_data * pData)
	{
		pMan->_cur_pet_counter ++;
		if(pMan->_cur_pet_counter >= 15)	//ÿ15��ͬ��һ���Լ������ݸ�����
		{
			if(pMan->_cur_pet_state <= 0)
			{
				//����15��û�����ݵ������򽫳����Զ��ջ�
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
			//ÿ��16���ж�һ�γ����Ƿ��ܹ���������
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
			//ÿ4�뷢���Լ���λ�ø�����
			char layer = pImp->_layer_ctrl.GetLayer();
			pImp->SendTo<0>(GM_MSG_MASTER_NOTIFY_LAYER, pMan->_cur_pet_id, pMan->_pet_summon_stamp, &layer,sizeof(layer));
		}

		pMan->_cur_pet_notify_counter ++;
		if(pMan->_cur_pet_notify_counter >= pet_manager::NOTIFY_MASTER_TIME)
		{
			//�����Լ������ݸ�����
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
		
		//֪ͨ�����Լ���honor����
		pImp->SendTo<0>(GM_MSG_PET_HONOR_MODIFY,pMan->_cur_pet_id,pMan->_pet_summon_stamp,&hl,sizeof(hl));
	}
	
	virtual bool OnPetRelocate(gplayer_imp *pImp, pet_manager * pMan,pet_data *pData,const XID &who,int stamp,bool is_disappear)
	{
		if(pMan->_cur_pet_id != who) return false;
		if(pMan->_pet_summon_stamp != stamp) return false;
		//��������������ܼ���

		msg_pet_pos_t data;
		data.pos = pImp->_parent->pos;
		const pet_data_temp * pTemp = pet_dataman::Get(pData->pet_tid);
		if(is_disappear || pTemp == NULL || pet_gen_pos::FindValidPos(data.pos,data.inhabit_mode,pImp->_layer_ctrl.GetLayer(), pTemp->inhabit_type, pImp->_plane) != 0)
		{
			//�޷��ҵ����ʵ����� ���߳���Ҫ����ʧ
			pMan->RecallPetWithoutFree(pImp);
			return true;
		}
		//�ҵ��˺��ʵ����귢�͸��Է�
		pImp->SendTo<0>(GM_MSG_PET_CHANGE_POS, who, stamp,&data,sizeof(data));
		return true;
	}

	virtual bool OnPetNotifyHP(gplayer_imp *pImp, pet_manager * pMan,pet_data *pData,const XID &who,int stamp,const msg_pet_hp_notify & info)
	{
		if(pMan->_cur_pet_id != who) return false;
		if(pMan->_pet_summon_stamp != stamp) return false;
		//��������������ܼ���

		//����֪ͨ��������һ
		pMan->_cur_pet_state ++;

		if(pData->hp_factor != info.hp_ratio || pMan->_cur_pet_hp != info.cur_hp
				|| pMan->_cur_pet_mp_factor != info.mp_ratio || pMan->_cur_pet_mp != info.cur_mp)
		{
			//���³����Ѫֵ
			pData->hp_factor = info.hp_ratio;
			pMan->_cur_pet_hp = info.cur_hp;
			pMan->_cur_pet_mp_factor = info.mp_ratio;
			pMan->_cur_pet_mp = info.cur_mp;
			//֪ͨ�ͻ���
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
			//֪ͨ�ͻ���
			pImp->_runner->pet_ai_state(pMan->_cur_pet_aggro_state,pMan->_cur_pet_stay_state);
		}
		return true;
	}

	virtual bool OnPetDeath(gplayer_imp *pImp, pet_manager * pMan,pet_data *pData,const XID &who,int stamp)
	{
		if(pMan->_cur_pet_id != who) return false;
		if(pMan->_pet_summon_stamp != stamp) return false;
		//��������������ܼ���

		//���³����Ѫֵ
		pData->hp_factor = 0;
		int index = pMan->GetCurActivePet();
		pMan->_cur_pet_hp = 0;

		//�ٻس���
		pMan->RecallPetWithoutFree(pImp,pet_manager::PET_DEATH);

		//�����ҳϵȲ������������

		//֪ͨ�ͻ���
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
		//���������Ѿ����������ݵĸı䲻����Ŀ�ķ�������Ч
		if(!pMan->_cur_pet_id.IsActive()) 
		{
			return;
		}

		//��ʱ��Ҫ֪ͨ����Ӧ����ʧ ��ת����ɺ��������ٻ�����
		pImp->SendTo<0>(GM_MSG_PET_DISAPPEAR,pMan->_cur_pet_id,pMan->_pet_summon_stamp);
	}

	virtual void PostSwitchServer(gplayer_imp * pImp, pet_manager * pMan, pet_data * pData)
	{
		//�����������ٻ�����
		XID  who;
		A3DVECTOR pos = pImp->_parent->pos;
		char inhabit_mode;
		//�г���ͨ�����������Ҫ���㻷�� ˮ�²����г����г��� �����޷��г�������� 
		const pet_data_temp * pTemp = pet_dataman::Get(pData->pet_tid);
		if(pTemp == NULL || pet_gen_pos::FindValidPos(pos,inhabit_mode,pImp->_layer_ctrl.GetLayer(), pTemp->inhabit_type, pImp->_plane) != 0)
		{
			//û�п�������ĵص㣬�������ٻ�
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

		//�Ƿ����Ʒ��ʹ�������  �����Ȳ��� �Ժ���Կ��ǣ� ���Լ�����Ϣ��Ŀ
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

				//���¼���
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
				//���ڱ�֤�����б��в����пն�
				//ǰ���ɾ�����ܲ�������ն�
				break;
			}
			if(pData->skills[i].skill == skill_id)
			{
				index = i;
			}
		}
		if(pMan->GetNormalSkillNum(pImp,pData)>= 4 && index == -1) return false;	//������＼�ܴﵽ4����ֻ���������ܲ���ѧϰ�¼���
		
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

		//���¼���
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
		//��ý���id
		const pet_data_temp * pTemp = pet_dataman::Get(pData->pet_tid);
		const pet_data_temp * eTemp = pet_dataman::Get(evolution_pet_id);
		if(pTemp == NULL || pTemp->evolution_id == 0 || eTemp->pet_class != pet_data::PET_CLASS_EVOLUTION)
			return false;
		//�޸�data
		pData->pet_tid = evolution_pet_id;
		pData->pet_egg_tid = pTemp->evolution_id;
		pData->pet_class = pet_data::PET_CLASS_EVOLUTION; 
		//���ϵ��
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
		//���������Ѿ����������ݵĸı䲻����Ŀ�ķ�������Ч
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
		//�г���ͨ�����������Ҫ���㻷�� ˮ�²����г����г��� �����޷��г�������� 
		if(pTemp == NULL || pet_gen_pos::FindValidPos(pos,inhabit_mode,pImp->_layer_ctrl.GetLayer(), pTemp->inhabit_type, pImp->_plane) != 0)
		{
			//�ٻ�ʧ�ܣ� ��Ϊû�п�������ĵص�
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
		//�����������ٻ�����
		XID  who;
		A3DVECTOR pos = pImp->_parent->pos;
		char inhabit_mode;
		//�г���ͨ�����������Ҫ���㻷�� ˮ�²����г����г��� �����޷��г�������� 
		const pet_data_temp * pTemp = pet_dataman::Get(pData->pet_tid);
		if(pTemp == NULL || pet_gen_pos::FindValidPos(pos,inhabit_mode,pImp->_layer_ctrl.GetLayer(), pTemp->inhabit_type, pImp->_plane) != 0)
		{
			//û�п�������ĵص㣬�������ٻ�
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
		//��������״̬���޷����ٻ�
		if(pData->hp_factor == 0)
		{
			pImp->_runner->error_message(S2C::ERR_CANNOT_SUMMON_DEAD_PET);
			return false;
		}

		XID  who;
		A3DVECTOR pos = pImp->_parent->pos;
		char inhabit_mode;
		const pet_data_temp * pTemp = pet_dataman::Get(pData->pet_tid);
		//�г���ͨ�����������Ҫ���㻷�� ˮ�²����г����г��� �����޷��г�������� 
		if(pTemp == NULL || pet_gen_pos::FindValidPos(pos,inhabit_mode,pImp->_layer_ctrl.GetLayer(), pTemp->inhabit_type, pImp->_plane) != 0)
		{
			//�ٻ�ʧ�ܣ� ��Ϊû�п�������ĵص�
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
		//�г���ͨ�����������Ҫ���㻷�� ˮ�²����г����г��� �����޷��г�������� 
		if(pTemp == NULL || pet_gen_pos::FindValidPos(pos,inhabit_mode,pImp->_layer_ctrl.GetLayer(), pTemp->inhabit_type, pImp->_plane) != 0)
		{
			//û�п�������ĵص㣬�������ٻ�
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
//�ٻ�����
bool 
pet_manager::ActivePet(gplayer_imp * pImp, size_t index)
{
	//player ��״̬����������ж�
	//���Ƚ��л����ж�
	//if(IsPetActive()) return false;
	//�г�������ٻ�
	if(IsPetActive())
	{
		if(RecallPetWithoutFree(pImp))
			TryFreePet(pImp);
	}
	pet_data * pData = GetPetData(index);
	if(pData == NULL) return false;
	
	//���ݳ�����������ٻ�����
	//�������裬������ҽ������״̬
	//�����ս����߹��ͳ裬 ��ų�����NPC
	size_t cls = (size_t)pData->pet_class;
	if(cls < pet_data::PET_CLASS_MAX)
	{
		_cur_pet_id = XID(-1,-1);
		extend_prop prop;
		if(__pet_imp[cls]->DoActivePet(pImp, this, pData, &prop))
		{
			//���ü����־
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
	//player ��״̬����������ж�
	//���Ƚ��л����ж�
	if(!IsPetActive()) return false;
	pet_data * pData = GetPetData(_cur_active_pet);
	if(pData == NULL) return false;

	//���ݳ��������������
	size_t cls = (size_t)pData->pet_class;
	if(cls < pet_data::PET_CLASS_MAX)
	{
		if(__pet_imp[cls]->DoRecallPet(pImp, this, pData))
		{
			//�����ٻز���
			pImp->_runner->recall_pet(_cur_active_pet,pData->pet_tid,reason);
			_cur_active_pet = -1;
			_cur_pet_id = XID(-1,-1);

			//�ٻغ��޸��ٻ���ʱ���
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

	//���ݳ��������������
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
	if(size != sizeof(pet_data)) return false;	//�������Ӱ汾���� 
	ASSERT(_pet_list[index] == NULL);
	int pet_id = ((const pet_data*)data)->pet_tid;
	if(world_manager::IsExpireItem(pet_id)) return false; //��Ӧ����ʧ�ĳ���
	
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
			|| data.pet_class == pet_data::PET_CLASS_PLANT) return -1;	//�ٻ��ﲻ�������ٻ���
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

	//֪ͨ�ͻ��� 
	pImp->_runner->free_pet(index,_pet_list[index]->pet_tid);
	
	//�ͷ�
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

	//֪ͨ�ͻ��� 
	pImp->_runner->free_pet(index,_pet_list[index]->pet_tid);
	GLog::log(GLOG_INFO,"�û�%d�����˳���%d", pImp->_parent->ID.id, _pet_list[index]->pet_tid);
	
	//�ͷ�
	abase::fastfree(_pet_list[index],sizeof(pet_data));
	_pet_list[index] = NULL;
	return true;
}

void pet_manager::RecvExp(gplayer_imp * pImp, size_t index, int exp)
{
	pet_data * pData = GetPetData(index);
	if(!pData) return;
	if(index != (size_t)_cur_active_pet) return;	//�����ǰδ����þ���
//	if(pData->pet_tid != pet_id) return;		//�����ǰ���ﲻ���ϲ��þ��飬 ���Ǽ���һ��XID
	int max_lvl = pet_dataman::GetMaxLevel(pData->pet_tid);
	if(pData->level >= max_lvl ) return;		//������Կ��ǻ��浱ǰ�������󼶱������Ͳ���ÿ�ζ���ѯ�� 
	
	int cur_exp = pData->exp + exp;

	//����Ƿ�����
	bool lp = false;
	do
	{
		int lvl_exp = pet_dataman::GetLvlupExp(pData->level);
		if(cur_exp < lvl_exp) break;
		if(pData->level >= pImp->GetHistoricalMaxLevel()) 
		{
			//�������ȼ���������ȼ��������� ����Ȼ����������
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
		//�˴ξ�����������������˲���ת�������ﴦ��֮��
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

	//���ݳ��������������
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
		//�������������ι��״̬
		//����ι���Ĵ���
		pData->feed_period ++;
		if(pData->feed_period >= pet_data::FEED_TIME_UNIT)
		{
			int old_honor = pData->honor_point;
			//�ﵽһ��ι��ʱ�䵥λ
			HandleFeedTimeTick(pImp, pData);

			//�����ҳ϶���������Ӱ��
			__pet_imp[cls]->OnHonorModify(pImp, this, pData,old_honor);

			pData->feed_period  = 0;
		}
	}

	__pet_imp[cls]->Heartbeat(pImp, this, pData);	//�˲������ܻ��ٻس�����Է������ִ��
	
	TryFreePet(pImp);
}

static struct 
{
	float honor_factor;		//ι���ҳ϶���������
	int   feed_effect;		//ιһ�εļ����ȸı�
	int   honor_dec;		//һ��ʱ�䵥λ֮�ڲ�ι���ı���ҳ϶�
	int   hunger;			//һ��ʱ�䵥λ�ڲ�ι�����ٵļ�����
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

	//���ڴﵽ��һ��ʱ�䵥λ����ι���ĳ̶� ���ټ����Ⱥͼ����ҳ϶� 
	//�ȼ��ټ�����
	ModifyHungerGauge(pData, __pet_feed_param_list[pData->hunger_gauge].hunger);
	//�����µļ����ȼ����ҳ϶�
	ModifyHonor(pData, __pet_feed_param_list[pData->hunger_gauge].honor_dec);

	//֪ͨ�ͻ���
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
	//ȡ���������ݣ�������ʳ������
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

	//�����Ӻøж�
	ModifyHonor(pData, honor_offset);
	//�����ӱ�ʳ��
	ModifyHungerGauge(pData, -__pet_feed_param_list[h].feed_effect);

	//֪ͨ�ͻ���
	pImp->_runner->notify_pet_honor(_cur_active_pet,pData->honor_point);
	pImp->_runner->notify_pet_hunger(_cur_active_pet,pData->hunger_gauge);

	//�����ҳ϶���������Ӱ��
	__pet_imp[pData->pet_class]->OnHonorModify(pImp, this, pData,old_honor);

	//��յ�ǰ��ιʳ����
	pData->feed_period  = 0;
	return true;
}

bool
pet_manager::RelocatePos(gplayer_imp * pImp,const XID & who , int stamp,bool dis)
{
	if(!IsPetActive()) return false;
	pet_data * pData = GetPetData(_cur_active_pet);
	if(pData == NULL) return false;
	
	//��ʵ������ɴ˲���
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
	
	//��ʵ������ɴ˲���
	return __pet_imp[pData->pet_class]->OnPetNotifyHP(pImp, this, pData,who, stamp,info);

}

bool 
pet_manager::PetDeath(gplayer_imp * pImp,const XID & who , int stamp)
{
	if(!IsPetActive()) return false;
	pet_data * pData = GetPetData(_cur_active_pet);
	if(pData == NULL) return false;

	int index = _cur_active_pet;
	
	//��ʵ������ɴ˲���
	if(__pet_imp[pData->pet_class]->OnPetDeath(pImp, this, pData,who, stamp))
	{
		//��ȷ������ҳ϶�
		ModifyHonor(pData, -(int)(pData->honor_point*0.10 + 0.5f));

		//֪ͨ�ͻ���
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
	
	//��ʵ������ɴ˲���
	return __pet_imp[pData->pet_class]->OnPetCtrl(pImp, this, pData,cur_target,pet_cmd, buf, size);
}

void 
pet_manager::KillMob(gplayer_imp * pImp, int mob_level)
{
	if(!IsPetActive()) return ;
	pet_data * pData = GetPetData(_cur_active_pet);
	if(pData == NULL) return;
	
	//��ʵ������ɴ˲���
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
	
	//��ʵ������ɴ˲���
	__pet_imp[pData->pet_class]->OnNotifyMasterInfo(pImp, this, pData,at_once);
}

void 
pet_manager::PreSwitchServer(gplayer_imp * pImp)
{
	if(!IsPetActive()) return ;
	pet_data * pData = GetPetData(_cur_active_pet);
	if(pData == NULL) return;
	
	//��ʵ������ɴ˲���
	__pet_imp[pData->pet_class]->PreSwitchServer(pImp, this, pData);
}

void 
pet_manager::PostSwitchServer(gplayer_imp * pImp)
{
	if(!IsPetActive()) return ;
	pet_data * pData = GetPetData(_cur_active_pet);
	if(pData == NULL) return;
	
	//��ʵ������ɴ˲���
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

	//��ʵ������ɴ˲���
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

	//�����﷢�͹�����Ϣ
	_cur_pet_combat_state = 1;
	pImp->SendTo<0>(GM_MSG_PET_AUTO_ATTACK,_cur_pet_id, force_attack, &target, sizeof(target));
}

int
pet_manager::ChangePetName(gplayer_imp * pImp, size_t index, const char * name, size_t name_len)
{
	pet_data * pData = GetPetData(index);
	if(pData == NULL) return S2C::ERR_PET_IS_NOT_EXIST;
	if(index == (size_t)_cur_active_pet)  return S2C::ERR_PET_IS_ALEARY_ACTIVE;

	//��ʵ������ɴ˲���
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
	//�г�������ٻ�
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
		//���ü����־
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
		//֪ͨ�ͻ��� 
		pImp->_runner->free_pet(SUMMON_SLOT,_pet_list[SUMMON_SLOT]->pet_tid);
		
		//�ͷ�
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
	if(index != (size_t)_cur_active_pet) return false;	//�����ǰδ����þ���
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
			//�������ȼ���������ȼ��������� ����Ȼ����������
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
		//�˴ξ�����������������˲���ת�������ﴦ��֮��
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

//������ֲ��������
bool 
plant_pet_manager::ActivePlant(gplayer_imp * pImp, pet_data & data, int life,int skill_level, const XID & target, char force_attack)
{
	if(data.pet_class != pet_data::PET_CLASS_PLANT) return false;
	
	//��ֲ������Ŀ���Ա�
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
	//�г���ͨ�����������Ҫ���㻷�� ˮ�²����г����г��� �����޷��г�������� 
	if(pTemp == NULL || pTemp->group_limit <= 0 || pet_gen_pos::FindValidPos(pos,inhabit_mode,pImp->_layer_ctrl.GetLayer(), pTemp->inhabit_type,pImp->_plane,dis) != 0)
	{
		//�ٻ�ʧ�ܣ� ��Ϊû�п�������ĵص�
		pImp->_runner->error_message(S2C::ERR_SUMMON_PET_INVALID_POS);
		return false;
	}

	//��鼼�ܣ�ֲ��������һ�����ܣ���һ���������Զ�ʩ�ż��ܣ��ڶ��������Ա�����
	if(data.skills[0].skill <= 0) return false;
	//�Զ��ͷż��ܵ����;���ֲ���ǹ������Ǹ���
	char aggro_state; 
	char skill_type = GNET::SkillWrapper::GetType(data.skills[0].skill);
	if(skill_type == 1 || skill_type == 3)
		aggro_state = gpet_imp::PET_AGGRO_AUTO;		//������ֲ��
	else if(skill_type == 2)
		aggro_state = gpet_imp::PET_AGGRO_PASSIVE;	//������ֲ��
	else 
		return false;
	int honor_level = GetHonorLevel(data.honor_point);
	XID  who;
	object_interface oi(pImp);
	bool bRst = oi.CreatePet2(pos,inhabit_mode,&data,0/*��Ч��ʱ�*/, honor_level,who,aggro_state,gpet_imp::PET_STAY_STAY,skill_level);
	if(!bRst) return false;
			
	//���ﳬ�����˾����ٻ�һ��
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
		pImp->SendTo<0>(GM_MSG_PET_DISAPPEAR,it->id,0/*��Ч��ʱ�*/);
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
	//����ǹ�����ֲ���Ŀ����Ϊ��������
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
		//���ֲ������
		if(plant.life > 0)
		{
			if(--plant.life <= 0)
			{
				pImp->SendTo<0>(GM_MSG_PET_DISAPPEAR,plant.id,0/*��Ч��ʱ�*/);
				pImp->_runner->plant_pet_disappear(it->id.id, PLANT_LIFE_EXHAUST);
				it = _plant_list.erase(it);
				continue;
			}		
		}
		//���ֲ��״̬
		if(++plant.plant_state >= PET_STATE_THRESHOLD)
		{
			pImp->SendTo<0>(GM_MSG_PET_DISAPPEAR,plant.id,0/*��Ч��ʱ�*/);
			pImp->_runner->plant_pet_disappear(it->id.id, PLANT_OUT_OF_RANGE);
			it = _plant_list.erase(it);
			continue;
		}
		++it;
	}
	
	//֪ͨmaster info
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
	//���������Ѿ����������ݵĸı䲻����Ŀ�ķ�������Ч
	if(!_plant_list.size()) return;
	
	PLANT_LIST::iterator it = _plant_list.begin();
	for( ; it!=_plant_list.end(); ++it)
	{
		pImp->SendTo<0>(GM_MSG_PET_DISAPPEAR,it->id,0/*��Ч��ʱ�*/);
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
				//֪ͨ�ͻ���
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
			pImp->SendTo<0>(GM_MSG_PET_DISAPPEAR,it->id,0/*��Ч��ʱ�*/);
			pImp->_runner->plant_pet_disappear(it->id.id, PLANT_OUT_OF_RANGE);
			_plant_list.erase(it);	
			return true;
		}
	}
	return false;
}

