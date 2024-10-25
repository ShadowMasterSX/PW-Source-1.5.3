#ifndef __GNET_FACTIONLIB_H
#define __GNET_FACTIONLIB_H

class object_interface;
namespace GNET
{
	//class FactionOPSyncInfo;
	struct syncdata_t
	{
		unsigned int money;
		int sp;
		syncdata_t(unsigned int _m,int _s) : money(_m),sp(_s) { }
	};
	void AccelerateExpelschedule(int wastetime);//s
	bool ForwardFactionOP( int optype,int roleid,const void* pParams,size_t param_len,object_interface obj_if );
	bool QueryPlayerFactionInfo( int roleid );
	bool QueryPlayerFactionInfo( const int* list, int list_len );
	bool SendFactionLockResponse(int retcode,int tid,int roleid,const syncdata_t& syncdata);

	bool SendBattleEnd(int battle_id, int result, int defender, int attacker);
	bool SendBattleServerRegister(int map_type, int server_id, int world_tag);
	bool ResponseBattleStart(int battle_id, int retcode);
	bool ForwardBattleOP(unsigned int type,const void* pParams,size_t param_len,object_interface obj_if );

	/* To implement the following interfaces */
	void ReceivePlayerFactionInfo(int roleid,unsigned int faction_id,char faction_role);
	void FactionLockPlayer(unsigned int tid,int roleid);
	void FactionUnLockPlayer(unsigned int tid,int roleid,const syncdata_t& syncdata);

	bool FactionRenameVerify(int roleid,int fid,const void* name,size_t len);
	bool FactionRenameRespond(int roleid,int fid,int itemid,int itemnum,int itempos,const void* name,size_t len,object_interface& obj_if);
	//°ïÅÉ»ùµØ
	bool SendFactionServerRegister(int server_id, int world_tag);
	bool ForwardFactionFortressOP(unsigned int type,const void* pParams,size_t param_len,object_interface obj_if);
	bool SendFactionFortressState(int factionid, int state);
	
	struct ivec
	{
		const void * data;
		unsigned int size;
	};
	struct faction_fortress_data
	{
		int factionid;	
		int level;
		int exp;
		int exp_today;
		int exp_today_time;
		int tech_point;
		ivec technology;
		ivec material;
		ivec building;
		ivec common_value;
		ivec actived_spawner;
	};
	struct faction_fortress_data2
	{
		int factionid;	
		int health;
		int offense_faction;
		int offense_starttime;
		int offense_endtime;
	};
	class FactionFortressResult
	{
	public:
		virtual void OnTimeOut() = 0;
		virtual void OnFailed() = 0;
		virtual void OnGetData(const faction_fortress_data * data, const faction_fortress_data2 * data2){}
		virtual void OnPutData(){}
		virtual ~FactionFortressResult(){}
	};
	bool get_faction_fortress(int factionid, FactionFortressResult * callback);
	bool put_faction_fortress(int factionid, const faction_fortress_data * data, FactionFortressResult * callback=NULL);
};
#endif
