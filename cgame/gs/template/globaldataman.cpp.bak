/*
 * FILE: globaldataman.cpp
 *
 * DESCRIPTION: global data loader and manager
 *
 * CREATED BY: Hedi, 2005/7/18
 *
 * HISTORY: 
 *
 * Copyright (c) 2005 Archosaur Studio, All Rights Reserved.
 */

#include <vector.h>
#include "globaldataman.h"
#include "string.h"

static abase::vector<TRANS_TARGET_SERV>	global_trans_targets_server;

abase::vector<TRANS_TARGET_SERV> & globaldata_gettranstargetsserver()
{
	return global_trans_targets_server;
}

bool load_transdata(const char * file)
{
	FILE * fpServer = fopen(file, "rb");
	if(!fpServer) return false;

	int nCount;
	fread(&nCount, 1, sizeof(int), fpServer);

	// now output a trans target list for server usage
	for(int i=0; i<nCount; i++)
	{
		TRANS_TARGET_SERV target;
		fread(&target, 1, sizeof(target), fpServer);
		global_trans_targets_server.push_back(target);
	}

	fclose(fpServer);
	return true;
}


static abase::vector<MALL_ITEM_SERV>  global_mall_item_service;
static int mall_timestamp = 0;

#pragma pack(push, GSHOP_ITEM_PACK, 1)
typedef struct _GSHOP_ITEM
{
	int						local_id;		// id of this shop item, used only for localization purpose
	int						main_type;		// index into the main type array
	int						sub_type;		// index into the sub type arrray

	unsigned int			id;				// object id of this item
	unsigned int			num;			// number of objects in this item

	struct {
		unsigned int		price;			// price of this item	
		unsigned int end_time; //(年/月/日/时/分/秒)-如果为0，则持续生效
		unsigned int time; //租借时间（秒，0表示无期）
		unsigned int start_time;//开启时间：(年/月/日/时/分/秒)-如果为0，则在结束生效时间点前都生效		
		int type; //时间类型，0持续时间，1每周，2每月, -1 无效
		unsigned int day; //由位表示是否选择了某一天，可表示周也可表示月，由低到高 bit0-6 周日到周六 bit0-30 1-31号
		unsigned int status; //物品状态，0无，1新品，2促销，3推荐
		unsigned int flag; //分类控制
	} buy[4];

	unsigned int idGift;
	unsigned int iGiftNum;
	unsigned int iGiftTime;
	unsigned int iLogPrice;
	unsigned int owner_npcs[TREASURE_ITEM_OWNER_NPC_COUNT];
} GSHOP_ITEM;
#pragma pack(pop, GSHOP_ITEM_PACK)

abase::vector<MALL_ITEM_SERV> & globaldata_getmallitemservice()
{       
	return global_mall_item_service;
}

int globaldata_getmalltimestamp()
{       
	return mall_timestamp;
}

static abase::vector<MALL_ITEM_SERV>  global_mall2_item_service;
static int mall2_timestamp = 0;

abase::vector<MALL_ITEM_SERV> & globaldata_getmall2itemservice()
{       
	return global_mall2_item_service;
}

int globaldata_getmall2timestamp()
{       
	return mall2_timestamp;
}

static bool load_malldata(const char * file, abase::vector<MALL_ITEM_SERV> & __global_mall_item_service, int & __mall_timestamp)
{
	FILE * fpServer = fopen(file, "rb");
	if(!fpServer) return false;

	bool bRst = true;
	try  
	{
		int timestamp;
		if(fread(&timestamp, 1, sizeof(int), fpServer) != sizeof(int)) throw -1;

		int nCount;
		if(fread(&nCount, 1, sizeof(int), fpServer) != sizeof(int) || nCount < 0 || nCount > 65535) throw -2;

		// now output a trans target list for server usage
		for(int i=0; i<nCount; i++)
		{
			GSHOP_ITEM data;
			if(fread(&data, 1, sizeof(data), fpServer) != sizeof(data))
			{
				throw -3;
			}       
			MALL_ITEM_SERV goods;
			goods.goods_id = data.id;
			goods.goods_count = data.num;
			for(size_t j = 0;j < 4; j ++)
			{
				goods.list[j].group_id = data.buy[j].flag;
				if(data.buy[j].type == -1)
				{
					goods.list[j].st_type = 0;
					goods.list[j].st_param1 = 0;
					goods.list[j].st_param2 = 0;
				}
				else if(data.buy[j].type == 0)
				{
					goods.list[j].st_type = 1;
					goods.list[j].st_param1 = data.buy[j].start_time;
					goods.list[j].st_param2 = data.buy[j].end_time;
				}
				else if(data.buy[j].type == 1)
				{
					goods.list[j].st_type = 2;
					goods.list[j].st_param1 = data.buy[j].day & 0x7F;
					goods.list[j].st_param2 = 0;
				}
				else if(data.buy[j].type == 2)
				{
					goods.list[j].st_type = 3;
					goods.list[j].st_param1 = (data.buy[j].day & 0x7FFFFFFF)<<1;
					goods.list[j].st_param2 = 0;
				}
				else
					throw -3;	
				goods.list[j].status = data.buy[j].status;
				
				goods.list[j].cash_need = data.buy[j].price;
				goods.list[j].expire_date_valid = 0; 
				goods.list[j].expire_time = data.buy[j].time;
			}

			goods.gift_id = data.idGift;
			goods.gift_count = data.iGiftNum;
			goods.gift_expire_time = data.iGiftTime;
			goods.gift_log_price = data.iLogPrice;
			memcpy(goods.spec_owner,data.owner_npcs,sizeof(goods.spec_owner));

			__global_mall_item_service.push_back(goods);
		}

		__mall_timestamp = timestamp;
	}
	catch(...)
	{
		bRst = false;
	}
	fclose(fpServer);
	return bRst;
}

bool globaldata_loadserver(const char * file, const char * file2, const char * file3)
{
	if(!load_transdata(file)) return false;
	if(!load_malldata(file2,global_mall_item_service,mall_timestamp)) return false;
	return (file3==NULL || strlen(file3)==0) ? true : load_malldata(file3,global_mall2_item_service,mall2_timestamp);
}

bool globaldata_releaseserver()
{
	global_trans_targets_server.clear();
	global_mall_item_service.clear();
	global_mall2_item_service.clear();
	return true;
}
