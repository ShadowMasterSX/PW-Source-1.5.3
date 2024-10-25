#include "titlemanager.h"
#include "template/itemdataman.h"
#include <common/protocol.h>
#include "world.h"
#include "player_imp.h"

void title_essence::ActivateEssence(gplayer_imp *pImp) const
{
	pImp->TitleEnhance(_phy_damage,_magic_damage,_phy_defence,_magic_defence,_attack,_armor);

	__PRINTF("title active pd%d md%d pdf%d mdf%d atk%d arm%d\n",_phy_damage,_magic_damage,_phy_defence,_magic_defence,_attack,_armor);
}
void title_essence::DeactivateEssence(gplayer_imp *pImp) const
{
	pImp->TitleImpair(_phy_damage,_magic_damage,_phy_defence,_magic_defence,_attack,_armor);
	
	__PRINTF("title deactive pd%d md%d pdf%d mdf%d atk%d arm%d\n",_phy_damage,_magic_damage,_phy_defence,_magic_defence,_attack,_armor);
}

bool title_manager::InitTitle(itemdataman & dataman)
{
	DATA_TYPE dt;

	size_t id = dataman.get_first_data_id(ID_SPACE_CONFIG,dt);
	for(; id != 0; id = dataman.get_next_data_id(ID_SPACE_CONFIG,dt))
	{
		if(dt == DT_TITLE_CONFIG)
		{
			const TITLE_CONFIG &tc = *(const TITLE_CONFIG*)dataman.get_data_ptr(id,ID_SPACE_CONFIG,dt); 
			ASSERT(dt == DT_TITLE_CONFIG);
			
			_title_map[tc.id] = new	title_essence(tc.phy_damage,tc.magic_damage,
					                tc.phy_defence,tc.magic_defence,tc.attack,tc.armor,tc.broadcast>0);
		}
		else if(dt == DT_COMPLEX_TITLE_CONFIG)
		{
			const COMPLEX_TITLE_CONFIG &ctc = *(const COMPLEX_TITLE_CONFIG*)dataman.get_data_ptr(id,ID_SPACE_CONFIG,dt); 
			ASSERT(dt == DT_COMPLEX_TITLE_CONFIG);
			
			_title_map[ctc.id] = new title_essence(ctc.phy_damage,ctc.magic_damage,
					                 ctc.phy_defence,ctc.magic_defence,ctc.attack,ctc.armor,ctc.broadcast>0);

			ASSERT((sizeof(_super_title_cond_map[ctc.id].sub_titles) == sizeof(ctc.sub_titles)) && "复合称号条件长度问题");
			memcpy(_super_title_cond_map[ctc.id].sub_titles,ctc.sub_titles, sizeof(ctc.sub_titles));
			ASSERT(ctc.sub_titles[0] && "复合称号条件缺少");
			for(int i = 0; i < SUB_TITLE_COUNT; ++i)
			{
				if(ctc.sub_titles[i])	_sub_title_map[TITLE_ID(ctc.sub_titles[i])].push_back(ctc.id);
			}

		}

	}

	return CheckSubTitleValid();
}

bool title_manager::CheckSubTitleValid()
{
	SUB_TITLE_MAP::iterator iter = _sub_title_map.begin();
	SUB_TITLE_MAP::iterator iend = _sub_title_map.end();
		
	for(;iter != iend; ++ iter)
	{
		if(!IsVailidTitle(iter->first))
		{
			__PRINTF("subtitle%d  invailid\n",iter->first);
			return false;
		}
	}	

	return true;
}

bool title_manager::TouchSuperTitle(player_title* pt,TITLE_ID tid)
{
	SUB_TITLE_MAP::iterator iter = _sub_title_map.find(tid);
	
	if(iter == _sub_title_map.end())
		return false;

	SUPER_TITLE_LIST& slist = iter->second;

	SUPER_TITLE_LIST::iterator sliter = slist.begin();
	SUPER_TITLE_LIST::iterator sliend = slist.end();

	for(;sliter != sliend; ++sliter)
	{
		TITLE_ID stid = *sliter;

		if(pt->IsExistTitle(stid))
			continue;

		SUPER_TITLE_COND_MAP::iterator siter = _super_title_cond_map.find(stid);	
		
		if(siter == _super_title_cond_map.end())
			continue;

		super_title_cond& cond = siter->second;

		int i = 0;
		for(; i < SUB_TITLE_COUNT; ++i)
		{
			TITLE_ID ctid = cond.sub_titles[i];
			if(ctid && !pt->IsExistTitle(ctid))
				break;
		}

		if(i == SUB_TITLE_COUNT)	pt->ObtainTitle(stid,0);
	}

	return true;
}
