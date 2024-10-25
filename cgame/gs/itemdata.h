#ifndef __ONLINEGAME_GS_ITEM_DATA_H__
#define __ONLINEGAME_GS_ITEM_DATA_H__
struct item_data 
{
	int type;		//类型
	size_t count;		//数量
	size_t pile_limit;	//堆叠限制
	int equip_mask;		//装备标志	表示可以装备在哪个地方 0x80000000表示是否64位扩展 0x40000000表示是否有附加属性
	int proc_type;		//物品的处理方式
	int classid;		//对应item对象的GUID ，如果是-1，表示不需要item对象
	struct 
	{
		int guid1;
		int guid2;
	}guid;			//GUID
	size_t price;		//单价 这个单价作为一个参考值,实际值以模板中的为准
	int expire_date; 	//物品期限
	size_t content_length;	//额外数据的大小
	char * item_content;	//额外数据 供item对象使用

};

#endif

