#ifndef __GAMETALK_MANAGER_H__
#define __GAMETALK_MANAGER_H__

#include "thread.h"
#include <map>
#include <set>
#include <vector>
#include <list>
#include "gametalkdefs.h"
#include "rolestatusbean"
#include "teambean"
#include "protocol.h"
#include "mapuser.h"

namespace GNET {

enum {
	MAX_BUFFER_COUNT = 10000,
	COUNT_PER_FLUSH = 5
};

class RoleStatusManager {
	typedef std::map<int, char> RSMap; 

	RSMap _statusMap;		//map roleid to status mask
	Thread::Mutex _lockerMap;	//lock of _statusMap
public:
	enum {
		GT_OFFLINE = 0,
		GT_ONLINE_GAME = 1,
		GT_ONLINE_IM = 2,
		GT_ONLINE_WEB = 4,
		GT_ONLINE_INVISABLE = 128
	};

	enum {
		GT_CONFLIT_NONE = 0,
		GT_CONFLIT_GT = 1,
		GT_CONFLIT_GAME = 2
	};

	class RoleStatusWalker {
	public:
		virtual char Visit(int roleid, char status) = 0;
		virtual ~RoleStatusWalker() {};
	};
	
	RoleStatusManager() : _lockerMap("RoleStatusManager::_lockerMap") {}

	int RoleGTUpdate(int roleid, char status);
	int RoleOnline(int roleid);
	int RoleOffline(int roleid);
	char GetRoleStatus(int roleid);
	void Walk(RoleStatusWalker &walker);
	bool RoleGTOnline(int roleid);
	bool RoleGameOnline(int roleid);
};

class ProtocolBuffer {
	typedef std::list<Protocol *> BufferType;
	BufferType _buffer;
	Thread::Mutex _lockerBuffer;
public:
	ProtocolBuffer() : _lockerBuffer("ProtocolBuffer::_lockerBuffer") {}

	void Put(const Protocol &p) { 
		if(_buffer.size() >= MAX_BUFFER_COUNT) return;

		Thread::Mutex::Scoped l(_lockerBuffer);
		_buffer.push_back(p.Clone());
	}

	void Pop() {
		Thread::Mutex::Scoped l(_lockerBuffer);
		_buffer.front()->Destroy();
		_buffer.pop_front();
	}

	bool Empty() {
		Thread::Mutex::Scoped l(_lockerBuffer);
		return _buffer.empty();
	}

	Protocol *Peek() {
		Thread::Mutex::Scoped l(_lockerBuffer);
		return _buffer.front();
	}

	void Clear() {
		Thread::Mutex::Scoped l(_lockerBuffer);
		BufferType::iterator it = _buffer.begin();
		BufferType::iterator end = _buffer.end();
		for(; it != end; ++it)
			(*it)->Destroy();
		_buffer.clear();
	}
};

class OfflineMessageManager {
	std::set<int> _set;
	Thread::Mutex _lockerSet;
public:
	OfflineMessageManager() : _lockerSet("OfflineMessageManager::_lockerSet") {}

	void SetOfflineMessage(int roleid) {
		Thread::Mutex::Scoped l(_lockerSet);
		_set.insert(roleid);
	}

	void ClearOfflineMessage(int roleid) {
		Thread::Mutex::Scoped l(_lockerSet);
		_set.erase(roleid);
	}

	bool HasOfflineMessage(int roleid) {
		Thread::Mutex::Scoped l(_lockerSet);
		return _set.find(roleid) != _set.end();
	}
};

class TeamWalker{
	public:
		TeamBeanVector teams;
		void Visit(TeamBean team)
		{
			teams.push_back(team);
		}
};

class TeamManager {
	typedef std::map<int64_t,TeamBean> TeamInfo;
	TeamInfo _teamMap;
	
public:
	void Walk(TeamWalker &walker);
	
	void AddTeam(const TeamBean team)
	{
		_teamMap[team.teamid] = team;
	}

	void DismissTeam(const int64_t team_uid)
	{
		_teamMap.erase(team_uid);
	}
	
	bool ChangeLeader(const int64_t team_uid,int new_leader)
	{
		TeamInfo::iterator it = _teamMap.find(team_uid);
		if(it == _teamMap.end())
			return false;
		std::vector<int64_t>::iterator m_it = it->second.members.begin();
		for(;m_it != it->second.members.end();m_it++)
		{
			if(*m_it == (int64_t)new_leader)
			{
				it->second.captain == (int64_t)new_leader;
				return true;
			}
		}
		return false;
	}

	bool RemoveMember(const int64_t team_uid,int member)
	{
		TeamInfo::iterator it = _teamMap.find(team_uid);
		if(it == _teamMap.end())
			return false;
			
		std::vector<int64_t>::iterator m_it = it->second.members.begin();
		for(;m_it != it->second.members.end();m_it++)
		{
			if(*m_it == (int64_t)member)
			{
				it->second.members.erase(m_it);
				return true;
			}
		}
		return false;
	}

	bool AddMember(const int64_t team_uid,int member)
	{
		TeamInfo::iterator it = _teamMap.find(team_uid);
		if(it == _teamMap.end())
			return false;
		
		std::vector<int64_t>::iterator m_it = it->second.members.begin();
		for(;m_it!=it->second.members.end();m_it++)
		{
			if((int64_t)member == *m_it)
				return false;
		}
		it->second.members.push_back((int64_t)member);
		return true;
	}
};

class GameTalkManager {
	bool _connectedGT;
	int _aid;
	int _zoneid;
	time_t _bootTimeGame;
	time_t _bootTimeGT;
	RoleStatusManager _statusMan;
	ProtocolBuffer _buffer;
	OfflineMessageManager _offMsgMan;
	TeamManager _teamMan;
	
	void _ClearAllGTStatus();
	void _SendAllGameStatus();
	void _SendOrBuffer(const Protocol& p);
	void _SendStatusToFriends(int roleid, const std::vector<int64_t> &friends);
	void _SendOfflineMessages(int roleid);
	bool _SendStatusUpdate(int roleid, char status);
	bool _SendStatusResp(const std::vector<int> &roles, int64_t localrid = 0);
	bool _SendAnnounce(int aid, int zoneid, time_t boottime);
	bool _SendTeamInfo(TeamBeanVector teams);
public:
	inline static GameTalkManager *GetInstance() {
		static GameTalkManager instance;
		return &instance;
	}

	GameTalkManager() : _connectedGT(false), _bootTimeGT(0) {}
	inline char GetRoleStatus(int roleid) { return _statusMan.GetRoleStatus(roleid); }
	inline void SetOfflineMessage(int roleid) { _offMsgMan.SetOfflineMessage(roleid); }
	inline void ClearOfflineMessage(int roleid) { _offMsgMan.ClearOfflineMessage(roleid); }
//	inline bool HasOfflineMessage(int roleid) { return _offMsgMan.HasOfflineMessage(roleid); } 
	
	void GTRoleUpdate(int roleid, char status, const std::vector<int64_t> &friends);
	void GTRoleResponse(const std::map<int64_t, RoleStatusBean> &map);
	void GTRoleRequest(const std::vector<int64_t> &roles, int64_t localrid);

	void GameRoleOnline(int roleid);
	void GameRoleOffline(int roleid);

	void OnStartUp(int aid, int zoneid);
	void OnConnected();
	void OnIdentification(time_t boottime);
	void OnDisconnected();

	void NotifyUpdateFriend(int roleid, int operation, int frid, int groupid = 0, int gtype = GT_GROUP_NORMAL, const Octets &name = Octets());
	void NotifyUpdateGroup(int roleid, int operation, int groupid = 0, int gtype = GT_GROUP_NORMAL, const Octets &name = Octets());
	void NotifyUpdateRole(int roleid, int level);
	void NotifyUpdateRole(int roleid, Octets &name);
	void NotifyCreateRole(int userid, int roleid, Octets &name, char gender, int occupation);
	void NotifyRemoveRole(int userid, int roleid);
	void NotifyPreRemoveRole(int roleid);
	void NotifyUndoRemoveRole(int roleid);
	void NotifyUpdate(const Protocol &p);
	void NotifyTeamCreate(int64_t team_uid,int captain,std::vector<int> members);
	void NotifyTeamDismiss(int64_t team_uid);
	void NotifyTeamMemberOp(int64_t team_uid,char op,int member);
	
	void SendActiveRole(int roleid, char operation);
	bool SendRoleMsg(int receiver, int sender, const Octets &senderName, const Octets &content, unsigned char emotion);
	void FlushBuffer();
		
	bool RoleGameOnline(int roleid);
	bool RoleGTOnline(int roleid);
	
	void SendPlayerEnterLeaveGT(PlayerInfo *pinfo,int op);
};

}

#endif
