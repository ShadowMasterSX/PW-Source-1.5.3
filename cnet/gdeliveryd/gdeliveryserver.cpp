#include "state.hxx"

#include "gdeliveryserver.hpp"
#include "gauthclient.hpp"
#include "gproviderserver.hpp"

#include "onlineannounce.hpp"
#include "kickoutuser.hpp"
#include "statusannounce.hpp"
#include "userlogout.hrp"
#include "playeroffline.hpp"
#include "tradediscard.hpp"
#include "trade.h"
#include <algorithm>
#include <functional>

#include "maperaser.h"
#include "protocolexecutor.h"

#include "mapforbid.h"
#include "maplinkserver.h"
#include "mapuser.h"
#include "timer.h"
#include "announceserverattribute.hpp"
#include "announcechallengealgo.hpp"
#include "announceauthdversion.hpp"
namespace GNET
{
using namespace std;

GDeliveryServer GDeliveryServer::instance;
const Protocol::Manager::Session::State* GDeliveryServer::GetInitState() const
{
	return &state_GDeliverServer;
}

void GDeliveryServer::OnAddSession(Session::ID sid)
{
	LinkServer::GetInstance().Insert( sid );
	// announce server attribute
	Send( sid,AnnounceServerAttribute(serverAttr.GetAttr(), freecreatime, serverAttr.GetExpRate()) );
	Send( sid,AnnounceChallengeAlgo(challenge_algo) );
	Send( sid,AnnounceAuthdVersion(GAuthClient::GetInstance()->GetVersion()));
}

class DeliveryServerQueryUser : public UserContainer::IQueryUser
{ 
public:
	int freecreatime;
	Protocol::Manager::Session::ID	sid;
	bool Update( int userid, UserInfo & ui )
	{
		if (ui.linksid==sid)
		{
			//discard trade object
			GDeliveryServer* gdsm=GDeliveryServer::GetInstance();
			GNET::Transaction* t;
			if (ui.status == _STATUS_ONGAME && (t=GNET::Transaction::FindTransactionbyTrader(ui.roleid))!=NULL)
			{
				ProtocolExecutor* task=new ProtocolExecutor( gdsm,ui.linksid,new TradeDiscard(t->GetTid(),ui.roleid,ui.localsid) );
				Thread::Pool::AddTask(task);   			
			}

			//announce user offline to game
			GAuthClient::GetInstance()->SendProtocol( Rpc::Call(RPC_USERLOGOUT,UserLogoutArg(userid,ui.localsid)));
			if (ui.switch_gsid==_GAMESERVER_ID_INVALID)//没有切换服务器
			{
				PlayerOffline po(ui.roleid,ui.linkid,ui.localsid);
				if (GProviderServer::GetInstance()->DispatchProtocol(ui.gameid,po))
				{
					ForbiddenUsers::GetInstance().Push(userid,ui.roleid,ui.status);
				}
				return false;
			}
		}
		return true;
	}
};

void GDeliveryServer::OnDelSession(Session::ID sid)
{
	LinkServer::GetInstance().Erase( sid );

	//erase all users that belongs to this linkserver. To prevent accounting error
	//这里只做简单的下线处理，即给Au发送下线信息，防止多计费，同时将用户从map里删除，
	//发送playeroffline给game，并把用户放入forbidset中(如果用户正在切换服务器，则不做这个操作)
	//取消交易

	if (sid != iweb_sid)
	{
		Log::log(LOG_ERR,"Disconnect from linkserver. sid=%d\n",sid);
		DeliveryServerQueryUser	q;
		q.sid = sid;
		q.freecreatime = freecreatime;
		UserContainer::GetInstance().DeleteWalkUser( q );
	}
	else
	{
		iweb_sid = _SID_INVALID;
	}

	RemoveLinkType(sid);
}

void GDeliveryServer::BroadcastStatus()
{
	static time_t timer = 0;
	time_t now = Timer::GetTime();

	if(now - timer < 10) return;
	timer = now;
	
	UserContainer& container = UserContainer::GetInstance();
	unsigned int _load=(unsigned int)( (double)(container.Size()*200) / (double)(container.GetFakePlayerLimit()) );
	
	if ( _load > 200 ) _load = 200;
	serverAttr.SetLoad((unsigned char)_load);
	LinkServer::GetInstance().BroadcastProtocol(AnnounceServerAttribute(serverAttr.GetAttr(), freecreatime, serverAttr.GetExpRate()));

	DEBUG_PRINT("gdeliveryserver::statusannounce,online=%d,fakemax=%d,load=%d,attr=%d\n",
		container.Size(), container.GetFakePlayerLimit(),(unsigned char)_load,serverAttr.GetAttr());
}

void StatusAnnouncer::Run()
{       
	GDeliveryServer* dsm = GDeliveryServer::GetInstance();
	UserContainer& container = UserContainer().GetInstance();
	unsigned int _load = (unsigned int)((double)(container.Size()*200)/(double)(container.GetFakePlayerLimit()));
	if ( _load>200 )
		_load=200;
	dsm->serverAttr.SetLoad((unsigned char)_load);
	LinkServer::GetInstance().BroadcastProtocol( AnnounceServerAttribute(dsm->serverAttr.GetAttr(),dsm->freecreatime, dsm->serverAttr.GetExpRate()));
	DEBUG_PRINT("gdeliveryserver::statusannounce,online=%d,fakemax=%d,load=%d,attr=%d\n",
		container.Size(), container.GetFakePlayerLimit(),(unsigned char)_load,dsm->serverAttr.GetAttr());
	Thread::HouseKeeper::AddTimerTask(this,update_time);
}


};
