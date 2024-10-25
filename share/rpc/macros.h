#ifndef	__GNET_MACROS_H
#define	__GNET_MACROS_H
//define mail system macros
enum MAIL_ATTR
{
	_MA_UNREAD=0,
	_MA_ATTACH_OBJ,
	_MA_ATTACH_MONEY,
	_MA_ATTACH_BOTH,
	_MA_PRESERVE,
	_MA_TYPE_NUM,
};
#define MAIL_RESERVED ((1<<_MA_ATTACH_MONEY)|(1<<_MA_ATTACH_OBJ)|(1<<_MA_PRESERVE))
#define MAIL_ATTACHED ((1<<_MA_ATTACH_MONEY)|(1<<_MA_ATTACH_OBJ))
#define MAIL_FORCE_DELETE     110
enum AUCTION_INFORM
{
	_AUCTION_ITEM_SOLD, 
	_AUCTION_BID_WIN,
	_AUCTION_BID_LOSE,
	_AUCTION_BID_CANCEL,
	_AUCTION_BID_TIMEOUT,
};
enum MAIL_SENDER_TYPE
{
	_MST_PLAYER=0,
	_MST_NPC,
	_MST_AUCTION,
	_MST_WEB,
	_MST_BATTLE,
	_MST_TYPE_NUM,
};

enum BATTLE_INFORM
{
	_BATTLE_BONUS,           // 领地分红
	_BATTLE_WIN_PRIZE,       // 城战胜利,获得奖金
	_BATTLE_BID_FAILED,      // 竞价失败,退还押金
	_BATTLE_BID_WITHDRAW,    // 城战取消,退还押金
};

enum BATTLE_RESULT
{
	_BATTLE_ATTACKER_WIN = 1,   // 攻方获胜
	_BATTLE_DEFENDER_WIN = 2,   // 防御方获胜
	_BATTLE_TIMEOUT = 3,        // 城战超时
	_BATTLE_CANCEL = 4,         // 城战取消
};

enum BATTLE_SETREASON
{
	_BATTLE_INITIALIZE = 0,     // 初始化
	_BATTLE_SETTIME    = 1,     // 保存城战时间
	_BATTLE_DEBUG      = 2,     // 调试命令
	_BATTLE_EXTEND     = 3,     // 地图扩充
};

//define point sell macros
enum POINT_SELL_STATUS
{
	_PST_NOTSELL=0,
	_PST_SELLING,
	_PST_SOLD,
	_PST_TYPE_NUM,
};
//define faction roles
enum {
	_R_UNMEMBER = 0,
	_R_SYSTEM = 1,
	_R_MASTER = 2,
	_R_VICEMASTER = 3,
	_R_BODYGUARD = 4,
	_R_POINEER = 5,
	_R_MEMBER = 6,
}; //end of Roles

enum {
	REASON_LOGIN,
	REASON_FETCH,
};

//define chat channels
enum {
	GP_CHAT_LOCAL = 0,		//普通频道 
	GP_CHAT_WORLD,			//世界频道
	GP_CHAT_TEAM,			//队伍频道
	GP_CHAT_FACTION,		//帮派频道
	GP_CHAT_WHISPER,		//密语信息
	GP_CHAT_DAMAGE,			//伤害消息
	GP_CHAT_FIGHT,			//战斗信息
	GP_CHAT_TRADE,			//交易频道
	GP_CHAT_SYSTEM,			//系统信息
	GP_CHAT_BROADCAST,		//广播频道
	GP_CHAT_MISC,			//其他信息
	GP_CHAT_FAMILY,			//家族聊天
	GP_CHAT_CIRCLE,                	//班级聊天
	GP_CHAT_ZONE, 	                //同服聊天频道 诛仙跨服战场引入
};
//define GM operations
enum {
	GM_OP_BROADCAST = 1,	//广播
	GM_OP_LISTUSER	= 2,	//列出在线用户
	GM_OP_SHUTUP	= 3,	//禁言
	GM_OP_KICKOUT	= 4,	//踢出用户，并禁止上线
	GM_OP_RESTART	= 5,	//重启服务器
};

enum {
	CHANNEL_NORMAL    = 0,     //普通私聊
	CHANNEL_NORMALRE  = 1,     //自动回复
	CHANNEL_FRIEND    = 2,     //好友信息
	CHANNEL_FRIENDRE  = 3,     //好友回复
	CHANNEL_FORMATTED = 4,     //特殊格式消息
	CHANNEL_GMREPLY	  = 5,	   //GM对投诉的回复消息
	CHANNEL_GAMETALK  = 6,	   //GameTalk消息
	CHANNEL_GT_NORMAL  = 7,	   //发送给GameTalk和客户端密语的消息
	CHANNEL_GT_FRIEND  = 8,	   //发送给GameTalk和客户端好友的消息
};
//defeine money limit in package
#define MAX_PACKAGE_MONEY 200000000
//define reward type
#define _REWARDTYPE_INVALID 0
//define auction id
#define _AUCTIONID_INVALID 0
//define forbid complaint time
#define _FORBID_COMPLAIN 3600

//define client alive time
#define _CLIENT_TTL	180

//faction id related
#define _FACTION_ID_INVALID 0

//occupation id num
#define MAX_OCCUPATION	8
//groupids
#define	__GROUP_DEFAULT		0
#define __GROUP_BLKLIST		1
//handles
#define _HANDLE_BEGIN		-1
#define _HANDLE_END			-1
#define _HANDLE_PAGESIZE	16

//worldtag
#define _WORLDTAG_INVALID -1
//roles
#define _ROLE_INVALID		-1

//zones
#define _ZONE_INVALID		-1

//session id
#define _SID_INVALID		0

//provider id
#define _PROVIDER_ID_INVALID -1

//gameserve id
#define _GAMESERVER_ID_INVALID -1

//Trade id
#define _TRADE_ID_INVALID 0

//Trade end cause
#define _TRADE_END_TIMEOUT	0
#define _TRADE_END_NORMAL	1			

//player logout style
#define _PLAYER_LOGOUT_FULL	0
#define _PLAYER_LOGOUT_HALF	1
//user status
#define _STATUS_OFFLINE			0
#define _STATUS_READYLOGOUT		1
#define _STATUS_SELECTROLE 		2
#define _STATUS_ONLINE			3
#define _STATUS_READYGAME		4	//准备进入游戏
#define _STATUS_ONGAME			5
#define _STATUS_HIDDEN			6
#define _STATUS_READYLOGIN		7
#define _STATUS_SWITCH			8	//正在切换服务器

#define _STATUS_BUSY			8	//qq专用，忙状态
#define _STATUS_DEPART			9	//qq专用，离开状态
#define _STATUS_REMOTE_HALFLOGIN	10	//诛仙跨服战场引入
#define _STATUS_REMOTE_LOGIN		11 	//诛仙跨服战场引入
#define _STATUS_REMOTE_LOGINQUERY	12	//诛仙跨服战场引入
#define _STATUS_REMOTE_CACHERANDOM	13	//诛仙跨服战场引入

//user extended status (especially for GM)
#define _EXT_STATUS_ISGM		0X1	//是GM
#define _EXT_STATUS_ONLINE		0x2	//GM在线
#define _EXT_STATUS_CHAT		0x4 //允许与GM密语

//role status
#define  _ROLE_STATUS_NORMAL	1
#define  _ROLE_STATUS_MUSTDEL	2
#define  _ROLE_STATUS_READYDEL	3
#define  _ROLE_STATUS_FROZEN	4	//诛仙跨服战场引入

//define relationship between RoleID and Userid
#define ROLEID2USERID(rid)	(int)((rid) & 0xFFFFFFF0)
//define debug_print
#ifdef _DEBUGINFO
	#define DEBUG_PRINT Log::trace	
#else
	#define DEBUG_PRINT(...)
#endif

//define accounting attribute params
#define	_ACCOUNT_START			0
#define _ACCOUNT_STOP			1
#define _ACCOUNT_ELAPSE_TIME	2
#define _ACCOUNT_IN_OCTETS		3
#define _ACCOUNT_OUT_OCTETS		4
#define _ACCOUNT_IN_PACKETS		5
#define _ACCOUNT_OUT_PACKETS	6
#define _ACCOUNT_SYNC_TIME   	7
//define offline message type
#define _MSG_CONVERSATION	1
#define _MSG_ADDFRD_RQST	2
#define _MSG_ADDFRD_RE		3
//define client type
#define _UNKNOWN_CLIENT -1
#define _DELIVERY_CLIENT 1
#define _LINK_CLIENT 2
#define _GM_CLINET 3
#define _CONTROL_CLIENT 4

#define MAX_CASH_IN_POCKET 200000000
#define ITEM_NONTRADABLE      (1<<4)

// MASK for GetRole

#define GET_STOREHOUSE     0x00000001
#define GET_INVENTORY      0x00000002
#define GET_TASK           0x00000004
#define GET_TASKCOMPLETE   0x00000008
#define GET_TASKINVENTORY  0x00000010
#define GET_USERFACTION    0x00000020
#define GET_ALL            0x0000003F

#define PUT_STOREHOUSE     0x00000001
#define PUT_INVENTORY      0x00000002
#define PUT_TASK           0x00000004
#define PUT_TASKCOMPLETE   0x00000008
#define PUT_TASKINVENTORY  0x00000010
#define PUT_EQUIPMENT      0x00000020
#define PUT_MONEY          0x00000040
#define PUT_ALL            0x0000007F
#define PUT_TRADE_TIMEOUT  (PUT_ALL^(PUT_INVENTORY | PUT_MONEY))
#define PUT_BID_TIMEOUT    (PUT_ALL^PUT_MONEY)
#define PUT_MAIL_SYNC      (PUT_MONEY|PUT_INVENTORY|PUT_STOREHOUSE)
#define PUT_HOMESTORAGE_SYNC      (PUT_MONEY|PUT_INVENTORY|PUT_STOREHOUSE)
#define PUT_BID_SYNC       (PUT_MONEY)

#define QPUT_STOREHOUSE     0x00000001
#define QPUT_INVENTORY      0x00000002
#define QPUT_TASK           0x00000004
#define QPUT_CASH           0x00000008
#define QPUT_ALL            0x0000000F
#define QPUT_SYNC           (DBMASK_PUT_INVENTORY|DBMASK_PUT_STOREHOUSE|DBMASK_PUT_CASH)
#define QPUT_SYNC_TIMEOUT   (DBMASK_PUT_ALL^DBMASK_PUT_INVENTORY)

// MASK for GetMoneyInventory
#define DBMASK_GET_MONEY          0x00000001
#define DBMASK_GET_INVENTORY      0x00000002
#define DBMASK_GET_ALL            0x00000003

#endif
