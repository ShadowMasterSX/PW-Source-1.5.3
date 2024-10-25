#include "chatroom.h"
#include "chatroomlist_re.hpp"
#include "chatroomjoin_re.hpp"
#include "chatroomleave.hpp"
#include "chatroominvite.hpp"
#include "chatroomspeak.hpp"
#include "chatroomexpel.hpp"
#include "matcher.h"
#include "mapuser.h"

namespace GNET
{
	int ChatRoom::AddMember(int roleid, GChatMemberVector& list, Octets& pass)
	{
		Member m;
		GDeliveryServer* dsm = GDeliveryServer::GetInstance();

		if(password.size()>0 && password!=pass)
			return ERR_CHAT_INVALID_PASSWORD;

		{
			Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
			PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline((roleid));
			if(NULL == pinfo)
				return ERR_CHAT_INVALID_ROLE;
			m.roleid = pinfo->roleid;
			m.name = pinfo->name;
			m.sid = pinfo->linksid;
			m.lsid = pinfo->localsid;
		}
		{
			Thread::Mutex::Scoped l(locker);
			if(members.size()>=(unsigned int)capacity)
				return ERR_CHAT_ROOM_FULL;

			if(find(blacklist.begin(),blacklist.end(),roleid)!=blacklist.end())
				return ERR_CHAT_JOIN_REFUSED;

			ChatRoomJoin_Re re;
			re.retcode = 0;
			re.roomid = id;
			re.roleid = roleid;
			re.name = m.name;
			for(std::vector<Member>::iterator it=members.begin();it!=members.end();it++)
			{
				re.localsid = it->lsid;
				dsm->Send(it->sid, re);
				list.push_back(GChatMember(it->roleid, it->name));
			}
			members.push_back(m);
		}
		return 0;
	}

	int ChatRoom::Invite(int invitee, int inviter)
	{
		ChatRoomInvite re;
		{
			Thread::Mutex::Scoped l(locker);
			re.roomid = id;
			re.invitee = invitee;
			re.inviter = inviter;
			re.name = ownername;
			re.subject = subject;
			re.capacity = capacity;
			re.number = members.size();;
			re.password = password;
			if(inviter==ownerid)
				blacklist.erase(remove(blacklist.begin(),blacklist.end(),invitee),blacklist.end());
		}
		{
			GDeliveryServer* dsm = GDeliveryServer::GetInstance();
			Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
			PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline((invitee));
			if(NULL==pinfo)
				return ERR_CHAT_INVALID_ROLE;
			re.localsid = pinfo->localsid;
			dsm->Send(pinfo->linksid, re);
		}
		return 0;
	}

	int ChatRoom::AddOwner(int roleid)
	{
		Member m;
		{
			Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
			PlayerInfo * pinfo = UserContainer::GetInstance().FindRole((roleid));
			if(NULL==pinfo)
				return ERR_CHAT_INVALID_ROLE;
			m.roleid = pinfo->roleid;
			m.name = pinfo->name;
			m.sid = pinfo->linksid;
			m.lsid = pinfo->localsid;
		}
		ownername = m.name;
		members.push_back(m);
		return 0;
	}

	int ChatRoom::Speak(int src, int dst, const Octets& message)
	{
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRole(src);
		if(NULL == pinfo)
			return ERR_CHAT_SEND_FAILURE;
		ChatRoomSpeak re;
		re.emotion = pinfo->emotion;
		re.roomid = id;
		re.src = src;
		re.dst = dst;
		re.message = message;

		GDeliveryServer* dsm = GDeliveryServer::GetInstance();

		Thread::Mutex::Scoped l(locker);
		MemberVector::iterator iend = members.end();
		for(MemberVector::iterator i=members.begin();i!=iend;i++)
		{
			if(i->roleid==src)
			{
				for(MemberVector::iterator it=members.begin();it!=iend;it++)
				{
					if(!dst||dst==it->roleid)
					{
						re.localsid = it->lsid;
						dsm->Send(it->sid, re);
					}
				}
				return 0;
			}
		}
		return ERR_CHAT_SEND_FAILURE;
	}

	int ChatRoom::DelMember(int roleid, bool informself)
	{
		ChatRoomLeave re;
		re.roomid = id;
		re.roleid = (roleid==ownerid)?0:roleid;
		GDeliveryServer* dsm = GDeliveryServer::GetInstance();

		Thread::Mutex::Scoped l(locker);
		for(std::vector<Member>::iterator i=members.begin();i!=members.end();)
		{
			if(i->roleid==roleid)
			{
				if(informself)
				{
					re.localsid = i->lsid;
					dsm->Send(i->sid, re);
				}
				i = members.erase(i);
			}
			else
			{
				re.localsid = i->lsid;
				dsm->Send(i->sid, re);
				i++;
			}
		}
		return 0; 
	}

	int ChatRoom::Expel(int roleid, int owner)
	{
		if(owner!=ownerid || roleid==ownerid)
			return ERR_CHAT_PERMISSION_DENY;

		ChatRoomExpel re;
		re.roomid = id;
		re.roleid = roleid;
		re.owner = owner;
		GDeliveryServer* dsm = GDeliveryServer::GetInstance();

		Thread::Mutex::Scoped l(locker);
		for(std::vector<Member>::iterator it=members.begin();it!=members.end();)
		{
			re.localsid = it->lsid;
			dsm->Send(it->sid, re);
			if(it->roleid==roleid)
				it = members.erase(it);
			else
				it++;
		}
		if(blacklist.size()<64)
			blacklist.push_back(roleid);
		return 0;
	}

	int RoomManager::Create(unsigned short& roomid, int roleid, Octets& subject, Octets& password, unsigned short capacity)
	{
		if(subject.size()>20 || Matcher::GetInstance()->Match((char*)subject.begin(),subject.size())!=0)
			return ERR_CHAT_INVALID_SUBJECT;

		{
			Thread::RWLock::WRScoped l(locker);
			if(rooms.size()>MAX_ROOM_COUNT)
				return ERR_CHAT_CREATE_FAILED;
			EnrollMap::iterator ie = enrolls.find(roleid);
			if(ie!=enrolls.end())
				LeaveRoom(ie->second.roomid, roleid);
			ChatRoom* room = new ChatRoom(roleid, subject, password, capacity);
			room->AddOwner(roleid);

			for(RoomMap::iterator it = rooms.find(nextid);it!=rooms.end()&&it->first==nextid;it++,nextid++);
			room->id = nextid++;
			rooms[room->id] = room;
			if(password.size()>0)
				room->status = ChatRoom::STATUS_PROTECTED;
			enrolls.insert(std::pair<int, Enroll>(roleid, Enroll(room->id, 1)));
			roomid = room->id;
		}
		return 0;
	}

	int RoomManager::List(unsigned int sid, unsigned int lsid, unsigned short begin, char reverse)
	{
		ChatRoomList_Re re;
		re.localsid = lsid;
		re.end = 0;
		{
			Thread::RWLock::RDScoped l(locker);
			re.sum = rooms.size();
			RoomMap::iterator it = rooms.end();
			if(!reverse)
			{
				it = rooms.upper_bound(begin);
				if(it==rooms.end()) 
					it = rooms.begin();
			}else if(re.sum>=1){
				it = rooms.lower_bound(begin);
				if(it==rooms.end())
				{
					re.end = 2;
					it = rooms.find(rooms.rbegin()->first);
				}
				for(int i=0;i<ROOM_LIST_PAGE_SIZE&&it!=rooms.begin();i++,it--);
			}
			for(int i=0;i<ROOM_LIST_PAGE_SIZE && it!=rooms.end();i++,it++)
			{
				ChatRoom* p = it->second; 
				re.rooms.push_back(GChatRoom(p->id,p->subject,p->ownername,p->capacity,p->members.size(),p->status));
			}
		}
		GDeliveryServer::GetInstance()->Send(sid, re);
		return 0;
	}

	int RoomManager::Join(unsigned short& roomid, int ownerid, int roleid, Octets& password, GRoomDetail& detail)
	{
		{
			Thread::RWLock::WRScoped l(locker);
			EnrollMap::iterator ie = enrolls.find(roleid);
			if(ie!=enrolls.end())
				LeaveRoom(ie->second.roomid, roleid);
			if(roomid==0)
				roomid = FindRoomByOwner(ownerid);
			if(!roomid)
				return ERR_CHAT_ROOM_NOT_FOUND;

			RoomMap::iterator it = rooms.find(roomid);
			if(it==rooms.end())
				return ERR_CHAT_ROOM_NOT_FOUND;
			ChatRoom* proom = it->second; 
			int ret = proom->AddMember(roleid, detail.members, password);
			if(!ret)
			{
				detail.roomid = roomid;
				detail.subject = proom->subject;
				detail.capacity = proom->capacity;
				detail.status = proom->status;
				detail.owner = proom->ownername;
				enrolls.insert(std::pair<int, Enroll>(roleid, Enroll(roomid, 0)));
			}
			return ret;
		}
		return 0;
	}

	int RoomManager::Expel(unsigned short roomid, int roleid, int owner)
	{
		Thread::RWLock::WRScoped l(locker);
		RoomMap::iterator it = rooms.find(roomid);
		if(it==rooms.end())
			return ERR_CHAT_ROOM_NOT_FOUND;
		int ret = it->second->Expel(roleid,owner);
		if(!ret)
			DelEnroll(roomid, roleid);
		return ret;
	}

	int RoomManager::Speak(unsigned short roomid, Octets& message, int src, int dst)
	{
		Thread::RWLock::RDScoped l(locker);
		RoomMap::iterator it = rooms.find(roomid);
		if(it==rooms.end())
			return ERR_CHAT_ROOM_NOT_FOUND;
		return it->second->Speak(src, dst, message);
	}

	int RoomManager::Invite(unsigned short roomid, int invitee, int inviter)
	{
		Thread::RWLock::RDScoped l(locker);
		RoomMap::iterator it = rooms.find(roomid);
		if(it==rooms.end())
			return ERR_CHAT_ROOM_NOT_FOUND;
		return it->second->Invite(invitee, inviter);
	}

	void RoomManager::DelEnroll(unsigned short roomid, int roleid)
	{
		enrolls.erase(roleid);
	}

	unsigned short RoomManager::FindRoomByOwner(int roleid)
	{
		EnrollMap::iterator it = enrolls.find(roleid);
		if(it!=enrolls.end() && it->second.owner)
			return it->second.roomid;
		return 0;
	}

	void RoomManager::ResetRole(int roleid)
	{
		Thread::RWLock::WRScoped l(locker);
		EnrollMap::iterator it = enrolls.find(roleid);
		if(it!=enrolls.end())
		{
			Logout(it->second.roomid, roleid);
			enrolls.erase(it);
		}
	}

	int RoomManager::Leave(unsigned short roomid, int roleid)
	{
		Thread::RWLock::WRScoped l(locker);
		return LeaveRoom(roomid, roleid);
	}

	int RoomManager::LeaveRoom(unsigned short roomid, int roleid)
	{
		RoomMap::iterator it = rooms.find(roomid);
		if(it==rooms.end())
			return ERR_CHAT_ROOM_NOT_FOUND;
		ChatRoom* proom = it->second;
		proom->DelMember(roleid);
		if(proom->ownerid==roleid)
		{
			rooms.erase(it);
			DestroyRoom(proom);
		}
		DelEnroll(roomid, roleid);
		return 0;
	}

	int RoomManager::Logout(unsigned short roomid, int roleid)
	{
		RoomMap::iterator it = rooms.find(roomid);
		if(it==rooms.end())
			return ERR_CHAT_ROOM_NOT_FOUND;
		ChatRoom* proom = it->second;
		proom->DelMember(roleid, false);
		if(proom->ownerid==roleid)
		{
			rooms.erase(it);
			DestroyRoom(proom);
		}
		return 0;
	}

	void RoomManager::DestroyRoom(ChatRoom* proom)
	{
		for(ChatRoom::MemberVector::iterator it=proom->members.begin();it!=proom->members.end();it++)
			DelEnroll(proom->id, it->roleid);
		nextid = (proom->id >= (rooms.size()*2)) ? 1 : proom->id;
		proom->Destroy();
	}
}

