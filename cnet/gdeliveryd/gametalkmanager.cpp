#include "gametalkmanager.h"
#include "rpcdefs.h"
#include "log.h"
#include "time.h"

#include "rolegroupupdate.hpp"
#include "rolefriendupdate.hpp"
#include "roleforbidupdate.hpp"
#include "roleinfoupdate.hpp"
#include "removerole.hpp"
#include "roleactivation.hpp"
#include "friendstatus.hpp"
#include "getmessage.hrp"
#include "rolestatusupdate.hpp"
#include "rolestatusresp.hpp"
#include "announcezoneidtoim.hpp"
#include "rolemsg.hpp"
#include "syncteams.hpp"
#include "teamcreate.hpp"
#include "teamdismiss.hpp"
#include "teammemberupdate.hpp"
#include "playerenterleavegt.hpp"

#include "gametalkclient.hpp"
#include "gdeliveryserver.hpp"
#include "gamedbclient.hpp"
#include "gproviderserver.hpp"

namespace GNET {

int RoleStatusManager::RoleGTUpdate(int roleid, char status) {
	DEBUG_PRINT("RoleStatusManager: RoleGTUpdate(%d, %x)", roleid, status);
	int ret = GT_CONFLIT_NONE;

	Thread::Mutex::Scoped l(_lockerMap);
	RSMap::iterator it = _statusMap.find(roleid);
	char old_status = it == _statusMap.end() ? GT_OFFLINE : it->second;

	//game online status inconsistent, need update
	if(old_status & GT_ONLINE_GAME != status & GT_ONLINE_GAME) {
		ret |= GT_CONFLIT_GT;
		if(old_status & GT_ONLINE_GAME) status |= GT_ONLINE_GAME;
		else status &= ~GT_ONLINE_GAME;
	}

	if(old_status != status) {
		ret |= GT_CONFLIT_GAME;
		if(status == GT_OFFLINE) _statusMap.erase(roleid);
		else _statusMap[roleid] = status;
	}
	return ret;
}

int RoleStatusManager::RoleOnline(int roleid) {
	DEBUG_PRINT("RoleStatusManager: RoleOnline(%d)", roleid);
	Thread::Mutex::Scoped l(_lockerMap);
	RSMap::iterator it = _statusMap.find(roleid);

	char old_status = it == _statusMap.end() ? GT_OFFLINE : it->second;
	if(old_status & GT_ONLINE_GAME) return GT_CONFLIT_NONE;

	_statusMap[roleid] = old_status | GT_ONLINE_GAME;
	return GT_CONFLIT_GT;
}

int RoleStatusManager::RoleOffline(int roleid) {
	DEBUG_PRINT("RoleStatusManager: RoleOffline(%d)", roleid);
	Thread::Mutex::Scoped l(_lockerMap);
	RSMap::iterator it = _statusMap.find(roleid);

	char old_status = it == _statusMap.end() ? GT_OFFLINE : it->second;
	if(!(old_status & GT_ONLINE_GAME)) return GT_CONFLIT_NONE;

	if(old_status == GT_ONLINE_GAME) _statusMap.erase(roleid);
	else _statusMap[roleid] = old_status & ~GT_ONLINE_GAME;
	return GT_CONFLIT_GT;
}

char RoleStatusManager::GetRoleStatus(int roleid) {
	DEBUG_PRINT("RoleStatusManager: GetRoleStatus(%d)", roleid);
	Thread::Mutex::Scoped l(_lockerMap);
	RSMap::iterator it = _statusMap.find(roleid);
	if(it == _statusMap.end()) return GT_OFFLINE;
	else return it->second;
}

void RoleStatusManager::Walk(RoleStatusWalker &walker) {
	DEBUG_PRINT("RoleStatusManager: Walk");
	Thread::Mutex::Scoped l(_lockerMap);
	RSMap::iterator it = _statusMap.begin();
	RSMap::iterator end = _statusMap.end();
	char newStatus;
	for(; it != end;) {
		newStatus = walker.Visit(it->first, it->second);
		if(newStatus == GT_OFFLINE) {
			_statusMap.erase(it++);
		} else {
			it->second = newStatus;
			++it;
		}
	}
}

bool RoleStatusManager::RoleGTOnline(int roleid)
{
	DEBUG_PRINT("RoleStatusManager: RoleGTOnline");
	Thread::Mutex::Scoped l(_lockerMap);
	RSMap::iterator it = _statusMap.find(roleid);
	
	if(it == _statusMap.end())
		return false;
	else
	{
		char status = it->second;
		if((status & GT_ONLINE_IM) || (status & GT_ONLINE_WEB) || (status & GT_ONLINE_INVISABLE))
		{
			return true;
		}
	}
	return false;
}
	
bool RoleStatusManager::RoleGameOnline(int roleid)
{
	DEBUG_PRINT("RoleStatusManager: RoleGameOnline");
	Thread::Mutex::Scoped l(_lockerMap);
	RSMap::iterator it = _statusMap.find(roleid);

	if(it == _statusMap.end())
		return false;
	else
	{
		char status = it->second;
		if(status & GT_ONLINE_GAME)
			return true;
	}
	return false;
}
namespace {

	class GTStatusClearWalker : public RoleStatusManager::RoleStatusWalker {
	public:
		char Visit(int roleid, char status) {
			if(status & RoleStatusManager::GT_ONLINE_GAME) return RoleStatusManager::GT_ONLINE_GAME;
			else return RoleStatusManager::GT_OFFLINE;
		}
	};

	class GameStatusWalker : public RoleStatusManager::RoleStatusWalker {
	public:
		std::vector<int> roles;
		char Visit(int roleid, char status) {
			if(status & RoleStatusManager::GT_ONLINE_GAME) {
				roles.push_back(roleid);
			}
			return status;
		}
	};
	
}

void TeamManager::Walk(TeamWalker &walker)
{
	DEBUG_PRINT("RoleStatusManager: Walk");
	TeamInfo::iterator it = _teamMap.begin();
	for(;it!=_teamMap.end();it++)
	{
		walker.Visit(it->second);
	}
}


void GameTalkManager::_SendAllGameStatus() {
	GameStatusWalker walker;
	TeamWalker t_walker;
	_statusMan.Walk(walker);
	_teamMan.Walk(t_walker);
	if(_connectedGT) {
		if(!_SendStatusResp(walker.roles)) {
			Log::log(LOG_ERR, "GameTalkManager: GTRoleRequest send RoleStatusResp failed.");
		}
		if(!_SendTeamInfo(t_walker.teams)){
			Log::log(LOG_ERR, "GameTalkManager: SynTeam failed.");
		}
	}
}

void GameTalkManager::_ClearAllGTStatus() {
	GTStatusClearWalker walker;
	_statusMan.Walk(walker);
}

void GameTalkManager::_SendOrBuffer(const Protocol &p) {
	if(!_connectedGT || !GameTalkClient::GetInstance()->SendProtocol(p)) {
		_buffer.Put(p);
	} else {
		FlushBuffer();
	}
}

void GameTalkManager::_SendStatusToFriends(int roleid, const std::vector<int64_t> &friends) {
	GDeliveryServer* dsm = GDeliveryServer::GetInstance();
	FriendStatus stat(roleid, GetRoleStatus(roleid), 0);
	std::vector<int64_t>::const_iterator it = friends.begin();
	std::vector<int64_t>::const_iterator end = friends.end();
	for(; it != end; ++it) {
		PlayerInfo * finfo = UserContainer::GetInstance().FindRoleOnline((int)(*it));
		if(finfo) {
			GFriendInfoVector::iterator fit = finfo->friends.begin();
			GFriendInfoVector::iterator fend = finfo->friends.end();
			for(; fit != fend; ++fit) {
				if(fit->rid == roleid) {
					stat.localsid = finfo->localsid;
					dsm->Send(finfo->linksid,stat);
					break;
				}
			}
		}
	}
}

void GameTalkManager::_SendOfflineMessages(int roleid) {
	if(_offMsgMan.HasOfflineMessage(roleid)) {
		GetMessage* rpc = (GetMessage*) Rpc::Call(RPC_GETMESSAGE,RoleId(roleid));
		rpc->toGT = true;
		GameDBClient::GetInstance()->SendProtocol(rpc);
	}
}

bool GameTalkManager::_SendStatusUpdate(int roleid, char status) {
	DEBUG_PRINT("GameTalkManager: _SendStatusUpdate(%d, %x)", roleid, status);
	RoleStatusUpdate msg;
	msg.roleid = (int64_t)roleid;
	msg.status.status = status;
	return GameTalkClient::GetInstance()->SendProtocol(msg);
}

bool GameTalkManager::_SendStatusResp(const std::vector<int> &roles, int64_t localrid) {
	DEBUG_PRINT("GameTalkManager: _SendStatusResp(%d)", roles.size());
	RoleStatusResp msg;
	msg.localrid = localrid;
	std::vector<int>::const_iterator it = roles.begin();
	std::vector<int>::const_iterator end = roles.end();
	for(; it != end; ++it) 
		msg.rolestatus[(int64_t)(*it)] = RoleStatusManager::GT_ONLINE_GAME;
	return GameTalkClient::GetInstance()->SendProtocol(msg);
}

bool GameTalkManager::_SendAnnounce(int aid, int zoneid, time_t boottime) {
	DEBUG_PRINT("GameTalkManager: _SendAnnounce(%d, %d, %s)", aid, zoneid, ctime(&boottime));
	AnnounceZoneidToIM msg;
	msg.aid = aid;
	msg.zoneid = zoneid;
	msg.boottime = (int64_t)boottime;
	return GameTalkClient::GetInstance()->SendProtocol(msg);
}

bool GameTalkManager::_SendTeamInfo(TeamBeanVector teams)
{
	DEBUG_PRINT("GameTalkManager: _SendTeamInfo(%d)", teams.size());
	SyncTeams msg(teams);
	return GameTalkClient::GetInstance()->SendProtocol(msg);
}

void GameTalkManager::FlushBuffer() {
	Protocol *p;
	int cnt = COUNT_PER_FLUSH;
	while(!_buffer.Empty() && cnt--) {
		p = _buffer.Peek();
		if(!_connectedGT || !GameTalkClient::GetInstance()->SendProtocol(p))
			break;
		_buffer.Pop();
	}
}

void GameTalkManager::GTRoleUpdate(int roleid, char status, const std::vector<int64_t> &friends) {
	DEBUG_PRINT("GameTalkManager: GTRoleUpdate(%d, %x)", roleid, status);
	int res = _statusMan.RoleGTUpdate(roleid, status);
	if(res & RoleStatusManager::GT_CONFLIT_GAME) {
		_SendStatusToFriends(roleid, friends);
		char newStatus = GetRoleStatus(roleid);
		//role is online in somewhere but not game
		if(newStatus != RoleStatusManager::GT_OFFLINE && !(newStatus & RoleStatusManager::GT_ONLINE_GAME))
			_SendOfflineMessages(roleid);
	} else if(res & RoleStatusManager::GT_CONFLIT_GT) {
		if(_connectedGT) {
			if(!_SendStatusUpdate(roleid, GetRoleStatus(roleid))) {
				Log::log(LOG_ERR, "GameTalkManager: GTRoleRequest send RoleStatusUpdate failed.");
			}
		}
	}
}

void GameTalkManager::GTRoleResponse(const std::map<int64_t, RoleStatusBean> &map) {
	DEBUG_PRINT("GameTalkManager: GTRoleResponse(%d)", map.size());
	std::map<int64_t, RoleStatusBean>::const_iterator it = map.begin();
	std::map<int64_t, RoleStatusBean>::const_iterator end = map.end();
	for(; it != end; ++it) {
		if(_statusMan.RoleGTUpdate(it->first, it->second.status) & RoleStatusManager::GT_CONFLIT_GAME) {
			//_SendStatusToFriends(it->first);
		}
	}
}

void GameTalkManager::GTRoleRequest(const std::vector<int64_t> &roles, int64_t localrid) {
	DEBUG_PRINT("GameTalkManager: GTRoleRequest(%d)", roles.size());
	std::vector<int> rolesOnline;
	std::vector<int64_t>::const_iterator it = roles.begin();
	std::vector<int64_t>::const_iterator end = roles.end();
	for(; it != end; ++it) {
		if(GetRoleStatus((int)(*it)) & RoleStatusManager::GT_ONLINE_GAME)
			rolesOnline.push_back((int)(*it));
	}
	if(_connectedGT) {
		if(!_SendStatusResp(rolesOnline)) {
			Log::log(LOG_ERR, "GameTalkManager: GTRoleRequest send RoleStatusResp failed.");
		}
	}
}

void GameTalkManager::GameRoleOnline(int roleid) {
	DEBUG_PRINT("GameTalkManager: GTRoleOnline(%d)", roleid);
	int res = _statusMan.RoleOnline(roleid);
	if(res & RoleStatusManager::GT_CONFLIT_GT) {
		if(_connectedGT) {
			if(!_SendStatusUpdate(roleid, GetRoleStatus(roleid))) {
				Log::log(LOG_ERR, "GameTalkManager: GTRoleRequest send RoleStatusUpdate failed.");
			}
		}
	}
}

void GameTalkManager::GameRoleOffline(int roleid) {
	DEBUG_PRINT("GameTalkManager: GTRoleOffline(%d)", roleid);
	int res = _statusMan.RoleOffline(roleid);
	if(res & RoleStatusManager::GT_CONFLIT_GT) {
		if(_connectedGT) {
			if(!_SendStatusUpdate(roleid, GetRoleStatus(roleid))) {
				Log::log(LOG_ERR, "GameTalkManager: GTRoleRequest send RoleStatusUpdate failed.");
			}
		}
	}
}

void GameTalkManager::OnStartUp(int aid, int zoneid) {
	DEBUG_PRINT("GameTalkManager: OnStartUp(aid, zoneid)", aid, zoneid);
	_aid = aid;
	_zoneid = zoneid;
	_bootTimeGame = time(NULL);
}

void GameTalkManager::OnConnected() {
	DEBUG_PRINT("GameTalkManager: OnConnected");
	if(!_SendAnnounce(_aid, _zoneid, _bootTimeGame)) {
		Log::log(LOG_ERR, "GameTalkManager: GTRoleRequest send RoleStatusUpdate failed.");
	}
}

void GameTalkManager::OnIdentification(time_t boottime) {
	DEBUG_PRINT("GameTalkManager: OnIdentification(%s)", ctime(&boottime));
	_connectedGT = true;
	//reconnect and gt restart, clear gt online status
	if(_bootTimeGT != 0 && _bootTimeGT != boottime) {
		_ClearAllGTStatus();
	}
	_bootTimeGT = boottime;
	_SendAllGameStatus();
	FlushBuffer();
}

void GameTalkManager::OnDisconnected() {
	DEBUG_PRINT("GameTalkManager: OnDisconnected");
	_connectedGT = false;
}

void GameTalkManager::NotifyUpdateFriend(int roleid, int operation, int frid, int groupid, int gtype, const Octets &name) {
	DEBUG_PRINT("GameTalkManager: NotifyUpdateFriend(%d)", roleid);
	RoleFriendUpdate update;
	update.roleid = (int64_t)roleid;
	update.rolefriend.info.roleid = (int64_t)frid;
	update.operation = operation;
	update.gtype = gtype;
	update.groupid = (int64_t)groupid;
	update.rolefriend.info.rolename = name;
	update.rolefriend.status.status = GetRoleStatus(frid);

	_SendOrBuffer(update);
}

void GameTalkManager::NotifyUpdateGroup(int roleid, int operation, int groupid, int gtype, const Octets &name) {
	DEBUG_PRINT("GameTalkManager: NotifyUpdateGroup(%d)", roleid);
	RoleGroupUpdate update;
	update.roleid = (int64_t)roleid;
	update.gtype = gtype;
	update.operation = operation;
	update.groupid = groupid;
	update.groupname = name;

	_SendOrBuffer(update);
}

void GameTalkManager::NotifyUpdateRole(int roleid, int level) {
	DEBUG_PRINT("GameTalkManager: NotifyUpdateRole(%d, %d)", roleid, level);
	RoleInfoUpdate update;
	update.updateflag = GT_UPDATE_LEVEL;
	update.roleinfo.roleid = (int64_t)roleid;
	update.roleinfo.level = level;

	_SendOrBuffer(update);
}

void GameTalkManager::NotifyUpdateRole(int roleid, Octets &name) {
	DEBUG_PRINT("GameTalkManager: NotifyUpdateRole(%d)", roleid);
	RoleInfoUpdate update;
	update.updateflag = GT_UPDATE_NAME;
	update.roleinfo.roleid = (int64_t)roleid;
	update.roleinfo.rolename = name;

	_SendOrBuffer(update);
}

void GameTalkManager::NotifyCreateRole(int userid, int roleid, Octets &name, char gender, int occupation) {
	DEBUG_PRINT("GameTalkManager: NotifyCreateRole(%d, %d)", userid, roleid);
	RoleInfoUpdate update;
	update.updateflag = GT_UPDATE_NAME | GT_UPDATE_GENDER | GT_UPDATE_RACE | GT_UPDATE_LEVEL | GT_UPDATE_EXT;
	update.roleinfo.roleid = (int64_t)roleid;
	update.roleinfo.rolename.swap(name);
	update.roleinfo.gender = gender;
	update.roleinfo.occupation = occupation;
	update.roleinfo.level = 1;
	Marshal::OctetsStream os;
	os << (int64_t)userid;
	update.roleinfo.extinfo.swap(os);

	_SendOrBuffer(update);
}

void GameTalkManager::NotifyUndoRemoveRole(int roleid) {
	DEBUG_PRINT("GameTalkManager: NotifyUndoRemoveRole(%d)", roleid);
	RoleForbidUpdate forbid;
	forbid.forbid[roleid].endtime[GT_FORBID_PREDELETE] = 0;

	_SendOrBuffer(forbid);
}

void GameTalkManager::NotifyPreRemoveRole(int roleid) {
	DEBUG_PRINT("GameTalkManager: NotifyPreRemoveRole(%d)", roleid);
	RoleForbidUpdate forbid;
	forbid.forbid[roleid].endtime[GT_FORBID_PREDELETE] = -1;

	_SendOrBuffer(forbid);
}

void GameTalkManager::NotifyRemoveRole(int userid, int roleid) {
	DEBUG_PRINT("GameTalkManager: NotifyRemoveRole(%d, %d)", userid, roleid);
	RemoveRole remove;
	remove.userid = (int64_t)userid;
	remove.roleid = (int64_t)roleid;
	_SendOrBuffer(remove);
}

void GameTalkManager::NotifyUpdate(const Protocol& p) {
	_SendOrBuffer(p);
}

void GameTalkManager::NotifyTeamCreate(int64_t team_uid,int captain,std::vector<int> members)
{
	DEBUG_PRINT("GameTalkManager: NotifyTeamCreate");
	TeamBean team;
	team.teamid = team_uid; 
	team.captain = captain;
	std::vector<int>::iterator it= members.begin();
	for(;it!=members.end();it++)
	{
		team.members.push_back((int64_t)(*it));
	}
	_teamMan.AddTeam(team);
	TeamCreate tc;
	tc.team = team;
	if(_connectedGT)
	{
		if(!GameTalkClient::GetInstance()->SendProtocol(tc))
		{
			Log::log(LOG_ERR,"GameTalkManager,NotifyTeamCreate failed");
		}
	}
	return;
}

void GameTalkManager::NotifyTeamDismiss(int64_t team_uid)
{
	DEBUG_PRINT("GameTalkManager: NotifyTeamDissmiss");
	_teamMan.DismissTeam(team_uid);
	TeamDismiss td;
	td.teamid = team_uid;
	if(_connectedGT)
	{
		if(!GameTalkClient::GetInstance()->SendProtocol(td))
		{
			Log::log(LOG_ERR,"GameTalkManager,NotifyTeamDismiss failed");
		}
	}
	return;
}

void GameTalkManager::NotifyTeamMemberOp(int64_t team_uid,const char op,const int member)
{
	DEBUG_PRINT("GameTalkManager: NotifyTeamMemberOp(%d,%d)", op,member);
	bool send = false;
	switch(op)
	{
	case GT_TEAM_ADD:
		if(_teamMan.AddMember(team_uid,member))
		{
			send = true;	
		}
		break;
	case GT_TEAM_DEL:
		if(_teamMan.RemoveMember(team_uid,member))
		{
			send = true;
		}
		break;
	case GT_TEAM_CAP:
		if(_teamMan.ChangeLeader(team_uid,member))
		{
			send = true;
		}
		break;
	default:
		break;
	}
	if(!send)
		return;
	TeamMemberUpdate tmu;
	tmu.teamid = team_uid;
	tmu.operation = op;
	tmu.members.push_back((int64_t) member);
	if(_connectedGT)
	{
		if(!GameTalkClient::GetInstance()->SendProtocol(tmu))
		{
			Log::log(LOG_ERR,"GameTalkManager,TeamMemberUpdate failed");
		}
	}

	return;
}

void GameTalkManager::SendActiveRole(int roleid, char operation) {
	DEBUG_PRINT("GameTalkManager: SendActiveRole(%d)", roleid);
	RoleActivation active;
	active.roleid = (int64_t)roleid;
	active.operation = operation;
	if(_connectedGT && !GameTalkClient::GetInstance()->SendProtocol(active))
		DEBUG_PRINT("GameTalkManager send RoleActivation failed.");
}


bool GameTalkManager::SendRoleMsg(int receiver, int sender, const Octets &senderName, const Octets &content, unsigned char emotion) {
	DEBUG_PRINT("GameTalkManager: _SendRoleMsg(%d, %d)", receiver, sender);
	RoleMsg msg;
	msg.receiver = receiver;
	msg.message.sender = sender;
	msg.message.sendername = senderName;
	msg.message.content = content;
	msg.message.emotiongroup = emotion;
	return _connectedGT && GameTalkClient::GetInstance()->SendProtocol(msg);
}

bool GameTalkManager::RoleGameOnline(int roleid)
{
	DEBUG_PRINT("GameTalkManager: RoleGameOnline(%d)",roleid);
	return _statusMan.RoleGameOnline(roleid);
}

bool GameTalkManager::RoleGTOnline(int roleid)
{
	DEBUG_PRINT("GameTalkManager: RoleGTOnline(%d)",roleid);
	return _statusMan.RoleGTOnline(roleid);
}

void GameTalkManager::SendPlayerEnterLeaveGT(PlayerInfo *pinfo,int op)
{
	DEBUG_PRINT("GameTalkManager: SendPlayerEnterLeaveGT(%d)",pinfo->roleid);
	PlayerEnterLeaveGT msg;
	msg.roleid = pinfo->roleid;
	msg.operation = op;
	GProviderServer::GetInstance()->DispatchProtocol(pinfo->user->gameid,msg); 
}

};
