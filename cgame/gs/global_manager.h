#ifndef __ONLINEGAME_GS_GLOBAL_MANAGER_H__
#define __ONLINEGAME_GS_GLOBAL_MANAGER_H__

#include "world.h"
#include <string>
#include "weddingmanager.h"
#include "dpsrankmanager.h"

class global_world_manager : public world_manager
{
protected:
	world  _plane;  
	MsgQueueList _msg_queue; 
	std::string _restart_shell;
	player_cid   _cid;
	wedding_manager * _wedding_man;
	dps_rank_manager * _dps_rank_man;
	bool InitNetClient(const char * gmconf);
	friend class global_world_message_handler;
public:
	global_world_manager():_wedding_man(NULL),_dps_rank_man(NULL)
	{}
	~global_world_manager()
	{
		if(_wedding_man) delete _wedding_man;
		if(_dps_rank_man) delete _dps_rank_man;
	}
	int Init(const char * gmconf_file,const char * servername);
	virtual int GetWorldType(){ return WORLD_TYPE_BIG_WORLD; }
	virtual void Heartbeat();
public:
	virtual void RestartProcess();
	virtual bool InitNetIO(const char * servername);
	virtual void GetPlayerCid(player_cid & cid);
	virtual int GetServerNear(const A3DVECTOR & pos) const;
	virtual int GetServerGlobal(const A3DVECTOR & pos) const;
	virtual gplayer* FindPlayer(int uid, int & world_index); 
	virtual world * GetWorldByIndex(size_t index);
	virtual size_t GetWorldCapacity();
	virtual void HandleSwitchRequest(int lid,int uid, int sid,int source, const instance_key & key);
	virtual int GetOnlineUserNumber();
	virtual void GetLogoutPos(gplayer_imp * pImp, int &world_tag, A3DVECTOR & pos);
	virtual void SwitchServerCancel(int link_id,int user_id, int localsid);
	virtual void UserLogin(int cs_index,int cs_sid,int uid,const void * auth_data, size_t auth_size, bool isshielduser, char flag);
	virtual void SetFilterWhenLogin(gplayer_imp * pImp, instance_key * );
	virtual bool IsUniqueWorld();
	virtual world_message_handler * CreateMessageHandler();
	virtual bool TriggerSpawn(int sid);
	virtual bool ClearSpawn(int sid);

	virtual bool IsWeddingServer(){return _wedding_man != NULL;}
	virtual bool WeddingCheckOngoing(int groom, int bride, int scene);
	virtual bool WeddingCheckOngoing(int id);
	virtual bool WeddingSendBookingList(int id, int cs_index, int cs_sid);
	virtual bool WeddingCheckBook(int start_time, int end_time, int scene, int card_year, int card_month, int card_day);
	virtual bool WeddingTryBook(int start_time, int end_time, int groom, int bride, int scene);
	virtual bool WeddingCheckCancelBook(int start_time, int end_time, int groom, int bride, int scene);
	virtual bool WeddingTryCancelBook(int start_time, int end_time, int groom, int bride, int scene);
	virtual bool WeddingDBLoad(archive & ar);
	virtual bool WeddingDBSave(archive & ar);

	virtual bool HasDpsRank(){return _dps_rank_man != NULL;}
	virtual bool DpsRankUpdateRankInfo(int roleid, int level, int cls, int dps, int dph);
	virtual bool DpsRankSendRank(int link_id, int roleid, int link_sid, unsigned char rank_mask);
	virtual bool DpsRankDBLoad(archive & ar);
	virtual bool DpsRankDBSave(archive & ar);
public:
	virtual int SendRemotePlayerMsg(int uid, const MSG & msg);
	virtual void SendRemoteMessage(int id, const MSG & msg);
	virtual int  BroadcastSvrMessage(const rect & rt,const MSG & message,float extend_size);
	virtual void PostMessage(world * plane, const MSG & msg); 
	virtual void PostMessage(world * plane, const MSG & msg,int latancy);
	virtual void PostMessage(world * plane, const XID * first, const XID * last, const MSG & msg);
	virtual void PostPlayerMessage(world * plane, int * player_list, size_t count, const MSG & msg);
	virtual void PostMultiMessage(world * plane,abase::vector<gobject*,abase::fast_alloc<> > &list, const MSG & msg);
	virtual void OnDeliveryConnected();
	virtual void OnMafiaPvPStatusNotice(int status,std::vector<int> &ctrl_list);
	virtual void OnMafiaPvPElementRequest(unsigned int version);		
};

class global_world_message_handler : public world_message_handler
{
protected:
	global_world_manager * _manager;
	world * _plane;
	virtual ~global_world_message_handler(){}
	int PlayerComeIn(world * pPlane,const MSG &msg);
public:
	global_world_message_handler(global_world_manager * pManager, world * plane):_manager(pManager),_plane(plane) {}
	virtual int HandleMessage(world * pPlane, const MSG& msg);
	virtual int RecvExternMessage(int msg_tag,const MSG & msg);
};
/*
class countryterritory_world_manager : public global_world_manager
{
	//首都所在地图及坐标用于玩家下线存盘使用,由delivery通过协议初始化
	struct capital_entry
	{
		int country_id;
		int world_tag;
		A3DVECTOR pos;
	};
	abase::vector<capital_entry> _capital_list;
	bool GetCapital(int country_id, A3DVECTOR &pos, int & tag)
	{
		int list[4];
		int counter = 0;
		for(size_t i=0; i<_capital_list.size() && counter < 4; i++)	
		{
			if(country_id == _capital_list[i].country_id)
			{
				list[counter++] = i;
			}
		}
		if(counter > 0)
		{
			int index = abase::Rand(0,counter-1);
			pos = _capital_list[list[index]].pos;
			tag = _capital_list[list[index]].world_tag;
			return true;
		}
		return false;
	}
	void SetCapital(int country_id, const A3DVECTOR &pos, int tag)
	{
		capital_entry ent;
		ent.country_id = country_id;
		ent.world_tag = tag;
		ent.pos = pos;
		_capital_list.push_back(ent);	
	}
	bool IsCapitalPos(int country_id, const A3DVECTOR &pos)
	{
		for(size_t i=0; i<_capital_list.size(); i++)
		{
			if(country_id == _capital_list[i].country_id && pos.squared_distance(_capital_list[i].pos) <= 1.f) return true;	
		}
		return false;
	}

public:
	countryterritory_world_manager(){}
	virtual ~countryterritory_world_manager(){}
	virtual int GetWorldType(){ return WORLD_TYPE_COUNTRYTERRITORY; }

public:
	virtual world_message_handler * CreateMessageHandler();
	virtual void OnDeliveryConnected();
	virtual bool IsCountryTerritoryWorld(){ return true; }
	virtual void NotifyCountryBattleConfig(GMSV::CBConfig * config);

	virtual void SetFilterWhenLogin(gplayer_imp * pImp, instance_key * );
	virtual void PlayerAfterSwitch(gplayer_imp * pImp);
	virtual void GetLogoutPos(gplayer_imp * pImp, int &world_tag, A3DVECTOR & pos);
	virtual bool GetTownPosition(gplayer_imp *pImp, const A3DVECTOR &opos, A3DVECTOR &pos, int & tag);
};

class countryterritory_world_message_handler : public global_world_message_handler
{
public:
	countryterritory_world_message_handler(global_world_manager * pManager, world * plane):global_world_message_handler(pManager, plane){}
	virtual ~countryterritory_world_message_handler(){}
};
*/
#endif

