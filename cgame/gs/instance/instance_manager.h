#ifndef __ONLINEGAME_GS_INSTANCE_MANAGER_H__
#define __ONLINEGAME_GS_INSTANCE_MANAGER_H__

#include "../world.h"
#include <string>

class instance_world_manager : public world_manager
{
public:
	enum
	{
		HEARTBEAT_CHECK_INTERVAL = 10,
		INSTANCE_REENTER_INTERVAL = 300,   // 5min
	};
	enum
	{
		OWNER_MODE_TEAM = 0,
		OWNER_MODE_SINGLE,
	};
protected:
	typedef abase::hashtab<int, instance_hash_key ,instance_hash_function, abase::fast_alloc<> > KEY_MAP;

	player_cid		_cid;
	world *			_plane_template;	//����ģ�壬���е����綼�����︴��(Ӧ�ò��ȴ���NPC)
	abase::vector<world*>  	_cur_planes;  		//��ǰ����������б�
	abase::vector<int>  	_planes_state; 		//��ǰ����������б��״̬�������Ƿ���Բ���ʹ�ã���
	int			_alive_plane_count;	//��ǰ���ڵ�������Ŀ
	int			_active_plane_count;	//��ǰ�����������Ŀ
	size_t			_max_active_index;	//������������������
	int			_planes_capacity;	//������Ŀ���ޣ� ����ؼӼ���ļ���ȴ��
	int			_pool_threshold_low;	//����ص���ֵ������������ֵ�����´β�ѯʱ�����������Ŀ
	int			_pool_threshold_high;	//����ص���ֵ������ص���Ŀ����������ֵ
	int			_idle_time;		//������ж�ûᱻ�ջ�
	int			_life_time;		//�������������, -1��������
	int			_owner_mode;	//����������ģʽ:����,����

	KEY_MAP		 	_key_map;		//���������world��Ӧ��
	int			_key_lock;		//�������������
	MsgQueueList2 		_msg_queue; 		//ͳһ����Ϣ����
	size_t			_heartbeat_counter; 	//����������ʱ�� 
	size_t			_heartbeat_counter2; 	//����������ʱ�� 
//	int			_world_free_count;	//���е�������Ŀ�����Ժ����Ҫɾ����

	std::string 		_restart_shell;		//�������������
	query_map 		_pworld_map; 		//����������ڵ�������б�
	int			_pworld_lock;		//�ϱ��Ӧ����
	int			_player_limit_per_instance;	//ÿ�������������������
	int			_effect_player_per_instance;//ÿ����������Ч���������ĿǰӰ����Ӿ������
	int			_heartbeat_lock;	//Ϊ�˱����ظ�heartbeat����
	int			_pool_lock;		//����ص�lock


	static void timer_tick(int index,void *object,int remain);
	void TimerTick();

	abase::vector<world *> _planes_pool;		//����أ�����Ķ��ǿ�������
	abase::vector<world *> _planes_cooldown;	//������ȴ�� Ҫ���ͷŵ����綼���ȷŵ�����
	abase::vector<world *> _planes_cooldown2;	//������ȴ�� �ǿ��Խ����ͷŵ� �ͷ����������ȴ�ؽ��н���

	virtual bool InitNetClient(const char * gmconf);
	virtual void FinalInit(const char * servername) {}
	virtual void PreInit(const char * servername) {}
protected:
	void RegroupCoolDownWorld();
	void FillWorldPool();
public:
	inline int GetPlayerWorldIdx(int uid)
	{
		int index = -1;
		{
			int *pTmp;
			mutex_spinlock(&_pworld_lock);
			pTmp = _pworld_map.nGet(uid);
			if(pTmp) index = * pTmp;
			mutex_spinunlock(&_pworld_lock);
		}
		return index;
	}

	inline void SetPlayerWorldIdx(int uid, int svr)
	{
		spin_autolock alock(_pworld_lock);
		_pworld_map.find_or_insert(uid,svr) = svr;
	}

	inline void RemovePlayerWorldIdx(int uid)
	{
		spin_autolock alock(_pworld_lock);
		_pworld_map.erase(uid);
	}

	inline void RemovePlayerWorldIdx(int uid, int plane_index)
	{
		spin_autolock alock(_pworld_lock);
		query_map::iterator it = _pworld_map.find(uid);
		if(it != _pworld_map.end())
		{
			if(*(it.value()) == plane_index)
			{
				_pworld_map.erase(uid);
			}
		}
	}

	inline int GetWorldByKey(const instance_hash_key & ikey )
	{
		int index = -1;
		{
			int *pTmp;
			mutex_spinlock(&_key_lock);
			pTmp = _key_map.nGet(ikey);
			if(pTmp) index = *pTmp; 
			mutex_spinunlock(&_key_lock);
		}
		return index;
	}

	virtual void TransformInstanceKey(const instance_key::key_essence & key, instance_hash_key & hkey)
	{
		if(_owner_mode == OWNER_MODE_TEAM)
		{
			hkey.key1 = key.key_level2.first;
			hkey.key2 = key.key_level2.second;
			if(hkey.key1 <= 0)
			{
				hkey.key1 = key.key_level1;
			}
		}
		else if(_owner_mode == OWNER_MODE_SINGLE)
		{
			hkey.key1 = key.key_level1;
			hkey.key2 = 0;
		}
		else
		{
			ASSERT(false);
		}
	}
	static bool TransformInstanceHashKey(const instance_hash_key & hkey,instance_key::key_essence & key)
	{
		key.key_level2.first = hkey.key1;
		key.key_level2.second = hkey.key2;
		return true;
	}

	virtual int GetInstanceReenterTimeout(world* plane);
public:
	instance_world_manager():_key_map(300),_pworld_map(1024)
	{
		_heartbeat_counter = 0;
		_heartbeat_counter2 = TICK_PER_SEC*HEARTBEAT_CHECK_INTERVAL/2;
		_pworld_lock = 0;
		_key_lock = 0;

		_plane_template = NULL;
		_alive_plane_count = 0;
		_active_plane_count = 0;
		_max_active_index = 0;
		_planes_capacity = 0;
		_pool_threshold_low = 0;
		_pool_threshold_high = 0;
		_heartbeat_lock = 0;
		_pool_lock = 0;
		_idle_time = 60*20;
		_life_time = 3600*4;	//4 hour
		_owner_mode = OWNER_MODE_TEAM;
	}

	int Init(const char * gmconf_file,const char * servername);
	virtual int GetWorldType(){ return WORLD_TYPE_INSTANCE; }
	virtual void Heartbeat();
	virtual int CheckPlayerSwitchRequest(const XID & who, const instance_key * key,const A3DVECTOR & pos, int ins_timer);
	world * AllocWorld(const instance_hash_key & ikey,int & world_index, int ctrl_id = 0);
	world * AllocWorldWithoutLock(const instance_hash_key & ikey,int & world_index,int ctrl_id = 0);
	void FreeWorld(world * pPlane, int ins_index);
	virtual world * GetWorldInSwitch(const instance_hash_key & ikey,int & world_index,int ctrl_id);
	virtual world * GetWorldOnLogin(const instance_hash_key & ikey,int & world_index);
	
	virtual void SetIncomingPlayerPos(gplayer * pPlayer, const A3DVECTOR & origin_pos,int special_mask);
	virtual bool GetTownPosition(gplayer_imp *pImp, const A3DVECTOR &opos, A3DVECTOR &pos, int & wold_tag);	
public:
	virtual void RestartProcess();
	virtual void ShutDown();
	virtual bool InitNetIO(const char * servername);
	virtual void GetPlayerCid(player_cid & cid);
	virtual bool CompareInsKey(const instance_key & key, const instance_hash_key & hkey);
	virtual int GetPlayerLimitPerInstance(){ return _player_limit_per_instance; }
	virtual int GetEffectPlayerPerInstance(){ return _effect_player_per_instance; }
	virtual int GetServerNear(const A3DVECTOR & pos) const;
	virtual int GetServerGlobal(const A3DVECTOR & pos) const;
	virtual gplayer* FindPlayer(int uid, int & world_index); 
	virtual world * GetWorldByIndex(size_t index);
	virtual size_t GetWorldCapacity();
	virtual int GetOnlineUserNumber();
	virtual void HandleSwitchRequest(int lid,int uid, int sid,int source, const instance_key &key);
	virtual void PlayerLeaveThisWorld(int plane_index, int useid);
	virtual void GetLogoutPos(gplayer_imp * pImp, int & world_tag ,A3DVECTOR & pos);
	virtual void SwitchServerCancel(int link_id,int user_id, int localsid);
	virtual void UserLogin(int cs_index,int cs_sid,int uid,const void * auth_data, size_t auth_size, bool isshielduser, char flag);
	virtual void SetFilterWhenLogin(gplayer_imp * pImp, instance_key * ikey);
	virtual bool IsUniqueWorld();
	virtual world * CreateWorldTemplate();
	virtual world_message_handler * CreateMessageHandler();
	virtual bool ClearSpawn(int sid);

public:
	virtual int SendRemotePlayerMsg(int uid, const MSG & msg);
	virtual void SendRemoteMessage(int id, const MSG & msg);
	virtual int  BroadcastSvrMessage(const rect & rt,const MSG & message,float extend_size);
	virtual void PostMessage(world * plane, const MSG & msg);
	virtual void PostMessage(world * plane, const MSG & msg,int latancy);
	virtual void PostMessage(world * plane, const XID * first, const XID * last, const MSG & msg);
	virtual void PostPlayerMessage(world * plane, int * player_list, size_t count, const MSG & msg);
	virtual void PostMultiMessage(world * plane,abase::vector<gobject*,abase::fast_alloc<> > &list, const MSG &msg);
};

class instance_world_message_handler : public world_message_handler
{
protected:
	instance_world_manager * _manager;
	virtual ~instance_world_message_handler(){}
	int PlayerComeIn(instance_world_manager *, world * pPlane,const MSG &msg);

	virtual void PlayerPreEnterServer(gplayer * pPlayer, gplayer_imp * pimp,instance_key &  ikey) {}
public:
	instance_world_message_handler(instance_world_manager * man):_manager(man) {}
	virtual int HandleMessage(world * pPlane, const MSG& msg);
	virtual int RecvExternMessage(int msg_tag,const MSG & msg);
};

/*---------------------------------�����������--------------------------------------*/

class faction_world_manager : public instance_world_manager 
{
protected:
	struct wait_node
	{
		XID first;
		instance_key second;
		A3DVECTOR pos;
		wait_node(const XID & f, const instance_key &s , const A3DVECTOR & p):first(f),second(s),pos(p)
		{}
	};
	typedef abase::vector<wait_node > WAIT_ENTRY;
	typedef abase::hashtab<WAIT_ENTRY *, instance_hash_key ,instance_hash_function, abase::fast_alloc<> > WAIT_MAP;
	
	bool AddWaitList(const XID & who, const instance_hash_key & hkey,const instance_key & ikey, const A3DVECTOR &pos);
	void ClearWaitList(const instance_hash_key & hkey,int err_code);
	bool SendReplyToWaitList(const instance_hash_key & hkey,bool is_verify);

	WAIT_MAP _wait_queue;
	int	 _wait_queue_lock;
	virtual void UserLogin(int cs_index,int cs_sid,int uid,const void * auth_data, size_t auth_size, bool isshielduser, char flag);
	virtual void SetFilterWhenLogin(gplayer_imp * pImp, instance_key * ikey);
	virtual void GetLogoutPos(gplayer_imp * pImp, int & world_tag ,A3DVECTOR & pos);
	virtual void SetIncomingPlayerPos(gplayer * pPlayer, const A3DVECTOR & origin_pos,int special_mask);
	virtual bool GetTownPosition(gplayer_imp *pImp, const A3DVECTOR &opos, A3DVECTOR &pos, int & tag);
	virtual void OnDeliveryConnected();
	virtual world * CreateWorldTemplate();
	virtual world_message_handler * CreateMessageHandler();
	virtual int OnMobDeath(world * pPlane, int faction,  int tid);
	virtual void OnMafiaPvPStatusNotice(int status,std::vector<int> &ctrl_list);
	
	virtual bool IsFactionWorld(){ return true; }
	virtual bool FactionLogin(const instance_hash_key &hkey,const GNET::faction_fortress_data * data, const GNET::faction_fortress_data2 * data2);
	virtual bool NotifyFactionData(GNET::faction_fortress_data2 * data2);
	
public:
	faction_world_manager():instance_world_manager(),_wait_queue(1024),_wait_queue_lock(0)
	{
		//���ɸ�����300������
		_idle_time = 300;
		_life_time = -1;
	}
	virtual int GetWorldType(){ return WORLD_TYPE_FACTION; }
	virtual int CheckPlayerSwitchRequest(const XID & who,const instance_key * key,const A3DVECTOR & pos,int ins_timer);
	virtual void TransformInstanceKey(const instance_key::key_essence & key, instance_hash_key & hkey)
	{
		hkey.key1 = key.key_level3;
		hkey.key2 = 0;
	}
	virtual world * GetWorldInSwitch(const instance_hash_key & ikey,int & world_index,int);
	void MakeInstanceKey(int factionid, instance_hash_key & hkey)
	{
		hkey.key1 = factionid;
		hkey.key2 = 0;
	}
};

class faction_world_message_handler : public instance_world_message_handler 
{
protected:
	virtual ~faction_world_message_handler(){}
	virtual void PlayerPreEnterServer(gplayer * pPlayer, gplayer_imp * pimp,instance_key &  ikey);
public:
	faction_world_message_handler(instance_world_manager * man):instance_world_message_handler(man){}
	virtual int HandleMessage(world * pPlane, const MSG& msg);
	virtual int RecvExternMessage(int msg_tag,const MSG & msg);
};

#endif

