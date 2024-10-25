
#ifndef __GNET_GMAILENDSYNC_HPP
#define __GNET_GMAILENDSYNC_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "groleinventory"
#include "gmailsyncdata"
#include "../gdbclient/db_if.h"
#include "../include/localmacro.h"

void player_end_sync(int role_id, unsigned int money, const GDB::itemlist & item_change_list, bool storesaved, bool cashsaved);
void player_cash_notify(int role_id, int cash_plus_used);
void player_modify_cashused(int role_id, int cash_used);
namespace GNET
{

class GMailEndSync : public GNET::Protocol
{
	#include "gmailendsync"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if(retcode==ERR_SUCCESS)
		{
			GDB::itemdata ar[128];
			if(syncdata.inventory.items.size() > 128)
			{
				Log::log(LOG_ERR,"gamed:: invalid syncdata roleid=%d", roleid);
				return;
			}
			GDB::itemlist list;
			list.list = ar;
			list.count = GDB::convert_item(syncdata.inventory.items,ar,128);
			//要返回SYNC_MODIFYCASHUSED必须先在gamedbd进行流水号加1
			if (syncdata.data_mask&SYNC_MODIFYCASHUSED)
				player_modify_cashused(roleid, syncdata.cash_used);
            if (syncdata.data_mask&(SYNC_CASHTOTAL))
				player_cash_notify(roleid, syncdata.cash_total);
			bool storesaved = syncdata.data_mask&SYNC_STOTEHOUSE;
			bool logsaved   = syncdata.data_mask&SYNC_SHOPLOG;
			player_end_sync(roleid, syncdata.inventory.money, list, storesaved, logsaved);
		}else
		{
			GDB::itemlist list = {0,0};
			player_end_sync(roleid, syncdata.inventory.money, list, false,false);
		}
	}
};

};

#endif
