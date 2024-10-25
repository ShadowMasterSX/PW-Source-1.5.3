#ifndef __ONLINEGAME_GS_AUTO_TEAM_MAN_H__
#define __ONLINEGAME_GS_AUTO_TEAM_MAN_H__

#include <hashmap.h>
#include <amemory.h>
#include <common/types.h>

typedef abase::pair<char/*occupation*/, char/*need_player_cnt*/> OCCUPATION_NEED;
typedef abase::vector<OCCUPATION_NEED, abase::fast_alloc<> > OCCUPATION_CONFIG_LIST;
typedef abase::vector< int, abase::fast_alloc<> > WORLDTAG_FROM_LIST;


class gplayer_imp;
class itemdataman;

struct autoteam_goal
{
	int _id;
	short _require_lvl[2];
	short _require_maxlvl[2];
	char _require_gender;
	char _need_player_cnt;
	char _sec_lvl;
	char _require_reincarnation_times[2];
	char _require_realm_lvl[2];
	int _soul_floor;
	int _worldtag;
	A3DVECTOR _entrance_pos;	
	WORLDTAG_FROM_LIST _worldtag_from_list;
	OCCUPATION_CONFIG_LIST _occupation_config_list;
};

class autoteam_man
{
private:
	typedef abase::hash_map<int/*goal_id*/, autoteam_goal, abase::_hash_function, abase::fast_alloc<> > GOAL_CONFIG_MAP;
	GOAL_CONFIG_MAP _goal_map;
	
public:
	autoteam_man() { }
	~autoteam_man() 
	{ 
		_goal_map.clear(); 
	}

	bool InitAutoTeamConfig(itemdataman& dataman);
	void SendConfigData();
	bool CanPlayerAutoComposeTeam(int goal_id, gplayer_imp* pImp); 
	bool GetGoalEntrancePos(int goal_id, gplayer_imp* pImp, int current_worldtag, int& target_tag, A3DVECTOR& target_pos);
};

#endif //__ONLINEGAME_GS_AUTO_TEAM_MAN_H__

