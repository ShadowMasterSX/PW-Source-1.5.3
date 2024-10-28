/*
 * FILE: globaldataman.h
 *
 * DESCRIPTION: global data loader and manager
 *
 * CREATED BY: Hedi, 2005/7/18
 *
 * HISTORY: 
 *
 * Copyright (c) 2005 Archosaur Studio, All Rights Reserved.
 */

#ifndef _GLOBALDATAMAN_H_
#define _GLOBALDATAMAN_H_

#include <vector.h>
#include <a3dvector.h>

enum {
	TREASURE_ITEM_OWNER_NPC_COUNT = 8,
};
typedef struct _TRANS_TARGET_SERV
{
	int		id;
	int 		world_tag;
	A3DVECTOR3	vecPos;
	int		domain_id;
} TRANS_TARGET_SERV;

bool globaldata_loadserver(const char * trans_data,const char * mall_data,const char * mall2_data, const char * mall3_data);
bool globaldata_releaseserver();

abase::vector<TRANS_TARGET_SERV> & globaldata_gettranstargetsserver();


typedef struct _MALL_ITEM_SERV
{
	int     goods_id;
	int     goods_count;

	struct __entry
	{
		int 	group_id;	//��id
		int 	st_type;	//sale_time::type
		int 	st_param1;	//sale_time::param1
		int 	st_param2;	//sale_time::param2
		int 	status;		//��Ʒ״̬����Ʒ���������Ƽ�
		int 	expire_date_valid;	//expire_time �Ƿ�date
		int     expire_time;
		int     cash_need;
		int     min_vip_level;
	}list[4];
	
	int gift_id;
	int gift_count;
	int gift_expire_time;
	int gift_log_price;
	int spec_owner[TREASURE_ITEM_OWNER_NPC_COUNT];
	int buy_times_limit;//�޹�����
	int buy_times_limit_mode;//�޹���ʽ 0���޹� 1ÿ���޹� 2ÿ���޹�
} MALL_ITEM_SERV;
abase::vector<MALL_ITEM_SERV> & globaldata_getmallitemservice();
int globaldata_getmalltimestamp();

abase::vector<MALL_ITEM_SERV> & globaldata_getmall2itemservice();
int globaldata_getmall2timestamp();

abase::vector<MALL_ITEM_SERV> & globaldata_getmall3itemservice();
int globaldata_getmall3timestamp();

#endif//_GLOBALDATAMAN_H_
