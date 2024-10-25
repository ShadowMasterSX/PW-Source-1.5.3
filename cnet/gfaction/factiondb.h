#ifndef __GNET_FACTIONDB_H
#define __GNET_FACTIONDB_H

#include <vector>
#include <map>

#include "thread.h"
#include "ids.hxx"
#include "operwrapper.h"
#include "gfactioninfo"
#include "guserfaction"
#include "fmemberinfo"
#include "gfactiondetail"
#include "updatefactionarg"
#include "userfactionres"
#include "settings.h"
#include "localmacro.h"

#define FACTIONDB_UPDATE_INTERVAL	60000000	//60s update 一次, 不要低于rpc timeout
#define ALLIANCE_APPLY_FEE		3000000
#define AGREE_ALLIANCE_FEE		3000000
#define DISAGREE_ALLIANCE_FEE 	0
#define HOSTILE_APPLY_FEE		3000000
#define AGREE_HOSTILE_FEE		0
#define DISAGREE_HOSTILE_FEE	3000000
#define REMOVE_RELATION_FEE		0
#define FORCE_REMOVE_RELATION_FEE	6000000
#define OP_COOLDOWN_TIME		1800
#define APPLY_TIMEOUT			86400
#define RELATION_DURATION		2592000
#define DELAY_EXPEL_TIME		(3600*72)       // 72 hour
#define REASON_EXPEL_SYNC	 	99	

namespace GNET
{
	class FactionBriefInfo
	{
	public:
        unsigned int fid;
        Octets name;
        char level;
        int mem_num;

    public:
        FactionBriefInfo (unsigned int _fid = 0,const Octets& _name = Octets(0),char _level = 0 ,int _mems = 0)
            : fid(_fid),name(_name),level(_level),mem_num(_mems)
        {
        }
	};

	class FactionDetailInfo
	{
	public:
		GFactionDetail info;
		char           version;
		char           status;   // 1, data ready
		IntVector      online;
		int            maxbonus;
		time_t         updatetime;
		bool 		   waitload;
	};

	class Factiondb : public IntervalTimer::Observer
	{
	public:
		struct faction_pair
		{
			int min_fid;
			int max_fid;
			faction_pair(int fid1, int fid2)
			{
				if(fid1 <= fid2)
				{
					min_fid = fid1;
					max_fid = fid2;
				}
				else
				{
					min_fid = fid2;
					max_fid = fid1;
				}
			}
			bool operator < (const faction_pair & rhs) const
			{
				return min_fid < rhs.min_fid || min_fid == rhs.min_fid && max_fid < rhs.max_fid;	
			}
		};
		
		struct faction_relation
		{
			int type;
			int end_time;
			int fid1;
			int fid2;
			faction_relation(){}
			faction_relation(int _type, int _end_time, int _fid1, int _fid2):type(_type),end_time(_end_time),fid1(_fid1),fid2(_fid2){}
		};
		
	private:
		Thread::RWLock locker;
		typedef std::map<unsigned int,FactionDetailInfo*>  Map;
		Map factions;
		typedef std::map<faction_pair,faction_relation> RelationMap;
		RelationMap relation_map;				//此map用于factiond对已加载帮派的外交进行超时判定
		typedef std::map<unsigned int,std::pair<int,unsigned int> >  DelayExpelMap; // 此map进行延迟删除倒计时
		DelayExpelMap delayexpel_map;
		int last_delayupdate_time;
		char pvp_status;
		typedef std::map<unsigned int, unsigned char> PvPMaskMap;
		PvPMaskMap pvpmask_map;
		Factiondb() : locker("Factiondb::locker_faction") {}  
	
		inline int GetRelationType(int fid1, int fid2)	//只能获取已加载的帮派的关系 
		{
			RelationMap::iterator it = relation_map.find(faction_pair(fid1,fid2));
			if(it == relation_map.end())
				return NONE_RELATION; 
			else
				return it->second.type; 
		}
		
		inline void AddDelayExpel(unsigned int fid, unsigned int rid, int time)
		{
			delayexpel_map[rid] = std::make_pair(time-Timer::GetTime(),fid);
		}
		inline void RmvDelayExpel(unsigned int rid)
		{
			delayexpel_map.erase(rid);
		}

		inline void DelayExpelTick(int now);
	public:
		~Factiondb() 
		{ 
			for(Map::iterator it=factions.begin(),ie=factions.end();it!=ie;++it)
			{
				delete it->second;
			}
			factions.clear();
		}

		static Factiondb* GetInstance() { static Factiondb instance; return &instance;}
		bool InitFactiondb();
		bool Update();

		void SetPvpStatus(char status);
		char GetPvpStatus() { return pvp_status; }
		unsigned char GetPvpMask(unsigned int fid);
		void SetPvpMask(unsigned int fid, unsigned char pmask);
		// 特殊标志判断
		bool GetMemberInfo(unsigned int fid, unsigned int rid, FMemberInfo& info);
		bool GetMemberList(unsigned int fid, unsigned int rid, std::vector<int>& list);
		bool CheckMemberList(unsigned int fid, unsigned int rid, std::vector<int>& list);
		bool IsSpecialMember(unsigned int fid, unsigned int rid,unsigned char frole = 255); // 255 表示只要是本帮成员就行不关心职务
		bool NeedDelayExpel(unsigned int fid, unsigned int rid);
		bool AlreadyDelayExpel(unsigned int rid);
		// 建立帮派
		int CreateFaction(unsigned int fid,Octets& name, unsigned int rid, OperWrapper::wref_t oper);
		// 删除帮派
		int RemoveFaction(unsigned int fid, OperWrapper::wref_t oper);
		// 接收成员
		int AcceptJoin(unsigned int fid, unsigned int rid, OperWrapper::wref_t oper);
		// 驱逐帮众
		int DismissMember(unsigned int fid, unsigned int rid, OperWrapper::wref_t oper);
		// 延时驱逐
		int DelayExpelMember(unsigned int fid, unsigned int rid, OperWrapper::wref_t oper);
		int CancelDelayExpel(unsigned int fid, unsigned int rid, OperWrapper::wref_t oper);
		int DelayExpelTimeout(unsigned int fid, unsigned int rid);
		int GetDelayExpelTime(unsigned int rid);
		void SettleDelayExpel(int wastetime);
		// 更改职位
		int UpdateRole(unsigned int fid, int superior, int roleid, char suprole, char role, OperWrapper::wref_t oper);
		int UpdateNickname(unsigned int fid, unsigned int uid, Octets& nick, OperWrapper::wref_t oper);
		int UpdateAnnounce(unsigned int fid, Octets& announce, OperWrapper::wref_t oper);
		int UpgradeFaction(unsigned int fid, int roleid, unsigned int money, OperWrapper::wref_t oper);
		int AllianceApply(int fid, int dst_fid, OperWrapper::wref_t oper);
		int AllianceReply(int fid, int dst_fid, char agree, OperWrapper::wref_t oper);
		int HostileApply(int fid, int dst_fid, OperWrapper::wref_t oper);
		int HostileReply(int fid, int dst_fid, char agree, OperWrapper::wref_t oper);
		int RemoveRelationApply(int fid, int dst_fid, char force, OperWrapper::wref_t oper);
		int RemoveRelationReply(int fid, int dst_fid, char agree, OperWrapper::wref_t oper);

		int FindUserFaction(unsigned int rid, GUserFaction& user, int reason);
		int FindFaction(unsigned int fid, OperWrapper::wref_t oper);
		int ListMember(unsigned int fid, unsigned int rid, int version);
		int ListRelation(unsigned int fid, unsigned int rid);
		void ListOnlineFaction(std::vector<unsigned int>& list); 	//列出在线的3级帮派
		void NotifyPlayerFactionRelation(int fid, int roleid=0);
		int GetMaxbonus(unsigned int fid, int rid=0);
		void UpdateUser(unsigned int fid, GUserFaction& user);
		void UpdateFactionCache(UpdateFactionArg* arg);
		bool FindFactionInCache(FactionBriefInfo& info);
		bool FindFactionName(unsigned int fid, Octets& name);
		void PutFactionCache(unsigned int fid, GFactionDetail& info);
		void ClearLoadFlag(unsigned int fid);
		void SyncFactionInfo(GFactionInfo& info);
		bool ObtainFactionInfo(unsigned int fid);
		bool IsReady(unsigned int fid, Map::iterator& it, bool autoload=true);
		void RemoveFactionInfo(unsigned int fid);
		void NotifyMemberPlayerRename(unsigned int fid, int rid, Octets& name);
		void OnJoin(unsigned int fid, FMemberInfo& user);
		void OnLeave(unsigned int fid, unsigned int rid);
		void OnLogin(unsigned int fid, unsigned int rid, char role, int level, int contrib, int reputation, unsigned char reincarn_times, unsigned char gender,Octets& delayexpel);
		void OnLogout(unsigned int fid, unsigned int rid);
		void OnDeleteRole(unsigned int fid, unsigned int rid);
		void OnUpgrade(unsigned int fid, char level);
		void OnPromote(unsigned int fid, int superior, int roleid, char suprole, char newrole);
		void OnAllianceApply(int fid, int dst_fid, int end_time, int op_time);
		void OnAllianceReply(int fid, int dst_fid, char agree, int end_time, Octets& fname1, Octets& fname2);
		void OnHostileApply(int fid, int dst_fid, int end_time, int op_time);
		void OnHostileReply(int fid, int dst_fid, char agree, int end_time, Octets& fname1, Octets& fname2);
		void OnRemoveRelationApply(int fid, int dst_fid, char force, int end_time, int op_time, Octets& fname1, Octets& fname2);
		void OnRemoveRelationReply(int fid, int dst_fid, char agree, Octets& fname1, Octets& fname2);
		void OnRelationTimeout(int type, int fid1, int fid2, Octets& fname1, Octets& fname2);
		void OnRelationApplyTimeout(int type, int fid1, int fid2, int end_time, Octets& fname1, Octets& fname2);
		void OnPlayerRename(int fid, int roleid, Octets& name);
		void OnDelayExpel(int fid, int roleid, int time, int operatorid);
		void OnCancelExpel(int fid, int roleid);
		void OnTimeoutExpel(int fid, int roleid);
		void OnUpdateExpel(unsigned int fid, unsigned int rid, char role, int level, int contrib,int reputation, unsigned char reincarn_times, unsigned char gender,Octets& delayexpel);
		void OnFactionRename(int fid,const Octets& new_name);
	};
};
#endif

