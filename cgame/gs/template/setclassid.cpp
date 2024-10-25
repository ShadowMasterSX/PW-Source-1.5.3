#include "../world.h"
#include "itemdataman.h"
#include "../clstab.h"
#include "../item/equip_item.h"
#include "../worldmanager.h"
#include <interlocked.h>
#include <timer.h>

static int guid_counter = 0;
extern abase::timer	g_timer;

void get_item_guid(int id, int & g1, int &g2)
{
	unsigned int index = (world_manager::GetWorldIndex() & 0xFF) << 24;
	unsigned int counter = interlocked_increment(&guid_counter) & 0xFFFFFF;
	int t = g_timer.get_systime();
	ASSERT(t);
	g1 = t;
	g2 = index | counter; 
//	printf("%d声称GUID%x,%x\n",id,g1,g2);
}

void set_to_classid(DATA_TYPE type, item_data * data, int major_type)
{
	if(data->type == 11208)
	{
		data->classid = 10000;
		return;
	}
	switch(type)
	{
		case 	DT_WEAPON_ESSENCE:
			{
				int weapon_type = weapon_item::GetWeaponType(data);
				if(weapon_type == weapon_essence::WEAPON_TYPE_RANGE)	
				{
					data->classid = CLS_ITEM_RANGE_WEAPON;
				}
				else
				{
					data->classid = CLS_ITEM_MELEE_WEAPON;
				}
			}
			break;
		case 	DT_DAMAGERUNE_ESSENCE:
			{
				data->classid = CLS_ITEM_DAMAGE_RUNE;
			}
			break;
		case 	DT_ARMORRUNE_ESSENCE:
			{
				int *rune_type = (int*)data->item_content;
				if(!rune_type)
				{
					printf("错误的优化符数据\n");
					data->classid = -1;
					break;
				}
				if(*rune_type)
				{
					data->classid = CLS_ITEM_RESISTANCE_RUNE;
				}
				else
				{
					data->classid = CLS_ITEM_DEFENSE_RUNE;
				}
			}
			break;

		case 	DT_ARMOR_ESSENCE:
			data->classid = CLS_ITEM_ARMOR;
			break;
		case 	DT_PROJECTILE_ESSENCE:
			data->classid = CLS_ITEM_PROJECTILE;
			break;

		case 	DT_DECORATION_ESSENCE:
			data->classid = CLS_ITEM_DECORATION;
			break;

		case 	DT_MEDICINE_ESSENCE:
			switch(major_type)
			{
				case 1794: //加血的
					data->classid = CLS_ITEM_HEALING_POTION;
					break;
				case 1802: //加魔
					data->classid = CLS_ITEM_MANA_POTION;
					break;
				case 1810: //加血和魔
					data->classid = CLS_ITEM_REJUVENATION_POTION;
					break;
				case 1815: //解毒
					data->classid = CLS_ITEM_HALF_ANTIDOTE;
					break;
				case 2038: //瞬间解毒
					data->classid = CLS_ITEM_FULL_ANTIDOTE;
					break;
				default:
				data->classid = -1;
			}
			break;
		case    DT_STONE_ESSENCE:
			data->classid = CLS_ITEM_STONE;
			break;
		case 	DT_ELEMENT_ESSENCE:
			data->classid = -1;
			break;
		case 	DT_FLYSWORD_ESSENCE:
			data->classid = CLS_ITEM_CLS_FLY_SWORD;
			break;
		case 	DT_WINGMANWING_ESSENCE:
			data->classid = CLS_ITEM_ANGEL_WING;
			break;
		case 	DT_TOWNSCROLL_ESSENCE:
			data->classid = CLS_ITEM_TOWNSCROLL;
			break;
		case 	DT_TOSSMATTER_ESSENCE:
			data->classid = CLS_ITEM_TOSSMATTER;
			break;

		case 	DT_TASKDICE_ESSENCE:
			data->classid = CLS_ITEM_TASKDICE;
			break;
		case 	DT_FASHION_ESSENCE:
			data->classid = CLS_ITEM_FASHION_ITEM;
			break;
		case 	DT_FACEPILL_ESSENCE:
			data->classid = CLS_ITEM_FACEPILL;
			break;

		case 	DT_GM_GENERATOR_ESSENCE:
			data->classid = CLS_ITEM_MOBGEN;
			break;
		
		case 	DT_PET_EGG_ESSENCE:
			data->classid = CLS_ITEM_PET_EGG;
			break;

		case 	DT_PET_FOOD_ESSENCE:
			data->classid = CLS_ITEM_PET_FOOD;
			break;
			
		case 	DT_WAR_TANKCALLIN_ESSENCE:
			data->classid = CLS_ITEM_CONTROLLER;
			break;

		case 	DT_FIREWORKS_ESSENCE:
			data->classid = CLS_ITEM_FIREWORKS;
			break;

		case 	DT_SKILLMATTER_ESSENCE:
			data->classid = CLS_ITEM_SKILL_TRIGGER;
			break;

		case 	DT_DESTROYING_ESSENCE:
			data->classid = CLS_ITEM_DUMMY;
			break;

		case DT_BIBLE_ESSENCE:
			data->classid = CLS_ITEM_BIBLE;
			break;
			
		case DT_SPEAKER_ESSENCE:
			data->classid = CLS_ITEM_BUGLE;
			break;

		case DT_AUTOMP_ESSENCE:
			data->classid = CLS_ITEM_MP_AMULET;
			break;

		case DT_AUTOHP_ESSENCE:
			data->classid = CLS_ITEM_HP_AMULET;
			break;
		case DT_DOUBLE_EXP_ESSENCE:
			data->classid = CLS_ITEM_DBL_EXP;
			break;

		case DT_TRANSMITSCROLL_ESSENCE:	
			data->classid = CLS_ITEM_TOWNSCROLL2;
			break;


		case 	DT_TASKMATTER_ESSENCE:
		case 	DT_SKILLTOME_ESSENCE:
		case 	DT_UNIONSCROLL_ESSENCE:
		case 	DT_REVIVESCROLL_ESSENCE:
			data->classid = -1;
			break;

		case DT_GOBLIN_ESSENCE:		//lgc
			data->classid = CLS_ITEM_ELF;
			break;

		case DT_GOBLIN_EQUIP_ESSENCE:		
			data->classid = CLS_ITEM_ELF_EQUIP;
			break;
		
		case DT_GOBLIN_EXPPILL_ESSENCE:
			data->classid = CLS_ITEM_ELF_EXPPILL;
			break;
	
		case DT_SELL_CERTIFICATE_ESSENCE:
			data->classid = CLS_ITEM_STALLCARD;
			break;
		
		case DT_TARGET_ITEM_ESSENCE:
			data->classid = CLS_ITEM_SKILLTRIGGER2;
			break;
			
		case DT_LOOK_INFO_ESSENCE:
			data->classid = CLS_ITEM_QUERYOTHERPROPERTY;
			break;
		
		case DT_INC_SKILL_ABILITY_ESSENCE:
			data->classid = CLS_ITEM_INCSKILLABILITY;
			break;
		
		case DT_WEDDING_BOOKCARD_ESSENCE:
			data->classid = CLS_ITEM_WEDDING_BOOKCARD; 
			break;
			
		case DT_WEDDING_INVITECARD_ESSENCE:
			data->classid = CLS_ITEM_WEDDING_INVITECARD; 
			break;
			
		case DT_SHARPENER_ESSENCE:
			data->classid = CLS_ITEM_SHARPENER;
			break;

		case DT_FACTION_MATERIAL_ESSENCE:
			data->classid = -1;
			break;

		case DT_CONGREGATE_ESSENCE:
			data->classid = CLS_ITEM_CONGREGATE;
			break;

		case DT_FORCE_TOKEN_ESSENCE:
			data->classid = CLS_ITEM_FORCE_TICKET;
			break;

		case DT_DYNSKILLEQUIP_ESSENCE:
			data->classid = CLS_ITEM_DYNSKILL;
			break;
			
		case DT_MONEY_CONVERTIBLE_ESSENCE:
			data->classid = -1;
			break;

		case DT_MONSTER_SPIRIT_ESSENCE:
			data->classid = CLS_ITEM_SOUL;
			break;

		case DT_POKER_ESSENCE:
			data->classid = CLS_ITEM_GENERALCARD;
			break;

		case DT_POKER_DICE_ESSENCE:
			data->classid = CLS_ITEM_GENERALCARD_DICE;
			break;

		case DT_SHOP_TOKEN_ESSENCE:
		case DT_UNIVERSAL_TOKEN_ESSENCE:
			data->classid = -1;
			break;

		default:
			//printf("未能确定的Datatype %d\n",type);
			data->classid = -1;
	}
}

