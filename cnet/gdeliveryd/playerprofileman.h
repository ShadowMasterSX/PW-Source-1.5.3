#ifndef __GNET_PLAYER_PROFILE_MAN_H
#define __GNET_PLAYER_PROFILE_MAN_H

#include <map>
#include <set>
#include <vector>

#include "playerprofiledata"
#include "profilematchresult"

namespace GNET
{

class PlayerProfileMan
{
public:
	enum
	{
		STATE_UNSET = 0,
		STATE_READY = 1,
	};

private:
	enum
	{
		MATCH_MODE_MENTOR = 0,
		MATCH_MODE_FRIEND,
		MATCH_MODE_LOVER,
		MATCH_MODE_MAX,
	};
	
	enum
	{
		MATCH_MASK_NULL = 0x00,
		MATCH_MASK_MENTOR = 0x01,
		MATCH_MASK_FRIEND = 0x02,
		MATCH_MASK_LOVER = 0x04,
	};
	
	enum
	{
		MENTOR_LEVEL_MIN = 90,
		CANDICATE_CEIL = 20,
		MATCH_RESULT_CNT = 5,
		MATCH_COLDDOWN = 10,

		GENDER_FEMALE = 0,
		GENDER_MALE = 1,
	};
	
	enum
	{
		MASK_GAME_TIME_OPT_CNT = 4,
		MASK_GAME_INTEREST_OPT_CNT = 5,
		MASK_PERSONAL_INTEREST_OPT_CNT = 9,
		MASK_MATCH_OPT_CNT = 3,
	};
	
	enum
	{
		ST_CLOSE = 0,
		ST_OPEN,
	};
	
	struct PlayerProfileEntry
	{
		char _state;
		unsigned char _gender;
		time_t _match_rqst_timestamp;
		PlayerProfileData _data;
	};

	typedef std::map< int, PlayerProfileEntry > PROFILE_ENTRY_MAP;
	typedef std::set< int > ROLE_ID_SET;
	typedef std::map< int, ROLE_ID_SET > CANDICATE_MAP;

	int _state;
	PROFILE_ENTRY_MAP _profiles;
	CANDICATE_MAP _candicates[MATCH_MODE_MAX];
	
	static PlayerProfileMan _instance; 

private:
	PlayerProfileMan(): _state(ST_CLOSE)
	{
		_profiles.clear();
		for(int i = 0; i < MATCH_MODE_MAX; ++i) {
			_candicates[i].clear();
		}
	}

	bool QueryDB(int roleid);
	void UpdateCandicate(CANDICATE_MAP& candicates, int key, int roleid);
	void ClearCandicate(CANDICATE_MAP& candicates, int key, int roleid);
	void CalcMatchResult(int rqst_roleid, const PlayerProfileEntry& self, const ROLE_ID_SET& matchers, std::vector<ProfileMatchResult>& result);
	float CalcSimilarity(int self_roleid, const PlayerProfileData& self_data, int matcher_roleid, const PlayerProfileData& macher_data);
	int GetOptionSelectedCnt(unsigned short mask, unsigned char total_cnt);
	float CalcMaskSimilarity(unsigned short self_mask, unsigned short matcher_mask, unsigned char total_opt_cnt, float weight);

public:
	static PlayerProfileMan* GetInstance() { return &_instance; }
	bool Initialize() { _state = ST_OPEN; return true; }

	void OnPlayerLogin(int roleid);
	void OnPlayerLogout(int roleid, int occupation);
	void GetPlayerProfileData(int roleid, int linksid, int localsid);
	void SetPlayerProfileData(int roleid, const PlayerProfileData& data);
	bool UpdatePlayerProfile(int roleid, char state, unsigned char gender, const PlayerProfileData& data);
	void GetMatchResult(int roleid, int match_mode, int linksid, int localsid);
};

}

#endif //__GNET_PLAYER_PROFILE_MAN_H

