#ifndef __GNET_GAMEMASTER_H
#define __GNET_GAMEMASTER_H

#include <map>

namespace GNET
{

class GameMaster
{
public:
	enum
	{
		PRV_TOGGLE_NAMEID	= 0,	//切换玩家名字与ID
		PRV_HIDE_BEGOD		= 1,	//进入隐身或无敌状态
		PRV_ONLINE_ORNOT	= 2,	//切换是否在线
		PRV_CHAT_ORNOT		= 3,	//切换是否可以密语
		PRV_MOVETO_ROLE		= 4,	//移动到指定角色身边
		PRV_FETCH_ROLE		= 5,	//将指定角色召唤到GM身边
		PRV_MOVE_ASWILL		= 6,	//移动到指定位置
		PRV_MOVETO_NPC		= 7,	//移动到指定NPC位置
		PRV_MOVETO_MAP		= 8,	//移动到指定地图（副本）
		PRV_ENHANCE_SPEED	= 9,	//移动加速
		PRV_FOLLOW		= 10,	//跟随玩家
		PRV_LISTUSER		= 11,	//获取在线玩家列表
		PRV_FORCE_OFFLINE	= 100,	//强制玩家下线，并禁止在一定时间上线
		PRV_FORBID_TALK		= 101,	//禁言
		PRV_FORBID_TRADE	= 102,	//禁止玩家间、玩家与NPC交易，仅针对一个玩家
		PRV_FORBID_SELL		= 103,	//禁卖
		PRV_BROADCAST		= 104,	//系统广播
		PRV_SHUTDOWN_GAMESERVER	= 105,	//关闭游戏服务器
		PRV_SUMMON_MONSTER	= 200,	//召唤怪物
		PRV_DISPEL_SUMMON	= 201,	//驱散被召唤物体
		PRV_PRETEND		= 202,	//伪装
		PRV_GMMASTER		= 203,	//GM管理员
	};

	enum
	{
		STATUS_SILENT      = 0x01,
	};

	int userid;
	unsigned int linksid;
	unsigned int localsid;
	int status;
	ByteVector privileges;
	GameMaster(int _uid,unsigned int _link,unsigned int _local, ByteVector& _rights) : userid(_uid), linksid(_link), 
		localsid(_local), status(0), privileges(_rights)
	{
	}
	GameMaster(const GameMaster& r) : userid(r.userid), linksid(r.linksid), localsid(r.localsid), status(r.status),
		privileges(r.privileges)	
	{
	}
	~GameMaster() {}

	bool IsAuthorized(unsigned char right)
	{
		for(ByteVector::iterator it=privileges.begin();it!=privileges.end();++it)
			if(*it==right)
				return true;
		return false;
	}
};

class PrivateChat;
class MasterContainer
{
public:
	typedef std::map<int/*roleid*/,GameMaster> MasterMap;
	MasterMap  masters;

	static MasterContainer & Instance() { static MasterContainer instance; return instance; }

	GameMaster* Find( int roleid )
	{
		MasterMap::iterator it = masters.find(roleid);
		if(it!=masters.end())
			return &(it->second);
		return NULL;
	}

	void Insert(int userid, int roleid, int linksid, int localsid, ByteVector& privileges);
	void Erase(int roleid);
	bool Authenticate(int roleid, unsigned int sid, unsigned char right);
	void Broadcast(PrivateChat& data, int &roleid);
	bool SetSilent(int roleid, bool set);
	
};

};

#endif

