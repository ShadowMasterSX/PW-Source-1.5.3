#ifndef __GNET_KINGELECTION_H__
#define __GNET_KINGELECTION_H__

#include "mapuser.h"
#include "gmailsyncdata"
#include "kecandidate"
#include "keking"
#include "kingelectiondetail"

#define KINGELECTION_UPDATE_INTERVAL	60000000
#define CANDIDATE_APPLY_MAX		1000	//增加此值需要调整协议maxsize
#define CANDIDATE_MAX			3		//增加此值需要调整协议maxsize
namespace GNET
{

class KingElection : public IntervalTimer::Observer
{

public:
	enum
	{
		ST_CLOSE = 0,
		ST_INIT,
		ST_OPEN,
		ST_CANDIDATE_APPLY,
		ST_WAIT_VOTE,
		ST_VOTE,
	};
	enum
	{
		CANDIDATE_APPLY_BEGIN_TIME 	= 3*86400 + 20*3600 + 30*60,	//周三20:30开始候选人申请
		CANDIDATE_APPLY_END_TIME 	= 3*86400 + 21*3600,			//周三21:00结束候选人申请
		VOTE_BEGIN_TIME 			= 3*86400 + 22*3600,			//周三22:00开始选举
		VOTE_END_TIME 				= 4*86400 + 22*3600,			//周四22:00结束选举

		KING_END_TIME				= 3*86400 + 20*3600,			//国王任期至下周三20:00
	};
	enum{ DELKING_REASON_EXPIRE = 0,	DELKING_REASON_ERROR = 1, };
	enum{ DELCANDIDATE_REASON_ERROR = 1, };


public:
	static KingElection & GetInstance(){ static KingElection instance; return instance; }
	bool Initialize();
	void OnDBConnect(Protocol::Manager * manager, int sid);
	void OnDBLoad(KingElectionDetail & detail);
	bool Update();
	
	inline bool IsOpen(){ return _status != ST_CLOSE && _status != ST_INIT; }
	const char * Status2Str(int status);
	time_t UpdateTime(); 
	void BroadCastMsg(int id, const Octets & msg = Octets());

	void GetStatus(int & status, KEKing & king, KECandidateVector & candidate_list);
	int TryCandidateApply(int roleid, int item_id, int item_num, const PlayerInfo & pinfo, const GMailSyncData & sync);
	int TryVote(int roleid, int item_id, int item_pos, int item_num, int candidate_roleid, const PlayerInfo & pinfo, const GMailSyncData & sync);

	bool AddCandidate(int roleid, int serial_num, int deposit);
	void DecCandidateApplyHalf(); 
	bool AddVote(int candidate_roleid);
	void OnLogin(const PlayerInfo & pinfo);

private:
	void __DeleteKing(int reason);
	void __DeleteCandidate(int reason);
	void __GenerateCandidate();
	void __GenerateKing();

private:
	KingElection() : _status(ST_CLOSE),_week_begin(0),_max_serial_num(0),_candidate_apply_half(0){}

	int 				_status;
	int					_week_begin;
	int					_max_serial_num;
	
	int					_candidate_apply_half;
	KEKing 				_king;
	KECandidateVector 	_candidate_list;

	
};

}

#endif
