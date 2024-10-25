#ifndef __ONLINEGAME_GS_PLAYER_IMP_H__
#define __ONLINEGAME_GS_PLAYER_IMP_H__

#include "player.h"
#include "actobject.h"
#include "item_list.h"
#include "playerteam.h"
#include "playerinvade.h"
#include "playertrashbox.h"
#include <db_if.h>
#include <gsp_if.h>
#include "playerstall.h"
#include "cooldown.h"
#include "breath_ctrl.h"
#include "petman.h"
#include "playerduel.h"
#include "playerbind.h"
#include "wallow/wallowobj.h"
#include "playermall.h"
#include "vector.h"
#include "item/item_elf.h" //lgc
#include "playermisc.h"
#include "multi_exp_ctrl.h"
#include "playerlimit.h"
#include "playerforce.h"
#include "online_award.h"
#include "meridianmanager.h"
#include "touchtrade.h"
#include "playertitle.h"
#include "playerdailysign.h"
#include "player_reincarnation.h"
#include "playerfatering.h"
#include "playergiftcard.h"
#include "playerclock.h"
#include "playersanctuarycheck.h"
#include "playerinstancereenter.h"

struct MSG;
struct attack_msg;
struct enchant_msg;
struct pet_leader_prop;
//lgc
struct elf_skill_data;
struct elf_info   //用于在player_imp中保存当前装备的小精灵信息
{
	unsigned int id;//无效id为(unsigned int)-1

	bool refresh_vigor;//vigor更新
	bool refresh_enhance;//buff更新
	
	bool refine_effect_active;  //精炼效果是否激活
	short refine_level;
	
	short strength;	//小精灵的本身的属性，与elf_item中prop中cur_strength相同，方便使用
	short agility;
	short vitality;
	short energy;

	short final_strength;//加了buf的小精灵属性
	short final_agility;
	short final_vitality;
	short final_energy;

	float vigor;		//当前元气
	float max_vigor;	//元气上限
	float vigor_gen;	//元气回复
	
	abase::vector<struct elf_skill_data, abase::fast_alloc<> > skillvect;	//当前可用技能

	elf_info(): id((unsigned int)-1), refresh_vigor(false), refresh_enhance(false), refine_effect_active(false), refine_level(0), strength(0), agility(0), vitality(0), energy(0), final_strength(0), final_agility(0), final_vitality(0), final_energy(0), vigor(0.0f), max_vigor(0.0f), vigor_gen(0.0f){}
	
	void clear()//不清除refine_effect_active,vigor
	{
		id = (unsigned int)-1; 
		refresh_vigor = false; refresh_enhance = false; 
		refine_level = 0; 
		strength = 0; agility = 0; vitality = 0; energy = 0; 
		final_strength = 0; final_agility = 0; final_vitality = 0; final_energy = 0;
		max_vigor = 0.0f; vigor_gen = 0.0f;
		skillvect.clear();
	}
	
	template <typename WRAPPER>
	WRAPPER & operator>>(WRAPPER & wrapper) const
	{
		wrapper << id << refresh_vigor << refresh_enhance << refine_effect_active << refine_level << strength << agility << vitality << energy << final_strength << final_agility << final_vitality << final_energy << vigor<< max_vigor << vigor_gen;
		size_t size = skillvect.size();
		wrapper << size;
		for(size_t i=0; i<size; i++)
			wrapper << skillvect[i];
		return wrapper;
	}

	template <typename WRAPPER>
	WRAPPER & operator<< (WRAPPER & wrapper)
	{
		wrapper >> id >> refresh_vigor >> refresh_enhance >> refine_effect_active >> refine_level >> strength >> agility >> vitality >> energy >> final_strength >> final_agility >> final_vitality >> final_energy >> vigor >> max_vigor >> vigor_gen;
		size_t size = -1;
		wrapper >> size;
		if(size < 0 || size > MAX_ELF_SKILL_CNT)
			throw -1;
		struct elf_skill_data sk;
		for(size_t i=0; i<size; i++)
		{
			wrapper >> sk;
			skillvect.push_back(sk);
		}
		return wrapper;
	}
};

struct random_mall_info
{
	enum	// random mall cmd
	{
		RAND_MALL_OPT_QUERY,
		RAND_MALL_OPT_ROLL,
		RAND_MALL_OPT_PAY,
	};
	enum
	{
		RM_ENTRY_ERR = -1,
		RM_ROLE_STATE_ERR = -2,
		RM_OPT_STATE_ERR = -3,
		RM_CASH_ERR = -4,
		RM_INV_ERR = -5,
	};
	static const int RAND_MALL_LEVEL_LIMIT = 20;

	struct random_mall_result
	{
		int itemid;
		int price;
		int paytime;
		int reserve1;
		int reserve2;
		random_mall_result(int id = 0,int pr = 0,int pt = 0,int r1 = 0,int r2 = 0) 
			: itemid(id),price(pr),paytime(pt),reserve1(r1),reserve2(r2) {}
	};
	typedef	abase::static_multimap<int,random_mall_result, abase::fast_alloc<> > RAND_MALL_RES_MAP;
	RAND_MALL_RES_MAP	_result_datas;
	
	template <typename WRAPPER>
	bool Save(WRAPPER & wrapper) const
	{
		size_t size = _result_datas.size();	
		wrapper << size;

		RAND_MALL_RES_MAP::const_iterator iter = _result_datas.begin();
		RAND_MALL_RES_MAP::const_iterator iend = _result_datas.end();
		for(; iter != iend; ++iter)
		{
			wrapper << iter->first;
			wrapper << iter->second.itemid;
			wrapper << iter->second.price;
			wrapper << iter->second.paytime;
			wrapper << iter->second.reserve1;
			wrapper << iter->second.reserve2;
		}

		return true;
	}

	template <typename WRAPPER>
	bool Load(WRAPPER & wrapper)
	{
		size_t size ;
		wrapper >> size;	
		if(0 == size) return true;

		for(size_t i = 0; i < size; ++i)
		{
			int entryid,itemid,price,paytime,reserve1,reserve2;			
			wrapper >> entryid;
			wrapper >> itemid;
			wrapper >> price;
			wrapper >> paytime;
			wrapper >> reserve1;
			wrapper >> reserve2;
			_result_datas[entryid] = random_mall_result(itemid,price,paytime,reserve1,reserve2);
		}
		return true;
	}
	
	void Swap(random_mall_info & rhs)
	{
		_result_datas.swap(rhs._result_datas);			
	}
	
	bool InitFromDB(archive & ar,int roleid)
	{
		if(0 != ar.size())
		{
			try{
				size_t size ;
				ar >> size;				
				if(0 == size) return true;

				for(size_t i = 0; i < size; ++i)
				{
					int entryid,itemid,price,paytime,reserve1,reserve2;										
					ar >> entryid;
					ar >> itemid;
					ar >> price;
					ar >> paytime;
					ar >> reserve1;
					ar >> reserve2;
					_result_datas[entryid] = random_mall_result(itemid,price,paytime,reserve1,reserve2);
				}
			}
			catch(...)
			{
				_result_datas.clear();
				GLog::log(LOG_ERR,"roleid:%d unmarshal randmall info fail \n",roleid);
				return false;
			}
		}		
		return true;
	}
	bool SaveToDB(archive & ar) const
	{
		size_t size = _result_datas.size();	
		if(0 == size)
			return true;
		ar << size;

		RAND_MALL_RES_MAP::const_iterator iter = _result_datas.begin();
		RAND_MALL_RES_MAP::const_iterator iend = _result_datas.end();
		for(; iter != iend; ++iter)
		{
			ar << iter->first;
			ar << iter->second.itemid;
			ar << iter->second.price;
			ar << iter->second.paytime;
			ar << iter->second.reserve1;
			ar << iter->second.reserve2;
		}
		return true;
	}

	bool IsFirstPay(int eid) { return _result_datas.find(eid) == _result_datas.end();}
	void PendingPay(int eid,const random_mall_result& res) { _result_datas[eid] = res;}
	void ClosingPay(int eid) { _result_datas[eid].itemid = 0; ++_result_datas[eid].paytime;}
	bool QueryResult(int eid, random_mall_result& res) const
	{  
		RAND_MALL_RES_MAP::iterator iter = _result_datas.find(eid);
		if(iter != _result_datas.end())
		{
			res = iter->second;
			return true;
		}
		return false;
	}
};

#pragma pack(1)
struct stall_info
{
	int stallcard_id;
	int max_sell_slot;
	int max_buy_slot;
	int max_name_length;
	stall_info():stallcard_id(-1),max_sell_slot(PLAYER_MARKET_SELL_SLOT),max_buy_slot(PLAYER_MARKET_BUY_SLOT),max_name_length(PLAYER_MARKET_NAME_LEN){}
};
#pragma pack()

template <typename WRAPPER>
WRAPPER & operator<<(WRAPPER & wrapper, const struct stall_info & info)
{
	wrapper.push_back(&info, sizeof(info));
	return wrapper;
}

template <typename WRAPPER>
WRAPPER & operator>>(WRAPPER & wrapper, struct stall_info & info)
{
	wrapper.pop_back(&info, sizeof(info));
	return wrapper;
}

struct dividend_mall_info
{
	int referrer_userid;
	int dividend_add;	//总共获取的鸿利累计值,这一项由delivery写数据库
	int dividend_used;	//总共已使用的鸿利累计值,这一项由gs写数据库
	
	dividend_mall_info():referrer_userid(-1),dividend_add(0),dividend_used(0){}

	bool Save(archive & ar)
	{
		ar << referrer_userid << dividend_add << dividend_used;
		return true;	
	}
	bool Load(archive & ar)
	{
		ar >> referrer_userid >> dividend_add >> dividend_used;
		return true;	
	}

	void SetDividendMallInfo(int _referrer_userid,int _dividend_add,int _dividend_used)
	{
		referrer_userid = _referrer_userid;
		dividend_add = _dividend_add;	
		dividend_used = _dividend_used;
	}

	void GetDividendMallInfo(int & _referrer_userid,int & _dividend_add, int & _dividend_used)
	{
		_referrer_userid = referrer_userid;
		_dividend_add = dividend_add;	
		_dividend_used = dividend_used;
	}

	int GetDividend()
	{
		return dividend_add - dividend_used;	
	}

	void IncDividendAdd(int _dividend)
	{
		dividend_add += _dividend;
	}

	bool SpendDividend(int used)
	{
		int tmp = dividend_used + used;
		if(tmp <= dividend_used || tmp > dividend_add) return false;
		dividend_used = tmp;
		return true;
	}
	
	bool HasReferrer()
	{
		return referrer_userid > 0;	
	}
};

struct pet_enhance		//召唤物可以获取一定比例的召唤者的属性
{
	int hp_percent;
	int mp_percent;
	int damage_percent;
	int magic_damage_percent;
	int defense_percent;
	int magic_defense_percent;
	int attack_degree_percent;
	int defend_degree_percent;
};

struct faction_contrib
{
	int consume_contrib;
	int exp_contrib;
	int cumulate_contrib;
};

struct force_ticket_info
{
	int require_force;
	int repu_inc_ratio;
	force_ticket_info():require_force(0),repu_inc_ratio(0){}
};

class switch_additional_data : public substance
{
public:
	DECLARE_SUBSTANCE(switch_additional_data);
	virtual ~switch_additional_data(){}
	virtual bool Save(archive & ar) { return true;}
	virtual bool Load(archive & ar) { return true;}
};

class countryterritory_switch_data : public switch_additional_data
{
	countryterritory_switch_data(){}
public:
	int country_id;
	int country_expiretime;
	int major_strength;
	int minor_strength;
public:
	DECLARE_SUBSTANCE(countryterritory_switch_data);
	countryterritory_switch_data(int country, int expiretime, int major_str, int minor_str)
		: country_id(country), country_expiretime(expiretime), major_strength(major_str), minor_strength(minor_str){}
	virtual bool Save(archive & ar)
	{ 
		ar << country_id << country_expiretime << major_strength << minor_strength;	
		return true;
	}
	virtual bool Load(archive & ar)
	{
		ar >> country_id >> country_expiretime >> major_strength >> minor_strength;
		return true;
	}
};

class trickbattle_switch_data : public switch_additional_data
{
	trickbattle_switch_data(){}
public:
	int chariot;
public:
	DECLARE_SUBSTANCE(trickbattle_switch_data);
	trickbattle_switch_data(int _chariot) : chariot(_chariot){}
	virtual bool Save(archive & ar)
	{
		ar << chariot;
		return true;
	}
	virtual bool Load(archive & ar)
	{
		ar >> chariot;
		return true;
	}
};

class gplayer_dispatcher : public  dispatcher
{
protected:
	packet_wrapper _mw;
	packet_wrapper _nw;
	packet_wrapper _pw;
	packet_wrapper _self;
	packet_wrapper _tbuf;
	link_sid       _header;
	abase::vector<int,abase::fast_alloc<> > _leave_list;		//保存离开时不再看到的对象ID
public:
	DECLARE_SUBSTANCE(gplayer_dispatcher);
public:
	bool Save(archive & ar)
	{
		ar << _header;
		return true;
	}
	bool Load(archive & ar)
	{
		ar >> _header;
		return true;
	}
	gplayer_dispatcher()
	{}
	virtual void init(gobject_imp * imp)
	{
		dispatcher::init(imp);
		gplayer * pPlayer = (gplayer*)imp->_parent;
		make_link_sid(pPlayer,_header);
	}
	virtual ~gplayer_dispatcher(){}
	virtual void begin_transfer();
	virtual void end_transfer();
	virtual void enter_slice(slice * ,const A3DVECTOR &pos);
	virtual void leave_slice(slice * ,const A3DVECTOR &pos);
	virtual void notify_pos(const A3DVECTOR & pos);
	virtual void get_base_info();
	virtual void enter_world();	//cache
	virtual void leave_world();
	virtual void appear();
	virtual void move(const A3DVECTOR & target, int cost_time,int speed,unsigned char move_mode);
	virtual void stop_move(const A3DVECTOR & target, unsigned short speed,unsigned char dir,unsigned char move_mmode);
	virtual void notify_move(const A3DVECTOR &oldpos, const A3DVECTOR & newpos);
	virtual void be_damaged(const XID & id,int skill_id, const attacker_info_t &,int damage,int dura_index,int at_state,char speed,bool orange,unsigned char section);
	virtual void dodge_attack(const XID &attacker, int skill_id, const attacker_info_t & , int at_state,char speed,bool orange,unsigned char section);
	virtual void be_hurt(const XID & id, const attacker_info_t & ,int damage,bool invader);

	virtual void equipment_damaged(int index,char reason);
	virtual void start_attack(const XID &);
	virtual void attack_once(unsigned char dec_amount);
	virtual void stop_attack(int flag);
	virtual void on_death(const XID &,bool);
	virtual void error_message(int msg);
	virtual void resurrect(int);
	virtual void disappear();
	virtual void pickup_money(int);
	virtual void pickup_item(int type,int expire_item, int amount,int slot_amount, int where,int index);
	virtual void purchase_item(int type, size_t money, int amount,int slot_amount, int where,int index);
	virtual	void query_info00(const XID & target, int cs_index,int sid);
	virtual void query_info00();
	virtual void query_elf_vigor(); //lgc
	virtual void query_elf_enhance();
	virtual void query_elf_stamina(int sta);
	virtual void query_elf_exp(int exp);
	virtual void elf_cmd_result(int cmd, int result, int param1, int param2);
	virtual void elf_levelup(); 
	virtual void receive_exp(int exp,int sp);
	virtual void level_up();
	virtual void unselect();
	virtual void self_item_empty_info(int where,int index);
	virtual void self_item_info(int where,int index,item_data & data,unsigned short crc);
	virtual void self_inventory_data(int where,unsigned char inv_size,const void * data, size_t len);
	virtual void self_inventory_detail_data(int where,unsigned char inv_size,const void * data, size_t len);
	virtual void exchange_inventory_item(size_t idx1,size_t idx2);
	virtual void move_inventory_item(size_t src,size_t dest,size_t count);
	virtual void player_drop_item(size_t where, size_t index,int type,size_t count, unsigned char drop_type);
	virtual void exchange_equipment_item(size_t index1,size_t index2);
	virtual void equip_item(size_t index_inv,size_t index_equip,int count_inv,int count_equip);
	virtual void move_equipment_item(size_t index_inv,size_t index_equip, size_t count);
	virtual void self_get_property(size_t status_point, const extend_prop & prop , int attack_degree, int defend_degree, int crit_rate, int crit_damage_bonus, int invisible_degree, int anti_invisible_degree, int penetration, int resilience, int vigour);
	virtual void set_status_point(size_t vit, size_t eng, size_t str, size_t agi, size_t remain);
	virtual void player_select_target(int id);
	virtual void get_extprop_base();
	virtual void get_extprop_move();
	virtual void get_extprop_attack();
	virtual void get_extprop_defense();
	virtual void player_reject_invite(const XID & member);
	virtual void leader_invite(const XID & leader,int seq,int pickup_flag);
	virtual void join_team(const XID & leader,int pickup_flag);
	virtual void member_leave(const XID & leader, const XID& member,int type);
	virtual void leave_party(const XID & leader, int type);
	virtual void new_member(const XID & member);
	virtual void leader_cancel_party(const XID & leader);
	virtual void teammate_get_pos(const XID & target,const A3DVECTOR & pos, int tag, bool same_plane);
	virtual void send_equipment_info(const XID & target, int cs_index, int sid);
	virtual void equipment_info_changed(uint64_t madd,uint64_t mdel,const void * buf, size_t size);
	virtual void team_member_pickup(const XID & target,int type, int count);
	virtual void npc_greeting(const XID & provider);
	virtual void repair_all(size_t cost);
	virtual void repair(int where,int index,size_t cost);
	virtual void renew();
	virtual void spend_money(size_t cost);
	virtual void get_player_money(size_t amount,size_t capacity);
	virtual void cast_skill(const XID & target, int skill,unsigned short time, unsigned char level);
	virtual void cast_rune_skill(const XID & target, int skill,unsigned short time, unsigned char level);
	virtual void skill_interrupt(char reason);
	virtual void skill_perform();
	virtual void stop_skill();
	virtual void get_skill_data();
	virtual void clear_embedded_chip(unsigned short equip_idx,size_t use_money);
	virtual void cost_skill_point(int sp);
	virtual void learn_skill(int skill, int level);
	virtual void takeoff();
	virtual void landing();
	virtual void flysword_time_capacity(unsigned char where, unsigned char index, int cur_time);
	virtual void obtain_item(int type, int expire_date, int amount,int slot_amount, int where,int index);
	virtual void produce_start(int type, int use_time ,int count);
	virtual void produce_once(int type, int amount,int slot_amount, int where,int index);
	virtual void produce_end();
	virtual void decompose_start(int use_time,int type);
	virtual void decompose_end();
	virtual void get_task_data();
	virtual void send_task_var_data(const void * buf, size_t size);
	virtual void start_use_item(int item_type, int use_time);
	virtual void cancel_use_item();
	virtual void use_item(char where, unsigned char index , int item_type , unsigned short count); //给自己
	virtual void use_item(int item_type); //给他人
	virtual void use_item(char where, unsigned char index , int item_type , unsigned short count,const char * arg, size_t arg_size);
	virtual void use_item(int item_type,const char * arg, size_t arg_size);
	virtual void start_use_item_with_target(int item_type, int use_time,const XID & target);
	virtual void sit_down();
	virtual void stand_up();
	virtual void do_emote(unsigned short emotion);
	virtual void do_emote_restore(unsigned short emotion);
	virtual void do_action(unsigned char action);
	virtual void send_timestamp();
	virtual void notify_root(unsigned char type);
	virtual void dispel_root(unsigned char type);
	virtual void invader_rise();
	virtual void pariah_rise();
	virtual void invader_fade();
	virtual void gather_start(int mine,unsigned char t);
	virtual void gather_stop();
	virtual void trashbox_passwd_changed(bool has_passwd);
	virtual void trashbox_passwd_state(bool has_passwd);
	virtual void trashbox_open(char is_usertrashbox);
	virtual void trashbox_close(char is_usertrashbox);
	virtual void trashbox_wealth(char is_usertrashbox, size_t money);
	virtual void exchange_trashbox_item(int where, size_t idx1, size_t idx2);
	virtual void move_trashbox_item(int where, size_t src, size_t dest, size_t delta);
	virtual void exchange_trashbox_inventory(int where, size_t idx_tra,size_t idx_inv);
	virtual void trash_item_to_inventory(int where, size_t idx_tra, size_t idx_inv, size_t delta);
	virtual void inventory_item_to_trash(int where, size_t idx_inv, size_t idx_tra, size_t delta);
	virtual void exchange_trash_money(char is_usertrashbox, int inv_money, int tra_money);
	virtual void set_adv_data(int data1,int data2);
	virtual void clear_adv_data();
	virtual void player_in_team(unsigned char state);
	virtual void send_party_apply(int id);
	virtual void query_info_1(int uid,int cs_index, int cs_sid);
	virtual void concurrent_emote_request(int id, unsigned short action);
	virtual void do_concurrent_emote(int id, unsigned short action);
	virtual void mafia_info_notify();
	virtual void task_deliver_reputaion(int offset,int cur_reputaion);
	virtual void task_deliver_exp(int exp, int sp);
	virtual void task_deliver_money(size_t amount,size_t cur_money);
	virtual void task_deliver_item(int type, int expire_date,int amount,int slot_amount, int where,int index);
	virtual void task_deliver_level2(int level2);
	virtual void get_reputation(int reputation);
	virtual void identify_result(char index, char result);
	virtual void change_shape(char shape);
	virtual void elf_refine_activate(char);		//lgc
	virtual void enter_sanctuary();
	virtual void leave_sanctuary();
	virtual void begin_personal_market(int market_id,const char *name, size_t len);
	virtual void cancel_personal_market();
	virtual void market_trade_success(int trader);
	virtual void send_market_name(const XID & target, int cs_index, int sid,const char * name ,size_t len);
	virtual void player_start_travel(int  line_no,const A3DVECTOR & dest_pos,float speed,int vehicle);
	virtual void player_complete_travel(int vehicle);
	virtual void gm_toggle_invisible(char tmp);
	virtual void toggle_invincible(char tmp);
	virtual void trace_cur_pos(unsigned short seq);
	virtual void cast_instant_skill(const XID & target, int skill,unsigned char level);
	virtual void cast_rune_instant_skill(const XID & target, int skill,unsigned char level);
	virtual void cast_elf_skill(const XID & target, int skill,unsigned char level);		//lgc
	virtual void activate_waypoint(unsigned short waypoint);
	virtual void player_waypoint_list(const unsigned short * buf, size_t count);
	virtual void unlock_inventory_slot(unsigned char where, unsigned short index);
	virtual void team_invite_timeout(int who);
	virtual void enable_pvp_state(char type);
	virtual void disable_pvp_state(char type);
	virtual void player_pvp_cooldown(int cooldown);
	virtual void send_cooldown_data();
	virtual void skill_ability_notify(int id, int ability);
	virtual void personal_market_available();
	virtual void breath_data(int breath, int breath_capacity);
	virtual void stop_dive();
	virtual void trade_away_item(int buyer,short inv_idx,int type, size_t count);
	virtual void player_enable_fashion_mode(char is_enable);
	virtual void player_enable_free_pvp(char is_enable);
	virtual void player_enable_effect(short effect);
	virtual void player_disable_effect(short effect);
	virtual void enable_resurrect_state(float exp_reduce);
	virtual void set_cooldown(int idx, int cooldown);
	virtual void change_team_leader(const XID & old_leader, const XID & new_leader);
	virtual void kickout_instance(char reason, int timeout);
	virtual void begin_cosmetic(unsigned short inv_index);
	virtual void end_cosmetic(unsigned short inv_index);
	virtual void cosmetic_success(unsigned short crc);
	virtual void cast_pos_skill(const A3DVECTOR & pos, const XID &target, int skill,unsigned short time,unsigned char level);
	virtual void change_move_seq(unsigned short seq);
	virtual void server_config_data();
	virtual void active_rush_mode(char is_active);
	virtual void produce_null(int recipe_id);
	virtual void enable_double_exp_time(int mode, int end_time);
	virtual void available_double_exp_time();
	virtual void active_pvp_combat_state(bool is_active);
	virtual void duel_recv_request(const XID &target);
	virtual void duel_reject_request(const XID &target,int reason);
	virtual void duel_prepare(const XID & target, int delay);
	virtual void duel_cancel(const XID & target);
	virtual void duel_start(const XID & who);
	virtual void duel_stop();
	virtual void duel_result(const XID & target, bool is_failed);
	virtual void player_bind_request(const XID & target);
	virtual void player_bind_invite(const XID & target);
	virtual void player_bind_request_reply(const XID & target, int param);
	virtual void player_bind_invite_reply(const XID & target, int param); 
	virtual void player_bind_start(const XID & target);
	virtual void player_bind_stop();
	virtual void player_mounting(int mount_id, unsigned short mount_color);
	virtual void send_equip_detail(int cs_indx, int cs_sid, int target, const void * data, size_t size);
	virtual void send_inventory_detail(int cs_indx, int cs_sid, int target, size_t money, unsigned char inv_size, const void * data, size_t size);
	virtual void send_others_property(const void * data, size_t size, const void * self_data, size_t self_size);
	virtual void pariah_duration(int remain_time);

	virtual void gain_pet(int index, const void * buf, size_t size);
	virtual void free_pet(int index, int pet_id);
	virtual void summon_pet(int index, int pet_tid, int pet_id, int life_time);
	virtual void start_pet_operation(int index, int pet_id,int delay,int operation);
	virtual void end_pet_operation();
	virtual void recall_pet(int index, int pet_id, char reason);
	virtual void pet_recv_exp(int index, int pet_id, int exp);
	virtual void pet_level_up(int index, int pet_id, int level, int cur_exp);
	virtual void pet_room_capacity(int cap);
	virtual void notify_pet_honor(int index,int honor);
	virtual void notify_pet_hunger(int index, int hunger);
	virtual void enter_battleground(int role,int battle_id, int end_time);
	virtual void battle_result(int result);
	virtual void battle_score(int oscore, int ogoal, int dscore, int dgoal);
	virtual void pet_dead(int index);
	virtual void pet_revive(int index,float hp_factor);
	virtual void pet_hp_notify(int index, float hp_factor, int cur_hp, float mp_factor, int cur_mp);
	virtual void pet_ai_state(char aggro_state, char stay_state);
	virtual void refine_result(int rst);
	virtual void pet_set_cooldown(int index, int cd_index, int msec);
	virtual void player_cash(int cash);
	virtual void player_bind_success(size_t index, int id);
	virtual void player_change_inventory_size(int size);
	virtual void player_pvp_mode();
	virtual void player_wallow_info(int level, int ptime,int light_t, int heavy_t, int reason);
	

	void mafia_trade_start();
	void mafia_trade_end();
	//下面函数是player专属的组队信息传递函数,其他对象暂时没有 所以暂时不是虚函数
	void send_team_data(const XID & leader,size_t team_count,size_t data_count, const player_team::member_entry ** list);
	void send_team_data(const XID & leader,size_t team_count,const player_team::member_entry * list);

	void item_to_money(size_t index, int type,size_t count , size_t price);
	void pickup_money_in_trade(size_t);
	void pickup_item_in_trade(int type, int amount);
	void receive_money_after_trade(size_t);
	void receive_item_after_trade(int type,int expire_date, int amount, int slot_amount, int index);
	void embed_item(size_t chip_idx, size_t equip_idx);
	void object_is_invalid(int id);
	void trashbox_capacity_notify(int where, int cap);
	void send_pet_room(pet_data **,size_t start, size_t end);
	void player_change_spouse(int id);
    void equip_can_inherit_addons(int equip_id, unsigned char inv_idx, unsigned char addon_num, int addon_id_list[]);
	virtual void notify_safe_lock(char active, int time, int max_time);
	virtual void mall_item_buy_failed(short index, char reason);//lgc
	virtual void equip_trashbox_item(int where, unsigned char trash_idx,unsigned char equip_idx); 
	virtual void security_passwd_checked();
	virtual void appear_to_spec(int invi_degree);		//离开invi_degree级隐身状态,向指定人发送appear消息
	virtual void disappear_to_spec(int invi_degree);	//进入invi_degree级隐身状态,向指定人发送disappear消息
	virtual void on_inc_invisible(int prev_invi_degree, int cur_invi_degree);	//隐身等级提高向指定人发送disappear消息
	virtual void on_dec_invisible(int prev_invi_degree, int cur_invi_degree);	//隐身等级降低向指定人发送appear消息
	virtual void on_inc_anti_invisible(int prev_a_invi_degree, int cur_a_invi_degree);	//反隐等级提高获取新的可视对象
	virtual void on_dec_anti_invisible(int prev_a_invi_degree, int cur_a_invi_degree);	//反隐等级降低获取不再可见的对象
	virtual void hp_steal(int hp);
	virtual void player_dividend(int dividend);
	virtual void dividend_mall_item_buy_failed(short index, char reason);
	virtual void multi_exp_info(int last_timestamp, float enhance_factor);
	virtual void change_multi_exp_state(char state, int enchance_time, int buffer_time, int impair_time, int activate_times_left);
	virtual void send_world_life_time(int life_time);
	virtual void wedding_book_success(int type);
	virtual void calc_network_delay(int timestamp);
	virtual void player_knockback(const A3DVECTOR & pos, int time);
	virtual void summon_plant_pet(int plant_tid/*模板ID*/, int plant_id/*世界ID*/, int life_time);
	virtual void plant_pet_disappear(int id, char reason);
	virtual void plant_pet_hp_notify(int id, float hp_factor, int cur_hp, float mp_factor, int cur_mp);
	virtual void pet_property(int index, const extend_prop & prop);
	virtual void faction_contrib_notify();
	virtual void faction_relation_notify();
	virtual void enter_factionfortress(int role_in_war, int factionid, int offense_endtime);
	virtual void player_equip_disabled(int64_t mask);
	virtual void send_spec_item_list(int cs_index, int cs_sid, int target, int type, void * data, size_t size);
	virtual void send_error_message(int cs_index, int cs_sid, int target, int message);
	virtual void congregate_request(char type, int sponsor, int timeout);
	virtual void reject_congregate(char type, int id);
	virtual void congregate_start(char type, int time);
	virtual void cancel_congregate(char type);
	virtual void engrave_start(int recipe_id, int use_time);
	virtual void engrave_end();
	virtual void engrave_result(int addon_num);
	virtual void addonregen_start(int recipe_id, int use_time);
	virtual void addonregen_end();
	virtual void addonregen_result(int addon_num);
	virtual void invisible_obj_list(gobject ** ppObject, size_t count);
	virtual void set_player_limit(int index, char b);
	virtual void player_teleport(const A3DVECTOR& pos, unsigned short use_time, char mode);
	virtual void forbid_be_selected(char b);
	virtual void send_player_force_data(int cur_force, size_t count, const void* data, size_t data_size);
	virtual void player_force_changed(int force);
	virtual void player_force_data_update(int force, int repu, int contri);
	virtual void send_force_global_data(char data_ready, size_t count, const void * data, size_t data_size);
	virtual void add_multiobj_effect(int target, char type);
	virtual void remove_multiobj_effect(int target, char type);
	virtual void enter_wedding_scene(int groom, int bride);
	virtual void produce4_item_info(int stime, item_data & data, unsigned short crc); //发送客户端新继承生产产生的物品信息
	virtual void online_award_data(int total_award_time, size_t count, const void * data, size_t data_size);
	virtual void toggle_online_award(int type, char activate);
	virtual void update_profit_time(char flag, int profit_time, int profit_level);
	virtual void notify_profit_state(char state);
	virtual void enter_nonpenalty_pvp_state(char state);
	virtual void self_country_notify(int country_id);
	virtual void player_country_changed(int country_id);
	virtual void enter_countrybattle(int role, int battle_id,int end_time,int offense_country, int defence_country);
	virtual void countrybattle_result(int result);
	virtual void countrybattle_score(int oscore, int ogoal, int dscore, int dgoal);
	virtual void countrybattle_resurrect_rest_times(int times);
	virtual void countrybattle_became_flag_carrier(char is_carrier);
	virtual void countrybattle_personal_score(int combat_time, int attend_time, int kill_count, int death_count, int country_kill_count, int country_death_count);
	virtual void defense_rune_enabled(char rune_type, char enable);
	virtual void countrybattle_info(int attacker_count, int defender_count);
	virtual void cash_money_exchg_rate(char open, int rate);
	virtual void pet_rebuild_inherit_start(size_t index,int use_time);
	virtual void pet_rebuild_inherit_info(int stime,int pet_id,size_t index,int r_attack,int r_defense,int r_hp,int r_atk_lvl,int r_def_lvl); 
	virtual void pet_rebuild_inherit_end(size_t index);
	virtual void pet_evolution_done(size_t index);
	virtual void pet_rebuild_nature_start(size_t index,int use_time);
	virtual void pet_rebuild_nature_info(int stime,int pet_id,size_t index,int nature);
	virtual void pet_rebuild_nature_end(size_t index);
	virtual void notify_meridian_data(int meridian_level,int lifegate_times,int deathgate_times,int free_refine_times,int paid_refine_times,int continu_login_days,int trigrams_map[3]);
	virtual void try_refine_meridian_result(int index,int result);
	virtual void equip_addon_update_notify(unsigned char update_type,unsigned char equip_idx,unsigned char equip_socket_idx,int old_stone_type,int new_stone_type);
	virtual void self_king_notify(char is_king, int expire_time);
	virtual void player_king_changed(char is_king);
	virtual void notify_touch_query(int64_t income,int64_t remain,int retcode);
	virtual void notify_touch_cost(int64_t income,int64_t remain,unsigned int cost,unsigned int index,unsigned int lots,int retcode);
	virtual void notify_addup_money(int64_t addupmoney);
	virtual void query_title(int roleid,int count,int ecount, const void * data, size_t data_size, const void *edata, size_t edata_size);
	virtual void notify_curr_title_change(int roleid,unsigned short titleid);
	virtual void notify_title_modify(unsigned short titleid,int expiretime,char flag);
	virtual void refresh_signin(char type,int moncal,int cys,int lys, int uptime, int localtime, char awardedtimes, char latesignintimes);
	virtual void notify_giftcard_redeem(int retcode,int cardtype,int parenttype, const char(&cardnumber)[player_giftcard::GIFT_CARDNUMBER_LEN] );
	virtual void player_reincarnation(size_t reincarnation_times);
	virtual void activate_reincarnation_tome(char active);
	virtual void realm_exp_receive(int exp,int receive_exp);
	virtual void realm_level_up(unsigned char level);
	virtual void enter_trickbattle(int role, int battle_id,int end_time);
	virtual void trickbattle_personal_score(int kill, int death, int score, int multi_kill);
	virtual void trickbattle_chariot_info(int chariot, int energy);
	virtual void player_leadership(int leadership, int inc_leadership);
	virtual void generalcard_collection_data(const void * buf, size_t size);
	virtual void add_generalcard_collection(size_t collection_idx);
	virtual void player_active_combat(bool is_active);
	virtual void random_mall_shopping_result(int eid,int opt,int res,int item,int price,bool flag);
	virtual void player_mafia_pvp_mask_notify(unsigned char mafia_pvp_mask);
	virtual void player_world_contribution(int world_contrib, int change, int total_cost);
	virtual void randommap_order_init(int row,int col,const int* room_src);
	virtual void send_scene_service_npc_list(unsigned int count, int * data);
	virtual void player_screen_effect_notify(int type, int eid, int state);
	virtual void player_combo_skill_prepare(int skillid,int timestamp,int arg1, int arg2, int arg3);
	virtual void player_pray_distance_change(float pd);
	virtual void instance_reenter_notify(int tag, int timeout);

public:
	friend class gplayer_imp;
	virtual void LoadFrom(gplayer_dispatcher * rhs)		//从原有的dispatcher中取得数据
	{
		memcpy(&_header , &rhs->_header,sizeof(_header));
	}
protected:
	virtual void enter_region();	//为了给gm更多权限，将其改为虚函数，在gm_dispatcher重写这个函数
};

class gplayer_controller : public controller
{
protected:
	XID  _cur_target;
	XID  _cur_sub_target;  // 目标的目标
	bool _target_refresh_state;	// 目标更新与否通知订阅者
	A3DVECTOR _last_pos;
	int  _load_stats;
	short _peep_counter;		//定时发送仇恨广播的定时器
	short _select_counter;		//定期进行查询对象是否存在的计数器
	bool  _debug_command_enable;	//特殊命令是否激活
	char  _banish_counter;
	unsigned short _move_cmd_seq;
	short  _deny_cmd_list[CMD_MAX];
	short  _pickup_counter;
	int    _pickup_timestamp;
	int    _safe_lock_timestamp;	//还剩多长时间安全琐打开,保护时间为时限时有效
	int    _max_safe_lock;			//安全琐保护时间,-1为永久 >0为时限
	GNET::Privilege * _gm_auth;
	enum 
	{
		MAX_PICKUP_PER_SECOND = 5, //最快每秒捡起五次
		STD_PICKUP_PER_SECOND = 2, //平均每秒捡起二次
	};
private:
	//放在这里禁止子类调用的目的是 禁止子类添加独特的需要保存的数据结构，使得各种结构保存均一致
	virtual bool Save(archive & ar);
	virtual bool Load(archive & ar);


	inline bool CheckBanish()
	{
		_load_stats ++;
		if(_banish_counter >0)
		{
			_load_stats += (_banish_counter >> 2) + 2;
			return false;
		}
		return true;
	}
	
	inline void IncreaseLoadStat(int offset)
	{
		_load_stats += offset;
	}

	inline bool CheckDeny(size_t  type)
	{
		return _deny_cmd_list[type];
	}
public:
	inline const XID & GetCurTarget() { return _cur_target;}
	void SetPrivilege(const void * data, size_t size);
	inline void SetDebugMode(bool is_debug)
	{
		_debug_command_enable = is_debug;
	}
	
	inline bool GetDebugMode()
	{
		return _debug_command_enable;
	}
	virtual bool HasGMPrivilege();

	inline void SetSafeLock(int locktime, int max_locktime)
	{
		_safe_lock_timestamp = locktime;
		_max_safe_lock= max_locktime;
		_imp->_runner->notify_safe_lock( InSafeLock()?1:0, _safe_lock_timestamp, _max_safe_lock);
	}

	inline bool InSafeLock()
	{       
		return _max_safe_lock == -1 || _safe_lock_timestamp > 0;
	}

	inline bool TestSafeLock()
	{
		if(InSafeLock())
		{
			error_cmd(S2C::ERR_FORBIDDED_OPERATION_IN_SAFE_LOCK);
			return false;
		}
		return true;
	}
public:
	DECLARE_SUBSTANCE(gplayer_controller);

	gplayer_controller();
	~gplayer_controller();

	virtual void Init(gobject_imp * imp);
	virtual void ReInit();

	virtual int GetCurMoveSeq() 
	{ 
		return _move_cmd_seq;
	}

	virtual int GetNextMoveSeq() 
	{ 
		_move_cmd_seq = (_move_cmd_seq + 1 ) & 0xFFFF;
		return _move_cmd_seq;
	}

	virtual void SetNextMoveSeq(int seq) 
	{ 
		_move_cmd_seq = seq;
	}

	virtual int MessageHandler(world * pPlane, const MSG & msg);

	virtual int DebugCommandHandler(int cmd_type,const void * buf, size_t size);
	virtual int GMCommandHandler(int cmd_type,const void * buf, size_t size);
	//各种状态下的命令处理函数
	virtual int UnLockInventoryHandler(int cmd_type,const void * buf, size_t size);
	virtual int CommandHandler(int cmd_type,const void * buf, size_t size);
	virtual int ZombieCommandHandler(int cmd_type,const void * buf, size_t size);
	virtual int SealedCommandHandler(int cmd_type,const void * buf, size_t size);
	virtual int StayInCommandHandler(int cmd_type,const void * buf, size_t size);
	virtual int MarketCommandHandler(int cmd_type,const void * buf, size_t size);
	virtual int TravelCommandHandler(int cmd_type,const void * buf, size_t size);
	virtual int CosmeticCommandHandler(int cmd_type, const void * buf, size_t size);
	virtual int BoundCommandHandler(int cmd_type, const void * buf, size_t size);

	virtual int MoveBetweenSlice(gobject * obj,slice * src, slice * dest);
	virtual void SwitchSvr(int dest, const A3DVECTOR & oldpos,const A3DVECTOR &newpos,const instance_key *switch_key);
	virtual void Release(bool free_parent);	//这个函数会忽视free_parent
	virtual	void OnHeartbeat(size_t tick);
	virtual void DenyCmd(size_t cmd_type);
	virtual void AllowCmd(size_t cmd_type);

	void ResurrectByItem(float exp_reduce, int param);
	void ResurrectInTown(float exp_reduce, int param);
public:
	void OnResurrect();
protected:
	void TryPeepMobs();
	void error_cmd(int msg);
	int cmd_user_move(const void * buf, size_t size);
	int cmd_user_stop_move(const void * buf, size_t size);
	friend class gplayer_imp;
	
public:
	virtual	void LoadFrom(gplayer_controller * rhs)		//从原有的controller中取得数据
	{
		_cur_target = rhs->_cur_target;
		_cur_sub_target = rhs->_cur_sub_target;
		_target_refresh_state = rhs->_target_refresh_state;
		_last_pos = rhs->_last_pos;
		_peep_counter = rhs->_peep_counter;
		_select_counter = rhs->_select_counter;
		_load_stats = rhs->_load_stats;
		_debug_command_enable = rhs->_debug_command_enable;
		_banish_counter = rhs->_banish_counter;
		_move_cmd_seq = rhs->_move_cmd_seq;
		_safe_lock_timestamp = rhs->_safe_lock_timestamp;
		_max_safe_lock =  rhs->_max_safe_lock;
		
		memcpy(_deny_cmd_list,rhs->_deny_cmd_list,sizeof(_deny_cmd_list));

		ASSERT(!_gm_auth);
		_gm_auth = rhs->_gm_auth;
		rhs->_gm_auth = NULL;
	}
	
	//内部调用的函数
	template < typename T>
	inline void TrySelect(const T & attacker)
	{
		if(_cur_target.type == -1)
		{
			SelectTarget(attacker.id);
		}
	}

	void UnSelect(bool force=false);
	void SelectTarget(int id, bool force=false);
	void SubscibeConfirm(const XID & who);

	void SelectSubTarget(const XID & who, int id);
	void SecondSubscibeConfirm(const XID & who);
};

class player_trade;
class player_stall;
class gplayer_imp : public gactive_imp 
{
protected:
	typedef abase::static_multimap<int, int, abase::fast_alloc<> >  ENEMY_LIST;
	instance_key	_instance_switch_key;	//进行副本转移时使用的key
	item_list 	_inventory;		//物品栏
	item_list	_equipment;		//装备栏
	item_list	_task_inventory;	//任务物品栏
	size_t 		_player_money;		//钱数
	size_t		_money_capacity;	//钱数上限	
	int		_player_state;		//玩家的基本状态
	int		_combat_timer;		//计算最近一次格斗的时间
	int		_reputation;		//声望
	player_team	_team;			//队伍数据
	player_duel	_duel;			//决斗数据
	player_bind	_bind_player;		//玩家间绑定
	player_invade	_invade_ctrl;		//红名粉名控制数据（包含状态的副本）
	player_trade  * _trade_obj;		//交易对象，只有交易状态下生效
	player_stall  * _stall_obj;		//摆摊对象，只有摆摊的时候才生效
	int		_stall_trade_id;	//摆摊次数，用于校验摆摊的正确性
	int		_stall_trade_timer;	//记录上次摆摊的时间戳，必须有差额才能摆摊 不需要再切换中保存
	struct stall_info _stall_info;	//摊位的容量，名称长度
	int		_last_move_mode;	//上一次的移动模式
	ENEMY_LIST _enemy_list;			//敌人列表，即自己处于那些怪物的仇恨度列表头部
	int		_pvp_cooldown;		//pvp冷却时间
	int		_ap_per_hit;		//一次攻击增长的怒气值
	breath_ctrl	_breath;
	phase_control _ph_control;	//检查玩家移动的合法性，解决飞天挂连续跳问题
	int		_db_user_id;		//数据库需要的奇特id
	struct
	{
		uint64_t mask;
		abase::octets data;
	} _equip_info;				//装备信息 即需要其他人知道的装备类型，供发给客户端显示用

	struct
	{
		XID  id;
		A3DVECTOR pos;			//这个坐标也用于临时保存旅行时的目标点
		int id_mafia;
	} _provider;				//当前给自己提供服务的商人的坐标和id  


	unsigned char _inv_level;		//包裹栏的等级，是变化量 ,这个值决定了包裹栏的数量和金钱的最大上限 
	bool	_is_moved;			//给gnpc_controller使用的_is_moved
	bool	_trash_box_open_flag;		//物品箱打开标志
	bool	_trash_box_open_view_only_flag;		//物品箱打开标志
	bool	_user_trash_box_open_flag;	//帐号物品箱打开标志
	bool 	_security_passwd_checked;	//安全密码(仓库密码)检查通过标志
	bool	_pvp_enable_flag;		//是否开启了PVP标志
	char	_force_attack;			//客户端发来的强制攻击标志，在技能的带有攻击的击中效果中使用
	char 	_refuse_bless;			//客户端发来的不接受祝福标志
	unsigned char _write_counter;		//在本次服务器上写入的次数 目前没什么用
	bool	_kill_by_player;		//是否被玩家杀死的
	bool	_free_pvp_mode;			//是否在竞技场模式  这个在切换服务器时不保存
	bool	_nonpenalty_pvp_state;	//是否是无惩罚PVP状态，此状态下杀人不红名，死亡不掉落
	bool    _no_cooldown_mode;		//是否无cooldown模式  不存盘 不切换
	unsigned short _eq_change_counter;	//装备更改标志，每次更改装备这个值会加一，如果为0则表示已经写入
	unsigned short _tb_change_counter;	//物品箱写入标志，每次打开箱子会加一，如果为0则表示已经写入
	unsigned short _user_tb_change_counter;	//帐号物品箱写入标志，每次打开箱子会加一，如果为0则表示已经写入

	float	_resurrect_exp_reduce;		//原地复活时减少的exp数目
	float 	_resurrect_hp_factor;		//原地复活后血量比例
	float 	_resurrect_mp_factor;		//原地复活后蓝比例
	int		_resurrect_exp_lost_reduce;	//所有方式复活经验损失减少百分比，被动技能使用
	int 	_con_emote_target;		//邀请协同做动作的对象
	int	_pvp_combat_timer;		//是否出于pvp中 以及pvp中的剩余时间(秒)
	unsigned short _con_emote_id;		//邀请协同动作的动作id

	bool	_resurrect_state;		//是否可以原地复活的标志
	unsigned char _logout_pos_flag;		//是否使用特殊坐标登出
	A3DVECTOR _logout_pos;			//登出时使用的坐标
	int	  _logout_tag;			//登出时使用的tag

	A3DVECTOR _last_instance_pos;		//最后一次进入副本后的坐标位置 无需保存
	int	  _last_instance_tag;		//最后一次进入副本后的世界ID   无需保存
	int 	  _last_instance_logout_ts; //最后一次退出副本的时间戳     无需保存	
	int 	  _last_instance_timestamp;	//最后一次进入副本的时间戳     无需保存
	A3DVECTOR _last_source_instance_pos;	//最后一次进入副本后的坐标位置 无需保存
	int	  _last_source_instance_tag;	//最后一次进入副本后的世界ID   无需保存
	int	  _db_save_error;		//数据存盘错误计数
	int	  _db_timestamp;		//数据库存盘计数，这个很重要
	char	  _username[MAX_USERNAME_LENGTH];	//玩家的名字，最大10个unicode字符
	size_t	  _username_len;		//玩家名字的长度，以字节为单位
	
	int	  _double_exp_timeout;		//双倍经验时间到期时间 最多不能超过多少小时
	int	  _double_exp_mode;		//双倍时间的类型 0 无，1 大卡 2 物品 3 其他
	int	  _rest_counter_time;		//上次计算双倍时间的计时点
	int 	  _rest_time_used;		//本计时周期内已经使用的计时点
	int	  _rest_time_capacity;		//目前可用多少双倍时间
	int	  _mafia_rest_time;		//帮派奖励的双倍时间 这个时间和上面的时间和起来是最终可用的双倍时间
	int	  _mafia_rest_counter_time;	//上次计算帮派奖励的时间点
	int	  _login_timestamp;		//登录时的时间戳
	int	  _played_time;			//本角色已经进行游戏的时间
	int   _last_login_timestamp;	//上次登陆的时间戳
	int	  _create_timestamp;		//角色创建时间
	int	  _spec_task_reward;		//数据库发奖
	int	  _spec_task_reward2;		//数据库发奖
	int	  _spec_task_reward_param;	//数据库发奖的参数
	int	  _spec_task_reward_mask;	//新的数据库发奖，是一个mask
	float	  _speed_ctrl_factor;		//限制速度的统计因子，意义是当前最近用过的最大合法速度此值无需存盘
	int	  _duel_target;			//决斗时的目标，传送给宠物使用的
	bool	  _inv_switch_save_flag;	//在切换副本时记录是否有离开副本消失物品的标志，有此标志在切换完成后就会给客户端更新包裹数据
	bool	  _eqp_switch_save_flag;	//同上
	bool	  _tsk_switch_save_flag;	//同上

	abase::vector<unsigned short, abase::fast_alloc<> > _waypoint_list;	//已激活路点的列表
	cd_manager _cooldown;
	pet_manager _petman;			//宠物管理
	plant_pet_manager _plantpetman;	//植物管理,因植物切换服务器消失，所以不需保存
	
	elf_info _cur_elf_info;			//当前装备的小精灵信息，//lgc
	int _min_elf_status_value;		//保存所有处于转化状态的小精灵中最小status_value值
	
	int       _mall_cash;                   //原来有的可用点数
	int       _mall_cash_used;              //总共使用了多少点数
	int       _mall_cash_offset;            //消耗了多少点数
	int 	  _mall_cash_add;				//历史累计冲值点数,仅在任务中使用
	int       _mall_order_id;               //商场购物的流水号
	int       _mall_order_id_saved;         //商场购物的流水号，最近一次存盘的流水号
	abase::vector<netgame::mall_invoice, abase::fast_alloc<> > _mall_invoice;
	int		  _mall_consumption;			//消费值

	char	  _cheat_punish;		//作弊惩罚
	char	  _cheat_mode;			//作弊标志
	char 	  _cheat_report;		//是否向delivery汇报过作弊
	char	  _chat_emote;			//聊天表情 ID 
	int	  _auto_hp_value;		//自动回血状态的值
	float	  _auto_hp_percent;
	int	  _auto_mp_value;		//自动回魔状态的值
	float	  _auto_mp_percent;

	int _equip_refine_level[item::EQUIP_INVENTORY_COUNT];//身上装备的精炼等级 
	int _soul_power;									 //根据精炼等级计算出来的魂力
	int _soul_power_en;									//附加属性提升的魂力值
	int _min_addon_expire_date;		//是否存在有期限的附加属性，这里保存最近一次到期的到期时间
	pet_enhance _pet_enhance;	//召唤物可以获取一定比例的召唤者的属性,被动技能改变此值
	player_limit _player_limit;
	XID _skill_attack_transmit_target;
	int _country_expire_time;	//国家过期时间点
	bool _in_central_server;	//是否在跨服中央服中,根据登陆时flag设置
	int _src_zoneid;			//如果在跨服中央服，表示来自哪个服务器
	int _king_expire_time;		//国王过期时间点
	touch_trade _touch_order;       //Touch交易 
	player_title _player_title;    // 角色称号
	player_dailysign _player_dailysign; // 日常签到
	player_giftcard	_player_giftcard;  // 礼品卡
	player_fatering _player_fatering;	//命轮
    player_sanctuary_check _player_sanctuary_check; // 检查玩家安全区状态
	player_instance_reenter _player_instance_reenter; // 玩家重新进入副本 无需save load
	player_clock _player_clock;  // 给到期刷新组件提供时间提醒
	//--> 需要到期刷新组件顺序排列
	//<--
	random_mall_info _player_randmall;// 随机商城数据

	bool _need_refresh_equipment;	//装备栏是否需要刷新 切换场景时无需保存
	unsigned char _realm_level;		// 境界等级
	int _realm_exp;					// 境界经验
	int _leadership;
	int _leadership_occupied;
	int _world_contribution;
	int _world_contribution_cost;

	friend class gplayer_controller;
	friend class gplayer_dispatcher;
public:
/*
	Player 状态
*/
	enum
	{
		PLAYER_STATE_NORMAL,		//通常状态
		PLAYER_WAITING_TRADE,		//交易状态 
		PLAYER_TRADE,			//交易状态 
		PLAYER_WAIT_TRADE_COMPLETE,	//交易状态 
		PLAYER_WAIT_TRADE_READ,		//交易状态
		PLAYER_WAITING_FACTION_TRADE,	//交易状态 
		PLAYER_WAIT_LOGOUT,		//玩家正在等待登出
		PLAYER_DISCONNECT,		//player 已经断线，但是在等待退出的时间
		PLAYER_WAIT_SWITCH,		//player 在等待切换服务器操作
		PLAYER_SIT_DOWN,		//player 处于坐下的状态
		PLAYER_STATE_MARKET,		//player 处于摆摊的状态
		PLAYER_STATE_TRAVEL,		//player 处于旅行的状态
		PLAYER_STATE_COSMETIC,		//player 处于整容的状态 
		PLAYER_STATE_BIND,		//player 处于连接状态
		PLAYER_WAIT_FACTION_TRADE_READ, //player 处于帮派交易超时并试图读取数据的等待状态
	};

/*
	客户端特殊显示类型
*/
	enum
	{
		CLIENT_SCREEN_EFFECT,
		CLIENT_GFX_EFFECT,
	};

	DECLARE_SUBSTANCE(gplayer_imp);
	int 	_disconnect_timeout;	//断线时的超时设置
	int 	_offline_type;		//断线的类型，登出，断线或者踢出
	int	_write_timer;		//写盘使用的计时器
	int	_link_notify_timer;	//给link server发送心跳的定时器
	int 	_general_timeout;	//通用的超时器，目前只有交易时使用
	float 	_fall_counter;
	int	 _wallow_level;		//沉迷等级
//	wallow_object _wallow_obj;	//沉迷时间控制对象

	int _profit_time;		//剩余收益时间(单位为秒,>0,有收益时间;=0无收益时间)
	int _profit_level;		//收益级别,由收益时间转换得到
	int _profit_timestamp;	//收益时间戳,为当天的0点时间戳
	int _active_state_delay;//针对收益地图2,战斗或移动时设置

	bool	_level_up;

	struct 
	{
		int error_counter;
	} move_checker;
	
	int DecMoveCheckerError(int offset)
	{
		if((move_checker.error_counter -= offset) < 0)
		{
			move_checker.error_counter = 0;
		}
		return 	move_checker.error_counter;
	}

	int IncMoveCheckerError(int offset)
	{
		return 	(move_checker.error_counter += offset);
	}

	void ClrMoveCheckerError()
	{
		move_checker.error_counter  = 0;
	}

	//任务相关的内容
	unsigned long _task_mask;
	abase::vector<char> _active_task_list;
	abase::vector<char> _finished_task_list;
	abase::vector<char> _finished_time_task_list;
	abase::vector<char>	_finish_task_count_list;
	abase::vector<char>	_storage_task_list;

	//声望线(角色计数器)相关数据
	abase::vector<unsigned char> _role_reputation_uchar;
	abase::vector<unsigned short> _role_reputation_ushort;
	abase::vector<unsigned int> _role_reputation_uint;

	//副本key相关的内容
	abase::static_multimap<int ,abase::pair<int,int> ,abase::default_alloc>  _cur_ins_key_list;
	abase::static_multimap<int ,abase::pair<int,int> ,abase::default_alloc>  _team_ins_key_list;
	abase::vector<abase::pair<int,int> >  _cur_tag_counter;
	abase::vector<abase::pair<int,int> >  _ins_key_timer;		//记录副本key的使用时间，避免刷副本

	//仓库结构
	player_trashbox  _trashbox;
	//帐号仓库，同帐号下的所有角色均可操作
	player_trashbox  _user_trashbox;

	//鸿利商城相关数据
	dividend_mall_info _dividend_mall_info;
	
	multi_exp_ctrl _multi_exp_ctrl;	//多倍经验控制
	faction_contrib _faction_contrib; //帮派贡献度
	abase::hash_map<int/*factionid*/,int> _faction_alliance; 	//帮派同盟
	abase::hash_map<int/*factionid*/,int> _faction_hostile;		//帮派敌对

	enum CONGREGATE_TYPE
	{
		CONGREGATE_TYPE_TEAM,
		CONGREGATE_TYPE_FACTION,
		CONGREGATE_TYPE_TEAM_MEMBER,
	};	
	struct congregate_req
	{
		char type;
		int sponsor;
		int timeout;
		int world_tag;
		A3DVECTOR pos;
	};
	abase::vector<congregate_req> _congregate_req_list;
	player_force _player_force;
	force_ticket_info _force_ticket_info;
	online_award _online_award;
	switch_additional_data * _switch_additional_data;	//在切换场景前后需要用到的数据
	meridian_manager _meridianman; //玩家的经脉系统
	player_reincarnation _player_reincarnation;			//转生
	abase::bitmap<GENERALCARD_MAX_COLLECTION> _generalcard_collection;//卡牌收集图鉴

	//玩家的下线类型，这决定了给link发送何种协议
	enum
	{
		PLAYER_OFF_LOGOUT,
		PLAYER_OFF_OFFLINE,
		PLAYER_OFF_KICKOUT,
		PLAYER_OFF_LPG_DISCONNECT,
		PLAYER_OFF_CHANGEDS,
	};

	//物品列表的定义，主要用于和客户端的联系
	enum
	{
		IL_INVENTORY,
		IL_EQUIPMENT,
		IL_TASK_INVENTORY,
		IL_TRASH_BOX,		//这个在GetInventroy中无法取得
		IL_TRASH_BOX2,		//材料仓库
		IL_TRASH_BOX3,		//时装仓库(随身)
		IL_USER_TRASH_BOX,	//帐号仓库
		IL_TRASH_BOX4,		//卡牌仓库(随身)

		IL_MAX,

		IL_INVENTORY_BEGIN = IL_INVENTORY,
		IL_INVENTORY_END = IL_TASK_INVENTORY,
		IL_TRASH_BOX_BEGIN = IL_TRASH_BOX,
		IL_TRASH_BOX_END = IL_TRASH_BOX4,
	};

	//玩家开启和关闭pvp状态的原因
	enum
	{
		PLAYER_PVP_CLIENT,		//客户端主动操作,开启或关闭pvp状态(适用于PVE服务器)
		PLAYER_PVP_PROTECTED,	//玩家进入或脱离新手保护区域
	};

	//帮派pvp活动奖励类型
	enum
	{
		AWARD_MAFIAPVP_MINECAR = 1, 	// 帮派pvp矿车奖励
		AWARD_MAFIAPVP_MINEBASE,		// 帮派pvp基地奖励
		AWARD_MAFIAPVP_MINECAR_ARRIVED,	// 矿车到达
		AWARD_MAFIAPVP_HIJACK_KILL,		// 掠夺者击杀
		AWARD_MAFIAPVP_MINECAR_PROTECT, // 矿车保护
		AWARD_MAFIAPVP_NO_OWNER_MINECAR,// 无归属矿车奖励
		AWARD_MAFIAPVP_NO_OWNER_MINEBASE,// 无归属基地奖励
	};

    enum
    {
        PRODUCE_INHERIT_REFINE = 0x0001,
        PRODUCE_INHERIT_SOCKET = 0x0002,
        PRODUCE_INHERIT_STONE = 0x0004,
        PRODUCE_INHERIT_ENGRAVE = 0x0008,
        PRODUCE_INHERIT_ADDON = 0x0010,
    };

public:
	gplayer_imp();
	~gplayer_imp();
public:
	inline gplayer * GetParent() { return (gplayer*)_parent; }
	inline int GetPlayerClass() { return ((gplayer*)_parent)->base_info.race  & 0x7FFFFFFF;}
	virtual int GetObjectClass() { return GetPlayerClass();}			//取得对象的职业
	inline bool IsPlayerFemale() { return ((gplayer*)_parent)->base_info.race < 0;}
	inline void SetPlayerClass(int cls, bool gender) 
	{ 
		gplayer * pPlayer = GetParent();
		if(gender)
		{
			pPlayer->base_info.race  = cls | 0x80000000;
			pPlayer->object_state2 |=  gactive_object::STATE_PLAYER_GENDER;
		}
		else
		{
			pPlayer->base_info.race  = cls;
		}
	}

	inline void GetPlayerClass(int & cls,bool & gender)
	{
		int race = ((gplayer*)_parent)->base_info.race;
		cls = race & 0x7FFFFFFF;
		gender = race & 0x80000000;
	}

	inline void SetPlayerName(const void  * name, size_t size)
	{
		if(size > MAX_USERNAME_LENGTH) size = MAX_USERNAME_LENGTH;
		memcpy(_username,name,size);
		_username_len = size;
	}

	inline const void  * GetPlayerName(size_t & len)
	{
		len = _username_len;
		return _username;
	}

	inline int GetDBUserId(){ return _db_user_id; }

	inline void MallSaveDone(int id)
	{
		_mall_order_id_saved = id;
		size_t i = 0;
		for(i = 0;i  < _mall_invoice.size(); i ++)
		{
			const netgame::mall_invoice & mi =  _mall_invoice[i];
			if(mi.order_id - id >= 0) break;
		}

		if(i == _mall_invoice.size())
		{
			_mall_invoice.clear();
		}
		else if( i > 0)
		{
			_mall_invoice.erase(_mall_invoice.begin(), _mall_invoice.begin() + i);
		}
	}
	
	inline void SetMallInfo(int cash, int cash_used, int delta, int cash_add, int order_id)
	{
		_mall_cash = cash;
		_mall_cash_used = cash_used;
		_mall_cash_offset = delta;
		_mall_cash_add = cash_add;
		_mall_order_id = order_id;
		_mall_order_id_saved = order_id;
	}

	inline void SetMallConsumption(int consumption)
	{
		_mall_consumption = consumption;
	}

	inline void GetMallInfo(int & cash,int & cash_used, int & delta, int & order_id)
	{
		cash = _mall_cash;
		cash_used = _mall_cash_used;
		delta = _mall_cash_offset;
		order_id = _mall_order_id;
	}

	inline int GetMallCash()
	{
		return _mall_cash + _mall_cash_offset;
	}

	inline int GetMallCashAdd()
	{
		return _mall_cash_add;	
	}

	inline int GetMallConsumption() const
	{
		return _mall_consumption;
	}

	inline bool NeedSaveMallInfo()
	{
		return _mall_order_id != _mall_order_id_saved;
	}

	inline void DeliveryNotifyCash(int cash_plus_used)
	{
		_mall_cash = cash_plus_used - _mall_cash_used;
		_runner->player_cash(GetMallCash());
	} 
	
	inline void DeliveryNotifyModifyCashUsed(int cash_used)
	{
		//由于gamedbd修改了cash_used，所以一定要先在gamedbd进行流水号加1，然后再增加gs的
		_mall_order_id++;
		_mall_cash = _mall_cash - (cash_used - _mall_cash_used);   
		_mall_cash_used = cash_used;
		_mall_cash_offset = 0;
		_runner->player_cash(GetMallCash());
	}

	inline void GetBasicData(int &level,int & sec_level,int &exp,int &sp,int &hp,int &mp, int & pp,int &money, unsigned int & db_user_id)
	{
		level = _basic.level;
		sec_level = _basic.sec_level;
		exp = _basic.exp;
		sp = _basic.skill_point;
		hp = _basic.hp;
		mp = _basic.mp;
		pp = _basic.status_point;

		money =_player_money;
		db_user_id = _db_user_id;
	}

	inline void SetBasicData(int level,int sec_level,int exp,int sp,int hp,int mp,int pp,int money,int db_user_id)
	{
		_basic.level = level;
		_basic.sec_level = sec_level;
		_basic.exp = exp;
		_basic.skill_point = sp;
		_basic.hp = hp;
		_basic.mp = mp;
		_basic.status_point = pp;

		_db_user_id = db_user_id;
		_player_money = money;
	}
	
	//覆盖原有的函数
	//目前该状态设置只在player内部使用
	inline void ActiveCombatState(bool state) 
	{
		if(state != _combat_state) 
		{
			SetRefreshState();		
			_combat_state = state;
			if(!_combat_state)
			{
				SetAttackMonster(false);
				GetParent()->object_state2 &= ~gactive_object::STATE_IN_COMBAT;	
			}
			else
				GetParent()->object_state2 |= gactive_object::STATE_IN_COMBAT;
			
			_runner->player_active_combat(_combat_state);
		}

		if(_combat_state)
		{
			//脱离战斗时收益时间相关设置
			SetActiveStateDelay(MAX_ACTIVE_STATE_DELAY);
		}
	} 
	
	inline bool IsDeliverLegal()
	{
		return _player_state == PLAYER_STATE_NORMAL || _player_state == PLAYER_SIT_DOWN;
	}

	inline int GetPlayerState()
	{
		return _player_state;
	}

	inline item_list & GetInventory(){ return _inventory;}
	inline item_list & GetEquipInventory(){ return _equipment;}
	inline item_list & GetTaskInventory(){ return _task_inventory;}
	inline player_trashbox & GetTrashBox() { return _trashbox;}
	inline player_trashbox & GetUserTrashBox() { return _user_trashbox;}
	inline item_list & GetInventory(int where)
	{
		switch(where)
		{
			case IL_INVENTORY:
			default:
				return _inventory;
			case IL_EQUIPMENT:
				return _equipment;
			case IL_TASK_INVENTORY:
				return _task_inventory;
		}
	}

	inline item_list & GetTrashInventory(int where)
	{
		switch(where)
		{
			case IL_TRASH_BOX:
			default:
				return _trashbox.GetBackpack1();
			case IL_TRASH_BOX2:
				return _trashbox.GetBackpack2();
			case IL_TRASH_BOX3:
				return _trashbox.GetBackpack3();
			case IL_TRASH_BOX4:
				return _trashbox.GetBackpack4();
			case IL_USER_TRASH_BOX:
				return _user_trashbox.GetBackpack1();
		}
	}

	inline bool IsPortableTrashBox(int where)	//是否是随身仓库
	{
		return where == IL_TRASH_BOX3 || where == IL_TRASH_BOX4;
	}
	
	// count_consumption: 是否需要计算消费值 返回值: 实际删除多少物品
	inline size_t RemoveItems(int type, size_t num, int drop_type, bool count_consumption)
	{
		size_t old_num = num;
		int rst = 0;
		while(num && (rst = _inventory.Find(rst,type)) >= 0)
		{
			size_t count = num;
			if(_inventory[rst].count < count) count = _inventory[rst].count;

			if (count_consumption)
			{
				item& it = _inventory[rst];
				UpdateMallConsumptionDestroying(it.type, it.proc_type, count);
			}
			
			_inventory.DecAmount(rst,count);
			_runner->player_drop_item(IL_INVENTORY,rst,type,count,drop_type);
			num -= count;
			rst ++;
		}
		return old_num - num;
	}

	inline bool InventoryIsFull(int item_type) { return _inventory.HasSlot(item_type);} 
	inline bool InventoryHasSlot(size_t count) { return count <= _inventory.GetEmptySlotCount();} 
	inline bool IsItemExist(size_t inv_index, int type, size_t count) { return _inventory.IsItemExist(inv_index,type,count); }
	inline bool IsItemExist(int where, size_t index, int type, size_t count)
	{
		switch(where)
		{
			case IL_INVENTORY:
				return _inventory.IsItemExist(index,type,count);
			case IL_EQUIPMENT:
				return _equipment.IsItemExist(index,type,count);
			case IL_TASK_INVENTORY:
				return _task_inventory.IsItemExist(index,type,count);
		}
		return false;
	}
	
	inline bool IsItemExist(int type)
	{
		if(type == 0 || type == -1) return false;
		return _inventory.Find(0,type) >= 0;
	}
	
	inline bool IsItemCanSell(size_t inv_index, int type, size_t count)
	{
		return _inventory.IsItemExist(inv_index,type,count) && 
			!(_inventory[inv_index].proc_type & item::ITEM_PROC_TYPE_NOSELL);
	}

	inline bool IsItemNeedRepair(int where, size_t index, int type)
	{
		ASSERT(type != -1);
		item_list & inv = GetInventory(where);
		if(!inv.IsItemExist(index,type,1)) return false;
		int d,md;
		inv[index].GetDurability(d,md);
		return (md >0 && md > d);
	}

	inline int GetItemCount(int item_id)
	{
		int start = 0;
		int count = 0;
		while((start = _inventory.Find(start, item_id)) >= 0)
		{
			count += _inventory[start].count;
			start ++;
		}
		return count;
	}

	inline int GetItemPos(int item_id)
	{
		return  _inventory.Find(0,item_id);
	}

	inline int GetPKCount()
	{
		return _invade_ctrl._kill_count;
	}

	inline int GetPVPCoolDown()
	{
		return _pvp_cooldown;
	}

	inline bool GetPVPFlag()
	{
		return _pvp_enable_flag;
	}

	inline char GetDeadFlag()
	{
		if(!_parent->IsZombie())  return false;
		return _kill_by_player?player_var_data::KILLED_BY_PLAYER:player_var_data::KILLED_BY_NPC;
	}

	inline void SetDeadFlag(char d)
	{
		if(d)
		{
			_parent->b_zombie = true;
			if(d == player_var_data::KILLED_BY_PLAYER) 
				_kill_by_player = true;
			else 
				_kill_by_player = false;
		}
	}

	inline bool GetResurrectState(float & exp_reduce, float & hp_factor, float & mp_factor)
	{
		if(!_parent->IsZombie()) return false;
		exp_reduce = _resurrect_exp_reduce;
		hp_factor = _resurrect_hp_factor;
		mp_factor = _resurrect_mp_factor;
		return _resurrect_state;
	}

	inline void SetResurrectState(bool state, float  exp_reduce, float hp_factor, float mp_factor)
	{
		if(!_parent->IsZombie()) return;
		if(exp_reduce <0.f) exp_reduce = 0.f;
		if(exp_reduce >1.0f) exp_reduce = 1.0f;
		if(hp_factor <0.f) hp_factor = 0.f;
		if(hp_factor >1.0f) hp_factor = 1.0f;
		if(mp_factor <0.f) mp_factor = 0.f;
		if(mp_factor >1.0f) mp_factor = 1.0f;
		_resurrect_state = state;
		_resurrect_exp_reduce = exp_reduce;
		_resurrect_hp_factor = hp_factor;
		_resurrect_mp_factor = mp_factor;
	}

	inline void SetPVPState(int pkcount, int pvp_cooldown, bool pvp_flag)
	{
		_invade_ctrl._kill_count = pkcount;
		_pvp_cooldown = pvp_cooldown;
		//如果是红粉名，则强制开启PK开关
		if(_invader_state != INVADER_LVL_0) pvp_flag = true;
		_pvp_enable_flag = pvp_flag;
		if(pvp_flag)
		{
			GetParent()->object_state |= gactive_object::STATE_PVPMODE;
		}
	}

	inline void GetInvadeState(int &invader_state, int &invader_time, int &pariah_time)
	{
		ASSERT(_invader_state == _invade_ctrl._invader_state);
		invader_state = _invade_ctrl._invader_state;
		invader_time = _invade_ctrl._invader_time;
		pariah_time  = _invade_ctrl._pariah_time;
	}

	inline void SetInvadeState(int invader_state, int invader_time, int pariah_time) 
	{
		_invader_state = invader_state & 0x03;
		_invade_ctrl.SetState(_invader_state,invader_time,pariah_time);
		_invade_ctrl.SetParentState<0>(_invader_state);
	}

	inline size_t GetMoney()
	{
		return _player_money;
	}


/*
	inline void IncMoney(size_t inc)
	{
		size_t newmoney = _money + inc;
		ASSERT((int)_money >= 0);
		if((int)newmoney < (int)_money || newmoney > _money_capacity)
		{
			//这里溢出了
			newmoney = (size_t)-1;
			size_t delta = _money_capacity - _money;
			inc -= delta;
			_money = _money_capacity; 
			DropMoneyItem(_plane,_parent->pos,inc,_parent->ID,0,0);
		}
		else
		{
			_money = newmoney;
		}
	}

	inline void DecMoney(size_t offset)
	{
		ASSERT(offset <= _money);
		_money -= offset;
	}
*/
	inline bool CanUseService()
	{
		//距离判断改为只在npc进行
		return _provider.id.id != -1 
			&& (!_provider.id_mafia || _provider.id_mafia == GetParent()->id_mafia);
	}


	inline void ClearTrahsBoxWriteFlag()
	{
		_tb_change_counter = 0;
	}

	inline bool IsTrashBoxChanged()
	{
		return _tb_change_counter;
	}

	inline void IncTrashBoxChangeCounter()
	{
		_tb_change_counter ++;
		if(_tb_change_counter == 0) _tb_change_counter = 1;
	}

	inline void ClearUserTrahsBoxWriteFlag()
	{
		_user_tb_change_counter = 0;
	}

	inline bool IsUserTrashBoxChanged()
	{
		return _user_tb_change_counter;
	}

	inline void IncUserTrashBoxChangeCounter()
	{
		_user_tb_change_counter ++;
		if(_user_tb_change_counter == 0) _user_tb_change_counter = 1;
	}

	inline void IncEquipChangeFlag()
	{
		_eq_change_counter ++;
		if(_eq_change_counter == 0)
		{
			_eq_change_counter = 1;
		}
	}


	inline bool IsBled()
	{
		//以后要加入特殊状态的检验
		return _basic.hp < _cur_prop.max_hp ||_basic.mp < _cur_prop.max_mp;
	}

	inline void Renew()
	{
		_basic.hp = _cur_prop.max_hp;
		_basic.mp = _cur_prop.max_mp;
		SetRefreshState();
		_filters.ClearSpecFilter(filter::FILTER_MASK_DEBUFF);
		_runner->renew();
	}

	inline void EnterStayInState()
	{
		ASSERT(_player_state == PLAYER_STATE_NORMAL);
		_player_state = PLAYER_SIT_DOWN;
		//修正player的血量回复状态

		ObjectSitDown();
		//要发送坐下的消息给所有人
		_runner->sit_down();
	}

	inline void LeaveStayInState()
	{
		ASSERT(_player_state == PLAYER_SIT_DOWN);
		_player_state = PLAYER_STATE_NORMAL;
		//修正player的血量回复状态

		ObjectStandUp();
		_runner->stand_up();

		//清除坐下的filter
		_filters.RemoveFilter(FILTER_INDEX_SITDOWN);
	}
	
	inline player_team & GetTeamCtrl() { return _team;}
	inline bool IsTeamLeader() { return _team.IsLeader();}
	inline bool IsMember(const XID & member) { return _team.IsMember(member);}
	inline bool IsInTeam() { return _team.IsInTeam();}
	inline bool IsAutoComposingTeam() { return _team.IsAutoComposingTeam(); }
	inline const XID & GetTeamLeader() { return _team.GetLeader();}
	inline int GetTeamMemberNum() { return _team.GetMemberNum(); }
	inline int GetTeamID()
	{
		if(IsInTeam()) 
			return _team.GetLeader().id;
		else
			return 0;
	}
	inline int GetTeamSeq() 
	{ 
		if(IsInTeam()) 
			return _team.GetTeamSeq();
		else
			return -1;
	}
	inline const player_team::member_entry & GetTeamMember(int index) {return _team.GetMember(index);}
	inline void TeamChangeLeader(int id) { return _team.CliChangeLeader(XID(GM_TYPE_PLAYER,id));}
	inline int GetMemberList(XID * list) { return _team.GetMemberList(list);}
	inline void SendTeamData(const XID & leader,size_t team_count,size_t data_count, 
			const player_team::member_entry ** list)
	{
		((gplayer_dispatcher*)_runner)->send_team_data(leader,team_count,data_count,list);
	}

	inline void SendTeamData(const XID & leader,size_t team_count,
			const player_team::member_entry * list)
	{
		((gplayer_dispatcher*)_runner)->send_team_data(leader,team_count,list);
	}
	
	inline void ReceiveTaskExp(int exp,int sp) 		//任务加经验操作
	{
		if(exp < 0) exp = 0;
		if(sp  < 0) sp = 0;

		float double_exp_sp_factor = 1.0f;
		//转生后经验调整
		double_exp_sp_factor += _player_reincarnation.GetExpBonus();

		IncExp(exp,sp,double_exp_sp_factor,false);
		_runner->task_deliver_exp(exp,sp);
		//小精灵获得经验
		if(exp/10 > 0)
			ElfReceiveExp((unsigned int)(exp/10));
	}

	inline void ReceiveCommonExp(int exp, int sp)
	{
		if(exp < 0) exp = 0;
		if(sp  < 0) sp = 0;
		IncExp(exp,sp);
		_runner->receive_exp(exp,sp);
	}	

	inline void SetConcurrentEmote(int target, unsigned char emote)
	{
		_con_emote_target = target;
		_con_emote_id = emote;
	}

	inline void ClearConcurrentEmote()
	{
		_con_emote_target = 0;
		_con_emote_id = 0;
	}

	inline void SetReputation(int rep)
	{
		_reputation = rep;
	}

	inline int GetReputation()
	{
		return _reputation;
	}

	inline void SetWaypointList(const void * buf, size_t size)
	{
		if((size & 0x01) || size > MAX_WAYPOINT_COUNT*sizeof(unsigned short)) 
		{
			ASSERT(false);
			return ;
		}
		size_t count = size / 2;
		if(!count) return;
		_waypoint_list.reserve(count);
		unsigned short * list = (unsigned short*)buf;
		for(size_t i = 0; i < count; i ++)
		{
			_waypoint_list.push_back(list[i]);
		}
	}

	inline bool IsWaypointActived(unsigned short wp )
	{
		for(size_t i = 0; i < _waypoint_list.size(); i ++)
		{
			if(wp == _waypoint_list[i]) return true;
		}
		return false;
	}

	inline void ActivateWaypoint(unsigned short wp)
	{
		for(size_t i = 0; i < _waypoint_list.size(); i ++)
		{
			if(wp == _waypoint_list[i]) return ;
		}
		_waypoint_list.push_back(wp);
		_runner->activate_waypoint(wp);
		return ;
	}
	inline const void * GetWaypointBuffer(size_t & size)
	{
		size = _waypoint_list.size() * sizeof(unsigned short);
		return _waypoint_list.begin();
	}

	inline void ClearExpiredCoolDown()
	{
		_cooldown.ClearExpiredCoolDown();
	}

	//只取得自己的副本KEY
	inline void GetSelfInstanceKey(int world_tag, instance_key & key)
	{
		key.essence.key_level1 = _parent->ID.id;
		key.essence.key_level3 = ((gplayer*)_parent)->id_mafia;
		key.essence.key_level4 = 0;

		abase::static_multimap<int ,abase::pair<int,int> ,abase::default_alloc>::iterator it;
		it = _cur_ins_key_list.find(world_tag);
		if(it == _cur_ins_key_list.end())
		{
			//只有大世界和公共副本才会这样  所以都进一个副本
			key.essence.key_level2.first = key.essence.key_level2.second = 0;
		}
		else
		{
			key.essence.key_level2.first = it->second.first;
			key.essence.key_level2.second = it->second.second;
		}
	}

	inline void GetInstanceKey(int world_tag, instance_key & key)
	{
		key.essence.key_level1 = _parent->ID.id;
		key.essence.key_level3 = ((gplayer*)_parent)->id_mafia;
		key.essence.key_level5 = GetCountryId();

		abase::static_multimap<int ,abase::pair<int,int> ,abase::default_alloc>::iterator it;
		abase::static_multimap<int ,abase::pair<int,int> ,abase::default_alloc> * pMap;
		if(_team.IsInTeam())
		{
			pMap = &_team_ins_key_list; 
			it = _team_ins_key_list.find(world_tag);
		}
		else
		{
			pMap = &_cur_ins_key_list; 
			it = _cur_ins_key_list.find(world_tag);
		}

		if(it == pMap->end())
		{
			//只有大世界和公共副本才会这样  所以都进一个副本
			key.essence.key_level2.first = key.essence.key_level2.second = 0;
		}
		else
		{
			key.essence.key_level2.first = it->second.first;
			key.essence.key_level2.second = it->second.second;
		}
	}

	inline void SetLastInstancePos(int tag, const A3DVECTOR &pos, int timestamp)
	{
		_last_instance_pos = pos;
		_last_instance_tag = tag;
		_last_instance_timestamp = timestamp;
	}

	inline void SetLastInstanceSourcePos(int tag, const A3DVECTOR &pos)
	{
		_last_source_instance_pos = pos;
		_last_source_instance_tag = tag;
	}

	inline void GetLastInstancePos(int &tag, A3DVECTOR &pos, int & timestamp)
	{
		pos = _last_instance_pos;
		tag = _last_instance_tag;
		timestamp = _last_instance_timestamp;
	}

	inline void GetLastInstanceSourcePos(int &tag, A3DVECTOR &pos)
	{
		pos = _last_source_instance_pos;
		tag = _last_source_instance_tag;
	}

	inline void SetPVPCombatState()
	{
		if(!_pvp_combat_timer)
		{
			_runner->active_pvp_combat_state(true);
			GetParent()->object_state |= gactive_object::STATE_IN_PVP_COMBAT;
		}
		_pvp_combat_timer = PVP_COMBAT_HIGH_TH;
	}

	inline void PVPCombatHeartbeat()
	{
		if(!_pvp_combat_timer) return;
		if((--_pvp_combat_timer) <= 0)
		{
			_runner->active_pvp_combat_state(false);
			GetParent()->object_state &= ~gactive_object::STATE_IN_PVP_COMBAT;
			_pvp_combat_timer = 0;
		}
	}

	inline bool IsInPVPCombatStateHigh()
	{
		return _pvp_combat_timer > PVP_COMBAT_LOW_TH;
	}

	inline bool IsInPVPCombatState()
	{
		return _pvp_combat_timer;
	}

	inline int GetPVPCombatTimer()
	{
		return _pvp_combat_timer;
	}

	inline void CalcRestTime()
	{
		world_manager::CalcRestTime(_rest_counter_time,_rest_time_used,_rest_time_capacity,
				GetParent()->id_mafia, _mafia_rest_time, _mafia_rest_counter_time);
	}

	inline bool TestRestTime(int t)
	{
		return false;
		//免费版没有双倍经验服务，都使用物品完成
		//return _rest_time_used + t <= _rest_time_capacity + _mafia_rest_time;
		
	}

	inline void SetRestTimeParam(int dbl_timeout,int  dbl_mode,int  rest_counter_time,int rest_time_used,int rest_time_cap)
	{
		_double_exp_timeout	= dbl_timeout;
		_double_exp_mode	= dbl_mode;
		_rest_counter_time	= rest_counter_time;
		_rest_time_used		= rest_time_used;
		_rest_time_capacity	= rest_time_cap;
		if(_double_exp_mode)
		{
			int t = g_timer.get_systime();
			if(t >= _double_exp_timeout) 
			{
				_double_exp_mode = 0;
			}
		}
		
	}
	
	void SetExtRestParam(const void * buf, size_t size);
	void GetExtRestParam(archive & ar);
	
	inline int GetDoubleExpTime()
	{
		if(_double_exp_mode)
		{
			return _double_exp_timeout - g_timer.get_systime();
		}
		else
		{
			return 0;
		}
	}

	inline int GetDoubleExpAvailTime()
	{
		return _mafia_rest_time + _rest_time_capacity - _rest_time_used;
	}

	inline void GetRestTimeParam(int &dbl_timeout,int  &dbl_mode,int  &rest_counter_time,int &rest_time_used,int &rest_time_cap)
	{
		dbl_timeout 		=_double_exp_timeout;
		dbl_mode 		=_double_exp_mode;
		rest_counter_time 	=_rest_counter_time;
		rest_time_used 		=_rest_time_used;
		rest_time_cap 		=_rest_time_capacity;
	}

	inline void SetPlayEd(int timestamp, int playtime)
	{
		_login_timestamp = timestamp;
		_played_time = playtime;
	}

	inline void SetWallowData(const void * buf, size_t len)
	{
	/*	time_t t = g_timer.get_systime();
		_wallow_obj.Init(t, buf, len);
		if(world_manager::AntiWallow())
		{
			_wallow_level = _wallow_obj.Tick(t);
		}
		else
		{
			_wallow_level = 0;
		}*/
	}
	
	inline int GetPlayEd()
	{
		return _played_time + (g_timer.get_systime() - _login_timestamp);
	}

	inline void SetLastLoginTime(int lastlogin)
	{
		_last_login_timestamp = lastlogin;	
	}

	inline void SetCreateTime(int createtime)
	{
		_create_timestamp = createtime;	
	}
	
	inline int GetLastLoginTime()
	{
		return _last_login_timestamp;	
	}

	inline int GetCreateTime()
	{
		return _create_timestamp;	
	}

	inline void GetSpecailTaskAward(unsigned long & id,unsigned long & id2,unsigned long &param , unsigned long& mask)
	{
		id = _spec_task_reward;
		id2 = _spec_task_reward2;
		param = _spec_task_reward_param;
		mask = _spec_task_reward_mask;
	}
	
	inline void SetSpecailTaskAward(int id,int id2,int param, int rewardmask)
	{
		_spec_task_reward = id;
		_spec_task_reward2 = id2;
		_spec_task_reward_param = param;
		_spec_task_reward_mask = rewardmask;
	}
	inline bool IsUnderWater(float offset = 0.0f)
	{
		return _breath.IsUnderWater(offset);
	}
	
	int TransformChatData(const void * data,size_t dsize, void * out_buffer, size_t len);
	int PhaseControl(const A3DVECTOR & target, float terrain_height, int mode, int use_time);
	
	inline void SetSecLevel(unsigned long per)
	{
		_basic.sec_level = per & 0xFFFF;
		SetRefreshState();
		GetParent()->sec_level = _basic.sec_level;
		_runner->task_deliver_level2(_basic.sec_level);
	}

	inline void SetDBTimeStamp(int db_timestamp)
	{
		_db_timestamp = db_timestamp;
	}

	inline void SetCombatTimer(int new_timer)
	{
		if(_combat_timer < new_timer)
		{
			_combat_timer = new_timer;
		}
	}
	
	inline void GetDBDailySignin(int& uptime, int& monthcal, int& curryear, int& lastyear, char& awardedtimes, char& latesignintimes)
	{
		_player_dailysign.SaveToDB(uptime, monthcal, curryear, lastyear, awardedtimes, latesignintimes);
	}
	
	inline void SetDBDailySignin(int uptime, int monthcal, int curryear, int lastyear, char awardedtimes, char latesignintimes)
	{
		_player_dailysign.InitFromDB(uptime, monthcal, curryear, lastyear, awardedtimes, latesignintimes);
	}
	
	inline void GetDBTouchTrade(int64_t &sn,char &state,unsigned int& cost,int &type,unsigned int &count,int& expiretime,unsigned int &lots )
	{
		_touch_order.GetData(sn,state,cost,type,count,expiretime,lots);
	}

	inline void SetDBTouchTrade(int64_t sn,char state,unsigned int cost,int type,unsigned int count,int expiretime,unsigned int lots )
	{
		_touch_order.InitData(sn,state,cost,type,count,expiretime,lots,_parent->ID.id);
	}
	
	inline void GetDBGiftCard(char& state,int& type,int& parenttype,char (&cardnumber)[player_giftcard::GIFT_CARDNUMBER_LEN] )
	{
		_player_giftcard.GetData(state,type,parenttype,cardnumber);
	}

	inline void SetDBGiftCard(char state,int type,int parenttype,const char (&cardnumber)[player_giftcard::GIFT_CARDNUMBER_LEN] )
	{
		_player_giftcard.InitData(state,type,parenttype,cardnumber);
	}

	inline void GetDBClockData(archive & ar)
	{
		_player_clock.SaveToDB(ar);
	}

	inline void SetDBClockData(archive & ar)
	{
		_player_clock.InitFromDB(ar,_parent->ID.id);
	}
	
	inline void GetDBRandMallData(archive & ar)
	{
		_player_randmall.SaveToDB(ar);
	}

	inline void SetDBRandMallData(archive & ar)
	{
		_player_randmall.InitFromDB(ar,_parent->ID.id);
	}
	
	inline void GetDBTitleData(archive & ar)
	{
		_player_title.SaveToDB(ar);
	}

	inline void InitDBTitleData(archive & ar)
	{
		_player_title.InitFromDB(ar);
	}

	inline void InitDBTitleEnhance()
	{
		_player_title.CalcAllTitleEnhance(false);
	}

	inline void SetDBRealmData(int level,int exp)
	{
		_realm_level = level;
		_realm_exp = exp;
		
		SetVigourBase(player_template::GetRealmVigour(_realm_level));
		
		if(_realm_level)
		{
			GetParent()->realmlevel = _realm_level;	
			GetParent()->object_state2 |= gactive_object::STATE_REALMLEVEL;
		}
	}

	inline void GetDBRealmData(int& level,int& exp)
	{
		level = _realm_level;
		exp = _realm_exp;
	}

	inline void InitInstanceReenter(int tag,int type,instance_hash_key key,int timeout,A3DVECTOR pos)
	{
		_player_instance_reenter.LoadFromDB(tag,type,key,timeout,pos);
	}

	inline player_title& GetPlayerTitle() { return _player_title;}
	inline player_clock& GetPlayerClock() { return _player_clock;}

	inline void GetDBFateRingData(archive & ar)
	{
		_player_fatering.SaveToDB(ar);
	}

	inline void InitDBFateRingData(archive & ar)
	{
		_player_fatering.InitFromDB(ar);
	}

	inline void InitDBFateRingEnhance()
	{
		_player_fatering.EnhanceAll();
	}

	inline bool IsMarried()
	{
		return GetParent()->spouse_id != 0;
	}

	inline int GetSpouse()
	{
		return GetParent()->spouse_id;
	}

	inline void SetSpouse(int id)
	{
		//这个操作不负责发送广播
		gplayer * pPlayer = GetParent();
		if(id == 0)
		{
			pPlayer->object_state &= ~gactive_object::STATE_SPOUSE;
			pPlayer->spouse_id = 0;
		}
		else
		{
			pPlayer->spouse_id = id;
			pPlayer->object_state |= gactive_object::STATE_SPOUSE;
		}
	}

	inline void ExternSaveDB()
	{
		if(_player_state == PLAYER_STATE_NORMAL)
		{
			AutoSaveData();
		}
		else
		{
			//能这样村的外面必须检查
			ASSERT(false);
		}
	}

	inline void EnablePVPFlag(char type)
	{
		if(!_pvp_enable_flag)
		{
			GetParent()->object_state |= gactive_object::STATE_PVPMODE;
			_pvp_enable_flag = true;
			_pvp_cooldown = PVP_STATE_COOLDOWN;
			_runner->enable_pvp_state(type);
			_runner->player_pvp_cooldown(_pvp_cooldown);
		}
	}

	inline void DisablePVPFlag(char type)
	{
		if(_pvp_enable_flag)
		{
			GetParent()->object_state &= ~gactive_object::STATE_PVPMODE;
			_pvp_enable_flag = false;
			_pvp_cooldown = 0;
			_runner->disable_pvp_state(type);
			_runner->player_pvp_cooldown(_pvp_cooldown);
		}
	}
	
	inline void SetSecurityPasswdChecked(bool b)
	{
		_security_passwd_checked = b;
		if(b) 
			_runner->security_passwd_checked();
	}

	inline float GetCurSpeed()
	{
		float speed[]={_cur_prop.run_speed,_cur_prop.flight_speed,_cur_prop.swim_speed,_cur_prop.run_speed}; 
		int index = _layer_ctrl.GetLayer(); 
		if(index == 2 && !IsUnderWater(2.0f)) return speed[0]; //在深水中才是游泳
		return speed[index];	
	}
	
	void UpdateEquipRefineLevel()
	{
		memset(_equip_refine_level, 0, sizeof(_equip_refine_level));
		for(size_t i=0; i<item::EQUIP_INVENTORY_COUNT; i++)
		{
			item& it = _equipment[i];
			if(it.type <= 0 || it.body == NULL || !it.body->IsActive()) continue;
			int material_need;
			int refine_addon = world_manager::GetDataMan().get_item_refine_addon(it.type, material_need);
			if(material_need <= 0 || refine_addon <= 0) continue;
			_equip_refine_level[i] = it.body->GetRefineLevel(refine_addon);			
			ASSERT(_equip_refine_level[i] >= 0 && _equip_refine_level[i] <= 12);
		}
		UpdateBaseSoulPower();
	}
	
	void UpdateBaseSoulPower()
	{
		//计算魂力
		const int refine_cost[13] = {0,1,5,15,40,100,230,445,850,1600,2970,5495,10140};
		_soul_power = _basic.level*_basic.level + 50*_basic.level;
		_soul_power += int(refine_cost[_equip_refine_level[0]] * 1.5f);
		for(size_t i=1; i<item::EQUIP_INVENTORY_COUNT; i++)
		{ 
			_soul_power += int(refine_cost[_equip_refine_level[i]]*0.25f);	
		}
		__PRINTF("魂力:%d\n",_soul_power + _soul_power_en);
	}

	bool CheckEquipRefineLevel(int count ,int level)
	{
		for(size_t i=0; i<item::EQUIP_INVENTORY_COUNT; i++)
		{
			if(_equip_refine_level[i] >= level)
			{
				if(--count <= 0) return true;
			}
		}
	
		return false;
	}

	inline int GetFactionConsumeContrib()
	{
		return _faction_contrib.consume_contrib;
	}

	inline int GetFactionExpContrib()
	{
		return _faction_contrib.exp_contrib;
	}

	inline int GetFactionCumulateContrib()
	{
		return _faction_contrib.cumulate_contrib;
	}

	inline bool IncFactionContrib(int consume, int exp)
	{
		int tmp1 = _faction_contrib.consume_contrib + consume;
		int tmp2 = _faction_contrib.exp_contrib + exp;
		int tmp3 = _faction_contrib.cumulate_contrib + consume;
		if(tmp1 < _faction_contrib.consume_contrib || tmp2 < _faction_contrib.exp_contrib || tmp3 < _faction_contrib.cumulate_contrib)
			return false;
		_faction_contrib.consume_contrib = tmp1;
		_faction_contrib.exp_contrib = tmp2;
		_faction_contrib.cumulate_contrib = tmp3;
		_runner->faction_contrib_notify();
		return true;
	}
	
	inline void DecFactionContrib(int consume, int exp)
	{
		_faction_contrib.consume_contrib -= consume;
		if(_faction_contrib.consume_contrib < 0) _faction_contrib.consume_contrib = 0;
		_faction_contrib.exp_contrib -= exp;
		if(_faction_contrib.exp_contrib < 0) _faction_contrib.exp_contrib = 0;
		_runner->faction_contrib_notify();
	}

	inline int GetDisabledEquipMask(){ return ((gplayer*)_parent)->disabled_equip_mask;}

	//预留接口,通过道具取消收益时间限制
	inline void SetProfitTime(int time) {_profit_time = time;}
	inline void SetActiveStateDelay(int delay)
	{
		if(world_manager::ProfitTimeLimit2() && _active_state_delay == 0)
		{
			_runner->notify_profit_state(1);
		}
		_active_state_delay = delay;
	}
	inline void UpdateProfitTime()
	{
		if(_profit_time > 0)
		{
			//处于收益地图
			_profit_time--;
			int level = player_template::GetProfitLevel(_profit_time);
			if(level != _profit_level)
			{
			 	_profit_level = level;
				_runner->update_profit_time(S2C::CMD::player_profit_time::PROFIT_LEVEL_CHANGE, _profit_time, _profit_level);
			}
		}
	}
	inline void CalcProfitLevel()
	{
		if(!world_manager::GetInstance()->ProfitMap())
		{
			_profit_level = PROFIT_LEVEL_NORMAL;
			return;
		}

		int level = player_template::GetProfitLevel(_profit_time);
		if(level != _profit_level)
		{
			_profit_level = level;
		}
	}
	inline void SaveProfitTime(archive & ar)
	{
		ar << _profit_time << _profit_timestamp;
	}
	inline void LoadProfitTime(archive & ar)
	{
		time_t now = g_timer.get_systime();
		struct tm *tm_now = localtime(&now);
		ASSERT(tm_now);

		if(ar.size() == 2*sizeof(int))
		{
			ar >> _profit_time >> _profit_timestamp;
			if(now - _profit_timestamp >= TOTAL_SEC_PER_DAY)
			{
				//收益时间过期,重置收益时间
				_profit_time = world_manager::GetWorldConfig().profit_time;
				_profit_timestamp = now - tm_now->tm_hour*3600 - tm_now->tm_min*60 - tm_now->tm_sec;
			}
		}
		else
		{
			//DB无收益时间数据,首次设置
			_profit_time = world_manager::GetWorldConfig().profit_time;
			_profit_timestamp = now - tm_now->tm_hour*3600 - tm_now->tm_min*60 - tm_now->tm_sec;
		}
	}

	inline int GetCountryId(){	return ((gplayer*)_parent)->country_id; }
	inline int GetCountryExpireTime(){ return _country_expire_time; }
	void SetCountryId(int country_id, int expire_time)
	{
		gplayer * pPlayer = (gplayer*)_parent;
		pPlayer->country_id = country_id;
		if(pPlayer->country_id)
		{
			pPlayer->object_state |= gactive_object::STATE_COUNTRY;
			_country_expire_time = expire_time;
		}
		else
		{
			pPlayer->object_state &= ~gactive_object::STATE_COUNTRY;
			_country_expire_time = 0;
		}	
		_runner->player_country_changed(pPlayer->country_id);
	}
	void SetCountryData(int country_id, int expire_time)
	{
		gplayer * pPlayer = (gplayer*)_parent;
		if(country_id && !(expire_time && g_timer.get_systime() >= expire_time))
		{
			pPlayer->country_id = country_id;
			pPlayer->object_state |= gactive_object::STATE_COUNTRY;
			_country_expire_time = expire_time;
		}
	}
	inline bool InCentralServer(){ return _in_central_server; }
	inline void SetInCentralServer(bool b, int src_zoneid)
	{ 
		if(b)
		{
			_in_central_server = true;
			_src_zoneid = src_zoneid;
		}
	}
	inline int GetKingExpireTime(){ return _king_expire_time; }
	void SetKing(bool is_king, int expire_time)
	{
		gplayer * pPlayer = (gplayer*)_parent;
		if(is_king)
		{
			pPlayer->object_state2 |= gactive_object::STATE_KING;
			_king_expire_time = expire_time;
		}
		else
		{
			pPlayer->object_state2 &= ~gactive_object::STATE_KING;
			_king_expire_time = 0;
		}
		_runner->player_king_changed(is_king ? 1 : 0);
	}
	void SetKingData(int is_king, int expire_time)
	{
		gplayer * pPlayer = (gplayer*)_parent;
		if(is_king && !(expire_time && g_timer.get_systime() >= expire_time))
		{
			pPlayer->object_state2 |= gactive_object::STATE_KING;
			_king_expire_time = expire_time;
		}
	}
	int Get16Por9JWeapon()		//0-普通 1-16品 2-9军
	{
		int rst = _equipment[item::EQUIP_INDEX_WEAPON].Is16Por9JWeapon();
		if(rst == 2) return rst;
		for(size_t i=0; i<_inventory.Size(); i++)
		{
			int tmp = _inventory[i].Is16Por9JWeapon();
			if(tmp > rst)
			{
				rst = tmp;
				if(rst == 2) return rst;	
			}
		}
		return rst;	
	}
	void ClearSwitchAdditionalData()
	{
		if(_switch_additional_data)
		{
			delete _switch_additional_data;
			_switch_additional_data = NULL;
		}
	}
	void UpdateReincarnation(size_t times, bool notify_client)
	{
		gplayer * pPlayer = GetParent();
		if(times)
		{
			pPlayer->object_state2 |= gactive_object::STATE_REINCARNATION;	
			pPlayer->reincarnation_times = times & 0xFF;
		}
		else
		{
			pPlayer->object_state2 &= ~gactive_object::STATE_REINCARNATION;
			pPlayer->reincarnation_times = 0;
		}
		if(notify_client) _runner->player_reincarnation(times);	
	}
	inline size_t GetReincarnationTimes(){ return _player_reincarnation.GetTimes(); }

	inline int GetRealmLevel() const { return _realm_level;}
	inline void SetDBLeadership(int v){ _leadership = v; }
	inline int GetDBLeadership(){ return _leadership; }
	inline void IncLeadership(int v)
	{
		_leadership += v;
		_runner->player_leadership(_leadership, v);
	}
	inline void SetDBWorldContrib(int wc,int wcc) { _world_contribution = wc; _world_contribution_cost = wcc;}
	inline void GetDBWorldContrib(int& wc,int& wcc) { wc = _world_contribution; wcc = _world_contribution_cost; }
	inline void IncWorldContrib(int wc)
	{
		_world_contribution += wc;
		if(wc < 0)	_world_contribution_cost -= wc;
		_runner->player_world_contribution(_world_contribution, wc, _world_contribution_cost);
	}
	inline void ClearWorldContrib()
	{
		_world_contribution = 0;
		_world_contribution_cost = 0;
		_runner->player_world_contribution(0,0,0);
	}

protected:
	template <typename MESSAGE,typename ENCHANT_MSG>
	inline bool TestHelpfulEnchant(const MESSAGE & msg, ENCHANT_MSG & emsg)
	{
		//蓝名有益法术无法对PK状态打开的玩家使用 如果攻击者是蓝名的话
		//红名和粉名都可以接受蓝名的有益法术
		XID attacker = emsg.ainfo.attacker;
		if(attacker == _parent->ID) return true;
		if(attacker.IsPlayerClass())
		{
			if(!(emsg.attacker_mode & attack_msg::PVP_ENABLE)
					&& (IsInPVPCombatState() ) )
			{
				return false;
			}
		}
		//玩家可以选择是否接受他人祝福
		if(emsg.helpful == 1)
		{
			if((_refuse_bless & C2S::REFUSE_NON_TEAMMATE_BLESS) 
					&& (!IsInTeam() || !IsMember(attacker))) return false;
		}
		else if(emsg.helpful == 2)
		{
			if(_refuse_bless & C2S::REFUSE_NEUTRAL_BLESS) return false;
		}
			
		emsg.is_invader = IsInPVPCombatState() && (_invader_state != INVADER_LVL_0) && (emsg.attacker_mode & attack_msg::PVP_ENABLE);

		//如果对方并非PVP状态，并且自己处于PVP状态，则发一个消息让对方成为PVP状态
		if(!(emsg.attacker_mode & attack_msg::PVP_DURATION) && IsInPVPCombatState())
		{
			SendTo<0>(GM_MSG_ENABLE_PVP_DURATION,emsg.ainfo.attacker,0);
		}
		return true;
	}

	template <typename MESSAGE,typename ENCHANT_MSG>
	inline bool TestHarmfulEnchant(const MESSAGE & msg, ENCHANT_MSG & emsg)
	{
		//有害法术的攻击判定和攻击判定完全一样 
		//由于变量名也一致，所以可以直接调用攻击判定的函数
		return TestAttackMsg(msg, emsg);
	}

	template <typename MESSAGE,typename ATTACK_MSG>
	inline bool TestAttackMsg(const MESSAGE & msg, ATTACK_MSG & amsg)
	{
		//攻击的攻击判定
		//不能攻击自己和队友
		bool IsInvader = false;
		if(amsg.attacker_mode & attack_msg::PVP_DUEL)
		{
			amsg.target_faction = 0xFFFFFFFF;
		}
		else
		{
			XID attacker = amsg.ainfo.attacker;
			if( attacker.IsPlayerClass())
			{
				//如果自己未开PK开关不会被玩家攻击
				if(!_pvp_enable_flag) return false;

				//如果不是强制攻击则不允许攻击
				if(!amsg.force_attack) return false;

				//队友和自己不会被攻击
				if(attacker.id == _parent->ID.id || (IsInTeam() && IsMember(attacker))) return false;

				//如果对方未开PK开关也不会被攻击
				if(!(amsg.attacker_mode & attack_msg::PVP_ENABLE))
				{
					return false;
				}

				//如果受攻击者是白名并且攻击消息是保护白名那么不会被攻击到
				if(_invader_state == INVADER_LVL_0 && (amsg.force_attack & C2S::FORCE_ATTACK_NO_WHITE))
				{
					return false;
				}

				//如果开启了帮派保护，并且攻击者和被攻击者的帮派相同，则不会被攻击到
				if(amsg.ainfo.mafia_id)
				{
					if(amsg.force_attack & C2S::FORCE_ATTACK_NO_MAFIA 
							&& amsg.ainfo.mafia_id == ((gplayer*)_parent)->id_mafia) return false;
					if(amsg.force_attack & C2S::FORCE_ATTACK_NO_MAFIA_ALLIANCE 
							&& OI_IsFactionAlliance(amsg.ainfo.mafia_id)) return false;
				}

				//如果开启了势力保护，则同势力玩家不会被攻击
				if(amsg.ainfo.force_id && amsg.ainfo.force_id == _player_force.GetForce()) return false;

				//如果对方并非PVP状态，则回传消息让玩家处于PVP
				if(!(amsg.attacker_mode & attack_msg::PVP_DURATION))
				{
					SendTo<0>(GM_MSG_ENABLE_PVP_DURATION,attacker,0);
				}

				//如果被攻击者不是红名则定为非法攻击
				//IsInvader = (_invader_state != INVADER_LVL_2);
				//现在改成一律非法攻击，白名攻击红名也是非法攻击
				IsInvader = true;
			}
		}
		amsg.is_invader = IsInvader;
		return true;
	}

	inline int AutoGenStat(int cooldown_idx, int idx ,int offset, bool bSave)
	{
		//应该自动使用药物了 要先检查冷却
		if(CheckCoolDown(cooldown_idx))
		{
			//开始使用回复物品
			int rst = _equipment[idx].AutoTrigger(this,cooldown_idx,offset);
			if(rst < 0) return 0;
			if(rst == 0)
			{
				//表示这个物品应该删除了
				if(bSave)
				{
					int type = _equipment[idx].type;
					int proc_type = _equipment[idx].proc_type;

					UpdateMallConsumptionDestroying(type, proc_type, 1);

					_equipment.DecAmount(idx,1);
					_runner->player_drop_item(IL_EQUIPMENT,idx,type,1,S2C::DROP_TYPE_USE);
				}
				else
				{
					PlayerGetItemInfo(IL_EQUIPMENT,idx);
				}
				return 0;
			}
			//更新物品
			PlayerGetItemInfo(IL_EQUIPMENT,idx);
		}
		return 1;
	}


public:
//各种状态下的消息处理函数
	int GeneralTradeMessageHandler(world * pPlane, const MSG & msg);
	int TradeMessageHandler(world * pPlane ,const MSG & msg);
	int WaitingTradeMessageHandler(world * pPlane ,const MSG & msg);
	int WaitingTradeCompleteHandler(world * pPlane ,const MSG & msg);
	int WaitingTradeReadHandler(world * pPlane ,const MSG & msg);
	int WatingFactionTradeReadHandler(world * pPlane ,const MSG & msg);
	int WaitingFactionTradeTradeHandler(world * pPlane ,const MSG & msg);
	int DisconnectMessageHandler(world * pPlane ,const MSG & msg);
	int WaitingSwitchServer(world * pPlane, const MSG & msg);
	int StayInHandler(world * pPlane, const MSG & msg);
	int MarketHandler(world * pPlane, const MSG & msg);
	int TravelMessageHandler(world * pPlane, const MSG & msg);
	virtual int ZombieMessageHandler(world * pPlane ,const MSG & msg);
	virtual int MessageHandler(world * pPlane ,const MSG & msg);
	virtual int DispatchMessage(world * pPlane ,const MSG & msg);

	
	void FromTradeToNormal(int type = 0);		//从交易状态回归到正常状态
	void FromFactionTradeToNormal(int type = 0);	//从帮派交易状态回归到正常状态

	void GetPlayerCharMode(archive & ar);		//保存数据库时，将玩家当前的一些状态进行保存
	void SetPlayerCharMode(const void * buf, size_t size);
	void PlaneEnterNotify(bool is_enter);
	void GetCommonDataList(bool send_content);

private:
	//放在这里禁止子类调用的目的是 禁止子类添加独特的需要保存的数据结构，使得各种结构保存均一致
	virtual bool Save(archive & ar);
	virtual bool Load(archive & ar);

protected:
	static bool __GetPetAttackHook(gactive_imp * __this, const MSG & , attack_msg & );
	static bool __GetPetEnchantHook(gactive_imp * __this, const MSG & ,enchant_msg & );
	static void __GetPetAttackFill(gactive_imp * __this, attack_msg & attack);
	static void __GetPetEnchantFill(gactive_imp * __this, enchant_msg & enchant);

public:
//虚函数群 包含了npc和玩家统一的操作和以后可能扩展的操作
	virtual void PlayerEnterServer();		//玩家进入服务器，用于切换服务器
	virtual void PlayerEnterWorld();		//玩家进入世界
	virtual void PlayerLeaveServer();		//玩家离开服务器，用于切换服务器
	virtual void PlayerLeaveWorld();		//玩家离开世界
	virtual void PlayerEnterParallelWorld();//玩家进入平行世界，用于玩家换线
	virtual void PlayerLeaveParallelWorld();//玩家离开平行世界
	virtual void Init(world * pPlane,gobject*parent);
	virtual void InitClock();
	virtual void SwitchSvr(int dest, const A3DVECTOR & oldpos, const A3DVECTOR &newpos,const instance_key *switch_key);
	virtual int DoAttack(const XID & target, char force_attack);
	virtual void AdjustDamage(const MSG & msg, attack_msg * attack,damage_entry & dmg,float & damage_adjust);
	virtual void OnDamage(const XID & attacker,int skill_id,const attacker_info_t&info,int damage,int at_state,char speed,bool orange,unsigned char section);
	virtual void OnHurt(const XID & attacker,const attacker_info_t&info,int damage,bool invader);
	virtual void OnHeal(const XID & healer, int life);
	virtual bool CanTrade(const XID & target);
	virtual void StartTrade(int trade_id,const XID & target);
	virtual bool StartFactionTrade(int trade_id,int get_mask, int put_mask,bool no_response = false);
	virtual void TradeComplete(int trade_id,int reason,bool need_read);
	virtual void FactionTradeTimeout();
	virtual void FactionTradeComplete(int trade_id,const GNET::syncdata_t & data);
	virtual void SyncTradeComplete(int trade_id, size_t money, const GDB::itemlist & item_change,bool writetrashbox, bool cash_change);
	virtual void WaitingTradeWriteBack(int trade_id, bool bSuccess);
	virtual void WaitingTradeReadBack(int trade_id,const GDB::itemlist * inventory,int money);
	virtual void WaitingFactionTradeReadBack(const GDB::itemlist * inventory,int money,int timestamp,int mask);
	virtual bool CanAttack(const XID & target);
	virtual bool CheckLevitate();
	virtual void PhaseControlInit();
	virtual const XID & GetCurTarget();
	virtual int GetAmmoCount();
	virtual int TakeOutItem(int item_id);
	virtual void TakeOutItem(int item_id, size_t count);//lgc
	virtual void TakeOutItem(const int * id_list, size_t list_count, size_t count);	//消耗包裹中count个list中任意物品
	virtual bool CheckItemExist(int item_id, size_t count);
	virtual bool CheckItemExist(int inv_index,int item_id, size_t count);
	virtual bool CheckItemExist(const int * id_list, size_t list_count, size_t count);	//检查包裹中是否存在count个list中任意物品
	virtual int CheckItemPrice(int inv_index, int item_id);
	virtual void DropSpecItem(bool isProtected, const XID & owner);
	virtual size_t GetMoneyAmount() { return _player_money;}
	virtual void DecMoneyAmount(size_t money) { SpendMoney(money);_runner->spend_money(money);}
	virtual void DropMoneyAmount(size_t money, bool isProtected);
	virtual int DispatchCommand(int cmd_type, const void * buf,size_t size);
	virtual bool StepMove(const A3DVECTOR &offset);
	virtual void SendDataToSubscibeList();
	virtual void SendTeamDataToSubscibeList();
	virtual void SendTeamDataToMembers();
	virtual	void OnHeartbeat(size_t tick);
	virtual void SetCombatState();
	virtual bool CheckInvaderAttack(const XID & who);
	virtual void FillAttackMsg(const XID & target, attack_msg & attack,int dec_arrow);
	virtual void FillEnchantMsg(const XID & target,enchant_msg & enchant);
	virtual void SendAttackMsg(const XID & target, attack_msg & attack);
	virtual int  GetCSIndex() { return GetParent()->cs_index;}
	virtual int  GetCSSid() { return GetParent()->cs_sid;}
	virtual void SendEnchantMsg(int message,const XID & target, enchant_msg & attack);
	virtual void SendMsgToTeam(const MSG & msg, float range, bool exclude_self);
	virtual bool OI_IsMember(const XID & member) { return IsMember(member);}
	virtual bool OI_IsInTeam() { return IsInTeam();}
	virtual bool OI_IsTeamLeader() { return IsTeamLeader();}
	virtual int SpendFlyTime(int tick);
	virtual int GetFlyTime();
	virtual void KnockBack(const XID & target, const A3DVECTOR & source, float distance,int time,int stun_time);
	virtual void PullOver(const XID & target, const A3DVECTOR & source,float distance, int time);
	virtual void Teleport(const A3DVECTOR & pos, int time, char mode);
	virtual void Teleport2(const A3DVECTOR & pos, int time, char mode);
	virtual void KnockUp(float distance,int time);
	virtual void AddAggroToEnemy(const XID & who,int rage);
	virtual void ClearAggroToEnemy();	//用于玩家进入隐身状态时清除npc对自己的仇恨
	virtual bool DrainMana(int mana);
	virtual void SetIdleMode(bool sleep, bool stun);
	virtual void SetSealMode(bool silent,bool root);
	virtual bool Resurrect(float exp_reduce); //给技能用的接口
	virtual void OnDuelStart(const XID & target);
	virtual void OnDuelStop();
	virtual void Die(const XID & attacker, bool is_pariah, char attacker_mode, int taskdead);
	virtual void OnDeath(const XID & lastattack,bool is_pariah, char attacker_mode, int taskdead);
	virtual void OnPickupMoney(size_t money,int drop_id);
	virtual void OnPickupItem(const A3DVECTOR & pos,const void * data, size_t size,bool isTeam,int drop_id);
	virtual void OnAttacked(world *pPlane, 	const MSG & msg, attack_msg * attack, damage_entry & dmg,bool hit); 
	virtual bool UseProjectile(int count);
	virtual void CancelSwitch();
	virtual void PlayerForceOffline();
	virtual void ServerShutDown();
	virtual void UpdateMafiaPvP(unsigned char pvp_mask);
	virtual void UpdateMafiaInfo(int id, char rank, unsigned char pvp_mask);	//更新帮派信息
	virtual void UpdateFactionRelation(int * alliance, size_t asize, int * hostile, size_t hsize, bool notify_client);//更新帮派同盟信息
	virtual const A3DVECTOR & GetLogoutPos(int & world_tag);//取得登出时应该使用的坐标，可以被覆盖
	virtual bool CheckCoolDown(int idx);
	virtual void SetCoolDown(int idx, int msec);
	virtual void GetCoolDownData(archive & ar);
	virtual void GetCoolDownDataForClient(archive & ar);
	virtual int GetMonsterFaction() { return PLAYER_MONSTER_TYPE;}
	virtual int GetFactionAskHelp() { return PLAYER_MONSTER_TYPE;}
	virtual void EnhanceBreathCapacity(int value);
	virtual void ImpairBreathCapacity(int value);
	virtual void InjectBreath(int value);
	virtual void EnableEndlessBreath(bool bRst);
	virtual void AdjustBreathDecPoint(int offset);
	virtual void EnableFreePVP(bool bVal);
	virtual void ObjReturnToTown();
	virtual void AddEffectData(short effect);
	virtual void RemoveEffectData(short effect);
	virtual void EnterCosmeticMode(unsigned short inv_index,int cos_id);
	virtual void LeaveCosmeticMode(unsigned short inv_index);
	virtual void SetPerHitAP(int ap_per_hit);
	virtual void ModifyPerHitAP(int delta);
	virtual bool IsPlayerClass() { return true;}
	virtual bool IsEquipWing();
	virtual void OnUseAttackRune();
	virtual int GetLinkIndex();
	virtual int GetLinkSID();
	virtual int SummonMonster(int mod_id, int count, const XID& target, int target_distance, int remain_time, char die_with_who, int path_id);
	virtual int SummonNPC(int npc_id, int count, const XID& target, int target_distance, int remain_time);
	virtual int SummonMine(int mine_id, int count, const XID& target, int target_distance, int remain_time);
	virtual bool UseSoulItem(int type, int level, int power);
	virtual void IncAntiInvisiblePassive(int val);
	virtual void DecAntiInvisiblePassive(int val);
	virtual void IncAntiInvisibleActive(int val);
	virtual void DecAntiInvisibleActive(int val);
	virtual void IncInvisiblePassive(int val);
	virtual void DecInvisiblePassive(int val);
	virtual void SetInvisible(int invisible_degree);
	virtual void ClearInvisible();
	virtual void SendDisappearToTeam();	//当玩家处于隐身状态下,离队前要向队友发送消失信息
	virtual void SendDisappearToTeamMember(int id,int cs_index,int cs_sid);//当玩家处于隐身状态下,队友离队了要向其发送消失信息
	virtual void SendAppearToTeam();	//当玩家处于隐身状态下,入队后要向队友发送出现信息
	virtual void SendAppearToTeamMember(int id,int cs_index,int cs_sid);//当玩家处于隐身状态下,队友入队了要向其发送出现消息
	virtual bool CanSeeMe(int player_id);	//根据反隐等级判断对方是否可以看到自己,不考虑组队情况
	virtual int GetSoulPower(){ return _soul_power + _soul_power_en; } 
	virtual void EnhanceSoulPower(int val){ _soul_power_en += val; }
	virtual void ImpairSoulPower(int val){ _soul_power_en -= val; }
	virtual void UpdateMinAddonExpireDate(int addon_expire);  
	virtual void SetGMInvisible();
	virtual void ClearGMInvisible();
	virtual bool ActivateSharpener(int id, int equip_index);
	virtual bool SummonPet2(int pet_egg_id, int skill_level, int life_time);
	virtual bool SummonPlantPet(int pet_egg_id, int skill_level, int life_time, const XID & target, char force_attack);
	virtual bool CalcPetEnhance(int skill_level, extend_prop& prop, int& attack_degree, int& defend_degree, int& vigour);
	virtual bool PetSacrifice();
	virtual bool PlantSuicide(float distance, const XID & target, char force_attack);
	virtual void InjectPetHPMP(int hp, int mp);
	virtual void DrainPetHPMP(const XID & pet_id, int hp, int mp);
	virtual void LongJumpToSpouse();
	virtual void WeaponDisabled(bool disable);
	virtual void DetectInvisible(float range);
	virtual void ForceSelectTarget(const XID & target);
	virtual void ExchangePosition(const XID & target);
	virtual void SetSkillAttackTransmit(const XID & target);
	virtual void CallUpTeamMember(const XID& member){ TeamMemberCongregateRequest(member); }
	virtual void QueryOtherInventory(const XID& target);
	virtual void IncPetHp(int inc){ _pet_enhance.hp_percent += inc; }
	virtual void IncPetMp(int inc){ _pet_enhance.mp_percent += inc; }
	virtual void IncPetDamage(int inc){ _pet_enhance.damage_percent += inc; }
	virtual void IncPetMagicDamage(int inc){ _pet_enhance.magic_damage_percent += inc; }
	virtual void IncPetDefense(int inc){ _pet_enhance.defense_percent += inc; }
	virtual void IncPetMagicDefense(int inc){ _pet_enhance.magic_defense_percent += inc; }
	virtual void IncPetAttackDegree(int inc){ _pet_enhance.attack_degree_percent += inc; }
	virtual void IncPetDefendDegree(int inc){ _pet_enhance.defend_degree_percent += inc; }
	virtual void ReduceResurrectExpLost(int value)	{ _resurrect_exp_lost_reduce += value; }
	virtual void IncreaseResurrectExpLost(int value){ _resurrect_exp_lost_reduce -= value; }
	virtual void SetPlayerLimit(int index, bool b);
	virtual bool GetPlayerLimit(int index);
	virtual void DenyAttackCmd(){ _commander->DenyCmd(controller::CMD_ATTACK); }
	virtual void AllowAttackCmd(){ _commander->AllowCmd(controller::CMD_ATTACK); }
	virtual void DenyElfSkillCmd(){ _commander->DenyCmd(controller::CMD_ELF_SKILL);}
	virtual void AllowElfSkillCmd(){ _commander->AllowCmd(controller::CMD_ELF_SKILL);}
	virtual void DenyUseItemCmd(){_commander->DenyCmd(controller::CMD_USE_ITEM);}
	virtual void AllowUseItemCmd(){_commander->AllowCmd(controller::CMD_USE_ITEM);}
	virtual void DenyNormalAttackCmd(){_commander->DenyCmd(controller::CMD_NORMAL_ATTACK);}
	virtual void AllowNormalAttackCmd(){_commander->AllowCmd(controller::CMD_NORMAL_ATTACK);}
	virtual void DenyPetCmd(){_commander->DenyCmd(controller::CMD_PET);}
	virtual void AllowPetCmd(){_commander->AllowCmd(controller::CMD_PET);}
	virtual void EnterNonpenaltyPVPState(bool b);
	virtual int GetHistoricalMaxLevel(){ return _player_reincarnation.GetHistoricalMaxLevel(); }
	virtual int GetAvailLeadership(){ return _leadership - _leadership_occupied; }	
	virtual void OccupyLeadership(int v){ _leadership_occupied += v; }
	virtual void RestoreLeadership(int v){ _leadership_occupied -= v; }
	virtual void SyncScoreToPlane(){}
	virtual void SetFlagCarrier(bool b){}
	virtual bool TrickBattleTransformChariot(int chariot){ return false; }
	virtual bool TrickBattleUpgradeChariot(int chariot){ return false; }
	virtual void TrickBattleIncChariotEnergy(int energy){}
	virtual size_t OI_GetInventorySize();
	virtual size_t OI_GetEmptySlotSize();
	virtual int OI_GetInventoryDetail(GDB::itemdata * list, size_t size);
	virtual int OI_GetEquipmentDetail(GDB::itemdata * list, size_t size);
	virtual size_t OI_GetEquipmentSize();
	virtual int TradeLockPlayer(int get_mask, int put_mask);
	virtual int TradeUnLockPlayer();
	virtual void ActiveMountState(int mount_id, unsigned short mount_color);
	virtual void DeactiveMountState();
	virtual bool AddPetToSlot(void * data, int inv_index);
	virtual bool FeedPet(int food_mask, int honor);
	virtual bool OI_IsMafiaMember();
	virtual int OI_GetMafiaID();
	virtual char OI_GetMafiaRank();
	virtual bool OI_IsMafiaMaster(); 
	virtual bool OI_IsFactionAlliance(int fid);
	virtual bool OI_IsFactionHostile(int fid);
	virtual int OI_GetSpouseID(){ return GetSpouse(); }
	virtual int OI_GetReputation(){ return _reputation; }
	virtual bool CheckGMPrivilege();
	virtual size_t OI_GetTrashBoxCapacity(int where);
	virtual int OI_GetTrashBoxDetail(int where, GDB::itemdata * list, size_t size);
	virtual bool OI_IsTrashBoxModified();
	virtual bool OI_IsEquipmentModified();
	virtual size_t OI_GetTrashBoxMoney();
	virtual void EnterBattleground(int world_tag, int battle_id);
	virtual void EnterFactionFortress(int dst_world_tag, int dst_factionid);
	virtual void EnterCountryBattle(int world_tag, int battle_id);
	virtual int OI_GetDBTimeStamp();
	virtual int OI_InceaseDBTimeStamp();
	virtual bool CanResurrect(int param);
	virtual int  Resurrect(const A3DVECTOR & pos,bool nomove,float exp_reduce,int target_tag,float hp_factor, float mp_factor, int param);
	virtual bool CheckWaypoint(int point_index, int & point_domain);
	virtual bool ReturnWaypoint(int point);
	virtual attack_judge GetPetAttackHook();
	virtual enchant_judge GetPetEnchantHook();
	virtual attack_fill GetPetAttackFill();
	virtual enchant_fill GetPetEnchantFill();
	virtual bool OI_IsPVPEnable() { return _pvp_enable_flag;}
	virtual char OI_GetForceAttack() { return _force_attack;}
	virtual bool OI_IsInPVPCombatState() { return IsInPVPCombatState();}
	virtual int CheckUseTurretScroll();
	virtual bool OI_IsInventoryFull() { return _inventory.IsFull();} 
	virtual XID OI_GetPetID();
	virtual void OI_ResurrectPet();
	virtual void OI_RecallPet(){ RecallPet(); }
	virtual void Notify_StartAttack(const XID & target,char force_attack);
	virtual bool OI_GetMallInfo(int & cash, int &cash_used, int &cash_delta,  int &order_id);
	virtual bool OI_IsCashModified();
	virtual void ActivePetNoFeed(bool feed);
	virtual size_t OI_GetMallOrdersCount();
	virtual int OI_GetMallOrders(GDB::shoplog * list, size_t size);
	virtual void OnAntiCheatAttack(float rate);
	virtual bool CanTeamRelation();
	virtual bool OI_TestSafeLock();
	virtual size_t OI_GetPetsCount() { return _petman.GetPetsCount(); }
	virtual size_t OI_GetPetSlotCapacity() { return _petman.GetAvailPetSlot(); }
	virtual pet_data * OI_GetPetData(size_t index){ return _petman.GetPetData(index); }
	virtual void OI_TryCancelPlayerBind();
	virtual int OI_GetTaskMask(){ return _task_mask; }
	virtual int OI_GetForceID(){ return _player_force.GetForce(); }
	virtual void UpdateMallConsumptionShopping(int id, unsigned int proc_type, int count, int total_price); // 购买时更新玩家消费值
	virtual void UpdateMallConsumptionBinding(int id, unsigned int proc_type, int count); // 天人合一时更新玩家消费值
	virtual void UpdateMallConsumptionDestroying(int id, unsigned int proc_type, int count); // 消耗或扣除时更新玩家消费值
	virtual bool CalcPetEnhance2(const pet_data *pData, extend_prop& prop, int& attack_degree, int& defend_degree, int& vigour);
	virtual	void GetNatureSkill(int nature,int &skill1,int &skill2); 
	virtual void SaveRealWorldPos(int world_tag, const A3DVECTOR & pos){}
	virtual void SendAllData(bool detail_inv, bool detail_equip, bool detail_task);
	virtual int OI_GetRealm() { return _realm_level; }
	virtual void QueryTrickBattleChariots() {}
    virtual void SetHasPVPLimitFilter(bool has_pvp_limit_filter);
	virtual void EnhanceMountSpeedEn(float sp);
	virtual void ImpairMountSpeedEn(float sp);
public:
//逻辑操作函数群
	int  GetInstanceReenterTimeout();
	int  GetWorldType();
	void CalcEquipmentInfo();		//计算装备代表的数据内容
	bool ObtainItem(int where, item_data * pData, bool isTask = false);//返回false表示这个data被使用了，不要释放
	void DropItemOnDeath(size_t drop_count_inv,size_t drop_count_equip, const XID & spec_owner);
	void DamageItemOnDeath(bool restrict_bind, const XID & killer);
	void ThrowDeadDropItem();
	bool ReturnToTown();
	bool SitDown();
	bool CanSitDown();
	void StandUp();
	void PurchaseItem(abase::pair<const item_data *, int> *pItem,size_t size,size_t money, int consume_contrib, int force_contrib);
	bool ItemToMoney(size_t inv_index, int type, size_t count, int price);
	inline bool UseItem(item_list & inv, int inv_index, int where, int item_type, size_t count)
	{
		bool bRst = inv[inv_index].IsBroadcastUseMsg();
		int type = inv[inv_index].type;
		int proc_type = inv[inv_index].proc_type;
		int rst = inv.UseItem(inv_index,this,count);
		if(rst >= 0)
		{
			ASSERT(rst == item_type);
			UpdateMallConsumptionDestroying(type, proc_type, count);
			_runner->use_item(where,inv_index, item_type,count);
			if(bRst) _runner->use_item(item_type);
			return true;
		}
		return false;
	}
	
	inline bool UseItemWithTarget(item_list & inv, int inv_index, int where, int item_type,const XID& target, char force_attack )
	{
		bool bRst = inv[inv_index].IsBroadcastUseMsg();
		int type = inv[inv_index].type;
		int proc_type = inv[inv_index].proc_type;
		int count = 0;
		int rst = inv.UseItemWithTarget(inv_index,this,target, force_attack,count);
		if(rst >= 0)
		{
			ASSERT(rst == item_type);
			UpdateMallConsumptionDestroying(type, proc_type, count);
			_runner->use_item(where,inv_index, item_type,count);
			if(bRst) _runner->use_item(item_type);
			return true;
		}
		return false;
	}
	
	inline bool UseItemWithTarget(int where,size_t inv_index,int item_type,const XID & target,char force_attack) //内部调用
	{
		ASSERT(where == IL_INVENTORY || where == IL_EQUIPMENT);
		item_list & inv = GetInventory(where);
		ASSERT(inv_index < inv.Size());
		if(inv[inv_index].type != item_type) return false;
		return UseItemWithTarget(inv,inv_index,where, item_type, target,force_attack);
	}

	inline bool UseItem(int where, size_t inv_index, int item_type,size_t count) //内部调用
	{
		ASSERT(where == IL_INVENTORY || where == IL_EQUIPMENT);
		item_list & inv = GetInventory(where);
		ASSERT(inv_index < inv.Size());
		if(inv[inv_index].type != item_type) return false;
		return UseItem(inv,inv_index,where,item_type, count);
	}
	bool UseItemWithArg(item_list & inv, int inv_index, int where, int item_type, const char * arg, size_t arg_size);

	int ThrowDart(const XID & target,int damage,float throw_range,char force_attack = false);
	void MakeDartAttack(int damage, float throw_range,attack_msg &attack,char force_attack);
	void RepairAllEquipment();
	void RepairEquipment(int where, size_t index);
	bool EmbedChipToEquipment(size_t chip,size_t equip);
	bool SharpenEquipment(size_t index, addon_data * addon_list, size_t count, int sharpener_level, int sharpener_gfx);
	bool RechargeEquippedFlySword(size_t index, size_t count);
	bool RechargeFlySword(size_t element_index, size_t count,size_t fw_index,int fw_id);
	void LongJump(const A3DVECTOR &pos); 				//空间跳转
	bool LongJump(const A3DVECTOR &pos,int target_tag,int ctrl_id = 0); //穿越位面跳转
	void Swap(gplayer_imp * rhs);		//交换玩家数据，以更改玩家逻辑
	void IdentifyItemAddon(size_t index, size_t fee);
	bool RefineItemAddon(size_t index, int item_type,int rt_index);
	bool StartTravelSession(size_t min_time,size_t max_time, const A3DVECTOR & dest_pos,float speed,int vehicle);
	bool CompleteTravel(int vehicle,const A3DVECTOR & target);
	bool TestSanctuary();			//检查是否出于安全区
	bool TestPKProtected();		//检查是否处于新手安全区
	int CheckPlayerMove(const A3DVECTOR & offset, int mode, int use_time);
	void SetCoolDownData(const void * buf, size_t size);
	bool CheckAndSetCoolDown(int idx, int msec);
	void SwitchFashionMode();
	bool RegionTransport(int rindex, int tag);
	void EnterResurrectReadyState(float exp_reduce, float hp_factor, float mp_factor);
	void RebuildAllInstanceData(int create_time);	  //重建所有的数据
	void RebuildInstanceKey();			  //重建所有的key
	bool GetInstanceKeyBuf(char * buf, size_t size);  //取得Instance Key buf
	bool SetInstanceKeyBuf(const void * buf, size_t size); //设置所有的instance key 
	int  CheckInstanceTimer(int tag);		  //检查是否可以进行副本重置，如果可以设置当前的timer
	void SetInstanceTimer(int tag);			  //设置进入副本的时间，用来限定是否可以进行副本重置的 
	void SaveInstanceKey(archive & ar);		  //保存副本key数据进入数据库
	void InitInstanceKey(int create_time, const void * buf , size_t size);	//从数据库初始化副本数据，只有在buf和size为0的时候create_time才有用
	void ResetInstance(int world_tag);		//重置world_tag对应副本
	void CosmeticSuccess(int ticket_inv_idx, int ticket_id, int result, unsigned int crc);
	void Say(const char * msg, int channel = GMSV::CHAT_CHANNEL_LOCAL);
	void GBKSay(const char * msg, int channel = GMSV::CHAT_CHANNEL_LOCAL);
	void GainMoneyWithDrop(size_t inc, bool log = true);
	void GainMoney(size_t inc);		//no drop
	void SpendMoney(size_t delta,bool log = true);
	bool CheckIncMoney(size_t inc);
	bool CheckDecMoney(size_t delta);
	void LeaveDoubleExpMode();
	int EnterDoubleExpMode(int time_mode, int timeout);	//返回当前可用的双倍时间
	bool ActiveDoubleExpTime(int time);
	void TrashBoxOpen(bool view_only);
	void TrashBoxClose(bool view_only);
	void TryClearTBChangeCounter();
	void UserTrashBoxOpen();
	void UserTrashBoxClose();
	void TryClearUserTBChangeCounter();
	bool CheckPlayerAutoSupport();
	bool CheckPlayerBindRequest();
	bool CheckPlayerBindInvite();
	bool EnterBindMoveState(const XID & target);
	bool EnterBindFollowState(const XID & target);
	void ReturnBindNormalState();
	void TestUnderWater();
	void UpdatePlayerLayer();

	bool TestCreatePet(size_t index);
	bool SummonPet(size_t index);
	bool BanishPet(size_t index);
	bool RecallPet();
	void SetPetSlotCapacity(size_t new_size, bool notify = true);
	size_t GetPetSlotCapacity();
	void DBSetPetData(size_t index, const void * data, size_t size);
	pet_data * GetPetData(size_t index);
	
	int ServiceConvertPetToEgg(size_t index);
	void SendPetCommand(int cur_target,int cmd,const void * buf, size_t size);
	void SetPetLeaderData(pet_leader_prop & data);
	void SayHelloToNPC(const XID & target);
	int ConvertPetToEgg(size_t slot_index);
	int ConvertPetDataToEggData(void * data, size_t size, pet_data * pData, const void * src_temp);
	int RegroupPropPoint(int str, int agi, int vit,int eng);	//洗点
	void LeaveAbnormalState();		//试图离开非正常状态
	void SendFarCryChat(char channgel, const void * msg, size_t size, const void* data, size_t dsize);
	void SendTeamChat(char channel, const void * buf, size_t len, const void* data, size_t dsize, int use_id = 0);
	void SendNormalChat(char channel, const void * buf, size_t len, const void* data, size_t dsize);
	void SendBattleFactionChat(char channel, const void * buf, size_t len, const void* data, size_t dsize);
	void SendCountryChat(char channel, const void * msg, size_t size, const void* data, size_t dsize);
	bool ResurrectPet(size_t index);
	void NotifyMasterInfoToPet(bool at_once = false);
	void FirstAcquireItem(const item_data* itemdata);
	bool ChangePetName(size_t index,const char name[] , size_t name_len);
	bool ForgetPetSkill(int skill_id);
	int LearnPetSkill(int skill_id);
	bool IsPetExist(size_t index, int pet_tid);
	int DyePet(size_t pet_index, size_t dye_index);
	void RemoveAllExpireItems();
	void RemoveAllExpireAddon();
	bool CheckItemBindCondition(size_t index,  int id, int can_webtrade);
	bool BindItem(size_t index, int id, int can_webtrade);
	bool CheckBindItemDestory(size_t index, int id);
	bool DestoryBindItem(size_t index, int id);
	bool CheckRestoreDestoryItem(size_t index, int id);
	bool RestoreDestoryItem(size_t index, int id);
	void SetChatEmote(int emote_id);
	void SetHPAutoGen(int value, float rate);
	void SetMPAutoGen(int value, float rate);
	bool ProduceItem(const recipe_template & rt);
	bool ProduceItem2(const recipe_template & rt,int materials[16], int idxs[16]);
	bool ProduceItem3(const recipe_template & rt,int materials[16], int idxs[16], int equip_id, int equip_inv_idx, char inherit_type);
	bool ProduceItem4(const recipe_template & rt,int materials[16], int idxs[16], int equip_id, int equip_inv_idx, char & inherit_type, void **pItem, unsigned short &crc, int &eq_refine_level, int &eq_socket_count, int eq_stone_type[], addon_data eq_engrave_addon_list[3], size_t & eq_engrave_addon_count);
    bool ProduceItem5(const recipe_template & rt,int materials[16], int idxs[16], int equip_id, int equip_inv_idx, char inherit_type);
	bool EngraveItem(const engrave_recipe_template& ert, size_t inv_index);
	bool ItemAddonRegen(const addonregen_recipe_template& arrt, size_t inv_index);
	void ChangeInventorySize(int size);
	int  DyeFashionItem(size_t f_index, size_t d_index);
	int  DyeSuitFashionItem(unsigned char inv_idx_body,unsigned char inv_idx_leg,unsigned char inv_idx_foot,unsigned char inv_idx_wrist,unsigned char inv_idx_dye);
	int GetRequireDyeCount(unsigned char inv_idx, int equip_index);
	void FindCheater(int type, bool noreport);
	void PunishCheat();
	void FindCheater2();
	void WallowControl(int level, int p_time, int l_time, int h_time, int reason);
	void QuestionBonus();
	void DoTeamRelationTask(int reason);
	int RefineTransmit(size_t src_index, size_t dest_index);
	void TaskSendMessage(int task_id, int channel, int param);
	void ItemMakeSlot(size_t index, int id);
	void RepairDamagedItem(unsigned char inv_idx);
	void GodEvilConvert(unsigned char mode);
	int WeddingBook(int start_time, int end_time, int scene, int bookcard_index);
	int WeddingCancelBook(int start_time, int end_time, int scene);
	int WeddingInvite(int invitecard_index, int invitee);
	int GetKnockBackPos(const A3DVECTOR& attacker_pos, float back_distance, A3DVECTOR & back_pos); 
	int FactionFortressLevelUp();
	int FactionFortressSetTechPoint(size_t tech_index);
	int FactionFortressConstruct(int id, int accelerate);
	int FactionFortressHandInMaterial(size_t inv_index, int id, size_t count);
	int FactionFortressHandInContrib(int contrib);
	int FactionFortressMaterialExchange(size_t src_index,size_t dst_index,int material);
	int FactionFortressDismantle(int id);
	int FactionFortressResetTechPoint(size_t tech_index, size_t inv_index, int id);
	void FindSpecItem(unsigned char where, int type, archive& ar);
	int RemoveSpecItem(unsigned char where, unsigned char index, size_t count, int type);
	bool TryTransmitSkillAttack(const MSG & msg);
	int GetForceReputation(){ return _player_force.GetReputation(); }
	int GetForceContribution(){ return _player_force.GetContribution(); }
	bool IncForceReputation(int value);
	bool DecForceReputation(int value);
	bool IncForceContribution(int value);
	bool DecForceContribution(int value);
	void UpdateForceTicketInfo(int require_force, int repu_inc_ratio);
	int CountryJoinApply();
	bool CountryJoinStep1(int country_id, int country_expiretime, int major_strength, int minor_strength, int world_tag, const A3DVECTOR & pos);
	bool CountryJoinStep2();
	int CountryLeave();
	void CountryTerritoryMove(const A3DVECTOR & pos, bool capital);
	void GetCountryKickoutPos(int & world_tag, A3DVECTOR & pos);
	void GetAUMailTask(int level,char ex_reward);

	bool EquipSign(int ink_inv_idx, int ink_id, int equip_inv_idx, int equip_id, const char * signature, unsigned int signature_len);
	int PlayerEvolutionPet(size_t index,int formula_index);
	int PlayerAddPetExp(size_t index,size_t num);
	int PlayerRebuildPetInheritRatio(size_t index,int formula_index);
	int PlayerRebuildPetNature(size_t index,int formula_index);
	bool RebulidPetInheritRatio(int pet_id,size_t index,int formula_index,int &r_attack,int &r_defense,int &r_hp,int &r_atk_lvl,int &r_def_lvl);
	void AcceptInheritRatioResult(int pet_id,size_t index,int r_attack,int r_defense,int r_hp,int r_atk_lvl,int r_def_lvl);
	bool RebuildPetNature(int pet_id,size_t index,int formula_index,int &nature);
	void AcceptNatureResult(int pet_id,size_t index,int nature);
	bool ChangeEquipAddon(unsigned char equip_idx,unsigned char equip_socket_idx,int old_stone_type,int new_stone_type,int recipe_type,int* materials_ids,unsigned char* idxs,int count);
	bool ReplaceEquipAddon(unsigned char equip_idx,unsigned char equip_socket_idx,int old_stone_type,int new_stone_type,unsigned char new_stone_idx);
	void OnTouchPointQuery(int64_t income,int64_t remain);
	void OnTouchPointCost(int64_t orderid,unsigned int cost,int64_t income,int64_t remain,int retcode);
	void OnGiftCardRedeem(const char(&cn)[player_giftcard::GIFT_CARDNUMBER_LEN],int type,int parenttype,int retcode);
	void OnAuAddupMoneyQuery(int64_t addupmoney);
	void UpdateDisplayTitle(TITLE_ID titleid);
	void OnObtainRareTitle(TITLE_ID titleid);

	void EnterTrickBattleStep1(int world_tag, int battle_id, int chariot);
	void EnterTrickBattleStep2();
	void OnAutoTeamPlayerReady(int leader_id);
	void OnAutoTeamComposeFailed(int leader_id);
	void OnAutoTeamComposeStart(int member_list[], unsigned int cnt);

	void ReceiveRealmExp(int exp);
	void UpdateRealmLevel();
	bool ExpandRealmLevelMax();
	bool IsRealmExpFull();
	int GetObtainedGeneralCardCountByRank(int rank);
	void OnMafiaPvPAward(int type,const XID& sourceid, const A3DVECTOR& pos, int mafiaid, int domainid);
public:

//玩家操作函数群
//这些函数都是player的操作，所以参数可能是错误的
	int  PlayerGetItemInfo(int where, int index);
	int  PlayerGetItemInfoList(int where, size_t count , unsigned char *list);
	void PlayerGetInventory(int where);
	void PlayerGetInventoryDetail(int where);
	void PlayerGetTrashBoxInfo(bool detail);
	void PlayerGetPortableTrashBoxInfo(bool detail);
	void PlayerGetUserTrashBoxInfo(bool detail);
	void PlayerExchangeInvItem(size_t idx1,size_t idx2);
	void PlayerMoveInvItem(size_t src,size_t dest,size_t count);
	void PlayerExchangeTrashItem(int where, size_t idx1,size_t idx2);
	void PlayerMoveTrashItem(int where, size_t src,size_t dest,size_t count);
	void PlayerTrashItemToInv(int where, size_t idx_tra, size_t idx_inv, size_t count);
	void PlayerInvItemToTrash(int where, size_t idx_inv, size_t idx_tra, size_t count);
	void PlayerExchangeTrashInv(int where, size_t idx_tra, size_t idx_inv);
	void PlayerExchangeTrashMoney(char is_usertrashbox, size_t inv_money, size_t tra_money);
	void PlayerDropInvItem(size_t index,size_t count,bool isProtected);
	void PlayerDropEquipItem(size_t index,bool isProtected);
	void PlayerExchangeEquipItem(size_t index1,size_t index2);
	void PlayerEquipItem(size_t index_inv, size_t index_equip);
	void PlayerMoveEquipItem(size_t index_inv, size_t index_equip);
	void PlayerGoto(const A3DVECTOR &pos);
	void PlayerDropMoney(size_t amount,bool isProtected);
	void PlayerGetProperty();
	void PlayerSetStatusPoint(size_t vit, size_t eng, size_t str, size_t agi);
	bool PlayerSitDown();
	bool PlayerStandUp();
	void PlayerRestartSession();
	void PlayerAssistSelect(const XID & cur_target);
	bool PlayerUseItem(int where, size_t inv_index, int item_type,size_t count);
	bool PlayerSitDownUseItem(int where, size_t inv_index, int item_type,size_t count);
	bool PlayerUseItemWithTarget(int where,size_t inv_index,int item_type,char force_attack);
	bool PlayerUseItemWithArg(int where,size_t inv_index,int item_type,size_t count, const char * buf, size_t buf_size);
	bool PlayerOpenPersonalMarket(size_t count, const char name[PLAYER_MARKET_MAX_NAME_LEN], int * entry_list);
	bool PlayerTestPersonalMarket();
	bool PlayerDestroyItem(int where, size_t index, int item_type);
	void PlayerEnablePVPState();
	void PlayerDisablePVPState();
	void PlayerRegionTransport(int rindex, int tag);
	void PlayerLogout(int type);
	void PlayerDuelRequest(const XID & target);
	void PlayerDuelReply(const XID & target,int param);
	void PlayerBindRequest(const XID & target);
	void PlayerBindInvite(const XID & target);
	void PlayerBindRequestReply(const XID & target, int param);
	void PlayerBindInviteReply(const XID & target, int param);
	void PlayerBindCancel();
	int PlayerSummonPet(size_t index);
	int PlayerRecallPet();
	int PlayerBanishPet(size_t index);
	bool PlayerDoShopping(size_t item_count,const int * order, int shop_tid = 0);
	bool ForeignDoShoppingStep1(int goods_id, size_t goods_index, size_t goods_slot);
	bool ForeignDoShoppingStep2(int item_id, size_t count, int expire_time, size_t cash_need);
	bool PlayerGetMallItemPrice(int start_index, int end_index);	//lgc
	bool PlayerEquipTrashboxFashionItem(unsigned char trash_idx_body,unsigned char trash_idx_leg,unsigned char trash_idx_foot,unsigned char trash_idx_wrist,unsigned char trash_idx_head,unsigned char trash_idx_weapon); 
	bool CheckEquipTrashboxItem(item_list & trashbox, unsigned char trash_idx,unsigned char equip_idx);
	bool PlayerEquipTrashboxItem(int where, unsigned char trash_idx,unsigned char equip_idx); 
	void PlayerCheckSecurityPasswd(const char * passwd, size_t passwd_size);	
	void PlayerNotifyForceAttack(char force_attack, char refuse_bless);
	bool PlayerDoDividendShopping(size_t item_count,const int * order, int shop_tid = 0);
	bool PlayerGetDividendMallItemPrice(int start_index, int end_index);	//lgc
	bool PlayerChooseMultiExp(int index);
	bool PlayerGetFactionFortressInfo();
	bool TeamCongregateRequest(int reply_level_req, int reply_sec_level_req, int reply_reincarnation_times_req);
	bool TeamMemberCongregateRequest(const XID& member);
	bool FactionCongregateRequest(int reply_level_req, int reply_sec_level_req, int reply_reincarnation_times_req);
	void RecvCongregateRequest(char type, int sponsor, int world_tag, const A3DVECTOR& pos, int level_req, int sec_level_req, int reply_reincarnation_times_req);
	void PlayerCongregateReply(char type, char agree, int id);
	void PlayerGetDpsDphRank(unsigned char rank_mask);
	int PlayerJoinForce(int force_id);
	int PlayerLeaveForce();
	void PlayerProduce4Choose(bool remain);
	void PlayerRechargeOnlineAward(int type, size_t count, int * list);
	bool PlayerGetCountryBattlePersonalScore();
	bool PlayerGetCountryBattleStrongholdState();
	bool PlayerGetCountryBattleLiveShow();
	void PlayerLeaveCountryBattle();
	int PlayerTryChangeDS(int flag);
	void PlayerChangeDSLogout(int flag);
	void PlayerExchangeWanmeiYinpiao(bool is_sell, size_t count);	//仅摆摊状态下可操作
	bool GenerateNatureSkills(pet_data *pData); 
	void PlayerAcceptRebuildInheritResult(bool isaccept);
	void PlayerAcceptRebuildNatureResult(bool isaccept);
	void PlayerGivePresent(int roleid, int mail_id, int goods_id, int goods_index, int goods_slot);
	void PlayerAskForPresent(int roleid, int goods_id, int goods_index, int goods_slot);
	int PlayerTryRefineMeridian(int index);
	void PlayerAddFreeRefineTimes(int num);
	int PlayerDecomposeFashionItem(size_t inv_index, int fashion_type, size_t fee, int production_type);
	void PlayerTouchPointQuery();
	void PlayerTouchPointCost(unsigned int index,int type,unsigned int count,unsigned int price,int expire,unsigned int lots);
	void PlayerRedeemGiftCard(const char(&cn)[player_giftcard::GIFT_CARDNUMBER_LEN]);
	void PlayerQueryTitle(int roleid);
	void PlayerChangeTitle(TITLE_ID titleid);
	void PlayerDailySignin();
	void PlayerLateSignin(char type,int itempos,int desttime);
	void PlayerApplySigninAward(char type ,int mon);
	void PlayerRefreshSignin();
	void PlayerSwitchInParallelWorld(const instance_hash_key & key);
	void PlayerQueryParallelWorld();
	void PlayerReincarnation();
	void PlayerGetReincarnationTome();
	void PlayerRewriteReincarnationTome(size_t record_index, int record_level);
	void PlayerActiveReincarnationTome(bool b);
	int PlayerTrickBattleApply(int chariot);
	void PlayerLeaveTrickBattle();
	void PlayerSetAutoTeamGoal(char goal_type, char op, int goal_id);
	void PlayerJumpToGoal(int goal_id);
	int PlayerGeneralCardRebirth(size_t major_inv_idx, size_t minor_inv_idx);
	int PlayerSwallowGeneralCard(size_t equip_idx, bool is_inv, size_t inv_idx, size_t count);
	void PlayerQueryChariots();
	int PlayerImproveFlysword(size_t inv_idx, int flysword_id);
	void PlayerRandMallQuery(int eid);
	void PlayerRandMallRoll(int eid);
	void PlayerRandMallPay(int eid);
	void PlayerQueryMafiaPvPInfo();
    void PlayerQueryCanInheritAddons(int equip_id, unsigned int inv_idx);
    void ActivateRegionWayPoints(int num, int waypoints[]);
	void PlayerReenterInstance();
    void PlayerStartEnterSanctuarySession();
    void PlayerAddPVPLimitFilter();

public:
//断线操作函数
	void LostConnection(int offline_type = PLAYER_OFF_OFFLINE);
	void KickOut() { LostConnection(PLAYER_OFF_KICKOUT); }
	void SendLogoutRequest(int type, int retcode = 0);

protected:
//私有的函数逻辑
	static int DoPlayerMarketTrade(const XID & trader,const XID & customer ,gplayer * pTrade, gplayer *pBuyer, const void * order, size_t length);		//向商店购买
	static int DoPlayerMarketPurchase(const XID & trader,const XID & customer,gplayer * pTrade, gplayer *pBuyer, const void * order, size_t length);		//向商店卖出
	void DoPlayerMarketTrade(player_stall::trade_request & req, gplayer_imp * pImp, size_t need_money, size_t money_to_yinpiao);
	bool CheckMarketTradeRequest(player_stall::trade_request & req, size_t &need_money, size_t& money_to_yinpiao, gplayer_imp * pBuyerImp);
	bool CheckMarketTradeRequire(player_stall::trade_request & req, size_t need_money, gplayer_imp * pTraderImp);
	bool CancelPersonalMarket();

	bool CheckMarketPurchaseRequest(player_stall::trade_request & req,size_t & total_price, gplayer_imp * pBuyerImp);
	bool CheckMarketPurchaseRequire(player_stall::trade_request & req,size_t total_price, size_t& money_to_yinpiao, gplayer_imp * pTraderImp);
	void DoPlayerMarketPurchase(player_stall::trade_request & req, gplayer_imp * pImp, size_t total_price, size_t money_to_yinpiao);

	int Repair(item & it,int where,int index);
	void RefreshEquipment();		//重新计算装备数据
	int SelectRandomArmor();		//选择出对哪个装备进行减少耐久度的选择
	bool EquipItem(size_t index_inv,size_t index_equip);	//装备物品
	void Logout(int type);			//登出操作

	void QueryOtherPlayerInfo1(size_t count, int id[]);
	void QueryNPCInfo1(size_t count, int id[]);

	inline int DispatchNormalMessage(world * pPlane, const MSG & msg)
	{
		if(!(gplayer*)_parent->IsZombie())
		{
			return MessageHandler(pPlane,msg);
		}
		else
		{
			return ZombieMessageHandler(pPlane,msg);
		}
	}
	void PutItemAfterTrade(item_list & backpack);
	void DropAllAfterTrade(item_list & backpack,size_t money);
	void AutoSaveData();
	int DoSwitch(const MSG & msg);
	void FromSwitchToNormal();
	void IncExp(int& exp,int& sp,float double_exp_sp_factor=1.0f, bool double_sp = false);	 //直接加经验，不进行任何修正 其他函数都会到这里
	void ReceiveExp(int exp,int sp);			//直接加经验，不进行级别惩罚，但进行经验加成
	void ReceiveExp(const msg_exp_t & entry);		//会进行经验惩罚
	void ReceiveGroupExp(const A3DVECTOR &pos,int total_damage,int exp,int sp,int level,size_t count,int npcid, int npctag, int npc_planeindex, float r, const msg_grpexp_t * list);
	void LevelUp();
	void PickupMoneyInTrade(size_t money);
	void PickupItemInTrade(const A3DVECTOR & pos,const void * data, size_t size,bool isTeam, int drop_id);
	void SendServiceRequest(int service_type, const void * buf, size_t length);
	void QueryServiceContent(int service_type);
	void ThrowInvItem(size_t index, size_t count, bool isProtected, unsigned char drop_type, const XID & spec_owner=XID(-1,-1));
	void ThrowEquipItem(size_t index, bool isProtected,unsigned char drop_type, int throw_count = 0, const XID & spec_owner=XID(-1,-1));
	void ThrowMoney(size_t amount, bool isProtected);
	void ReduceSaveTimer(int counter) { _write_timer -= counter;}

	template <int foo>
	inline void DoWeaponOperation()
	{
		bool is_update = false;
		_equipment[item::EQUIP_INDEX_WEAPON].AfterAttack(_equipment,&is_update);
		if(is_update)
		{
			RefreshEquipment();
		}
	}

public:	//lgc
	bool IsElfEquipped(){return _cur_elf_info.id != (unsigned int)-1;}
	void UpdateCurElfInfo(unsigned int id, short refine_level, short str, short agi, short vit, short eng, const char * skilldata, int cnt);
	void ClearCurElfInfo();
	void ClearCurElfVigor(){ _cur_elf_info.vigor = 0.0f; }
	void UpdateElfProp(); //将elf_enhance更新到_cur_elf_info的final_...中
	void UpdateElfVigor();//更新小精灵元气相关

	void UpdateMinElfStatusValue(int value);
	void UpdateAllElfSecureStatus();
	void TriggerElfRefineEffect();
	bool IsElfRefineEffectActive(){return _cur_elf_info.id != (unsigned int)-1 && _cur_elf_info.refine_effect_active;}
	void ElfRefineActivate(short refine_level);
	void ElfRefineDeactivate(short refine_level);
	
	void ElfAddAttribute(short str, short agi, short vit, short eng);
	void ElfAddGenius(short g0, short g1, short g2, short g3, short g4);
	void ElfPlayerInsertExp(unsigned int exp, char use_sp);
	unsigned int ElfInsertExpUsePill(unsigned int exp, int exp_level);
	void ElfReceiveExp(unsigned int exp);
	void ElfEquipItem(size_t index_inv);
	void ElfChangeSecureStatus(int status);
	void CastElfSkill(unsigned short skill_id, char force_attack, int target_count, int * targets);
	bool RechargeEquippedElf(size_t index, size_t count);	//与飞剑加耐力逻辑完全相同
	
	void ElfDecAttribute(size_t inv_idx_elf, size_t inv_idx_ticket, short str, short agi, short vit,  short eng);
	void ElfFlushGenius(size_t inv_idx_elf, size_t inv_idx_ticket);
	void ElfLearnSkill(size_t inv_idx_elf, unsigned short skill_id);
	void ElfForgetSkill(size_t inv_idx_elf, unsigned short skill_id, short forget_level);
	void ElfRefine(size_t inv_idx_elf, size_t inv_idx_ticket, int ticket_cnt);
	void ElfRefineTransmit(size_t inv_idx_src, size_t inv_idx_dst);
	void ElfDecompose(size_t inv_idx_elf);
	void ElfDestroyItem(size_t inv_idx_elf, int mask, size_t inv_idx_equip);
	virtual void UpdateStallInfo(int id, int max_sell, int max_buy, int max_name);
	virtual void ClearStallInfo();
	virtual void OnStallCardTakeOut();
	virtual bool Produce4ChooseExec(const recipe_template & rt, int equip_id, int equip_inv_idx, char inherit_type, void **pItem, unsigned short crc, int eq_refine_level, int eq_socket_count, int eq_stone_type[], addon_data eq_engrave_addon_list[3], size_t eq_engrave_addon_count);
	void PlayerRenameRet(const void *new_name, size_t name_len, int ret);

	//obj_interface接口	
	bool OI_GetElfProp(short& level, short& str, short& agi, short& vit, short& eng);
	int OI_GetElfVigor(){return ((_cur_elf_info.id != (unsigned int)-1) ? (int)_cur_elf_info.vigor : -1);}
	int OI_GetElfStamina(){return _equipment[item::EQUIP_INDEX_ELF].GetStamina();}
	bool OI_DrainElfVigor(int dec);
	bool OI_DrainElfStamina(int dec);

//debug only
	void dump_elf_info();
	void get_mall_detail();	//获取当前商城中存在销售时间控制的商品的详细信息
	void change_elf_property(int index, int value);
	void change_factionfortress(int index, int value);
    void copy_other_role_data(int src_roleid);
//debug only
};

void TrySwapPlayerData(world * pPlane,const int cid[3],gplayer * pPlayer);

#endif
