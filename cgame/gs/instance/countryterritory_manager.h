#ifndef __ONLINEGAME_GS_COUNTRYTERRITORY_MANAGER_H__
#define __ONLINEGAME_GS_COUNTRYTERRITORY_MANAGER_H__

#include "instance_manager.h"

class countryterritory_world_manager : public instance_world_manager
{
	struct capital_entry
	{
		int country_id;
		int world_tag;
		A3DVECTOR pos;
	};
	abase::vector<capital_entry> _capital_list;
	bool GetCapital(int country_id, A3DVECTOR &pos, int & tag)
	{
		int list[4];
		int counter = 0;
		for(size_t i=0; i<_capital_list.size() && counter < 4; i++)	
		{
			if(country_id == _capital_list[i].country_id)
			{
				list[counter++] = i;
			}
		}
		if(counter > 0)
		{
			int index = abase::Rand(0,counter-1);
			pos = _capital_list[list[index]].pos;
			tag = _capital_list[list[index]].world_tag;
			return true;
		}
		return false;
	}
	void SetCapital(int country_id, const A3DVECTOR &pos, int tag)
	{
		capital_entry ent;
		ent.country_id = country_id;
		ent.world_tag = tag;
		ent.pos = pos;
		_capital_list.push_back(ent);	
	}
	bool IsCapitalPos(int country_id, const A3DVECTOR &pos)
	{
		for(size_t i=0; i<_capital_list.size(); i++)
		{
			if(country_id == _capital_list[i].country_id && pos.squared_distance(_capital_list[i].pos) <= 1.f) return true;	
		}
		return false;
	}

public:
	countryterritory_world_manager()	{}
	virtual ~countryterritory_world_manager(){}
	virtual int GetWorldType(){ return WORLD_TYPE_COUNTRYTERRITORY; }
	
public:	
	virtual world_message_handler * CreateMessageHandler();
	virtual void OnDeliveryConnected();
	virtual bool IsCountryTerritoryWorld(){ return true; }
	virtual void NotifyCountryBattleConfig(GMSV::CBConfig * config);

	virtual void SetFilterWhenLogin(gplayer_imp * pImp, instance_key * );
	virtual void PlayerAfterSwitch(gplayer_imp * pImp);
	virtual void GetLogoutPos(gplayer_imp * pImp, int &world_tag, A3DVECTOR & pos);
	virtual bool GetTownPosition(gplayer_imp *pImp, const A3DVECTOR &opos, A3DVECTOR &pos, int & tag);
	virtual void TransformInstanceKey(const instance_key::key_essence & key, instance_hash_key & hkey)
	{
		hkey.key1 = key.key_level4;
		hkey.key2 = 0;
	}	
};

class countryterritory_world_message_handler : public instance_world_message_handler
{
protected:
	virtual ~countryterritory_world_message_handler(){}
public:
	countryterritory_world_message_handler(instance_world_manager * man):instance_world_message_handler(man) {}
};

#endif
