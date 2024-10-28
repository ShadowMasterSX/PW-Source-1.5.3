/*
 * 	处理连接服务器来的命令，主要是用户客户端发来的命令
 */
#ifndef __ONLINEGAME_GS_NETMSG_H__
#define __ONLINEGAME_GS_NETMSG_H__

#include <octets.h>
#include <common/packetwrapper.h>
#include <common/protocol_imp.h>
#include <sys/uio.h>
#include <hashmap.h>

#include <db_if.h>
#include <gsp_if.h>
#include "slice.h"

void handle_user_cmd(int cs_index,int sid,int user_id,const void * buf, size_t size);
void handle_user_msg(int cs_index,int sid, int uid, const void * msg, size_t size, const void * aux_data, size_t size2, char channel);
size_t handle_chatdata(int uid, const void * aux_data, size_t size, void * buffer, size_t len);
void trade_end(int trade_id, int role1,int role2,bool need_read1,bool need_read2);
void trade_start(int trade_id, int role1,int role2, int localid1,int localid2);
void psvr_ongame_notify(int * start , size_t size,size_t step);
void psvr_offline_notify(int * start , size_t size,size_t step);
void get_task_data_reply(int taskid, int uid, const void * env_data, size_t env_size, const void * task_data, size_t task_size);
void get_task_data_timeout(int taskid, int uid, const void * env_data, size_t env_size);

void user_kickout(int cs_index,int sid,int uid);
void user_lost_connection(int cs_index,int sid,int uid);

void faction_trade_lock(int trade_id,int roleid,int localsid);
void gm_shutdown_server();
void player_cosmetic_result(int user_id, int ticket_id, int result, unsigned int crc);
void battleground_start(int battle_id, int attacker, int defender,int end_time, int type, int map_type);
void player_enter_battleground(int role_id, int server_id, int world_tag, int battle_id);
void OnDeliveryConnected();
bool gm_control_command(int target_tag, const char * cmd);
void player_cash_notify(int role, int cash_plus_used);
void player_add_cash_notify(int role);	//通知元宝数已变，需要重新向数据库获取
void player_dividend_notify(int role, int dividend);
bool query_player_level(int roleid, int & level, int & reputation);
bool generate_item_for_delivery(int id, GDB::itemdata & data);
void player_enter_leave_gt(int op,int roleid);

//帮派基地
bool get_faction_fortress_create_cost(int* cost, size_t& size);
bool get_faction_fortress_initial_value(int* technology, size_t& tsize, int* material, size_t& msize, int* building, size_t& bsize);
bool notify_faction_fortress_data(GNET::faction_fortress_data2 * data2);
void player_enter_faction_fortress(int role_id, int dst_world_tag, int dst_factionid);

void RecvFactionCongregateRequest(int factionid, int roleid, int sponsor, void * data, size_t size);
void UpdateForceGlobalData(int force_id, int player_count, int development, int construction, int activity, int activity_level);

bool player_join_country(int role_id, int country_id, int country_expiretime, int major_strength, int minor_strength, int world_tag, float posx, float posy, float posz);
void notify_country_battle_config(GMSV::CBConfig * config);
void country_battle_start(int battle_id, int attacker, int defender, int player_limit, int end_time, int attacker_total, int defender_total, int max_total);
void player_enter_country_battle(int role_id, int world_tag, int battle_id);
void player_country_territory_move(int role_id, int world_tag, float posx, float posy, float posz, bool capital);
void thread_usage_stat(const char * ident);
bool query_player_info(int roleid, char * name, size_t& name_len, int& level, int& sec_level, int& reputation, int& create_time, int& factionid, int itemid1, int& itemcount1, int itemid2, int& itemcount2, int itemid3, int& itemcount3);

void player_safe_lock_changed(int roleid, int locktime, int max_locktime);
void player_change_ds(int roleid, int flag);
void notify_cash_money_exchange_rate(bool open, int rate);
void king_notify(int roleid, int end_time);

void OnTouchPointQuery(int roleid,int64_t income,int64_t remain);
void OnTouchPointCost(int roleid,int64_t orderid,unsigned int cost,int64_t income,int64_t remain,int retcode);
void OnAuAddupMoneyQuery(int roleid,int64_t addupmoney);
void OnGiftCodeRedeem(int roleid,void* cardnumber,size_t cnsz,int type,int parenttype,int retcode);

void trick_battle_start(int battle_id, int player_limit, int end_time);
void player_enter_trick_battle(int roleid, int world_tag, int battle_id, int chariot);

void notify_serverforbid(std::vector<int> &ctrl_list,std::vector<int> &item_list,std::vector<int> &service_list,std::vector<int> &task_list,std::vector<int> &skill_list, std::vector<int> &shopitem_list, std::vector<int>& recipe_list);

void notify_mafia_pvp_status(int status,std::vector<int> &ctrl_list);
void request_mafia_pvp_elements(unsigned int version);

void mnfaction_battle_start(int domain_id, unsigned char domain_type, int64_t attacker_faction_id, int64_t defender_faction_id, int end_timestamp);
bool player_join_mnfaction(int retcode, int role, int64_t faction_id, int world_tag, int domain_id);

#endif

