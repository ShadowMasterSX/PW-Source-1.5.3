#ifndef __GNET_CDSMNFBATTLEMAN_H
#define __GNET_CDSMNFBATTLEMAN_H
#include "itimer.h"
#include "mndomaininfo"
#include "mnfactioninfo"
#include "mnfactionapplyinfo"
#include "mndomainbonus"
#include "mnfactionbriefinfo"
#include "mndomaindata"

//跨服帮战跨服逻辑 CentralDeliveryServer
namespace GNET
{
	class CDS_MNFactionBattleMan: public IntervalTimer::Observer
	{
		public:
#define MNF_DEBUG
			enum DOMAIN_TYPE
			{
				DOMAIN_TYPE_A,
				DOMAIN_TYPE_B,
				DOMAIN_TYPE_C,
				DOMAIN_TYPE_COUNT,
			};

			enum DOMAIN_NUM
			{
				DOMAIN_NUM_A = 8, //A城数量
				DOMAIN_NUM_B = 16,
				DOMAIN_NUM_C = 32,
			};

			enum
			{
				INIT_INSTANCE_ONCE = 1,
			};

			enum BATTLE_MAN_STATE 
			{
				STATE_CLOSE = 0,    			//活动没有开启
				STATE_APPLY_FILTRATED_BEGIN,	//中心服开始接受全服申请
				STATE_APPLY_FILTRATED_END,		//接受完申请之后，开始筛选
				STATE_BATTLE_WAIT_BEGIN,		//等待战场开启的中间状态
				STATE_BATTLE_BEGIN,				//战斗开启，开启战场
				STATE_BATTLE_END,				//战斗结束
				STATE_CALC_BONUS,				//计算奖励
				STATE_SAVE_DB_BONUS,			//保存奖励到DB
				STATE_SEND_BONUS,				//发奖
			};

			enum
			{
				INVITE_COUNT_PER_DOMAIN = 62, // 每座城可以邀请62个玩家，用于计算跨服的数量
				INVITE_COUNT_PER_DOMAIN_MAX  = 60, // 每次战斗最多可以进60个玩家，
				INVITE_COUNT_PER_DOMAIN_HAS_ENTER_MAX = 70, //每次战斗一个帮派最多能进去过70个玩家
				INVITE_COUNT_MAX = 200,
			};

			enum RATIO
			{
				RATIO_P = 1, //B城的奖励系数
			};

			enum 
			{
				ONE_DAY = 86400, //一天的秒数
			};

			typedef std::map<int64_t, MNFactionInfo> FACTION_MAP;
			typedef std::map<int64_t, MNFactionApplyInfo> APPLYINFO_MAP;

			struct BattleDomainInfo
			{
				int domain_id; 
				unsigned char domain_type;
				int64_t owner_unifid;
				int64_t attacker_unifid;
				int64_t defender_unifid;
				int enter_domain_num_defender;
				int enter_domain_num_attacker;
				int has_enter_domain_num_defender;
				int has_enter_domain_num_attacker;

				void Init(int domain_id_, unsigned char domain_type_, int64_t owner_, int64_t attacker_, int64_t defender_);
			};
			typedef std::map<int, BattleDomainInfo> DOMAIN_MAP;

			struct PlayerEntry
			{
				int roleid;		//跨服roleid
				int64_t unifid;
				int domain_id;
			};
			typedef std::map<int/*cross_roleid*/, PlayerEntry> PLAYER_ENTRY_MAP;

			typedef std::map<int64_t/*unifid*/, MNDomainBonus> BONUS_MAP;

            struct FactionDomainCredit 
            {       
				int credit_domain_a;
				int credit_domain_b;
				int credit_domain_c;
                void Init(int credit_a, int credit_b, int credit_c);
			};

		private:
			//cache
			FACTION_MAP _faction_map;
			APPLYINFO_MAP _apply_map;
			std::vector<int64_t> _lvl3_list; //在本服有12座城以上的帮派，在筛选时具有高优先级
			std::vector<int64_t> _chosen_list;
			DOMAIN_MAP _domain_map;
			PLAYER_ENTRY_MAP _player_entry_map;
			BONUS_MAP _bonus_map;
			bool _is_init;
			unsigned int _sn;
			int _server_id;
			int _world_tag;
			unsigned int _has_init_instance_domain_num;

			//config
			unsigned char _state;
			int _apply_filtrated_begin_time;
			int _apply_filtrated_end_time;
			int _cross_day;
			int _cross_begin_time;
			int _battle_begin_time;
			int _battle_end_time;
			int _bonus_time;  //_battle_end_time<->_bonus_time用于接收帮战结果
			int _close_time;  //活动结束

			MNDomainBonusRewardItem _item_a;
			MNDomainBonusRewardItem _item_b;
			MNDomainBonusRewardItem _item_c;
			MNDomainBonusRewardItem _item_master;
			
			FactionDomainCredit faction_credit;

			//client data
			MNDomainDataVector _domain_data;
			
			//debug 
			int _apply_begin_time;
			int _adjust_time;
		public:	
			~CDS_MNFactionBattleMan() {}
			static CDS_MNFactionBattleMan* GetInstance() 
			{
				static CDS_MNFactionBattleMan _instance;
				return &_instance; 
			} 	
			bool Initialize();
			bool Update();

		public:
			//init
			void OnInitialize();
			void OnGetDBMNFactionCache(unsigned int sn, unsigned char state, MNDomainInfoVector & domaininfo_list, MNFactionInfoVector & factioninfo_list);
			void InitDomainInfoMap(MNDomainInfoVector & domaininfo_list);
			void UpdateMNFactionInfoMap(MNFactionInfoVector& factioninfo_list);
			void UpdateCDCMNFactionInfoMap(MNFactionInfoVector& factioninfo_list);
			void UpdateDBMNFactionInfo(MNFactionInfoVector& factioninfo_list);
			void SendMNFBattleCache(int zoneid);
			void GSMNDomainInfoNotice(unsigned int sid);

			//apply
			void OnGetMNFApplyInfoList(MNFactionApplyInfoVector& applyinfo_list);
			void OnGetMNFactionProclaim(int zoneid, MNFactionApplyInfoVector& applyinfo_list, MNFactionInfoVector& factioninfo_list, std::vector<int64_t>& lvl3_list);
			void SendFiltrateResult(int zoneid);

			//battle start
			void ClearEnterBattlePlayerNum(); //清空上次进入战场的人数
			void InitDomainInstance();
			void OnRegisterServer(int server_id, int world_tag);
			void OnPlayerBattleEnter(int roleid, int64_t unifid, int domain_id);
			void OnPlayerBattleEnterSuccess(int roleid, int64_t unifid, int domain_id);
			void OnPlayerBattleLeave(int roleid, int64_t unifid, int domain_id);
			void OnDomainBattleEnd(int domain_id, int64_t winner_fid);
			void SendPlayerLastEnterInfo(int roleid);

			//bonus
			void OnRecvSendBonusDataResult(int64_t unifid);
			void OnSendBonusSuccess(int64_t unifid);

			//notify client
			void SendClientDomainData(int roleid);
			void SendClientFactionInfo(int roleid, int64_t unifid);
			void SaveForClientDomainData();
			//debug
			void SetAdjustTime(int offset);
			void SetState(int state);
			void SetDomainInfo(int domain_id, unsigned int attacker_fid, int attacker_zoneid, unsigned int defender_fid, int defender_zoneid);
			void SetDomainWinner(int domain_id, bool attacker_or_defender);
			void SetPlayerBattleEnterSuccess(int roleid, unsigned int fid, int zoneid, int domain_id);
			void SetInitDomainInstance();
			void TestBonus();
			void TestUpdateDomainNum();
			void ClearDomainMap();
			// announce
			void AnnounceWinner(int domain_id,int64_t winner,int64_t loser);

		private:
			//init
			void UpdateDBMNFactionState();
			void UpdateDomainInfoMap(MNDomainInfoVector &domaininfo_list);
			void UpdateApplyInfoMap(MNFactionApplyInfoVector& applyinfo_list);
			void FiltrateApplyInfo();
			void ClassifyApplyInfo(std::vector<int64_t> &candidates_a, std::vector<int64_t> &candidates_b, std::vector<int64_t> &candidates_c);
			void FiltrateApplyInfo_A(std::vector<int64_t> &candidates_a);
			void FiltrateApplyInfo_B(std::vector<int64_t> &candidates_b);
			void FiltrateApplyInfo_C(std::vector<int64_t> &candidates_c);
			void RandMatchBattle(std::vector<int64_t>& filtrate_result_list, DOMAIN_TYPE domain_type);
			void CopyDomainMap(const DOMAIN_MAP& src, DOMAIN_MAP& dest);
			void UpdateMNFactionInviteCount();
			void UpdateDBMNFactionInfo();
			void UpdateDBMNDomainInfo();
			void GSMNDomainInfoBroadcast();
			//battle start
			int CheckPlayerBattleEnter(int cross_roleid, int64_t unifid, int domain_id);
			// bonus
			void CalcBonusCredit();
			void CalcBonusCreditA();
			void CalcBonusCreditB();
			void CalcBonusCreditC();
			void UpdateDomainNum();
			void UpdateDBDomainBonus();
			void GetCredit(int domain_type, int& credit);
			void SendDomainBonusData();

			//close
			void Clear();

			//tools
			inline bool is_same_week(time_t t1, time_t t2); 

			//debug
			time_t GetTime();
			void DebugDumpDomainData(const char* func, int line);
			void DebugDumpDomainInfo(const char* func, MNDomainInfoVector& domaininfo_list);
			void DebugDumpDomainInfo(const char* func);
			void DebugDumpFactionInfo(const char* func, MNFactionInfoVector& factioninfo_list);
			void DebugDumpFactionInfo(const char* func);
			void DebugDumpApplyInfo(const char* func, MNFactionApplyInfoVector& applyinfo_list);
			void DebugDumpApplyInfo(const char* func);
			void DebugDumpBonus(const char* func);
			void DebugDumpBonus(const char* func, MNDomainBonusVector& bonus_list);
			void DebugDumpUnifid(const char* str, std::vector<int64_t>& list);
			void DebugDumpFlag(const char* func, int line);

		private:
			CDS_MNFactionBattleMan(): _is_init(false), _sn(0), _server_id(0), _world_tag(0),_has_init_instance_domain_num(0),  _state(STATE_CLOSE), _apply_filtrated_begin_time(0), _apply_filtrated_end_time(0), _cross_day(0), _battle_begin_time(0), _battle_end_time(0), _bonus_time(0), _close_time(0), _adjust_time(0)
			{
				faction_credit.Init(0, 0, 0);
			}
			bool LoadConfig();
	};

	class CDS_MNToplistMan 
	{
	public:
		enum
		{
			TOPLIST_MAX_SIZE = 50,
		};

		void UpdateMNToplist(const MNFactionInfoVector& info_list);
		void UpdateMNToplist(const CDS_MNFactionBattleMan::FACTION_MAP& info_list);
		//int GetMNFactionRank(int64_t unifid);
		int GetMNFactionRank(unsigned int fid, int zoneid);
		void SendTopList(int zoneid);
		void AnnounceTop();
	
	public:
		static CDS_MNToplistMan* GetInstance() 
		{
			static CDS_MNToplistMan _instance;
			return &_instance; 
		} 	

	private:
		typedef std::multimap<int/*credit*/, MNFactionBriefInfo> TOPLIST_MAP;
		TOPLIST_MAP _toplist_map;
		MNFactionBriefInfoVector _toplist;

		void UpdateMNFactionRank(const MNFactionInfo& info);
		void SaveForClientToplist();

		//debug
		void DebugDumpToplist(const char* func);
	};
};
#endif
