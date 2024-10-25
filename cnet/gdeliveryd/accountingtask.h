#ifndef __GNET_ACCOUNTINGTASK_H
#define __GNET_ACCOUNTINGTASK_H
#include "protocol.h"

#include "accountingrequest.hpp"
#include "gdeliveryserver.hpp"
#include "gauthclient.hpp"

#include "macros.h"
#include <algorithm>
#include <functional>

#include "log.h"
#include "maperaser.h"
#include "mapremaintime.h"
#include "mapfeeleak.h"
#include "mapuser.h"
#include "kickoutuser.hpp"
namespace GNET
{
class AccountingTask : public Thread::Runnable
{
	static unsigned int update_interval;
	AccountingTask(unsigned int interval,int priority=1): Runnable(priority) { update_interval=interval; }
	static void SendAccountingInfo(time_t now, int userid, UserInfo& ui,int freecreatime)
	{
		if ((ui.status==_STATUS_ONGAME || ui.status==_STATUS_ONLINE || ui.status==_STATUS_READYGAME )
				&& ((unsigned int)(now-ui.last_accounting_time))>=update_interval)
		{
			if( freecreatime != ~0 && freecreatime < ui.creatime ) return;
			AccountingRequest accnt_rqst;
			unsigned int account_type=((unsigned int)(now-ui.last_sync_time))>=update_interval*3 ?
				 (unsigned int) _ACCOUNT_SYNC_TIME : (unsigned int)_ACCOUNT_ELAPSE_TIME;
			accnt_rqst.Setup(userid,account_type,(unsigned int)now);
			GAuthClient::GetInstance()->SendProtocol(&accnt_rqst);
			if ( account_type==(unsigned int) _ACCOUNT_SYNC_TIME )
				ui.last_sync_time=now;
			ui.last_accounting_time=now;
			if ( RemainTime::GetInstance().IsUserInFreeState(userid) )
			{
				RemainTime::GetInstance().UpdateFreeTime( userid, update_interval );
			}
			else	
			{
				RemainTime::GetInstance().UpdateRemainTime( userid, update_interval );
				//if remain_time<=0, kickout the user
			   	if ( RemainTime::GetInstance().GetPlayTime(userid).remain_time<=0 )
				{
					GDeliveryServer::GetInstance()->Send(
							ui.sid,
							KickoutUser(userid,ui.localsid,ERR_ACCOUNTEMPTY)
						);
				}
			}	
		}
	}
	static bool ResendAccountingRequest(time_t now,std::pair<const unsigned int, Protocol*> pair)
	{
		unsigned int send_time=((AccountingRequest*)pair.second)->attributes[0].value;
		int userid=((AccountingRequest*)pair.second)->userid;
		int type=((AccountingRequest*)pair.second)->attributes[0].type;
		if (((size_t)now)-send_time < update_interval*3/5)
		{
			GAuthClient::GetInstance()->SendProtocol(pair.second);
			return true;
		}
		else
		{
			Log::log(LOG_ERR,"gdelivery::resend time surmounts 3, discard the message. userid=%d,type=%d,timelast=%d,update_interval*3/5=%d\n",userid,type,((size_t)now)-send_time,update_interval*3/5);
			//bw  原有log接口被删除
			//Log::accounting(userid,type);
			
			FeeLeak::GetInstance().Insert( userid );

			//((AccountingRequest*)pair.second)->Destroy();
			//pair.second=NULL;
			//GAuthClient::GetInstance()->accntmap.erase(pair.first);

			return false;
		}
	}
public:
	
	static AccountingTask* GetInstance(signed int interval,int priority=1) 
	{
		static AccountingTask instance(interval,priority); 
		return &instance;
	}
	~AccountingTask() {}

	class QueryUser : public UserContainer::IQueryUser
	{   
	public:
		int freecreatime;
		bool Update( int userid, UserInfo & info )
		{
			AccountingTask::SendAccountingInfo(Timer::GetTime(),userid,info,freecreatime);
			return true;
		}
	};

	void Run()
	{
		AccountingRequest accnt_rqst;
		time_t now=time(NULL);
		{
			QueryUser	q;
			q.freecreatime = GDeliveryServer::GetInstance()->freecreatime;
			UserContainer::GetInstance().WalkUser( q );
		}
		STAT_MIN5("OnlineUsers",UserContainer::GetInstance().Size());
		{
			GAuthClient* aum=GAuthClient::GetInstance();
			Thread::RWLock::WRScoped l(aum->locker_accntmap);
			MapEraser<GAuthClient::AccountingMap> del_keys(aum->accntmap);
			for (GAuthClient::AccountingMap::iterator it=aum->accntmap.begin();it!=aum->accntmap.end(); ++it)
			{
				if (!ResendAccountingRequest(now,*it))
				{
					(*it).second->Destroy();
					(*it).second=NULL;
					del_keys.push((*it).first);
				}
			}
			//for_each(aum->accntmap.begin(),aum->accntmap.end(),bind1st(ptr_fun(&AccountingTask::ResendAccountingRequest),now));
		}
		PollIO::WakeUp();
		Thread::HouseKeeper::AddTimerTask(this,update_interval/5);
	}

};
unsigned int AccountingTask::update_interval=0;
};
#endif
