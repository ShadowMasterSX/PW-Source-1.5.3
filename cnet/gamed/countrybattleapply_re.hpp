
#ifndef __GNET_COUNTRYBATTLEAPPLY_RE_HPP
#define __GNET_COUNTRYBATTLEAPPLY_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "countrybattleapplyentry"

bool player_join_country(int role_id, int country_id, int country_expiretime, int major_strength, int minor_strength, int world_tag, float posx, float posy, float posz);

namespace GNET
{

class CountryBattleApply_Re : public GNET::Protocol
{
	#include "countrybattleapply_re"

	class PlayerJoinCountryTask : public Thread::Runnable
	{
		int _roleid;
		int _country_id;
		int _country_expire_time;
		int _major_strength;
		int _minor_strength;
		int _worldtag;
		float _pos[3];
	public:
		PlayerJoinCountryTask(int roleid, int country_id, int country_expire_time, int major_strength, int minor_strength, int worldtag, float posx, float posy, float posz) : _roleid(roleid), _country_id(country_id), _country_expire_time(country_expire_time), _major_strength(major_strength), _minor_strength(minor_strength), _worldtag(worldtag)
		{
			_pos[0] = posx;
			_pos[1] = posy;
			_pos[2] = posz;
		}
		void Run()
		{
			player_join_country(_roleid, _country_id, _country_expire_time, _major_strength, _minor_strength, _worldtag, _pos[0], _pos[1], _pos[2]);
			delete this;
		}
	};

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		if(retcode != ERR_SUCCESS) return;

		for(size_t i=0; i<list.size(); i++)
		{
			CountryBattleApplyEntry & ent = list[i];
			//player_join_country(ent.roleid, country_id, country_invalid_timestamp, ent.major_strength, ent.minor_strength, capital_worldtag, capital_posx, capital_posy, capital_posz);
			Thread::Runnable * task = new PlayerJoinCountryTask(ent.roleid, country_id, country_invalid_timestamp, ent.major_strength, ent.minor_strength, capital_worldtag, capital_posx, capital_posy, capital_posz);
			if(i%5 == 0)
				Thread::Pool::AddTask(task);
			else
				Thread::HouseKeeper::AddTimerTask(task, i%5);
		}		
	}
};

};

#endif
