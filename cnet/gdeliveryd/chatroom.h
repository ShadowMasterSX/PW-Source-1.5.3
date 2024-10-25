#ifndef __CHAT_ROOM_H
#define __CHAT_ROOM_H

#include <vector>
#include "thread.h"
#include "gdeliveryserver.hpp"
#include "gchatmember"
#include "groomdetail"

#define MAX_ROOM_COUNT	    4096
#define MAX_ROOM_CAPACITY   30
#define MAX_ROOM_PER_ROLE   1
#define ROOM_LIST_PAGE_SIZE 10

namespace GNET
{
	class ChatRoom
	{
	public:
		enum{
			STATUS_PROTECTED = 1,
		};
		struct Member
		{
			int roleid;
			Octets name;
			unsigned int sid;
			unsigned int lsid;
			Member(int id,const Octets& n,unsigned int s,unsigned int l):roleid(id),name(n),sid(s),lsid(l){}
			Member() {}
		};
		typedef std::vector<Member>     MemberVector;

		int ownerid;
		Octets subject;
		Octets password;
		int capacity;
		unsigned char status;

		Thread::Mutex locker;
		unsigned short id;
		Octets ownername;
		MemberVector members;
		std::vector<int> blacklist;

		ChatRoom(int i, Octets& s, Octets& p, int size):ownerid(i), subject(s), password(p), capacity(size), 
														status(0), locker("gdelivery::chatroom::locker")
		{}
		~ChatRoom() {} 
		void Destroy() 
		{
			delete this;
		}
		int AddOwner(int roleid);
		int AddMember(int roleid, GChatMemberVector& list, Octets& pass);
		int DelMember(int roleid, bool informself=true);
		int Expel(int roleid, int owner);
		int Invite(int invitee, int inviter);
		int Speak(int src, int dst, const Octets& message);
	};

	class RoomManager
	{
	protected:
		struct Enroll
		{
			unsigned short roomid;
			char owner;
			Enroll(unsigned short r,char o):roomid(r),owner(o){}
		};
		Thread::RWLock locker;
		typedef std::map<unsigned short, ChatRoom*> RoomMap;
		typedef std::map<int, Enroll> EnrollMap;
		RoomMap    rooms;
		EnrollMap  enrolls;

		int Logout(unsigned short roomid, int roleid);
		void DelEnroll(unsigned short roomid, int roleid);
		unsigned short FindRoomByOwner(int roleid);
		void DestroyRoom(ChatRoom* proom);
		int LeaveRoom(unsigned short roomid, int roleid);
		unsigned short nextid; //nextid < MAX_ROOM_COUNT*2 < 65535

	public:
		RoomManager():locker("gdelivery::roommanager::locker"),nextid(1){}
		~RoomManager() 
		{
			Thread::RWLock::WRScoped l(locker);
			for(RoomMap::iterator it=rooms.begin();it!=rooms.end();it++)
				delete it->second;
			rooms.clear();
		} 

		static RoomManager* GetInstance() {
			static RoomManager instance;
			return &instance;
		}	

		int Create(unsigned short& roomid, int roleid, Octets& subject, Octets& password, unsigned short capacity);
		int Leave(unsigned short roomid, int roleid);
		int Expel(unsigned short roomid, int roleid, int owner);
		int Join(unsigned short& roomid, int ownerid, int roleid, Octets& password, GRoomDetail& detail);
		int Invite(unsigned short roomid, int invitee, int inviter);
		int List(unsigned int sid, unsigned int lsid, unsigned short begin, char reverse);
		int Speak(unsigned short roomid, Octets& message, int src, int dst);
		void ResetRole(int roleid);
	};

};
#endif
