#ifndef _EXTRA_DROP_TABLE_H_
#define _EXTRA_DROP_TABLE_H_

#include <vector.h>
using namespace abase;

#define EXTRA_DROP_TABLE_VERSION	1

enum EDT_TYPE
{
	EDT_TYPE_REPLACE = 1,
	EDT_TYPE_ADDON,
};

struct EXTRADROPTABLE
{
	char					name[128];		// 掉落表名字
	EDT_TYPE				type;			// 1 完全替换；2 同时生效
	vector<int>				id_monsters;	// 怪物列表

	float					drop_num_probability[8];	// 掉落0~7个物品的概率
	struct 
	{
		unsigned int		id;				// 物品ID
		float				probability;	// 掉落概率

	}  drop_items[256];						// 掉落表
};


///////////////////////////////////////////////////////////////////////////////
// load data from a config file
// return	0 if succeed
//			-1 if failed.
///////////////////////////////////////////////////////////////////////////////
int load_extra_drop_tables(const char * filename);

// get extra drop tables array
vector<EXTRADROPTABLE>& get_extra_drop_table();

#endif//_EXTRA_DROP_TABLE_H_

