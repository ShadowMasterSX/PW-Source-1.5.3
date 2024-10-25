#ifndef __ONLINEGAME_GS_ITEM_H__
#define __ONLINEGAME_GS_ITEM_H__

#include <algorithm>
#include <vector.h>
#include "substance.h"
#include "property.h"
#include "matter.h"
#include "actobject.h"
#include <common/packetwrapper.h>
#include "item/item_addon.h"
#include <db_if.h>
#include "itemdata.h"

//lgc
#define ELF_DEC_ATTRIBUTE_TICKET_ID 23552	//小精灵洗属性点
#define ELF_DEC_ATTRIBUTE_TICKET_ID2 24337	//小精灵洗属性点,任务用
#define ELF_FLUSH_GENIUS_TICKET_ID 23553	//小精灵洗天赋点
#define ELF_FLUSH_GENIUS_TICKET_ID2 24338	//小精灵洗天赋点，任务用
#define ELF_REFINE_TICKET0_ID 23547 //普通精炼道具0，失败等级变0
#define ELF_REFINE_TICKET1_ID 23548	//玲珑丹, 失败等级变0
#define ELF_REFINE_TICKET2_ID 23549	//神韵丹，失败等级降1
#define ELF_REFINE_TICKET3_ID 23550	//梦幻丹，失败等级不变
#define ELF_EXPPILL_ID	23544		//拆分小精灵生成的小精灵经验丸
#define ELF_EXPPILL_ID2	23973		//小精灵经验丸，任务用
#define ELF_EXPPILL_ID3	23974		//小精灵经验丸，任务用
#define ELF_EXPPILL_ID4	23975		//小精灵经验丸，任务用
#define ELF_REFINE_TRANSMIT_TICKET_ID 23551 //小精灵精炼等级转移道具
#define MULTI_EXP_STONE_ID	27424	//选择多倍经验套餐需要的物品id
#define MULTI_EXP_STONE_ID2	27425	//选择多倍经验套餐需要的物品id
#define GOD_EVIL_CONVERT_TICKET_ID	27911 //仙魔转换需要的道具
#define GOD_EVIL_CONVERT_TICKET_ID2	44629 //高级仙魔转换需要的道具
#define WEDDING_BOOK_TICKET_ID		28452	//婚礼预约使用的道具
#define WEDDING_APPROVE_TICKET_ID	28454	//准婚证明
#define WEDDING_INVITE_TICKET_ID	28400	//兑换请柬使用的道具
#define WEDDING_INVITECARD_ID1		28444	//结婚者使用的请柬
#define WEDDING_INVITECARD_ID2		28445	//宾客使用的请柬
#define UNLIMITED_TRANSMIT_SCROLL_ID1	35206
#define UNLIMITED_TRANSMIT_SCROLL_ID2	35207
#define UNLIMITED_TRANSMIT_SCROLL_ID3	35208
#define UNLIMITED_TRANSMIT_SCROLL_ID4	35209
#define IMPROVE_FLYSWORD_TICKET_ID1		42839
#define IMPROVE_FLYSWORD_TICKET_ID2		42840
#define IMPROVE_FLYSWORD_TICKET_ID3		42838

class item;
class item_body;
class item_list;

inline int hsv2rgb( float h, float s, float v)
{
	float aa, bb, cc,f;
	int r, g, b;
	v *= 255;

	if( s == 0 )
		r = g = b = (int)v;
	else
	{
		if( h >= 1.0f ) h = 0.0f;
		if( h < 0.f) h = 0.0f;
		h *= 6.0f;
		int i = int(floor(h));
		f = h - i;
		aa = v * (1 - s);
		bb = v * (1 - s * f);
		cc = v * (1 - s * (1 - f));
		switch(i)
		{
			case 0: r = (int)v;	 g = (int)cc; b = (int)aa; break;
			case 1: r = (int)bb; g = (int)v;  b = (int)aa; break;
			case 2: r = (int)aa; g = (int)v;  b = (int)cc; break;
			case 3:	r = (int)aa; g = (int)bb; b = (int)v;  break;
			case 4: r = (int)cc; g = (int)aa; b = (int)v;  break;
			case 6:
			case 5: r = (int)v;  g = (int)aa; b = (int)bb; break;
			default:
				//不可能再有了 
				r = 0; g = 0; b = 0; break;
		}
	}
	return  (r << 16) | (g << 8) | b;
}

inline item_data * DupeItem(const item_data & data)
{
	size_t len = sizeof(item_data) + data.content_length;
	item_data * pData=(item_data*)abase::fast_allocator::align_alloc(len);
	*pData = data;
	pData->item_content = ((char*)pData) + sizeof(item_data);
	if(data.content_length)
	{
		memcpy(pData->item_content,data.item_content,data.content_length);
	}
	return pData;
}

inline void FreeItem(item_data * pData)
{
	ASSERT(!pData->content_length || 
		pData->content_length && pData->item_content == ((char*)&(pData->item_content)) + sizeof(char*));
	
	size_t len = sizeof(item_data) + pData->content_length;
	abase::fast_allocator::align_free(pData,len);
}


/*
 *	从物品数据生成物品条目的函数
 */
bool MakeItemEntry(item& entry,const item_data & data);
bool MakeItemEntry(item& entry,const GDB::itemdata &data); //从数据库的数据生成条目的函数

struct item
{
	int type;		//物品的类型
	size_t count;		//物品的数量
	size_t pile_limit;	//堆叠限制
	int equip_mask;		//装备标志	表示可以装备在哪个地方 0x80000000表示是否64位扩展 0x40000000表示是否有附加属性
	int proc_type;		//物品的处理方式
	size_t price;		//单价
	int expire_date;        //到期时间，如果<=0则无到期时间
	struct 
	{
		int guid1;
		int guid2;
	}guid;
	item_body * body;

	/*
		装备equip_mask说明：
		1. item/item_data/GDB::itemdata中的equip_mask为32位mask,各bit的含义参见"enum EQUIP_MASK",
			通过Equipmask32To64函数转换至64位mask后可以得到该装备可以装备的栏位
		2. elementdata各种essence中/经过Equipmask32To64函数装化后的equip_mask为64位mask(实际存储类型可能为int/int64),
			各bit与装备栏位一一对应,具体参见"enum EQUIP_MASK64"
	 */
	
	/*
	   64位装备MASK,MASK的每一位与装备栏的索引一一对应
	 */
	enum EQUIP_MASK64
	{
		EQUIP_MASK64_WEAPON			= 0x00000001,
		EQUIP_MASK64_HEAD			= 0x00000002,
		EQUIP_MASK64_NECK			= 0x00000004,
		EQUIP_MASK64_SHOULDER		= 0x00000008,
		EQUIP_MASK64_BODY			= 0x00000010,
		EQUIP_MASK64_WAIST			= 0x00000020,
		EQUIP_MASK64_LEG			= 0x00000040,
		EQUIP_MASK64_FOOT			= 0x00000080,
		EQUIP_MASK64_WRIST			= 0x00000100,
		EQUIP_MASK64_FINGER1		= 0x00000200,
		EQUIP_MASK64_FINGER2		= 0x00000400,
		EQUIP_MASK64_PROJECTILE		= 0x00000800,
		EQUIP_MASK64_FLYSWORD		= 0x00001000,
		EQUIP_MASK64_FASHION_BODY	= 0x00002000,
		EQUIP_MASK64_FASHION_LEG 	= 0x00004000,
		EQUIP_MASK64_FASHION_FOOT	= 0x00008000,
		EQUIP_MASK64_FASHION_WRIST	= 0x00010000,
		EQUIP_MASK64_ATTACK_RUNE	= 0x00020000,
		EQUIP_MASK64_BIBLE			= 0x00040000,
		EQUIP_MASK64_BUGLE			= 0x00080000,
		EQUIP_MASK64_HP_ADDON		= 0x00100000,
		EQUIP_MASK64_MP_ADDON		= 0x00200000,
		EQUIP_MASK64_TWEAK			= 0x00400000,
		EQUIP_MASK64_ELF			= 0x00800000,
		EQUIP_MASK64_STALLCARD		= 0x01000000,
		EQUIP_MASK64_FASHION_HEAD	= 0x02000000,
		EQUIP_MASK64_FORCE_TICKET	= 0x04000000,
		EQUIP_MASK64_DYNSKILL0		= 0x08000000,
		EQUIP_MASK64_DYNSKILL1		= 0x10000000,
		EQUIP_MASK64_FASHION_WEAPON = 0x20000000,//时装武器 
		EQUIP_MASK64_UNUSED1		= 0x40000000,
		EQUIP_MASK64_UNUSED2		= 0x80000000,
		
		EQUIP_MASK64_GENERALCARD1	= 0x0000000100000000LL,
		EQUIP_MASK64_GENERALCARD2	= 0x0000000200000000LL,
		EQUIP_MASK64_GENERALCARD3	= 0x0000000400000000LL,
		EQUIP_MASK64_GENERALCARD4	= 0x0000000800000000LL,
		EQUIP_MASK64_GENERALCARD5	= 0x0000001000000000LL,
		EQUIP_MASK64_GENERALCARD6	= 0x0000002000000000LL,

		EQUIP_MASK64_CAN_BIND		= 0x220DF7FF,
		EQUIP_MASK64_SECURITY_PASSWD_PROTECTED = 0x2205F7FF,//受安全密码保护的装备位置
		EQUIP_MASK64_DYNSKILL_ALL	= 0x18000000,
	};

	/*
	   装备栏的索引,0-63
	 */
	enum EQUIP_INDEX
	{
		EQUIP_INDEX_WEAPON			= 0,
		EQUIP_INDEX_HEAD			= 1,
		EQUIP_INDEX_NECK			= 2,
		EQUIP_INDEX_SHOULDER		= 3,
		EQUIP_INDEX_BODY			= 4,
		EQUIP_INDEX_WAIST			= 5,
		EQUIP_INDEX_LEG				= 6,
		EQUIP_INDEX_FOOT			= 7,
		EQUIP_INDEX_WRIST			= 8,
		EQUIP_INDEX_FINGER1			= 9,
		EQUIP_INDEX_FINGER2			= 10,
		EQUIP_INDEX_PROJECTILE		= 11,
		EQUIP_INDEX_FLYSWORD		= 12,
		EQUIP_INDEX_FASHION_BODY	= 13,
		EQUIP_INDEX_FASHION_LEG		= 14,
		EQUIP_INDEX_FASHION_FOOT	= 15,
		EQUIP_INDEX_FASHION_WRIST	= 16,
		EQUIP_INDEX_RUNE_SLOT		= 17,
		EQUIP_INDEX_BIBLE			= 18,
		EQUIP_INDEX_BUGLE			= 19,
		EQUIP_INDEX_HP_ADDON		= 20,
		EQUIP_INDEX_MP_ADDON		= 21,
		EQUIP_INDEX_TWEAK			= 22,
		EQUIP_INDEX_ELF				= 23,
		EQUIP_INDEX_STALLCARD		= 24,
		EQUIP_INDEX_FASHION_HEAD	= 25,
		EQUIP_INDEX_FORCE_TICKET	= 26,
		EQUIP_INDEX_DYNSKILL0		= 27,
		EQUIP_INDEX_DYNSKILL1		= 28,
		EQUIP_INDEX_FASHION_WEAPON	= 29, //时装武器
		EQUIP_INDEX_UNUSED1			= 30,
		EQUIP_INDEX_UNUSED2			= 31,
		EQUIP_INDEX_GENERALCARD1	= 32,
		EQUIP_INDEX_GENERALCARD2	= 33,
		EQUIP_INDEX_GENERALCARD3	= 34,
		EQUIP_INDEX_GENERALCARD4	= 35,
		EQUIP_INDEX_GENERALCARD5	= 36,
		EQUIP_INDEX_GENERALCARD6	= 37,
		EQUIP_INVENTORY_COUNT,		

		EQUIP_VISUAL_START	= EQUIP_INDEX_WEAPON,
		EQUIP_VISUAL_END	= EQUIP_INDEX_FASHION_WEAPON + 1,
		EQUIP_ARMOR_START	= EQUIP_INDEX_HEAD,
		EQUIP_ARMOR_END		= EQUIP_INDEX_PROJECTILE,
	};

	/*
	   32位装备MASK,特殊标志位
	 */
	enum EQUIP_MASK
	{
		EQUIP_MASK_HAS_ADDON	= 0x40000000,		//有附加属性
		EQUIP_MASK_EXTEND64		= 0x80000000,
		EQUIP_MASK_HIGH			= 0xC0000000,
	};

	static int Equipmask64To32(int64_t mask64)
	{
		int mask = 0;
		if(mask64 & 0xFFFFFFFF00000000LL)
		{
			mask = ((int)(mask64 >> 32)) & (~EQUIP_MASK_HIGH) | EQUIP_MASK_EXTEND64;
		}
		else
		{
			mask = ((int)(mask64 & 0x00000000FFFFFFFFLL)) & (~EQUIP_MASK_HIGH);
		}
		return mask;
	}

	static int64_t Equipmask32To64(int mask)
	{
		int64_t mask64 = 0;
		if(mask & EQUIP_MASK_EXTEND64)
		{
			mask64 = mask & (~EQUIP_MASK_HIGH);
			mask64 <<= 32;
		}
		else
		{
			mask64 = mask & (~EQUIP_MASK_HIGH);
		}
		return mask64;
	}

	static inline bool CheckEquipCanBind(int equip_mask)
	{
		return (Equipmask32To64(equip_mask) & EQUIP_MASK64_CAN_BIND);	
	}
	static inline bool CheckEquipProtected(int equip_mask)
	{
		return (Equipmask32To64(equip_mask) & EQUIP_MASK64_SECURITY_PASSWD_PROTECTED);
	}
	static inline bool CheckEquipDynSkill(int equip_mask)
	{
		return (Equipmask32To64(equip_mask) & EQUIP_MASK64_DYNSKILL_ALL);
	}
	static inline bool CheckEquipPostion(int equip_mask, int equip_index)
	{
		return (Equipmask32To64(equip_mask) & (1LL << equip_index)); 
	}
	static inline bool CheckEquipProtectedByIndex(int equip_index)
	{
		return ((1LL << equip_index) & EQUIP_MASK64_SECURITY_PASSWD_PROTECTED);
	}
	static inline bool CheckEquipDynSkillByIndex(int equip_index)
	{
		return ((1LL << equip_index) & EQUIP_MASK64_DYNSKILL_ALL);
	}
	
	enum
	{
		ITEM_PROC_TYPE_NODROP 	= 0x0001,	//死亡时不掉落
		ITEM_PROC_TYPE_NOTHROW	= 0x0002,	//无法扔在地上  down
		ITEM_PROC_TYPE_NOSELL	= 0x0004,	//无法卖给NPC   down
		ITEM_PROC_TYPE_CASHITEM	= 0x0008,	//人民币物品
		ITEM_PROC_TYPE_NOTRADE  = 0x0010,	//玩家间不能交易
		ITEM_PROC_TYPE_TASKITEM = 0x0020,	//是任务物品
		ITEM_PROC_TYPE_BIND2	= 0x0040,	//装备后即绑定的物品
		ITEM_PROC_TYPE_UNBIND	= 0x0080,	//是否可以解绑
		ITEM_PROC_TYPE_NO_SAVE	= 0x0100,	//离开场景时消失
		ITEM_PROC_TYPE_AUTO_USE	= 0x0200,	//自动使用
		ITEM_PROC_TYPE_DEATH_DROP= 0x0400,	//死亡掉落
		ITEM_PROC_TYPE_LEAVE_DROP= 0x0800,	//下线掉落
		ITEM_PROC_TYPE_UNREPAIRABLE = 0x1000,	//不可修理
		ITEM_PROC_TYPE_DAMAGED		= 0x2000,	//玩家pk被杀死后，物品损毁
		ITEM_PROC_TYPE_NOPUTIN_USERTRASH = 0x4000,	//不可放入帐号仓库物品
		ITEM_PROC_TYPE_BIND			= 0x8000,	//是已经绑定的物品
		ITEM_PROC_TYPE_CAN_WEBTRADE	= 0x10000,	//可以在寻宝网交易
		ITEM_PROC_TYPE_MALL			= 0x20000,	//从乾坤袋购买的物品，赠品除外

		ITEM_PROC_NO_BIND_MASK	=  ITEM_PROC_TYPE_NODROP | ITEM_PROC_TYPE_NOTHROW | ITEM_PROC_TYPE_NOSELL | ITEM_PROC_TYPE_NOTRADE,
		//是不能绑定的组合操作  策划要求必须同时具备所有条件 才不允许绑定，否则就可以绑定
	};

	enum
	{
		REFINE_SUCCESS		= 0x00,
		REFINE_CAN_NOT_REFINE 	= 0x01,
		REFINE_FAILED_LEVEL_0	= 0x02,		//失败，原料消失 装备不变
		REFINE_FAILED_LEVEL_1	= 0x03,		//失败，原料消失 装备降一级
		REFINE_FAILED_LEVEL_2 	= 0x04,		//失败，原料消失 装备归0
	};

	
	/* 有多种弓和多种箭，具体的关系由策划决定,这里不再定义mask值
	enum
	{
		PROJECTILE_MASK_ARROW	= 0x01,
		PROJECTILE_MASK_BOLT	= 0x02,
		PROJECTILE_MASK_PILL	= 0x04,
	};
	*/
private:
	friend class item_list;

	template <int foo>
	bool InsertTo(item & other)
	{
		ASSERT(type != -1 && other.type != -1);
		if(body && other.body)
		{
			return body->InsertToOther(type,other.body);
		}
		else
		{
			return false;
		}
	}
public:
	item():type(-1),count(0),pile_limit(0),equip_mask(0),proc_type(0),price(0),body(0){}
			       
	~item()
	{
		ASSERT(body == NULL);
	}
	inline void Release();
	inline void Clear();

	inline bool Save(archive & ar);
	inline bool Load(archive & ar);
	bool LoadBody(archive &ar, int classid);
	
	enum LOCATION
	{
		INVENTORY,
		BODY,
		TASK_INVENTORY,
		BACKPACK,
		TEMP_INV,
	};


/* 要转发到body的函数 */
	inline bool CheckAttack(item_list & list) const;  
	inline void AfterAttack(item_list & list,bool * pUpdate) const;  
	inline bool IsActive();
	inline bool IsSecActive();
	template<typename T>
	inline void SetSecActive(bool sec_active, T param);
	bool CanActivate(item_list & list, int index, gactive_imp * obj);
	inline bool CanUse(item::LOCATION l );
	inline bool SitDownCanUse(item::LOCATION l);
	inline bool IsBroadcastUseMsg();
	inline bool IsBroadcastArgUseMsg();
	inline bool CanUseWithTarget(item::LOCATION l);
	inline bool CanUseWithArg(item::LOCATION l,size_t buf_size);
	inline int  GetUseDuration();
	inline void Activate(item::LOCATION l,item_list & list,size_t pos, gactive_imp * obj);
	inline void Deactivate(item::LOCATION l,size_t pos, gactive_imp * obj);
	inline void PutIn(item::LOCATION l,item_list &, size_t pos, gactive_imp * obj);
	inline void TakeOut(item::LOCATION l,size_t pos,gactive_imp * obj);
	inline int Use(item::LOCATION l, int index, gactive_imp * obj,size_t count);
	inline int Use(item::LOCATION l, int index, gactive_imp * obj, const char * arg, size_t arg_size);
	inline int UseWithTarget(item::LOCATION l,int index, gactive_imp * obj,const XID & target, char force_attack);
	inline int  GetProjectileType() const;
	inline int  GetProjectileReqType() const;
	inline bool ArmorDecDurability(int amount) const;
	inline void GetDurability(int & dura, int & max_dura) const ;
	inline void Repair() const;
	inline unsigned short GetCRC() const;
	inline int GetIdModify() const ; 	//修正ID，供装备使用
	inline void InitFromShop() const;	//第一次从商店买入，进行的初始化操作
    inline void DyeItem(int value) const;	
	inline int AutoTrigger(gactive_imp * obj, int cooldown_idx,int value) const;
	inline int AutoAdjust(int& value, int max);
	//lgc 
	inline bool AddAttributePoint(short str, short agi, short vit, short eng, bool ischeck);//加属性点
	inline bool AddGeniusPoint(short g0, short g1, short g2, short g3, short g4, bool ischeck);//加天赋点
	inline unsigned int InsertExp(unsigned int exp, short exp_level, gactive_imp* imp, bool& is_levelup, bool ischeck);//注入经验
	inline bool EquipElfItem(unsigned int id, bool ischeck);//装备小精灵的装备
	inline bool ChangeElfSecureStatus(int status, bool ischeck);
	inline void UpdateElfSecureStatus();
	
	inline bool DecAttributePoint(short str, short agi, short vit, short eng);
	inline bool FlushGeniusPoint();
	inline int LearnSkill(gactive_imp * imp, unsigned short skill_id);
	inline int ForgetSkill(gactive_imp * imp, unsigned short skill_id, short forget_level);
	inline int GetStatusValue();
	inline int GetSecureStatus();
	inline int GetLevel();
	inline int GetStamina();
	inline void DecStamina(int sta);
	inline short GetRefineLevel();
	inline short SetRefineLevel(short level);
	inline int ElfRefine(int ticket_id, int ticket_cnt, int& original_level);
	inline bool GetDecomposeElfExp(unsigned int & exp, int & exp_level);
	inline bool IsElfItemExist(int mask);
	inline int DestroyElfItem(int mask, int equip_type);

	inline bool GetSkillData(unsigned int& skill_id, unsigned int& skill_level);
	inline bool GetBookCardData(int & year, int & month, int & day);
	inline bool SetInviteCardData(int start_time, int end_time, int groom, int bride, int scene, int invitee);
	inline bool GetInviteCardData(int& start_time, int& end_time, int& groom, int& bride, int& scene, int& invitee);
	inline int GetAddonExpireDate();
	inline int RemoveExpireAddon(int cur_t);
	inline bool Sharpen(addon_data * addon_list, size_t count, int sharpener_gfx);
	inline bool Engrave(addon_data * addon_list, size_t count);
	inline size_t GetEngraveAddon(addon_data * addon_list, size_t max_count);
    inline bool InheritAddon(addon_data * addon_list, size_t count);
    inline size_t GetCanInheritAddon(addon_data * addon_list, size_t max_count, int ex_addon_id);
	inline int RegenInherentAddon();
	inline int Is16Por9JWeapon();	//0-普通 1-16品 2-9军
	inline bool HasAddonAtSocket(unsigned char s_idx,int s_type); 
	inline bool ModifyAddonAtSocket(unsigned char s_idx,int s_type);

	inline int GetRank();
	inline int GetRebirthTimes();
	inline bool CheckRebirthCondition(int material_rebirth_times);
	inline void DoRebirth();
	inline bool InsertExp(int exp, bool ischeck);
	inline int GetSwallowExp();
	inline bool IsGeneralCardMatchPos(size_t pos);
	inline int GetImproveLevel();									//获取飞剑改良等级
	inline bool FlyswordImprove(float speed_inc, float speed_inc2);	//飞剑改良

	inline int GetProctypeState()
	{
		return Proctype2State(proc_type);
	}

	inline static int Proctype2State(const item_data & data)
	{
		return Proctype2State(data.proc_type);
	}
	inline static int Proctype2State(int proc_type)
	{
		int state = 0;
		if(proc_type & ITEM_PROC_TYPE_BIND) state = 0x001;
		if(proc_type & ITEM_PROC_TYPE_BIND2) state = 0x002;
		return state;
	}

	//根据equip_mask检查装备是否可绑定
	inline bool CheckEquipCanBind() const { return CheckEquipCanBind(equip_mask); }
	//根据equip_mask检查装备是否受安全密码保护
	inline bool CheckEquipProtected() const { return CheckEquipProtected(equip_mask);}
	//根据equip_mask检查装备是否是动态技能物品
	inline bool CheckEquipDynSkill() const { return CheckEquipDynSkill(equip_mask); }
	//检查该装备是否可以装备到指定栏位
	inline bool CheckEquipPostion(int equip_index) const { return CheckEquipPostion(equip_mask, equip_index); }
	//获取装备的64位equipmask
	inline int64_t GetEquipMask64(){ return Equipmask32To64(equip_mask); }

};

class item_body : public substance
{
protected:
	int _tid;		//物品类型
	bool _is_active;
	/*
	   第二激活标志: 在装备激活时(OnActivate/OnDeactivate)使用。
	   当达到某种外部条件时,装备额外激活某些属性,例如:卡牌装备成套后将增强卡牌属性
	 */
	bool _is_sec_active;		//第二激活标志，含义与物品种类相关	
	int _sec_active_param;		//第二激活参数，含义与物品种类相关
	friend class item;

	//供服务器切换时存盘使用
	void SetActive(bool active)
	{
		_is_active = active;
	}

public:
	DECLARE_SUBSTANCE(item_body);
	virtual bool Init(const void * content, size_t content_length) {return false;}
	enum ITEM_TYPE
	{
		ITEM_TYPE_WEAPON,
		ITEM_TYPE_ARMOR,
		ITEM_TYPE_DECORATION,
		ITEM_TYPE_PROJECTILE,
		ITEM_TYPE_POTION,
		ITEM_TYPE_STONE,
		ITEM_TYPE_WING,
		ITEM_TYPE_FLYSWORD,
		ITEM_TYPE_METERIAL,
		ITEM_TYPE_TOSSMATTER,
		ITEM_TYPE_TOWNSCROLL,
		ITEM_TYPE_RUNE,
		ITEM_TYPE_TASKDICE,
		ITEM_TYPE_FASHION,
		ITEM_TYPE_SKILLTOME,
		ITEM_TYPE_GPI,
		ITEM_TYPE_GENERAL_EFFECT,
		ITEM_TYPE_FACE_TICKET,
		ITEM_TYPE_RESURRECT_SCROLL,
		ITEM_TYPE_MOB_GEN,
		ITEM_TYPE_PET_EGG,
		ITEM_TYPE_PET_FOOD,
		ITEM_TYPE_TANK_CONTROL,
		ITEM_TYPE_FIREWORKS,
		ITEM_TYPE_DUMMY,
		ITEM_TYPE_BUGLE,
		ITEM_TYPE_BIBLE,
		ITEM_TYPE_AMULET,
		ITEM_TYPE_DBEXP,
		ITEM_TYPE_ELF, //lgc
		ITEM_TYPE_ELF_EQUIP,
		ITEM_TYPE_ELF_EXPPILL,
		ITEM_TYPE_STALLCARD,
		ITEM_TYPE_SKILLTRIGGER2,
		ITEM_TYPE_QUERYOTHERPROPERTY,
		ITEM_TYPE_INCSKILLABILITY,
		ITEM_TYPE_WEDDING_BOOKCARD,
		ITEM_TYPE_WEDDING_INVITECARD,
		ITEM_TYPE_SHARPENER,
		ITEM_TYPE_CONGREGATE,
		ITEM_TYPE_FORCE_TICKET,
		ITEM_TYPE_DYNSKILL,
		ITEM_TYPE_GENERALCARD,
		ITEM_TYPE_GENERALCARD_DICE,
		ITEM_TYPE_SOUL,
		ITEM_TYPE_MINECAR_PROTECT,
	};
public:
	item_body():_tid(0),_is_active(false),_is_sec_active(false),_sec_active_param(0) {}
	virtual ~item_body(){}
	virtual item_body * Clone() const = 0;
	
	int  GetProjectileType() { return OnGetProjectileType();}
	int  GetProjectileReqType() { return OnGetProjectileReqType();}
	bool CheckAttack(item_list & list) { return OnCheckAttack(list);}
	void AfterAttack(item_list & list,bool *pUpdate) { return OnAfterAttack(list,pUpdate);}
	bool IsActive() { return _is_active;}
	bool IsSecActive() { return _is_sec_active;}
	template<typename T>
	T GetSecActiveParam()
	{
		ASSERT(sizeof(T) <= sizeof(int));
		return *(T*)&_sec_active_param; 
	}
	template<typename T>
	void SetSecActive(bool sec_active, T param)
	{
		ASSERT(sizeof(T) <= sizeof(int));
		_is_sec_active = sec_active;
		*(T*)&_sec_active_param = param;
	}
	bool IsBroadcastUseMsg() { return IsItemBroadcastUse();}
	bool IsBroadcastArgUseMsg() { return IsItemBroadcastArgUse();}

	void PutIn(item::LOCATION l,item_list & list, size_t pos, size_t count, gactive_imp * obj)
	{
		OnPutIn(l,list, pos, count, obj);
	}

	void TakeOut(item::LOCATION l,size_t pos,size_t count, gactive_imp * obj)
	{	
		OnTakeOut(l,pos,count,obj);
	}

	int Use(item::LOCATION l, int index, gactive_imp * obj,size_t count)
	{
		return OnUse(l,index,obj,count);
	}

	int Use(item::LOCATION l, int index, gactive_imp * obj, const char * arg, size_t arg_size) 
	{
		return OnUse(l,index,obj,arg, arg_size);
	}

	int UseWithTarget(item::LOCATION l, int index, gactive_imp * obj,const XID & target, char force_attack)
	{
		return OnUseWithTarget(l,index,obj,target,force_attack);
	}

	bool HasSocket() 
	{
		return OnHasSocket();
	}
	
	int SpendFlyTime(int tick)
	{
		return OnFlying(tick);
	}

	int GetFlyTime()
	{
		return OnGetFlyTime();
	}

	virtual size_t GetSocketCount() { return 0;}
	virtual int GetSocketType(size_t index) { return 0;}
	virtual void SetSocketAndStone(int count, int * stone_type){ return; }

	bool InsertChip(int type, addon_data * data, size_t count)
	{
		return OnInsertChip(type,data,count);
	}

	int Recharge(int element_level,size_t count,int & cur_time)
	{
		return OnCharge(element_level, count,cur_time);
	}

	int GetLevel()
	{
		return OnGetLevel();
	}

	bool ClearChips()
	{
		return OnClearChips();
	}

	void RefreshItem() 	//刷新物品数据，用于当物品改变后的重新刷新
	{
		OnRefreshItem();
	}

	int GetUseDuration()
	{
		return OnGetUseDuration();
	}
	
protected:
	//由于item::proc_type增加了是否损毁,所以以下判断应该调用item类中的内联函数
	bool CanUse(item::LOCATION l ) { return IsItemCanUse(l);}
	bool SitDownCanUse(item::LOCATION l) { return IsItemSitDownCanUse(l);}
	bool CanUseWithTarget(item::LOCATION l ) { return IsItemCanUseWithTarget(l);}
	bool CanUseWithArg(item::LOCATION l,size_t buf_size) { return IsItemCanUseWithArg(l, buf_size);}
	bool CanActivate(item_list & list,gactive_imp * obj) {return VerifyRequirement(list,obj);}
	
	void Activate(item::LOCATION l , item_list & list, size_t pos, size_t count, gactive_imp * obj)
	{
		if(!IsActive() && CanActivate(list,obj))
		{
			OnActivate(l,pos,count,obj);
			_is_active = true;
		}
	}

	void Deactivate(item::LOCATION l,size_t pos,size_t count, gactive_imp * obj)
	{
		if(IsActive())
		{
			OnDeactivate(l,pos,count,obj);
			_is_active = false;
		}
	}
	
public:
	virtual ITEM_TYPE GetItemType() = 0;
	virtual unsigned short GetDataCRC()  { return 0;}
	virtual int GetIdModify() {return 0;}
	virtual void InitFromShop() {}
    virtual void DyeItem(int value) {}
	virtual void GetDurability(int &dura,int &max_dura) { dura = max_dura = 0; }
	virtual void Repair() {}
	virtual bool ArmorDecDurability(int amount) = 0;
	virtual bool Load(archive &ar) { return true;}
 	//返回保存的物品数据 ，这个数据由body自己维护，外面不需要释放
	virtual void GetItemData(const void ** data, size_t &len)
	{
		*data = NULL;
		len  = 0;
	}

	virtual bool RegenAddon(int item_id, bool (*regen_addon)(int item_id,addon_data& ent))
	{
		return false;
	}

	virtual int RefineAddon(int refine_addon, int & level_result, float adjust[4], float adjust2[12])
	{
		level_result = 0;
		return item::REFINE_CAN_NOT_REFINE;
	}
	virtual int GetAddonExpireDate(){return 0;}
	virtual int RemoveExpireAddon(int cur_t){return 0;}
	virtual bool Sharpen(addon_data * addon_list, size_t count, int sharpener_gfx){return false;}
	virtual bool Engrave(addon_data * addon_list, size_t count){return false;}
	virtual size_t GetEngraveAddon(addon_data * addon_list, size_t max_count){ return 0; }
    virtual bool InheritAddon(addon_data * addon_list, size_t count){ return false; }
    virtual size_t GetCanInheritAddon(addon_data * addon_list, size_t max_count, int ex_addon_id) { return 0; }
	virtual int RegenInherentAddon(){ return 0; }
	virtual int Is16Por9JWeapon(){ return 0; }
	virtual int GetRefineLevel(int refine_addon)
	{
		return -1;
	}

	virtual int SetRefineLevel(int refine_addon , int level)
	{
		return 0;
	}
	virtual int MakeSlot(gactive_imp *, int & count) { return -1;}
	virtual bool Sign(unsigned short color, const char * signature, unsigned int signature_len) { return false; }
	virtual bool HasAddonAtSocket(unsigned char s_idx,int s_type) { return false;}
	virtual bool ModifyAddonAtSocket(unsigned char s_idx,int s_type) { return false;}
	virtual int GetRank(){ return -1; }
	virtual int GetRebirthTimes(){ return -1; }
	virtual bool CheckRebirthCondition(int material_rebirth_times){ return false; }
	virtual void DoRebirth(){}
	virtual bool InsertExp(int exp, bool ischeck){ return false; }
	virtual int GetSwallowExp(){ return 0;}
	virtual bool IsGeneralCardMatchPos(size_t pos) { return false; }
	virtual int GetImproveLevel() { return 0; }
	virtual bool FlyswordImprove(float speed_inc, float speed_inc2) { return false; }

protected:
	bool InsertToOther(int self_type,item_body * body)
	{
		return OnInsertToOther(self_type,body);
	}
	virtual bool OnHasSocket() { return false;}
	virtual bool OnInsertChip(int type,addon_data * data, size_t count) {return false;}
	virtual bool OnClearChips() { return false;}
	virtual bool OnInsertToOther(int self_type,item_body * body) { return false;}
	virtual void OnRefreshItem() {}
	virtual void OnPutIn(item::LOCATION ,item_list & list, size_t pos, size_t count, gactive_imp*){}
	virtual void OnTakeOut(item::LOCATION ,size_t pos, size_t count, gactive_imp*){}
	virtual bool VerifyRequirement(item_list & list,gactive_imp*) {return false;}
private:
	virtual int OnGetUseDuration() { return -1;} //负数代表立刻使用，不进行排队
	virtual int OnCharge(int elment_level,size_t count,int &cur_time) { return 0;}
	virtual int OnGetLevel() {return 0;}
	virtual int OnFlying(int tick) { return -1;}
	virtual int OnGetFlyTime() { return 0;}
	virtual bool IsItemCanUse(item::LOCATION l) { return false;}
	virtual bool IsItemSitDownCanUse(item::LOCATION l) { return false;}
	virtual bool IsItemCanUseWithTarget(item::LOCATION l) { return false;}
	virtual bool IsItemCanUseWithArg(item::LOCATION l,size_t buf_size) { return false;}
	virtual void OnActivate(item::LOCATION ,size_t pos,size_t count, gactive_imp*) {}
	virtual void OnDeactivate(item::LOCATION ,size_t pos,size_t count,gactive_imp*) {}
	virtual int OnUse(item::LOCATION ,gactive_imp*,size_t count){return -1;}
	virtual int OnUse(item::LOCATION l,int index, gactive_imp* obj,size_t count){return OnUse(l,obj,count);}
	virtual int OnUse(item::LOCATION ,int index, gactive_imp*,const char * arg, size_t arg_size) {return -1;}
	virtual int OnUseWithTarget(item::LOCATION l,int index,gactive_imp * obj,const XID & target,char force_attack){return -1;}
	virtual bool IsItemBroadcastUse() {return false;}
	virtual bool IsItemBroadcastArgUse() {return false;} 
	virtual bool OnCheckAttack(item_list & list){ ASSERT(false); return false;}
	virtual void OnAfterAttack(item_list & list,bool * pUpdate){ ASSERT(false); return;}
	virtual int  OnGetProjectileType() const { return 0;}
	virtual int  OnGetProjectileReqType() const { return 0;}
	virtual int OnAutoTrigger(gactive_imp* obj, int cooldown_idx,int offset) { return -1;}
	virtual int OnAutoAdjust(int& value, int max){ return -1;}

public://lgc 小精灵虚函数
	virtual bool AddAttributePoint(short str, short agi, short vit, short eng, bool ischeck){return false;}//加属性点
	virtual bool AddGeniusPoint(short g0, short g1, short g2, short g3, short g4, bool ischeck){return false;}//加天赋点
	virtual unsigned int InsertExp(unsigned int exp, short exp_level, gactive_imp* imp, bool& is_levelup, bool ischeck){return (unsigned int)-1;}//注入经验
	virtual bool EquipElfItem(unsigned int id, bool ischeck){return false;}//装备小精灵的装备
	virtual bool ChangeElfSecureStatus(int status, bool ischeck){return false;}
	virtual void UpdateElfSecureStatus(){}
	
	virtual bool DecAttributePoint(short str, short agi, short vit, short eng){return false;}
	virtual bool FlushGeniusPoint(){return false;}
	virtual int LearnSkill(gactive_imp * imp, unsigned short skill_id){return -1;}
	virtual int ForgetSkill(gactive_imp * imp, unsigned short skill_id, short forget_level){return -1;}
	virtual int GetStatusValue(){return 0;}	
	virtual int GetSecureStatus(){return 0;}
	virtual int GetStamina(){return -1;}
	virtual void DecStamina(int sta){}
	virtual short GetRefineLevel(){return -1;}
	virtual short SetRefineLevel(short level){return -1;}
	virtual int ElfRefine(int ticket_id, int ticket_cnt, int& original_level){return item::REFINE_CAN_NOT_REFINE;}
	virtual bool GetDecomposeElfExp(unsigned int & exp, int & exp_level){return false;}
	virtual bool IsElfItemExist(int mask){return false;}
	virtual int DestroyElfItem(int mask,int equip_type){return -1;}
	
	virtual bool GetSkillData(unsigned int& skill_id, unsigned int& skill_level){return false;}
	virtual bool GetBookCardData(int & year, int & month, int & day){return false;}
	virtual bool SetInviteCardData(int start_time, int end_time, int groom, int bride, int scene, int invitee){return false;}
	virtual bool GetInviteCardData(int& start_time, int& end_time, int& groom, int& bride, int& scene, int& invitee){ return false;}
	
};

inline bool item::Save(archive & ar)
{
	ar << type;
	if(type == -1) return true;
	ar << count;
	ar << pile_limit;
	ar << equip_mask;
	ar << proc_type;
	ar << price;
	ar << expire_date;
	ar << guid.guid1;
	ar << guid.guid2;
	if(body)
	{
		ar << body->IsActive();
		ar << body->IsSecActive();
		ar << body->GetSecActiveParam<int>();
		ar << body->GetGUID();
		void * data;
		size_t len;
		body->GetItemData((const void **)&data,len);
		ar << len;
		if(len)
		{
			ar.push_back(data,len);
		}
		return true;
	}
	else
	{
		ar << false << false << 0 << -1;
	}
	return true;
}

inline bool item::Load(archive & ar)
{
	ASSERT(body == NULL);
	ar >> type;
	if(type == -1)
	{
		Clear();
		return true;
	}
	ar >> count;
	ar >> pile_limit;
	ar >> equip_mask;
	ar >> proc_type;
	ar >> price;
	ar >> expire_date;
	ar >> guid.guid1;
	ar >> guid.guid2;

	int classid;
	bool active;
	bool sec_active;
	int sec_active_param;
	ar >> active;
	ar >> sec_active;
	ar >> sec_active_param;
	ar >> classid;
	if(classid > 0)
	{
		size_t size;
		ar >> size;
		raw_wrapper rw(ar.cur_data(),size);
		if(!LoadBody(rw,classid))
		{
			return false;
		}
		body->SetActive(active);
		body->SetSecActive(sec_active, sec_active_param);
		return ar.shift(size);
	}
	return true;
}


inline void item::Release()
{
	if(body) {
		delete body;
	}
	Clear();
}

inline void item::Clear()
{
	type = -1;
	count = 0;
	pile_limit = 0;
	equip_mask = 0;
	body = NULL;
	price = 0;
	expire_date = 0;  
}

inline bool item::CheckAttack(item_list & list) const
{
	if(body) 
		return body->CheckAttack(list);
	else
		return true;
}

inline void item::AfterAttack(item_list & list,bool * pUpdate)  const
{
	if(body) body->AfterAttack(list,pUpdate);
}

inline bool item::IsActive()
{
	if(body)
		return body->IsActive();
	else
		return false;
}

inline bool item::IsSecActive()
{
	if(body)
		return body->IsSecActive();
	else
		return false;
}

template<typename T>
inline void item::SetSecActive(bool sec_active, T param)
{
	ASSERT(!IsActive() && "只有装备未激活时才能改变第二激活状态");
	if(body) body->SetSecActive(sec_active, param);
}

inline bool item::CanUse(item::LOCATION l )
{
	if(proc_type & ITEM_PROC_TYPE_DAMAGED) return false;
	if(body)
		return body->CanUse(l);
	else 
		return false;
}
inline bool item::SitDownCanUse(item::LOCATION l)
{
	if(proc_type & ITEM_PROC_TYPE_DAMAGED) return false;
	if(body)
		return body->SitDownCanUse(l);
	else 
		return false;
}

inline bool item::IsBroadcastUseMsg()
{
	if(body)
		return body->IsBroadcastUseMsg();
	else 
		return false;
}

inline bool item::IsBroadcastArgUseMsg()
{
	if(body)
		return body->IsBroadcastArgUseMsg();
	else
		return false;
}

inline bool item::CanUseWithTarget(item::LOCATION l)
{
	if(proc_type & ITEM_PROC_TYPE_DAMAGED) return false;
	if(body)
		return body->CanUseWithTarget(l);
	else 
		return false;
}

inline bool item::CanUseWithArg(item::LOCATION l,size_t buf_size)
{
	if(proc_type & ITEM_PROC_TYPE_DAMAGED) return false;
	if(body)
		return body->CanUseWithArg(l,buf_size);
	else
		return false;
}

inline int item::GetUseDuration()
{
	if(body)
		return body->GetUseDuration();
	else 
		return -1;
}

inline void item::Activate(item::LOCATION l,item_list & list,size_t pos,gactive_imp * obj)
{
	if(proc_type & ITEM_PROC_TYPE_DAMAGED) return;
	if(body) return body->Activate(l,list,pos,count,obj);
}

inline void item::Deactivate(item::LOCATION l,size_t pos,gactive_imp * obj)
{
	if(body) return body->Deactivate(l,pos,count,obj);
}

inline void item::PutIn(item::LOCATION l, item_list & list, size_t pos, gactive_imp * obj)
{
	if(l != TEMP_INV)
	{
		if(expire_date > 0)
		{
			//当有有期限物品放入的时候，计算一下最近的期限值
			obj->UpdateExpireItem(expire_date);
		}
		//lgc
		int value;
		if((value = GetStatusValue()) > 0)
			obj->UpdateMinElfStatusValue(value);

		int addon_expire;
		if( (addon_expire = GetAddonExpireDate()) > 0)
			obj->UpdateMinAddonExpireDate(addon_expire);
	}

	if(l != BACKPACK && l != TEMP_INV)
	{
		if(body) body->PutIn(l,list, pos, count, obj);
	}
}

inline void item::TakeOut(item::LOCATION l,size_t pos,gactive_imp * obj)
{	
	if(l != BACKPACK && l != TEMP_INV)
	{
		if(body) return body->TakeOut(l,pos,count,obj);
	}
}

inline int item::Use(item::LOCATION l, int index, gactive_imp * obj,size_t count)
{
	if(body) 
		return body->Use(l,index,obj,count);
	else
		return 0;
}

inline int item::Use(item::LOCATION l, int index, gactive_imp * obj, const char * arg, size_t arg_size)
{
	if(body) 
		return body->Use(l, index, obj,arg, arg_size);
	else
		return 0;
}

inline int item::UseWithTarget(item::LOCATION l,int index,gactive_imp * obj,const XID & target, char force_attack)
{
	if(body) 
		return body->UseWithTarget(l,index,obj,target,force_attack);
	else
		return 0;
}

inline int item::GetProjectileType() const
{
	if(body) 
		return body->GetProjectileType();
	else
		return 0;
}

inline int item::GetProjectileReqType() const
{
	if(body) 
		return body->GetProjectileReqType();
	else
		return 0;
}

inline bool item::ArmorDecDurability(int amount)  const
{
	if(body) 
		return body->ArmorDecDurability(amount);
	else
		return false;
}

inline void item::GetDurability(int & dura, int & max_dura) const
{
	if(body) 
		return body->GetDurability(dura,max_dura);
	else
	{
		dura = max_dura = 0;
	}
}

inline void item::Repair() const
{
	if(body) return body->Repair();
}

inline unsigned short item::GetCRC() const
{
	if(body) 
		return body->GetDataCRC();
	return 0;
}

inline int item::GetIdModify() const
{
	if(body) return body->GetIdModify();
	return 0;
}

inline void item::InitFromShop() const
{
	if(body) return body->InitFromShop();
	return;
}

inline void item::DyeItem(int value) const
{
	if(body) return body->DyeItem(value);
	return;
}

inline int item::AutoTrigger(gactive_imp * obj, int cooldown_idx, int value) const
{
	if(body) return body->OnAutoTrigger(obj, cooldown_idx,value);
	return  -1;
}
inline int item::AutoAdjust(int& value, int max)
{
	if(body) return body->OnAutoAdjust(value, max);
	return -1;
}
//lgc
inline bool item::AddAttributePoint(short str, short agi, short vit, short eng, bool ischeck)//加属性点
{
	if(body) return body->AddAttributePoint(str, agi, vit, eng, ischeck);
	return false;
	
}
inline bool item::AddGeniusPoint(short g0, short g1, short g2, short g3, short g4, bool ischeck)//加天赋点
{
	if(body) return body->AddGeniusPoint(g0, g1, g2, g3, g4, ischeck);
	return false;
	
}
inline unsigned int item::InsertExp(unsigned int exp, short exp_level, gactive_imp* imp, bool& is_levelup, bool ischeck)//注入经验
{
	if(body) return body->InsertExp(exp, exp_level, imp, is_levelup, ischeck);
	return (unsigned int)-1;
	
}
inline bool item::EquipElfItem(unsigned int id, bool ischeck)//装备小精灵的装备
{
	if(body) return body->EquipElfItem(id, ischeck);
	return false;
}	
inline bool item::ChangeElfSecureStatus(int status, bool ischeck)
{
	if(body) return body->ChangeElfSecureStatus(status, ischeck);
	return false;
	
}
inline void item::UpdateElfSecureStatus()
{
	if(body) return body->UpdateElfSecureStatus();
	return;
	
}
inline bool item::DecAttributePoint(short str, short agi, short vit, short eng)
{
	if(body) return body->DecAttributePoint(str, agi, vit, eng);
	return false;
	
}
inline bool item::FlushGeniusPoint()
{
	if(body) return body->FlushGeniusPoint();
	return false;
	
}
inline int item::LearnSkill(gactive_imp * imp, unsigned short skill_id)
{
	if(body) return body->LearnSkill(imp, skill_id);
	return -1;
	
}
inline int item::ForgetSkill(gactive_imp * imp, unsigned short skill_id , short forget_level)
{
	if(body) return body->ForgetSkill(imp, skill_id, forget_level);
	return -1;

}
inline int item::GetStatusValue()
{
	if(body) return body->GetStatusValue();
	return 0;
	
}
inline int item::GetSecureStatus()
{
	if(body) return body->GetSecureStatus();
	return 0;
	
}
inline int item::GetLevel()
{
	if(body) return body->GetLevel();
	return -1;

}
inline int item::GetStamina()
{
	if(body) return body->GetStamina();
	return -1;
}
inline void item::DecStamina(int sta)
{
	if(body) return body->DecStamina(sta);
	return;
}
inline short item::GetRefineLevel()
{
	if(body) return body->GetRefineLevel();
	return -1;
}
inline short item::SetRefineLevel(short level)
{
	if(body) return body->SetRefineLevel(level);
	return -1;
}
inline int item::ElfRefine(int ticket_id, int ticket_cnt, int& original_level)
{
	if(body) return body->ElfRefine(ticket_id, ticket_cnt, original_level);
	return item::REFINE_CAN_NOT_REFINE;
}
inline bool item::GetDecomposeElfExp(unsigned int & exp, int & exp_level)
{
	if(body) return body->GetDecomposeElfExp(exp, exp_level);
	return false;
}
inline bool item::IsElfItemExist(int mask)
{
	if(body) return body->IsElfItemExist(mask);
	return false;
	
}
inline int item::DestroyElfItem(int mask, int equip_type)
{
	if(body) return body->DestroyElfItem(mask, equip_type);
	return -1;
	
}
inline bool item::GetSkillData(unsigned int& skill_id, unsigned int& skill_level)
{
	if(body) return body->GetSkillData(skill_id,skill_level);
	return false;
}
inline bool item::GetBookCardData(int & year, int & month, int & day)
{
	if(body) return body->GetBookCardData(year,month,day);
	return false;
}
inline bool item::SetInviteCardData(int start_time, int end_time, int groom, int bride, int scene, int invitee)
{
	if(body) return body->SetInviteCardData(start_time,end_time,groom,bride,scene,invitee);
	return false;
}
inline bool item::GetInviteCardData(int& start_time, int& end_time, int& groom, int& bride, int& scene, int& invitee)
{
	if(body) return body->GetInviteCardData(start_time,end_time,groom,bride,scene,invitee);
	return false;
}
inline int item::GetAddonExpireDate()
{
	if(body) return body->GetAddonExpireDate();
	return 0;
}
inline int item::RemoveExpireAddon(int cur_t)
{
	if(body) return body->RemoveExpireAddon(cur_t);
	return 0;
}
inline bool item::Sharpen(addon_data * addon_list, size_t count, int sharpener_gfx)
{
	if(body) return body->Sharpen(addon_list,count,sharpener_gfx);
	return false;
}
inline bool item::Engrave(addon_data * addon_list, size_t count)
{
	if(body) return body->Engrave(addon_list,count);
	return false;
}

inline size_t item::GetEngraveAddon(addon_data * addon_list, size_t max_count)
{
	if(body) return body->GetEngraveAddon(addon_list, max_count);
	return 0;
}

inline bool item::InheritAddon(addon_data * addon_list, size_t count)
{
	if(body) return body->InheritAddon(addon_list,count);
	return false;
}

inline size_t item::GetCanInheritAddon(addon_data * addon_list, size_t max_count, int ex_addon_id)
{
    if(body) return body->GetCanInheritAddon(addon_list, max_count, ex_addon_id);
    return 0;
}

inline int item::RegenInherentAddon()
{
	if(body) return body->RegenInherentAddon();
	return 0;
}
inline int item::Is16Por9JWeapon()
{
	if(body) return body->Is16Por9JWeapon();
	return 0;
}
inline bool item::HasAddonAtSocket(unsigned char s_idx,int s_type)
{
	if(body) return body->HasAddonAtSocket(s_idx,s_type);
	return false;
}
inline bool item::ModifyAddonAtSocket(unsigned char s_idx,int s_type)
{
	if(body) return body->ModifyAddonAtSocket(s_idx,s_type);
	return false;
}
inline int item::GetRank()
{
	if(body) return body->GetRank();
	return -1;
}
inline int item::GetRebirthTimes()
{
	if(body) return body->GetRebirthTimes();
	else return -1;
}
inline bool item::CheckRebirthCondition(int target_merge_times)
{
	if(body) return body->CheckRebirthCondition(target_merge_times);
	return false;
}
inline void item::DoRebirth()
{
	if(body) body->DoRebirth();
}
inline bool item::InsertExp(int exp, bool ischeck)
{
	if(body) return body->InsertExp(exp, ischeck);
	return false;
}
inline int item::GetSwallowExp()
{
	if(body) return body->GetSwallowExp();
	return 0;
}
inline bool item::IsGeneralCardMatchPos(size_t pos)
{
	if(body) return body->IsGeneralCardMatchPos(pos);
	return false;
}
inline int item::GetImproveLevel()
{
	if(body) return body->GetImproveLevel();
	return 0;
}
inline bool item::FlyswordImprove(float speed_inc, float speed_inc2)
{
	if(body) return body->FlyswordImprove(speed_inc,speed_inc2);
	return false;
}
/*
 *	掉落在地上的item ,被matter所包装,可以实现相应的处理
 */
class gmatter_item_imp : public gmatter_item_base_imp
{
	item_data * _data;
public:
	DECLARE_SUBSTANCE(gmatter_item_imp);
public:
	gmatter_item_imp():_data(NULL)
	{
	}
	virtual void Init(world * pPlane,gobject*parent)
	{
		ASSERT(_data && "要先SetData才行\n");
		gmatter_item_base_imp::Init(pPlane,parent);
		gmatter * pMatter = (gmatter *) parent;
		pMatter->matter_type = _data->type;
		if(_data->equip_mask & item::EQUIP_MASK_HAS_ADDON)
		{
			pMatter->matter_type |= 0x80000000;
		}

	}

	~gmatter_item_imp();
	void SetData(const item_data & data);
	void AttachData(item_data * data);
	virtual int MessageHandler(world * pPlane ,const MSG & msg);
	virtual void OnPickup(const XID & who,int team_id,bool is_team);
};


class gmatter_item_controller : public  gmatter_controller
{
public:
	DECLARE_SUBSTANCE(gmatter_item_controller);
public:
};

void DropItemFromData(world * pPlane,const A3DVECTOR &pos,const item_data & data,const XID & owner, int owner_team,int seq,int drop_id);

//这个data必须是分配出来的,调用后就在内部保存了,所以不要在外面释放
void DropItemData(world * pPlane,const A3DVECTOR &pos, item_data * data,const XID & owner, int owner_team, int seq, int life);

int GetItemRealTimePrice(const item & it);
bool IsItemForbidShop(int type);	//物品是否禁止从商城购买
bool IsItemForbidSell(int type);	//物品是否禁止卖店
#endif

