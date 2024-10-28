#ifndef __ONLINEGAME_GS_IMP_H__
#define __ONLINEGAME_GS_IMP_H__

#include <octets.h>
#include <ASSERT.h>
#include <common/packetwrapper.h>
#include <vector.h>
#include <common/message.h>
#include "substance.h"
#include "attack.h"

namespace GNET
{
	struct syncdata_t;
}

namespace GDB
{
	struct itemlist;
}

struct item_data;
struct slice;
struct gobject;
class world;
class gobject_imp;
class item;
struct extend_prop;
struct instance_key;

class controller :public substance
{
public:
DECLARE_SUBSTANCE(controller);
public:
	gobject_imp * _imp;

public:
	controller():_imp(NULL){}
	virtual void Init(gobject_imp * imp) {_imp = imp;} 
	virtual ~controller(){}

	/*
	 *	控制部分的消息处理函数，由于控制部分有时可能需要截取一些消息
	 *	所以这里也可以对消息进行处理
	 *	能够进行处理的前提是_do_msg的值为True
	 *	对消息处理函数的要求和实现部分是一样的
	 */
	virtual  int MessageHandler(world * pPlane ,const MSG & msg)
	{
		return 0;
	}

	

	/*
	 *	处理到来的命令
	 */
	virtual  int CommandHandler(int cmd_type,const void * buf, size_t size) = 0;

	/**
	 *	在两个slice间移动，由于可能是不同的对象，所以做成虚函数
	 *	由于controller必然要和正确的gobject绑定，所以在这里面作
	 *	返回非0表示移动失败，一般指对象已经不在src中
	 */
	virtual int MoveBetweenSlice(gobject * obj,slice * src, slice * dest) = 0;

	/**
	 *	释放对象本身，由于只有controller肯定与NPC、Player绑定，所以由这里释放。
	 */
	virtual void Release(bool free_parent = true) = 0;

	/**
	 *	重生
	 */

	virtual void Reborn()
	{
		ASSERT(false);
	}

	/**
	 	服务器被切换时发生的调用
	 */
	virtual void SwitchSvr(int dest, const A3DVECTOR & oldpos, const A3DVECTOR &newpos,const instance_key * switch_key) { ASSERT(false);}


	virtual void error_cmd(int msg)
	{
	}
	virtual void OnHeartbeat(size_t tick)
	{
	}
	virtual void  NPCSessionStart(int task_id, int session_id)
	{}

	virtual void NPCSessionEnd(int task_id,int session_id, int retcode)
	{}

	virtual void NPCSessionUpdateChaseInfo(int task_id,const void * buf ,size_t size)
	{}

	virtual int GetNextMoveSeq() { return -1;}
	virtual int GetCurMoveSeq() { return -1;}
	virtual void SetNextMoveSeq(int seq) { return ;}

	virtual bool HasGMPrivilege() { return false;}

	virtual void DenyCmd(size_t cmd_type) {}
	virtual void AllowCmd(size_t cmd_type){}

public:
	enum
	{
		CMD_MOVE,
		CMD_ATTACK,
		CMD_PICKUP,
		CMD_MARKET,
		CMD_PET,
		CMD_ELF_SKILL,
		CMD_USE_ITEM,
		CMD_NORMAL_ATTACK,
		CMD_MAX,
	};
};

class dispatcher :public substance
{
public:
DECLARE_SUBSTANCE(dispatcher);
protected:
	gobject_imp * _imp;
public:
	dispatcher():_imp(NULL){}
	virtual ~dispatcher(){}

	void set_imp(gobject_imp * imp) {_imp = imp;} 
	virtual void init(gobject_imp * imp) {_imp = imp;} 
	virtual void set_gm_invisible(bool invisible = true) {}
	virtual bool is_gm_invisible() {return false;}

	virtual void begin_transfer() = 0;				//开始一次数据传送
	virtual void end_transfer() = 0;				//结束一次数据传送，清空缓冲
	virtual void enter_slice(slice * ,const A3DVECTOR &) = 0;	//进入一个格子，告诉其他人进入，并且取得这个格子的信息
	virtual void leave_slice(slice * ,const A3DVECTOR &) = 0;	//离开一个格子，告诉其他人离开
	virtual void notify_pos(const A3DVECTOR & pos) {};		//通知自己的当前位置，只有player用这个命令
	virtual void get_base_info() = 0;				//发送自己的基础数据给自己，以后可能会更改
	virtual void enter_world() = 0;					//进入世界
	virtual	void leave_world() = 0;					//离开世界
	virtual void appear() { enter_world();}				//显形
	virtual void move(const A3DVECTOR & target, int cost_time,int speed,unsigned char move_mode) = 0;//派发移动指令
	virtual void stop_move(const A3DVECTOR & target, unsigned short speed,unsigned char dir,unsigned char move_mode){}//派发停止移动指令
	virtual void notify_move(const A3DVECTOR &oldpos, const A3DVECTOR & newpos)  = 0;//将用户移动的命令转发到其他服务器上
	virtual void start_attack(const XID &){}
	virtual void attack_once(unsigned char dec_amount){}
	virtual void stop_attack(int flag){}
	virtual void dodge_attack(const XID &attacker,int skilll_id, const attacker_info_t &, int at_state,char speed,bool orange,unsigned char section) {}
	virtual void be_damaged(const XID & id, int skill_id,const attacker_info_t &,int damage,int dura_index,int at_state,char speed,bool orange,unsigned char section){}
	virtual void be_hurt(const XID & id, const attacker_info_t & ,int damage,bool invader){}
	virtual void equipment_damaged(int index,char reason){}
	virtual void on_death(const XID & killer,bool ) {}
	virtual void error_message(int message) {}
	virtual void disappear() {}					//发送消失的消息
	virtual void matter_pickup(int id) {}				//发送消失的消息
	virtual void resurrect(int){}
	virtual void pickup_money(int){}
	virtual void pickup_item(int type,int expire_date, int amount,int slot_amount, int where,int index){}
	virtual void purchase_item(int type, size_t money,int amount,int slot_amount, int where,int index){}
	virtual void query_info00(const XID & target, int cs_index,int sid){}
	virtual void query_info00(){}					//返回自己的info00，只有player有效 
	virtual void query_elf_vigor(){} //lgc
	virtual void query_elf_enhance(){}
	virtual void query_elf_stamina(int sta){}
	virtual void query_elf_exp(int exp){}
	virtual void elf_cmd_result(int cmd, int result, int param1, int param2){}
	virtual void elf_levelup(){} 
	virtual void receive_exp(int exp,int sp){}			//接受经验值和技能点
	virtual void level_up(){}					//升级
	virtual void unselect(){}					//没有选中任何人
	virtual void player_select_target(int id){}	
	virtual void self_item_empty_info(int where,int index) {}
	virtual void self_item_info(int where,int index,item_data & data,unsigned short crc) {}
	virtual void self_inventory_data(int where,unsigned char inv_size,const void * data, size_t len) {}
	virtual void self_inventory_detail_data(int where,unsigned char inv_size,const void * data, size_t len) {}
	virtual void exchange_inventory_item(size_t idx1,size_t idx2) {}
	virtual void move_inventory_item(size_t src,size_t dest,size_t count) {}
	virtual void player_drop_item(size_t where, size_t index,int type,size_t count,unsigned char drop_type) {}
	virtual void exchange_equipment_item(size_t index1,size_t index2) {}
	virtual void equip_item(size_t index_inv,size_t index_equip,int count_inv,int count_eq) {}
	virtual void move_equipment_item(size_t index_inv,size_t index_equip, size_t count){}
	virtual void self_get_property(size_t status_point, const extend_prop &, int attack_degree, int defend_degree, int crit_rate, int crit_damage_bonus, int invisible_degree, int anti_invisible_degree, int penetration, int resilience,int vigour) {}
	virtual void set_status_point(size_t vit, size_t eng, size_t str, size_t agi, size_t remain) {}
	virtual void get_extprop_base() {}
	virtual void get_extprop_move() {}
	virtual void get_extprop_attack() {}
	virtual void get_extprop_defense() {}
	virtual void player_reject_invite(const XID & member) {}
	virtual void leader_invite(const XID & leader,int seq,int pickup_flag) {}
	virtual void join_team(const XID & leader,int pickup_flag) {}
	virtual void member_leave(const XID & leader, const XID& member,int type) {}
	virtual void leave_party(const XID & leader, int type) {}
	virtual void new_member(const XID & member) {}
	virtual void leader_cancel_party(const XID & leader) {}
	virtual void teammate_get_pos(const XID & target,const A3DVECTOR & pos,int tag, bool same_plane) {}
	virtual void send_equipment_info(const XID & target, int cs_index, int sid) {}
	virtual void equipment_info_changed(uint64_t madd,uint64_t mdel,const void * buf, size_t size){} 
	virtual void team_member_pickup(const XID & target,int type, int count) {}
	virtual void npc_greeting(const XID & provider) {}
	virtual void repair_all(size_t cost) {}
	virtual void repair(int where,int index,size_t cost) {}
	virtual void renew() {}
	virtual void spend_money(size_t cost){}
	virtual void get_player_money(size_t amount,size_t ) {}
	virtual void cast_skill(const XID & target, int skill,unsigned short time, unsigned char level) {}
	virtual void cast_rune_skill(const XID & target, int skill,unsigned short time, unsigned char level) {}
	virtual void skill_interrupt(char reason) {}
	virtual void skill_perform() {}
	virtual void stop_skill() {}
	virtual void get_skill_data() {}
	virtual void clear_embedded_chip(unsigned short equip_idx,size_t use_money) {}
	virtual void cost_skill_point(int skill_point) {}
	virtual void learn_skill(int skill, int level) {}
	virtual void takeoff() {}
	virtual void landing() {}
	virtual void flysword_time_capacity(unsigned char where, unsigned char index, int cur_time){}
	virtual void obtain_item(int type,int expire_date, int amount,int slot_amount, int where,int index){}
	virtual void produce_start(int type, int use_time ,int count){}
	virtual void produce_once(int type, int amount,int slot_amount, int where,int index){}
	virtual void produce_end(){}
	virtual void decompose_start(int use_time,int type) {}
	virtual void decompose_end() {}
	virtual void get_task_data() {}
	virtual void send_task_var_data(const void * buf, size_t size) {}
	virtual void start_use_item(int item_type, int use_time) {}
	virtual void cancel_use_item() {}
	virtual void use_item(char where, unsigned char index , int item_type , unsigned short count){}	//给自己
	virtual void use_item(int item_type){} //给他人
	virtual void use_item(char where, unsigned char index , int item_type , unsigned short count,const char * arg, size_t arg_size){}//给自己
	virtual void use_item(int item_type,const char * arg, size_t arg_size){} //给他人
	virtual void start_use_item_with_target(int item_type, int use_time,const XID & target) {}

	virtual void sit_down() {}
	virtual void stand_up() {}
	virtual void do_emote(unsigned short emotion) {}
	virtual void do_emote_restore(unsigned short emotion) {}
	virtual void do_action(unsigned char action) {}
	virtual void send_timestamp() {}
	virtual void notify_root(unsigned char type) {}
	virtual void dispel_root(unsigned char type) {}
	virtual void invader_rise(){}
	virtual void pariah_rise(){}
	virtual void invader_fade(){}
	virtual void update_visible_state(unsigned int newstate, unsigned int newstate2, unsigned int newstate3, unsigned int newstate4, unsigned int newstate5, unsigned int newstate6);
	virtual void gather_start(int mine, unsigned char t) {}
	virtual void gather_stop() {}
	virtual void trashbox_passwd_changed(bool has_passwd) {}
	virtual void trashbox_passwd_state(bool has_passwd) {}
	virtual void trashbox_open(char is_usertrashbox) {}
	virtual void trashbox_close(char is_usertrashbox) {}
	virtual void trashbox_wealth(char is_usertrashbox, size_t money) {}
	virtual void exchange_trashbox_item(int where, size_t idx1, size_t idx2) {}
	virtual void move_trashbox_item(int where , size_t src, size_t dest, size_t delta) {}
	virtual void exchange_trashbox_inventory(int where, size_t idx_tra,size_t idx_inv) {}
	virtual void trash_item_to_inventory(int where, size_t idx_tra, size_t idx_inv, size_t delta) {}
	virtual void inventory_item_to_trash(int where, size_t idx_inv, size_t idx_tra, size_t delta) {}
	virtual void exchange_trash_money(char is_usertrashbox, int inv_money, int tra_money) {}
	virtual void enchant_result(const XID & caster, int skill, char level,bool invader, int at_state,unsigned char section);
	virtual void set_adv_data(int data1,int data2){}
	virtual void clear_adv_data(){}
	virtual void player_in_team(unsigned char state) {}
	virtual void send_party_apply(int id) {}
	virtual void query_info_1(int uid,int cs_index, int cs_sid) {}
	virtual void concurrent_emote_request(int id, unsigned short action) {}
	virtual void do_concurrent_emote(int id, unsigned short action) {}
	virtual void mafia_info_notify() {}
	virtual void task_deliver_reputaion(int offset,int cur_reputaion) {} 
	virtual void task_deliver_exp(int exp, int sp) {}
	virtual void task_deliver_money(size_t money,size_t cur_money) {}
	virtual void task_deliver_item(int type, int expire_date,int amount,int slot_amount, int where,int index){}
	virtual void task_deliver_level2(int level) {}
	virtual void get_reputation(int reputation) {}
	virtual void identify_result(char index, char result){}
	virtual void change_shape(char shape){}
	virtual void elf_refine_activate(char status){}	//lgc
	virtual void enter_sanctuary(){}
	virtual void leave_sanctuary(){}
	virtual void begin_personal_market(int market_id,const char * name,size_t len) {}
	virtual void cancel_personal_market() {}
	virtual void market_trade_success(int trader) {}
	virtual void send_market_name(const XID & target, int cs_index, int sid,const char * name ,size_t len) {}
	virtual void player_start_travel(int  line_no,const A3DVECTOR & dest_pos,float speed,int vehicle){}
	virtual void player_complete_travel(int vehicle) {}
	virtual void gm_toggle_invisible(char tmp) {}
	virtual void toggle_invincible(char tmp) {}
	virtual void trace_cur_pos(unsigned short seq) {}
	virtual void cast_instant_skill(const XID & target, int skill,unsigned char level) {}
	virtual void cast_rune_instant_skill(const XID & target, int skill,unsigned char level) {}
	virtual void cast_elf_skill(const XID & target, int skill,unsigned char level) {}	//lgc
	virtual void activate_waypoint(unsigned short waypoint){}
	virtual void player_waypoint_list(const unsigned short * buf, size_t count){}
	virtual void unlock_inventory_slot(unsigned char where, unsigned short index) {}
	virtual void team_invite_timeout(int who) {}
	virtual void enable_pvp_state(char type) {}
	virtual void disable_pvp_state(char type) {}
	virtual void player_pvp_cooldown(int cooldown) {}
	virtual void send_cooldown_data(){}
	virtual void skill_ability_notify(int id, int ability){}
	virtual void personal_market_available() {}
	virtual void breath_data(int breath, int breath_capacity) {}
	virtual void stop_dive() {}
	virtual void trade_away_item(int buyer,short inv_idx,int type, size_t count) {}
	virtual void player_enable_fashion_mode(char is_enable) {}
	virtual void player_enable_free_pvp(char is_enable){}
	virtual void player_enable_effect(short effect) {}
	virtual void player_disable_effect(short effect) {}
	virtual void enable_resurrect_state(float exp_reduce) {}
	virtual void set_cooldown(int idx, int cooldown){}
	virtual void change_team_leader(const XID & old_leader, const XID & new_leader) {}
	virtual void kickout_instance(char reason, int time_out) {}
	virtual void begin_cosmetic(unsigned short inv_index) {}
	virtual void end_cosmetic(unsigned short inv_index) {}
	virtual void cosmetic_success(unsigned short crc) {}
	virtual void cast_pos_skill(const A3DVECTOR & pos, const XID &target, int skill, unsigned short time, unsigned char level){}
	virtual void change_move_seq(unsigned short seq) {}
	virtual void server_config_data() {}
	virtual void active_rush_mode(char is_active) {}
	virtual void produce_null(int recipe_id) {}
	virtual void enable_double_exp_time(int mode, int end_time) {}
	virtual void available_double_exp_time() {}
	virtual void active_pvp_combat_state(bool is_active) {}
	virtual void duel_recv_request(const XID &target) {}
	virtual void duel_reject_request(const XID &target,int reason) {}
	virtual void duel_prepare(const XID & target, int delay) {}
	virtual void duel_cancel(const XID & target) {}
	virtual void duel_start(const XID & who) {}
	virtual void duel_stop() {}
	virtual void duel_result(const XID & target, bool is_failed) {}
	virtual void player_bind_request(const XID & target) {}
	virtual void player_bind_invite(const XID & target) {}
	virtual void player_bind_request_reply(const XID & target, int param) {}
	virtual void player_bind_invite_reply(const XID & target, int param) {}
	virtual void player_bind_start(const XID & target) {}
	virtual void player_bind_stop() {}
	virtual void player_mounting(int mount_id, unsigned short mount_color) {}
	virtual void send_equip_detail(int cs_indx, int cs_sid, int target, const void * data, size_t size){}
	virtual void send_inventory_detail(int cs_indx, int cs_sid, int target, size_t money, unsigned char inv_size, const void * data, size_t size){}
	virtual void send_others_property(const void * data, size_t size, const void * self_data, size_t self_size){}
	virtual void pariah_duration(int remain_time){}
	virtual void gain_pet(int index, const void * buf, size_t size){}
	virtual void free_pet(int index, int pet_id){}
	virtual void summon_pet(int index, int pet_tid/*模板ID*/, int pet_id/*世界ID*/, int life_time) {}
	virtual void start_pet_operation(int index, int pet_id,int delay,int operation) {}
	virtual void end_pet_operation() {}
	virtual void recall_pet(int index, int pet_id, char reason) {}
	virtual void pet_recv_exp(int index, int pet_id, int exp) {}
	virtual void pet_level_up(int index, int pet_id, int level, int cur_exp) {}
	virtual void pet_room_capacity(int cap) {}
	virtual void notify_pet_honor(int index,int honor) {}
	virtual void notify_pet_hunger(int index, int hunger) {}
	virtual void enter_battleground(int role, int battle_id,int end_time){}
	virtual void send_turrent_leader(int id) {}
	virtual void battle_result(int result) {}
	virtual void battle_score(int oscore, int ogoal, int dscore, int dgoal) {}
	virtual void pet_dead(int index) {}
	virtual void pet_revive(int index,float hp_factor) {}
	virtual void pet_hp_notify(int index, float hp_factor, int cur_hp, float mp_factor, int cur_mp) {}
	virtual void pet_ai_state(char aggro_state, char stay_state) {}
	virtual void refine_result(int rst) {}
	virtual void pet_set_cooldown(int index, int cd_index, int msec) {}
	virtual void player_cash(int cash) {}
	virtual void player_bind_success(size_t index, int id) {}
	virtual void player_change_inventory_size(int size) {}
	virtual void player_pvp_mode() {}
	virtual void player_wallow_info(int level, int ptime, int light_t, int heavy_t, int reason) {}
	virtual void player_change_spouse(int id) {}
	virtual void notify_safe_lock(char active, int time, int max_time) {}
	virtual void mall_item_buy_failed(short index, char reason){}//lgc
	virtual void equip_trashbox_item(int where, unsigned char trash_idx,unsigned char equip_idx){}
	virtual void security_passwd_checked(){}
	virtual void toggle_invisible(int invisible_degree);
	virtual void appear_to_spec(int invi_degree){}
	virtual void disappear_to_spec(int invi_degree){}
	virtual void on_inc_invisible(int prev_invi_degree, int cur_invi_degree){}
	virtual void on_dec_invisible(int prev_invi_degree, int cur_invi_degree){}
	virtual void on_inc_anti_invisible(int prev_a_invi_degree, int cur_a_invi_degree){}
	virtual void on_dec_anti_invisible(int prev_a_invi_degree, int cur_a_invi_degree){}
	virtual void hp_steal(int hp){}
	virtual void player_dividend(int cash) {}
	virtual void dividend_mall_item_buy_failed(short index, char reason){}
	virtual void multi_exp_info(int last_timestamp, float enhance_factor){}
	virtual void change_multi_exp_state(char state, int enchance_time, int buffer_time, int impair_time, int activate_times_left){}
	virtual void send_world_life_time(int life_time){}
	virtual void wedding_book_success(int type){}
	virtual void calc_network_delay(int timestamp){}
	virtual void player_knockback(const A3DVECTOR & pos, int time){}
	virtual void summon_plant_pet(int plant_tid/*模板ID*/, int plant_id/*世界ID*/, int life_time) {}
	virtual void plant_pet_disappear(int id, char reason) {}
	virtual void plant_pet_hp_notify(int id, float hp_factor, int cur_hp, float mp_factor, int cur_mp) {}
	virtual void pet_property(int index, const extend_prop & prop){}
	virtual void faction_contrib_notify(){}
	virtual void faction_relation_notify(){}
	virtual void enter_factionfortress(int role_in_war, int factionid, int offense_endtime){}
	virtual void player_equip_disabled(int64_t mask){}
	virtual void send_spec_item_list(int cs_index, int cs_sid, int target, int type, void * data, size_t size){}
	virtual void send_error_message(int cs_index, int cs_sid, int target, int message){}
	virtual void start_play_action(char action_name[128],int play_times,int action_last_time,int interval_time){}
	virtual void stop_play_action(){}
	virtual void congregate_request(char type, int sponsor, int timeout){}
	virtual void reject_congregate(char type, int id){}
	virtual void congregate_start(char type, int time){} 
	virtual void cancel_congregate(char type){} 
	virtual void engrave_start(int recipe_id, int use_time){}
	virtual void engrave_end(){}
	virtual void engrave_result(int addon_num){}
	virtual void addonregen_start(int recipe_id, int use_time){}
	virtual void addonregen_end(){}
	virtual void addonregen_result(int addon_num){}
	virtual void invisible_obj_list(gobject ** ppObject, size_t count){}
	virtual void set_player_limit(int index, char b){}
	virtual void player_teleport(const A3DVECTOR& pos, unsigned short use_time, char mode){}
	virtual void forbid_be_selected(char b){}
	virtual void send_player_force_data(int cur_force, size_t count, const void * data, size_t data_size){}
	virtual void player_force_changed(int force){}
	virtual void player_force_data_update(int force, int repu, int contri){}
	virtual void send_force_global_data(char data_ready, size_t count, const void * data, size_t data_size){}
	virtual void add_multiobj_effect(int target, char type){}
	virtual void remove_multiobj_effect(int target, char type){}
	virtual void enter_wedding_scene(int groom, int bride){}
	virtual void produce4_item_info(int stime, item_data & data, unsigned short crc){} //发送客户端新继承生产产生的物品信息
	virtual void online_award_data(int total_award_time, size_t count, const void * data, size_t data_size){}
	virtual void toggle_online_award(int type, char activate){}
	virtual void update_profit_time(char flag, int profit_time, int profit_level){}
	virtual void notify_profit_state(char state) {}//通知收益状态;开启或者关闭
	virtual void enter_nonpenalty_pvp_state(char state){}
	virtual void self_country_notify(int country_id){}
	virtual void player_country_changed(int country_id){}
	virtual void enter_countrybattle(int role, int battle_id,int end_time,int offense_country, int defence_country){}
	virtual void countrybattle_result(int result) {}
	virtual void countrybattle_score(int oscore, int ogoal, int dscore, int dgoal) {}
	virtual void countrybattle_resurrect_rest_times(int times){}
	virtual void countrybattle_became_flag_carrier(char is_carrier){}
	virtual void countrybattle_personal_score(int combat_time, int attend_time, int kill_count, int death_count, int country_kill_count, int country_death_count){}
	virtual void defense_rune_enabled(char rune_type, char enable){}
	virtual void countrybattle_info(int attacker_count, int defender_count){}
	virtual void cash_money_exchg_rate(char open, int rate){}
	virtual void pet_rebuild_inherit_start(size_t index,int use_time) {}
	virtual void pet_rebuild_inherit_info(int stime,int pet_id,size_t index,int r_attack,int r_defense,int r_hp,int r_atk_lvl,int r_def_lvl) {} 
	virtual void pet_rebuild_inherit_end(size_t index) {}
	virtual void pet_evolution_done(size_t index){}
	virtual void pet_rebuild_nature_start(size_t index,int use_time) {}
	virtual void pet_rebuild_nature_info(int stime,int pet_id,size_t index,int nature) {}
	virtual void pet_rebuild_nature_end(size_t index) {}
	virtual void equip_addon_update_notify(unsigned char update_type,unsigned char equip_idx,unsigned char equip_socket_idx,int old_stone_type,int new_stone_type) {}
	virtual void notify_meridian_data(int meridian_level,int lifegate_times,int deathgate_times,int free_refine_times,int paid_refine_times,int continu_login_days,int trigrams_map[3]) {}
	virtual void try_refine_meridian_result(int index,int result) {}
	virtual void self_king_notify(char is_king, int expire_time){}
	virtual void player_king_changed(char is_king){}
	virtual void notify_touch_query(int64_t income,int64_t remain,int retcode) {}
	virtual void notify_touch_cost(int64_t income,int64_t remain,unsigned int cost,unsigned int index,unsigned int lots,int retcode) {}
	virtual void notify_addup_money(int64_t addupmoney) {}
	virtual void notify_giftcard_redeem(int retcode,int cardtype,int parenttype, const char(&cardnumber)[20]) {}
	virtual void query_title(int roleid,int count,int ecount,const void * data, size_t data_size,const void * edata, size_t edata_size){}
	virtual void notify_curr_title_change(int roleid,unsigned short titleid){}
	virtual void notify_title_modify(unsigned short titleid,int expiretime,char flag){}
	virtual void refresh_signin(char type,int moncal,int cys,int lys, int uptime, int localtime, char awardedtimes, char latesignintimes) {}
	virtual void player_reincarnation(size_t reincarnation_times){}
	virtual void activate_reincarnation_tome(char active){}
	virtual void realm_exp_receive(int exp,int receive_exp){}
	virtual void realm_level_up(unsigned char level){}
	virtual void enter_trickbattle(int role, int battle_id,int end_time){}
	virtual void trickbattle_personal_score(int kill, int death, int score, int multi_kill){}
	virtual void trickbattle_chariot_info(int chariot, int energy){}
	virtual void player_leadership(int leadership, int inc_leadership){}
	virtual void generalcard_collection_data(const void * buf, size_t size){}
	virtual void add_generalcard_collection(size_t collection_idx){}
	virtual void refresh_fatering(const void * data,size_t datasize) {}
	virtual void broadcast_mine_gatherd(int mid, int pid, int item_type) {}
	virtual void player_active_combat(bool is_active) {}
	virtual void random_mall_shopping_result(int eid,int opt,int res,int item,int price,bool flag) {}
	virtual void player_mafia_pvp_mask_notify(unsigned char mafia_pvp_mask) {}
	virtual void player_world_contribution(int world_contrib,int change, int total_cost) {}
	virtual void send_scene_service_npc_list(unsigned int count, int * data) {}
	virtual void notify_visible_tid_change() {}
	virtual void player_screen_effect_notify(int type, int eid, int state) {}
	virtual void player_combo_skill_prepare(int skillid,int timestamp,int arg1, int arg2, int arg3) {}
	virtual void player_pray_distance_change(float pd) {}
	virtual void instance_reenter_notify(int tag, int timeout) {}

public:
	void MoveBetweenSlice(slice * pPiece,slice * pNewPiece,const A3DVECTOR &pos);
};


class gobject_imp : public substance
{
protected:

public:
DECLARE_SUBSTANCE(gobject_imp);
public:
	world * _plane;
	gobject * _parent;

	controller * _commander;
	dispatcher * _runner;
public:
	gobject_imp():_plane(NULL),_parent(NULL),_commander(0),_runner(0){}
	virtual void Init(world * pPlane,gobject*parent)
	{
		_plane = pPlane;
		_parent = parent;
	}
	virtual void ResetPlane(world * new_pPlane)
	{
		_plane = new_pPlane;
	}
	virtual ~gobject_imp(){}
	/**
	 *	调用这个函数时，_parent已经被锁定所以再次发送send_message和post_message时要注意先开锁,
	 *	返回值非零表示不需要再次告知外面不需要再开锁
	 *	即正常情况下(未开锁)一定要返回0
	 */
	virtual  int MessageHandler(world * pPlane ,const MSG & msg)
	{
		return 0;
	}
	
	typedef int (*msg_filter)(world * pPlane, const MSG & msg);
	/*
	 * 	 设置可能的消息过滤器，一个对象同时只能有一个消息过滤器
	 * 	 这个函数被设计为应该被controller对象调用
	 */
	msg_filter SetMsgFilter(msg_filter filter);
	

	/**
	 *	进行移动的函数，会进行比较复杂的操作，设置时钟进行移动判断。
	 */
	virtual int PrepareMove(const A3DVECTOR &dest) {return 0;};	//处理移动命令，会加入定时器，用来处理移动


	/**
	 *	重新复生，清除所有不利状态，
	 *	从死亡状态回复
	 */
	virtual void Reborn()
	{
		ASSERT(false);
	}

	/*
	 *	测试是否可以开始交易，只有player 可以交易
	 */
	virtual bool CanTrade(const XID & target)
	{
		return false;
	}

	/**
	 *	开始交易，只有player 可以完成这个操作
	 *
	 */
	virtual void StartTrade(int trade_id,const XID & target)
	{
		return ;
	}

	/*
	 *	开始帮派交易(帮派服务),	
	 */
	 virtual bool StartFactionTrade(int trade_id,int get_mask, int put_mask,bool no_response = false)
	 {
	 	ASSERT(false);
	 	return false;
	 }

	 /*
	  *	帮派交易完成，同时进行回写操作 参数是随便写的现在
	  */
	 virtual void FactionTradeComplete(int trade_id, const GNET::syncdata_t &data)
	 {
	 	ASSERT(false);
	 	return ;
	 }
	 
	 /*
	 	交易完成，同时进行回写操作
	 */
	 virtual void SyncTradeComplete(int trade_id, size_t money, const GDB::itemlist & item_change,bool write_trashbox, bool is_write_shoplog)
	 {
	 	ASSERT(false);
		return;
	 }

	/**
	 *	交易数据回写成功或者失败(在等待交易阶段调用)
	 */
	virtual void WaitingTradeWriteBack(int trade_id, bool bSuccess)
	{
		return;
	}

	/*
	 *	
	 *	交易服务器发来了交易完成的消息 （成功或者失败）
	 */
	virtual void TradeComplete(int trade_id,int reason, bool need_read){}

	
	/*
	*	取消切换服务器的操作
	*/
	virtual void CancelSwitch(){}

	/**
		更新帮派信息
	*/

	virtual void UpdateMafiaPvP(unsigned char pvp_mask)
	{
		//do nothing
	}

	virtual void UpdateMafiaInfo(int id, char rank, unsigned char pvp_mask)
	{
		//do nothing
	}

	virtual void UpdateFactionRelation(int * alliance, size_t asize, int * hostile, size_t hsize, bool notify_client)
	{
		//do nothing
	}
	
	/*
	*	玩家强制下线的操作
	*/
	virtual void PlayerForceOffline(){}

	/*
	*	关闭的操作
	*/
	virtual void ServerShutDown(){}

	virtual bool CanTeamRelation() { return false;}
	
	/**
	 * 取得基本数据
	 */
	template <typename WRAPPER>
	void GetBaseInfo(WRAPPER &wrapper)
	{
		wrapper<<_parent->ID.id<<_parent->pos;
	}
public:
	/**
	 *	分发命令到控制器，要求将本对象上锁后再调用
	 *	和 消息处理类似，如果返回了非0，那么表示外面不需要解锁
	 */
	virtual int DispatchCommand(int cmd_type, const void * buf,size_t size)
	{
		ASSERT(_commander);
		return _commander->CommandHandler(cmd_type,buf,size);
	}

	virtual int DispatchMessage(world * pPlane ,const MSG & msg)
	{
		return MessageHandler(pPlane,msg);
	}

	void EnterWorld()
	{
		_runner->begin_transfer();
		_runner->get_base_info();
		_runner->query_info00();
		_runner->enter_world();
		_runner->end_transfer();
	}

	virtual void CheckNPCData() {}

	virtual void OnNpcEnterWorld(){}
	virtual void OnNpcLeaveWorld(){}
	
//protected:
public:
	/*
	 * 真实移动对象，移动一个较小的量，移动的量如果过大的话
	 * 会丢失给原来所在区域的离开信息，以后可以考虑加上此判断
	 * 返回值表示是否进行了移动
	 */
	virtual bool StepMove(const A3DVECTOR &offset);

	/*
	 * 这个函数判定对象是否可以发生移动，可以不调用这个函数而直接调用StepMove
	 * 这个函数在于不进行真正的移动而判断是否可以移动
	 */
	virtual bool CanMove() { return true; }

	/*
	 * 进行服务器的切换
	 */
	virtual void SwitchSvr(int dest,const A3DVECTOR &oldpos , const A3DVECTOR &newpos,const instance_key *)
	{
		ASSERT(false);
	}
	friend class gobject;

};

gobject_imp * 	CF_CreateImp(int guid,world * plane,gobject * obj);
dispatcher * 	CF_CreateDispatcher(int guid,gobject_imp*);
controller*	CF_CreateCtrl(int guid,gobject_imp*);

inline 	gobject_imp * CF_Create(int guid_imp,int guid_dis,int guid_ctrl,world *plane,gobject *obj)
{
	gobject_imp * imp = CF_CreateImp(guid_imp,plane,obj);
	ASSERT(imp);
	if(!imp) return NULL;
	controller *ctrl = CF_CreateCtrl(guid_ctrl,imp);
	dispatcher *runner = CF_CreateDispatcher(guid_dis,imp);
	ASSERT(ctrl && runner);
	if(!ctrl || !runner)
	{
		delete ctrl;
		delete runner;
		delete imp;
		return NULL;
	}
	imp->_runner = runner;
	imp->_commander = ctrl;
	return imp;
}

#endif
