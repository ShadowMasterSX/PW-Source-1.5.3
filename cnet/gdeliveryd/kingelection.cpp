#include "kingelection.h"
#include "gamedbclient.hpp"
#include "gproviderserver.hpp"

#include "dbkeload.hrp"
#include "dbkedeleteking.hrp"
#include "dbkedeletecandidate.hrp"
#include "dbkecandidateconfirm.hrp"
#include "dbkekingconfirm.hrp"
#include "dbkecandidateapply.hrp"
#include "dbkevoting.hrp"
#include "kekingnotify.hpp"
#include "maplinkserver.h"
#include "chatbroadcast.hpp"


namespace GNET
{

class CmpCandidate
{
public:
	bool operator()(const KECandidate & a, const KECandidate & b)
	{
		return a.deposit > b.deposit || a.deposit == b.deposit && a.serial_num < b.serial_num;
	}
};

class CmpCandidate2
{
public:
	bool operator()(const KECandidate & a, const KECandidate & b)
	{
		return a.votes > b.votes 
			|| a.votes == b.votes && a.deposit > b.deposit 
			|| a.votes == b.votes && a.deposit == b.deposit && a.serial_num < b.serial_num;
	}
};

bool KingElection::Initialize()
{
	_status = ST_INIT;
	IntervalTimer::Attach( this,KINGELECTION_UPDATE_INTERVAL/IntervalTimer::Resolution() );
	return true;
}

void KingElection::OnDBConnect(Protocol::Manager * manager, int sid)
{
	if(_status == ST_INIT)
		manager->Send(sid,Rpc::Call(RPC_DBKELOAD,DBKELoadArg()));
}

void KingElection::OnDBLoad(KingElectionDetail & detail)
{
	if(_status == ST_INIT)
	{
		_king = detail.king;
		_candidate_list.swap(detail.candidate_list);

		for(size_t i=0; i<_candidate_list.size(); i++)
		{
			if(_candidate_list[i].serial_num > _max_serial_num) _max_serial_num = _candidate_list[i].serial_num;
		}
	
		time_t cur_time = UpdateTime();
		int weekoffset = cur_time - _week_begin;

		if(weekoffset >= CANDIDATE_APPLY_BEGIN_TIME && weekoffset < CANDIDATE_APPLY_END_TIME) _status = ST_CANDIDATE_APPLY;
		else if(weekoffset >= CANDIDATE_APPLY_END_TIME && weekoffset < VOTE_BEGIN_TIME)	_status = ST_WAIT_VOTE;
		else if(weekoffset >= VOTE_BEGIN_TIME && weekoffset < VOTE_END_TIME) _status = ST_VOTE;
		else _status = ST_OPEN;
		if(_status != ST_OPEN) Log::log(LOG_WARNING, "kingelection startup, status=%s", Status2Str(_status));

		if(_king.roleid > 0)
		{
			if(cur_time >= _king.end_time || _status != ST_OPEN)
			{
				int reason = cur_time > _king.end_time ? DELKING_REASON_EXPIRE : DELKING_REASON_ERROR;
				Log::log(LOG_WARNING, "kingelection startup, status=%s, delete king, kingid=%d reason=%d", Status2Str(_status), _king.roleid, reason);
				__DeleteKing(reason);
			}
		}
		if(_candidate_list.size())
		{
			if(_status == ST_OPEN)
			{
				bool recover_king = false; 
				if(weekoffset >= VOTE_END_TIME)
				{
					for(size_t i=0; i<_candidate_list.size(); i++)
					{
						if(_candidate_list[i].votes > 0)
						{
							recover_king = true;
							break;
						}
					}
				}
				if(recover_king)
				{
					__GenerateKing();
					Log::log(LOG_WARNING, "kingelection startup, status=%s, generate king, kingid=%d", Status2Str(_status), _king.roleid);
				}
				else
				{
					Log::log(LOG_WARNING, "kingelection startup, status=%s, delete candidate, count=%d", Status2Str(_status), _candidate_list.size());
					__DeleteCandidate(DELCANDIDATE_REASON_ERROR);
				}
			}
			else if(_status == ST_WAIT_VOTE || _status == ST_VOTE) 
			{
				if(_candidate_list.size() > CANDIDATE_MAX)
				{
					Log::log(LOG_WARNING, "kingelection startup, status=%s, generate candidate, count=%d", Status2Str(_status), _candidate_list.size());
					__GenerateCandidate();
				}
			}
		}
	}
}

bool KingElection::Update()
{
	if(!IsOpen()) return true;

	time_t cur_time = UpdateTime();
	int weekoffset = cur_time - _week_begin;
	
	if(_king.roleid > 0 && cur_time >= _king.end_time)
	{
		__DeleteKing(DELKING_REASON_EXPIRE);
	}
	
	if(_status == ST_OPEN)
	{
		if(weekoffset - weekoffset%60 == 3*86400 + 20*3600)
		{
			//周三8点公告提醒
			BroadCastMsg(CMSG_KE_CANDIDATE_APPLY_ANNOUNCE);
		}
		if(weekoffset >= CANDIDATE_APPLY_BEGIN_TIME && weekoffset < CANDIDATE_APPLY_END_TIME)
		{
			_status = ST_CANDIDATE_APPLY;
			BroadCastMsg(CMSG_KE_CANDIDATE_APPLY_START);
			if(_king.roleid > 0)
			{
				Log::log(LOG_WARNING, "kingelection update, status=%s, delete king, kingid=%d reason=%d", Status2Str(_status), _king.roleid, DELKING_REASON_ERROR);
				__DeleteKing(DELKING_REASON_ERROR);
			}
		}		
	}
	else if(_status == ST_CANDIDATE_APPLY)
	{
		if(weekoffset >= CANDIDATE_APPLY_END_TIME && weekoffset < VOTE_BEGIN_TIME)
		{
			__GenerateCandidate();
			_status = ST_WAIT_VOTE;
			Marshal::OctetsStream os;
			os << _candidate_list.size();
			for(size_t i=0; i<_candidate_list.size(); i++)
				os << _candidate_list[i].roleid;
			BroadCastMsg(CMSG_KE_CANDIDATE_APPLY_END, os);
		}
	}
	else if(_status == ST_WAIT_VOTE)
	{
		if(weekoffset - weekoffset%60 == 3*86400 + 21*3600 || weekoffset - weekoffset%60 == 3*86400 + 21*3600 + 30*60)
		{
			//周三9点 9点30 公告提醒
			BroadCastMsg(CMSG_KE_VOTE_ANNOUNCE);
		}
		if(weekoffset >= VOTE_BEGIN_TIME && weekoffset < VOTE_END_TIME)
		{
			_status = ST_VOTE;
			BroadCastMsg(CMSG_KE_VOTE_START);
		}
	}
	else if(_status == ST_VOTE)
	{
		if((weekoffset - weekoffset%60) % 300 == 0)
		{
			if(_candidate_list.size())
			{
				std::sort(_candidate_list.begin(), _candidate_list.end(), CmpCandidate2());
				BroadCastMsg(CMSG_KE_CANDIDATE_VOTES_TOP, Marshal::OctetsStream()<<_candidate_list[0].roleid<<_candidate_list[0].votes);
			}
		}
		if(weekoffset >= VOTE_END_TIME)
		{
			__GenerateKing();
			_status = ST_OPEN;	
			BroadCastMsg(CMSG_KE_VOTE_END, Marshal::OctetsStream()<<_king.roleid);
		}	
	}
	return true;
}

const char * KingElection::Status2Str(int status)
{
	static const char * status_str[] = {"close", "init", "open", "candidate apply", "wait vote", "vote"};
	static const char * unkonw_str = "unknown";
	if(status < 0 || (size_t)status >= sizeof(status_str)/sizeof(const char *)) return unkonw_str;
	else return status_str[status];
}

time_t KingElection::UpdateTime()
{
	time_t cur_time = Timer::GetTime();
	if(cur_time - _week_begin >= 86400*7)
	{
		struct tm * tm1 = localtime(&cur_time);
		_week_begin = cur_time - tm1->tm_wday * 86400 - tm1->tm_hour * 3600 - tm1->tm_min * 60 - tm1->tm_sec;
	}
	return cur_time;
}

void KingElection::BroadCastMsg(int id, const Octets & msg)
{
	ChatBroadCast cbc;
	cbc.channel = GN_CHAT_CHANNEL_SYSTEM;
	cbc.srcroleid = id;
	cbc.msg = msg;
	LinkServer::GetInstance().BroadcastProtocol(cbc);
}

void KingElection::GetStatus(int & status, KEKing & king, KECandidateVector & candidate_list)
{
	status = _status;
	if(_status == ST_OPEN)
	{
		king = _king;
	}
	if(_status == ST_WAIT_VOTE || _status == ST_VOTE)
	{
		candidate_list = _candidate_list;
	}
}

int KingElection::TryCandidateApply(int roleid, int item_id, int item_num, const PlayerInfo & pinfo, const GMailSyncData & sync)
{
	if(_status != ST_CANDIDATE_APPLY) return ERR_KE_CANNOT_APPLY_CANDIDATE;
	int weekoffset = Timer::GetTime() - _week_begin;
	if(weekoffset >= CANDIDATE_APPLY_END_TIME - 60) return ERR_KE_CANNOT_APPLY_CANDIDATE;
	
	if(_candidate_list.size() + _candidate_apply_half >= CANDIDATE_APPLY_MAX) return ERR_KE_CANDIDATE_APPLY_EXCEED_UPPER_LIMIT;
	for(size_t i=0; i<_candidate_list.size(); i++)
	{
		if(_candidate_list[i].roleid == roleid) return ERR_KE_ALREADY_APPLY_CANDIDATE;
	}

	++ _candidate_apply_half;
	DBKECandidateApply * rpc = (DBKECandidateApply *)Rpc::Call(
			RPC_DBKECANDIDATEAPPLY,
			DBKECandidateApplyArg(
				roleid,
				++_max_serial_num,
				item_id,
				item_num,
				sync
				)
			);
	rpc->save_linksid = pinfo.linksid;
	rpc->save_localsid = pinfo.localsid;
	rpc->save_gsid = pinfo.gameid;
	GameDBClient::GetInstance()->SendProtocol(rpc);
	return ERR_SUCCESS;
}

int KingElection::TryVote(int roleid, int item_id, int item_pos, int item_num, int candidate_roleid, const PlayerInfo & pinfo, const GMailSyncData & sync)
{
	if(_status != ST_VOTE) return ERR_KE_CANNOT_VOTE;
	int weekoffset = Timer::GetTime() - _week_begin;
	if(weekoffset >= VOTE_END_TIME - 60) return ERR_KE_CANNOT_VOTE;

	size_t i = 0;
	for( ; i<_candidate_list.size(); i++)
	{
		if(candidate_roleid == _candidate_list[i].roleid) break;	
	}
	if(i == _candidate_list.size()) return ERR_KE_CANDIDATE_NOT_EXIST;

	DBKEVoting * rpc = (DBKEVoting *)Rpc::Call(
			RPC_DBKEVOTING,
			DBKEVotingArg(
					roleid,
					item_id,
					item_pos,
					item_num,
					candidate_roleid,
					sync
				)			
			);

	rpc->save_linksid = pinfo.linksid;  
	rpc->save_localsid = pinfo.localsid; 
	rpc->save_gsid = pinfo.gameid;
	GameDBClient::GetInstance()->SendProtocol(rpc);
	return ERR_SUCCESS;
}

bool KingElection::AddCandidate(int roleid, int serial_num, int deposit)
{
	if(_status != ST_CANDIDATE_APPLY) return false;
	for(size_t i=0; i<_candidate_list.size(); i++)
	{
		if(_candidate_list[i].roleid == roleid) return false;
	}
	_candidate_list.push_back(KECandidate(roleid,serial_num,deposit,0));
	return true;
}

void KingElection::DecCandidateApplyHalf()
{
	if(_status != ST_CANDIDATE_APPLY) return;
	-- _candidate_apply_half;	
}

bool KingElection::AddVote(int candidate_roleid)
{
	if(_status != ST_VOTE) return false;
	size_t i = 0;
	for( ; i<_candidate_list.size(); i++)
	{
		if(candidate_roleid == _candidate_list[i].roleid) break;	
	}
	if(i == _candidate_list.size()) return false;
	++ _candidate_list[i].votes;
	if(_candidate_list[i].votes % 100 == 0)
	{
		BroadCastMsg(CMSG_KE_CANDIDATE_VOTES_100, Marshal::OctetsStream()<<_candidate_list[i].roleid<<_candidate_list[i].votes);
	}
	return true;
}

void KingElection::OnLogin(const PlayerInfo & pinfo)
{
	if(!IsOpen()) return;
	if(_king.roleid != pinfo.roleid) return;

	class KingNotifyTask : public Thread::Runnable
	{
		int _roleid;
		int _end_time;
		int _gs_id;
	public:
		KingNotifyTask(int roleid, int end_time, int gs_id) : _roleid(roleid), _end_time(end_time),_gs_id(gs_id){}
		void Run()
		{
			GProviderServer::GetInstance()->DispatchProtocol(_gs_id, KEKingNotify(_roleid, _end_time));
			delete this;
		}
	};
	//等待玩家完成enterworld流程，所以延迟5秒通知gs
	Thread::HouseKeeper::AddTimerTask(new KingNotifyTask(_king.roleid, _king.end_time, pinfo.gameid), 5);
}

void KingElection::__DeleteKing(int reason)
{
	GameDBClient::GetInstance()->SendProtocol(Rpc::Call(RPC_DBKEDELETEKING,DBKEDeleteKingArg(_king.roleid, reason)));
	_king = KEKing();
}

void KingElection::__DeleteCandidate(int reason)
{
	if(_candidate_list.size() == 0) return;
	
	IntVector list;
	for(size_t i=0; i<_candidate_list.size(); i++)
		list.push_back(_candidate_list[i].roleid);
	GameDBClient::GetInstance()->SendProtocol(Rpc::Call(RPC_DBKEDELETECANDIDATE,DBKEDeleteCandidateArg(list, reason)));
	
	_candidate_list.clear();
}

void KingElection::__GenerateCandidate()
{
	if(_candidate_list.size() <= CANDIDATE_MAX) return;
	
	std::sort(_candidate_list.begin(), _candidate_list.end(), CmpCandidate());
	IntVector list;
	for(size_t i=0; i<_candidate_list.size(); i++)
	{
		if(i<CANDIDATE_MAX)
			list.push_back(_candidate_list[i].roleid);
	}
	GameDBClient::GetInstance()->SendProtocol(Rpc::Call(RPC_DBKECANDIDATECONFIRM,DBKECandidateConfirmArg(list)));
	
	_candidate_list.erase(_candidate_list.begin() + CANDIDATE_MAX, _candidate_list.end());
}

void KingElection::__GenerateKing()
{
	if(_candidate_list.size() == 0) return;
	std::sort(_candidate_list.begin(), _candidate_list.end(), CmpCandidate2());
	_king.roleid = _candidate_list[0].roleid;
	_king.end_time = _week_begin + 86400*7 + KING_END_TIME;
	_candidate_list.clear();
	GameDBClient::GetInstance()->SendProtocol(Rpc::Call(RPC_DBKEKINGCONFIRM,DBKEKingConfirmArg(_king)));
}

}
