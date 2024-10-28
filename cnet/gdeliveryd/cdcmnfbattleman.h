#ifndef __GNET_CDCMNFBATTLEMAN_H
#define __GNET_CDCMNFBATTLEMAN_H 
#include "itimer.h"
#include "mndomaininfo"
#include "mnfactioninfo"

#include "mnfactioncacheget.hpp"
#include "centraldeliveryclient.hpp"
#include "mnfactionapplyinfo"
#include "gdeliveryserver.hpp"
#include "mngetdomaindata_re.hpp"
//跨服帮战普通服逻辑
namespace GNET
{
	class CDC_MNFactionBattleMan : public IntervalTimer::Observer
	{
		public:
			enum DOMAIN_TYPE
			{
				DOMAIN_TYPE_A,
				DOMAIN_TYPE_B,
				DOMAIN_TYPE_C,
				DOMAIN_TYPE_COUNT,
			};

			enum BATTLE_MAN_STATE 
			{
				STATE_CLOSE = 0,   				//活动没有开启
				STATE_APPLY_BEGIN,				//开始报名
				STATE_APPLY_END,				//结束报名
				STATE_FETCH_CDS_FILTRATE_RES,	//获取中心服的筛选结果
				STATE_CROSS_BEGIN,				//可以进入跨服
				STATE_CROSS_END,				//不能进入跨服(战斗结束)
			};

			typedef std::map<int64_t, MNFactionApplyInfo> APPLYINFO_MAP;
			typedef std::map<int, MNDomainInfo> DOMAIN_MAP;

			struct BattleFactionInfo
			{
				int64_t unifid;
				unsigned int fid;
				Octets faction_name;
				Octets master_name;
				std::vector<char> domain_num;
				int zoneid;
				int credit;
				int credit_this_week;
				int credit_get_time;
				unsigned int invite_count;
				unsigned int accept_sn;
				unsigned int bonus_sn;
				unsigned version;
				std::vector<int> cross_rolelist; //参加跨服帮战的人数，通过accept_sn判断是不是本届

				void Init(MNFactionInfo& info);
			};
			typedef std::map<int64_t/*unifid*/, BattleFactionInfo> FACTION_MAP;

		private:
			bool _is_init_db;
			bool _is_init_central;
			unsigned char _state;
			APPLYINFO_MAP _apply_map;
			FACTION_MAP _faction_map;
			DOMAIN_MAP _domain_map;
			unsigned int _sn;

			//config
			unsigned int _domain_count_lvl1;
			unsigned int _domain_count_lvl2;
			unsigned int _domain_count_lvl3;
			unsigned int _max_apply_faction_num;
			
			int _apply_begin_time;
			int _apply_end_time;
			int _fetch_filtrate_res_time;
			int _cross_begin_time;
			int _cross_end_time;
			int _close_time;

			//flag
			bool _is_filtrated_cdc; 			//报名结束后，本服是否筛选过
			bool _is_need_update_battle_cache;	//是否需要从中心服获取缓存
			bool _is_notify_db_apply_res;		//是否通知数据库，原服筛选的报名结果
			bool _is_send_proclaim_success;		//是否发送报名信息成功
			bool _is_get_cds_filtrate_res;		//是否获取到中心服筛选结果
			bool _is_get_cds_faction_toplist;  	//获取中心服排行榜数据

			//client data
			MNDomainDataVector _domain_data;
			MNFactionApplyDataVector _apply_data;

			//debug
			int _adjust_time;

		public:	

			~CDC_MNFactionBattleMan() {}
			static CDC_MNFactionBattleMan* GetInstance()
			{
				static CDC_MNFactionBattleMan _instance;
				return &_instance; 
			} 	
			//void InitCurrentState();
			void InitCurrentState(int apply_begin_time, int apply_end_time, int cross_begin_time, int battle_begin_time, int battle_end_time);
			bool Initialize();
			bool Update();

		public:
			//init
			void OnInitialize();
			bool NeedUpdateBattleCache() { return _is_need_update_battle_cache; } 
			void EnableUpdateBattleCacheFlag() { _is_need_update_battle_cache = true; } 
			void GSMNDomainInfoNotice(unsigned int sid);
			void EnableSendProclaimFlag() { _is_send_proclaim_success = true; } 
			void GetCDSBattleCache();
			void OnGetCDSBattleCache(unsigned int sn, int apply_begin_time, int apply_end_time, int cross_begin_time, int battle_begin_time, int battle_end_time, MNDomainInfoVector & domaininfo_list, MNFactionInfoVector & factioninfo_list);
			void OnGetMNFApplyInfoList(MNFactionApplyInfoVector& applyinfo_list);
			void UpdateMNFactionInfo(MNFactionInfo& factioninfo);
			void UpdateMNFactionInfoMap(MNFactionInfoVector& factioninfo_list);

			//apply
			int CheckMNFApply(unsigned int fid, int64_t unifid, unsigned char target);
			void OnGetCDSFiltrateResult(std::vector<int64_t>& chosen_list);
			void OnNotifyDBApplyResSuccess(unsigned char status, std::vector<int64_t>& rejected_list);
			void UpdateMNFApplyInfo(MNFactionApplyInfo& applyinfo);

			//cross 
			void OnMNFactionPlayerCross(int64_t unifid, int roleid, bool backflag);
			int CheckMNFactionPlayerCross(int64_t unifid);

			//battle end
			void EnableGetToplistFlag() {_is_get_cds_faction_toplist = true;}

			//bonus
			void OnRecvBonusData(MNDomainBonus& bonus);

			//notify client
			void SendClientFactionInfo(int roleid, int64_t unifid);
			void SendClientDomainData(int roleid);
			void SaveForClientDomainData();
			void SaveForClientApplyData();
			void SendMNFactionBriefInfo(unsigned int fid, int zoneid, int roleid);

			//debug
			void SetAdjustTime(int offset);
			void SetState(int state);
			void DebugFiltrateApplyInfo(MNFactionInfo& factioninfo, MNFactionApplyInfo& applyinfo);

		private:
			CDC_MNFactionBattleMan(): _is_init_db(false), _is_init_central(false), _state(STATE_CLOSE), _sn(0), _apply_begin_time(0), _apply_end_time(0), _fetch_filtrate_res_time(0), _cross_begin_time(0), _cross_end_time(0), _close_time(0), _is_filtrated_cdc(false), _is_need_update_battle_cache(true), _is_notify_db_apply_res(false), _is_send_proclaim_success(false), _is_get_cds_filtrate_res(false), _adjust_time(0){}
			bool LoadConfig();
			void GSMNDomainInfoBroadcast();
			void UpdateMNDomainInfoMap(MNDomainInfoVector& domaininfo_list);
			void UpdateDBMNFactionCache(MNDomainInfoVector& domaininfo_list, MNFactionInfoVector& factioninfo_list);
			unsigned int GetDomainCount(unsigned int fid);
			int UpdateCandidatesFaction(unsigned int fid, int64_t unifid, std::vector<int64_t>& candidates_lvl1, std::vector<int64_t>& candidates_lvl2, std::vector<int64_t>& candidates_lvl3);
			void FiltrateApplyInfoINCDC(std::vector<int64_t>& chosen_list);
			void SendCDSProclaim();
			void FetchCDSFiltrateRes();
			void NotifyDBFiltrateRes(std::vector<int64_t>& chosen_list);
			void Clear();
			time_t GetTime();
		
			//tools
			inline bool is_same_week(time_t t1, time_t t2);  
			void DebugDumpDomainInfo(const char* func, MNDomainInfoVector& domaininfo_list);
			void DebugDumpDomainInfo(const char* func);
			void DebugDumpFactionInfo(const char* func, MNFactionInfoVector& factioninfo_list);
			void DebugDumpFactionInfo(const char* func);
			void DebugDumpApplyInfo(const char* func, MNFactionApplyInfoVector& applyinfo_list);
			void DebugDumpApplyInfo(const char* func);
			void DebugDumpUnifid(const char* str, std::vector<int64_t>& list);
			void DebugDumpFlag(const char* func, int line);
			void DebugDumpDomainData(const char* func, int line);
			void DebugDumpApplyData(const char* func, int line);
	};

	class CDC_MNToplistMan
	{
	public:

		void EnableNeedFetchToplistFlag() { _need_fetch_toplist_flag = true; }
		bool NeedFetchToplist() { return _need_fetch_toplist_flag; }
		void OnGetCDSToplist(MNFactionBriefInfoVector& toplist);
		int GetMNFactionRank(unsigned int fid, int zoneid);
		//int GetMNFactionRank(int64_t unifid);
		void GetMNToplist();
		void SendClientToplist(int roleid);
		static CDC_MNToplistMan* GetInstance() 
		{
			static CDC_MNToplistMan _instance;
			return &_instance; 
		}

	private:
		MNFactionBriefInfoVector _toplist;
		bool _need_fetch_toplist_flag;
		CDC_MNToplistMan(): _need_fetch_toplist_flag(true){}

		//debug
		void DebugDumpToplist(const char* func);
	};

};

#endif
