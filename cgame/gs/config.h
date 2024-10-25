#ifndef __ONLINEGAME_GS_CONFIG_H__
#define __ONLINEGAME_GS_CONFIG_H__

#include "dbgprt.h"

#define GL_MAX_MATTER_COUNT 	164000		//���ɳ���65536*16
#define GL_MAX_NPC_COUNT 	164000		//���ɳ���65536*16
#define GL_MAX_PLAYER_COUNT 	8192
#define MAX_PLAYER_IN_WORLD	(4096*4)
#define MAX_GS_NUM		1024


#define ITEM_LIST_BASE_SIZE	32		//����İ�������С
#define ITEM_LIST_MAX_SIZE	80		//���İ�����С
//#define EQUIP_LIST_SIZE		13	//ÿ����λ�Ķ��忴item.h
#define TASKITEM_LIST_SIZE	32
//#define TRASHBOX_LIST_SIZE	48
#define TRASHBOX_BASE_SIZE	16		//��ͨ�ֿ��ʼ��С
#define TRASHBOX_BASE_SIZE4	120		//���Ʋֿ��ʼ��С
#define TRASHBOX_MAX_SIZE	200
#define GRID_SIGHT_RANGE	60.f		//��Ұ����

#define PICKUP_DISTANCE		10.f
#define MAGIC_CLASS		5
#define MAX_MAGIC_FACTION	50
#define MAX_MESSAGE_LATENCY	256*20
#define MAX_AGGRO_ENTRY		50
#define MAX_SOCKET_COUNT	4
#define MAX_SPAWN_COUNT		10240
#define MAX_MATTER_SPAWN_COUNT  10240	
#define MAX_PLAYER_SESSION	64
#define NORMAL_COMBAT_TIME	5		//ͨ��ս��ʱ�䣬ʹ���˹������ʱ��
#define MAX_COMBAT_TIME		15		//���ս��ʱ�䣬�ǹ���������������ʱ��	
//#define MAX_COMBAT_PROFIT_TIME	12*60*60//���ս������ʱ��,�̶�Ϊ8Сʱ
#define MAX_ACTIVE_STATE_DELAY	12	//ս��������ֹͣ�ƶ������������ά����������ʱ��
#define MAX_HURT_ENTRY		512		//�˺��б���������Ŀ��Ŀ
#define LOGOUT_TIME_IN_NORMAL   3		//3����ͨ�ĵǳ�ʱ��
#define LOGOUT_TIME_IN_COMBAT   15		//15��ǳ�ʱ��
#define LOGOUT_TIME_IN_TRAVEL   30		//30�����еǳ�ʱ��

#define NPC_IDLE_TIMER		20		//20�����IDLE״̬
#define NPC_IDLE_HEARTBEAT	30		//ÿ60����ͨ��������һ��idle״̬������
#define LEAVE_IDLE_MSG_LIMIT	40		//40��tick����һ�Σ�������Ϊ2��
#define TICK_PER_SEC		20		//һ�����ж��ٸ�tick�����ֵ��������䶯

#define GET_EQUIP_INFO_DIS	200.f		//ȡ��װ����Ϣ�����ƾ��루ˮƽ����
#define TEAM_INVITE_DIS		100.f		//��������ƾ���
#define TEAM_EXP_DISTANCE	100.f		//��Ӿ�������ƾ���
#define TEAM_ITEM_DISTANCE	100.f		//������������Ʒ�ľ���
#define NORMAL_EXP_DISTANCE	100.f		//��Ӿ�������ƾ���
#define TEAM_MEMBER_CAPACITY	10		//��ӵ��������
#define TEAM_WAIT_TIMEOUT	5		//5�룬��5������
#define TEAM_INVITE_TIMEOUT	30		//���볬ʱʱ�� 30 ��, ����ĳ�ʱ
#define TEAM_INVITE_TIMEOUT2	25		//���볬ʱʱ��  �����뷽�ĳ�ʱ,Ӧ��С����������뷽��ʱ
#define TEAM_LEADER_TIMEOUT	30		//��ʮ��memberû���յ����Զӳ�����Ϣ������ʱ
#define TEAM_MEMBER_TIMEOUT	15		//15��û�ܵ���Ա����Ϣ������ʱ
#define TEAM_LEADER_UPDATE_INTERVAL 20		//ÿ��20��leader������������
#define TEAM_LEADER_NOTIFY_TIME	10		//ÿ��10��ӳ���֪ͨ���ж�Ա�Լ����ڵ���Ϣ
#define MAX_PROVIDER_PER_NPC	48		//ÿ��NPC����ṩ���������
#define DURABILITY_UNIT_COUNT	100		//�ⲿ��ʾ��һ���;öȵ�λ��Ӧ�ڲ���ֵ
#define DURABILITY_DEC_PER_HIT 	25		//����ÿ�α����м��ٵ��;�ֵ
#define DURABILITY_DEC_PER_ATTACK 2		//����ÿ�ι������ٵ��;�ֵ
#define TOSSMATTER_USE_TIME	40		//������ʹ��ʱ��̶���1.5��
#define MAX_TOWN_REGION		1024
#define LINK_NOTIFY_TIMER	33
#define MAX_EXTEND_OBJECT_STATE 32		//ÿ���������ͬʱ���ڵ���չ������Ŀ
#define UNARMED_ATTACK_DELAY	12		//���ֵĹ����ӳ�ʱ�����0.3��
#define HELP_RANGE_FACTOR	1.0f		//��Ⱦ������Ұ���������
#define DEFAULT_AGGRESSIVE_RANGE 15.f		//��׼��������з�Χ����������������趨
#define MAX_INVADER_ENTRY_COUNT 10		//����¼���ٸ���������
#define MAX_PLAYER_ENEMY_COUNT  20		//������ҵ�����Ŀ
#define PARIAH_TIME_PER_KILL    7200		//ÿ��ɱ�˱�ɵĺ����ۼ�ʱ��
#define PARIAH_TIME_REDUCE	72		//ɱһֻ�߼���������pkֵ
#define MAX_PARIAH_TIME		(PARIAH_TIME_PER_KILL*100)	//����PKֵ
#define LOW_PROTECT_LEVEL	9		//�������ĵͼ�����
#define PVP_PROTECT_LEVEL	29		//�������ķ�PVP����
#define MATTER_HEARTBEAT_SEC	11		//��Ʒÿ������һ������
//#define GATHER_RANGE_LIMIT	4.f		//�ɼ�����ľ�������,�������ĳ������ļ����� 
#define TRASHBOX_MONEY_CAPACITY	2000000000	//�ֿ�������Ǯ����
#define MONEY_CAPACITY_BASE	200000000
#define MONEY_CAPACITY_PER_LVL	200000000
#define MAX_ITEM_DROP_COUNT	20		//������Ʒʱÿ�����������Ŀ
#define MONEY_DROP_RATE		0.7f		//��Ǯ�������
#define MONEY_MATTER_ID		3044		//��Ǯ�ĵ�����Ʒid
#define REVIVE_SCROLL_ID	3043		//����������Ʒid
#define REVIVE_SCROLL_ID2	32021		//����������Ʒid
#define FLEE_SKILL_ID 		40		//�������ܵļ���ID
#define ITEM_DESTROYING_ID	12332		//�ݻٰ���Ʒʱ����ʱID
#define SUICIDE_ATTACK_SKILL_ID	147		//�Ա������ļ���ID
#define ITEM_POPPET_DUMMY_ID	12361		//�������޵�ID 12361
#define ITEM_POPPET_DUMMY_ID2	31878		//�������޵�ID 31878
#define ITEM_POPPET_DUMMY_ID3	36309		//�������޵�ID 36309
#define WORLD_SPEAKER_ID	12979		//����ǧ�ﴫ������ƷID
#define WORLD_SPEAKER_ID2	36092		//����ǧ�ﴫ������ƷID
#define SUPERWORLD_SPEAKER_ID	27728	//��������ǧ�ﴫ������ƷID
#define SUPERWORLD_SPEAKER_ID2	27729	//��������ǧ�ﴫ������ƷID2
#define ALLSPARK_ID		12980		//�����ȼ�ת������Ǭ��ʯ��ID
#define MAKE_SLOT_ITEM_ID	21043		//��׵���
#define MAKE_SLOT_ITEM_ID2	34232		//��׵���2
#define STAYIN_BONUS		100		//�����ļӳ�
#define PLAYER_BODYSIZE		0.3f		//��ҵ����ʹ�С
#define MAX_MASTER_MINOR_RANGE	400.f		
#define BASE_REBORN_TIME	15		//����5�븴��ʱ��
#define NPC_FOLLOW_TARGET_TIME	0.5f		//��������һ��NPC׷������Сʱ��
#define NPC_FLEE_TIME		0.5f		//��������һ��NPC���ܵ���Сʱ��
#define MAX_FLIGHT_SPEED	20.f		//�������ٶ�
#define MAX_RUN_SPEED		15.f		//����ܶ��ٶ�
#define MAX_WALK_SPEED		8.f		//��������ٶ�
#define MIN_RUN_SPEED		0.1f		//��С�ܶ��ٶ�
#define MIN_WALK_SPEED		0.1f		//��С�����ٶ�
#define MAX_JUMP_SPEED		10.f		//�����Ծ�ٶ�
#define NPC_PATROL_TIME		1.0f
//#define PLAYER_MARKET_SLOT_CAP	24
#define PLAYER_MARKET_SELL_SLOT 		12	//δװ����̯ƾ֤ʱ����ֵ
#define PLAYER_MARKET_BUY_SLOT 			12
#define PLAYER_MARKET_NAME_LEN 			28
#define PLAYER_MARKET_MAX_SELL_SLOT 	20	//װ���˰�̯ƾ֤ʱ�������ֵ
#define PLAYER_MARKET_MAX_BUY_SLOT 		20
#define PLAYER_MARKET_MAX_NAME_LEN 		62
#define WANMEI_YINPIAO_ID	21652		//������Ʊ����ֵǧ��
#define WANMEI_YINPIAO_PRICE 10000000u	//������Ʊ�۸�
#define PVP_DAMAGE_REDUCE	0.25f
#define MAX_PLAYER_LEVEL	150		//������󼶱𣬲�Ҫ�������
#define MAX_WAYPOINT_COUNT	1024		//����·����Ŀ
#define NPC_REBORN_PASSIVE_TIME 5		//������������������ĵȴ�ʱ��
#define PVP_STATE_COOLDOWN	(10*3600)
#define WATER_BREATH_MARK	3.0f		//ˮ���������²���������
#define MAX_PLAYER_EFFECT_COUNT 32
#define MAX_MULTIOBJ_EFFECT_COUNT 3
#define PLAYER_REBORN_PROTECT	5		//��Ҹ�����ü����ӵķ���ʱ��(���ܲ������������ƣ������˶�)
#define CRIT_DAMAGE_BONUS	2.0f		//�ػ��������˺��ӳ�
#define CRIT_DAMAGE_BONUS_PERCENT 200	//�ػ��������˺��ӳ�
#define PLAYER_HP_GEN_FACTOR	5		//��һ�Ѫ������
#define PLAYER_MP_GEN_FACTOR	10		//��һ�ħ������
#define MAX_USERNAME_LENGTH	40		//������ֵ���󳤶�
#define PVP_COMBAT_HIGH_TH	300		//PVP��ʱ�������
#define PVP_COMBAT_LOW_TH	150		//PVP��ʱ��С��һ��ֵ�Ż���з���ˢ��
#define MAX_DOUBLE_EXP_TIME	(4*3600)	//������˫��ʱ��
#define MIN_TEAM_DISEXP_LEVEL	20		//���о������ʱ�� ��С��ӵ�Ч����
#define DOUBLE_EXP_FACTOR	1.5f		//˫������ʱ �����sp�ĳ˷�����
#define TASK_CHAT_MESSAGE_ID	24		//���񺰻���ϵͳ��ʽ���㲥
#define DPS_MAN_CHAT_MSG_ID		101		//ɳ������������ϵͳ��ʽ���㲥
#define TITLE_RARE_CHAT_MSG_ID	102		//ϡ�гƺŻ�ú���ϵͳ��ʽ���㲥 
#define FAC_RENAME_CHAT_MSG_ID  103	    //gamed ���ɸ���ռ��

#define ROLE_REPUTATION_UCHAR_SIZE	256		 
#define ROLE_REPUTATION_USHORT_SIZE	64		 
#define ROLE_REPUTATION_UINT_SIZE	32		 

#define ANTI_INVISIBLE_CONSTANT	0	//����ķ����ȼ�����

#define WEDDING_CONFIG_ID		801		//�������õ�id	
#define WEDDING_CANCELBOOK_FEE	3000000	//ȡ������ԤԼ�ķ���
#define WEDDING_SCENE_COUNT		2		//Ŀǰֻ��1�����񳡾�,���֧��10��,�ɻ��������޶�

#define DEFAULT_RESURRECT_HP_FACTOR 0.1f	//Ĭ�ϵ��������Ѫ����
#define DEFAULT_RESURRECT_MP_FACTOR 0.1f	//Ĭ�ϵ��������������

#define FACTION_FORTRESS_CONFIG_ID 854

#define CONGREGATE_REQUEST_TIMEOUT		120	//��������120�볬ʱ
#define TOTAL_SEC_PER_DAY			24*60*60//һ�����ж�����
#define PRODUCE4_CHOOSE_ITEM_TIME		30 //�¼̳������ȴ��ͻ��˽���ѡ���ʱ��
#define ONLINE_AWARD_CONFIG_ID		1023	
#define COUNTRYJOIN_LEVEL_REQUIRED	60
#define COUNTRYJOIN_APPLY_TICKET	36672	
#define COUNTRYBATTLE_CONFIG_ID		1027
#define COUNTRY_CHAT_FEE			10000
#define EQUIP_SIGNATURE_CLEAN_CONSUME	5	//���װ��ǩ�����ĵ�īˮ�������ֵ

#define PET_ADDEXP_ITEM				37401   //����ι�����ĵ���Ʒ
#define PET_EVOLUTION_ITEM1			37401   //���������ز������ĵ���Ʒ1
#define PET_EVOLUTION_ITEM2			12980	//���������ز������ĵ���Ʒ2
#define PET_REBUILD_TIME				1	//�����Ը���ѵ��ϴ�����ĵ�ʱ��
#define PET_CHOOSE_REBUILD_RESULT_TIME	30	//�����Ը���ѵϴ��ȴ��ͻ���ѡ���ʱ�� 
#define PET_EVOLVE_CONFIG_ID		1038	//�������ģ��id

#define CHANGEDS_LEVEL_REQUIRED		100
#define CHANGEDS_SEC_LEVEL_REQUIRED	20

#define PLAYER_RENAME_ITEM_ID       37302   //��ҽ��н�ɫ��������Ҫ���ĵ���Ʒid
#define PLAYER_RENAME_ITEM_ID_2     46901   //��ҽ��н�ɫ��������Ҫ���ĵ���Ʒid2

#define MERIDIAN_REFINE_COST1		38142	//������ѳ���������Ʒ
#define MERIDIAN_REFINE_COST2		38143
#define MERIDIAN_REFINE_COST3		38144
#define MERIDIAN_PAID_REFINE_COST1		42328 //�������ѳ������Ҫ����Ʒ
#define MERIDIAN_PAID_REFINE_COST2		38145
#define MERIDIAN_PAID_REFINE_COST3		38146
#define MERIDIAN_PAID_REFINE_COST4		38147
#define MERIDIAN_MAX_PAID_REFINE_TIMES 100   //�������ѳ����������
#define MERIDIAN_INC_PAID_REFINE_TIMES 50	//ÿ�����ӵĸ��ѳ������
#define MERIDIAN_MAX_REFINE_LEVEL 		80
#define MERIDIAN_TRIGRAMS_SIZE		48
#define TOUCH_SHOP_CONFIG_ID    1290
#define HISTORY_ADVANCE_CONFIG_ID    1425
#define FACTION_PVP_CONFIG_ID   1740

#define FACTION_FORTRESS_RESET_TECH_ITEM_ID	39202	//���ɻ������ÿƼ�����������Ʒid
#define TRICKBATTLE_LEVEL_REQUIRED		100
#define TRICKBATTLE_SEC_LEVEL_REQUIRED	20
#define TRICKBATTLE_CONFIG_ID			1444
#define GENERALCARD_MAX_COLLECTION		512		//��������ղ���
#define GENERALCARD_MAX_REBIRTH_TIMES	2		//�������ת������

#define AUTO_SUPPORT_STONE1 36764 //����ʯ1
#define AUTO_SUPPORT_STONE2 36765 //����ʯ2
#define AUTO_SUPPORT_STONE3 36766 //����ʯ3
#define AUTO_SUPPORT_STONE4 36767 //����ʯ4

#define PLAYER_FATE_RING_TOTAL			6	//��������
#define PLAYER_FATE_RING_MAX_LEVEL		10	//���ֵ���ߵȼ�
#define PLAYER_FATE_RING_GAIN_PER_WEEK	20	//����ÿ�ܲɼ�Ԫ�������
#define MATTER_ITEM_SOUL_LIFE	30	//�ɼ������Ԫ����Ʒ����(��)

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
