#include "../template/itemdataman.h"
#include "generalcard_set_man.h"



bool generalcard_set_manager::LoadTemplate(itemdataman & dataman)
{
	DATA_TYPE  dt;
	unsigned int id = dataman.get_first_data_id(ID_SPACE_ESSENCE,dt);
	for(; id != 0; id = dataman.get_next_data_id(ID_SPACE_ESSENCE,dt))
	{
		if(dt == DT_POKER_SUITE_ESSENCE)
		{
			const POKER_SUITE_ESSENCE & ess = *(const POKER_SUITE_ESSENCE *)dataman.get_data_ptr(id,ID_SPACE_ESSENCE,dt);
			ASSERT(&ess && dt == DT_POKER_SUITE_ESSENCE);

			generalcard_set * cardset = new generalcard_set();
			cardset->id = ess.id;
			cardset->enhance = ess.promote_ratio;
			for(size_t i=0; i<sizeof(ess.list)/sizeof(ess.list[0]); i++)
			{
				if((int)(ess.list[i]) > 0) cardset->total_count ++;
			}
			if(!Insert((const int *)&ess.list[0], sizeof(ess.list)/sizeof(ess.list[0]), cardset))
			{
				delete cardset;
				__PRINTINFO("卡牌套装%d存在错误的条目\n",ess.id);
				return false;
			}
		}	
	}
	return true;
}
