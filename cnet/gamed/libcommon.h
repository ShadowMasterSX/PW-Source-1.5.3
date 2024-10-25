#ifndef __GNET_LIBCOMMON_H
#define __GNET_LIBCOMMON_H

#include "../gdbclient/db_if.h" //for struct itemdata
#include "../include/localmacro.h"
#include "groleinventory"
#include "grolestorehouse"
#include "gmailsyncdata"
#include "../perf/perf.h"
#include "gpet"
#include "gpetcorral"
namespace GNET
{
	static void CvtItm2Ivry( const GDB::itemdata* list, size_t size, GRoleInventoryVector& inventory )
	{
		for ( size_t i=0;i<size;++i )
		{
			inventory.add(
				GRoleInventory(
					list[i].id,
					list[i].index,
					list[i].count,
					list[i].max_count,
					Octets( list[i].data,list[i].size ),
					list[i].proctype,
					list[i].expire_date,
					list[i].guid1,
					list[i].guid2,
					list[i].mask
				)	
			);
		}
	}
	static bool GetInventory( GRoleInventoryVector& inventory,object_interface& obj_if )
	{
		size_t inv_size=obj_if.GetInventorySize();
		int item_size=0;
		GDB::itemdata list[inv_size];
		if ( (item_size=obj_if.GetInventoryDetail(&list[0],inv_size))<0 )
		   	return false;
		CvtItm2Ivry( &list[0],item_size,inventory );
		return true;
	}
	static bool GetEquipment( GRoleInventoryVector& equipment,object_interface& obj_if )
	{       
		size_t equip_size = obj_if.GetEquipmentSize();
		int item_size=0;
		GDB::itemdata list[equip_size];
		if ( (item_size=obj_if.GetEquipmentDetail(&list[0],equip_size))<0 )
			return false;
		CvtItm2Ivry( &list[0],item_size,equipment );
		return true;
	}

    /*static bool GetShoplog( std::vector<GShopLog>& logs,object_interface& obj_if )
	{
		size_t size = obj_if.GetMallOrdersCount();
		if(!size)
			return true;
		GDB::shoplog *list = new GDB::shoplog[size];
		if ( obj_if.GetMallOrders(list,size)<0 )
			return false;
		int roleid = obj_if.GetSelfID().id;
		for(size_t i=0;i<size;++i)
		{
			logs.push_back(GShopLog(
						roleid,
						list[i].order_id,
						list[i].item_id,
						list[i].expire,
						list[i].item_count,
						list[i].order_count,
						list[i].cash_need,
						list[i].time,
						list[i].guid1,
						list[i].guid2)
				      );
		}
		delete list;
		return true;
	}*/

	static bool GetStorehouse( GRoleStorehouse& store,object_interface& obj_if )
	{
		store.money = obj_if.GetTrashBoxMoney();
		{
			size_t size = obj_if.GetTrashBoxCapacity();
			store.capacity = size;
			GDB::itemdata list[size];
			int item_size=0;
			if ( (item_size=obj_if.GetTrashBoxDetail(list,size))<0 )
				return false;
			CvtItm2Ivry( list,item_size,store.items );
		}
		{
			size_t size = obj_if.GetTrashBox2Capacity();
			store.size1 = size;
			GDB::itemdata list[size];
			int item_size=0;
			if ( (item_size=obj_if.GetTrashBox2Detail(list,size))<0 )
				return false;
			CvtItm2Ivry( list,item_size,store.dress );
		}
		{
			size_t size = obj_if.GetTrashBox3Capacity();
			store.size2 = size;
			GDB::itemdata list[size];
			int item_size=0;
			if ( (item_size=obj_if.GetTrashBox3Detail(list,size))<0 )
				return false;
			CvtItm2Ivry( list,item_size,store.material );
		}
		{
			size_t size = obj_if.GetTrashBox4Capacity();
			store.size3 = size;
			GDB::itemdata list[size];
			int item_size=0;
			if ( (item_size=obj_if.GetTrashBox4Detail(list,size))<0 )
				return false;
			CvtItm2Ivry( list,item_size,store.generalcard);
		}
		return true;
	}

	static bool GetSyncData(GMailSyncData& data, object_interface& obj_if)
	{
		int used, delta;
		data.data_mask = 0;
		data.inventory.timestamp = obj_if.InceaseDBTimeStamp();
		data.inventory.money = obj_if.GetMoney();
		data.inventory.capacity = obj_if.GetInventorySize();
		if(obj_if.IsCashModified())
			data.data_mask |= SYNC_CASHUSED;
		if(!obj_if.GetMallInfo(data.cash_total, used, delta, data.cash_serial))
			return false;
		data.cash_total += used;        //gs和delivery的意义转换
		data.cash_used = used - delta;
		if (!GetInventory(data.inventory.items,obj_if)) 
			return false;
		if (!GetEquipment(data.equipment,obj_if))
			return false;
		//if (!GetShoplog(data.logs,obj_if))
		//	return false;
		if(obj_if.IsTrashBoxModified())
		{
			data.data_mask |= SYNC_STOTEHOUSE;
			if(!GetStorehouse(data.storehouse,obj_if)) 
				return false;
		}
		return true;
	}

	static bool GetSkills(Octets& skills, unsigned int& crc, object_interface& obj_if)
	{
		size_t skill_size = obj_if.GetSkillDataSize();
		char skill_buf[skill_size];
		if(!obj_if.GetSkillData(skill_buf, skill_size))
			return false;
		skills.replace(skill_buf,skill_size);
		crc = crc32((const unsigned char *)skill_buf,skill_size);
		return true;
	}

	static bool GetEquipment(GRoleInventoryVector& equipment, unsigned int& crc, object_interface& obj_if)
	{
		if(!GetEquipment(equipment, obj_if))
			return false;
		size_t equip_size = equipment.size();
		if(equip_size)
		{
			//用id guid1 guid2判断装备是否改变 可以避免因耐久度频繁变化引起的数据更新,
			//但对于精炼打孔镶嵌等操作，则误认为没有变化
			struct{
				unsigned int id;
				int guid1;
				int guid2;
			}equip_brief[equip_size];
			for(size_t i=0; i<equip_size; i++)
			{
				equip_brief[i].id = equipment[i].id;
				equip_brief[i].guid1 = equipment[i].guid1;
				equip_brief[i].guid2 = equipment[i].guid2;
			}
			crc = crc32((const unsigned char *)equip_brief, sizeof(equip_brief));
		}
		else
			crc = 0;
		return true;
	}
	
	static void set_petcorral(GNET::Octets& data, const GDB::petcorral& corral)
	{
		GPet tmppet;
		GPetCorral tmpcor;
		Marshal::OctetsStream os;

		tmpcor.capacity = corral.capacity; 
		const GDB::pet* ptmp = corral.list;
		for(size_t i=0;i<corral.count;i++,ptmp++)
		{
			tmppet.index = ptmp->index;
			tmppet.data.replace(ptmp->data.data, ptmp->data.size);
			tmpcor.pets.push_back(tmppet);
		}
		os << tmpcor;
		data = os;
	}

	static bool GetPetCorral(Octets& petcorral, unsigned int& crc, object_interface& obj_if)
	{
		size_t pet_count = obj_if.GetPetsCount();
		GDB::petcorral pets;
		GDB::pet list[pet_count];
		pets.count = pet_count;
		if(pet_count)
			pets.list = list;
		else
			pets.list = NULL;
		if(!obj_if.GetPetsData(pets))
			return false;
		set_petcorral(petcorral, pets);
		crc = crc32((const unsigned char *)petcorral.begin(),petcorral.size());
		return true;
	}
}
#endif
