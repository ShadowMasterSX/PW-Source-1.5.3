#include "../clstab.h"
#include "../world.h"
#include "../player_imp.h"
#include "item_elf.h"

DEFINE_SUBSTANCE(elf_item, item_body, CLS_ITEM_ELF)		//CLS��clstab.h�ж���

//�����ȼ���Ӧ�ľ�������Ч����ÿ�����ı�׼ֵ
refine_effect elf_refine_effect_table[MAX_ELF_REFINE_LEVEL+1] = 
{
	{0, 0, 0, 0},//no use
	
	{45, 0, 0, 10},	//level 1
	{100, 0, 0, 10},
	{165, 0, 0, 10},
	{240, 0, 0, 10},
	{325, 0, 0, 10},
	{420, 0, 0, 10},
	{420, 1, 0, 10},
	{420, 2, 0, 20},
	{420, 2, 1, 30},
	{420, 2, 2, 40},
	
	{420, 3, 3, 50}, //11
	{420, 4, 4, 60},
	{420, 6, 5, 70},
	{420, 9, 6, 80},
	{420, 12, 7, 90},
	{420, 16, 8, 100},
	{420, 20, 9, 110},
	{420, 24, 11, 120},
	{420, 28, 13, 130},
	{420, 32, 15, 140},
	
	{420, 36, 17, 150}, //21
	{420, 41, 19, 160},
	{420, 46, 21, 170},
	{420, 51, 24, 180},
	{420, 56, 27, 190},
	{420, 61, 30, 200},
	{420, 66, 33, 210},
	{420, 72, 36, 220},
	{420, 78, 39, 230},
	{420, 84, 42, 240},
	
	{420, 90, 45, 250}, //31
	{420, 96, 48, 260},
	{420, 102, 51, 270},
	{420, 108, 54, 280},
	{420, 114, 57, 290},
	{420, 120, 60, 300}
};
//ʹ����ͨ��������0(ʧ�ܵȼ���0)Ŀ�꾫���ȼ���Ӧ�ĳɹ���
float elf_refine_succ_prob_ticket0[MAX_ELF_REFINE_LEVEL+1] = { 0.0f,//no use
		1.0f, 0.4f, 0.315789474f, 0.338983051f, 0.377358491f, 0.417754569f, 0.463208685f, 0.510769231f, 0.55f, 0.55f,
		0.55f, 0.55f, 0.55f, 0.55f, 0.55f, 0.55f, 0.55f, 0.55f, 0.55f, 0.55f, 
		0.55f, 0.55f, 0.55f, 0.55f, 0.55f, 0.55f, 0.55f, 0.55f, 0.55f, 0.55f, 
		0.55f, 0.55f, 0.55f, 0.55f, 0.55f, 0.55f};
//ʹ�����絤(��߳ɹ��ʣ�ʧ�ܵȼ���0)Ŀ�꾫���ȼ���Ӧ�ĳɹ���
float elf_refine_succ_prob_ticket1[MAX_ELF_REFINE_LEVEL+1] = { 0.0f,//no use
		1.0f, 0.6f, 0.368421053f, 0.355932203f, 0.383647799f, 0.420365535f, 0.464414958f, 0.511384615f, 0.557001027f, 0.60032861f, 
		0.640836732f, 0.678052261f, 0.7f, 0.7f, 0.7f, 0.7f, 0.7f, 0.7f, 0.7f, 0.7f, 
		0.7f, 0.7f, 0.7f, 0.7f, 0.7f, 0.7f, 0.7f, 0.7f, 0.7f, 0.7f, 
		0.7f, 0.7f, 0.7f, 0.7f, 0.7f, 0.7f};
//ʹ�����ϵ�(���ɹ��ʣ�ʧ�ܵȼ���1)Ŀ�꾫���ȼ���Ӧ�ĳɹ���
float elf_refine_succ_prob_ticket2[MAX_ELF_REFINE_LEVEL+1] = { 0.0f,//no use
		1.0f, 0.6f, 0.333333333f, 0.296296296f, 0.3f, 0.314814815f, 0.337313433f, 0.360708535f, 0.381453155f, 0.4f, 
		0.4f, 0.4f, 0.4f, 0.4f, 0.4f, 0.4f, 0.4f, 0.4f, 0.4f, 0.4f, 
		0.4f, 0.4f, 0.4f, 0.4f, 0.4f, 0.4f, 0.4f, 0.4f, 0.4f, 0.4f, 
		0.4f, 0.4f, 0.4f, 0.4f, 0.4f, 0.4f};
//ʹ���λõ�(�ɹ�����ʹ�ø����������и������ޣ�ʧ�ܵȼ�����)Ŀ�꾫���ȼ���Ӧ�����ʹ�������ɹ���=��������/���Էŵ��������
int elf_refine_max_use_ticket3[MAX_ELF_REFINE_LEVEL+1] = { 0,//no use
		1 , 2 , 7 , 20 , 50 , 112 , 223 , 398 , 648 , 974 , 
		1366 , 1806 , 2271 , 2739 , 3191 , 3612 , 3993 , 4331 , 4623 , 4873 , 
		5084 , 5260 , 5405 , 5525 , 5623 , 5702 , 5767 , 5819 , 5861 , 5895 , 
		5923 , 5945 , 5962 , 5977 , 5988 , 5997};
//����ת�����ĵ�Ǭ��ʯ��Ŀ
int elf_refine_transmit_cost[MAX_ELF_REFINE_LEVEL+1] = {0, //no use
		1, 2, 7, 20, 50, 110, 220, 390, 500, 550, 
		600, 650, 700, 750, 800, 850, 900, 950, 1000, 1050, 
		1100, 1150, 1200, 1250, 1300, 1350, 1400, 1450, 1500, 1550, 
		1600, 1650, 1700, 1750, 1800, 1850};
//��С����ע�뾭��ʱ����������С����ȼ���Ӧ��������/�����Ӧ��������=ע�뾭��ʱ����Ч����
int elf_exp_loss_constant[MAX_PLAYER_LEVEL+1] = {0, //no use
		1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 
		11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 
		21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 
		31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 
		41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 
		51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 
		61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 
		71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 
		81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 
		91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 
		100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 
		100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 
		100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 
		100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 
		100, 100, 100, 100, 100, 100, 100, 100, 100, 100};

static int refine_failed_type[] = 
{
	item::REFINE_SUCCESS,
	item::REFINE_FAILED_LEVEL_0, 
	item::REFINE_FAILED_LEVEL_1,
	item::REFINE_FAILED_LEVEL_2,
};

bool elf_item::SaveEssence(archive & ar)
{
	ar << ess;
	return true;
}

bool elf_item::LoadEssence(archive & ar)
{
	ar >> ess;
	if(ess.level < 1 || ess.level > player_template::GetMaxLevel()	
		|| ess.total_attribute < ess.level-1 || ess.total_attribute > ess.level-1+int(ess.level/10)*100
		|| ess.strength < 0 || ess.agility < 0 || ess.vitality < 0 || ess.energy < 0
		|| (ess.strength>ATTRIBUTE_UPPER_LIMIT ? ess.strength+ess.strength-ATTRIBUTE_UPPER_LIMIT : ess.strength)
		+(ess.agility>ATTRIBUTE_UPPER_LIMIT ? ess.agility+ess.agility-ATTRIBUTE_UPPER_LIMIT : ess.agility)
		+(ess.vitality>ATTRIBUTE_UPPER_LIMIT ? ess.vitality+ess.vitality-ATTRIBUTE_UPPER_LIMIT : ess.vitality)
		+(ess.energy>ATTRIBUTE_UPPER_LIMIT ? ess.energy+ess.energy-ATTRIBUTE_UPPER_LIMIT : ess.energy) > ess.total_attribute
		|| ess.total_genius != (ess.level <= 100 ? ess.level/5+1 : 21+(ess.level-100))		
		|| ess.genius[0] < 0 || ess.genius[1] < 0 || ess.genius[2] < 0 || ess.genius[3] < 0 || ess.genius[4] < 0 
		|| ess.genius[0] > GENIUS_UPPER_LIMIT || ess.genius[1] > GENIUS_UPPER_LIMIT || ess.genius[2] > GENIUS_UPPER_LIMIT || ess.genius[3] > GENIUS_UPPER_LIMIT || ess.genius[4] > GENIUS_UPPER_LIMIT
		|| ess.genius[0] + ess.genius[1] + ess.genius[2] + ess.genius[3] + ess.genius[4] > ess.total_genius
		|| ess.refine_level < 0 || ess.refine_level > MAX_ELF_REFINE_LEVEL
		|| ess.stamina < 0)		//stamina����status_value�������
			throw -201;
	return true;	
}

bool elf_item::SaveEquip(archive & ar)
{	
	ar << equipvect.size();
	abase::vector<unsigned int, abase::fast_alloc<> >::iterator it = equipvect.begin(), ite = equipvect.end();
	for( ; it != ite; it++)
		ar << *it;
	return true;
}

bool elf_item::LoadEquip(archive & ar)
{
	int size = -1;
	ar >> size;
	if(size < 0 || size > MAX_ELF_EQUIP_CNT)
		throw -202;
	unsigned int equipid;
	for(int i=0; i< size; i++)
	{
		ar >> equipid;
		equipvect.push_back(equipid);
	}
	return true;	
}

bool elf_item::SaveSkill(archive & ar)
{
	ar << skillvect.size();
	abase::vector<struct elf_skill_data, abase::fast_alloc<> >::iterator it = skillvect.begin(), ite = skillvect.end();
	for( ; it != ite; it++)
		ar << *it;	
	return true;	
}

bool elf_item::LoadSkill(archive & ar)
{
	int size = -1;
	ar >> size;
	if(size < 0 || size > MAX_ELF_SKILL_CNT)
		throw -203;
	struct elf_skill_data sk;
	for(int i=0; i< size; i++)
	{
		ar >> sk;
		if(sk.level <= 0)
			throw -203;
		skillvect.push_back(sk);
	}
	return true;	
}

bool elf_item::Save(archive & ar)
{
	try
	{
		SaveEssence(ar);
		SaveEquip(ar);
		SaveSkill(ar);
	}
	catch(...)
	{
		return false;	
	}
	return true;
}

bool elf_item::Load(archive & ar)
{
	ASSERT(_tid > 0);
	ASSERT(ar.offset() == 0);
	_raw_data.clear();
	_raw_data.push_back(ar.data(), ar.size());
	
	try
	{
		LoadEssence(ar);
		LoadEquip(ar);
		LoadSkill(ar);
	}
	catch(...)
	{
		GLog::log(GLOG_ERR,"��ҵ�С����%d���ݴ���!ess data:exp=%u level=%d total_attr=%d str=%d agi=%d vit=%d eng=%d total_genius=%d g0=%d g1=%d g2=%d g3=%d g4=%d refine_lvl=%d stamina=%d status_value=%d",
		_tid, ess.exp, ess.level, ess.total_attribute, ess.strength, ess.agility, ess.vitality, ess.energy, ess.total_genius, ess.genius[0], ess.genius[1], ess.genius[2], ess.genius[3], ess.genius[4], ess.refine_level, ess.stamina, ess.status_value);
		return false;	
	}
	ASSERT(ar.is_eof());

	if(!UpdateEssenceData()) return false;
	if(!UpdateEquipData()) return false;
	if(!UpdateSkillData()) return false;

	CalcCRC();
	return true;
}


bool elf_item::UpdateEssenceData()	//��ȡ����exp_factor,����elf_extend_prop
{
	DATA_TYPE dt;
	struct GOBLIN_ESSENCE * elf = (struct GOBLIN_ESSENCE *)world_manager::GetDataMan().get_data_ptr(_tid, ID_SPACE_ESSENCE, dt);	
	if(elf == NULL || dt != DT_GOBLIN_ESSENCE)
		return false;
	ASSERT(elf->exp_factor <= 2.45f);
	prop.exp_factor = elf->exp_factor;
	float average_per_level = 0.0f;
	int max_per_level = 0;
	for(int i=0; i<10; i++)
		if(elf->rand_prop[i].rand_rate > 0 && elf->rand_prop[i].rand_num > 0)
		{
			average_per_level += elf->rand_prop[i].rand_rate * elf->rand_prop[i].rand_num;
			if(elf->rand_prop[i].rand_num > max_per_level) 
				max_per_level = elf->rand_prop[i].rand_num;	
		}
	prop.max_rand_prop = max_per_level * 10;
	prop.average_rand_prop = int(average_per_level*10 +0.5f);
	
	prop.cur_strength = elf->init_strength;
	prop.cur_agility = elf->init_agility;
	prop.cur_vitality = elf->init_tili;
	prop.cur_energy = elf->init_energy;
	memset(&prop.cur_genius[0], 0, 5 * sizeof(prop.cur_genius[0]));

	prop.cur_strength += ess.strength;
	prop.cur_agility += ess.agility;
	prop.cur_vitality += ess.vitality;
	prop.cur_energy += ess.energy;
	prop.cur_genius[0] += ess.genius[0];
	prop.cur_genius[1] += ess.genius[1];
	prop.cur_genius[2] += ess.genius[2];
	prop.cur_genius[3] += ess.genius[3];
	prop.cur_genius[4] += ess.genius[4];
	
	if(ess.status_value > 0)
		prop.secure_status = STATUS_TRANSFORM; //���ﲻ��ת��״̬�����ж��ˣ���gplayer_imp::Heartbeat()ʵ��
	else if(ess.status_value == 0)
		prop.secure_status = STATUS_SECURE;
	else
		prop.secure_status = STATUS_TRADABLE;
	
	return true;
}

bool elf_item::UpdateEquipData()	//��ȡװ�����ͣ�����ȼ�,����elf_extend_prop
{
	size_t size = equipvect.size();
	
	DATA_TYPE dt;
	struct GOBLIN_EQUIP_ESSENCE * eq;
	for(size_t i=0; i<size; i++)
	{
		eq = (struct GOBLIN_EQUIP_ESSENCE *)world_manager::GetDataMan().get_data_ptr(equipvect[i], ID_SPACE_ESSENCE, dt);
		if(eq == NULL || dt != DT_GOBLIN_EQUIP_ESSENCE)
			return false;		//��װ�������ڸ���δ���
		if(ess.level < eq->req_goblin_level)		//������װ������
			return false;	
		if(prop.all_equip_mask & (1 << eq->equip_type)) //�Ѵ���ͬ����װ���ٶ�equip_type 0-3
			return false;
		prop.all_equip_mask |= (1 << eq->equip_type);

		prop.cur_strength += eq->strength;
		prop.cur_agility += eq->agility;
		prop.cur_vitality += eq->tili;
		prop.cur_energy += eq->energy;
		prop.cur_genius[0] += eq->magic[0];
		prop.cur_genius[1] += eq->magic[1];
		prop.cur_genius[2] += eq->magic[2];
		prop.cur_genius[3] += eq->magic[3];
		prop.cur_genius[4] += eq->magic[4];
		
	}
	if(prop.cur_genius[0] > GENIUS_UPPER_LIMIT)	prop.cur_genius[0] = GENIUS_UPPER_LIMIT;
	if(prop.cur_genius[1] > GENIUS_UPPER_LIMIT)	prop.cur_genius[1] = GENIUS_UPPER_LIMIT;
	if(prop.cur_genius[2] > GENIUS_UPPER_LIMIT)	prop.cur_genius[2] = GENIUS_UPPER_LIMIT;
	if(prop.cur_genius[3] > GENIUS_UPPER_LIMIT)	prop.cur_genius[3] = GENIUS_UPPER_LIMIT;
	if(prop.cur_genius[4] > GENIUS_UPPER_LIMIT)	prop.cur_genius[4] = GENIUS_UPPER_LIMIT;
	
	return true;
}

bool elf_item::UpdateSkillData()
{
	//prop.cur_skill_slot = INITIAL_SKILL_SLOT + prop.cur_energy/NEED_ENERGY_PER_SKILL_SLOT;
	//if(prop.cur_skill_slot > MAX_ELF_SKILL_CNT)
	//	prop.cur_skill_slot = MAX_ELF_SKILL_CNT;
	//if(skillvect.size() > (size_t)prop.cur_skill_slot)
	//	return false;
	//else
	//	return true;
	//��ѧ����������Ϊ��������Ե�������ڸ���ǰ���ֵľ������ѧ���ܿ��ܳ������ֵ����ʱ����ļ��ܲ�����
	const int ss_table[] = {50,70,80,90,100}; 
	int cur_rand_prop = ess.total_attribute - ess.level + 1;
	int i=0;
	for( ; (size_t)i<sizeof(ss_table)/sizeof(int); i++)
	{
		if(cur_rand_prop <= ss_table[i]) break;	
	}
	prop.cur_skill_slot = 4 + i;
	if(prop.cur_skill_slot > MAX_ELF_SKILL_CNT)
		prop.cur_skill_slot = MAX_ELF_SKILL_CNT;
	return true;
}

void elf_item::ClearData()	//�����������elf_extend_prop
{
	memset(&prop, 0, sizeof(prop));
}

void elf_item::OnRefreshItem()
{
	//�������Ӧ�ò��ô����쳣��������ķ���ֵ
	//����prop
	ClearData();
	UpdateEssenceData();
	UpdateEquipData();
	UpdateSkillData();
	//����ess_stamina
	ess.stamina += stamina_offset;
	stamina_offset = 0;
	//����_raw_data
	_raw_data.clear();
	raw_wrapper rw;
	Save(rw);
	rw.swap(_raw_data);
	CalcCRC();

	//dump_all();
}

void elf_item::OnRefreshRawData()
{
	//����ess_stamina
	ess.stamina += stamina_offset;
	stamina_offset = 0;
	//����_raw_data
	_raw_data.clear();
	raw_wrapper rw;
	Save(rw);
	rw.swap(_raw_data);
	CalcCRC();
	
	//dump_all();
}


bool elf_item::VerifyRequirement(item_list & list, gactive_imp* imp)
{
	if(list.GetLocation() == item::BODY)
	{
		return (imp->GetHistoricalMaxLevel() >= ess.level);		
	}
	else
		return false;	
}

void elf_item::OnActivate(item::LOCATION, size_t pos, size_t count, gactive_imp* imp)
{
	struct elf_skill_data skilldata[MAX_ELF_SKILL_CNT];
	int skillcnt = MAX_ELF_SKILL_CNT;
	CheckActiveSkill(skilldata, skillcnt, imp);
	imp->UpdateCurElfInfo(_tid, ess.refine_level, prop.cur_strength, prop.cur_agility, prop.cur_vitality, prop.cur_energy, 
		(const char *)skilldata, skillcnt);
}
void elf_item::OnDeactivate(item::LOCATION, size_t pos, size_t count,gactive_imp* imp)
{
	imp->ClearCurElfInfo();
}

void elf_item::OnPutIn(item::LOCATION l, item_list & list, size_t pos, size_t count,gactive_imp* obj)
{
	if(l == item::BODY) 
	{
		Activate(l, list, pos, count, obj);
	}
}

void elf_item::OnTakeOut(item::LOCATION l, size_t pos, size_t count, gactive_imp* obj)
{
	if(l == item::BODY) 
	{
		if(obj->IsElfRefineEffectActive())//���С���龫��Ч�������ˣ�����ȥ��
			obj->TriggerElfRefineEffect();
		obj->ClearCurElfVigor();
		Deactivate(l, pos, count,obj);
	}
}

void elf_item::CheckActiveSkill(struct elf_skill_data skilldata[], int & skillcnt, gactive_imp* imp)//skillcntֵ-���ز���
{
	struct SKILL::elf_requirement elf;
	elf.elf_level = ess.level;
	elf.str = prop.cur_strength; 
	elf.agi = prop.cur_agility;
	elf.vit = prop.cur_vitality;
	elf.eng = prop.cur_energy;
	elf.genius[0] = prop.cur_genius[0];
	elf.genius[1] = prop.cur_genius[1];
	elf.genius[2] = prop.cur_genius[2];
	elf.genius[3] = prop.cur_genius[3];
	elf.genius[4] = prop.cur_genius[4];
	
	int arraysize = skillcnt;
	skillcnt = 0;
	size_t size = skillvect.size();
	for(size_t i=0; i<size && skillcnt<prop.cur_skill_slot && skillcnt<arraysize; i++)
	{
		if(GNET::SkillWrapper::IsElfSkillActive(skillvect[i].id, skillvect[i].level, elf, object_interface(imp)))
		{
			skilldata[skillcnt++] = skillvect[i];			
		}
	}
}

int elf_item::OnUse(item::LOCATION l,gactive_imp * imp,size_t count)
{
	if(l != item::BODY || !IsActive()) 
	{
		imp->_runner->error_message(S2C::ERR_ELF_REFINE_ACTIVATE_FAILED);
		return -1;
	}
	imp->TriggerElfRefineEffect();
	
	return 0;
}

bool elf_item::AddAttributePoint(short str, short agi, short vit, short eng, bool ischeck)
{
	if(str < 0 || agi < 0 || vit < 0 || eng < 0 
		|| (str == 0 && agi == 0 && vit == 0 && eng == 0) )
		return false;
	int req_point = 0, temp;
	temp = str + ess.strength;
	req_point += (temp>ATTRIBUTE_UPPER_LIMIT ? temp+temp-ATTRIBUTE_UPPER_LIMIT : temp);
	temp = agi + ess.agility;
	req_point += (temp>ATTRIBUTE_UPPER_LIMIT ? temp+temp-ATTRIBUTE_UPPER_LIMIT : temp);
	temp = vit + ess.vitality;
	req_point += (temp>ATTRIBUTE_UPPER_LIMIT ? temp+temp-ATTRIBUTE_UPPER_LIMIT : temp);
	temp = eng + ess.energy;
	req_point += (temp>ATTRIBUTE_UPPER_LIMIT ? temp+temp-ATTRIBUTE_UPPER_LIMIT : temp);
	if(req_point > ess.total_attribute)
		return false;
	if(ischeck)
		return true;
		
	ess.strength += str;
	ess.agility += agi;
	ess.vitality += vit;
	ess.energy += eng;

	OnRefreshItem();
	return true;	
}
bool elf_item::DecAttributePoint(short str, short agi, short vit, short eng)
{
	if(str < 0 || agi < 0 || vit < 0 || eng < 0
	|| str > ess.strength || agi > ess.agility || vit > ess.vitality || eng > ess.energy
	|| (str == 0 && agi == 0 && vit == 0 && eng == 0) )
		return false;
	//��ֹϴ���Ե��ļ���������С����ѧ�ļ�����
	//int new_skill_slot = INITIAL_SKILL_SLOT + (prop.cur_energy-eng)/NEED_ENERGY_PER_SKILL_SLOT;
	//if(new_skill_slot > MAX_ELF_SKILL_CNT)
	//	new_skill_slot = MAX_ELF_SKILL_CNT;
	//if(skillvect.size() > (size_t)new_skill_slot)
	//	return false;

	ess.strength -= str;
	ess.agility -= agi;
	ess.vitality -= vit;
	ess.energy -= eng;

	OnRefreshItem();	
	return true;
}
bool elf_item::AddGeniusPoint(short g0, short g1, short g2, short g3, short g4, bool ischeck)
{
	if(g0 < 0 || g1 < 0 || g2 < 0 || g3 < 0 || g4 < 0
		|| g0 > GENIUS_UPPER_LIMIT-prop.cur_genius[0] || g1 > GENIUS_UPPER_LIMIT-prop.cur_genius[1] 
		|| g2 > GENIUS_UPPER_LIMIT-prop.cur_genius[2] || g3 > GENIUS_UPPER_LIMIT-prop.cur_genius[3] 
		|| g4 > GENIUS_UPPER_LIMIT-prop.cur_genius[4] 
		|| (g0 == 0 && g1 == 0 && g2 == 0 && g3 == 0 && g4 == 0) )
		return false;
	if(g0 + g1 + g2 + g3 + g4 + 
		ess.genius[0] + ess.genius[1] + ess.genius[2] + ess.genius[3] + ess.genius[4] > ess.total_genius) //���ݲ���Խ��	
		return false;
	if(ischeck)
		return true;
	
	ess.genius[0] += g0;
	ess.genius[1] += g1;
	ess.genius[2] += g2;
	ess.genius[3] += g3;
	ess.genius[4] += g4;

	OnRefreshItem();	
	return true;
}
bool elf_item::FlushGeniusPoint()
{
	if(ess.genius[0] + ess.genius[1] + ess.genius[2] + ess.genius[3] + ess.genius[4] == 0)
		return false;

	ess.genius[0] = ess.genius[1] = ess.genius[2] = ess.genius[3] = ess.genius[4] = 0;	

	OnRefreshItem();	
	return true;
}
int elf_item::LearnSkill(gactive_imp * imp, unsigned short skill_id)
{
	//�ü���С�����Ƿ���ѧ
	short skill_level = 0;
	int index = -1;
	size_t size = skillvect.size();
	for(size_t i=0; i<size; i++)
	{
		if(skillvect[i].id == skill_id)	
		{
			ASSERT(skillvect[i].level > 0);
			index = i;
			skill_level = skillvect[i].level;
			break;
		}	
	}
	//���С����û��ѧ�����ж��Ƿ��ܲ�����
	if(index < 0 && size >= (size_t)prop.cur_skill_slot)
		return -1;
	//ѧϰ
	struct SKILL::elf_requirement elf;
	elf.elf_level = ess.level;
	elf.str = prop.cur_strength; 
	elf.agi = prop.cur_agility;
	elf.vit = prop.cur_vitality;
	elf.eng = prop.cur_energy;
	elf.genius[0] = prop.cur_genius[0];
	elf.genius[1] = prop.cur_genius[1];
	elf.genius[2] = prop.cur_genius[2];
	elf.genius[3] = prop.cur_genius[3];
	elf.genius[4] = prop.cur_genius[4];
	int new_level;
	if( (new_level = GNET::SkillWrapper::ElfLearnSkill(skill_id, skill_level + 1, elf, object_interface(imp))) <= 0)
		return -1;
	ASSERT(new_level == skill_level + 1);
	//���¼�������
	if(index < 0)
	{
		struct elf_skill_data _sk;
		_sk.id = skill_id;
		_sk.level = new_level;
		skillvect.push_back(_sk);
	}
	else
		skillvect[index].level = new_level;
	
	OnRefreshRawData();
	return new_level;
}
int elf_item::ForgetSkill(gactive_imp * imp, unsigned short skill_id, short forget_level)
{
	//С������ѧ�������Ƿ�����ü���
	SKILL_VECT::iterator it = skillvect.begin(), ite = skillvect.end();
	short level = 0;
	for( ; it != ite; it++)
	{
		if(it->id == skill_id)	
		{
			ASSERT(it->level > 0);
			level = it->level;
			break;
		}	
	}
	if(it == ite)
		return -1;
	//�����ȼ��Ƿ���ȷ
	if(forget_level <= 0 || forget_level > level)
		return -1;
	//����
	if(level == forget_level)
	{
		skillvect.erase(it);
	}
	else
	{
		it->level -= forget_level;	
	}
	
	OnRefreshRawData();
	return (level - forget_level);
}

int elf_item::ElfRefine(int ticket_id, int ticket_cnt, int& original_level)
{	
	//����ʱ��֤��ticket_id��ȷ�ԣ�1 <= ticket_cnt <= max_use
	ASSERT(ess.refine_level >= 0 && ess.refine_level <= MAX_ELF_REFINE_LEVEL);
	
	//������ﵽ������ȼ�
	if(ess.refine_level >= MAX_ELF_REFINE_LEVEL)
		return item::REFINE_CAN_NOT_REFINE;
	//����ԭʼlevel
	original_level = ess.refine_level;
	//ȷ���ɹ���
	//�ֱ��Ӧ REFINE_SUCCESS REFINE_FAILED_LEVEL_0 REFINE_FAILED_LEVEL_1 REFINE_FAILED_LEVEL_2
	float prob[4] = {0.0f, 0.0f, 0.0f, 0.0f};	
	if(ticket_id == ELF_REFINE_TICKET0_ID)
	{
		prob[0] = elf_refine_succ_prob_ticket0[ess.refine_level+1];
		prob[3] = 1.0f - prob[0];
	}
	else if(ticket_id == ELF_REFINE_TICKET1_ID)
	{
		prob[0] = elf_refine_succ_prob_ticket1[ess.refine_level+1];
		prob[3] = 1.0f - prob[0];
	}
	else if(ticket_id == ELF_REFINE_TICKET2_ID)
	{
		prob[0] = elf_refine_succ_prob_ticket2[ess.refine_level+1];
		prob[2] = 1.0f - prob[0];
	}
	else if(ticket_id == ELF_REFINE_TICKET3_ID)
	{
		ASSERT(ticket_cnt >= 1 && ticket_cnt <= elf_refine_max_use_ticket3[ess.refine_level+1]);
		prob[0] = (float)ticket_cnt / (float)elf_refine_max_use_ticket3[ess.refine_level+1];
		prob[1] = 1.0f - prob[0];
	}
	else
	{
		ASSERT(false);
		return item::REFINE_CAN_NOT_REFINE;
	}
	for(int i=0; i<4; i++)
	{
		if(prob[i] < 0)	prob[i] = 0;
		if(prob[i] > 1)	prob[i] = 1;
	}
	
	int rst = abase::RandSelect(prob, 4);
	int failed_type = refine_failed_type[rst];
	
	if(failed_type == item::REFINE_SUCCESS)
		ess.refine_level ++;	
	else if(failed_type == item::REFINE_FAILED_LEVEL_0)
		return failed_type;
	else if(failed_type == item::REFINE_FAILED_LEVEL_1)
	{
		if(ess.refine_level == 0) return failed_type;
		ess.refine_level --;
	}
	else if(failed_type == item::REFINE_FAILED_LEVEL_2)
	{
		if(ess.refine_level == 0) return failed_type;
		ess.refine_level  = 0;
	}
	else
	{
		ASSERT(false);
		return failed_type;
	}
	
	OnRefreshRawData();
	return failed_type;
}

short elf_item::SetRefineLevel(short level)
{
	ASSERT(level >= 0 && level <= MAX_ELF_REFINE_LEVEL);
	ess.refine_level = level;
	OnRefreshRawData();
	return level;	
}


unsigned int elf_item::InsertExp(unsigned int exp, short exp_level, gactive_imp* imp, bool& is_levelup, bool ischeck)//exp_levelΪ�����裨100��������ȼ�
{
	short player_level = imp->GetHistoricalMaxLevel();
	if(exp <= 0 || player_level <= 0 || exp_level <= 0 || player_level < ess.level ||
		player_level == ess.level && (unsigned int)(player_template::GetLvlupExp(0, ess.level)*prop.exp_factor)<=ess.exp+1)
		return (unsigned int)-1;
	if(ischeck)
		return 0;

	ASSERT(CheckRawExp());
	unsigned int old_exp = exp;
	unsigned int levelupexp;
	is_levelup = false;
	while(exp > 0)
	{
		if((unsigned int)(player_template::GetLvlupExp(0, ess.level) * prop.exp_factor) <= ess.exp)
			levelupexp = 1;		//��ģ����exp_factor���޸�ʱ�����������辭����Ϊ1����֤�߼�������
		else
			levelupexp = (unsigned int)(player_template::GetLvlupExp(0, ess.level) * prop.exp_factor) - ess.exp;//player classû��	
		double obtain_factor = GetExpObtainFactor(exp_level, ess.level);
		if(obtain_factor < 0.1) obtain_factor = 0.1;	//ע����ʱ��ñ�������10%
		unsigned int can_obtain_exp = (unsigned int)(exp*obtain_factor+0.00001);	//�������㾫������
		if(can_obtain_exp >= levelupexp)
		{
			if(ess.level >= player_level) //С����ȼ��Դﵽ����ȼ� ����������
			{
				ess.exp += (levelupexp - 1);
				exp -= (unsigned int)(ceil((levelupexp - 1)/obtain_factor));
				if(exp > old_exp) exp = 0;//�����ֹexp����ʱԽ��
				break;
			}
			LevelUp(imp);//�����л�����ess.exp = 0;	
			exp -= (unsigned int)(ceil(levelupexp/obtain_factor));
			if(exp > old_exp) exp = 0;//�����ֹexp����ʱԽ��
			is_levelup = true;
		}
		else
		{
			ess.exp  += can_obtain_exp;
			exp -= (unsigned int)(ceil(can_obtain_exp/obtain_factor));
			if(exp > old_exp) exp = 0;//�����ֹexp����ʱԽ��
			break;
		}
	}

	if(old_exp > exp)
	{
		if(is_levelup)
			OnRefreshItem();
		else
		{
			UpdateRawExp();
			imp->_runner->query_elf_exp(ess.exp);
		}
	}
	return (old_exp - exp);
}

double elf_item::GetExpObtainFactor(short exp_level, short elf_level)
{
	int max_level = player_template::GetMaxLevel();
	if(exp_level < 1) exp_level = 1;
	if(elf_level < 1) elf_level = 1;
	if(exp_level > max_level) exp_level = max_level;
	if(elf_level > max_level) elf_level = max_level;
	double factor;
	if(exp_level <= elf_level)
		factor = 1;
	else
		factor = (double)elf_exp_loss_constant[elf_level]/(double)elf_exp_loss_constant[exp_level];
	
	ASSERT(factor <= 1 && factor > 0);
	return factor;
}

bool elf_item::LevelUp(gactive_imp* imp)
{
	int inc_attribute = 0, inc_genius = 0;
	inc_attribute ++;
	if((ess.level+1) % 10 == 0)
	{
		DATA_TYPE dt;
		struct GOBLIN_ESSENCE * elf = (struct GOBLIN_ESSENCE *)world_manager::GetDataMan().get_data_ptr(_tid, ID_SPACE_ESSENCE, dt);	
		if(elf == NULL || dt != DT_GOBLIN_ESSENCE)
		{
			ASSERT(false);
			return false;
		}
		float prob[10] = {0.0f};
		for(int i=0; i<10; i++)
			if(elf->rand_prop[i].rand_rate > 0)
				prob[i] = elf->rand_prop[i].rand_rate;			
		prob[9] = 1.0f;	//��֤�����RandSelect�������
		int rst = abase::RandSelect(prob, 10);
		if(elf->rand_prop[rst].rand_num > 0)
			inc_attribute += elf->rand_prop[rst].rand_num;
	}
	if((ess.level+1) <= 100)
	{
		if((ess.level+1) % 5 == 0)
			inc_genius ++;
	}
	else
		inc_genius ++;
	ess.exp = 0;
	ess.level ++;
	ess.total_attribute += inc_attribute;
	ess.total_genius += inc_genius;
	imp->_runner->elf_cmd_result(S2C::ELF_LEVELUP, _tid, ess.level, (inc_genius<<16)|inc_attribute);
	imp->_runner->elf_levelup();
	gplayer_imp * pImp = (gplayer_imp *)imp;
	int id1 = _tid | GetIdModify();
	pImp->CalcEquipmentInfo();
	pImp->_runner->equipment_info_changed(1ULL<<item::EQUIP_INDEX_ELF,0,&id1,sizeof(id1));//�˺���ʹ����CalcEquipmentInfo�Ľ��
	
	return true;	
}

void elf_item::UpdateElfSecureStatus()	//����ת��״̬->�ɽ���״̬���Զ�ת��
{
	if(ess.status_value > 0 && ess.status_value <= g_timer.get_systime())	//ת��״̬����
	{
		ess.status_value = -1;	//�ɽ���״̬
		prop.secure_status = STATUS_TRADABLE;
		OnRefreshRawData();
	}
}

bool elf_item::ChangeElfSecureStatus(int status, bool ischeck)//״̬ת��
{
	switch(prop.secure_status)
	{
	case STATUS_SECURE:
		if(status == STATUS_TRANSFORM)
		{
			if(ischeck)	return true;
			ess.status_value = g_timer.get_systime() + 7*24*3600;
			prop.secure_status = STATUS_TRANSFORM;
			OnRefreshRawData();
			return true;
		}
		break;
	case STATUS_TRANSFORM:
	case STATUS_TRADABLE:
		if(status == STATUS_SECURE)
		{
			if(ischeck)	return true;
			ess.status_value = 0;
			prop.secure_status = STATUS_SECURE;
			OnRefreshRawData();
			return true;
		}
		break;
	default:
		break;
	}
	return false;		
	
}

int elf_item::OnCharge(int element_level, size_t count,int & cur_time)
{
	//Ԫʯ�ȼ���Ӧ�Ĳ�������ֵ
	const int element_stamina[11] = {1, 40, 80, 160, 240, 320, 400, 480, 560, 640, 720};
	if(count <= 0 || element_level <= 0 || element_level >10)	return 0;
	
	int cur_stamina = ess.stamina + stamina_offset;
	if(cur_stamina >= MAX_STAMINA)	return 0;

	size_t offset = MAX_STAMINA - cur_stamina;
	size_t use_count  = offset / element_stamina[element_level];

	if(offset > use_count * element_stamina[element_level]) use_count ++;
	if(use_count > count) 
	{
		use_count = count;
		stamina_offset += count * element_stamina[element_level];
	}
	else
	{
		stamina_offset = MAX_STAMINA - ess.stamina;
	}
	cur_time = ess.stamina + stamina_offset;

	return (int)use_count;
}

bool elf_item::GetDecomposeElfExp(unsigned int & exp, int & exp_level)
{
	exp = 0;
	exp_level = ess.level;
	double obtain_factor;
	unsigned int levelexp;
	double _exp = 0.0;
	for(int i=1; i<ess.level; i++)
	{
		levelexp = (unsigned int)(player_template::GetLvlupExp(0, i) * prop.exp_factor);	//player classû��
		obtain_factor = GetExpObtainFactor(ess.level, i);
		_exp += (double)levelexp * ELF_DECOMPOSE_EXP_LOSS / obtain_factor;
		//__PRINTF("%d--%d--%f\n",i,levelexp,_exp);
	}
	_exp += (double)ess.exp * ELF_DECOMPOSE_EXP_LOSS;

	exp = (_exp>4.2e9 ? 4200000000u : (unsigned int)_exp);
	return true;
}

int elf_item::DestroyElfItem(int mask, int equip_type)//mask װ��λ��
{
	if(mask != 0x01 && mask != 0x02 && mask != 0x04 && mask != 0x08)
		return -1;
	if((prop.all_equip_mask & mask) == 0)//û�����װ��
		return -1;
	DATA_TYPE dt0;
	struct GOBLIN_EQUIP_ESSENCE * eq0;
	if(equip_type > 0)
	{
		eq0 = (struct GOBLIN_EQUIP_ESSENCE *)world_manager::GetDataMan().get_data_ptr(equip_type, ID_SPACE_ESSENCE, dt0);
		if(eq0 == NULL || dt0 != DT_GOBLIN_EQUIP_ESSENCE)
			return -1;
		if(mask != (1 << eq0->equip_type))
			return -1;
		if(ess.level < eq0->req_goblin_level)//�ȼ�����
			return -1;
	}
		
	abase::vector<unsigned int, abase::fast_alloc<> >::iterator it = equipvect.begin(), ite = equipvect.end();
	DATA_TYPE dt;
	struct GOBLIN_EQUIP_ESSENCE * eq;
	for( ; it != ite; it++)
	{
		eq = (struct GOBLIN_EQUIP_ESSENCE *)world_manager::GetDataMan().get_data_ptr(*it, ID_SPACE_ESSENCE, dt);
		if(eq == NULL || dt != DT_GOBLIN_EQUIP_ESSENCE)		//�������Ӧ�ó���
		{
			ASSERT(false && "С�������ϵ�װ����ģ�����Ҳ�����");
			return -1;
		}
		if(mask == (1 << eq->equip_type))
		{
			//��ֹ��װ����ļ���������С����ѧ�ļ�����
			//int new_skill_slot;
			//if(equip_type > 0)
			//	new_skill_slot = INITIAL_SKILL_SLOT+(prop.cur_energy - eq->energy + eq0->energy)/NEED_ENERGY_PER_SKILL_SLOT;
			//else
			//	new_skill_slot = INITIAL_SKILL_SLOT+(prop.cur_energy - eq->energy)/NEED_ENERGY_PER_SKILL_SLOT;
			//if(new_skill_slot > MAX_ELF_SKILL_CNT)
			//	new_skill_slot = MAX_ELF_SKILL_CNT;
			//if(skillvect.size() > (size_t)new_skill_slot)
			//	return -1;
			
			if(equip_type > 0)
				*it = equip_type;
			else
			{
				equipvect.erase(it);
				prop.all_equip_mask &= ~mask;
			}
			OnRefreshItem();
			return eq->id;	
		}
	}
	
	ASSERT(false && "�������Ҳ���Ҫɾ����װ����");
	return -1;
}

bool elf_item::EquipElfItem(unsigned int id, bool ischeck)
{
	DATA_TYPE dt;
	struct GOBLIN_EQUIP_ESSENCE * eq;
	eq = (struct GOBLIN_EQUIP_ESSENCE *)world_manager::GetDataMan().get_data_ptr(id, ID_SPACE_ESSENCE, dt);
	if(eq == NULL || dt != DT_GOBLIN_EQUIP_ESSENCE)
		return false;
	if(prop.all_equip_mask & (1 << eq->equip_type))//���λ����װ��
		return false;
	if(ess.level < eq->req_goblin_level)//�ȼ�����
		return false;
	if(ischeck)
		return true;

	prop.all_equip_mask |= (1 << eq->equip_type);
	equipvect.push_back(id);
	
	OnRefreshItem();
	return true;
}

//for debug only
void elf_item::dump_all()
{
	printf("----------------------------------------\n");	
	printf("ess:\n");
	printf("\texp=%u level=%d total_attr=%d str=%d agi=%d vit=%d eng=%d\n", ess.exp, ess.level, ess.total_attribute, ess.strength, ess.agility, ess.vitality, ess.energy);
	printf("\ttotal_genius=%d g0=%d g1=%d g2=%d g3=%d g4=%d refine_lvl=%d\n", ess.total_genius, ess.genius[0], ess.genius[1], ess.genius[2], ess.genius[3], ess.genius[4], ess.refine_level);
	printf("\tstamina=%d status_value=%d stamina_offset = %d\n", ess.stamina, ess.status_value, stamina_offset);
	printf("prop:\n");
	printf("\texp_factor=%f max_rand_prop=%d average_rand_prop=%d cur_strength=%d cur_agility=%d cur_vitality=%d cur_energy=%d\n\tcur_genius[0]=%d cur_genius[1]=%d cur_genius[2]=%d cur_genius[3]=%d cur_genius[4]=%d \n\tall_equip_mask=%d cur_skill_slot=%d secure_status=%d\n",
	prop.exp_factor ,prop.max_rand_prop, prop.average_rand_prop, prop.cur_strength ,prop.cur_agility ,prop.cur_vitality ,prop.cur_energy ,prop.cur_genius[0],prop.cur_genius[1],prop.cur_genius[2],prop.cur_genius[3],prop.cur_genius[4] ,prop.all_equip_mask ,prop.cur_skill_slot ,prop.secure_status);
	printf("equip:\n\t");
	for(size_t i=0; i<equipvect.size(); i++)
		printf("equip[%d]=%d ", i, equipvect[i]);
	printf("\n");
	printf("skill:\n\t");
	for(size_t i=0; i<skillvect.size(); i++)
		printf("skill[%d] id=%d level=%d ", i, skillvect[i].id, skillvect[i].level);
	
	printf("\n");
	printf("----------------------------------------\n");	
}

void elf_item::change_elf_property(int index, int value, gactive_imp* imp)
{
	switch(index)
	{
		case 1:
			if(value >= 0 && value <= int(ess.level/10)*100)
			{
				ess.total_attribute = ess.level-1+value;
				ess.strength = 0;
				ess.agility = 0;
				ess.vitality = 0;
				//int new_skill_slot = INITIAL_SKILL_SLOT + (prop.cur_energy-ess.energy)/NEED_ENERGY_PER_SKILL_SLOT;
				//if(new_skill_slot > MAX_ELF_SKILL_CNT)
				//	new_skill_slot = MAX_ELF_SKILL_CNT;
				//if(skillvect.size() > (size_t)new_skill_slot)
				//{
				//	SKILL_VECT::iterator it = skillvect.begin(), ite = skillvect.end();
				//	skillvect.erase(it+new_skill_slot, ite);	
				//}
				ess.energy = 0;
				OnRefreshItem();
				printf("�����debug���������С������������\n");
			}
			break;
		case 2:
			if(value >= 0 && value <= 999999)
			{
				ess.stamina = value;
				stamina_offset = 0;
				OnRefreshRawData();
			}
			break;
		case 3:
			if(prop.secure_status != STATUS_TRADABLE)
			{
				ess.status_value = g_timer.get_systime() + 3;
				prop.secure_status = STATUS_TRANSFORM;
				OnRefreshRawData();
				imp->UpdateMinElfStatusValue(ess.status_value);
			}				
			break;
		case 4:
			if(value > ess.level && value <= player_template::GetMaxLevel())
			{
				int n = value - ess.level;
				for(int i=0; i<n; i++)
					LevelUp(imp);
				OnRefreshItem();
			}
			break;
		default:
			break;
	}
}

void elf_item::dump_skill(char * buf, size_t buf_size)
{
	size_t skill_cnt = skillvect.size();
	if(buf_size < 15 + skill_cnt*15) return;
	sprintf(buf,"count:%d",skill_cnt);
	int len;
	for(size_t i=0; i<skill_cnt; i++)
	{
		len = strlen(buf);
		sprintf(buf+len,"(%d,%d)",skillvect[i].id,skillvect[i].level);
	}
}

