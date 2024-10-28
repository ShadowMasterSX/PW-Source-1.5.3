#include "../world.h"
#include "equip_item.h"
#include "item_addon.h"
#include "../clstab.h"
#include "../actobject.h"
#include "../item_list.h"
#include "../worldmanager.h"
#include "../playerteam.h"
#include "../player_imp.h"
#include "../task/taskman.h"

#include <glog.h>

DEFINE_SUBSTANCE_ABSTRACT(equip_item,item_body,CLS_ITEM_EQUIP)
DEFINE_SUBSTANCE_ABSTRACT(weapon_item,equip_item,CLS_ITEM_WEAPON)
DEFINE_SUBSTANCE(melee_weapon_item,weapon_item,CLS_ITEM_MELEE_WEAPON)
DEFINE_SUBSTANCE(range_weapon_item,weapon_item,CLS_ITEM_RANGE_WEAPON)
DEFINE_SUBSTANCE(armor_item,equip_item,CLS_ITEM_ARMOR)
DEFINE_SUBSTANCE(projectile_equip_item,equip_item,CLS_ITEM_PROJECTILE)
DEFINE_SUBSTANCE(decoration_equip_item,equip_item,CLS_ITEM_DECORATION)

equip_item::~equip_item()
{
}

int 
equip_item::GetIdModify()
{
	int mask = _modify_mask;
	mask <<= 16;
	return mask;
}

void 
equip_item::OnTakeOut(item::LOCATION l,size_t pos,size_t count, gactive_imp* obj)
{
	switch(l)
	{
		case item::BODY:
			Deactivate(l,pos,count,obj); 
			break;
		case item::INVENTORY:
		case item::TASK_INVENTORY:
		case item::BACKPACK:
		default:
			break;
	};
}

int
equip_item::OnUse(item::LOCATION l,gactive_imp * obj,size_t count)
{
	ASSERT(_use_addon.id != -1);
	if(_use_addon.id != -1)
	{
		addon_manager::Use(_use_addon,this,obj);
	}
	return 0;
}

bool 
equip_item::VerifyRequirement(item_list & list,gactive_imp* obj) 
{
	if(list.GetLocation() == item::BODY)
	{
		//装备物品增加声望限制
		int reputation_limit = world_manager::GetDataMan().get_item_reputation_limit(_tid);
		if(reputation_limit && ((gplayer_imp*)obj)->GetReputation() < reputation_limit) return false;

		return (obj->GetHistoricalMaxLevel() >= _base_limit.level && 
			((1 << (obj->GetObjectClass() & 0x0F)) & _base_limit.race) &&
			obj->_cur_prop.strength >= _base_limit.strength &&
			obj->_cur_prop.vitality >= _base_limit.vitality &&
			obj->_cur_prop.agility >= _base_limit.agility &&
			obj->_cur_prop.energy >= _base_limit.energy &&
			_base_limit.durability > 0);
	}
	else
	{
		return false;
	}
}

void 
equip_item::OnRefreshItem()
{
	//刷新一个装备，首先要更新它的数据	
	ClearData();
	_active_addon.clear();
	//Update
	UpdateEssence();
	UpdateAddOn();
	UpdateData();
	//考虑数据保存
	_raw_data.clear();
	raw_wrapper rw;
	Save(rw);
	rw.swap(_raw_data);
	CalcCRC();
}

bool
equip_item::Sign(unsigned short color, const char * signature, unsigned int signature_len)
{
	if (signature_len == 0)	//清空签名
	{
		if (_m_tag.tag_type == element_data::IMT_NULL || _m_tag.tag_size == 0)
			return false;
		_m_tag.tag_type = element_data::IMT_NULL;
		memset(_m_tag.tag_content, 0, MAX_USERNAME_LENGTH);
		_m_tag.tag_size = 0;
	}
	else	//修改签名
	{
		_m_tag.tag_type = element_data::IMT_SIGN;
		memset(_m_tag.tag_content, 0, MAX_USERNAME_LENGTH);
		memcpy(_m_tag.tag_content, (char *)&color, sizeof(unsigned short));
		memcpy(_m_tag.tag_content+sizeof(unsigned short), signature, signature_len);
		_m_tag.tag_size = sizeof(unsigned short)+signature_len;
	}
	OnRefreshItem();
	return true;
}

size_t 
equip_item::LoadMadeTag(archive & ar)
{
	unsigned short size;
	char tag_type;
	unsigned char tag_size;
	ar >> size >> tag_type >> tag_size;
	if(tag_size)
	{
		if(tag_size > MAX_USERNAME_LENGTH) throw 1;
		ar.pop_back(_m_tag.tag_content,tag_size);
		_m_tag.tag_type = tag_type;
		_m_tag.tag_size = tag_size;
	}
	else
	{
		_m_tag.tag_type = 0;
		_m_tag.tag_size = 0;
	}
	return size;
}

void 
equip_item::SaveMadeTag(archive & ar, size_t ess_size)
{
	unsigned short size = ess_size;
	ar << size << _m_tag.tag_type << _m_tag.tag_size;
	if(_m_tag.tag_size)
	{
		ar.push_back(_m_tag.tag_content,_m_tag.tag_size);
	}
}

bool 
equip_item::RegenAddon(int item_id,bool (*regen_addon)(int item_id, addon_data & ent))
{
	bool bRst = false;
	size_t count = _total_addon.size();
	for(size_t i = 0; i < count; i ++)
	{
		if(addon_manager::IsRandomAddon(_total_addon[i].id))
		{
			bRst = true;
			if(!regen_addon(item_id,_total_addon[i]))
			{
				_total_addon.erase(_total_addon.begin() + i);
				i --;
				count --;
			}
			else
			{
				addon_update_ess_data(_total_addon[i],GetEssence(),GetEssenceSize(),&_base_limit);
			}
		}
	}
	
	if(bRst)
	{
		OnRefreshItem();
	}
	return bRst;
}

struct refine_param_t
{
	int need_level;
	float prop[4];	//分别对应 成功 REFINE_FAILED_LEVEL_0 REFINE_FAILED_LEVEL_1 REFINE_FAILED_LEVEL_2
};

static refine_param_t refine_table[]=
{
	{0 ,{ 0.50, 0.7,  0 ,0 }},
	{1 ,{ 0.30, 0,    0 ,1 }},
	{2 ,{ 0.30, 0,    0 ,1 }},
	{3 ,{ 0.30, 0,    0 ,1 }},
	{4 ,{ 0.30, 0,    0 ,1 }},
	{5 ,{ 0.30, 0,    0 ,1 }},
	{6 ,{ 0.30, 0,    0 ,1 }},
	{7 ,{ 0.30, 0,    0 ,1 }},
	{8 ,{ 0.25, 0,    0 ,1 }},
	{9 ,{ 0.20, 0,    0 ,1 }},
	{10,{ 0.12, 0,    0 ,1 }},
	{11,{ 0.05, 0,    0 ,1 }},
};

static float refine_factor[] =
{
	0,	//not use
	1.0f,
	2.0f,
	3.05f,
	4.3f,
	5.75f,
	7.55f,
	9.95f,
	13.f,
	17.05f,
	22.3f,
	29.f,
	37.5f,
};

static int refine_failed_type[] = 
{
	item::REFINE_SUCCESS,
	item::REFINE_FAILED_LEVEL_0, 
	item::REFINE_FAILED_LEVEL_1,
	item::REFINE_FAILED_LEVEL_2,
};


static int randomNums4[] = 
{
    1,
    1,
    1,
    1,
	1,
	1,
	1,
	1,
	1,
    2,
	2,
	2,
	2,
	2,
    2,
    2,
	2,
	2,
    3,
	3,
    3,
    4,
};

static int randomNums3[] = 
{
    1,
    1,
    1,
    1,
	1,
	1,
	1,
	1,
	1,
    1,
	1,
	1,
	2,
	2,
    2,
    2,
	2,
	2,
    2,
	3,
    3,
    3,
};

static int randomNums2[] = 
{
    1,
    1,
    1,
    1,
	1,
	1,
	1,
	1,
	1,
    1,
	1,
	1,
	1,
	1,
    1,
    2,
	2,
	2,
    2,
	2,
    2,
    2,
};


int
equip_item::RefineAddon(int addon_id, int  & level_result, float adjust[4], float adjust2[12])
{
	//第一步寻找正确的addon内容
	size_t addon_level = 0;
	int addon_index = -1;
	size_t count = _total_addon.size();
	for(size_t i = 0; i < count; i ++)
	{
		addon_data & data = _total_addon[i];
		int id = addon_manager::GetAddonID(data.id);
		if(id == addon_id)
		{
			//得到已经升级后的数据
			addon_index = i;
			addon_level = data.arg[1];
			break;
		}
	}

	//超过或者达到最大的级别则不可升级
	if(addon_level >= sizeof(refine_table) / sizeof(refine_param_t)) return item::REFINE_CAN_NOT_REFINE;

	//保存原来的技能
	level_result = addon_level;
	
	//考虑概率
	float prop[4];
	memcpy(prop, refine_table[addon_level].prop,sizeof(prop));
	ASSERT(sizeof(prop) == sizeof(refine_table[addon_level].prop));
	//对prop进行修正  
	prop[0] += adjust[0]; prop[1] += adjust[1];
	prop[2] += adjust[2]; prop[3] += adjust[3];

	if(adjust[1] > 0)
	{
		//若特殊保留概率大于0 则使用adjust2里面带有的成功概率 并忽视原有的成功概率
		prop[0] = adjust2[addon_level];
	}
	int rst = abase::RandSelect(prop, 4);
	int failed_type = refine_failed_type[rst];

	if(failed_type != item::REFINE_SUCCESS)
	{
		//未成功，考虑如何进行处理
		switch(failed_type)
		{
			case item::REFINE_FAILED_LEVEL_0: //无变化
			return failed_type;

			case item::REFINE_FAILED_LEVEL_1: //装备降级 1级
			//第一次就失败，装备无变化
			if(addon_level == 0 || addon_index == -1) return item::REFINE_FAILED_LEVEL_0; 
			if(addon_level == 1)
			{
				//第二次失败，等同于归0
				_total_addon.erase(_total_addon.begin() + addon_index);
				OnRefreshItem();
				return failed_type;
			}

			//其他情况 回到后面进行降级处理
			break;

			case item::REFINE_FAILED_LEVEL_2: //装备归0
			if(addon_index != -1)
			{
				_total_addon.erase(_total_addon.begin() + addon_index);
				OnRefreshItem();
			}
			return failed_type;

			default:
			ASSERT(false);
			return failed_type;
		}
	}
	
	addon_data  newdata;
	if(!world_manager::GetDataMan().generate_addon(addon_id,newdata)) return item::REFINE_CAN_NOT_REFINE; 
	if(addon_index == -1)
	{
		ASSERT(failed_type == item::REFINE_SUCCESS);
		addon_level = 1;
		//新生成一个addon
		newdata.arg[0] = (int)(newdata.arg[0] * refine_factor[addon_level] + 0.1f);
		newdata.arg[1] = 1;	//当前级别为level1
		_total_addon.push_back(newdata);
	}
	else
	{
		if(failed_type == item::REFINE_FAILED_LEVEL_1)
			addon_level -= 1;
		else
			addon_level += 1;

		_total_addon[addon_index].arg[0] = (int)(newdata.arg[0] * refine_factor[addon_level] + 0.1f);
		_total_addon[addon_index].arg[1] = addon_level;

	/*	if(addon_manager::RefineAddonData(_total_addon[addon_index], newdata, failed_type == item::REFINE_FAILED_LEVEL_1) != 0)
		{
			return item::REFINE_CAN_NOT_REFINE;
		}
		*/
	}

	OnRefreshItem();
	return failed_type;
}


int
equip_item::RefineAddon(int addon_id, int  & level_result, float adjust[4], float adjust2[12], int material_id)
{
	//第一步寻找正确的addon内容
	size_t addon_level = 0;
	int addon_index = -1;
	size_t count = _total_addon.size();
	for(size_t i = 0; i < count; i ++)
	{
		addon_data & data = _total_addon[i];
		int id = addon_manager::GetAddonID(data.id);
		if(id == addon_id)
		{
			//得到已经升级后的数据
			addon_index = i;
			addon_level = data.arg[1];
			break;
		}
	}

	//超过或者达到最大的级别则不可升级
	if(addon_level >= sizeof(refine_table) / sizeof(refine_param_t)) return item::REFINE_CAN_NOT_REFINE;

	//保存原来的技能
	level_result = addon_level;
	
	//考虑概率
	float prop[4];
	memcpy(prop, refine_table[addon_level].prop,sizeof(prop));
	ASSERT(sizeof(prop) == sizeof(refine_table[addon_level].prop));
	//对prop进行修正  
	prop[0] += adjust[0]; prop[1] += adjust[1];
	prop[2] += adjust[2]; prop[3] += adjust[3];

	if(adjust[1] > 0)
	{
		//若特殊保留概率大于0 则使用adjust2里面带有的成功概率 并忽视原有的成功概率
		prop[0] = adjust2[addon_level];
	}
	int rst = abase::RandSelect(prop, 4);
	int failed_type = refine_failed_type[rst];

	if(failed_type != item::REFINE_SUCCESS)
	{
		//未成功，考虑如何进行处理
		switch(failed_type)
		{
			case item::REFINE_FAILED_LEVEL_0: //无变化
			return failed_type;

			case item::REFINE_FAILED_LEVEL_1: //装备降级 1级
			//第一次就失败，装备无变化
			if(addon_level == 0 || addon_index == -1) return item::REFINE_FAILED_LEVEL_0; 
			if(addon_level == 1)
			{
				//第二次失败，等同于归0
				_total_addon.erase(_total_addon.begin() + addon_index);
				OnRefreshItem();
				return failed_type;
			}

			//其他情况 回到后面进行降级处理
			break;

			case item::REFINE_FAILED_LEVEL_2: //装备归0
			if(addon_index != -1)
			{
				_total_addon.erase(_total_addon.begin() + addon_index);
				OnRefreshItem();
			}
			return failed_type;

			default:
			ASSERT(false);
			return failed_type;
		}
	}
	
	addon_data  newdata;
	int instantRefino = 1;	
	//DATA_TYPE dt2;
	int random4 = rand() %  sizeof(randomNums4) / sizeof(randomNums4[0]);
	int random3 = rand() %  sizeof(randomNums3) / sizeof(randomNums3[0]);
	int random2 = rand() %  sizeof(randomNums2) / sizeof(randomNums2[0]);
	/*const REFINE_TICKET_ESSENCE &ess= *(const REFINE_TICKET_ESSENCE*)world_manager::GetDataMan().get_data_ptr(material_id, ID_SPACE_ESSENCE,dt2);
	if(dt2 == DT_REFINE_TICKET_ESSENCE || &ess != NULL)
	{
		random4 = rand() %  sizeof(randomNums4) / sizeof(randomNums4[0]);
		random3 = rand() %  sizeof(randomNums3) / sizeof(randomNums3[0]);
		random2 = rand() %  sizeof(randomNums2) / sizeof(randomNums2[0]);
	}*/
	
	int newvalue = 1;
	if(!world_manager::GetDataMan().generate_addon(addon_id,newdata)) return item::REFINE_CAN_NOT_REFINE; 
	if (addon_level <= 8)
		newvalue = randomNums4[random4];
	else if (addon_level == 9)
		newvalue = randomNums3[random3];
	else if (addon_level == 10)
		newvalue = randomNums2[random2];
	else
		newvalue = 1;
	if(addon_index == -1)
	{
		ASSERT(failed_type == item::REFINE_SUCCESS);		
		addon_level = (material_id == 50001) ? newvalue : instantRefino;
		//新生成一个addon
		newdata.arg[0] = (int)(newdata.arg[0] * refine_factor[addon_level] + 0.1f);
		newdata.arg[1] = (material_id == 50001) ? newvalue : instantRefino;	//当前级别为level1
		_total_addon.push_back(newdata);
	}
	else
	{
		if(failed_type == item::REFINE_FAILED_LEVEL_1)
			addon_level -= 1;
		else
			addon_level += (material_id == 50001) ? newvalue : 1;	

		_total_addon[addon_index].arg[0] = (int)(newdata.arg[0] * refine_factor[addon_level] + 0.1f);
		_total_addon[addon_index].arg[1] = addon_level;

	/*	if(addon_manager::RefineAddonData(_total_addon[addon_index], newdata, failed_type == item::REFINE_FAILED_LEVEL_1) != 0)
		{
			return item::REFINE_CAN_NOT_REFINE;
		}
		*/
	}
	

	OnRefreshItem();
	return failed_type;
}

int 
equip_item::RemoveExpireAddon(int cur_t)
{
	bool changed = false;
	ADDON_LIST::iterator it = _total_addon.begin();
	for(; it != _total_addon.end(); )
	{
		int tmp = addon_manager::GetExpireDate(*it);
		if(tmp > 0 && cur_t >= tmp)
		{
			changed = true;
			it = _total_addon.erase(it);		
		}
		else
		{
			++it;
		}
	}
	if(changed)
	{
		_modify_mask &= ~0xFF00;
		OnRefreshItem();
	}
	return _addon_expire_date;
}

bool 
equip_item::Sharpen(addon_data * addon_list, size_t count, int sharpener_gfx)
{
	//清除原有的时效附加属性
	ADDON_LIST::iterator it = _total_addon.begin();
	for(; it != _total_addon.end(); )
	{
		int tmp = addon_manager::GetExpireDate(*it);
		if(tmp > 0)
			it = _total_addon.erase(it);		
		else
			++it;
	}
	//加入新的附加属性
	for(size_t i=0; i<count; i++)
	{
		_total_addon.push_back(addon_list[i]);	
	}
	_modify_mask &= ~0xFF00;
	_modify_mask |= ((sharpener_gfx & 0xFF)<<8);
	OnRefreshItem();
	return true;
}

bool 
equip_item::Engrave(addon_data * addon_list, size_t count)
{
	//清除原有的镌刻属性
	ADDON_LIST::iterator it = _total_addon.begin();
	for(; it != _total_addon.end(); )
	{
		if(addon_manager::IsAddOnEngraved(*it))
			it = _total_addon.erase(it);		
		else
			++it;
	}
	//加入新的附加属性
	for(size_t i=0; i<count; i++)
	{
		addon_manager::SetAddOnEngraved(addon_list[i]);
		_total_addon.push_back(addon_list[i]);	
	}
	OnRefreshItem();
	return true;
}

size_t 
equip_item::GetEngraveAddon(addon_data * addon_list, size_t max_count)
{
	size_t count = 0;
	for(ADDON_LIST::iterator it = _total_addon.begin(); it != _total_addon.end(); ++it)
	{
		if(addon_manager::IsAddOnEngraved(*it))
		{
			if(count >= max_count) break;
			addon_list[count] = *it;
			addon_manager::ClearAddOnEngraved(addon_list[count]);
			count ++;
		}	
	}
	return count;
}

bool 
equip_item::InheritAddon(addon_data * addon_list, size_t count)
{
	//加入新的附加属性
	for(size_t i = 0; i < count; i++)
	{
        _total_addon.push_back(addon_list[i]);	
		if(addon_manager::TestUpdate(addon_list[i]) == addon_manager::ADDON_MASK_ESSENCE)
		{
			addon_update_ess_data(addon_list[i],GetEssence(),GetEssenceSize(),&_base_limit);	
		}
	}
	OnRefreshItem();
	return true;
}

size_t 
equip_item::GetCanInheritAddon(addon_data * addon_list, size_t max_count, int ex_addon_id)
{
    abase::hash_map<int, int> inherit_addons;
    DATA_TYPE dt;
    const void* pess = world_manager::GetDataMan().get_data_ptr(_tid, ID_SPACE_ESSENCE, dt);
    ASSERT(pess);

    switch(dt)
    {
        case DT_WEAPON_ESSENCE:
        {
            const WEAPON_ESSENCE& ess = *(const WEAPON_ESSENCE*)pess;
            for(size_t i=0; i<sizeof(ess.rands)/sizeof(ess.rands[0]); i++)
                if(ess.rands[i].id_rand) inherit_addons[ess.rands[i].id_rand] = 1;
            for(size_t i=0; i<sizeof(ess.uniques)/sizeof(ess.uniques[0]); i++)
                if(ess.uniques[i].id_unique) inherit_addons[ess.uniques[i].id_unique] = 1;
        }
        break;
        case DT_ARMOR_ESSENCE:
        {
            const ARMOR_ESSENCE& ess = *(const ARMOR_ESSENCE*)pess;
            for(size_t i=0; i<sizeof(ess.rands)/sizeof(ess.rands[0]); i++)
                if(ess.rands[i].id_rand) inherit_addons[ess.rands[i].id_rand] = 1;
        }
        break;
        case DT_DECORATION_ESSENCE:
        {
            const DECORATION_ESSENCE& ess = *(const DECORATION_ESSENCE*)pess;
            for(size_t i=0; i<sizeof(ess.rands)/sizeof(ess.rands[0]); i++)
                if(ess.rands[i].id_rand) inherit_addons[ess.rands[i].id_rand] = 1;
        }
        break;
        default:
        break;
    }
    
	if(inherit_addons.size() == 0) return 0;
    
    size_t count = 0;
    for(size_t i = 0; i < _total_addon.size(); ++i)
    {
        addon_data& data = _total_addon[i];
        
        bool need_check_template = false;
        if (!addon_manager::IsAddOnInherited(data)) need_check_template = true;
        
        int addon_id = addon_manager::GetAddonID(data.id);
        if(need_check_template && inherit_addons.find(addon_id) == inherit_addons.end()) continue;
        if(addon_id == ex_addon_id) continue;
      //  if(addon_manager::TestUpdate(data) == addon_manager::ADDON_MASK_ESSENCE) continue;
        if(addon_manager::TestUpdate(data) == addon_manager::ADDON_MASK_USE) continue;
        if(addon_manager::IsAddOnEngraved(data)) continue;
        if(addon_manager::IsAddOnEmbedded(data)) continue;

        addon_list[count] = data;
        ++count;
        if(count >= max_count) break;
    }

    return count;
}

int 
equip_item::RegenInherentAddon()
{
	//获取所有可能的天生的属性集合
	abase::hash_map<int,int> inherent_addons;
	DATA_TYPE dt;
	const void * pess = world_manager::GetDataMan().get_data_ptr(_tid, ID_SPACE_ESSENCE,dt);
	ASSERT(pess);
	switch(dt)
	{
		case DT_WEAPON_ESSENCE:
		{
			const WEAPON_ESSENCE & ess = *(const WEAPON_ESSENCE *)pess;
			for(size_t i=0; i<sizeof(ess.addons)/sizeof(ess.addons[0]); i++)
				if(ess.addons[i].id_addon) inherent_addons[ess.addons[i].id_addon] = 1;
			for(size_t i=0; i<sizeof(ess.rands)/sizeof(ess.rands[0]); i++)
				if(ess.rands[i].id_rand) inherent_addons[ess.rands[i].id_rand] = 1;
			for(size_t i=0; i<sizeof(ess.uniques)/sizeof(ess.uniques[0]); i++)
				if(ess.uniques[i].id_unique) inherent_addons[ess.uniques[i].id_unique] = 1;
		}
		break;
		case DT_ARMOR_ESSENCE:
		{
			const ARMOR_ESSENCE & ess = *(const ARMOR_ESSENCE *)pess;
			for(size_t i=0; i<sizeof(ess.addons)/sizeof(ess.addons[0]); i++)
				if(ess.addons[i].id_addon) inherent_addons[ess.addons[i].id_addon] = 1;
			for(size_t i=0; i<sizeof(ess.rands)/sizeof(ess.rands[0]); i++)
				if(ess.rands[i].id_rand) inherent_addons[ess.rands[i].id_rand] = 1;
		}
		break;
		case DT_DECORATION_ESSENCE:
		{
			const DECORATION_ESSENCE & ess = *(const DECORATION_ESSENCE *)pess;
			for(size_t i=0; i<sizeof(ess.addons)/sizeof(ess.addons[0]); i++)
				if(ess.addons[i].id_addon) inherent_addons[ess.addons[i].id_addon] = 1;
			for(size_t i=0; i<sizeof(ess.rands)/sizeof(ess.rands[0]); i++)
				if(ess.rands[i].id_rand) inherent_addons[ess.rands[i].id_rand] = 1;
		}
		break;
		default:
		break;
	}
	if(inherent_addons.size() == 0) return 0;
	//遍历所有附加属性，如果是天生的则重新生成
	int regen_cnt = 0;	
	for(size_t i=0; i<_total_addon.size(); ++i)
	{
		addon_data & data = _total_addon[i];
		int addon_id = addon_manager::GetAddonID(data.id);
		if(inherent_addons.find(addon_id) == inherent_addons.end()) continue;
		if(addon_manager::TestUpdate(data) == addon_manager::ADDON_MASK_ESSENCE) continue;
		addon_data  newdata;
		if(!world_manager::GetDataMan().generate_addon(addon_id,newdata)) continue;
		_total_addon[i] = newdata;	
		++ regen_cnt;
	}
	if(regen_cnt) OnRefreshItem();
	return regen_cnt;
}

int
equip_item::GetRefineLevel(int addon_id)
{
	//第一步寻找正确的addon内容
	int addon_level = 0;
	size_t count = _total_addon.size();
	for(size_t i = 0; i < count; i ++)
	{
		addon_data & data = _total_addon[i];
		int id = addon_manager::GetAddonID(data.id);
		if(id == addon_id)
		{
			//得到已经升级后的数据
			addon_level = data.arg[1];
			break;
		}
	}
	return addon_level;
}

int 
equip_item::SetRefineLevel(int addon_id , int level)
{
	ASSERT(level >= 0 && level <=  (int)(sizeof(refine_table) / sizeof(refine_param_t)));
	//第一步寻找正确的addon内容
	size_t addon_level = 0;
	int addon_index = -1;
	size_t count = _total_addon.size();
	for(size_t i = 0; i < count; i ++)
	{
		addon_data & data = _total_addon[i];
		int id = addon_manager::GetAddonID(data.id);
		if(id == addon_id)
		{
			//得到已经升级后的数据
			addon_index = i;
			addon_level = data.arg[1];
			break;
		}
	}

	//保存原来的技能
	if(level == 0)
	{
		//这是清除操作 进行一次归 0
		if(addon_level > 0)
		{
			_total_addon.erase(_total_addon.begin() + addon_index);
			OnRefreshItem();
		}
		return 0;
	}

	addon_data  newdata;
	if(!world_manager::GetDataMan().generate_addon(addon_id,newdata)) return -1; 
	if(addon_level > 0)
	{
		//原来就有addon，这是一次修改
		_total_addon[addon_index].arg[0] = (int)(newdata.arg[0] * refine_factor[level] + 0.1f);
		_total_addon[addon_index].arg[1] = level;
	}
	else
	{
		//这是新加入的精炼
		newdata.arg[0] = (int)(newdata.arg[0] * refine_factor[level] + 0.1f);
		newdata.arg[1] = level;
		_total_addon.push_back(newdata);
	}

	OnRefreshItem();
	return level;
}
	

bool 
equip_item::Save(archive & ar)
{
	try
	{
		SaveLimit(ar);
		if(!SaveEssence(ar)) return false;
		SaveSocketData(ar);
		SaveAddOn(ar);
	}
	catch(...)
	{
		return false;
	}
	return true;
}

bool 
equip_item::Load(archive & ar)
{	
	ASSERT(_tid > 0);
	//保存数据至raw_data中
	//要求ar必定是头部
	ASSERT(ar.offset() == 0);
	_raw_data.clear();
	_raw_data.push_back(ar.data(),ar.size());

	try
	{
		//装载限定数据
		LoadLimit(ar);
		//装载本体数据
		if(!LoadEssence(ar)) return false;
		//装载孔洞数据
		LoadSocketData(ar);
		//装载附加属性
		LoadAddOn(ar);
	}
	catch(...)
	{
		return false;
	}
	ASSERT(ar.is_eof());		//由于保存数据到eof中，所以这里必须是全部数据

	try
	{
		UpdateEssence();
		UpdateAddOn();
		UpdateData();
	}
	catch(...)
	{
		return false;
	}
	CalcCRC();
	return true;
}


void 
equip_item::OnActivate(item::LOCATION l,size_t pos,size_t count, gactive_imp* obj)
{
	EssenceActivate(l,obj);
	for(size_t i = 0;i < _active_addon.size(); i ++)
	{
		addon_manager::Activate(_active_addon[i],this,obj);
	}
	if(_extra_addon)
	{
		for(size_t i = 0;i < _extra_addon->size(); i ++)
		{
			addon_manager::Activate((*_extra_addon)[i],this,obj);
		}
	}
}

void 
equip_item::OnDeactivate(item::LOCATION l,size_t pos,size_t count,gactive_imp* obj)
{
	EssenceDeactivate(l,obj);
	for(size_t i = 0;i < _active_addon.size(); i ++)
	{
		addon_manager::Deactivate(_active_addon[i],this,obj);
	}

	if(_extra_addon)
	{
		for(size_t i = 0;i < _extra_addon->size(); i ++)
		{
			addon_manager::Deactivate((*_extra_addon)[i],this,obj);
		}
	}
}

void 
equip_item::LoadLimit(archive &ar)
{
	_base_limit << ar;
	//装备的职业限制从模版中获取
	int class_limit	= world_manager::GetDataMan().get_item_class_limit(_tid);
	if(class_limit && class_limit != _base_limit.race)
	{
		//模版中的职业限制发生了改变，要更新装备原始数据
		ASSERT(CheckRawRace());
		_base_limit.race = class_limit;
		UpdateRawRace();
	}
	
	if(_base_limit.level < 0 || _base_limit.strength < 0 
		|| _base_limit.agility < 0 
		|| _base_limit.max_durability < 0 
		|| _base_limit.durability > _base_limit.max_durability) throw 104;
}

/* --------------------  可以有孔的物品 -------------------- */
bool 
socket_item::OnInsertChip(int chip_type,addon_data * data, size_t count)
{
	if(!count) return false;
	size_t i;
	for(i = 0; i < _socket_list.size(); i ++)
	{
		if(_socket_list[i]) continue;
		//找到了
		_socket_list[i] = chip_type;
		for(size_t j = 0; j < count; j ++)
		{
			addon_data adata = data[j];
			addon_manager::SetAddOnEmbedded(adata);
			_total_addon.push_back(adata);
		}
		AfterChipChanged();
		OnRefreshItem();
		return true;
	}
	return false;	//没有找到空闲的槽位
}

bool 
socket_item::OnClearChips()
{
	//清除嵌入物的标记
	size_t i;
	for(i = 0; i < _socket_list.size(); i ++)
	{
		if(_socket_list[i]) _socket_list[i] = 0;
	}

	//清除嵌入物的addon
	ADDON_LIST::iterator it = _total_addon.begin();
	for(; it != _total_addon.end(); )
	{
		if(addon_manager::IsAddOnEmbedded(*it))
		{
			_total_addon.erase(it);
		}
		else
		{
			++it;
		}
	}
	AfterChipChanged();
	OnRefreshItem();
	return true;
}

void 
socket_item::SetSocketCount(size_t count) 
{
	ASSERT(_socket_list.empty());
	_socket_list.insert(_socket_list.begin(),count,0);
}

void 
socket_item::SetSocketType(size_t index, int type)
{
	_socket_list[index] = type;
}

size_t 
socket_item::GetSocketCount()
{
	return _socket_list.size();
}

int 
socket_item::GetSocketType(size_t index)
{
	return _socket_list[index];
}

bool 
socket_item::RemoveAddon(unsigned char s_idx)
{
	if(_socket_list.size() <= s_idx || 0 == _socket_list[s_idx])
		return false;
		
	DATA_TYPE dt;
	STONE_ESSENCE * ess = (STONE_ESSENCE*) world_manager::GetDataMan().get_data_ptr(_socket_list[s_idx], ID_SPACE_ESSENCE, dt);
		
	if(!ess || dt != DT_STONE_ESSENCE) 
		return false;
		
    int addon_id = 0;
    switch (GetItemType())
    {
        case ITEM_TYPE_WEAPON: addon_id = ess->id_addon_damage; break;
        case ITEM_TYPE_ARMOR: addon_id = ess->id_addon_defence; break;
        case ITEM_TYPE_DECORATION: addon_id = ess->id_addon_decoration; break;
        default: break;
    }

	ADDON_LIST::iterator it = _total_addon.begin();
	for(; it != _total_addon.end(); ++it)
	{
		int old_id = addon_manager::GetAddonID((*it).id);
		if(old_id == addon_id && addon_manager::IsAddOnEmbedded(*it))
		{
			_total_addon.erase(it);
			_socket_list[s_idx] = 0;
			break;
		}
	}

	if(_socket_list[s_idx])
		return false;

	return true;
}

bool socket_item::ModifyAddonAtSocket(unsigned char s_idx,int stone_id)
{
	DATA_TYPE dt;
	STONE_ESSENCE * ess = (STONE_ESSENCE*) world_manager::GetDataMan().get_data_ptr(stone_id, ID_SPACE_ESSENCE, dt);
		
	if(!ess || dt != DT_STONE_ESSENCE) 
		return false;

    int addon_id = 0;
	switch (GetItemType())
    {
        case ITEM_TYPE_WEAPON: addon_id = ess->id_addon_damage; break;
        case ITEM_TYPE_ARMOR: addon_id = ess->id_addon_defence; break;
        case ITEM_TYPE_DECORATION: addon_id = ess->id_addon_decoration; break;
        default: break;
    }
	
	addon_data  newdata;		
	if(!world_manager::GetDataMan().generate_addon(addon_id,newdata)) 
		return false;

	if(!RemoveAddon(s_idx)) 
		return false;

	_socket_list[s_idx] = stone_id;
	addon_manager::SetAddOnEmbedded(newdata);
	_total_addon.push_back(newdata);

	AfterChipChanged();
	OnRefreshItem();
	// 由外部通知客户端装备数据改变

	return true;
}

void 
weapon_item::EssenceActivate(item::LOCATION l,gactive_imp* obj)
{
	obj->WeaponItemEnhance(_ess.weapon_type,_ess.weapon_delay,_ess.weapon_class,_ess.weapon_level,
			_ess.attack_speed, _ess.attack_range,_ess.attack_short_range,
			_base_param,_base_param_percent);
}

void 
weapon_item::EssenceDeactivate(item::LOCATION l,gactive_imp* obj)
{
	obj->WeaponItemImpair(_base_param,_base_param_percent);
}

bool 
weapon_item::LoadEssence(archive & ar)
{
	size_t count = LoadMadeTag(ar);
	if(count != sizeof(_ess))
	{
		ASSERT(false && "essence size not match");
		return false;
	}
	ar >> _ess;
	return true;
}

bool 
weapon_item::SaveEssence(archive & ar)
{
	SaveMadeTag(ar,sizeof(_ess));
	ar << _ess;
	return true;
}

void 
weapon_item::UpdateEssence()
{
	_base_param.damage_low 		= _ess.damage_low;
	_base_param.damage_high 	= _ess.damage_high;
	_base_param.magic_damage_low 	= _ess.magic_damage_low;
	_base_param.magic_damage_high 	= _ess.magic_damage_high;
}

void 
weapon_item::UpdateData()
{
	float scale = (100 + _base_param_percent.damage) * 0.01f;

	_base_param.damage_low 		= (int)(_base_param.damage_low * scale + 0.5f);
	_base_param.damage_high 	= (int)(_base_param.damage_high * scale + 0.5f);

	scale = (100 + _base_param_percent.magic_damage) * 0.01f;
	_base_param.magic_damage_low 	= (int)(_base_param.magic_damage_low * scale + 0.5f);
	_base_param.magic_damage_high 	= (int)(_base_param.magic_damage_high * scale + 0.5f);

	_base_param_percent.damage = 0;
	_base_param_percent.magic_damage = 0;
}

int 
weapon_item::OnGetProjectileReqType()  const
{
	return  _ess.require_projectile;
}

void weapon_item::OnAfterAttack(item_list & list, bool * pUpdate)
{
	ASSERT(CheckRawDurability());
	if((_base_limit.durability -= DURABILITY_DEC_PER_ATTACK) <= 0)
	{
		_base_limit.durability = 0;
		*pUpdate = true;
	}
	else
		*pUpdate = false;
	UpdateRawDurability();
}

void
weapon_item::AfterChipChanged()
{
	//首先清空宝石标志 如果减少了宝石这里就会有用
	_modify_mask &= ~0xFF;

	if(_socket_list.size() != 2 ) return;
	if(!_socket_list[0] || !_socket_list[1]) return ;
	int color1,color2;
	int level1,level2;
	level1 = GetStoneColorLevel(_socket_list[0],color1);
	level2 = GetStoneColorLevel(_socket_list[1],color2);
	if(level1 < 7 || level2 < 7) return ;
	
	//设置宝石标志
	_modify_mask |= ((color2 & 0x07) << 3) | (color1 & 0x07);
}

void
armor_item::AfterChipChanged()
{
	int level_5_counter = 0;
	int level_8_counter = 0;
	int color_mask = 0;

	for(size_t i = 0; i < _socket_list.size(); i ++)
	{
		if(!_socket_list[i]) continue;
		int color;
		int level = GetStoneColorLevel(_socket_list[i],color);
		if(color == 0) continue;
		if(level <5) continue;
		level_5_counter ++;
		if(level >= 8) level_8_counter ++;
		if(color_mask)
		{
			if(color_mask != color) color_mask= 7;
		}
		else
		{
			color_mask = color & 0x07;
		}
	}
//第一判断是否满足10级要求
	if(level_8_counter >= 4)
	{
		color_mask += 8;
	}
	else if(level_5_counter < 2)
	{
		color_mask = 0;
	}

	_modify_mask &= ~0xFF;
	_modify_mask |= (color_mask & 0xFF);
}

void 
weapon_item::SetSocketAndStone(int count, int * stone_type)
{
	//首先清除宝石
	OnClearChips();	
	//然后改变孔数
	int cur_count = _socket_list.size();
	if(count > cur_count)
	{
		_socket_list.insert(_socket_list.end(),count-cur_count,0);	
	}
	else if(count < cur_count)
	{
		_socket_list.erase(_socket_list.end()+count-cur_count,_socket_list.end());
	}
	//最后镶嵌宝石
	for(int i=0; i<count; i++)
	{
		if(stone_type[i] <= 0) continue;
		DATA_TYPE dt;
		STONE_ESSENCE * ess = (STONE_ESSENCE*) world_manager::GetDataMan().get_data_ptr(stone_type[i], ID_SPACE_ESSENCE, dt);
		if(!ess || dt != DT_STONE_ESSENCE) continue;
        if (!IsStoneFit(DT_WEAPON_ESSENCE, ess->combined_switch)) continue;

		addon_data  data;
		if(!world_manager::GetDataMan().generate_addon(ess->id_addon_damage,data)) continue; 
		_socket_list[i] = stone_type[i];
		addon_manager::SetAddOnEmbedded(data);
		_total_addon.push_back(data);
	}
	AfterChipChanged();
	OnRefreshItem();
}

int weapon_item::Is16Por9JWeapon()
{
	if(_ess.weapon_level >= 16)
	{
		if(world_manager::GetDataMan().get_item_reputation_limit(_tid) >= 300000) return 2; //9军
		return 1;//16P
	}
	return 0;//普通
}

void 
armor_item::SetSocketAndStone(int count, int * stone_type)
{
	//首先清除宝石
	OnClearChips();	
	//然后改变孔数
	int cur_count = _socket_list.size();
	if(count > cur_count)
	{
		_socket_list.insert(_socket_list.end(),count-cur_count,0);	
	}
	else if(count < cur_count)
	{
		_socket_list.erase(_socket_list.end()+count-cur_count,_socket_list.end());
	}
	//最后镶嵌宝石
	for(int i=0; i<count; i++)
	{
		if(stone_type[i] <= 0) continue;
		DATA_TYPE dt;
		STONE_ESSENCE * ess = (STONE_ESSENCE*) world_manager::GetDataMan().get_data_ptr(stone_type[i], ID_SPACE_ESSENCE, dt);
		if(!ess || dt != DT_STONE_ESSENCE) continue;
        if (!IsStoneFit(DT_ARMOR_ESSENCE, ess->combined_switch)) continue;

		addon_data  data;
		if(!world_manager::GetDataMan().generate_addon(ess->id_addon_defence,data)) continue; 
		_socket_list[i] = stone_type[i];
		addon_manager::SetAddOnEmbedded(data);
		_total_addon.push_back(data);
	}
	AfterChipChanged();
	OnRefreshItem();
}


void decoration_equip_item::SetSocketAndStone(int count, int* stone_type)
{
    // 首先清除宝石
    OnClearChips();

    // 然后改变孔数
    int cur_count = GetSocketCount();
    if (count > cur_count)
    {
        _socket_list.insert(_socket_list.end(), count - cur_count, 0);
    }
    else if (count < cur_count)
    {
        _socket_list.erase(_socket_list.end() + count - cur_count, _socket_list.end());
    }

    // 最后镶嵌宝石
    for (int i = 0; i < count; ++i)
    {
        if (stone_type[i] <= 0) continue;

        DATA_TYPE dt;
        STONE_ESSENCE* ess = (STONE_ESSENCE*)world_manager::GetDataMan().get_data_ptr(stone_type[i], ID_SPACE_ESSENCE, dt);
        if ((ess == NULL) || (dt != DT_STONE_ESSENCE)) continue;
        if (!IsStoneFit(DT_DECORATION_ESSENCE, ess->combined_switch)) continue;

        addon_data data;
        if (!world_manager::GetDataMan().generate_addon(ess->id_addon_decoration, data)) continue;
        _socket_list[i] = stone_type[i];
        addon_manager::SetAddOnEmbedded(data);
        _total_addon.push_back(data);
    }

    AfterChipChanged();
    OnRefreshItem();
}


bool melee_weapon_item::OnCheckAttack(item_list & list)
{
	ASSERT(_ess.weapon_type == weapon_essence::WEAPON_TYPE_MELEE || _ess.weapon_type == weapon_essence::WEAPON_TYPE_MELEE_ASN);
	return true;
}


bool 
range_weapon_item::VerifyRequirement(item_list & list,gactive_imp* obj)
{
	if(!equip_item::VerifyRequirement(list,obj))
	{
		return false;
	}
	if(obj->GetForm() != 0)
	{
		return false;
	}
	return true;
}

bool 
range_weapon_item::OnCheckAttack(item_list & list)
{
	ASSERT(_ess.weapon_type == weapon_essence::WEAPON_TYPE_RANGE);
	if(!list.IsItemActive(item::EQUIP_INDEX_PROJECTILE))
	{
		return false;
	}
	return list[item::EQUIP_INDEX_PROJECTILE].GetProjectileType() == _ess.require_projectile;
}

/*		发射物的处理		*/
bool 
projectile_equip_item::LoadEssence(archive & ar)
{
	size_t count = LoadMadeTag(ar);
	if(count != sizeof(_ess))
	{
		//ASSERT(false && "essence size not match");
		return false;
	}
	try{
	ar >> _ess;
	}catch(...)
	{
//	ASSERT(false);
throw;
	}
	return true;
}

bool 
projectile_equip_item::SaveEssence(archive & ar)
{
	SaveMadeTag(ar,sizeof(_ess));
	ar << _ess;
	return true;
}

void 
projectile_equip_item::EssenceActivate(item::LOCATION l,gactive_imp* obj)
{
	obj->NormalEnhance(_base_param,_base_param_percent);
	return ;
}

void 
projectile_equip_item::EssenceDeactivate(item::LOCATION l,gactive_imp* obj) 
{ 
	obj->NormalImpair(_base_param,_base_param_percent);
	return; 
}

void 
projectile_equip_item::UpdateEssence() 
{ 
	_base_param.damage_low 		= _ess.enhance_damage;
	_base_param.damage_high 	= _ess.enhance_damage;
	_base_param_percent.damage 	= _ess.scale_enhance_damage;
	return; 
}

void 
projectile_equip_item::UpdateData() { return; }

bool 
projectile_equip_item::VerifyRequirement(item_list & list,gactive_imp* obj)
{
	if(!equip_item::VerifyRequirement(list,obj))
	{
		return false;
	}

	if(!list.IsItemActive(item::EQUIP_INDEX_WEAPON))
	{
		return false;
	}
	if(list[item::EQUIP_INDEX_WEAPON].GetProjectileReqType() != _ess.projectile_type) 
	{
		return false;
	}
	if(obj->_cur_item.weapon_level < _ess.weapon_level_low || obj->_cur_item.weapon_level > _ess.weapon_level_high) 
	{
		return false;
	}
	return true;
}

int 
projectile_equip_item::OnGetProjectileType() const
{
	return  _ess.projectile_type;
}


void 
armor_item::EssenceActivate(item::LOCATION l,gactive_imp* obj)
{
	obj->ArmorEnhance(_base_param,_base_param_percent,_ess.hp_enhance,_ess.mp_enhance);
	obj->EnhanceResistance(_ess.resistance);
}

void 
armor_item::EssenceDeactivate(item::LOCATION l,gactive_imp* obj)
{
	obj->ArmorImpair(_base_param,_base_param_percent,_ess.hp_enhance,_ess.mp_enhance);
	obj->ImpairResistance(_ess.resistance);
}

bool 
armor_item::LoadEssence(archive & ar)
{
	size_t count = LoadMadeTag(ar);
	if(count != sizeof(_ess))
	{
		ASSERT(false && "essence size not match");
		return false;
	}
	ar >> _ess;
	return true;
}

bool 
armor_item::SaveEssence(archive & ar)
{
	SaveMadeTag(ar,sizeof(_ess));
	ar << _ess;
	return true;
}

void 
armor_item::UpdateEssence()
{
	_base_param.defense = _ess.defense;
	_base_param.armor = _ess.armor;
}

void 
armor_item::UpdateData()
{
	float scale = (100 + _base_param_percent.defense) * 0.01f;
	_base_param.defense = (int)(_base_param.defense * scale + 0.5f);

	scale = (100 + _base_param_percent.armor) * 0.01f;
	_base_param.armor = (int)(_base_param.armor * scale + 0.5f);

	_base_param_percent.armor = 0;
	_base_param_percent.defense = 0;
}

bool 
decoration_equip_item::LoadEssence(archive & ar)
{
	size_t count = LoadMadeTag(ar);
	if(count != sizeof(_ess))
	{
		ASSERT(false && "essence size not match");
		return false;
	}
	ar >> _ess;
	return true;
}

bool 
decoration_equip_item::SaveEssence(archive & ar)
{
	SaveMadeTag(ar,sizeof(_ess));
	ar << _ess;
	return true;
}

void 
decoration_equip_item::EssenceActivate(item::LOCATION l,gactive_imp* obj)
{
	obj->NormalEnhance(_base_param,_base_param_percent);
	obj->EnhanceResistance(_ess.resistance);
}

void 
decoration_equip_item::EssenceDeactivate(item::LOCATION l,gactive_imp* obj)
{
	obj->NormalImpair(_base_param,_base_param_percent);
	obj->ImpairResistance(_ess.resistance);
}

void 
decoration_equip_item::UpdateEssence()
{
	_base_param.defense = _ess.defense;
	_base_param.armor = _ess.armor;
	_base_param.damage_low = _base_param.damage_high = _ess.damage;
	_base_param.magic_damage_low = _base_param.magic_damage_high = _ess.magic_damage;
}

void 
decoration_equip_item::UpdateData()
{
	//nothing
}

const static int weapon_slot_material_count[2][20] = {	{5, 10,15,20,25,30,35,40,45,50, 500, 1000,4000,6000, 8000, 10000,12000,14000,16000,18000},
							{10,20,30,40,50,60,70,80,90,100,1000,2000,8000,12000,16000,20000,24000,28000,32000,36000}};
int weapon_item::MakeSlot(gactive_imp* obj, int& count, unsigned int material_id, int material_count)
{
	gplayer_imp * pImp = (gplayer_imp *) obj;
	
	//取出物品模板
	DATA_TYPE dt;
	WEAPON_ESSENCE * ess = (WEAPON_ESSENCE*)world_manager::GetDataMan().get_data_ptr(_tid, ID_SPACE_ESSENCE,dt);
	if( !ess || dt != DT_WEAPON_ESSENCE) return S2C::ERR_MAKE_SLOT_FAILURE;
	
	//得到品级
	int level = ess->level;
	if(level <= 0 || level> 20) return S2C::ERR_MAKE_SLOT_FAILURE;
	level --;
	//得到当前孔数
	int slot_num = GetSocketCount();
	if(slot_num >=2) return S2C::ERR_MAKE_SLOT_FAILURE;
	//计算原料消耗
	int need_count = weapon_slot_material_count[slot_num][level];
	int material1_count = pImp->GetItemCount(MAKE_SLOT_ITEM_ID);
	int material2_count = pImp->GetItemCount(MAKE_SLOT_ITEM_ID2);
	if(need_count > material1_count + material2_count) return S2C::ERR_NOT_ENOUGH_MATERIAL;

	//增加孔
	_socket_list.push_back(0);
    count = GetSocketCount();
	//重新生成装备数据
	OnRefreshItem();
	//删除所需材料
	PlayerTaskInterface TaskIf(pImp);
	if(material1_count >= need_count)
		TaskIf.TakeAwayCommonItem(MAKE_SLOT_ITEM_ID, need_count);
	else
	{
		TaskIf.TakeAwayCommonItem(MAKE_SLOT_ITEM_ID, material1_count);
		TaskIf.TakeAwayCommonItem(MAKE_SLOT_ITEM_ID2, need_count - material1_count);
	}
	//返回成功
	return 0;
}


static const int  armor_slot_material_count[4][20] ={	{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 100, 200, 800, 1200, 1600, 2000, 2400, 2800, 3200, 3600},
							{2, 4, 6, 8, 10,12,14,16,18,20, 200, 400, 1600,2400, 3200, 4000, 4800, 5600, 6400, 7200},
							{3, 6, 9, 12,15,18,21,24,27,30, 300, 600, 2400,3600, 4800, 6000, 7200, 8400, 9600, 10800},
							{10,20,30,40,50,60,70,80,90,100,1000,2000,8000,12000,16000,20000,24000,28000,32000,36000}
						    };
int armor_item::MakeSlot(gactive_imp* obj, int& count, unsigned int material_id, int material_count)
{
	gplayer_imp * pImp = (gplayer_imp *) obj;
	
	//取出物品模板
	DATA_TYPE dt;
	ARMOR_ESSENCE  * ess = (ARMOR_ESSENCE *)world_manager::GetDataMan().get_data_ptr(_tid, ID_SPACE_ESSENCE,dt);
	if( !ess || dt != DT_ARMOR_ESSENCE) return S2C::ERR_MAKE_SLOT_FAILURE;
	
	//得到品级
	int level = ess->level;
	if(level <= 0 || level> 20) return S2C::ERR_MAKE_SLOT_FAILURE;
	level --;
	//得到当前孔数
	int slot_num = GetSocketCount();
	if(slot_num >=4) return S2C::ERR_MAKE_SLOT_FAILURE;
	//计算原料消耗
	int need_count = armor_slot_material_count[slot_num][level];
	int material1_count = pImp->GetItemCount(MAKE_SLOT_ITEM_ID);
	int material2_count = pImp->GetItemCount(MAKE_SLOT_ITEM_ID2);
	if(need_count > material1_count + material2_count) return S2C::ERR_NOT_ENOUGH_MATERIAL;

	//增加孔
	_socket_list.push_back(0);
    count = GetSocketCount();
	//重新生成装备数据
	OnRefreshItem();
	//删除所需材料
	PlayerTaskInterface TaskIf(pImp);
	if(material1_count >= need_count)
		TaskIf.TakeAwayCommonItem(MAKE_SLOT_ITEM_ID, need_count);
	else
	{
		TaskIf.TakeAwayCommonItem(MAKE_SLOT_ITEM_ID, material1_count);
		TaskIf.TakeAwayCommonItem(MAKE_SLOT_ITEM_ID2, need_count - material1_count);
	}
	//返回成功
	return 0;
}


int decoration_equip_item::MakeSlot(gactive_imp* obj, int& count, unsigned int material_id, int material_count)
{
    gplayer_imp* pImp = (gplayer_imp*)obj;

    // 取出物品模板
    DATA_TYPE dt;
    DECORATION_ESSENCE* ess = (DECORATION_ESSENCE*)world_manager::GetDataMan().get_data_ptr(_tid, ID_SPACE_ESSENCE, dt);
    if ((ess == NULL) || (dt != DT_DECORATION_ESSENCE))
        return S2C::ERR_MAKE_SLOT_FAILURE;

    // 取出物品小类别
    DECORATION_SUB_TYPE* type = (DECORATION_SUB_TYPE*)world_manager::GetDataMan().get_data_ptr(ess->id_sub_type, ID_SPACE_ESSENCE, dt);
    if ((type == NULL) || (dt != DT_DECORATION_SUB_TYPE))
        return S2C::ERR_MAKE_SLOT_FAILURE;

    // 目前不支持戒指打孔
    unsigned int decoration_mask = (item::EQUIP_MASK64_NECK | item::EQUIP_MASK64_WAIST);
    if ((type->equip_mask & decoration_mask) == 0)
        return S2C::ERR_MAKE_SLOT_FAILURE;

    // 得到品级
    int level = ess->level;
    if ((level <= 0) || (level > 20))
        return S2C::ERR_MAKE_SLOT_FAILURE;
    --level;

    // 得到当前孔数
    int slot_num = GetSocketCount();
    if ((slot_num >= MAX_DECORATION_SOCKET_NUM) || (slot_num >= MAX_EQUIP_SOCKET_NUM))
        return S2C::ERR_MAKE_SLOT_FAILURE;

    // 计算原料消耗
    EQUIP_MAKE_HOLE_CONFIG* conf = (EQUIP_MAKE_HOLE_CONFIG*)world_manager::GetDataMan().get_data_ptr(EQUIP_MAKE_HOLE_CONFIG_ID, ID_SPACE_CONFIG, dt);
    if ((conf == NULL) || (dt != DT_EQUIP_MAKE_HOLE_CONFIG))
        return S2C::ERR_MAKE_SLOT_FAILURE;

    unsigned int require_item_id = conf->level_list[level].hole_list[slot_num].require_item_id;
    int require_item_count = conf->level_list[level].hole_list[slot_num].require_item_count;
    int fee = conf->level_list[level].hole_list[slot_num].fee;

    if ((require_item_id <= 0) || (require_item_count <= 0) || (fee < 0))
        return S2C::ERR_MAKE_SLOT_FAILURE;

    if (material_id != require_item_id)
        return S2C::ERR_NOT_ENOUGH_MATERIAL;

    if ((material_count <= 0) || (pImp->GetItemCount(material_id) < material_count))
        return S2C::ERR_NOT_ENOUGH_MATERIAL;

    if (material_count > require_item_count)
        material_count = require_item_count;

    float success_rate = (float)material_count / require_item_count;
    int fee_cost = (int)(fee * success_rate + 0.1f);

    if (pImp->GetMoney() < fee_cost)
        return S2C::ERR_OUT_OF_FUND;

    int ret = S2C::ERR_MAKE_SLOT_FOR_DECOR_PROB;
    if (abase::Rand(1, require_item_count) <= material_count)
    {
        // 增加孔
        _socket_list.push_back(0);
        count = GetSocketCount();
        // 重新生成装备数据
        OnRefreshItem();
        ret = S2C::ERR_SUCCESS;
    }

    if (ret == S2C::ERR_SUCCESS)
    {
        GLog::formatlog("玩家%d对饰品%d进行了打孔操作，打孔成功，操作后有%d个孔，消耗材料%u共%d个，消耗游戏币%d", pImp->_parent->ID.id, ess->id, GetSocketCount(), material_id, material_count, fee_cost);
    }
    else
    {
        GLog::formatlog("玩家%d对饰品%d进行了打孔操作，打孔失败，操作后有%d个孔，消耗材料%u共%d个，消耗游戏币%d，需要消耗材料%u共%d个才能保证打孔成功", pImp->_parent->ID.id, ess->id, GetSocketCount(), material_id, material_count, fee_cost, require_item_id, require_item_count);
    }

    // 删除所需材料
    PlayerTaskInterface TaskIf(pImp);
    TaskIf.TakeAwayCommonItem(material_id, material_count);
    pImp->SpendMoney(fee_cost);
    pImp->_runner->spend_money(fee_cost);

    return ret;
}


