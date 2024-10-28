#include "../world.h"
#include "../player_imp.h"
#include "../aei_filter.h"
#include <gsp_if.h>
#include "countryterritory_manager.h"


world_message_handler * countryterritory_world_manager::CreateMessageHandler()
{
	return new countryterritory_world_message_handler(this);
}

void countryterritory_world_manager::OnDeliveryConnected()
{
	GMSV::SendCountryBattleServerRegister(0, GetWorldIndex(),GetWorldTag(),-1);
}

void countryterritory_world_manager::NotifyCountryBattleConfig(GMSV::CBConfig * config)
{
	_capital_list.clear();
	for(size_t i=0; i<config->capital_count; i++)
	{
		GMSV::CBConfig::CountryCapital & capital = config->capital_list[i];
		SetCapital(capital.country_id, A3DVECTOR(capital.posx,capital.posy,capital.posz), capital.worldtag);
	}
}

void countryterritory_world_manager::SetFilterWhenLogin(gplayer_imp * pImp, instance_key * )
{
	pImp->_filters.AddFilter(new aect_filter(pImp,FILTER_CHECK_KICKOUT));

	int country_id = pImp->GetCountryId();
	if(country_id && IsCapitalPos(country_id,pImp->_parent->pos))
	{
		//在首都则设置为GM隐身
		object_interface obj_if(pImp);
		obj_if.SetGMInvisibleFilter(true, -1, filter::FILTER_MASK_NOSAVE);
	}
}

void countryterritory_world_manager::PlayerAfterSwitch(gplayer_imp * pImp)
{
	countryterritory_switch_data * pData = substance::DynamicCast<countryterritory_switch_data>(pImp->_switch_additional_data);
	if(pData)
	{
		pImp->CountryJoinStep2();
	}
	else
	{
		pImp->ClearSwitchAdditionalData();
	}
}

void countryterritory_world_manager::GetLogoutPos(gplayer_imp * pImp, int &world_tag, A3DVECTOR & pos)
{
	int country_id = pImp->GetCountryId();
	if(country_id)
	{
		//登出点设置为国家首都
		if(GetCapital(country_id, pos, world_tag)) return;
		world_tag = 143;
		pos = A3DVECTOR(0,0,0);
		GLog::log(GLOG_ERR,"首都信息不存在worldtag=%d roleid=%d country=%d", GetWorldTag(), pImp->_parent->ID.id, country_id);
		return;
	}
	pImp->GetCarnivalKickoutPos(world_tag, pos);
}

bool countryterritory_world_manager::GetTownPosition(gplayer_imp *pImp, const A3DVECTOR &opos, A3DVECTOR &pos, int & tag)
{
	//回城点设置在原地
	pos = opos;
	tag = _world_tag;
	return true;
}
