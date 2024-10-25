#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <common/protocol.h>
#include "world.h"
#include "player_imp.h"
#include "usermsg.h"
#include "cooldowncfg.h"

#include "task/taskman.h"
#include "playergiftcard.h"
#include <gsp_if.h>

void player_giftcard::OnHeartbeat(gplayer_imp * pImp)
{
	if(IsFree()) return;

	if(--_timeout <= 0)
	{
		switch(data.state)
		{
			case GCR_STATE_HALFREDEEM:
				OnHalfRedeem(pImp);				
				break;
			case GCR_STATE_WAITTASK:
				OnWaitTask(pImp);
				break;
			default:
				{
					GLog::log(LOG_ERR,"roleid:%d gift code redeem state%d error[type%d parenttype%d cardnumber%s]",
					pImp->GetParent()->ID.id, data.state, data.type, data.parenttype, data.cardnumber);
					ClearData();
				}
				break;
		}
	}
}


void player_giftcard::ClearData()
{
	memset(&data,0,sizeof(data));
	_noticepass = 0;
	_timeout = 0;
}

void player_giftcard::OnRedeem(gplayer_imp * pImp, const char (&cn)[GIFT_CARDNUMBER_LEN],int type,int parenttype,int retcode)
{
	if(memcmp(data.cardnumber,cn,GIFT_CARDNUMBER_LEN))
	{
		//log
		return;
	}

	if(retcode != GCR_SUCCESS)
	{
		//log
		pImp->_runner->notify_giftcard_redeem(retcode,type,parenttype,cn);	
		ClearData();
		return;
	}	

	data.type = type;
	data.parenttype = parenttype;

	Complete(pImp);
}

bool player_giftcard::Complete(gplayer_imp * pImp)
{
	int res = GCR_SUCCESS;

	PlayerTaskInterface tif(pImp);
	if( pImp->_lock_inventory || 
	   (pImp->GetPlayerState() != gplayer_imp::PLAYER_STATE_NORMAL && pImp->GetPlayerState() != gplayer_imp::PLAYER_SIT_DOWN) ||
		!tif.OnGiftCardTask(data.type))
	{
		data.state = GCR_STATE_WAITTASK;
		SetTimeOut(GIFT_WAITTASK_TIME);
		res = GCR_WAIT_TASK;
		if(!(_noticepass++ % GIFT_NOTICE_INTERVAL))  // 任务发放条件不足间隔通知客户端
		{
			pImp->_runner->notify_giftcard_redeem(res,data.type, data.parenttype, data.cardnumber);
		}
	}
	else
	{
		GLog::log(LOG_INFO,"roleid:%d giftcard redeem complate [type%d parenttype%d cardnumber%.*s]",
			pImp->GetParent()->ID.id, data.type, data.parenttype, GIFT_CARDNUMBER_LEN, data.cardnumber);
		
		pImp->_runner->notify_giftcard_redeem(GCR_SUCCESS,data.type,data.parenttype,data.cardnumber);	
		ClearData();
	}

	return res == GCR_SUCCESS;
}

void player_giftcard::OnHalfRedeem(gplayer_imp * pImp)
{
	GLog::log(LOG_INFO,"roleid:%d giftcode redeem [type%d parenttype%d cardnumber%.*s] halfredeem tick", 
			pImp->GetParent()->ID.id, data.type, data.parenttype, GIFT_CARDNUMBER_LEN, data.cardnumber);
	
	GMSV::SendPlayerGiftCodeRedeem(pImp->GetParent()->ID.id,data.cardnumber);
	SetTimeOut(GIFT_WAITREDEEM_TIME);
}

void player_giftcard::OnWaitTask(gplayer_imp * pImp)
{
	GLog::log(LOG_INFO,"roleid:%d giftcode redeem [type%d parenttype%d cardnumber%.*s] waittask tick", 
			pImp->GetParent()->ID.id, data.type, data.parenttype, GIFT_CARDNUMBER_LEN, data.cardnumber);
	Complete(pImp);	
}

int player_giftcard::TryRedeem(gplayer_imp * pImp, const char (&cn)[GIFT_CARDNUMBER_LEN])
{
	if(!pImp->CheckCoolDown(COOLDOWN_INDEX_GIFTCARD_REDEEM) )
	{
		return GCR_WAIT_COOLDOWN;
	}

	if(data.state)
	{
		return data.state == GCR_STATE_HALFREDEEM ?	GCR_WAIT_AU : GCR_WAIT_TASK;
	}
	
	for(size_t i=0; i < GIFT_CARDNUMBER_LEN; ++i)
	{
		if(!isdigit(cn[i]))
		{
			return GCR_INVALID_CARD;
		}
	}

	data.state = GCR_STATE_HALFREDEEM;
	memcpy(data.cardnumber,cn,GIFT_CARDNUMBER_LEN);
	
	GMSV::SendPlayerGiftCodeRedeem(pImp->GetParent()->ID.id,data.cardnumber);
	SetTimeOut(GIFT_WAITREDEEM_TIME);
	
	pImp->SetCoolDown(COOLDOWN_INDEX_GIFTCARD_REDEEM,GIFTCARD_REDEEM_COOLDOWN_TIME);
	
	return GCR_SUCCESS;
}
