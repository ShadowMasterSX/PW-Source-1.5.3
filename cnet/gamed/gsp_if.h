#ifndef __GS_PROTO_IF_H__
#define __GS_PROTO_IF_H__

#include <string>
#include <vector>

#include "privilege.hxx"

class object_interface;

void user_login(int cs_index,int sid,int uid,const void * auth_buf, size_t auth_size);
void report_cheater(int roleid, int cheattype, const void *cheatinfo, size_t size);
void acquestion_ret(int roleid, int ret); // ret: 0 ?yè·, 1 ′í?ó, 2 3?ê±
void player_rename_ret(int roleid, const void *new_name, size_t name_len, int ret);

namespace GNET
{
class Protocol;
}

namespace GDB
{
class itemlist;
}

namespace GMSV
{
	enum
	{
		PLAYER_LOGOUT_FULL  =  0,
		PLAYER_LOGOUT_HALF  =  1,
	};
	enum
	{
		CHAT_CHANNEL_LOCAL = 0,		//普通频道
		CHAT_CHANNEL_FARCRY ,		//世界频道
		CHAT_CHANNEL_TEAM ,			//队伍频道
		CHAT_CHANNEL_FACTION ,		//帮派频道
		CHAT_CHANNEL_WHISPER ,		//密语信息
		CHAT_CHANNEL_DAMAGE,		//伤害信息
		CHAT_CHANNEL_FIGHT,			//战斗信息
		CHAT_CHANNEL_TRADE ,		//交易频道
		CHAT_CHANNEL_SYSTEM,		//系统信息
		CHAT_CHANNEL_BROADCAST,		//广播信息
		CHAT_CHANNEL_MISC,			//其它信息
		CHAT_CHANNEL_INSTANCE,		//副本频道
		CHAT_CHANNEL_SUPERFARCRY,	//超级世界频道
		CHAT_CHANNEL_BATTLE,		//战场己方
		CHAT_CHANNEL_COUNTRY,		//国家

	};
	enum
	{
		CMSG_FF_LEVELUP 			= 37,	//帮派基地升级
		CMSG_FF_TECHNOLOGYUP		= 38,	//帮派基地科技升级
		CMSG_FF_CONSTRUCT			= 39,	//帮派基地建造
		CMSG_FF_CONSTRUCTCOMPLETE 	= 40,	//帮派基地建造完成
		CMSG_FF_HANDINCONTRIB		= 41,	//帮派基地上交贡献
		CMSG_FF_HANDINMATERIAL		= 42,	//帮派基地上交材料
		CMSG_FF_BATTLEPREPARECLEAR 	= 43,	//帮派基地战斗前清场
		CMSG_FF_KEYBUILDINGDESTROY 	= 44,	//帮派基地摧毁主基地建筑
		CMSG_FF_DISMANTLE			= 45,	//帮派基地设施拆除
	};

	struct city_entry
	{
		int level;
		int owner;
	};

	
	void SetCityOwner(int id, int level,int owner);
	int GetCityOwner(unsigned int id);
	int GetCityLevel(unsigned int id);
	int GetMafiaCityCount(int mafiaid);
	int GetFactionBonus(int faction);
	void RefreshCityBonus();
	
//	typedef std::vector< > player_list;
	typedef std::pair<int /*userid*/,int /*sid*/>  puser;

	void StartPollIO();	//开始处理PollIO，这个函数会阻塞住，一直执行
	bool InitGSP(const char* conf,int gid, int worldtag, float x_min, float x_max, float z_min, float z_max, const char *version);
	bool SendClientData(int lid, int userid,int sid,const void * buf, size_t size);
	bool MultiSendClientData(int lid, const puser * first, const puser * last,const void * buf, size_t size,int except_id);
	bool MultiSendClientData(int lid, const puser * first, const puser * last,const void * buf, size_t size);
	bool SendLoginRe(int lid, int userid, int sid,int result, char flag);
	bool SendKickoutRe(int userid, int sid , int result);
	bool SendDisconnect(int lid, int userid,int sid, int result);
	bool SendLogout(int lid, int userid, int sid,int type=PLAYER_LOGOUT_FULL);
	bool SendOfflineRe(int lid,int userid,int sid, int result);
	bool SendPlayerInfoUpdate(int roleid, int level);
	bool SendSNSRoleBrief(int roleid, object_interface obj_if);
	void DiscardTrade(int trade_id, int userid);
	void ReplyTradeRequest(int trade_id, int userid,int localsid,bool isSuccess);
	bool SendPlayerTeamOp(char operation,int64_t team_uid,int captain,std::vector<int> members);
	bool SendPlayerTeamMemberOp(int64_t team_uid,char operation,int member);
	
	struct chat_msg
	{
		int speaker;
		const void * msg;
		size_t size;
		const void * data;
		size_t dsize;
		char channel;
		char emote_id;
		int speaker_level;
	};
	bool MultiChatMsg(int lid,const puser * first, const puser * last, const chat_msg & chat,int except_id);
	bool SendChatMsg(int lid, int userid, int sid,const chat_msg & chat);
	bool BroadChatMsg(const chat_msg &chat);
	bool SetChatEmote(int userid, char emote_id);
	bool SystemChatMsg(const void * msg, size_t size, char channel,	const void * data = NULL, size_t dsize = 0);
	bool CountryChatMsg(const chat_msg &chat);
	bool FactionBroadcastMsg(int fid, int type, const void * msg, size_t size);
	bool SendPlayerHeartbeat(int lid, int userid, int localsid);
	bool SendSwitchServerSuccess(int lid, int userid, int localsid,int gs_id);
	bool SendSwitchServerTimeout(int lid, int userid, int localsid);
	bool SendSwitchServerCancel(int lid, int userid, int localsid);
	bool SendSwitchServerStart(int lid, int userid, int localsid,int src_gsid, int dst_gsid,const void * key,size_t key_size);
	void SendCosmeticRequest(int userid, int ticket_index, int ticket_id);
	void CancelCosmeticRequest(int userid);
	void SendDebugCommand(int roleid, int type, const char * buf, size_t size);
	void SetCouple(int id1, int id2, int op); //op = 1 marry op = 0 unmarry
	void SendRefCashUsed(int roleid, int cash_used, int level);
	void SendTaskReward(int roleid, int bonus_add);	//任务奖励增加红利
	bool SendFactionCongregateRequest(int fid, int sponsor, void* data, size_t size);
	void SendPlayerJoinOrLeaveForce(int force_id, bool is_join);
	void SendIncreaseForceActivity(int force_id, int activity);
	void SendSynMutaData(int roleid,int level,int reincarnation_times); 
	struct CBApplyEntry
	{
		int roleid;
		int major_strength;
		int minor_strength;
	};
	void CountryBattleApply(CBApplyEntry * list, size_t count);
	void CountryBattleJoin(int roleid, int country_id, int worldtag, int major_strength, int minor_strength, char is_king);
	void CountryBattleLeave(int roleid, int country_id, int major_strength, int minor_strength);
	void CountryBattleOnline(int roleid, int country_id, int worldtag, int minor_strength, char is_king);
	void CountryBattleOffline(int roleid, int country_id);
	void CountryBattleEnterMap(int roleid, int worldtag);
	void SendCountryBattleServerRegister(int type/*0首都 1战场*/, int world_index, int world_tag, int battle_type);
	struct CBConfig
	{
		size_t capital_count;
		struct CountryCapital
		{
			int country_id;
			int worldtag;
			float posx;
			float posy;
			float posz;
		}capital_list[];
	};
	void ResponseCountryBattleStart(int battle_id, int world_tag, int retcode, int defender, int attacker);
	struct CBPersonalScore
	{
		int roleid;
		int cls;
		int minor_strength;
		int combat_time;
		int attend_time;
		int kill_count;
		int death_count;
		int score;
		int dmg_output;
		int dmg_output_weighted;
		int dmg_endure;
		int kill_count_weighted;
	};
	void SendCountryBattleEnd(int battle_id, int result, int defender, int attacker, CBPersonalScore* dscore, size_t dcount, CBPersonalScore* ascore, size_t acount);

	bool SendPlayerRename(int roleid, int item_pos, int item_id, int item_num, const void * new_name, size_t new_name_len, object_interface & obj_if);

	bool SendPlayerGivePresent(int roleid, int target_roleid, unsigned int cash_cost, char has_gift, int log_price1,
		   	int log_price2, int mail_id, GDB::itemlist & ilist, object_interface & obj_if);

	bool SendPlayerAskForPresent(int roleid, int target_roleid, int goods_id, int goods_index, int goods_slot);

	bool InitMatcher(const char * file, const char * in_code, const char * check_code, const char * table_code);
	bool CheckMatcher(char * str, unsigned int size);

	bool SendPlayerQueryTouchPoint(int roleid);
    bool SendPlayerCostTouchPoint(int roleid,int64_t orderid,unsigned int cost);
	bool SendPlayerGiftCodeRedeem(int roleid,const char (&cardnumber)[20]);
	
	enum	//与localmacro.h定义的一致
	{
		CHDS_FLAG_DS_TO_CENTRALDS = 1,
		CHDS_FLAG_CENTRALDS_TO_DS = 2,
		CHDS_FLAG_DIRECT_TO_CENTRALDS = 3,
	};
	bool SendTryChangeDS(int roleid, int flag);
	bool SendPlayerChangeDSRe(int retcode, int roleid, int flag);
	void SendMobileServerRegister(int world_index, int world_tag);

	void InitUniqueData(int world_tag,int version);
	//  修改unique 数据的接口，有新参数在下面添加
	void ModifyUniqueData(int key,int type,const void* odata, size_t osz,const void* ndata,size_t nsz,int world_tag, bool exclusive, bool broadcast,int version,bool timeout);

	struct AutoTeamConfig
	{
		unsigned int goal_cnt;
		
		struct OccupationInfo
		{
			char occupation;
			char need_cnt;
		};
		
		struct Goal
		{
			int id;
			unsigned char need_player_cnt;
			unsigned char occupation_list_size;
			OccupationInfo* occupation_list;
		} goal_list[];
	};
	void SendAutoTeamData(const AutoTeamConfig* data);
	void SendPlayerSetAutoTeamGoal(int roleid, char goal_type, char op, int goal_id);
	void SendAutoTeamPlayerReady_Re(int roleid, int leader_id, char retcode);

	void SendTrickBattleServerRegister(int world_index, int world_tag);
	void ResponseTrickBattleStart(int battle_id, int world_tag, int retcode);
	void SendTrickBattleEnd(int battle_id, int world_tag);
	struct TBPersonalScore
	{
		int roleid;
		int kill_count;
		int death_count;
		int score;
	};
	void SendTrickBattlePersonalScore(int battle_id,  int world_tag, TBPersonalScore * list, size_t size);
	void SendTrickBattleApply(int roleid, int chariot);
	void SendTrickBattleEnter(int roleid, int battle_id, int world_tag);
	void SendTrickBattleLeave(int roleid, int battle_id, int world_tag);

	struct MafiaMemberInfo
	{
		int roleid;
		unsigned char rank;
	};
	void SendMafiaPvPEvent(int type, int mafia_src, int mafia_dest , int roleid, unsigned char rank, int domainid, std::vector<MafiaMemberInfo>* memberlist = NULL);
	void SendMafiaPvPRegister(int serverid,int worldtag);
	void SendMafiaPvPQuery(int roleid, int mafiaid);
	struct MPDomainConfig
	{
		unsigned short domain_count;
		unsigned short minebase_count;
		unsigned int bonus_base;
		unsigned int bonus_minecar;
		unsigned short rob_minebase_limit;
		unsigned short rob_minecar_limit;
	};
	void SendMafiaDomainConfig(MPDomainConfig* dlist, size_t dsize, int* clist, size_t csize);

	/*
		下面的函数一般不需要调用
	*/
	bool SendToLS(int lid, GNET::Protocol * p);

	void SetTaskData(int taskid, const void * buf, size_t size);
	void GetTaskData(int taskid, int uid, const void * env_data, size_t env_size);

	void ReportCheater2Gacd(int roleid, int cheattype, const void *buf, size_t size);
	void TriggerQuestion2Gacd(int roleid);

	void ThreadUsageStat();
}

#endif

