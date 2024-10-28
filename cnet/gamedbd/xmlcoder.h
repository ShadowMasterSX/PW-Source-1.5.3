#ifndef __XMLCODER_H
#define __XMLCODER_H

#include <string>
#include <vector>
#include <set>
#include "rpcdefs.h"
#include "pshopdetail"
#include "guserstorehouse"
#include "gfactionrelationapply"
#include "gterritorydetail"
#include "gauctiondetail"
#include "grolebase"
#include "gfactionhostile"
#include "gmailheader"
#include "gfactioninfo"
#include "gwebtradedetail"
#include "mappassworddata"
#include "gfriendextinfo"
#include "kingelectiondetail"
#include "gfriendinfo"
#include "gsendaumailrecord"
#include "playerprofiledata"
#include "gfriendlist"
#include "gmail"
#include "mndomaininfo"
#include "mappasswordvalue"
#include "gfactionfortressdetail"
#include "gwebtradeitem"
#include "stocklog"
#include "gfactionrelation"
#include "pshopitem"
#include "gforceglobaldatalist"
#include "gforceglobaldata"
#include "user"
#include "grolestorehouse"
#include "gmember"
#include "guniquedataelem"
#include "groletask"
#include "mnfactioninfo"
#include "kecandidate"
#include "message"
#include "gterritorystore"
#include "gserverdata"
#include "gfactionfortressinfo2"
#include "gtabledefinition"
#include "stockorder"
#include "mndomainbonus"
#include "grolepocket"
#include "grolestatus"
#include "gsyslog"
#include "weborderitemdetail"
#include "groledata"
#include "solochallengerankdata"
#include "solochallengerankdataext"
#include "gglobalcontroldata"
#include "gfactionfortressinfo"
#include "groleequipment"
#include "groleforbid"
#include "gmailbox"
#include "groleinventory"
#include "groletableclsconfig"
#include "ggroupinfo"
#include "mndomainbonusrewarditem"
#include "mnfactionapplyinfo"
#include "keking"
#include "gpair"
#include "gauctionitem"
#include "gfactionalliance"
#include "gfriendextra"
#include "guserfaction"

namespace GNET
{

class XmlCoder
{
protected:
	std::string  data;
	Octets       buffer;
	std::set<unsigned short> entities; 
public:
	XmlCoder() : buffer(32) 
	{ 
		entities.insert(0);
		entities.insert(34);
		entities.insert(38);
		entities.insert(39);
		entities.insert(60);
		entities.insert(62);
	}
	const char * c_str() { return data.c_str(); }

	void append_header()
	{
		data = "<?xml version=\"1.0\"?>\n";
	}
	void append_variable(const char* name, const char* type, const std::string& value)
	{
		data = data + "<variable name=\"" + name + "\" type=\"" + type + "\">" + value + "</variable>\n";
	}
	const std::string toString(char x)
	{
		sprintf((char*)buffer.begin(), "%d", (int)x);
		return (char*)buffer.begin();
	}
	const std::string toString(short x)
	{
		sprintf((char*)buffer.begin(), "%d", (int)x);
		return (char*)buffer.begin();
	}
	const std::string toString(int x)
	{
		sprintf((char*)buffer.begin(), "%d", x);
		return (char*)buffer.begin();
	}
	const std::string toString(float x)
	{
		sprintf((char*)buffer.begin(), "%.9g", x);
		return (char*)buffer.begin();
	}
	const std::string toString(int64_t x)
	{
		sprintf((char*)buffer.begin(), "%lld", x);
		return (char*)buffer.begin();
	}
	const std::string toString(const Octets& x)
	{
		buffer.resize(x.size()*2+1);
		unsigned char* p = (unsigned char*)x.begin();
		char* out = (char*)buffer.begin();
		*out = 0;
		for(size_t i=0;i<x.size();++i,out+=2)
		{
			sprintf(out,"%02x", p[i]);
		}
		return (char*)buffer.begin();
	}
	void append_string(const char* name, const Octets& x)
	{
		std::string result;
		const unsigned short *p = (const unsigned short*)x.begin();
		for(size_t len = x.size()/2;len>0;len--,p++)
		{
			unsigned short c = *p;
			if(c>0x7F || entities.find(c)!=entities.end()) 
				result.append("&#" + toString(c) + ";");
			else
				result += (char)c;
		}
		append_variable(name,"Octets",result);
	}
	void append(const char* name, char x)
	{
		append_variable(name, "byte", toString(x));
	}
	void append(const char* name, short x)
	{
		append_variable(name, "short", toString(x));
	}
	void append(const char* name, int x)
	{
		append_variable(name, "int", toString(x));
	}
	void append(const char* name, float x)
	{
		append_variable(name, "float", toString(x));
	}
	void append(const char* name, int64_t x)
	{
		append_variable(name, "long", toString(x));
	}
	void append(const char* name, const Octets& x)
	{
		append_variable(name, "Octets", toString(x));
	}
	template<typename T>
	void append(const char* name, const std::vector<T> &x) 
	{
		typedef const std::vector<T> VECTOR;
		for( typename VECTOR::const_iterator i=x.begin(),e=x.end();i!=e;++i)
			append(name, *i);
	}
	template<typename T>
	void append(const char* name, const std::list<T> &x) 
	{
		typedef const std::list<T> LIST;
		for( typename LIST::const_iterator i=x.begin(),e=x.end();i!=e;++i)
			append(name, *i);
	}
	template<typename T>
	void append(const char* name, const GNET::RpcDataVector<T> &x) 
	{
		typedef const GNET::RpcDataVector<T> VECTOR;
		for(typename VECTOR::const_iterator i=x.begin(),e=x.end();i!=e;++i)
			append(name, *i);
	}
	template<typename T>
	void append(const char* name, const std::set<T> &x)
	{
		for (typename std::set<T>::const_iterator i=x.begin(),e=x.end();i!=e;++i)
			append(name, *i);
	}
	void append(const char* name, const PShopDetail& x)
	{
		data = data + "<" + name + ">\n";
		append("roleid",  x.roleid);
		append("shoptype",  x.shoptype);
		append("status",  x.status);
		append("createtime",  x.createtime);
		append("expiretime",  x.expiretime);
		append("money", (int) x.money);
		append("yinpiao",  x.yinpiao);
		append("blist",  x.blist);
		append("slist",  x.slist);
		append("store",  x.store);
		append("reserved1",  x.reserved1);
		append("reserved2",  x.reserved2);
		append("reserved3",  x.reserved3);
		append("reserved4",  x.reserved4);
		data = data + "</" + name + ">\n";
	}
	void append(const char* name, const GUserStorehouse& x)
	{
		data = data + "<" + name + ">\n";
		append("capacity",  x.capacity);
		append("money", (int) x.money);
		append("items",  x.items);
		append("reserved1",  x.reserved1);
		append("reserved2",  x.reserved2);
		append("reserved3",  x.reserved3);
		append("reserved4",  x.reserved4);
		data = data + "</" + name + ">\n";
	}
	void append(const char* name, const GFactionRelationApply& x)
	{
		data = data + "<" + name + ">\n";
		append("type",  x.type);
		append("fid",  x.fid);
		append("end_time",  x.end_time);
		data = data + "</" + name + ">\n";
	}
	void append(const char* name, const GTerritoryDetail& x)
	{
		data = data + "<" + name + ">\n";
		append("id",  x.id);
		append("level",  x.level);
		append("owner", (int) x.owner);
		append("occupy_time",  x.occupy_time);
		append("challenger", (int) x.challenger);
		append("deposit", (int) x.deposit);
		append("cutoff_time",  x.cutoff_time);
		append("battle_time",  x.battle_time);
		append("bonus_time",  x.bonus_time);
		append("color",  x.color);
		append("status",  x.status);
		append("timeout",  x.timeout);
		append("maxbonus",  x.maxbonus);
		append("challenge_time",  x.challenge_time);
		append("challengerdetails",  x.challengerdetails);
		append("reserved1",  x.reserved1);
		append("reserved2",  x.reserved2);
		append("reserved3",  x.reserved3);
		data = data + "</" + name + ">\n";
	}
	void append(const char* name, const GAuctionDetail& x)
	{
		data = data + "<" + name + ">\n";
		append("info",  x.info);
		append("category", (short) x.category);
		append("baseprice", (int) x.baseprice);
		append("deposit", (int) x.deposit);
		append("elapse_time",  x.elapse_time);
		append("prolong",  x.prolong);
		append("reserved1",  x.reserved1);
		append("reserved2",  x.reserved2);
		append("reserved3",  x.reserved3);
		append("item",  x.item);
		data = data + "</" + name + ">\n";
	}
	void append(const char* name, const GRoleBase& x)
	{
		data = data + "<" + name + ">\n";
		append("version",  x.version);
		append("id", (int) x.id);
		append_string("name",  x.name);
		append("race",  x.race);
		append("cls",  x.cls);
		append("gender", (char) x.gender);
		append("custom_data",  x.custom_data);
		append("config_data",  x.config_data);
		append("custom_stamp", (int) x.custom_stamp);
		append("status", (char) x.status);
		append("delete_time",  x.delete_time);
		append("create_time",  x.create_time);
		append("lastlogin_time",  x.lastlogin_time);
		append("forbid",  x.forbid);
		append("help_states",  x.help_states);
		append("spouse", (int) x.spouse);
		append("userid", (int) x.userid);
		append("cross_data",  x.cross_data);
		append("reserved2", (char) x.reserved2);
		append("reserved3", (char) x.reserved3);
		append("reserved4", (char) x.reserved4);
		data = data + "</" + name + ">\n";
	}
	void append(const char* name, const GFactionHostile& x)
	{
		data = data + "<" + name + ">\n";
		append("fid",  x.fid);
		append("end_time",  x.end_time);
		data = data + "</" + name + ">\n";
	}
	void append(const char* name, const GMailHeader& x)
	{
		data = data + "<" + name + ">\n";
		append("id", (char) x.id);
		append("sender",  x.sender);
		append("sndr_type", (char) x.sndr_type);
		append("receiver",  x.receiver);
		append("title",  x.title);
		append("send_time",  x.send_time);
		append("attribute", (char) x.attribute);
		append("sender_name",  x.sender_name);
		data = data + "</" + name + ">\n";
	}
	void append(const char* name, const GFactionInfo& x)
	{
		data = data + "<" + name + ">\n";
		append("fid", (int) x.fid);
		append_string("name",  x.name);
		append("level",  x.level);
		append("master",  x.master);
		append("member",  x.member);
		append("announce",  x.announce);
		append("extenddata",  x.extenddata);
		data = data + "</" + name + ">\n";
	}
	void append(const char* name, const GWebTradeDetail& x)
	{
		data = data + "<" + name + ">\n";
		append("info",  x.info);
		append("item",  x.item);
		append("post_time",  x.post_time);
		append("deposit", (int) x.deposit);
		append("loginip",  x.loginip);
		append("rolebrief",  x.rolebrief);
		append("reserved10",  x.reserved10);
		append("reserved11",  x.reserved11);
		append("reserved2",  x.reserved2);
		append("reserved3",  x.reserved3);
		append("reserved4",  x.reserved4);
		append("reserved5",  x.reserved5);
		data = data + "</" + name + ">\n";
	}
	void append(const char* name, const MapPasswordData& x)
	{
		data = data + "<" + name + ">\n";
		append("username",  x.username);
		append("value",  x.value);
		data = data + "</" + name + ">\n";
	}
	void append(const char* name, const GFriendExtInfo& x)
	{
		data = data + "<" + name + ">\n";
		append("uid",  x.uid);
		append("rid",  x.rid);
		append("level",  x.level);
		append("last_logintime",  x.last_logintime);
		append("update_time",  x.update_time);
		append("reincarnation_times", (char) x.reincarnation_times);
		append("remarks",  x.remarks);
		append("reserved1",  x.reserved1);
		append("reserved2",  x.reserved2);
		append("reserved3",  x.reserved3);
		data = data + "</" + name + ">\n";
	}
	void append(const char* name, const KingElectionDetail& x)
	{
		data = data + "<" + name + ">\n";
		append("king",  x.king);
		append("candidate_list",  x.candidate_list);
		append("reserved1",  x.reserved1);
		append("reserved2",  x.reserved2);
		append("reserved3",  x.reserved3);
		append("reserved4",  x.reserved4);
		append("reserved5",  x.reserved5);
		data = data + "</" + name + ">\n";
	}
	void append(const char* name, const GFriendInfo& x)
	{
		data = data + "<" + name + ">\n";
		append("rid",  x.rid);
		append("cls",  x.cls);
		append("gid",  x.gid);
		append_string("name",  x.name);
		data = data + "</" + name + ">\n";
	}
	void append(const char* name, const GSendAUMailRecord& x)
	{
		data = data + "<" + name + ">\n";
		append("rid",  x.rid);
		append("sendmail_time",  x.sendmail_time);
		data = data + "</" + name + ">\n";
	}
	void append(const char* name, const PlayerProfileData& x)
	{
		data = data + "<" + name + ">\n";
		append("game_time_mask", (short) x.game_time_mask);
		append("game_interest_mask", (short) x.game_interest_mask);
		append("personal_interest_mask", (short) x.personal_interest_mask);
		append("age", (char) x.age);
		append("zodiac", (char) x.zodiac);
		append("match_option_mask", (short) x.match_option_mask);
		data = data + "</" + name + ">\n";
	}
	void append(const char* name, const GFriendList& x)
	{
		data = data + "<" + name + ">\n";
		append("groups",  x.groups);
		append("friends",  x.friends);
		data = data + "</" + name + ">\n";
	}
	void append(const char* name, const GMail& x)
	{
		data = data + "<" + name + ">\n";
		append("header",  x.header);
		append("context",  x.context);
		append("attach_obj",  x.attach_obj);
		append("attach_money", (int) x.attach_money);
		data = data + "</" + name + ">\n";
	}
	void append(const char* name, const MNDomainInfo& x)
	{
		data = data + "<" + name + ">\n";
		append("domain_id",  x.domain_id);
		append("domain_type", (char) x.domain_type);
		append("owner_unifid",  x.owner_unifid);
		append("attacker_unifid",  x.attacker_unifid);
		append("defender_unifid",  x.defender_unifid);
		data = data + "</" + name + ">\n";
	}
	void append(const char* name, const MapPasswordValue& x)
	{
		data = data + "<" + name + ">\n";
		append("userid",  x.userid);
		append("algorithm",  x.algorithm);
		append("password",  x.password);
		append("matrix",  x.matrix);
		append("seed",  x.seed);
		append("pin",  x.pin);
		append("rtime",  x.rtime);
		append("refreshtime",  x.refreshtime);
		data = data + "</" + name + ">\n";
	}
	void append(const char* name, const GFactionFortressDetail& x)
	{
		data = data + "<" + name + ">\n";
		append("factionid",  x.factionid);
		append("info",  x.info);
		append("info2",  x.info2);
		append("reserved1",  x.reserved1);
		append("reserved2",  x.reserved2);
		append("reserved3",  x.reserved3);
		append("reserved4",  x.reserved4);
		append("reserved5",  x.reserved5);
		data = data + "</" + name + ">\n";
	}
	void append(const char* name, const GWebTradeItem& x)
	{
		data = data + "<" + name + ">\n";
		append("sn",  x.sn);
		append("seller_roleid",  x.seller_roleid);
		append("seller_userid",  x.seller_userid);
		append("seller_name",  x.seller_name);
		append("posttype",  x.posttype);
		append("money", (int) x.money);
		append("item_id", (int) x.item_id);
		append("item_count",  x.item_count);
		append("state",  x.state);
		append("post_endtime",  x.post_endtime);
		append("show_endtime",  x.show_endtime);
		append("price",  x.price);
		append("sell_endtime",  x.sell_endtime);
		append("buyer_roleid",  x.buyer_roleid);
		append("buyer_userid",  x.buyer_userid);
		append("buyer_name",  x.buyer_name);
		append("commodity_id",  x.commodity_id);
		data = data + "</" + name + ">\n";
	}
	void append(const char* name, const StockLog& x)
	{
		data = data + "<" + name + ">\n";
		append("tid", (int) x.tid);
		append("time",  x.time);
		append("result",  x.result);
		append("volume",  x.volume);
		append("cost",  x.cost);
		data = data + "</" + name + ">\n";
	}
	void append(const char* name, const GFactionRelation& x)
	{
		data = data + "<" + name + ">\n";
		append("fid",  x.fid);
		append("last_op_time",  x.last_op_time);
		append("alliance",  x.alliance);
		append("hostile",  x.hostile);
		append("apply",  x.apply);
		append("reserved1",  x.reserved1);
		append("reserved2",  x.reserved2);
		append("reserved3",  x.reserved3);
		append("reserved4",  x.reserved4);
		append("reserved5",  x.reserved5);
		data = data + "</" + name + ">\n";
	}
	void append(const char* name, const PShopItem& x)
	{
		data = data + "<" + name + ">\n";
		append("item",  x.item);
		append("price", (int) x.price);
		append("reserved1",  x.reserved1);
		append("reserved2",  x.reserved2);
		data = data + "</" + name + ">\n";
	}
	void append(const char* name, const GForceGlobalDataList& x)
	{
		data = data + "<" + name + ">\n";
		append("list",  x.list);
		append("update_time",  x.update_time);
		append("reserved1",  x.reserved1);
		append("reserved2",  x.reserved2);
		append("reserved3",  x.reserved3);
		append("reserved4",  x.reserved4);
		append("reserved5",  x.reserved5);
		append("reserved6",  x.reserved6);
		append("reserved7",  x.reserved7);
		append("reserved8",  x.reserved8);
		data = data + "</" + name + ">\n";
	}
	void append(const char* name, const GForceGlobalData& x)
	{
		data = data + "<" + name + ">\n";
		append("force_id",  x.force_id);
		append("player_count",  x.player_count);
		append("development",  x.development);
		append("construction",  x.construction);
		append("activity",  x.activity);
		append("activity_level",  x.activity_level);
		append("reserved1",  x.reserved1);
		append("reserved2",  x.reserved2);
		append("reserved3",  x.reserved3);
		append("reserved4",  x.reserved4);
		append("reserved5",  x.reserved5);
		append("reserved6",  x.reserved6);
		append("reserved7",  x.reserved7);
		append("reserved8",  x.reserved8);
		data = data + "</" + name + ">\n";
	}
	void append(const char* name, const User& x)
	{
		data = data + "<" + name + ">\n";
		append("logicuid", (int) x.logicuid);
		append("rolelist", (int) x.rolelist);
		append("cash",  x.cash);
		append("money",  x.money);
		append("cash_add", (int) x.cash_add);
		append("cash_buy", (int) x.cash_buy);
		append("cash_sell", (int) x.cash_sell);
		append("cash_used", (int) x.cash_used);
		append("add_serial",  x.add_serial);
		append("use_serial",  x.use_serial);
		append("exg_log",  x.exg_log);
		append("addiction",  x.addiction);
		append("cash_password",  x.cash_password);
		append("autolock",  x.autolock);
		append("status",  x.status);
		append("forbid",  x.forbid);
		append("reference",  x.reference);
		append("consume_reward",  x.consume_reward);
		append("taskcounter",  x.taskcounter);
		append("cash_sysauction",  x.cash_sysauction);
		append("login_record",  x.login_record);
		append("mall_consumption",  x.mall_consumption);
		append("reserved32",  x.reserved32);
		data = data + "</" + name + ">\n";
	}
	void append(const char* name, const GRoleStorehouse& x)
	{
		data = data + "<" + name + ">\n";
		append("capacity", (int) x.capacity);
		append("money", (int) x.money);
		append("items",  x.items);
		append("size1", (char) x.size1);
		append("size2", (char) x.size2);
		append("dress",  x.dress);
		append("material",  x.material);
		append("size3", (char) x.size3);
		append("generalcard",  x.generalcard);
		append("reserved",  x.reserved);
		data = data + "</" + name + ">\n";
	}
	void append(const char* name, const GMember& x)
	{
		data = data + "<" + name + ">\n";
		append("rid", (int) x.rid);
		append("role",  x.role);
		data = data + "</" + name + ">\n";
	}
	void append(const char* name, const GUniqueDataElem& x)
	{
		data = data + "<" + name + ">\n";
		append("vtype",  x.vtype);
		append("value",  x.value);
		append("version",  x.version);
		append("reserved",  x.reserved);
		data = data + "</" + name + ">\n";
	}
	void append(const char* name, const GRoleTask& x)
	{
		data = data + "<" + name + ">\n";
		append("task_data",  x.task_data);
		append("task_complete",  x.task_complete);
		append("task_finishtime",  x.task_finishtime);
		append("task_inventory",  x.task_inventory);
		data = data + "</" + name + ">\n";
	}
	void append(const char* name, const MNFactionInfo& x)
	{
		data = data + "<" + name + ">\n";
		append("unifid",  x.unifid);
		append("fid", (int) x.fid);
		append("faction_name",  x.faction_name);
		append("master_name",  x.master_name);
		append("zoneid",  x.zoneid);
		append("domain_num",  x.domain_num);
		append("credit",  x.credit);
		append("credit_this_week",  x.credit_this_week);
		append("credit_get_time",  x.credit_get_time);
		append("invite_count", (int) x.invite_count);
		append("accept_sn", (int) x.accept_sn);
		append("bonus_sn", (int) x.bonus_sn);
		append("version", (int) x.version);
		data = data + "</" + name + ">\n";
	}
	void append(const char* name, const KECandidate& x)
	{
		data = data + "<" + name + ">\n";
		append("roleid",  x.roleid);
		append("serial_num",  x.serial_num);
		append("deposit",  x.deposit);
		append("votes",  x.votes);
		append("reserved1",  x.reserved1);
		append("reserved2",  x.reserved2);
		append("reserved3",  x.reserved3);
		data = data + "</" + name + ">\n";
	}
	void append(const char* name, const Message& x)
	{
		data = data + "<" + name + ">\n";
		append("channel", (char) x.channel);
		append("src_name",  x.src_name);
		append("srcroleid",  x.srcroleid);
		append("dst_name",  x.dst_name);
		append("dstroleid",  x.dstroleid);
		append("msg",  x.msg);
		data = data + "</" + name + ">\n";
	}
	void append(const char* name, const GTerritoryStore& x)
	{
		data = data + "<" + name + ">\n";
		append("cities",  x.cities);
		append("status",  x.status);
		append("special_time",  x.special_time);
		append("reserved2",  x.reserved2);
		append("reserved3",  x.reserved3);
		append("reserved4",  x.reserved4);
		data = data + "</" + name + ">\n";
	}
	void append(const char* name, const GServerData& x)
	{
		data = data + "<" + name + ">\n";
		append("world_tag",  x.world_tag);
		append("wedding_data",  x.wedding_data);
		append("dpsrank_data",  x.dpsrank_data);
		append("reserved11",  x.reserved11);
		append("reserved12",  x.reserved12);
		append("reserved2",  x.reserved2);
		append("reserved3",  x.reserved3);
		append("reserved4",  x.reserved4);
		append("reserved5",  x.reserved5);
		data = data + "</" + name + ">\n";
	}
	void append(const char* name, const GFactionFortressInfo2& x)
	{
		data = data + "<" + name + ">\n";
		append("health",  x.health);
		append("offense_faction",  x.offense_faction);
		append("offense_starttime",  x.offense_starttime);
		append("offense_endtime",  x.offense_endtime);
		append("challenge_list",  x.challenge_list);
		append("reserved1",  x.reserved1);
		append("reserved2",  x.reserved2);
		append("reserved3",  x.reserved3);
		data = data + "</" + name + ">\n";
	}
	void append(const char* name, const GTableDefinition& x)
	{
		data = data + "<" + name + ">\n";
		append("user",  x.user);
		append("base",  x.base);
		append("status",  x.status);
		append("task",  x.task);
		append("inventory",  x.inventory);
		append("equipment",  x.equipment);
		append("storehouse",  x.storehouse);
		append("mailbox",  x.mailbox);
		append("friends",  x.friends);
		append("messages",  x.messages);
		append("factioninfo",  x.factioninfo);
		append("order",  x.order);
		append("syslog",  x.syslog);
		append("config",  x.config);
		append("factionname",  x.factionname);
		append("waitdel",  x.waitdel);
		append("clsconfig",  x.clsconfig);
		append("rolename",  x.rolename);
		append("shoplog",  x.shoplog);
		append("auction",  x.auction);
		append("userfaction",  x.userfaction);
		append("sellpoint",  x.sellpoint);
		append("translog",  x.translog);
		append("city",  x.city);
		append("gtask",  x.gtask);
		append("userstore",  x.userstore);
		append("webtrade",  x.webtrade);
		append("webtradesold",  x.webtradesold);
		append("serverdata",  x.serverdata);
		append("factionfortress",  x.factionfortress);
		append("factionrelation",  x.factionrelation);
		append("force",  x.force);
		append("friendext",  x.friendext);
		append("globalcontrol",  x.globalcontrol);
		append("rolenamehis",  x.rolenamehis);
		append("kingelection",  x.kingelection);
		append("playershop",  x.playershop);
		append("weborderitem",  x.weborderitem);
		append("playerprofile",  x.playerprofile);
		append("uniquedata",  x.uniquedata);
		append("recalluser",  x.recalluser);
		append("mappassword",  x.mappassword);
		append("solochallengerank",  x.solochallengerank);
		append("mnfactioninfo",  x.mnfactioninfo);
		append("mnfactionapplyinfo",  x.mnfactionapplyinfo);
		append("mndomaininfo",  x.mndomaininfo);
		append("mndomainbonus",  x.mndomainbonus);
		data = data + "</" + name + ">\n";
	}
	void append(const char* name, const StockOrder& x)
	{
		data = data + "<" + name + ">\n";
		append("tid", (int) x.tid);
		append("time",  x.time);
		append("userid",  x.userid);
		append("price",  x.price);
		append("volume",  x.volume);
		append("status",  x.status);
		data = data + "</" + name + ">\n";
	}
	void append(const char* name, const MNDomainBonus& x)
	{
		data = data + "<" + name + ">\n";
		append("unifid",  x.unifid);
		append("bonus_sn", (int) x.bonus_sn);
		append("reward_list",  x.reward_list);
		append("master_reward",  x.master_reward);
		append("rolelist",  x.rolelist);
		data = data + "</" + name + ">\n";
	}
	void append(const char* name, const GRolePocket& x)
	{
		data = data + "<" + name + ">\n";
		append("capacity", (int) x.capacity);
		append("timestamp", (int) x.timestamp);
		append("money", (int) x.money);
		append("items",  x.items);
		append("reserved1",  x.reserved1);
		append("reserved2",  x.reserved2);
		data = data + "</" + name + ">\n";
	}
	void append(const char* name, const GRoleStatus& x)
	{
		data = data + "<" + name + ">\n";
		append("version",  x.version);
		append("level",  x.level);
		append("level2",  x.level2);
		append("exp",  x.exp);
		append("sp",  x.sp);
		append("pp",  x.pp);
		append("hp",  x.hp);
		append("mp",  x.mp);
		append("posx",  x.posx);
		append("posy",  x.posy);
		append("posz",  x.posz);
		append("worldtag",  x.worldtag);
		append("invader_state",  x.invader_state);
		append("invader_time",  x.invader_time);
		append("pariah_time",  x.pariah_time);
		append("reputation",  x.reputation);
		append("custom_status",  x.custom_status);
		append("filter_data",  x.filter_data);
		append("charactermode",  x.charactermode);
		append("instancekeylist",  x.instancekeylist);
		append("dbltime_expire",  x.dbltime_expire);
		append("dbltime_mode",  x.dbltime_mode);
		append("dbltime_begin",  x.dbltime_begin);
		append("dbltime_used",  x.dbltime_used);
		append("dbltime_max",  x.dbltime_max);
		append("time_used",  x.time_used);
		append("dbltime_data",  x.dbltime_data);
		append("storesize", (short) x.storesize);
		append("petcorral",  x.petcorral);
		append("property",  x.property);
		append("var_data",  x.var_data);
		append("skills",  x.skills);
		append("storehousepasswd",  x.storehousepasswd);
		append("waypointlist",  x.waypointlist);
		append("coolingtime",  x.coolingtime);
		append("npc_relation",  x.npc_relation);
		append("multi_exp_ctrl",  x.multi_exp_ctrl);
		append("storage_task",  x.storage_task);
		append("faction_contrib",  x.faction_contrib);
		append("force_data",  x.force_data);
		append("online_award",  x.online_award);
		append("profit_time_data",  x.profit_time_data);
		append("country_data",  x.country_data);
		append("king_data",  x.king_data);
		append("meridian_data",  x.meridian_data);
		append("extraprop",  x.extraprop);
		append("title_data",  x.title_data);
		append("reincarnation_data",  x.reincarnation_data);
		append("realm_data",  x.realm_data);
		append("rank",  x.rank);
		append("reserved3",  x.reserved3);
		data = data + "</" + name + ">\n";
	}
	void append(const char* name, const GSysLog& x)
	{
		data = data + "<" + name + ">\n";
		append("roleid",  x.roleid);
		append("time",  x.time);
		append("ip",  x.ip);
		append("source",  x.source);
		append("money",  x.money);
		append("items",  x.items);
		append("reserved1",  x.reserved1);
		append("reserved2",  x.reserved2);
		append("reserved3",  x.reserved3);
		append("reserved4",  x.reserved4);
		data = data + "</" + name + ">\n";
	}
	void append(const char* name, const WebOrderItemDetail& x)
	{
		data = data + "<" + name + ">\n";
		append("userid",  x.userid);
		append("roleid",  x.roleid);
		append("goods_id",  x.goods_id);
		append("count",  x.count);
		append("proctype",  x.proctype);
		append("goods_flag",  x.goods_flag);
		append("goods_price",  x.goods_price);
		append("goods_price_before_discount",  x.goods_price_before_discount);
		append("goods_paytype",  x.goods_paytype);
		append("attach_money", (int) x.attach_money);
		append("time_stamp",  x.time_stamp);
		append("reserved1",  x.reserved1);
		append("reserved2",  x.reserved2);
		append("reserved3",  x.reserved3);
		data = data + "</" + name + ">\n";
	}
	void append(const char* name, const GRoleData& x)
	{
		data = data + "<" + name + ">\n";
		append("base",  x.base);
		append("status",  x.status);
		append("pocket",  x.pocket);
		append("equipment",  x.equipment);
		append("storehouse",  x.storehouse);
		append("task",  x.task);
		data = data + "</" + name + ">\n";
	}
	void append(const char* name, const SoloChallengeRankData& x)
	{
		data = data + "<" + name + ">\n";
		append("roleid",  x.roleid);
		append("level",  x.level);
		append("cls",  x.cls);
		append("total_time",  x.total_time);
		append_string("name",  x.name);
		append("type",  x.type);
		append("zoneid", (char) x.zoneid);
		append("update_time",  x.update_time);
		append("reserved1",  x.reserved1);
		append("reserved2",  x.reserved2);
		append("reserved3",  x.reserved3);
		data = data + "</" + name + ">\n";
	}
	void append(const char* name, const SoloChallengeRankDataExt& x)
	{
		data = data + "<" + name + ">\n";
		append("update_time",  x.update_time);
		append("data",  x.data);
		append("zoneid",  x.zoneid);
		append("reserved1",  x.reserved1);
		append("reserved2",  x.reserved2);
		append("reserved3",  x.reserved3);
		data = data + "</" + name + ">\n";
	}
	void append(const char* name, const GGlobalControlData& x)
	{
		data = data + "<" + name + ">\n";
		append("cash_money_exchange_open",  x.cash_money_exchange_open);
		append("cash_money_exchange_rate",  x.cash_money_exchange_rate);
		append("forbid_ctrl_list",  x.forbid_ctrl_list);
		append("forbid_item_list",  x.forbid_item_list);
		append("forbid_service_list",  x.forbid_service_list);
		append("forbid_task_list",  x.forbid_task_list);
		append("forbid_skill_list",  x.forbid_skill_list);
		append("trigger_ctrl_list",  x.trigger_ctrl_list);
		append("forbid_shopitem_list",  x.forbid_shopitem_list);
		append("forbid_recipe_list",  x.forbid_recipe_list);
		append("reserved3",  x.reserved3);
		append("reserved4",  x.reserved4);
		append("reserved5",  x.reserved5);
		append("reserved6",  x.reserved6);
		append("reserved7",  x.reserved7);
		append("reserved8",  x.reserved8);
		append("reserved9",  x.reserved9);
		append("reserved10",  x.reserved10);
		data = data + "</" + name + ">\n";
	}
	void append(const char* name, const GFactionFortressInfo& x)
	{
		data = data + "<" + name + ">\n";
		append("level",  x.level);
		append("exp",  x.exp);
		append("exp_today",  x.exp_today);
		append("exp_today_time",  x.exp_today_time);
		append("tech_point",  x.tech_point);
		append("technology",  x.technology);
		append("material",  x.material);
		append("building",  x.building);
		append("common_value",  x.common_value);
		append("actived_spawner",  x.actived_spawner);
		append("reserved11",  x.reserved11);
		append("reserved12",  x.reserved12);
		append("reserved2",  x.reserved2);
		append("reserved3",  x.reserved3);
		data = data + "</" + name + ">\n";
	}
	void append(const char* name, const GRoleEquipment& x)
	{
		data = data + "<" + name + ">\n";
		append("inv",  x.inv);
		data = data + "</" + name + ">\n";
	}
	void append(const char* name, const GRoleForbid& x)
	{
		data = data + "<" + name + ">\n";
		append("type", (char) x.type);
		append("time",  x.time);
		append("createtime",  x.createtime);
		append("reason",  x.reason);
		data = data + "</" + name + ">\n";
	}
	void append(const char* name, const GMailBox& x)
	{
		data = data + "<" + name + ">\n";
		append("timestamp", (int) x.timestamp);
		append("status", (int) x.status);
		append("mails",  x.mails);
		data = data + "</" + name + ">\n";
	}
	void append(const char* name, const GRoleInventory& x)
	{
		data = data + "<" + name + ">\n";
		append("id", (int) x.id);
		append("pos",  x.pos);
		append("count",  x.count);
		append("max_count",  x.max_count);
		append("data",  x.data);
		append("proctype",  x.proctype);
		append("expire_date",  x.expire_date);
		append("guid1",  x.guid1);
		append("guid2",  x.guid2);
		append("mask",  x.mask);
		data = data + "</" + name + ">\n";
	}
	void append(const char* name, const GRoleTableClsconfig& x)
	{
		data = data + "<" + name + ">\n";
		append("version",  x.version);
		append("base",  x.base);
		append("status",  x.status);
		append("inventory",  x.inventory);
		append("equipment",  x.equipment);
		append("storehouse",  x.storehouse);
		data = data + "</" + name + ">\n";
	}
	void append(const char* name, const GGroupInfo& x)
	{
		data = data + "<" + name + ">\n";
		append("gid",  x.gid);
		append_string("name",  x.name);
		data = data + "</" + name + ">\n";
	}
	void append(const char* name, const MNDomainBonusRewardItem& x)
	{
		data = data + "<" + name + ">\n";
		append("item_id",  x.item_id);
		append("item_num", (int) x.item_num);
		append("proc_type",  x.proc_type);
		append("max_count",  x.max_count);
		data = data + "</" + name + ">\n";
	}
	void append(const char* name, const MNFactionApplyInfo& x)
	{
		data = data + "<" + name + ">\n";
		append("unifid",  x.unifid);
		append("applicant_id",  x.applicant_id);
		append("dest", (char) x.dest);
		append("cost", (int) x.cost);
		data = data + "</" + name + ">\n";
	}
	void append(const char* name, const KEKing& x)
	{
		data = data + "<" + name + ">\n";
		append("roleid",  x.roleid);
		append("end_time",  x.end_time);
		append("reserved1",  x.reserved1);
		append("reserved2",  x.reserved2);
		append("reserved3",  x.reserved3);
		data = data + "</" + name + ">\n";
	}
	void append(const char* name, const GPair& x)
	{
		data = data + "<" + name + ">\n";
		append("key",  x.key);
		append("value",  x.value);
		data = data + "</" + name + ">\n";
	}
	void append(const char* name, const GAuctionItem& x)
	{
		data = data + "<" + name + ">\n";
		append("auctionid", (int) x.auctionid);
		append("bidprice", (int) x.bidprice);
		append("binprice", (int) x.binprice);
		append("end_time", (int) x.end_time);
		append("itemid", (int) x.itemid);
		append("count", (short) x.count);
		append("seller", (int) x.seller);
		append("bidder", (int) x.bidder);
		data = data + "</" + name + ">\n";
	}
	void append(const char* name, const GFactionAlliance& x)
	{
		data = data + "<" + name + ">\n";
		append("fid",  x.fid);
		append("end_time",  x.end_time);
		data = data + "</" + name + ">\n";
	}
	void append(const char* name, const GFriendExtra& x)
	{
		data = data + "<" + name + ">\n";
		append("friendExtInfo",  x.friendExtInfo);
		append("sendaumailinfo",  x.sendaumailinfo);
		append("enemylistinfo",  x.enemylistinfo);
		append("reserved11",  x.reserved11);
		append("reserved12",  x.reserved12);
		append("reserved2",  x.reserved2);
		append("reserved3",  x.reserved3);
		append("reserved4",  x.reserved4);
		append("reserved5",  x.reserved5);
		data = data + "</" + name + ">\n";
	}
	void append(const char* name, const GUserFaction& x)
	{
		data = data + "<" + name + ">\n";
		append("rid", (int) x.rid);
		append_string("name",  x.name);
		append("fid", (int) x.fid);
		append("cls",  x.cls);
		append("role",  x.role);
		append("delayexpel",  x.delayexpel);
		append("extend",  x.extend);
		append("nickname",  x.nickname);
		data = data + "</" + name + ">\n";
	}

};

};

#endif
