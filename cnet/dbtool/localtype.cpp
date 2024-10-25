#include "marshal.h"
#include "rpc.h"
#include "conv_charset.h"
#include "ggroupinfo"
#include "gfriendinfo"
#include "gmember"
#include "factionid"
#include "gfactioninfo"
#include "guserfaction"
#include "gfriendlist"
#include "gshoplog"
#include "grolestatus"
#include "gmailheader"
#include "gmail"
#include "gmailbox"
#include "groleinventory"
#include "grolestorehouse"
#include "groleforbid"
#include "grolebase"
#include "gauctionitem"
#include "gauctiondetail"
#include "grolepocket"
#include "groletask"
#include "stocklog"
#include "user"
#include "stockorder"
#include "storage.h"
#include "dbbuffer.h"
#include "databrowser.h"

using namespace GNET;
extern iconv_t _iconvu2g;
extern iconv_t _iconvg2u;

typedef void (*dumpper_t)(FILE* fp, GNET::Marshal::OctetsStream& data);
typedef std::map<std::string, dumpper_t> DumpperMap;
DumpperMap dumppers;
extern void hexdump(FILE* fp, const char* data, int len);
extern int dec2hex(const char* data, int slen, char* dst, int dlen);
extern unsigned char hex2dec(unsigned char c);
extern int hex2dec(const char* data, int len, char* dst, int dlen);
extern int IconvOctets(Octets& src, Octets& dst, iconv_t cd);

void DumpFriends(FILE* fp, Marshal::OctetsStream& data)
{
	GFriendList list;
	data >> list;
	fprintf(fp, "[friends]\n");
	for(size_t i=0;i<list.friends.size();i++)
	{
		Octets name;
		IconvOctets(list.friends[i].name,name,_iconvu2g);
		fprintf(fp, "%10d  %-.*s\n", list.friends[i].rid, name.size(), (char*)name.begin());
	}
}
void DumpFactionInfo(FILE* fp, Marshal::OctetsStream& data)
{
}

void DumpStatus(FILE* fp, Marshal::OctetsStream& data)
{
	GRoleStatus info;
	data >> info;
	fprintf(fp, "size          %-d\n"
		    "level         %-d\n"
		    "exp           %-lld\n"
		    "hp            %-d\n"
		    "mp            %-d\n"
		    "skill         %-d\n" 
		    "cooling       %-d\n" 
		    "filter        %-d\n"
		    "vardata       %-d\n",
		data.size(), info.level, info.exp, info.hp, info.mp,  
		info.skills.size(), info.coolingtime.size(), info.filter_data.size(), 
		info.var_data.size());
}

void DumpMail(FILE* fp, Marshal::OctetsStream& data)
{
	GMailBox box;
	data >> box;
	char buf[1024];
	for(size_t i=0;i<box.mails.size();i++)
	{
		size_t dlen = 1024;
		size_t slen = box.mails[i].header.sender_name.size();
		char *in = (char*)(box.mails[i].header.sender_name.begin());
		char *out = buf;
		size_t len;
		if(box.mails[i].header.sndr_type==0)
		{
			len = iconv(_iconvu2g,&in,&slen,&out,&dlen);
			if (((size_t)-1)==len)
			{
				printf("ERROR: conversion failed\n");
				perror("iconv");
				return;
			}
		}
		else
			dlen = 1024;
		fprintf(fp, "From: %-.*s(%d)\nSend_time: %s", 
				1024-dlen, buf, box.mails[i].header.sender, 
				ctime((time_t*)&(box.mails[i].header.send_time)));
		dlen = 1024;
		slen = box.mails[i].header.title.size();
		in = (char*)(box.mails[i].header.title.begin());
		out = buf;
		if(box.mails[i].header.sndr_type==0)
		{
			len = iconv(_iconvu2g,&in,&slen,&out,&dlen);
			if (((size_t)-1)==len)
			{
				printf("ERROR: conversion failed\n");
				perror("iconv");
				return;
			}
		}
		else
			dlen = 1024;
		fprintf(fp, "Read_time: %sID(%d) Status(%d)\nSubject: %-.*s\n",
			ctime((time_t*)&(box.mails[i].header.receiver)),  
			box.mails[i].header.id, box.mails[i].header.attribute, 1024-dlen, buf);
		fprintf(fp, "Money: %d\nItem: %d %d\n\n",box.mails[i].attach_money, box.mails[i].attach_obj.id, 
			box.mails[i].attach_obj.count);
	}
}

void DumpBin(FILE* fp, Marshal::OctetsStream& data)
{
	fwrite(data.begin(), 1, data.size(), fp);
}

void DumpItems(FILE* fp, Marshal::OctetsStream& data)
{
	GRolePocket v;
	data >> v;
	fprintf(fp, "size        %d\n", data.size());
	fprintf(fp, "capacity    %d\n", v.capacity);
	fprintf(fp, "timestamp   %d\n", v.timestamp);
	fprintf(fp, "money       %d\n", v.money);
	if(v.items.size())
	{
		fprintf(fp, "\n[inventory]\n id  pos  count  size\n");
		for(unsigned int i=0;i<v.items.size();i++)
		{
			fprintf(fp, "%-5d %-2d   %-4d  %-3d\n", v.items[i].id, v.items[i].pos, v.items[i].count, 
				v.items[i].data.size());
		}
	}
}

void DumpUser(FILE* fp, Marshal::OctetsStream& data)
{
	User s;
	data >> s;

	fprintf(fp, "logicuid     %d\n", s.logicuid);
	fprintf(fp, "roles        %x\n", s.rolelist);
	fprintf(fp, "cash         %d\n", s.cash);
	fprintf(fp, "money        %d\n", s.money);
	fprintf(fp, "cash_add     %d\n", s.cash_add);
	fprintf(fp, "cash_buy     %d\n", s.cash_buy);
	fprintf(fp, "cash_sell    %d\n", s.cash_sell);
	fprintf(fp, "cash_used    %d\n", s.cash_used);
	fprintf(fp, "add_serial   %d\n", s.add_serial);
	fprintf(fp, "use_serial   %d\n", s.use_serial);
	fprintf(fp, "exg_log      %d\n", s.exg_log.size());
	fprintf(fp, "cash_pass    %d\n", s.cash_password.size());
	fprintf(fp, "autolock     %d\n", s.autolock.size());
	fprintf(fp, "status       %d\n", s.status);
}

void DumpTask(FILE* fp, Marshal::OctetsStream& data)
{
	GRoleTask s;
	data >> s;

	fprintf(fp, "size        %d\n", data.size());
	fprintf(fp, "data        %d\n", s.task_data.size());
	fprintf(fp, "complete    %d\n", s.task_complete.size());
	fprintf(fp, "time        %d\n", s.task_finishtime.size());
	if(s.task_inventory.size())
	{
		fprintf(fp, "\n id   pos  count  size\n");
		for(unsigned int i=0;i<s.task_inventory.size();i++)
		{
			fprintf(fp, "%-5d  %-2d    %-4d   %-3d\n", s.task_inventory[i].id, s.task_inventory[i].pos, 
				s.task_inventory[i].count, s.task_inventory[i].data.size());
		}
	}
}

void DumpDepot(FILE* fp, Marshal::OctetsStream& data)
{
	GRoleStorehouse s;
	data >> s;

	fprintf(fp, "size        %d\n", data.size());
	fprintf(fp, "capacity    %d\n", s.capacity);
	fprintf(fp, "money       %d\n", s.money);
	fprintf(fp, "\n id   pos  count  size\n");
	for(unsigned int i=0;i<s.items.size();i++)
	{
		fprintf(fp, "%-5d  %-2d    %-4d   %-3d\n", s.items[i].id, s.items[i].pos, s.items[i].count, 
				s.items[i].data.size());
	}
}
void DumpBase(FILE* fp, Marshal::OctetsStream& data)
{
	GRoleBase s;
	data >> s;

	Octets name;
	IconvOctets(s.name,name,_iconvu2g);
	fprintf(fp, "id           %-d\n"
		    "name         %-.*s\n"
		    "size         %-d\n"
		    "config       %-d\n"
		    "help         %-d\n"
		    "spouse       %-d\n",
		    s.id, name.size(),(char*)name.begin(), data.size(), 
		    s.config_data.size(), s.help_states.size(), 
		    s.spouse);
}

void DumpOrder(FILE* fp, Marshal::OctetsStream& data)
{
	StockOrder s;
	data >> s;

	fprintf(fp, "tid      %-8d\ntime     %-8d\nroleid   %-8d\nprice    %-8d\nvolume   %-8d\nstatus   %-4d\n",
		 s.tid, s.time, s.userid, s.price, s.volume, s.status);
}
void DumpName(FILE* fp, Marshal::OctetsStream& data)
{
	int roleid;
	data >> roleid;
	fprintf(fp, "roleid=%d", roleid);
}
void DumpLog(FILE* fp, Marshal::OctetsStream& data)
{
	std::vector<GShopLog> s;
	data >> s;

	fprintf(fp, "roleid     order_id item_id expire     item_count order_count cost time       guid1    guid2\n");
	for(std::vector<GShopLog>::iterator it=s.begin(),ie=s.end();it!=ie;++it)
	{
		fprintf(fp, "%-11d%-9d%-8d%-11d%-11d%-12d%-5d%-11d%-8x %-4x\n", it->roleid, it->order_id, it->item_id, 
			it->expire, it->item_count, it->order_count, it->cash_need, it->time, it->guid1, it->guid2);
	}
}
void InitDumpType()
{
	dumppers.insert(std::make_pair("bin",          DumpBin));
	dumppers.insert(std::make_pair("base",         DumpBase));
	dumppers.insert(std::make_pair("gfriendlist",  DumpFriends));
	dumppers.insert(std::make_pair("friends",      DumpFriends));
	dumppers.insert(std::make_pair("grolestatus",  DumpStatus));
	dumppers.insert(std::make_pair("status",       DumpStatus));
	dumppers.insert(std::make_pair("gfactioninfo", DumpFactionInfo));
	dumppers.insert(std::make_pair("faction",      DumpFactionInfo));
	dumppers.insert(std::make_pair("gmailbox",     DumpMail));
	dumppers.insert(std::make_pair("mailbox",      DumpMail));
	dumppers.insert(std::make_pair("inventory",    DumpItems));
	dumppers.insert(std::make_pair("task",         DumpTask));
	dumppers.insert(std::make_pair("storehouse",   DumpDepot));
	dumppers.insert(std::make_pair("user",         DumpUser));
	dumppers.insert(std::make_pair("order",        DumpOrder));
	dumppers.insert(std::make_pair("log",          DumpLog));
	dumppers.insert(std::make_pair("shoplog",      DumpLog));
	dumppers.insert(std::make_pair("rolename",     DumpName));
}

void DumpData(FILE* fp, const char* type, Octets& data)
{
	try{
		Marshal::OctetsStream copy(data);
		DumpperMap::iterator it = dumppers.find(type);
		if(it==dumppers.end())
		{
			printf("Unrecognized data type\n");
			return;
		}
		(it->second)(fp, copy);
	}
	catch(...)
	{
		printf("Dump exception: data size %d\n", data.size());
	}
}

