#ifndef __ONLINEGAME_GS_CONFIG_H__
#define __ONLINEGAME_GS_CONFIG_H__

#include "dbgprt.h"

#define GL_MAX_MATTER_COUNT 	164000		//不可超过65536*16
#define GL_MAX_NPC_COUNT 	164000		//不可超过65536*16
#define GL_MAX_PLAYER_COUNT 	8192
#define MAX_PLAYER_IN_WORLD	(4096*4)
#define MAX_GS_NUM		1024


#define ITEM_LIST_BASE_SIZE	32		//最初的包裹栏大小
#define ITEM_LIST_MAX_SIZE	80		//最大的包裹大小
//#define EQUIP_LIST_SIZE		13	//每个槽位的定义看item.h
#define TASKITEM_LIST_SIZE	32
//#define TRASHBOX_LIST_SIZE	48
#define TRASHBOX_BASE_SIZE	16		//普通仓库初始大小
#define TRASHBOX_BASE_SIZE4	120		//卡牌仓库初始大小
#define TRASHBOX_MAX_SIZE	200
#define GRID_SIGHT_RANGE	60.f		//视野距离

#define PICKUP_DISTANCE		10.f
#define MAGIC_CLASS		5
#define MAX_MAGIC_FACTION	50
#define MAX_MESSAGE_LATENCY	256*20
#define MAX_AGGRO_ENTRY		50
#define MAX_SOCKET_COUNT	4
#define MAX_SPAWN_COUNT		10240
#define MAX_MATTER_SPAWN_COUNT  10240	
#define MAX_PLAYER_SESSION	64
#define NORMAL_COMBAT_TIME	5		//通常战斗时间，使被人攻击后的时间
#define MAX_COMBAT_TIME		15		//最大战斗时间，是攻击别人所产生的时间	
//#define MAX_COMBAT_PROFIT_TIME	12*60*60//最大战斗收益时间,固定为8小时
#define MAX_ACTIVE_STATE_DELAY	12	//战斗结束或停止移动后持续多少秒维持消耗收益时间
#define MAX_HURT_ENTRY		512		//伤害列表中最多的条目数目
#define LOGOUT_TIME_IN_NORMAL   3		//3秒普通的登出时间
#define LOGOUT_TIME_IN_COMBAT   15		//15秒登出时间
#define LOGOUT_TIME_IN_TRAVEL   30		//30秒旅行登出时间

#define NPC_IDLE_TIMER		20		//20秒进入IDLE状态
#define NPC_IDLE_HEARTBEAT	30		//每60次普通心跳进行一次idle状态的心跳
#define LEAVE_IDLE_MSG_LIMIT	40		//40个tick发送一次，即限制为2秒
#define TICK_PER_SEC		20		//一秒钟有多少个tick，这个值不能随意变动

#define GET_EQUIP_INFO_DIS	200.f		//取得装备信息的限制距离（水平方向）
#define TEAM_INVITE_DIS		100.f		//邀请的限制距离
#define TEAM_EXP_DISTANCE	100.f		//组队经验的限制距离
#define TEAM_ITEM_DISTANCE	100.f		//组队随机分配物品的距离
#define NORMAL_EXP_DISTANCE	100.f		//组队经验的限制距离
#define TEAM_MEMBER_CAPACITY	10		//组队的最大人数
#define TEAM_WAIT_TIMEOUT	5		//5秒，即5个心跳
#define TEAM_INVITE_TIMEOUT	30		//邀请超时时间 30 秒, 邀请的超时
#define TEAM_INVITE_TIMEOUT2	25		//邀请超时时间  被邀请方的超时,应该小于上面的邀请方超时
#define TEAM_LEADER_TIMEOUT	30		//三十秒member没有收到来自队长的消息，即超时
#define TEAM_MEMBER_TIMEOUT	15		//15秒没受到队员的消息，即超时
#define TEAM_LEADER_UPDATE_INTERVAL 20		//每隔20秒leader更新所有数据
#define TEAM_LEADER_NOTIFY_TIME	10		//每隔10秒队长会通知所有队员自己仍在的信息
#define MAX_PROVIDER_PER_NPC	48		//每个NPC最多提供服务的种类
#define DURABILITY_UNIT_COUNT	100		//外部显示的一个耐久度单位对应内部的值
#define DURABILITY_DEC_PER_HIT 	25		//防具每次被击中减少的耐久值
#define DURABILITY_DEC_PER_ATTACK 2		//武器每次攻击减少的耐久值
#define TOSSMATTER_USE_TIME	40		//暗器的使用时间固定是1.5秒
#define MAX_TOWN_REGION		1024
#define LINK_NOTIFY_TIMER	33
#define MAX_EXTEND_OBJECT_STATE 32		//每个对象最多同时存在的扩展属性数目
#define UNARMED_ATTACK_DELAY	12		//空手的攻击延迟时间的是0.3秒
#define HELP_RANGE_FACTOR	1.0f		//求救距离和视野距离的因子
#define DEFAULT_AGGRESSIVE_RANGE 15.f		//标准的最大索敌范围，参数里可以重新设定
#define MAX_INVADER_ENTRY_COUNT 10		//最多纪录多少给橙名对象
#define MAX_PLAYER_ENEMY_COUNT  20		//最大的玩家敌人数目
#define PARIAH_TIME_PER_KILL    7200		//每次杀人变成的红名累计时间
#define PARIAH_TIME_REDUCE	72		//杀一只高级怪所减少pk值
#define MAX_PARIAH_TIME		(PARIAH_TIME_PER_KILL*100)	//最大的PK值
#define LOW_PROTECT_LEVEL	9		//被保护的低级级别
#define PVP_PROTECT_LEVEL	29		//被保护的非PVP级别
#define MATTER_HEARTBEAT_SEC	11		//物品每多少秒一次心跳
//#define GATHER_RANGE_LIMIT	4.f		//采集矿物的距离限制,废弃，改成配置文件设置 
#define TRASHBOX_MONEY_CAPACITY	2000000000	//仓库里最大金钱数量
#define MONEY_CAPACITY_BASE	200000000
#define MONEY_CAPACITY_PER_LVL	200000000
#define MAX_ITEM_DROP_COUNT	20		//掉落物品时每次最多掉落的数目
#define MONEY_DROP_RATE		0.7f		//金钱掉落概率
#define MONEY_MATTER_ID		3044		//金钱的掉落物品id
#define REVIVE_SCROLL_ID	3043		//复活卷轴的物品id
#define REVIVE_SCROLL_ID2	32021		//复活卷轴的物品id
#define FLEE_SKILL_ID 		40		//怪物逃跑的技能ID
#define ITEM_DESTROYING_ID	12332		//摧毁绑定物品时的临时ID
#define SUICIDE_ATTACK_SKILL_ID	147		//自爆攻击的技能ID
#define ITEM_POPPET_DUMMY_ID	12361		//替身娃娃的ID 12361
#define ITEM_POPPET_DUMMY_ID2	31878		//替身娃娃的ID 31878
#define ITEM_POPPET_DUMMY_ID3	36309		//替身娃娃的ID 36309
#define WORLD_SPEAKER_ID	12979		//世界千里传音的物品ID
#define WORLD_SPEAKER_ID2	36092		//世界千里传音的物品ID
#define SUPERWORLD_SPEAKER_ID	27728	//超级世界千里传音的物品ID
#define SUPERWORLD_SPEAKER_ID2	27729	//超级世界千里传音的物品ID2
#define ALLSPARK_ID		12980		//精炼等级转换符（乾坤石）ID
#define MAKE_SLOT_ITEM_ID	21043		//打孔道具
#define MAKE_SLOT_ITEM_ID2	34232		//打孔道具2
#define STAYIN_BONUS		100		//打坐的加成
#define PLAYER_BODYSIZE		0.3f		//玩家的体型大小
#define MAX_MASTER_MINOR_RANGE	400.f		
#define BASE_REBORN_TIME	15		//至少5秒复生时间
#define NPC_FOLLOW_TARGET_TIME	0.5f		//现在试验一下NPC追击的最小时间
#define NPC_FLEE_TIME		0.5f		//现在试验一下NPC逃跑的最小时间
#define MAX_FLIGHT_SPEED	20.f		//最大飞行速度
#define MAX_RUN_SPEED		15.f		//最大跑动速度
#define MAX_WALK_SPEED		8.f		//最大行走速度
#define MIN_RUN_SPEED		0.1f		//最小跑动速度
#define MIN_WALK_SPEED		0.1f		//最小行走速度
#define MAX_JUMP_SPEED		10.f		//最大跳跃速度
#define NPC_PATROL_TIME		1.0f
//#define PLAYER_MARKET_SLOT_CAP	24
#define PLAYER_MARKET_SELL_SLOT 		12	//未装备摆摊凭证时允许值
#define PLAYER_MARKET_BUY_SLOT 			12
#define PLAYER_MARKET_NAME_LEN 			28
#define PLAYER_MARKET_MAX_SELL_SLOT 	20	//装备了摆摊凭证时最大允许值
#define PLAYER_MARKET_MAX_BUY_SLOT 		20
#define PLAYER_MARKET_MAX_NAME_LEN 		62
#define WANMEI_YINPIAO_ID	21652		//完美银票，价值千万
#define WANMEI_YINPIAO_PRICE 10000000u	//完美银票价格
#define PVP_DAMAGE_REDUCE	0.25f
#define MAX_PLAYER_LEVEL	150		//物理最大级别，不要随意调整
#define MAX_WAYPOINT_COUNT	1024		//最大的路点数目
#define NPC_REBORN_PASSIVE_TIME 5		//怪物重生后进入主动的等待时间
#define PVP_STATE_COOLDOWN	(10*3600)
#define WATER_BREATH_MARK	3.0f		//水深两米以下才启动呼吸
#define MAX_PLAYER_EFFECT_COUNT 32
#define MAX_MULTIOBJ_EFFECT_COUNT 3
#define PLAYER_REBORN_PROTECT	5		//玩家复活后获得几秒钟的放逐时间(不能操作，不能治疗，不能运动)
#define CRIT_DAMAGE_BONUS	2.0f		//重击产生的伤害加成
#define CRIT_DAMAGE_BONUS_PERCENT 200	//重击产生的伤害加成
#define PLAYER_HP_GEN_FACTOR	5		//玩家回血的因子
#define PLAYER_MP_GEN_FACTOR	10		//玩家回魔的因子
#define MAX_USERNAME_LENGTH	40		//玩家名字的最大长度
#define PVP_COMBAT_HIGH_TH	300		//PVP格斗时间的设置
#define PVP_COMBAT_LOW_TH	150		//PVP格斗时间小于一定值才会进行反馈刷新
#define MAX_DOUBLE_EXP_TIME	(4*3600)	//最大积累双倍时间
#define MIN_TEAM_DISEXP_LEVEL	20		//进行经验分配时的 最小组队等效级别
#define DOUBLE_EXP_FACTOR	1.5f		//双倍经验时 经验和sp的乘法因子
#define TASK_CHAT_MESSAGE_ID	24		//任务喊话的系统格式化广播
#define DPS_MAN_CHAT_MSG_ID		101		//沙包排名喊话的系统格式化广播
#define TITLE_RARE_CHAT_MSG_ID	102		//稀有称号获得喊话系统格式化广播 
#define FAC_RENAME_CHAT_MSG_ID  103	    //gamed 帮派改名占用

#define ROLE_REPUTATION_UCHAR_SIZE	256		 
#define ROLE_REPUTATION_USHORT_SIZE	64		 
#define ROLE_REPUTATION_UINT_SIZE	32		 

#define ANTI_INVISIBLE_CONSTANT	0	//人物的反隐等级常数

#define WEDDING_CONFIG_ID		801		//婚礼配置的id	
#define WEDDING_CANCELBOOK_FEE	3000000	//取消婚礼预约的费用
#define WEDDING_SCENE_COUNT		2		//目前只有1个婚礼场景,最多支持10个,由婚礼配置限定

#define DEFAULT_RESURRECT_HP_FACTOR 0.1f	//默认的死亡后回血比例
#define DEFAULT_RESURRECT_MP_FACTOR 0.1f	//默认的死亡后回蓝比例

#define FACTION_FORTRESS_CONFIG_ID 854

#define CONGREGATE_REQUEST_TIMEOUT		120	//集结请求120秒超时
#define TOTAL_SEC_PER_DAY			24*60*60//一天内有多少秒
#define PRODUCE4_CHOOSE_ITEM_TIME		30 //新继承生产等待客户端进行选择的时间
#define ONLINE_AWARD_CONFIG_ID		1023	
#define COUNTRYJOIN_LEVEL_REQUIRED	60
#define COUNTRYJOIN_APPLY_TICKET	36672	
#define COUNTRYBATTLE_CONFIG_ID		1027
#define COUNTRY_CHAT_FEE			10000
#define EQUIP_SIGNATURE_CLEAN_CONSUME	5	//清除装备签名消耗的墨水数量最低值

#define PET_ADDEXP_ITEM				37401   //宠物喂养消耗的物品
#define PET_EVOLUTION_ITEM1			37401   //宠物进化相关操作消耗的物品1
#define PET_EVOLUTION_ITEM2			12980	//宠物进化相关操作消耗的物品2
#define PET_REBUILD_TIME				1	//宠物性格培训，洗髓消耗的时间
#define PET_CHOOSE_REBUILD_RESULT_TIME	30	//宠物性格培训洗髓等待客户端选择的时间 
#define PET_EVOLVE_CONFIG_ID		1038	//宠物进化模板id

#define CHANGEDS_LEVEL_REQUIRED		100
#define CHANGEDS_SEC_LEVEL_REQUIRED	20

#define PLAYER_RENAME_ITEM_ID       37302   //玩家进行角色重命名需要消耗的物品id
#define PLAYER_RENAME_ITEM_ID_2     46901   //玩家进行角色重命名需要消耗的物品id2

#define MERIDIAN_REFINE_COST1		38142	//经脉免费冲击所需的物品
#define MERIDIAN_REFINE_COST2		38143
#define MERIDIAN_REFINE_COST3		38144
#define MERIDIAN_PAID_REFINE_COST1		42328 //经脉付费冲击所需要的物品
#define MERIDIAN_PAID_REFINE_COST2		38145
#define MERIDIAN_PAID_REFINE_COST3		38146
#define MERIDIAN_PAID_REFINE_COST4		38147
#define MERIDIAN_MAX_PAID_REFINE_TIMES 100   //经脉付费冲击的最大次数
#define MERIDIAN_INC_PAID_REFINE_TIMES 50	//每天增加的付费冲击次数
#define MERIDIAN_MAX_REFINE_LEVEL 		80
#define MERIDIAN_TRIGRAMS_SIZE		48
#define TOUCH_SHOP_CONFIG_ID    1290
#define HISTORY_ADVANCE_CONFIG_ID    1425
#define FACTION_PVP_CONFIG_ID   1740

#define FACTION_FORTRESS_RESET_TECH_ITEM_ID	39202	//帮派基地重置科技点数消耗物品id
#define TRICKBATTLE_LEVEL_REQUIRED		100
#define TRICKBATTLE_SEC_LEVEL_REQUIRED	20
#define TRICKBATTLE_CONFIG_ID			1444
#define GENERALCARD_MAX_COLLECTION		512		//卡牌最大收藏数
#define GENERALCARD_MAX_REBIRTH_TIMES	2		//卡牌最大转生次数

#define AUTO_SUPPORT_STONE1 36764 //养生石1
#define AUTO_SUPPORT_STONE2 36765 //养生石2
#define AUTO_SUPPORT_STONE3 36766 //养生石3
#define AUTO_SUPPORT_STONE4 36767 //养生石4

#define PLAYER_FATE_RING_TOTAL			6	//命轮数量
#define PLAYER_FATE_RING_MAX_LEVEL		10	//命轮的最高等级
#define PLAYER_FATE_RING_GAIN_PER_WEEK	20	//命轮每周采集元魂的上限
#define MATTER_ITEM_SOUL_LIFE	30	//采集掉落的元魂物品生命(秒)

#define AUTO_TEAM_JUMP_ITEM1 41542
#define AUTO_TEAM_JUMP_ITEM2 41543
#define AUTO_TEAM_JUMP_ITEM3 41544
#define AUTO_TEAM_JUMP_ITEM4 41545

#define GT_CONFIG_ID 1637
#define MAFIA_PVP_CTRLID 3624
#define MAX_VISIBLE_STATE_COUNT	192

#define FACTION_RENAME_ITEM_1	46903
#define FACTION_RENAME_ITEM_2	46902
#define MAX_NPC_GOLDSHOP_SLOT	48

enum eFESTIVE_AWARD_TYPE
{
	FAT_MAFIA_PVP,
};

#endif
