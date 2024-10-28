#include "playerprofileman.h"
#include "playerprofilegetprofiledata_re.hpp"
#include "playerprofilegetmatchresult_re.hpp"
#include "dbgetplayerprofiledata.hrp"
#include "dbputplayerprofiledata.hrp"

#include "mapuser.h"
#include "gamedbclient.hpp"
#include "gdeliveryserver.hpp"

namespace GNET
{

PlayerProfileMan PlayerProfileMan::_instance;

bool PlayerProfileMan::QueryDB(int roleid)
{
	DBGetPlayerProfileDataArg arg;
	arg.roleid = roleid;
	
	DBGetPlayerProfileData* rpc = (DBGetPlayerProfileData*)Rpc::Call(RPC_DBGETPLAYERPROFILEDATA, arg);
	return GameDBClient::GetInstance()->SendProtocol(rpc);
}

void PlayerProfileMan::OnPlayerLogin(int roleid)
{
	if(_state != ST_OPEN) return;
	QueryDB(roleid);
}

void PlayerProfileMan::OnPlayerLogout(int roleid, int occupation)
{
	if(_state != ST_OPEN) return;

	PROFILE_ENTRY_MAP::iterator it = _profiles.find(roleid);
	if(it == _profiles.end()) return;
	
	int gender = it->second._gender;
	
	_profiles.erase(roleid);
	
	for(int i = 0; i < MATCH_MODE_MAX; ++i) {
		CANDICATE_MAP& candicates = _candicates[i];
		if( i == MATCH_MODE_MENTOR ) {
			int key = occupation;
			ClearCandicate(candicates, key, roleid);
		} else if(i == MATCH_MODE_FRIEND) {
			int key = 0;
			ClearCandicate(candicates, key, roleid);
		} else if(i == MATCH_MODE_LOVER) {
			int key = (int)gender;
			ClearCandicate(candicates, key, roleid);
		}
	}
	
	LOG_TRACE( "PlayerProfile PlayerLogout roleid=%d\n", roleid);
}

void PlayerProfileMan::GetPlayerProfileData(int roleid, int linksid, int localsid)
{
	LOG_TRACE( "PlayerProfile GetPlayerProfileData roleid=%d\n", roleid);
	if(_state != ST_OPEN) return;
	
	PlayerProfileGetProfileData_Re re;
	re.retcode = -1;
	re.localsid = localsid;
	
	PROFILE_ENTRY_MAP::iterator it = _profiles.find(roleid);
	if(it == _profiles.end()) { //理论上说玩家登陆就会向DB请求，不应该找不到，这里是为了以防万一的处理
		QueryDB(roleid);
		GDeliveryServer::GetInstance()->Send(linksid, re);
		return;
	}

	PlayerProfileEntry& entry = it->second;
	re.retcode = entry._state;
	re.data = entry._data;
		
	GDeliveryServer::GetInstance()->Send(linksid, re);
}

void PlayerProfileMan::UpdateCandicate(CANDICATE_MAP& candicates, int key, int roleid)
{
	CANDICATE_MAP::iterator it = candicates.find(key);
	if(it != candicates.end()) {
		ROLE_ID_SET& roles = it->second;
		roles.insert(roleid);
	} else {
		ROLE_ID_SET roles;
		roles.insert(roleid);
		candicates[key] = roles;
	}
}

void PlayerProfileMan::ClearCandicate(CANDICATE_MAP& candicates, int key, int roleid)
{
	CANDICATE_MAP::iterator it = candicates.find(key);
	if(it != candicates.end()) {
		ROLE_ID_SET& roles = it->second;
		roles.erase(roleid);
	}
}

bool PlayerProfileMan::UpdatePlayerProfile(int roleid, char state, unsigned char gender, const PlayerProfileData& data)
{
	PlayerProfileEntry entry;
	entry._state = state;
	entry._gender = gender;
	entry._match_rqst_timestamp = 0;
	entry._data = data;
	_profiles[roleid] = entry;
	
	if(state == STATE_UNSET) return true;
	
	for(int i = 0; i < MATCH_MODE_MAX; ++i) {
		PlayerInfo* pInfo = UserContainer::GetInstance().FindRoleOnline( roleid );
		if(pInfo == NULL) return false;

		CANDICATE_MAP& candicates = _candicates[i];
		if(i == MATCH_MODE_MENTOR) { 
			if(pInfo->level < MENTOR_LEVEL_MIN) continue;
			int key = pInfo->cls;

			if(data.match_option_mask & MATCH_MASK_MENTOR) {
				UpdateCandicate(candicates, key, roleid);
			} else {
				ClearCandicate(candicates, key, roleid);
			}
		} else if(i == MATCH_MODE_FRIEND) {
			int key = 0;
			if(data.match_option_mask & MATCH_MASK_FRIEND){
				UpdateCandicate(candicates, key, roleid);
			} else {
				ClearCandicate(candicates, key, roleid);
			}
		} else if(i == MATCH_MODE_LOVER) {
			int key = (int)gender;
			if(data.match_option_mask & MATCH_MASK_LOVER) {
				UpdateCandicate(candicates, key, roleid);
			} else {
				ClearCandicate(candicates, key, roleid);
			}
		}
	}

	return true;
}

void PlayerProfileMan::SetPlayerProfileData(int roleid, const PlayerProfileData& data)
{
	LOG_TRACE( "PlayerProfile SetPlayerProfileData roleid=%d, data(game_time_mask=%0x, game_interest_mask=%0x, personal_interest_mask=%0x, age=%d, zodiac=%d, match_option_mask=%0x)\n", 
		roleid, data.game_time_mask, data.game_interest_mask, data.personal_interest_mask, data.age, data.zodiac, data.match_option_mask);
	
	if(_state != ST_OPEN) return;

	PROFILE_ENTRY_MAP::iterator it = _profiles.find(roleid);
	if(it == _profiles.end()) return;
	
	PlayerProfileEntry& entry = it->second;
	
	bool ret = UpdatePlayerProfile(roleid, STATE_READY, entry._gender, data);
	if(ret) {
		DBPutPlayerProfileDataArg arg;
		arg.roleid = roleid;
		arg.data = data;

		DBPutPlayerProfileData* rpc = (DBPutPlayerProfileData*)Rpc::Call(RPC_DBPUTPLAYERPROFILEDATA, arg);
		GameDBClient::GetInstance()->SendProtocol(rpc);
	}
}

bool compaire_smiliarity(const ProfileMatchResult& info1, const ProfileMatchResult& info2)
{
	if(info1.similarity != info2.similarity) {
		return (info1.similarity > info2.similarity);
	} else {
		return (info1.roleid < info2.roleid);
	}
}

int PlayerProfileMan::GetOptionSelectedCnt(unsigned short mask, unsigned char total_opt_cnt)
{
	int count = 0;
	for(int i = 0; i < total_opt_cnt; ++i) {
		count += mask & 0x0001u;
		mask >>= 1;
	}

	return count;
}

float PlayerProfileMan::CalcMaskSimilarity(unsigned short self_mask, unsigned short matcher_mask, unsigned char total_opt_cnt, float weight)
{
	int self_opt_cnt = GetOptionSelectedCnt(self_mask, total_opt_cnt);
	if(self_opt_cnt == 0) return 0.0f;

	unsigned short mask_same = self_mask & matcher_mask;
	int same_opt_cnt = GetOptionSelectedCnt(mask_same, total_opt_cnt);
	
	float ret = (float)(same_opt_cnt) / (self_opt_cnt) * weight;
	return ret;
}

float PlayerProfileMan::CalcSimilarity(int self_roleid, const PlayerProfileData& self_data, int matcher_roleid, const PlayerProfileData& matcher_data)
{
	const float WEIGHT_GAME_TIME = 0.15f;
	const float WEIGHT_GAME_INTEREST = 0.35f;
	const float WEIGHT_PERSONAL_INTEREST = 0.25f;
	const float WEIGHT_AGE = 0.1f;
	const float WEIGHT_ZODIAC = 0.15f;

	float ret = 0.0f;
	
	float f1 = CalcMaskSimilarity(self_data.game_time_mask, matcher_data.game_time_mask, MASK_GAME_TIME_OPT_CNT, WEIGHT_GAME_TIME);
	float f2 = CalcMaskSimilarity(self_data.game_interest_mask, matcher_data.game_interest_mask, MASK_GAME_INTEREST_OPT_CNT, WEIGHT_GAME_INTEREST);
	float f3 = CalcMaskSimilarity(self_data.personal_interest_mask, matcher_data.personal_interest_mask, MASK_PERSONAL_INTEREST_OPT_CNT, WEIGHT_PERSONAL_INTEREST);
	
	float f4 = ((self_data.age == matcher_data.age) ? 1 : 0) * WEIGHT_AGE;
	float f5 = ( (float)(rand() % 100) / 100 ) * WEIGHT_ZODIAC;

	ret = (f1 + f2 + f3 + f4 + f5) * 0.7 + 0.3;
	//LOG_TRACE( "PlayerProfile CalcSimilarity self_roleid=%d, matcher_roleid=%d, f1=%f, f2=%f, f3=%f, f4=%f, f5=%f, ret=%f\n", self_roleid, matcher_roleid, f1, f2, f3, f4, f5, ret);

	return ret;
}

void PlayerProfileMan::CalcMatchResult(int rqst_roleid, const PlayerProfileEntry& self, const ROLE_ID_SET& matchers, std::vector<ProfileMatchResult>& result)
{
	const PlayerProfileData& self_data = self._data;
	
	std::vector<ProfileMatchResult> tmp_vec;
	for(ROLE_ID_SET::const_iterator it = matchers.begin(); it != matchers.end(); ++it) {
		int roleid = *it;
		if(roleid == rqst_roleid) continue;
		
		PlayerInfo* pInfo = UserContainer::GetInstance().FindRoleOnline( roleid );
		if(pInfo == NULL) continue;

		PROFILE_ENTRY_MAP::iterator it_profile = _profiles.find(roleid);
		if(it_profile == _profiles.end() || it_profile->second._state != STATE_READY) continue;

		const PlayerProfileData& matcher_data = it_profile->second._data;
	
		ProfileMatchResult info;
		info.roleid = roleid;
		info.level = pInfo->level;
		info.occupation = pInfo->cls;
		info.gender = it_profile->second._gender;
		info.similarity = CalcSimilarity(rqst_roleid, self_data, roleid, matcher_data);

		tmp_vec.push_back(info);
	}

	std::sort(tmp_vec.begin(), tmp_vec.end(), compaire_smiliarity);
	int cnt = tmp_vec.size();
	if(cnt > MATCH_RESULT_CNT) cnt = MATCH_RESULT_CNT; 
	
	result.clear();
	result.insert(result.begin(), tmp_vec.begin(), tmp_vec.begin() + cnt);
}

void PlayerProfileMan::GetMatchResult(int roleid, int match_mode, int linksid, int localsid)
{
	LOG_TRACE( "PlayerProfile GetMatchResult roleid=%d, match_mode=%d\n", roleid, match_mode);
	
	if(_state != ST_OPEN) return;
	if((match_mode < MATCH_MODE_MENTOR) || (match_mode >= MATCH_MODE_MAX)) return;
	
	PlayerInfo* pInfo = UserContainer::GetInstance().FindRoleOnline( roleid );
	if(pInfo == NULL) return;
	
	PROFILE_ENTRY_MAP::iterator it = _profiles.find(roleid);
	if(it == _profiles.end() || it->second._state != STATE_READY) return;
	
	PlayerProfileEntry& entry = it->second;
	time_t now = Timer::GetTime();
	if((now - entry._match_rqst_timestamp) < MATCH_COLDDOWN) return;
	entry._match_rqst_timestamp = now;

	CANDICATE_MAP& candicates = _candicates[match_mode];
	int key = -1;
	if(match_mode == MATCH_MODE_MENTOR) {
		key = pInfo->cls;
	} else if(match_mode == MATCH_MODE_FRIEND) {
		key = 0;
	} else if(match_mode == MATCH_MODE_LOVER) {
		key = ( (entry._gender == GENDER_MALE) ? GENDER_FEMALE : GENDER_MALE );
	}
	
	PlayerProfileGetMatchResult_Re re;
	re.roleid = roleid;
	re.localsid = localsid;
	
	CANDICATE_MAP::iterator it_role_set = candicates.find(key);
	if(it_role_set == candicates.end() || it_role_set->second.empty()) {
		GDeliveryServer::GetInstance()->Send(linksid, re);
		return;
	}
	
	ROLE_ID_SET& roles = it_role_set->second;
	unsigned int cnt = roles.size();
	if(cnt > CANDICATE_CEIL) {
		ROLE_ID_SET matchers;
		
		unsigned int offset = rand() % cnt;
		if(offset + CANDICATE_CEIL <= cnt) {
			ROLE_ID_SET::iterator it_first = roles.begin();
			ROLE_ID_SET::iterator it_last = roles.begin();	
			std::advance(it_first, offset);
			std::advance(it_last, offset + CANDICATE_CEIL);
			
			matchers.insert(it_first, it_last);
		} else {
			ROLE_ID_SET::iterator it_first = roles.begin();
			std::advance(it_first, offset);
			matchers.insert(it_first, roles.end());
			
			ROLE_ID_SET::iterator it_last = roles.begin();	
			unsigned dis = offset + CANDICATE_CEIL - cnt;
			std::advance(it_last, dis);
			matchers.insert(roles.begin(), it_last);
		}

		CalcMatchResult(roleid, entry, matchers, re.result);
	} else {
		CalcMatchResult(roleid, entry, roles, re.result);
	}

	GDeliveryServer::GetInstance()->Send(linksid, re);
}

bool PlayerProfileMan::HasMatchOptionMask(int roleid)
{
    bool has_mask = false;
    if(_state != ST_OPEN) return has_mask;

    PROFILE_ENTRY_MAP::const_iterator iter = _profiles.find(roleid);
    if (iter != _profiles.end())
    {
        has_mask = (iter->second._data.match_option_mask != MATCH_MASK_NULL);
    }

    return has_mask;
}

}
