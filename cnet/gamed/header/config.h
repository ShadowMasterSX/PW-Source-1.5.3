#ifndef __ONLINEGAME_GS_CONFIG_H__
#define __ONLINEGAME_GS_CONFIG_H__

#define MAX_MATTER_COUNT 	64000		//不可超过65536*16
#define MAX_NPC_COUNT 		164000		//不可超过65536*16
#define MAX_PLAYER_COUNT 	4096
#define MAX_CS_NUM		1024
#define MAX_GS_NUM		1024


#define ITEM_LIST_SIZE		50
//#define EQUIP_LIST_SIZE		13	//每个槽位的定义看item.h
#define TASKITEM_LIST_SIZE	20

#define PICKUP_DISTANCE		10.f
#define MAGIC_CLASS		5
#define MAX_MAGIC_FACTION	50
#define MAX_MESSAGE_LATENCY	256
#define MAX_AGGRO_ENTRY		50
#define MAX_SOCKET_COUNT	4
#define MAX_SPAWN_COUNT		10240
#define MAX_PLAYER_SESSION	64
#define NORMAL_COMBAT_TIME	5
#define MAX_COMBAT_TIME		30
#define MAX_HURT_ENTRY		512		//伤害列表中最多的条目数目
#define LOGOUT_TIME_IN_COMBAT   15		//15秒登出时间

#define NPC_IDLE_TIMER		20		//20秒进入IDLE状态
#define NPC_IDLE_HEARTBEAT	30		//每60次普通心跳进行一次idle状态的心跳
#define LEAVE_IDLE_MSG_LIMIT	40		//40个tick发送一次，即限制为2秒
#define TICK_PER_SEC		20		//一秒钟有多少个tick，这个值不能随意变动

#define GET_EQUIP_INFO_DIS	150.f		//取得装备信息的限制距离（水平方向）
#define TEAM_INVITE_DIS		50.f		//邀请的限制距离
#define TEAM_EXP_DISTANCE	100.f		//组队经验的限制距离
#define TEAM_ITEM_DISTANCE	100.f		//组队随机分配物品的距离
#define NORMAL_EXP_DISTANCE	100.f		//组队经验的限制距离
#define TEAM_MEMBER_CAPACITY	10
#define TEAM_WAIT_TIMEOUT	5		//5秒，即5个心跳
#define TEAM_INVITE_TIMEOUT	30		//邀请超时时间 30 秒, 邀请的超时
#define TEAM_INVITE_TIMEOUT2	25		//邀请超时时间  被邀请方的超时,应该小于上面的邀请方超时
#define TEAM_LEADER_TIMEOUT	30		//三十秒member没有收到来自队长的消息，即超时
#define TEAM_MEMBER_TIMEOUT	15		//15秒没受到队员的消息，即超时
#define TEAM_LEADER_UPDATE_INTERVAL 20		//每隔20秒leader更新所有数据
#define TEAM_LEADER_NOTIFY_TIME	10		//每隔10秒队长会通知所有队员自己仍在的信息
#define MAX_PROVIDER_PER_NPC	32
#define MAX_SERVICE_DISTANCE	10.f		//最大的服务距离为 10.f 米
#define PURCHASE_PRICE_SCALE	0.3f		//物品买给NPC的价格比例
#define DURABILITY_UNIT_COUNT	100		//外部显示的一个耐久度单位对应内部的值
#define REPAIR_PRICE_FACTOR	0.1f		//修理时基础价格上乘以的因子
#define TOSSMATTER_USE_TIME	40		//暗器的使用时间固定是1.5秒
#define MAX_TOWN_REGION		1024
#define LINK_NOTIFY_TIMER	33
#define MAX_EXTEND_OBJECT_STATE 32		//每个对象最多同时存在的扩展属性数目
#define UNARMED_ATTACK_DELAY	12		//空手的攻击延迟时间的是0.3秒
#define HELP_RANGE_FACTIOR	2.0f		//求救距离和视野距离的因子
#define MAX_AGGRESSIVE_RANGE	10.f		//最大的主动npc索敌范围
#define MAX_INVADER_ENTRY_COUNT 10		//最多纪录多少给橙名对象

#endif
