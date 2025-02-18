#ifdef LINUX
#include <ASSERT.h>
#else
#include <ASSERT.h>
#define ASSERT ASSERT
#endif
#include "elementdataman.h"
#include "itemdataman.h"

#define CASE_GET_ITEM_ARG(ESSENCE,ARG) case DT_##ESSENCE: \
	{\
		return ((ESSENCE*)dataptr)->ARG; \
	}break;

itemdataman::itemdataman()
{
	_edm = new elementdataman;
}

itemdataman::~itemdataman()
{
	delete _edm;
	for(size_t i=0; i<sale_item_ptr_array.size(); i++)
	{
		abase::fastfree(sale_item_ptr_array[i], sale_item_size_array[i]);
	}
}

int itemdataman::load_data(const char * pathname,bool disable_bind2) 
{ 
	if(_edm->load_data(pathname) == 0)
	{
		generate_item_for_sell(disable_bind2);
		return 0;
	}
	else
		return -1;
}

unsigned int itemdataman::get_data_id(ID_SPACE idspace, unsigned int index, DATA_TYPE& datatype)
{	
	return _edm->get_data_id(idspace, index, datatype);
}

unsigned int itemdataman::get_first_data_id(ID_SPACE idspace, DATA_TYPE& datatype)
{
	return _edm->get_first_data_id(idspace, datatype);
}

unsigned int itemdataman::get_next_data_id(ID_SPACE idspace, DATA_TYPE& datatype)
{
	return _edm->get_next_data_id(idspace, datatype);
}

unsigned int itemdataman::get_data_num(ID_SPACE idspace)
{
	return _edm->get_data_num(idspace);
}

DATA_TYPE itemdataman::get_data_type(unsigned int id, ID_SPACE idspace)
{
	return _edm->get_data_type(id, idspace);
}

const void * itemdataman::get_data_ptr(unsigned int id, ID_SPACE idspace, DATA_TYPE& datatype)
{
	return _edm->get_data_ptr(id, idspace, datatype);
}

bool
itemdataman::generate_addon(unsigned int addon_id, addon_data & data)
{
	int rparanum;		
	if((rparanum = generate_addon(DT_INVALID,addon_id,data)) < 0) return false;
	data.id = (addon_id & (~(0x3<<13))) | (rparanum<<13);
	return true;
}

int 
itemdataman::generate_addon(DATA_TYPE datatype,unsigned int addon_id, addon_data & data)
{
	DATA_TYPE dt;
	EQUIPMENT_ADDON * addon = (EQUIPMENT_ADDON *) get_data_ptr(addon_id, ID_SPACE_ADDON, dt);
	size_t paramnum = 0;
	if(addon == NULL || dt != DT_EQUIPMENT_ADDON)	// error
	{
		return -1;
	}
	else
	{
		data.id = addon_id;
		paramnum = addon->num_params;
		int * param_ptr = &(addon->param1);
		for(size_t j=0; j<paramnum; j++, param_ptr++)
		{
			data.arg[j] = *param_ptr;
		}
	}
	return addon_generate_arg(datatype, data, paramnum);
}

size_t
itemdataman::generate_addon_buffer(DATA_TYPE datatype,unsigned int addon_id, char * buf)
{
	addon_data data;
	int rparam;
	if((rparam = generate_addon(datatype,addon_id,data)) < 0) return 0;
	int * pData = (int*)buf;
	*pData++ = (data.id & (~(0x3<<13))) | (rparam<<13);
	for(int i = 0; i < rparam; i ++)
	{
		*pData ++= data.arg[i];
	}
	return rparam * sizeof(int) + sizeof(int);	
}

size_t 
itemdataman::generate_spec_addon_buffer(DATA_TYPE essencetype, char * addon_buffer, size_t max_addon_size, size_t &addon_num, int * sa_list)
{
	addon_num = 0;
	if(!sa_list) return 0;
	size_t i;
	for(i = 0; i < max_addon_size; i ++)
	{
		if(sa_list[i] <= 0) break;
	}
	if(i == 0 ) return 0;
	addon_num = i;
	return generate_equipment_addon_buffer_2(essencetype,sa_list,sizeof(int),i,addon_buffer,addon_num);
}

size_t itemdataman::generate_equipment_addon_buffer
		(DATA_TYPE essencetype, 
		const char * candidate_header, 
		size_t candidate_num, 				
		char * addon_buffer,
		size_t &addon_num)
{
	if(!addon_num) return 0;
	int addon_list[32];
	size_t i;
	size_t anum;
	//进行addon的随机工作 
	for(anum = 0,i=0; i<addon_num; i++)
	{
		float * prob_header =(float*) (candidate_header + sizeof(unsigned int));
		int addon_index = abase::RandSelect(prob_header, sizeof(unsigned int)+sizeof(float), candidate_num);
		int id	= *(int*)(candidate_header + addon_index*(sizeof(unsigned int)+sizeof(float)));
		if(id <= 0) continue;
		/*
		//不会生成多个一样的addon
		for(j =0; j < anum ; j ++)
		{
			if(addon_list[j] == id) break;
		}
		if(j != anum) continue;
		*/
		addon_list[anum++] = id;


	}
	addon_num = anum;
	return generate_equipment_addon_buffer_2(essencetype,addon_list,sizeof(int),anum,addon_buffer,addon_num);
}

size_t itemdataman::generate_equipment_addon_buffer_2
		(DATA_TYPE essencetype, 
		int * candidate_addon, 
		size_t candidate_addon_size,
		size_t candidate_num, 				
		char * addon_buffer,
		size_t &addon_num)
{
	//生成多个addon，可能会有部分生成失败
	if(!addon_num) return 0;
	char * addon_sld = addon_buffer;
	size_t i,j;
	size_t anum = 0;
	for(i=0; i<candidate_num; i++)
	{
		int id = *(int*)( ((char*)candidate_addon) + (candidate_addon_size)*i);
		if(id <= 0) continue;

		//开始生成 addon
		addon_data addondata;
		int rparanum;
		if((rparanum = generate_addon(essencetype,id,addondata)) < 0) continue;
		*(int*)addon_sld = (addondata.id &(~(0x3<<13))) | (rparanum<<13);
		addon_sld += sizeof(int);
		anum ++;
		for(j=0; j<(size_t)rparanum; j++)
		{
			*(int*)addon_sld = addondata.arg[j];
			addon_sld += sizeof(int);
		}
	}
	addon_num = anum;
	return addon_sld - addon_buffer;	
}

int itemdataman::get_refine_meterial_id()
{
	return 11208;
}

int itemdataman::get_item_refine_addon(unsigned int id,int & material_need)
{
	DATA_TYPE datatype;
	const void * dataptr = get_data_ptr(id, ID_SPACE_ESSENCE, datatype);
	if(dataptr == NULL || datatype == DT_INVALID)	return 0;
	
	switch(datatype) {
	case DT_WEAPON_ESSENCE:
		material_need = ((WEAPON_ESSENCE*)dataptr)->material_need;
		return ((WEAPON_ESSENCE*)dataptr)->levelup_addon;

	case DT_ARMOR_ESSENCE:
		material_need = ((ARMOR_ESSENCE*)dataptr)->material_need;
		return ((ARMOR_ESSENCE*)dataptr)->levelup_addon;

	case DT_DECORATION_ESSENCE:
		material_need = ((DECORATION_ESSENCE*)dataptr)->material_need;
		return ((DECORATION_ESSENCE*)dataptr)->levelup_addon;
	default:
		break;
	}
	return 0;
}

int itemdataman::get_item_proc_type(unsigned int id)
{
	DATA_TYPE datatype;
	const void * dataptr = get_data_ptr(id, ID_SPACE_ESSENCE, datatype);
	if(dataptr == NULL || datatype == DT_INVALID)	return 0;
	
	switch(datatype) {
	case DT_WEAPON_ESSENCE:
		return ((WEAPON_ESSENCE*)dataptr)->proc_type;

	case DT_ARMOR_ESSENCE:
		return ((ARMOR_ESSENCE*)dataptr)->proc_type;

	case DT_PROJECTILE_ESSENCE:
		return ((PROJECTILE_ESSENCE*)dataptr)->proc_type;

	case DT_DECORATION_ESSENCE:
		return ((DECORATION_ESSENCE*)dataptr)->proc_type;		

	case DT_STONE_ESSENCE:
		return ((STONE_ESSENCE*)dataptr)->proc_type;
		
	case DT_MEDICINE_ESSENCE:
		return ((MEDICINE_ESSENCE*)dataptr)->proc_type;

	case DT_WINGMANWING_ESSENCE:
		return ((WINGMANWING_ESSENCE*)dataptr)->proc_type;

	case DT_MATERIAL_ESSENCE:
		return ((MATERIAL_ESSENCE*)dataptr)->proc_type;
		
	case DT_DYE_TICKET_ESSENCE:
		return ((DYE_TICKET_ESSENCE*)dataptr)->proc_type;
	
	case DT_DAMAGERUNE_ESSENCE:
		return ((DAMAGERUNE_ESSENCE*)dataptr)->proc_type;

	case DT_ARMORRUNE_ESSENCE:
		return ((ARMORRUNE_ESSENCE*)dataptr)->proc_type;

	case DT_SKILLTOME_ESSENCE:
		return ((SKILLTOME_ESSENCE*)dataptr)->proc_type;

	case DT_FLYSWORD_ESSENCE:
		return ((FLYSWORD_ESSENCE*)dataptr)->proc_type;

	case DT_TOWNSCROLL_ESSENCE:
		return ((TOWNSCROLL_ESSENCE*)dataptr)->proc_type;

	case DT_UNIONSCROLL_ESSENCE:
		return ((UNIONSCROLL_ESSENCE*)dataptr)->proc_type;
	
	case DT_REVIVESCROLL_ESSENCE:
		return ((REVIVESCROLL_ESSENCE*)dataptr)->proc_type;

	case DT_ELEMENT_ESSENCE:
		return ((ELEMENT_ESSENCE*)dataptr)->proc_type;

	case DT_TASKMATTER_ESSENCE:
		return ((TASKMATTER_ESSENCE*)dataptr)->proc_type;

	case DT_TOSSMATTER_ESSENCE:
		return ((TOSSMATTER_ESSENCE*)dataptr)->proc_type;

	case DT_TASKNORMALMATTER_ESSENCE:
		return ((TASKNORMALMATTER_ESSENCE*)dataptr)->proc_type;
	
	case DT_TASKDICE_ESSENCE:
		return ((TASKDICE_ESSENCE*)dataptr)->proc_type;

	case DT_FASHION_ESSENCE:
		return ((FASHION_ESSENCE*)dataptr)->proc_type;

	case DT_FACEPILL_ESSENCE:
		return ((FACEPILL_ESSENCE*)dataptr)->proc_type;

	case DT_FACETICKET_ESSENCE:
		return ((FACETICKET_ESSENCE*)dataptr)->proc_type;
		
	case DT_PET_EGG_ESSENCE:
		return ((PET_EGG_ESSENCE*)dataptr)->proc_type;

	case DT_PET_FOOD_ESSENCE:
		return ((PET_FOOD_ESSENCE*)dataptr)->proc_type;

	case DT_PET_FACETICKET_ESSENCE:
		return ((PET_FACETICKET_ESSENCE*)dataptr)->proc_type;

	case DT_FIREWORKS_ESSENCE:
		return ((FIREWORKS_ESSENCE*)dataptr)->proc_type;

	case DT_WAR_TANKCALLIN_ESSENCE:
		return ((WAR_TANKCALLIN_ESSENCE*)dataptr)->proc_type;

	case DT_SKILLMATTER_ESSENCE:
		return ((SKILLMATTER_ESSENCE*)dataptr)->proc_type;
		
	case DT_REFINE_TICKET_ESSENCE:
		return((REFINE_TICKET_ESSENCE*)dataptr)->proc_type;
		
	case DT_DESTROYING_ESSENCE:
		return ((DESTROYING_ESSENCE*)dataptr)->proc_type;
		
	case DT_BIBLE_ESSENCE:
		return((BIBLE_ESSENCE*)dataptr)->proc_type;

	case DT_SPEAKER_ESSENCE:
		return((SPEAKER_ESSENCE*)dataptr)->proc_type;

	case DT_AUTOMP_ESSENCE:
		return((AUTOMP_ESSENCE*)dataptr)->proc_type;

	case DT_AUTOHP_ESSENCE:
		return((AUTOHP_ESSENCE*)dataptr)->proc_type;
		
	case DT_GM_GENERATOR_ESSENCE:
		return 0;

	case DT_DOUBLE_EXP_ESSENCE:
		return((DOUBLE_EXP_ESSENCE*)dataptr)->proc_type;		

	case DT_TRANSMITSCROLL_ESSENCE:
		return((TRANSMITSCROLL_ESSENCE*)dataptr)->proc_type;

	case DT_GOBLIN_ESSENCE:
		return((GOBLIN_ESSENCE*)dataptr)->proc_type;
		
	case DT_GOBLIN_EQUIP_ESSENCE:
		return((GOBLIN_EQUIP_ESSENCE*)dataptr)->proc_type;
		
	case DT_GOBLIN_EXPPILL_ESSENCE:
		return((GOBLIN_EXPPILL_ESSENCE*)dataptr)->proc_type;
		
	case DT_SELL_CERTIFICATE_ESSENCE:
		return((SELL_CERTIFICATE_ESSENCE*)dataptr)->proc_type;
	
	case DT_TARGET_ITEM_ESSENCE:
		return((TARGET_ITEM_ESSENCE*)dataptr)->proc_type;
	
	case DT_LOOK_INFO_ESSENCE:
		return((LOOK_INFO_ESSENCE*)dataptr)->proc_type;
	
	case DT_INC_SKILL_ABILITY_ESSENCE:
		return((INC_SKILL_ABILITY_ESSENCE*)dataptr)->proc_type;
		
	case DT_WEDDING_BOOKCARD_ESSENCE:
		return((WEDDING_BOOKCARD_ESSENCE*)dataptr)->proc_type;
		
	case DT_WEDDING_INVITECARD_ESSENCE:
		return((WEDDING_INVITECARD_ESSENCE*)dataptr)->proc_type;
		
	case DT_SHARPENER_ESSENCE:
		return((SHARPENER_ESSENCE*)dataptr)->proc_type;
		
	case DT_FACTION_MATERIAL_ESSENCE:
		return((FACTION_MATERIAL_ESSENCE*)dataptr)->proc_type;
		
	case DT_CONGREGATE_ESSENCE:
		return((CONGREGATE_ESSENCE*)dataptr)->proc_type;
		
	case DT_FORCE_TOKEN_ESSENCE:
		return((FORCE_TOKEN_ESSENCE*)dataptr)->proc_type;
		
	case DT_DYNSKILLEQUIP_ESSENCE:
		return((DYNSKILLEQUIP_ESSENCE*)dataptr)->proc_type;
		
	case DT_MONEY_CONVERTIBLE_ESSENCE:
		return((MONEY_CONVERTIBLE_ESSENCE*)dataptr)->proc_type;

	case DT_MONSTER_SPIRIT_ESSENCE:
		return ((MONSTER_SPIRIT_ESSENCE*)dataptr)->proc_type;
		
	case DT_POKER_ESSENCE:
		return((POKER_ESSENCE*)dataptr)->proc_type;
		
	case DT_POKER_DICE_ESSENCE:
		return((POKER_DICE_ESSENCE*)dataptr)->proc_type;

	case DT_SHOP_TOKEN_ESSENCE:
		return((SHOP_TOKEN_ESSENCE*)dataptr)->proc_type;
		
	case DT_FIREWORKS2_ESSENCE:
		return ((FIREWORKS2_ESSENCE*)dataptr)->proc_type;
		
	case DT_FIX_POSITION_TRANSMIT_ESSENCE:
		return ((FIX_POSITION_TRANSMIT_ESSENCE*)dataptr)->proc_type;

		
	case DT_UNIVERSAL_TOKEN_ESSENCE:
		return((UNIVERSAL_TOKEN_ESSENCE*)dataptr)->proc_type;

	CASE_GET_ITEM_ARG(ASTROLABE_ESSENCE, proc_type)
	CASE_GET_ITEM_ARG(ASTROLABE_RANDOM_ADDON_ESSENCE, proc_type)
	CASE_GET_ITEM_ARG(ASTROLABE_INC_INNER_POINT_VALUE_ESSENCE, proc_type)
	CASE_GET_ITEM_ARG(ASTROLABE_INC_EXP_ESSENCE, proc_type)
	CASE_GET_ITEM_ARG(ITEM_PACKAGE_BY_PROFESSION_ESSENCE, proc_type)

	default:
		return 0;
	}
	return 0;
}

int itemdataman::get_item_sell_price(unsigned int id)
{
	DATA_TYPE datatype;
	const void * dataptr = get_data_ptr(id, ID_SPACE_ESSENCE, datatype);
	if(dataptr == NULL || datatype == DT_INVALID)	return 0;
	
	switch(datatype) {
	case DT_WEAPON_ESSENCE:
		return ((WEAPON_ESSENCE*)dataptr)->price;

	case DT_ARMOR_ESSENCE:
		return ((ARMOR_ESSENCE*)dataptr)->price;

	case DT_PROJECTILE_ESSENCE:
		return ((PROJECTILE_ESSENCE*)dataptr)->price;

	case DT_DECORATION_ESSENCE:
		return ((DECORATION_ESSENCE*)dataptr)->price;		

	case DT_STONE_ESSENCE:
		return ((STONE_ESSENCE*)dataptr)->price;
		
	case DT_MEDICINE_ESSENCE:
		return ((MEDICINE_ESSENCE*)dataptr)->price;

	case DT_WINGMANWING_ESSENCE:
		return ((WINGMANWING_ESSENCE*)dataptr)->price;

	case DT_MATERIAL_ESSENCE:
		return ((MATERIAL_ESSENCE*)dataptr)->price;
		
	case DT_DYE_TICKET_ESSENCE:
		return ((DYE_TICKET_ESSENCE*)dataptr)->price;
	
	case DT_DAMAGERUNE_ESSENCE:
		return ((DAMAGERUNE_ESSENCE*)dataptr)->price;

	case DT_ARMORRUNE_ESSENCE:
		return ((ARMORRUNE_ESSENCE*)dataptr)->price;

	case DT_SKILLTOME_ESSENCE:
		return ((SKILLTOME_ESSENCE*)dataptr)->price;

	case DT_FLYSWORD_ESSENCE:
		return ((FLYSWORD_ESSENCE*)dataptr)->price;

	case DT_TOWNSCROLL_ESSENCE:
		return ((TOWNSCROLL_ESSENCE*)dataptr)->price;

	case DT_UNIONSCROLL_ESSENCE:
		return ((UNIONSCROLL_ESSENCE*)dataptr)->price;
	
	case DT_REVIVESCROLL_ESSENCE:
		return ((REVIVESCROLL_ESSENCE*)dataptr)->price;

	case DT_ELEMENT_ESSENCE:
		return ((ELEMENT_ESSENCE*)dataptr)->price;

	case DT_TOSSMATTER_ESSENCE:
		return ((TOSSMATTER_ESSENCE*)dataptr)->price;

	case DT_TASKNORMALMATTER_ESSENCE:
		return ((TASKNORMALMATTER_ESSENCE*)dataptr)->price;
	
	case DT_FASHION_ESSENCE:
		return ((FASHION_ESSENCE*)dataptr)->price;

	case DT_FACEPILL_ESSENCE:
		return ((FACEPILL_ESSENCE*)dataptr)->price;

	case DT_FACETICKET_ESSENCE:
		return ((FACETICKET_ESSENCE*)dataptr)->price;
		
	case DT_PET_EGG_ESSENCE:
		return ((PET_EGG_ESSENCE*)dataptr)->price;

	case DT_PET_FOOD_ESSENCE:
		return ((PET_FOOD_ESSENCE*)dataptr)->price;

	case DT_FIREWORKS_ESSENCE:
		return ((FIREWORKS_ESSENCE*)dataptr)->price;

	case DT_WAR_TANKCALLIN_ESSENCE:
		return ((WAR_TANKCALLIN_ESSENCE*)dataptr)->price;

	case DT_SKILLMATTER_ESSENCE:
		return ((SKILLMATTER_ESSENCE*)dataptr)->price;
		
	case DT_REFINE_TICKET_ESSENCE:
		return((REFINE_TICKET_ESSENCE*)dataptr)->price;
		
	case DT_BIBLE_ESSENCE:
		return((BIBLE_ESSENCE*)dataptr)->price;

	case DT_SPEAKER_ESSENCE:
		return((SPEAKER_ESSENCE*)dataptr)->price;

	case DT_AUTOMP_ESSENCE:
		return((AUTOMP_ESSENCE*)dataptr)->price;

	case DT_AUTOHP_ESSENCE:
		return((AUTOHP_ESSENCE*)dataptr)->price;
		
	case DT_GM_GENERATOR_ESSENCE:
		return 0;

	case DT_DOUBLE_EXP_ESSENCE:
		return((DOUBLE_EXP_ESSENCE*)dataptr)->price;		

	case DT_TRANSMITSCROLL_ESSENCE:
		return((TRANSMITSCROLL_ESSENCE*)dataptr)->price;
	
	case DT_GOBLIN_ESSENCE:
		return((GOBLIN_ESSENCE*)dataptr)->price;
		
	case DT_GOBLIN_EQUIP_ESSENCE:
		return((GOBLIN_EQUIP_ESSENCE*)dataptr)->price;
		
	case DT_GOBLIN_EXPPILL_ESSENCE:
		return((GOBLIN_EXPPILL_ESSENCE*)dataptr)->price;

	case DT_SELL_CERTIFICATE_ESSENCE:
		return((SELL_CERTIFICATE_ESSENCE*)dataptr)->price;
	
	case DT_TARGET_ITEM_ESSENCE:
		return((TARGET_ITEM_ESSENCE*)dataptr)->price;
	
	case DT_LOOK_INFO_ESSENCE:
		return((LOOK_INFO_ESSENCE*)dataptr)->price;
	
	case DT_INC_SKILL_ABILITY_ESSENCE:
		return((INC_SKILL_ABILITY_ESSENCE*)dataptr)->price;
		
	case DT_WEDDING_BOOKCARD_ESSENCE:
		return((WEDDING_BOOKCARD_ESSENCE*)dataptr)->price;
		
	case DT_WEDDING_INVITECARD_ESSENCE:
		return((WEDDING_INVITECARD_ESSENCE*)dataptr)->price;
		
	case DT_SHARPENER_ESSENCE:
		return((SHARPENER_ESSENCE*)dataptr)->price;
		
	case DT_FACTION_MATERIAL_ESSENCE:
		return((FACTION_MATERIAL_ESSENCE*)dataptr)->price;
		
	case DT_CONGREGATE_ESSENCE:
		return((CONGREGATE_ESSENCE*)dataptr)->price;
		
	case DT_FORCE_TOKEN_ESSENCE:
		return((FORCE_TOKEN_ESSENCE*)dataptr)->price;
		
	case DT_DYNSKILLEQUIP_ESSENCE:
		return((DYNSKILLEQUIP_ESSENCE*)dataptr)->price;
		
	case DT_MONEY_CONVERTIBLE_ESSENCE:
		return((MONEY_CONVERTIBLE_ESSENCE*)dataptr)->price;

	case DT_MONSTER_SPIRIT_ESSENCE:
		return ((MONSTER_SPIRIT_ESSENCE*)dataptr)->price;
		
	case DT_POKER_ESSENCE:
		return((POKER_ESSENCE*)dataptr)->price;
		
	case DT_POKER_DICE_ESSENCE:
		return((POKER_DICE_ESSENCE*)dataptr)->price;

	case DT_SHOP_TOKEN_ESSENCE:
		return((SHOP_TOKEN_ESSENCE*)dataptr)->price;
		
	case DT_UNIVERSAL_TOKEN_ESSENCE:
		return((UNIVERSAL_TOKEN_ESSENCE*)dataptr)->price;

	case DT_FIREWORKS2_ESSENCE:
		return ((FIREWORKS2_ESSENCE*)dataptr)->price;
		
	case DT_FIX_POSITION_TRANSMIT_ESSENCE:
		return ((FIX_POSITION_TRANSMIT_ESSENCE*)dataptr)->price;


	CASE_GET_ITEM_ARG(ASTROLABE_ESSENCE, price)
	CASE_GET_ITEM_ARG(ASTROLABE_RANDOM_ADDON_ESSENCE, price)
	CASE_GET_ITEM_ARG(ASTROLABE_INC_INNER_POINT_VALUE_ESSENCE, price)
	CASE_GET_ITEM_ARG(ASTROLABE_INC_EXP_ESSENCE, price)
	CASE_GET_ITEM_ARG(ITEM_PACKAGE_BY_PROFESSION_ESSENCE, price)

	default:
		return 0;
	}
	return 0;
}

int itemdataman::get_item_shop_price(unsigned int id)
{
	DATA_TYPE datatype;
	const void * dataptr = get_data_ptr(id, ID_SPACE_ESSENCE, datatype);
	if(dataptr == NULL || datatype == DT_INVALID)	return 0;
	
	switch(datatype) {
	case DT_WEAPON_ESSENCE:
		return ((WEAPON_ESSENCE*)dataptr)->shop_price;

	case DT_ARMOR_ESSENCE:
		return ((ARMOR_ESSENCE*)dataptr)->shop_price;

	case DT_PROJECTILE_ESSENCE:
		return ((PROJECTILE_ESSENCE*)dataptr)->shop_price;

	case DT_DECORATION_ESSENCE:
		return ((DECORATION_ESSENCE*)dataptr)->shop_price;		

	case DT_STONE_ESSENCE:
		return ((STONE_ESSENCE*)dataptr)->shop_price;
		
	case DT_MEDICINE_ESSENCE:
		return ((MEDICINE_ESSENCE*)dataptr)->shop_price;

	case DT_WINGMANWING_ESSENCE:
		return ((WINGMANWING_ESSENCE*)dataptr)->shop_price;

	case DT_MATERIAL_ESSENCE:
		return ((MATERIAL_ESSENCE*)dataptr)->shop_price;

	case DT_DYE_TICKET_ESSENCE:
		return ((DYE_TICKET_ESSENCE*)dataptr)->shop_price;
	
	case DT_DAMAGERUNE_ESSENCE:
		return ((DAMAGERUNE_ESSENCE*)dataptr)->shop_price;

	case DT_ARMORRUNE_ESSENCE:
		return ((ARMORRUNE_ESSENCE*)dataptr)->shop_price;

	case DT_SKILLTOME_ESSENCE:
		return ((SKILLTOME_ESSENCE*)dataptr)->shop_price;

	case DT_FLYSWORD_ESSENCE:
		return ((FLYSWORD_ESSENCE*)dataptr)->shop_price;

	case DT_TOWNSCROLL_ESSENCE:
		return ((TOWNSCROLL_ESSENCE*)dataptr)->shop_price;

	case DT_UNIONSCROLL_ESSENCE:
		return ((UNIONSCROLL_ESSENCE*)dataptr)->shop_price;
	
	case DT_REVIVESCROLL_ESSENCE:
		return ((REVIVESCROLL_ESSENCE*)dataptr)->shop_price;

	case DT_ELEMENT_ESSENCE:
		return ((ELEMENT_ESSENCE*)dataptr)->shop_price;

	case DT_TOSSMATTER_ESSENCE:
		return ((TOSSMATTER_ESSENCE*)dataptr)->shop_price;

	case DT_TASKNORMALMATTER_ESSENCE:
		return ((TASKNORMALMATTER_ESSENCE*)dataptr)->shop_price;
	
	case DT_FASHION_ESSENCE:
		return ((FASHION_ESSENCE*)dataptr)->shop_price;

	case DT_FACEPILL_ESSENCE:
		return ((FACEPILL_ESSENCE*)dataptr)->shop_price;

	case DT_FACETICKET_ESSENCE:
		return ((FACETICKET_ESSENCE*)dataptr)->shop_price;	

	case DT_PET_EGG_ESSENCE:
		return ((PET_EGG_ESSENCE*)dataptr)->shop_price;

	case DT_PET_FOOD_ESSENCE:
		return ((PET_FOOD_ESSENCE*)dataptr)->shop_price;

	case DT_FIREWORKS_ESSENCE:
		return ((FIREWORKS_ESSENCE*)dataptr)->shop_price;

	case DT_WAR_TANKCALLIN_ESSENCE:
		return ((WAR_TANKCALLIN_ESSENCE*)dataptr)->shop_price;

	case DT_SKILLMATTER_ESSENCE:
		return ((SKILLMATTER_ESSENCE*)dataptr)->shop_price;

	case DT_REFINE_TICKET_ESSENCE:
		return((REFINE_TICKET_ESSENCE*)dataptr)->shop_price;
		
	case DT_BIBLE_ESSENCE:
		return((BIBLE_ESSENCE*)dataptr)->shop_price;

	case DT_SPEAKER_ESSENCE:
		return((SPEAKER_ESSENCE*)dataptr)->shop_price;

	case DT_AUTOMP_ESSENCE:
		return((AUTOMP_ESSENCE*)dataptr)->shop_price;

	case DT_AUTOHP_ESSENCE:
		return((AUTOHP_ESSENCE*)dataptr)->shop_price;
		
	case DT_GM_GENERATOR_ESSENCE:
		return 0;

	case DT_DOUBLE_EXP_ESSENCE:
		return((DOUBLE_EXP_ESSENCE*)dataptr)->shop_price;		

	case DT_TRANSMITSCROLL_ESSENCE:
		return((TRANSMITSCROLL_ESSENCE*)dataptr)->shop_price;

	case DT_GOBLIN_ESSENCE:
		return((GOBLIN_ESSENCE*)dataptr)->shop_price;
		
	case DT_GOBLIN_EQUIP_ESSENCE:
		return((GOBLIN_EQUIP_ESSENCE*)dataptr)->shop_price;
		
	case DT_GOBLIN_EXPPILL_ESSENCE:
		return((GOBLIN_EXPPILL_ESSENCE*)dataptr)->shop_price;

	case DT_SELL_CERTIFICATE_ESSENCE:
		return((SELL_CERTIFICATE_ESSENCE*)dataptr)->shop_price;
	
	case DT_TARGET_ITEM_ESSENCE:
		return((TARGET_ITEM_ESSENCE*)dataptr)->shop_price;
	
	case DT_LOOK_INFO_ESSENCE:
		return((LOOK_INFO_ESSENCE*)dataptr)->shop_price;
	
	case DT_INC_SKILL_ABILITY_ESSENCE:
		return((INC_SKILL_ABILITY_ESSENCE*)dataptr)->shop_price;
		
	case DT_WEDDING_BOOKCARD_ESSENCE:
		return((WEDDING_BOOKCARD_ESSENCE*)dataptr)->shop_price;
		
	case DT_WEDDING_INVITECARD_ESSENCE:
		return((WEDDING_INVITECARD_ESSENCE*)dataptr)->shop_price;
		
	case DT_SHARPENER_ESSENCE:
		return((SHARPENER_ESSENCE*)dataptr)->shop_price;
		
	case DT_FACTION_MATERIAL_ESSENCE:
		return((FACTION_MATERIAL_ESSENCE*)dataptr)->shop_price;
		
	case DT_CONGREGATE_ESSENCE:
		return((CONGREGATE_ESSENCE*)dataptr)->shop_price;
		
	case DT_FORCE_TOKEN_ESSENCE:
		return((FORCE_TOKEN_ESSENCE*)dataptr)->shop_price;
		
	case DT_DYNSKILLEQUIP_ESSENCE:
		return((DYNSKILLEQUIP_ESSENCE*)dataptr)->shop_price;
		
	case DT_MONEY_CONVERTIBLE_ESSENCE:
		return((MONEY_CONVERTIBLE_ESSENCE*)dataptr)->shop_price;
	
	case DT_MONSTER_SPIRIT_ESSENCE:
		return ((MONSTER_SPIRIT_ESSENCE*)dataptr)->shop_price;
		
	case DT_POKER_ESSENCE:
		return((POKER_ESSENCE*)dataptr)->shop_price;
		
	case DT_POKER_DICE_ESSENCE:
		return((POKER_DICE_ESSENCE*)dataptr)->shop_price;

	case DT_SHOP_TOKEN_ESSENCE:
		return((SHOP_TOKEN_ESSENCE*)dataptr)->shop_price;
		
	case DT_UNIVERSAL_TOKEN_ESSENCE:
		return((UNIVERSAL_TOKEN_ESSENCE*)dataptr)->shop_price;

	case DT_FIREWORKS2_ESSENCE:
		return ((FIREWORKS2_ESSENCE*)dataptr)->shop_price;
		
	case DT_FIX_POSITION_TRANSMIT_ESSENCE:
		return ((FIX_POSITION_TRANSMIT_ESSENCE*)dataptr)->shop_price;


	CASE_GET_ITEM_ARG(ASTROLABE_ESSENCE, shop_price)
	CASE_GET_ITEM_ARG(ASTROLABE_RANDOM_ADDON_ESSENCE, shop_price)
	CASE_GET_ITEM_ARG(ASTROLABE_INC_INNER_POINT_VALUE_ESSENCE, shop_price)
	CASE_GET_ITEM_ARG(ASTROLABE_INC_EXP_ESSENCE, shop_price)
	CASE_GET_ITEM_ARG(ITEM_PACKAGE_BY_PROFESSION_ESSENCE, shop_price)

	default:
		return 0;
	}
	return 0;
}

int itemdataman::get_item_pile_limit(unsigned int id)
{
	DATA_TYPE datatype;
	const void * dataptr = get_data_ptr(id, ID_SPACE_ESSENCE, datatype);
	if(dataptr == NULL || datatype == DT_INVALID)	return 0;
	
	switch(datatype) {
	case DT_WEAPON_ESSENCE:
		return ((WEAPON_ESSENCE*)dataptr)->pile_num_max;

	case DT_ARMOR_ESSENCE:
		return ((ARMOR_ESSENCE*)dataptr)->pile_num_max;

	case DT_PROJECTILE_ESSENCE:
		return ((PROJECTILE_ESSENCE*)dataptr)->pile_num_max;

	case DT_DECORATION_ESSENCE:
		return ((DECORATION_ESSENCE*)dataptr)->pile_num_max;		

	case DT_STONE_ESSENCE:
		return ((STONE_ESSENCE*)dataptr)->pile_num_max;
		
	case DT_MEDICINE_ESSENCE:
		return ((MEDICINE_ESSENCE*)dataptr)->pile_num_max;

	case DT_WINGMANWING_ESSENCE:
		return ((WINGMANWING_ESSENCE*)dataptr)->pile_num_max;

	case DT_MATERIAL_ESSENCE:
		return ((MATERIAL_ESSENCE*)dataptr)->pile_num_max;

	case DT_DYE_TICKET_ESSENCE:
		return ((DYE_TICKET_ESSENCE*)dataptr)->pile_num_max;
	
	case DT_DAMAGERUNE_ESSENCE:
		return ((DAMAGERUNE_ESSENCE*)dataptr)->pile_num_max;

	case DT_ARMORRUNE_ESSENCE:
		return ((ARMORRUNE_ESSENCE*)dataptr)->pile_num_max;

	case DT_SKILLTOME_ESSENCE:
		return ((SKILLTOME_ESSENCE*)dataptr)->pile_num_max;

	case DT_FLYSWORD_ESSENCE:
		return ((FLYSWORD_ESSENCE*)dataptr)->pile_num_max;

	case DT_TOWNSCROLL_ESSENCE:
		return ((TOWNSCROLL_ESSENCE*)dataptr)->pile_num_max;

	case DT_UNIONSCROLL_ESSENCE:
		return ((UNIONSCROLL_ESSENCE*)dataptr)->pile_num_max;
	
	case DT_REVIVESCROLL_ESSENCE:
		return ((REVIVESCROLL_ESSENCE*)dataptr)->pile_num_max;

	case DT_ELEMENT_ESSENCE:
		return ((ELEMENT_ESSENCE*)dataptr)->pile_num_max;

	case DT_TASKMATTER_ESSENCE:
		return ((TASKMATTER_ESSENCE*)dataptr)->pile_num_max;

	case DT_TASKDICE_ESSENCE:
		return ((TASKDICE_ESSENCE*)dataptr)->pile_num_max;

	case DT_TOSSMATTER_ESSENCE:
		return ((TOSSMATTER_ESSENCE*)dataptr)->pile_num_max;

	case DT_TASKNORMALMATTER_ESSENCE:
		return ((TASKNORMALMATTER_ESSENCE*)dataptr)->pile_num_max;
	
	case DT_FASHION_ESSENCE:
		return ((FASHION_ESSENCE*)dataptr)->pile_num_max;

	case DT_FACEPILL_ESSENCE:
		return ((FACEPILL_ESSENCE*)dataptr)->pile_num_max;

	case DT_FACETICKET_ESSENCE:
		return ((FACETICKET_ESSENCE*)dataptr)->pile_num_max;	
		
	case DT_GM_GENERATOR_ESSENCE:
		return ((FACETICKET_ESSENCE*)dataptr)->pile_num_max;
		
	case DT_PET_EGG_ESSENCE:
		return ((PET_EGG_ESSENCE*)dataptr)->pile_num_max;

	case DT_PET_FOOD_ESSENCE:
		return ((PET_FOOD_ESSENCE*)dataptr)->pile_num_max;
		
	case DT_FIREWORKS_ESSENCE:
		return ((FIREWORKS_ESSENCE*)dataptr)->pile_num_max;

	case DT_WAR_TANKCALLIN_ESSENCE:
		return ((WAR_TANKCALLIN_ESSENCE*)dataptr)->pile_num_max;

	case DT_SKILLMATTER_ESSENCE:
		return ((SKILLMATTER_ESSENCE*)dataptr)->pile_num_max;

	case DT_REFINE_TICKET_ESSENCE:
		return((REFINE_TICKET_ESSENCE*)dataptr)->pile_num_max;

	case DT_BIBLE_ESSENCE:
		return 1;	

	case DT_SPEAKER_ESSENCE:
		return 1;	

	case DT_AUTOMP_ESSENCE:
		return 1;	

	case DT_AUTOHP_ESSENCE:
		return 1;	

	case DT_DESTROYING_ESSENCE:
		return 1;

	case DT_DOUBLE_EXP_ESSENCE:
		return((DOUBLE_EXP_ESSENCE*)dataptr)->pile_num_max;		

	case DT_TRANSMITSCROLL_ESSENCE:
		return((TRANSMITSCROLL_ESSENCE*)dataptr)->pile_num_max;
	
	case DT_GOBLIN_ESSENCE:		//lgc
		return ((GOBLIN_ESSENCE*)dataptr)->pile_num_max;
	
	case DT_GOBLIN_EQUIP_ESSENCE:
		return ((GOBLIN_EQUIP_ESSENCE*)dataptr)->pile_num_max;
	
	case DT_GOBLIN_EXPPILL_ESSENCE:
		return ((GOBLIN_EXPPILL_ESSENCE*)dataptr)->pile_num_max;
	
	case DT_SELL_CERTIFICATE_ESSENCE:
		return ((SELL_CERTIFICATE_ESSENCE*)dataptr)->pile_num_max;
	
	case DT_TARGET_ITEM_ESSENCE:
		return ((TARGET_ITEM_ESSENCE*)dataptr)->pile_num_max;
	
	case DT_LOOK_INFO_ESSENCE:
		return ((LOOK_INFO_ESSENCE*)dataptr)->pile_num_max;
	
	case DT_INC_SKILL_ABILITY_ESSENCE:
		return((INC_SKILL_ABILITY_ESSENCE*)dataptr)->pile_num_max;

	case DT_WEDDING_BOOKCARD_ESSENCE:
		return((WEDDING_BOOKCARD_ESSENCE*)dataptr)->pile_num_max;

	case DT_WEDDING_INVITECARD_ESSENCE:
		return((WEDDING_INVITECARD_ESSENCE*)dataptr)->pile_num_max;

	case DT_SHARPENER_ESSENCE:
		return((SHARPENER_ESSENCE*)dataptr)->pile_num_max;

	case DT_FACTION_MATERIAL_ESSENCE:
		return((FACTION_MATERIAL_ESSENCE*)dataptr)->pile_num_max;

	case DT_CONGREGATE_ESSENCE:
		return((CONGREGATE_ESSENCE*)dataptr)->pile_num_max;

	case DT_FORCE_TOKEN_ESSENCE:
		return((FORCE_TOKEN_ESSENCE*)dataptr)->pile_num_max;

	case DT_DYNSKILLEQUIP_ESSENCE:
		return((DYNSKILLEQUIP_ESSENCE*)dataptr)->pile_num_max;

	case DT_MONEY_CONVERTIBLE_ESSENCE:
		return((MONEY_CONVERTIBLE_ESSENCE*)dataptr)->pile_num_max;

	case DT_MONSTER_SPIRIT_ESSENCE:
		return ((MONSTER_SPIRIT_ESSENCE*)dataptr)->pile_num_max;

	case DT_POKER_ESSENCE:
		return((POKER_ESSENCE*)dataptr)->pile_num_max;

	case DT_POKER_DICE_ESSENCE:
		return((POKER_DICE_ESSENCE*)dataptr)->pile_num_max;
	
	case DT_SHOP_TOKEN_ESSENCE:
		return((SHOP_TOKEN_ESSENCE*)dataptr)->pile_num_max;

	case DT_UNIVERSAL_TOKEN_ESSENCE:
		return((UNIVERSAL_TOKEN_ESSENCE*)dataptr)->pile_num_max;
	
	case DT_FIREWORKS2_ESSENCE:
		return ((FIREWORKS2_ESSENCE*)dataptr)->pile_num_max;
		
	case DT_FIX_POSITION_TRANSMIT_ESSENCE:
		return ((FIX_POSITION_TRANSMIT_ESSENCE*)dataptr)->pile_num_max;


	CASE_GET_ITEM_ARG(ASTROLABE_ESSENCE, pile_num_max)
	CASE_GET_ITEM_ARG(ASTROLABE_RANDOM_ADDON_ESSENCE, pile_num_max)
	CASE_GET_ITEM_ARG(ASTROLABE_INC_INNER_POINT_VALUE_ESSENCE, pile_num_max)
	CASE_GET_ITEM_ARG(ASTROLABE_INC_EXP_ESSENCE, pile_num_max)
	CASE_GET_ITEM_ARG(ITEM_PACKAGE_BY_PROFESSION_ESSENCE, pile_num_max)

	default:
		return 0;
	}
	return 0;
}


int itemdataman::get_item_repair_fee(unsigned int id)
{
	DATA_TYPE datatype;
	const void * dataptr = get_data_ptr(id, ID_SPACE_ESSENCE, datatype);
	if(dataptr == NULL || datatype == DT_INVALID)	return 0;
	
	switch(datatype) {
	case DT_WEAPON_ESSENCE:
		return ((WEAPON_ESSENCE*)dataptr)->repairfee;

	case DT_ARMOR_ESSENCE:
		return ((ARMOR_ESSENCE*)dataptr)->repairfee;

	case DT_DECORATION_ESSENCE:
		return ((DECORATION_ESSENCE*)dataptr)->repairfee;
	default:
		return 0;
	}
	return 0;
}

int itemdataman::generate_addon(unsigned int item_id, unsigned int addon_id, addon_data & data)
{
	DATA_TYPE datatype;
	const void * ess = get_data_ptr(item_id, ID_SPACE_ESSENCE, datatype);
	switch(datatype)
	{
	case DT_WEAPON_ESSENCE:
		return generate_equipment_addon(datatype,(char *)&(((WEAPON_ESSENCE*)ess)->addons[0]), addon_id, data);

	case DT_ARMOR_ESSENCE:
		return generate_equipment_addon(datatype,(char *)&(((ARMOR_ESSENCE*)ess)->addons[0]), addon_id, data);

	case DT_DECORATION_ESSENCE:
		return generate_equipment_addon(datatype,(char *)&(((DECORATION_ESSENCE*)ess)->addons[0]), addon_id, data);

	default:
		data.id = 0;
		return -1;
	}	
	return -1;
}

int itemdataman::generate_addon_from_rands(unsigned int item_id, unsigned int addon_id, addon_data & data)
{
	DATA_TYPE datatype;
	const void * ess = get_data_ptr(item_id, ID_SPACE_ESSENCE, datatype);
	switch(datatype)
	{
	case DT_WEAPON_ESSENCE:
		return generate_equipment_addon(datatype,(char *)&(((WEAPON_ESSENCE*)ess)->rands[0]), addon_id, data);

	case DT_ARMOR_ESSENCE:
		return generate_equipment_addon(datatype,(char *)&(((ARMOR_ESSENCE*)ess)->rands[0]), addon_id, data);

	case DT_DECORATION_ESSENCE:
		return generate_equipment_addon(datatype,(char *)&(((DECORATION_ESSENCE*)ess)->rands[0]), addon_id, data);

	default:
		data.id = 0;
		return -1;
	}	
	return -1;
}


int	itemdataman::generate_equipment_addon(DATA_TYPE datatype,char * header, unsigned int addon_id, addon_data & data)
{

	unsigned int id = addon_id;

	{
		float * prob_header =(float*) (header + sizeof(unsigned int));
		for(size_t i=0; i<4 && id == addon_id; i++)
		{
			int addon_index = abase::RandSelect(prob_header, sizeof(unsigned int)+sizeof(float), 32);
			id	= *(int*)(header + addon_index*(sizeof(unsigned int)+sizeof(float)));
		}
		if(id == addon_id)
		{
			data.id = 0;
			return -1;
		}
	}
	int rparanum;		
	if((rparanum = generate_addon(datatype,id,data)) < 0) return -1;
	data.id = (id & (~(0x3<<13))) | (rparanum<<13);
	return 0;
}

int itemdataman::generate_astrolabe_addonlist(const char * candidate_header, size_t candidate_num, addon_data* addon_list,size_t addon_num, unsigned int default_addon_id)
{
	if(!addon_num) return 0;

	size_t anum = 0;
	// candidate_header 对应 ASTROLABE_ESSENCE::rands
	size_t candidate_step = sizeof(unsigned int)+sizeof(float)+sizeof(int);
	size_t limit_offset = sizeof(unsigned int)+sizeof(float);
	size_t prob_offset = sizeof(unsigned int);
	abase::hash_map<unsigned int,int> hmap;

	for(int i = 0; i < 100 && anum < addon_num; ++i)
	{
		float * prob_header =(float*) (candidate_header + prob_offset);
		int addon_index = abase::RandSelect(prob_header, candidate_step, candidate_num);
		unsigned int id = *(unsigned int*)(candidate_header + addon_index*candidate_step);
		int limit = *(int*)(candidate_header+ limit_offset + addon_index*candidate_step); 
		if(id <= 0 || hmap[id] >= limit) continue;
	
		addon_data newaddon;
		if(!generate_addon(id,newaddon)) continue;

		addon_list[anum] = newaddon;		
		hmap[id] += 1;	
		++anum;
	}

	if(anum < addon_num)
	{
		addon_data newaddon;
		if(generate_addon(default_addon_id,newaddon))
		{
			for(;anum < addon_num;++anum)
			{
				addon_list[anum] = newaddon;
			}
		}
	}

	return anum;
}


bool itemdataman::generate_mine_from_monster(unsigned int id, int & mine_id, int & remain_time)
{
	DATA_TYPE datatype;
	const MONSTER_ESSENCE * mon = (const MONSTER_ESSENCE*) get_data_ptr(id, ID_SPACE_ESSENCE, datatype);
	if(mon == NULL || datatype != DT_MONSTER_ESSENCE)	
	{
		return false;
	}

	int index = abase::RandSelect(&(mon->drop_mines[0].probability), sizeof(unsigned int)+sizeof(float)+sizeof(int), 4); 
	mine_id = mon->drop_mines[index].id;
	remain_time = mon->drop_mines[index].life;

	return true;
}

int itemdataman::get_monster_drop_times(unsigned int id)
{
	DATA_TYPE datatype;
	 const MONSTER_ESSENCE * mon = (const MONSTER_ESSENCE*) get_data_ptr(id, ID_SPACE_ESSENCE, datatype);
	if(mon == NULL || datatype != DT_MONSTER_ESSENCE)	
	{
		return 0;
	}
	return mon->drop_times;
}

void itemdataman::get_monster_drop_money(unsigned int id,int & low, int &high)
{
	DATA_TYPE datatype;
	 const MONSTER_ESSENCE * mon = (const MONSTER_ESSENCE*) get_data_ptr(id, ID_SPACE_ESSENCE, datatype);
	if(mon == NULL || datatype != DT_MONSTER_ESSENCE)	
	{
		low = 0;
		high = 0;
		return;
	}
	low = mon->money_average-mon->money_var;
	high = mon->money_average+mon->money_var;
	return;
}

int itemdataman::generate_item_from_monster(unsigned int id, int * list,  size_t list_size,int * money)
{
	DATA_TYPE datatype;
	 const MONSTER_ESSENCE * mon = (const MONSTER_ESSENCE*) get_data_ptr(id, ID_SPACE_ESSENCE, datatype);
	if(mon == NULL || datatype != DT_MONSTER_ESSENCE)	
	{
		return -1;
	}

	size_t mcount = mon->drop_times;

	size_t drop_count = 0;
	for(size_t j = 0; j < mcount && drop_count < list_size; j ++)
	{

		unsigned int drop_num = abase::RandSelect(&(mon->probability_drop_num0),sizeof(float), 4);
		for(size_t i=0; i<drop_num; i++)
		{
			int index = abase::RandSelect(&(mon->drop_matters[0].probability), sizeof(unsigned int)+sizeof(float), 32); 
			if(j && index >= 16) continue;	//如果是多次丢出则只有一次能生成后面的物品
			list[drop_count] = mon->drop_matters[index].id;
			drop_count ++;
			if(drop_count >= list_size) break;
		}
	}

	*money = abase::RandNormal(mon->money_average-mon->money_var, mon->money_average+mon->money_var);
	return drop_count;
}

item_data * itemdataman::generate_item_for_shop(unsigned int id,const void * tag, size_t tag_size)
{
	item_data * item = NULL;
	size_t size;
	if(generate_item(id, &item, size,element_data::NORMAL(0),element_data::ADDON_LIST_SHOP,tag,tag_size)) return NULL;
	ASSERT(size == sizeof(item_data) + item->content_length);			
	return item;
}

item_data * itemdataman::generate_item_for_drop(unsigned int id,const void * tag, size_t tag_size)
{
	item_data * item = NULL;
	size_t size;
	if(generate_item(id, &item, size,element_data::NORMAL(0),element_data::ADDON_LIST_DROP,tag,tag_size)) return NULL;
	ASSERT(size == sizeof(item_data) + item->content_length);
	return item;
}

item_data * itemdataman::generate_item_from_player(unsigned int id,const void * tag, size_t tag_size)
{
	item_data * item = NULL;
	size_t size;
	if(generate_item(id, &item, size,element_data::NORMAL(0),element_data::ADDON_LIST_PRODUCE,tag,tag_size)) return NULL;
	ASSERT(size == sizeof(item_data) + item->content_length);			
	return item;
}

int
itemdataman::reset_classid(item_data * data)
{
	DATA_TYPE datatype = get_data_type(data->type, ID_SPACE_ESSENCE);	
	switch(datatype)
	{
	case DT_WEAPON_ESSENCE:
	case DT_ARMOR_ESSENCE:
	case DT_PROJECTILE_ESSENCE:
	case DT_QUIVER_ESSENCE:		
	case DT_DECORATION_ESSENCE:		
	case DT_MATERIAL_ESSENCE:
	case DT_DYE_TICKET_ESSENCE:
	case DT_DAMAGERUNE_ESSENCE:
	case DT_ARMORRUNE_ESSENCE:
	case DT_SKILLTOME_ESSENCE:
	case DT_FLYSWORD_ESSENCE:
	case DT_TOWNSCROLL_ESSENCE:
	case DT_UNIONSCROLL_ESSENCE:
	case DT_REVIVESCROLL_ESSENCE:
	case DT_ELEMENT_ESSENCE:
	case DT_TASKMATTER_ESSENCE:
	case DT_TOSSMATTER_ESSENCE:
	case DT_STONE_ESSENCE:
	case DT_WINGMANWING_ESSENCE:
	case DT_TASKDICE_ESSENCE:
	case DT_TASKNORMALMATTER_ESSENCE:
	case DT_FASHION_ESSENCE:
	case DT_FACEPILL_ESSENCE:
	case DT_FACETICKET_ESSENCE:
	case DT_GM_GENERATOR_ESSENCE:
	case DT_PET_EGG_ESSENCE:
	case DT_PET_FOOD_ESSENCE:
	case DT_FIREWORKS_ESSENCE:
	case DT_WAR_TANKCALLIN_ESSENCE:
	case DT_SKILLMATTER_ESSENCE:
	case DT_REFINE_TICKET_ESSENCE:
	case DT_DESTROYING_ESSENCE:
	case DT_BIBLE_ESSENCE:
	case DT_SPEAKER_ESSENCE:
	case DT_AUTOMP_ESSENCE:
	case DT_AUTOHP_ESSENCE:
	case DT_DOUBLE_EXP_ESSENCE:
	case DT_TRANSMITSCROLL_ESSENCE:
	case DT_GOBLIN_ESSENCE:  //lgc
	case DT_GOBLIN_EQUIP_ESSENCE:
	case DT_GOBLIN_EXPPILL_ESSENCE:
	case DT_SELL_CERTIFICATE_ESSENCE:
	case DT_TARGET_ITEM_ESSENCE:
	case DT_LOOK_INFO_ESSENCE:
	case DT_INC_SKILL_ABILITY_ESSENCE:
	case DT_WEDDING_BOOKCARD_ESSENCE:
	case DT_WEDDING_INVITECARD_ESSENCE:
	case DT_SHARPENER_ESSENCE:
	case DT_FACTION_MATERIAL_ESSENCE:
	case DT_CONGREGATE_ESSENCE:
	case DT_FORCE_TOKEN_ESSENCE:
	case DT_DYNSKILLEQUIP_ESSENCE:
	case DT_MONEY_CONVERTIBLE_ESSENCE:
	case DT_MONSTER_SPIRIT_ESSENCE:
	case DT_POKER_ESSENCE:
	case DT_POKER_DICE_ESSENCE:
	case DT_SHOP_TOKEN_ESSENCE:
	case DT_UNIVERSAL_TOKEN_ESSENCE:
	case DT_ASTROLABE_ESSENCE:
	case DT_ITEM_PACKAGE_BY_PROFESSION_ESSENCE:
	case DT_FIREWORKS2_ESSENCE:
	case DT_FIX_POSITION_TRANSMIT_ESSENCE:
		set_to_classid(datatype, data, -1);
		break;
	case DT_MEDICINE_ESSENCE:
		{
			DATA_TYPE dt;
			MEDICINE_ESSENCE * ess = (MEDICINE_ESSENCE *)get_data_ptr(data->type, ID_SPACE_ESSENCE, dt);
			if(ess == NULL || dt != DT_MEDICINE_ESSENCE)	return -1;
			set_to_classid(DT_MEDICINE_ESSENCE, data, ess->id_major_type);
		}
		break;
		
	case DT_MONSTER_ESSENCE:
	case DT_NPC_ESSENCE:
	default:
		return -1;
	}
	return 0;
}



int itemdataman::generate_item_for_sell(bool disable_bind2)
{
#define CASE_CLEAR_PROC_TYPE(ESSENCE)  case DT_##ESSENCE: \
	{\
	DATA_TYPE dt2;\
	ESSENCE * ess = (ESSENCE*)get_data_ptr(id, ID_SPACE_ESSENCE, dt2); \
	if(dt2 == datatype && ess && disable_bind2) ess->proc_type &= ~(0x0040);\
	}

	char* item;
	size_t size;
	int ret;

	DATA_TYPE datatype;
	unsigned int id = _edm->get_first_data_id(ID_SPACE_ESSENCE, datatype);
	element_data::item_tag_t tag = {element_data::IMT_SHOP,0};
	for(;id != 0 && datatype != DT_INVALID;id=_edm->get_next_data_id(ID_SPACE_ESSENCE, datatype))
	{

		switch(datatype)
		{
		CASE_CLEAR_PROC_TYPE(WEAPON_ESSENCE)
			ret = generate_weapon(id, ID_SPACE_ESSENCE, (char **)&item, size, element_data::SPECIFIC(0),element_data::ADDON_LIST_SHOP,&tag,sizeof(tag));
			break;

		CASE_CLEAR_PROC_TYPE(ARMOR_ESSENCE)
			ret = generate_armor(id, ID_SPACE_ESSENCE, (char **)&item, size, element_data::SPECIFIC(0),element_data::ADDON_LIST_SHOP,&tag,sizeof(tag));
			break;

		CASE_CLEAR_PROC_TYPE(PROJECTILE_ESSENCE)
		ret = generate_projectile(id, ID_SPACE_ESSENCE, (char **)&item, size, element_data::SPECIFIC(0));
		break;
		
		case DT_QUIVER_ESSENCE:
		ret = generate_quiver_for_sell(id, ID_SPACE_ESSENCE, (char **)&item, size, element_data::SPECIFIC(0));
		break;

		CASE_CLEAR_PROC_TYPE(STONE_ESSENCE)
		ret = generate_stone(id, ID_SPACE_ESSENCE, (char **)&item, size, element_data::SPECIFIC(0));
		break;		

		CASE_CLEAR_PROC_TYPE(DECORATION_ESSENCE)
		ret = generate_decoration(id, ID_SPACE_ESSENCE, (char **)&item, size, element_data::SPECIFIC(0),element_data::ADDON_LIST_SHOP,&tag,sizeof(tag));
		break;

		CASE_CLEAR_PROC_TYPE(MEDICINE_ESSENCE)
		ret = generate_medicine(id, ID_SPACE_ESSENCE, (char **)&item, size, element_data::SPECIFIC(0));
		break;

		CASE_CLEAR_PROC_TYPE(WINGMANWING_ESSENCE)
		ret = generate_wingmanwing(id, ID_SPACE_ESSENCE, (char **)&item, size, element_data::SPECIFIC(0),&tag,sizeof(tag));
		break;

		CASE_CLEAR_PROC_TYPE(MATERIAL_ESSENCE)
		ret = generate_material(id, ID_SPACE_ESSENCE, (char **)&item, size,(MATERIAL_ESSENCE*)0,DT_MATERIAL_ESSENCE);
		break;

		CASE_CLEAR_PROC_TYPE(DYE_TICKET_ESSENCE)
		ret = generate_material(id, ID_SPACE_ESSENCE, (char **)&item, size,(DYE_TICKET_ESSENCE*)0,DT_DYE_TICKET_ESSENCE);
		break;

		CASE_CLEAR_PROC_TYPE(ASTROLABE_RANDOM_ADDON_ESSENCE)
		ret =  generate_material(id, ID_SPACE_ESSENCE, (char **)&item, size,(ASTROLABE_RANDOM_ADDON_ESSENCE*)0,DT_ASTROLABE_RANDOM_ADDON_ESSENCE);
		break;

		CASE_CLEAR_PROC_TYPE(ASTROLABE_INC_INNER_POINT_VALUE_ESSENCE)
		ret =  generate_material(id, ID_SPACE_ESSENCE, (char **)&item, size,(ASTROLABE_INC_INNER_POINT_VALUE_ESSENCE*)0,DT_ASTROLABE_INC_INNER_POINT_VALUE_ESSENCE);
		break;

		CASE_CLEAR_PROC_TYPE(ASTROLABE_INC_EXP_ESSENCE)
		ret =  generate_material(id, ID_SPACE_ESSENCE, (char **)&item, size,(ASTROLABE_INC_EXP_ESSENCE*)0,DT_ASTROLABE_INC_EXP_ESSENCE);
		break;
		
		CASE_CLEAR_PROC_TYPE(FIREWORKS_ESSENCE)
		ret = generate_fireworks(id, ID_SPACE_ESSENCE, (char **)&item, size, element_data::SPECIFIC(0));
		break;

		CASE_CLEAR_PROC_TYPE(WAR_TANKCALLIN_ESSENCE)
		ret = generate_tankcallin(id, ID_SPACE_ESSENCE, (char **)&item, size, element_data::SPECIFIC(0));
		break;

		CASE_CLEAR_PROC_TYPE(SKILLMATTER_ESSENCE)
		ret = generate_skillmatter(id, ID_SPACE_ESSENCE, (char **)&item, size, element_data::SPECIFIC(0));
		break;
		
		CASE_CLEAR_PROC_TYPE(DAMAGERUNE_ESSENCE)
		ret = generate_damagerune(id, ID_SPACE_ESSENCE, (char **)&item, size, element_data::SPECIFIC(0));
		break;

		CASE_CLEAR_PROC_TYPE(ARMORRUNE_ESSENCE)
		ret = generate_armorrune(id, ID_SPACE_ESSENCE, (char **)&item, size, element_data::SPECIFIC(0));
		break;

		CASE_CLEAR_PROC_TYPE(SKILLTOME_ESSENCE)
		ret = generate_skilltome(id, ID_SPACE_ESSENCE, (char **)&item, size, element_data::SPECIFIC(0));
		break;

		CASE_CLEAR_PROC_TYPE(FLYSWORD_ESSENCE)
		ret = generate_flysword(id, ID_SPACE_ESSENCE, (char **)&item, size, element_data::SPECIFIC(0),&tag,sizeof(tag));
		break;

		CASE_CLEAR_PROC_TYPE(TOWNSCROLL_ESSENCE)
		ret = generate_townscroll(id, ID_SPACE_ESSENCE, (char **)&item, size, element_data::SPECIFIC(0));
		break;

		CASE_CLEAR_PROC_TYPE(UNIONSCROLL_ESSENCE)
		ret = generate_unionscroll(id, ID_SPACE_ESSENCE, (char **)&item, size, element_data::SPECIFIC(0));
		break;
		
		CASE_CLEAR_PROC_TYPE(REVIVESCROLL_ESSENCE)
		ret = generate_revivescroll(id, ID_SPACE_ESSENCE, (char **)&item, size, element_data::SPECIFIC(0));
		break;

		CASE_CLEAR_PROC_TYPE(ELEMENT_ESSENCE)
		ret = generate_element(id, ID_SPACE_ESSENCE, (char **)&item, size, element_data::SPECIFIC(0));
		break;

		CASE_CLEAR_PROC_TYPE(TASKMATTER_ESSENCE)
		ret = generate_taskmatter(id, ID_SPACE_ESSENCE, (char **)&item, size, element_data::SPECIFIC(0));
		break;

		CASE_CLEAR_PROC_TYPE(TOSSMATTER_ESSENCE)
		ret = generate_tossmatter(id, ID_SPACE_ESSENCE, (char **)&item, size, element_data::SPECIFIC(0));
		break;

		CASE_CLEAR_PROC_TYPE(TASKDICE_ESSENCE)
		ret = generate_taskdice(id, ID_SPACE_ESSENCE, (char **)&item, size, element_data::SPECIFIC(0));
		break;	
		
		CASE_CLEAR_PROC_TYPE(TASKNORMALMATTER_ESSENCE)
		ret = generate_tasknormalmatter(id, ID_SPACE_ESSENCE, (char **)&item, size, element_data::SPECIFIC(0));
		break;

		CASE_CLEAR_PROC_TYPE(FASHION_ESSENCE)
		ret = generate_fashion_item(id, ID_SPACE_ESSENCE, (char **)&item, size,element_data::SPECIFIC(0),&tag,sizeof(tag));
		break;
		
		CASE_CLEAR_PROC_TYPE(FACEPILL_ESSENCE)
		ret = generate_facepill(id, ID_SPACE_ESSENCE, (char **)&item, size,element_data::SPECIFIC(0));
		break;

		CASE_CLEAR_PROC_TYPE(FACETICKET_ESSENCE)
		ret = generate_faceticket(id, ID_SPACE_ESSENCE, (char **)&item, size,element_data::SPECIFIC(0));
		break;

		CASE_CLEAR_PROC_TYPE(GM_GENERATOR_ESSENCE)
		ret = generate_gm_generator(id,ID_SPACE_ESSENCE, (char **)&item, size,element_data::SPECIFIC(0));
		break;

		CASE_CLEAR_PROC_TYPE(PET_EGG_ESSENCE)
		ret = generate_pet_egg(id, ID_SPACE_ESSENCE, (char **)&item, size,element_data::SPECIFIC(0));
		break;

		CASE_CLEAR_PROC_TYPE(PET_FOOD_ESSENCE)
		ret = generate_pet_food(id, ID_SPACE_ESSENCE, (char **)&item, size,element_data::SPECIFIC(0));
		break;

		CASE_CLEAR_PROC_TYPE(REFINE_TICKET_ESSENCE)
		ret = generate_refine_ticket(id, ID_SPACE_ESSENCE, (char **)&item, size, element_data::SPECIFIC(0));
		break;

		CASE_CLEAR_PROC_TYPE(BIBLE_ESSENCE)
		ret = generate_bible(id, ID_SPACE_ESSENCE, (char **)&item, size, element_data::SPECIFIC(0));
		break;
		
		CASE_CLEAR_PROC_TYPE(SPEAKER_ESSENCE)
		ret = generate_speaker(id, ID_SPACE_ESSENCE, (char **)&item, size, element_data::SPECIFIC(0));
		break;
		
		CASE_CLEAR_PROC_TYPE(AUTOHP_ESSENCE)
		ret = generate_hp_amulet(id, ID_SPACE_ESSENCE, (char **)&item, size, element_data::SPECIFIC(0));
		break;
		
		CASE_CLEAR_PROC_TYPE(AUTOMP_ESSENCE)
		ret = generate_mp_amulet(id, ID_SPACE_ESSENCE, (char **)&item, size, element_data::SPECIFIC(0));
		break;

		CASE_CLEAR_PROC_TYPE(DOUBLE_EXP_ESSENCE)
		ret = generate_double_exp(id, ID_SPACE_ESSENCE, (char **)&item, size, element_data::SPECIFIC(0));
		break;

		CASE_CLEAR_PROC_TYPE(TRANSMITSCROLL_ESSENCE)
		ret = generate_transmitscroll(id, ID_SPACE_ESSENCE, (char **)&item, size, element_data::SPECIFIC(0));
		break;
	
		CASE_CLEAR_PROC_TYPE(GOBLIN_ESSENCE)
		ret = generate_elf(id, ID_SPACE_ESSENCE, (char **)&item, size);
		break;

		CASE_CLEAR_PROC_TYPE(GOBLIN_EQUIP_ESSENCE)
		ret = generate_elf_equip(id, ID_SPACE_ESSENCE, (char **)&item, size);
		break;

		CASE_CLEAR_PROC_TYPE(GOBLIN_EXPPILL_ESSENCE)	
		ret = generate_elf_exppill(id, ID_SPACE_ESSENCE, (char **)&item, size);
		break;

		CASE_CLEAR_PROC_TYPE(SELL_CERTIFICATE_ESSENCE)	
		ret = generate_stallcard(id, ID_SPACE_ESSENCE, (char **)&item, size);
		break;
		
		CASE_CLEAR_PROC_TYPE(TARGET_ITEM_ESSENCE)	
		ret = generate_skilltrigger2(id, ID_SPACE_ESSENCE, (char **)&item, size);
		break;
		
		CASE_CLEAR_PROC_TYPE(LOOK_INFO_ESSENCE)	
		ret = generate_queryotherproperty(id, ID_SPACE_ESSENCE, (char **)&item, size);
		break;
		
		CASE_CLEAR_PROC_TYPE(INC_SKILL_ABILITY_ESSENCE)	
		ret = generate_incskillability(id, ID_SPACE_ESSENCE, (char **)&item, size);
		break;
		
		CASE_CLEAR_PROC_TYPE(WEDDING_BOOKCARD_ESSENCE)	
		ret = generate_wedding_bookcard(id, ID_SPACE_ESSENCE, (char **)&item, size);
		break;
		
		CASE_CLEAR_PROC_TYPE(WEDDING_INVITECARD_ESSENCE)	
		ret = generate_wedding_invitecard(id, ID_SPACE_ESSENCE, (char **)&item, size);
		break;
		
		CASE_CLEAR_PROC_TYPE(SHARPENER_ESSENCE)	
		ret = generate_sharpener(id, ID_SPACE_ESSENCE, (char **)&item, size);
		break;
		
		CASE_CLEAR_PROC_TYPE(FACTION_MATERIAL_ESSENCE)	
		ret = generate_factionmaterial(id, ID_SPACE_ESSENCE, (char **)&item, size);
		break;
		
		CASE_CLEAR_PROC_TYPE(CONGREGATE_ESSENCE)	
		ret = generate_congregate(id, ID_SPACE_ESSENCE, (char **)&item, size);
		break;
		
		CASE_CLEAR_PROC_TYPE(FORCE_TOKEN_ESSENCE)	
		ret = generate_force_ticket(id, ID_SPACE_ESSENCE, (char **)&item, size);
		break;
		
		CASE_CLEAR_PROC_TYPE(DYNSKILLEQUIP_ESSENCE)	
		ret = generate_dynskillequip(id, ID_SPACE_ESSENCE, (char **)&item, size);
		break;
		
		CASE_CLEAR_PROC_TYPE(MONEY_CONVERTIBLE_ESSENCE)	
		ret = generate_moneyconvertibleitem(id, ID_SPACE_ESSENCE, (char **)&item, size);
		break;
		
		CASE_CLEAR_PROC_TYPE(MONSTER_SPIRIT_ESSENCE)
		ret = generate_soul(id, ID_SPACE_ESSENCE, (char **)&item, size);
		break;
		
		CASE_CLEAR_PROC_TYPE(POKER_ESSENCE)	
		ret = generate_generalcard(id, ID_SPACE_ESSENCE, (char **)&item, size, element_data::SPECIFIC(0));
		break;
		
		CASE_CLEAR_PROC_TYPE(POKER_DICE_ESSENCE)	
		ret = generate_generalcard_dice(id, ID_SPACE_ESSENCE, (char **)&item, size);
		break;
		
		CASE_CLEAR_PROC_TYPE(SHOP_TOKEN_ESSENCE)
		ret = generate_shoptoken(id, ID_SPACE_ESSENCE, (char **)&item, size);
		break;
		
		CASE_CLEAR_PROC_TYPE(UNIVERSAL_TOKEN_ESSENCE)	
		ret = generate_universal_token(id, ID_SPACE_ESSENCE, (char **)&item, size);
		break;
		
		CASE_CLEAR_PROC_TYPE(ASTROLABE_ESSENCE)
		ret = generate_astrolabe(id, ID_SPACE_ESSENCE, (char **)&item, size);
		break;

		CASE_CLEAR_PROC_TYPE(ITEM_PACKAGE_BY_PROFESSION_ESSENCE)
		ret = generate_occup_package(id, ID_SPACE_ESSENCE, (char **)&item, size);
		break;

		CASE_CLEAR_PROC_TYPE(FIREWORKS2_ESSENCE)
		ret = generate_fireworks2(id, ID_SPACE_ESSENCE, (char **)&item, size, element_data::SPECIFIC(0));
		break;
		
		CASE_CLEAR_PROC_TYPE(FIX_POSITION_TRANSMIT_ESSENCE)
		ret = generate_fixpositiontransmit(id, ID_SPACE_ESSENCE, (char **)&item, size, element_data::SPECIFIC(0));
		break;
		
		case DT_MONSTER_ESSENCE:
		case DT_NPC_ESSENCE:
		default:
			continue;
		}

		if(ret==0 && size != 0)		
		{
			sale_item_ptr_array.push_back(item);
			sale_item_size_array.push_back(size);
			LOCATION loc;
			loc.type = datatype;
			loc.pos = sale_item_ptr_array.size()-1;
			sale_item_id_index_map[id] = loc;
		}
	}
	return 0;
}

int itemdataman::duplicate_static_item(unsigned int id, char ** list,  size_t& size)
{
	DATA_TYPE datatype;
	size_t pos;
	IDToLOCATIONMap::iterator itr;
	itr = sale_item_id_index_map.find(id);

	if(itr != sale_item_id_index_map.end())
	{
		pos = (itr->second).pos;
		datatype = (itr->second).type;
		void * data_static = sale_item_ptr_array[pos];
		size = sale_item_size_array[pos];
		
		*list = (char*) abase::fastalloc(size);
		memcpy(*list, data_static, size);
		item_data * pItem = (item_data*) *list;
		if(pItem->guid.guid1)
		{
			int g1,g2;
			get_item_guid(id,g1,g2);

			pItem->guid.guid1 = g1;
			pItem->guid.guid2 = g2;
		}
		((item_data*)(*list))->item_content = (*list) + sizeof(item_data);
		return 0;
	}
	else
		return -1;	
}


const void * itemdataman::get_item_for_sell(unsigned int id)
{
	size_t pos;
	IDToLOCATIONMap::iterator itr;
	itr = sale_item_id_index_map.find(id);

	if(itr != sale_item_id_index_map.end())
	{
		pos = (itr->second).pos;
		return sale_item_ptr_array[pos];
	}
	else
		return NULL;
}

item_data * 
itemdataman::generate_equipment(unsigned int id, float rlist[32], int ridxlist[32], int addon[ELEMENTDATAMAN_MAX_NUM_ADDONS])
{
#ifdef LINUX
	int ret = -1;
	item_data * item = NULL;
	size_t size;
	DATA_TYPE datatype = get_data_type(id, ID_SPACE_ESSENCE);
	element_data::SpecRand sr;
	sr.IndexList = ridxlist;
	sr.IdxCap = 32;
	sr.RandList = rlist;
	sr.RandCap = 32;
	element_data::SpecCls cls(&sr);

	element_data::item_tag_t tag = {element_data::IMT_CREATE,0};

	switch(datatype)
	{
	case DT_WEAPON_ESSENCE:
		ret = generate_weapon(id, ID_SPACE_ESSENCE, (char **)&item, size,cls,element_data::ADDON_LIST_SPEC,&tag,sizeof(tag),addon);
		break;

	case DT_ARMOR_ESSENCE:
		ret = generate_armor(id, ID_SPACE_ESSENCE, (char **)&item, size, cls,element_data::ADDON_LIST_SPEC,&tag,sizeof(tag),addon);
		break;

	case DT_DECORATION_ESSENCE:
		ret = generate_decoration(id, ID_SPACE_ESSENCE, (char **)&item, size, cls,element_data::ADDON_LIST_SPEC,&tag,sizeof(tag),addon);
		break;
	default: 
		return NULL;
	}
	if(!ret) return item;
	return NULL;	
#else
	return NULL;
#endif
}

int itemdataman::get_cool_time(unsigned int id)
{
	DATA_TYPE datatype;
	const void * dataptr = get_data_ptr(id, ID_SPACE_ESSENCE, datatype);
	if(dataptr == NULL || datatype == DT_INVALID)	return 0;
	
	switch(datatype) {
	case DT_WEAPON_ESSENCE:
		return 0;

	case DT_ARMOR_ESSENCE:
		return 0;

	case DT_PROJECTILE_ESSENCE:
		return 0;

	case DT_DECORATION_ESSENCE:
		return 0;

	case DT_STONE_ESSENCE:
		return 0;
		
	case DT_MEDICINE_ESSENCE:
		return ((MEDICINE_ESSENCE*)dataptr)->cool_time;

	case DT_WINGMANWING_ESSENCE:
		return 0;

	case DT_MATERIAL_ESSENCE:
		return 0;

	case DT_DYE_TICKET_ESSENCE:
		return 0;
	
	case DT_DAMAGERUNE_ESSENCE:
		return 0;

	case DT_ARMORRUNE_ESSENCE:
		return 0;

	case DT_SKILLTOME_ESSENCE:
		return 0;

	case DT_FLYSWORD_ESSENCE:
		return 0;

	case DT_TOWNSCROLL_ESSENCE:
		return 0;

	case DT_UNIONSCROLL_ESSENCE:
		return 0;

	case DT_REVIVESCROLL_ESSENCE:
		return ((REVIVESCROLL_ESSENCE*)dataptr)->cool_time;

	case DT_ELEMENT_ESSENCE:
		return 0;

	case DT_TOSSMATTER_ESSENCE:
		return 0;

	case DT_TASKNORMALMATTER_ESSENCE:
		return 0;
	
	case DT_FASHION_ESSENCE:
		return 0;

	case DT_FACEPILL_ESSENCE:
//		return ((FACEPILL_ESSENCE*)dataptr)->cool_time;
		return 0;

	case DT_FACETICKET_ESSENCE:
//		return ((FACETICKET_ESSENCE*)dataptr)->cool_time;		
		return 0;

	case DT_PET_EGG_ESSENCE:
		return 0;

	case DT_PET_FOOD_ESSENCE:
		return 150;		//食物的冷却时间为2.5分钟,这里返回值没有使用

	case DT_FIREWORKS_ESSENCE:
		return 0;

	case DT_WAR_TANKCALLIN_ESSENCE:
		return 0;

	case DT_SKILLMATTER_ESSENCE:
		return 0;

	case DT_AUTOMP_ESSENCE:
		return ((AUTOMP_ESSENCE*)dataptr)->cool_time;
		
	case DT_AUTOHP_ESSENCE:
		return ((AUTOHP_ESSENCE*)dataptr)->cool_time;

	case DT_DOUBLE_EXP_ESSENCE:
		return 0;

	case DT_TRANSMITSCROLL_ESSENCE:
		return 0;

	case DT_FIREWORKS2_ESSENCE:
		return 0;

	default:
		return 0;
	}
	return 0;
}

int itemdataman::get_item_damaged_drop(unsigned int id,unsigned int &damaged_drop)
{
	DATA_TYPE datatype;
	const void * dataptr = get_data_ptr(id, ID_SPACE_ESSENCE, datatype);
	if(dataptr == NULL || datatype == DT_INVALID) return -1;

	switch(datatype)
	{
	case DT_WEAPON_ESSENCE:
		damaged_drop = ((WEAPON_ESSENCE*)dataptr)->id_drop_after_damaged;
		return ((WEAPON_ESSENCE*)dataptr)->num_drop_after_damaged;
		
	case DT_ARMOR_ESSENCE:
		damaged_drop = ((ARMOR_ESSENCE*)dataptr)->id_drop_after_damaged;
		return ((ARMOR_ESSENCE*)dataptr)->num_drop_after_damaged;
		
	case DT_DECORATION_ESSENCE:
		damaged_drop = ((DECORATION_ESSENCE*)dataptr)->id_drop_after_damaged;
		return ((DECORATION_ESSENCE*)dataptr)->num_drop_after_damaged;
		
	case DT_FLYSWORD_ESSENCE:
		damaged_drop = ((FLYSWORD_ESSENCE*)dataptr)->id_drop_after_damaged;
		return ((FLYSWORD_ESSENCE*)dataptr)->num_drop_after_damaged;
		
	case DT_BIBLE_ESSENCE:
		damaged_drop = ((BIBLE_ESSENCE*)dataptr)->id_drop_after_damaged;
		return ((BIBLE_ESSENCE*)dataptr)->num_drop_after_damaged;
	
	default:
		return -1;
	}
	return -1;
}

int itemdataman::get_item_class_limit(unsigned int id)
{
	DATA_TYPE datatype;
	const void * dataptr = get_data_ptr(id,ID_SPACE_ESSENCE,datatype);
	if(dataptr == NULL || datatype == DT_INVALID) return 0;
	
	switch(datatype)
	{
	case DT_WEAPON_ESSENCE:
		return ((WEAPON_ESSENCE*)dataptr)->character_combo_id;	
		
	case DT_ARMOR_ESSENCE:
		return ((ARMOR_ESSENCE*)dataptr)->character_combo_id;
	
	case DT_DECORATION_ESSENCE:
		return ((DECORATION_ESSENCE*)dataptr)->character_combo_id;
	
	case DT_FLYSWORD_ESSENCE:
		return ((FLYSWORD_ESSENCE*)dataptr)->character_combo_id;
	
	case DT_FACEPILL_ESSENCE:
		return ((FACEPILL_ESSENCE*)dataptr)->character_combo_id;
	
	case DT_PROJECTILE_ESSENCE:
		return 0xFFFF;

	case DT_PET_ESSENCE:
		return ((PET_ESSENCE*)dataptr)->character_combo_id;

	case DT_FASHION_ESSENCE:
		return ((FASHION_ESSENCE*)dataptr)->character_combo_id;

	CASE_GET_ITEM_ARG(ASTROLABE_ESSENCE, character_combo_id)

	default:
		return 0;
	}
	
	return 0;
}

int itemdataman::get_item_reputation_limit(unsigned id)
{
	DATA_TYPE datatype;
	const void * dataptr = get_data_ptr(id,ID_SPACE_ESSENCE,datatype);
	if(dataptr == NULL || datatype == DT_INVALID) return 0;
	
	switch(datatype)
	{
	case DT_WEAPON_ESSENCE:
		return ((WEAPON_ESSENCE*)dataptr)->require_reputation;	
		
	case DT_ARMOR_ESSENCE:
		return ((ARMOR_ESSENCE*)dataptr)->require_reputation;
	
	case DT_DECORATION_ESSENCE:
		return ((DECORATION_ESSENCE*)dataptr)->require_reputation;
	
	default:
		return 0;
	}

	return 0;	
}

int itemdataman::get_item_level(unsigned id)
{
	DATA_TYPE datatype;
	const void * dataptr = get_data_ptr(id,ID_SPACE_ESSENCE,datatype);
	if(dataptr == NULL || datatype == DT_INVALID) return 0;
	
	switch(datatype)
	{
	case DT_WEAPON_ESSENCE:
		return ((WEAPON_ESSENCE*)dataptr)->level;	
		
	case DT_ARMOR_ESSENCE:
		return ((ARMOR_ESSENCE*)dataptr)->level;
	
	case DT_DECORATION_ESSENCE:
		return ((DECORATION_ESSENCE*)dataptr)->level;
		
	case DT_FLYSWORD_ESSENCE:
		return ((FLYSWORD_ESSENCE*)dataptr)->level;
		
	case DT_ELEMENT_ESSENCE:
		return ((ELEMENT_ESSENCE*)dataptr)->level;
		
	case DT_STONE_ESSENCE:
		return ((STONE_ESSENCE*)dataptr)->level;
		
	case DT_SHARPENER_ESSENCE:
		return ((SHARPENER_ESSENCE*)dataptr)->level;
	
	default:
		return 0;
	}

	return 0;	
}

unsigned char * itemdataman::get_item_name(unsigned id,int &name_len)
{
#define CASE_GET_ITEM_NAME(ESSENCE)  case DT_##ESSENCE: \
	{\
		name_len = get_name_length(((ESSENCE*)dataptr)->name); \
		return (unsigned char *)(((ESSENCE*)dataptr)->name); \
	}break;

	DATA_TYPE datatype;
	const void * dataptr = get_data_ptr(id,ID_SPACE_ESSENCE,datatype);
	if(dataptr == NULL || datatype == DT_INVALID)
		return 0;
	switch(datatype)
	{
		case DT_WEAPON_ESSENCE:
			name_len = get_name_length(((WEAPON_ESSENCE*)dataptr)->name);
			return (unsigned char *)(((WEAPON_ESSENCE*)dataptr)->name);
		case DT_ARMOR_ESSENCE:
			name_len = get_name_length(((ARMOR_ESSENCE*)dataptr)->name);
			return (unsigned char *)(((ARMOR_ESSENCE*)dataptr)->name);
		case DT_PROJECTILE_ESSENCE:
			name_len = get_name_length(((PROJECTILE_ESSENCE*)dataptr)->name);
			return (unsigned char *)(((PROJECTILE_ESSENCE*)dataptr)->name);
		case DT_DECORATION_ESSENCE:
			name_len = get_name_length(((DECORATION_ESSENCE*)dataptr)->name);
			return (unsigned char *)(((DECORATION_ESSENCE*)dataptr)->name);
		case DT_STONE_ESSENCE:
			name_len = get_name_length(((STONE_ESSENCE*)dataptr)->name);
			return (unsigned char *)(((STONE_ESSENCE*)dataptr)->name);
		case DT_MEDICINE_ESSENCE:
			name_len = get_name_length(((MEDICINE_ESSENCE*)dataptr)->name);
			return (unsigned char *)(((MEDICINE_ESSENCE*)dataptr)->name);
		case DT_WINGMANWING_ESSENCE:
			name_len = get_name_length(((WINGMANWING_ESSENCE*)dataptr)->name);
			return (unsigned char *)(((WINGMANWING_ESSENCE*)dataptr)->name);
		case DT_MATERIAL_ESSENCE:
			name_len = get_name_length(((MATERIAL_ESSENCE*)dataptr)->name);
			return (unsigned char *)(((MATERIAL_ESSENCE*)dataptr)->name);
		case DT_DYE_TICKET_ESSENCE:
			name_len = get_name_length(((DYE_TICKET_ESSENCE*)dataptr)->name);
			return (unsigned char *)(((DYE_TICKET_ESSENCE*)dataptr)->name);
		case DT_DAMAGERUNE_ESSENCE:
			name_len = get_name_length(((DAMAGERUNE_ESSENCE*)dataptr)->name);
			return (unsigned char *)(((DAMAGERUNE_ESSENCE*)dataptr)->name);
		case DT_ARMORRUNE_ESSENCE:
			name_len = get_name_length(((ARMORRUNE_ESSENCE*)dataptr)->name);
			return (unsigned char *)(((ARMORRUNE_ESSENCE*)dataptr)->name);
		case DT_SKILLTOME_ESSENCE:
			name_len = get_name_length(((SKILLTOME_ESSENCE*)dataptr)->name);
			return (unsigned char *)(((SKILLTOME_ESSENCE*)dataptr)->name);
		case DT_FLYSWORD_ESSENCE:
			name_len = get_name_length(((FLYSWORD_ESSENCE*)dataptr)->name);
			return (unsigned char *)(((FLYSWORD_ESSENCE*)dataptr)->name);
		case DT_TOWNSCROLL_ESSENCE:
			name_len = get_name_length(((TOWNSCROLL_ESSENCE*)dataptr)->name);
			return (unsigned char *)(((TOWNSCROLL_ESSENCE*)dataptr)->name);
		case DT_UNIONSCROLL_ESSENCE:
			name_len = get_name_length(((UNIONSCROLL_ESSENCE*)dataptr)->name);
			return (unsigned char *)(((UNIONSCROLL_ESSENCE*)dataptr)->name);
		case DT_REVIVESCROLL_ESSENCE:
			name_len = get_name_length(((REVIVESCROLL_ESSENCE*)dataptr)->name);
			return (unsigned char *)(((REVIVESCROLL_ESSENCE*)dataptr)->name);
		case DT_ELEMENT_ESSENCE:
			name_len = get_name_length(((ELEMENT_ESSENCE*)dataptr)->name);
			return (unsigned char *)(((ELEMENT_ESSENCE*)dataptr)->name);
		case DT_TASKMATTER_ESSENCE:
			name_len = get_name_length(((TASKMATTER_ESSENCE*)dataptr)->name);
			return (unsigned char *)(((TASKMATTER_ESSENCE*)dataptr)->name);
		case DT_TASKDICE_ESSENCE:
			name_len = get_name_length(((TASKDICE_ESSENCE*)dataptr)->name);
			return (unsigned char *)(((TASKDICE_ESSENCE*)dataptr)->name);
		case DT_TOSSMATTER_ESSENCE: 
			name_len = get_name_length(((TOSSMATTER_ESSENCE*)dataptr)->name);
			return (unsigned char *)(((TOSSMATTER_ESSENCE*)dataptr)->name);
		case DT_TASKNORMALMATTER_ESSENCE:
			name_len = get_name_length(((TASKNORMALMATTER_ESSENCE*)dataptr)->name);
			return (unsigned char *)(((TASKNORMALMATTER_ESSENCE*)dataptr)->name);
		case DT_FASHION_ESSENCE:
			name_len = get_name_length(((FASHION_ESSENCE*)dataptr)->name);
			return (unsigned char *)(((FASHION_ESSENCE*)dataptr)->name);
		case DT_FACEPILL_ESSENCE:
			name_len = get_name_length(((FACEPILL_ESSENCE*)dataptr)->name);
			return (unsigned char *)(((FACEPILL_ESSENCE*)dataptr)->name);
		case DT_FACETICKET_ESSENCE:
			name_len = get_name_length(((FACETICKET_ESSENCE*)dataptr)->name);
			return (unsigned char *)(((FACETICKET_ESSENCE*)dataptr)->name);
		case DT_PET_EGG_ESSENCE:
			name_len = get_name_length(((PET_EGG_ESSENCE*)dataptr)->name);
			return (unsigned char *)(((PET_EGG_ESSENCE*)dataptr)->name);
		case DT_PET_FOOD_ESSENCE:
			name_len = get_name_length(((PET_FOOD_ESSENCE*)dataptr)->name);
			return (unsigned char *)(((PET_FOOD_ESSENCE*)dataptr)->name);
		case DT_FIREWORKS_ESSENCE:
			name_len = get_name_length(((FIREWORKS_ESSENCE*)dataptr)->name);
			return (unsigned char *)(((FIREWORKS_ESSENCE*)dataptr)->name);
		case DT_WAR_TANKCALLIN_ESSENCE:
			name_len = get_name_length(((WAR_TANKCALLIN_ESSENCE*)dataptr)->name);
			return (unsigned char *)(((WAR_TANKCALLIN_ESSENCE*)dataptr)->name);
		case DT_SKILLMATTER_ESSENCE:
			name_len = get_name_length(((SKILLMATTER_ESSENCE*)dataptr)->name);
			return (unsigned char *)(((SKILLMATTER_ESSENCE*)dataptr)->name);
		case DT_REFINE_TICKET_ESSENCE:
			name_len = get_name_length(((REFINE_TICKET_ESSENCE*)dataptr)->name);
			return (unsigned char *)(((REFINE_TICKET_ESSENCE*)dataptr)->name);
		case DT_BIBLE_ESSENCE:
			name_len = get_name_length(((BIBLE_ESSENCE*)dataptr)->name);
			return (unsigned char *)(((BIBLE_ESSENCE*)dataptr)->name);
		case DT_SPEAKER_ESSENCE:
			name_len = get_name_length(((SPEAKER_ESSENCE*)dataptr)->name);
			return (unsigned char *)(((SPEAKER_ESSENCE*)dataptr)->name);
		case DT_AUTOMP_ESSENCE:
			name_len = get_name_length(((AUTOMP_ESSENCE*)dataptr)->name);
			return (unsigned char *)(((AUTOMP_ESSENCE*)dataptr)->name);
		case DT_AUTOHP_ESSENCE:
			name_len = get_name_length(((AUTOHP_ESSENCE*)dataptr)->name);
			return (unsigned char *)(((AUTOHP_ESSENCE*)dataptr)->name);
		case DT_DESTROYING_ESSENCE:
			name_len = get_name_length(((DESTROYING_ESSENCE*)dataptr)->name);
			return (unsigned char *)(((DESTROYING_ESSENCE*)dataptr)->name);
		case DT_GM_GENERATOR_ESSENCE:
			name_len = get_name_length(((GM_GENERATOR_ESSENCE*)dataptr)->name);
			return (unsigned char *)(((GM_GENERATOR_ESSENCE*)dataptr)->name);
		case DT_DOUBLE_EXP_ESSENCE:
			name_len = get_name_length(((DOUBLE_EXP_ESSENCE*)dataptr)->name);
			return (unsigned char *)(((DOUBLE_EXP_ESSENCE*)dataptr)->name);
		case DT_TRANSMITSCROLL_ESSENCE:
			name_len = get_name_length(((TRANSMITSCROLL_ESSENCE*)dataptr)->name);
			return (unsigned char *)(((TRANSMITSCROLL_ESSENCE*)dataptr)->name);
		case DT_GOBLIN_ESSENCE:
			name_len = get_name_length(((GOBLIN_ESSENCE*)dataptr)->name);
			return (unsigned char *)(((GOBLIN_ESSENCE*)dataptr)->name);
		case DT_GOBLIN_EQUIP_ESSENCE:
			name_len = get_name_length(((GOBLIN_EQUIP_ESSENCE*)dataptr)->name);
			return (unsigned char *)(((GOBLIN_EQUIP_ESSENCE*)dataptr)->name);
		case DT_GOBLIN_EXPPILL_ESSENCE:
			name_len = get_name_length(((GOBLIN_EXPPILL_ESSENCE*)dataptr)->name);
			return (unsigned char *)(((GOBLIN_EXPPILL_ESSENCE*)dataptr)->name);
		case DT_SELL_CERTIFICATE_ESSENCE:
			name_len = get_name_length(((SELL_CERTIFICATE_ESSENCE*)dataptr)->name);
			return (unsigned char *)(((SELL_CERTIFICATE_ESSENCE*)dataptr)->name);
		case DT_TARGET_ITEM_ESSENCE:
			name_len = get_name_length(((TARGET_ITEM_ESSENCE*)dataptr)->name);
			return (unsigned char *)(((TARGET_ITEM_ESSENCE*)dataptr)->name);
		case DT_LOOK_INFO_ESSENCE:
			name_len = get_name_length(((LOOK_INFO_ESSENCE*)dataptr)->name);
			return (unsigned char *)(((LOOK_INFO_ESSENCE*)dataptr)->name);
		case DT_INC_SKILL_ABILITY_ESSENCE:
			name_len = get_name_length(((INC_SKILL_ABILITY_ESSENCE*)dataptr)->name);
			return (unsigned char *)(((INC_SKILL_ABILITY_ESSENCE*)dataptr)->name);
		case DT_WEDDING_BOOKCARD_ESSENCE:
			name_len = get_name_length(((WEDDING_BOOKCARD_ESSENCE*)dataptr)->name);
			return (unsigned char *)(((WEDDING_BOOKCARD_ESSENCE*)dataptr)->name);
		case DT_WEDDING_INVITECARD_ESSENCE:
			name_len = get_name_length(((WEDDING_INVITECARD_ESSENCE*)dataptr)->name);
			return (unsigned char *)(((WEDDING_INVITECARD_ESSENCE*)dataptr)->name);
		case DT_SHARPENER_ESSENCE:
			name_len = get_name_length(((SHARPENER_ESSENCE*)dataptr)->name);
			return (unsigned char *)(((SHARPENER_ESSENCE*)dataptr)->name);
		case DT_FACTION_MATERIAL_ESSENCE:
			name_len = get_name_length(((FACTION_MATERIAL_ESSENCE*)dataptr)->name);
			return (unsigned char *)(((FACTION_MATERIAL_ESSENCE*)dataptr)->name);
		case DT_CONGREGATE_ESSENCE:
			name_len = get_name_length(((CONGREGATE_ESSENCE*)dataptr)->name);
			return (unsigned char *)(((CONGREGATE_ESSENCE*)dataptr)->name);
		case DT_FORCE_TOKEN_ESSENCE:
			name_len = get_name_length(((FORCE_TOKEN_ESSENCE*)dataptr)->name);
			return (unsigned char *)(((FORCE_TOKEN_ESSENCE*)dataptr)->name);
		case DT_DYNSKILLEQUIP_ESSENCE:
			name_len = get_name_length(((DYNSKILLEQUIP_ESSENCE*)dataptr)->name);
			return (unsigned char *)(((DYNSKILLEQUIP_ESSENCE*)dataptr)->name);
		case DT_MONEY_CONVERTIBLE_ESSENCE:
			name_len = get_name_length(((MONEY_CONVERTIBLE_ESSENCE*)dataptr)->name);
			return (unsigned char *)(((MONEY_CONVERTIBLE_ESSENCE*)dataptr)->name);
		case DT_MONSTER_SPIRIT_ESSENCE:
			name_len = get_name_length(((MONSTER_SPIRIT_ESSENCE*)dataptr)->name);
			return (unsigned char *)(((MONSTER_SPIRIT_ESSENCE*)dataptr)->name);
		case DT_POKER_ESSENCE:
			name_len = get_name_length(((POKER_ESSENCE*)dataptr)->name);
			return (unsigned char *)(((POKER_ESSENCE*)dataptr)->name);
		case DT_POKER_DICE_ESSENCE:
			name_len = get_name_length(((POKER_DICE_ESSENCE*)dataptr)->name);
			return (unsigned char *)(((POKER_DICE_ESSENCE*)dataptr)->name);

		CASE_GET_ITEM_NAME(ASTROLABE_ESSENCE)	
		CASE_GET_ITEM_NAME(ASTROLABE_RANDOM_ADDON_ESSENCE)
		CASE_GET_ITEM_NAME(ASTROLABE_INC_INNER_POINT_VALUE_ESSENCE)
		CASE_GET_ITEM_NAME(ASTROLABE_INC_EXP_ESSENCE)
		CASE_GET_ITEM_NAME(ITEM_PACKAGE_BY_PROFESSION_ESSENCE)
			
		case DT_SHOP_TOKEN_ESSENCE:
			name_len = get_name_length(((SHOP_TOKEN_ESSENCE*)dataptr)->name);
			return (unsigned char *)(((SHOP_TOKEN_ESSENCE*)dataptr)->name);
		case DT_UNIVERSAL_TOKEN_ESSENCE:
			name_len = get_name_length(((UNIVERSAL_TOKEN_ESSENCE*)dataptr)->name);
			return (unsigned char *)(((UNIVERSAL_TOKEN_ESSENCE*)dataptr)->name);

		case DT_FIREWORKS2_ESSENCE:
			name_len = get_name_length(((FIREWORKS2_ESSENCE*)dataptr)->name);

		default:
			return NULL;
	}
	return NULL;
}

int itemdataman::get_name_length(unsigned short *name)
{
	int length;
	for(length = 0;length < 32;length++)
	{
		if(name[length] == 0)
		{
			break;
		}
	}
	return length*2;
}


