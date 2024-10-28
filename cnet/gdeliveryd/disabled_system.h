
#ifndef __GNET_DISABLED_SYSTEM_H__
#define __GNET_DISABLED_SYSTEM_H__

#include <stddef.h>

namespace GNET
{

enum
{
	SYS_AUCTION = 0,	//拍卖
	SYS_STOCK,			//元宝委托交易
	SYS_BATTLE,			//城战
	SYS_SYSAUCTION,	//元宝拍卖
	SYS_WEBTRADE,	//寻宝网
	SYS_GAMETALK,	//GT
	SYS_SNS,		//SNS
	SYS_FACTIONFORTRESS,	//帮派基地
	SYS_COUNTRYBATTLE,		//国战
	SYS_REFERENCE,			//线上推广
	SYS_REWARD,				//消费返利
	SYS_RECALLOLDPLAYER,	//召回老玩家
	SYS_KINGELECTION,		//国王选举
	SYS_PLAYERSHOP,			//寄卖系统
	SYS_PLAYERPROFILE,		//情迷祖龙
	SYS_AUTOTEAM,			//自动组队
	SYS_UNIQUEDATAMAN,		//全局数据
	SYS_TANKBATTLE,			//战车战场
	SYS_FACTIONRESOURCEBATTLE, //帮派PVP
    SYS_MAPPASSWORD,        // 密保卡
    SYS_SOLOCHALLENGERANK,  // 单人副本排行榜
	SYS_MNFACTIONBATTLE,	//跨服帮战 
	SYS_FORBIDCROSS,		//参加跨服活动

	SYS_INDEX_MAX,
};	

class DisabledSystem
{
	bool _disabled_list[SYS_INDEX_MAX];
	
	DisabledSystem()
	{ 
		for(size_t i=0; i<SYS_INDEX_MAX; i++)
			_disabled_list[i] = false;
	}
	static DisabledSystem instance;
public:
	static bool GetDisabled(int index)
	{
		if(index >=0 && index < SYS_INDEX_MAX)
			return instance._disabled_list[index]; 
		else
			return true;
	}
	static void SetDisabled(int index)
	{
		if(index >=0 && index < SYS_INDEX_MAX)
			instance._disabled_list[index] = true; 
	}
};

}
#endif
