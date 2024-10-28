
#include "glinkserver.hpp"
#include "state.hxx"

#include "gdeliveryclient.hpp"
#include "gproviderserver.hpp"
#include "challenge.hpp"
#include "statusannounce.hpp"
#include "errorinfo.hpp"
#include "acprotostat.hpp"
#include "matrixfailure.hpp"

#include "macros.h"
#include "maperaser.h"
#include <vector>
#include "callid.hxx"
namespace GNET
{

GLinkServer GLinkServer::instance;

const Protocol::Manager::Session::State* GLinkServer::GetInitState() const
{
	return &state_GLoginServer;
}

void GLinkServer::OnSetTransport(Session::ID sid, const SockAddr& local, const SockAddr& peer)
{
	Iterator it = sessions.find(sid);
	if (it != sessions.end())
	{
		it->second.SetLocal(local);
		it->second.SetPeer(peer);
	}
}

void GLinkServer::OnAddSession(Session::ID sid)
{
	Challenge challenge;
	challenge.Setup(16);
	*(unsigned int*)challenge.nonce.begin()=GetServerAttr();
	*((int*)challenge.nonce.begin()+1)=GetFreeCreatime();
	challenge.version=version;
	challenge.algo=challenge_algo;
	challenge.edition = edition;
	challenge.exp_rate = GetExpRate();
	Send(sid, challenge);
	sessions[sid].challenge = challenge.nonce;
	sessions[sid].policy.Initialize();
	SetVerbose(sid, LOG_DEBUG);
}
void GLinkServer::AddChecker(Session::ID sid, MatrixChecker * checker)
{
	Iterator it = sessions.find(sid);
	if (it != sessions.end())
	{
		SessionInfo & info = it->second;
		if (info.checker) 
			delete info.checker;
		info.checker = checker;
	}
}

void GLinkServer::OnDelSession(Session::ID sid, int status)
{
	unsigned char not_ttl=1;
	not_ttl=alivetimemap.erase(sid);

	Iterator it=sessions.find(sid);
	if(it==sessions.end())
		return;
	SessionInfo & info = it->second;
	int userid = info.userid;
	int roleid = info.roleid;
	time_t now = time(NULL);

	if(userid>0)
	{
		char strpeer[256];
		strcpy( strpeer,inet_ntoa(((struct sockaddr_in*)info.GetPeer())->sin_addr) );
		Log::formatlog("logout","account=%.*s:userid=%d:sid=%d:peer=%s:time=%d:status=0x%x",
				info.identity.size(), (char*)info.identity.begin(), userid, sid, strpeer, now-info.login_time, status);
	}

    if (roleid > 0)
    {
#ifdef ANTI_TWIN_SYSTEM
        if (info.m_IsWindowRegistered == true)
        {
        	Thread::Mutex::Scoped l(m_WindowsMapMutex);
            uint32_t src_id = info.gsid;
            //if (info.gsid == 84 || info.gsid == 85 || info.gsid == 86) //Special for Dynasty Battle;
                //src_id = 83;
            WindowsMap::iterator it = m_WindowsMap.find(src_id);
            if (it != m_WindowsMap.end())
            {
                uint64_t hash = info.m_MachineHash;
                WindowPog& srcInst = it->second;
                //m_WindowsMapMutex.Lock();
                WindowPog::iterator instIt = srcInst.find(hash);
                //m_WindowsMapMutex.UNLock();
                if (instIt != srcInst.end())
                {
                    //m_WindowsMapMutex.Lock();
                    --srcInst[hash];
                    //m_WindowsMapMutex.UNLock();
                    info.m_IsWindowRegistered = false;
                    printf("Delete window! count=%d, inst_id=%d, sid=%d\n", srcInst[hash], info.gsid, sid);
                }
            }
        }
#endif //ANTI_TWIN_SYSTEM
    }

	if (info.IsOnlineSave)
	{
		return;
	}

	if (info.checker)
	{
		MatrixFailure mf(info.checker->GetUid(), info.checker->GetIp(), 1);
		GDeliveryClient::GetInstance()->SendProtocol(mf);
		delete 
			info.checker;
		info.checker = NULL;
	}
	halfloginset.erase(sid);
	sessions.erase(sid);
	if (roleid>0)   
	{
		RoleMap::iterator itui = roleinfomap.find(roleid);
		if ( itui!=roleinfomap.end() )
		{
			RoleData& ui = itui->second;
			if (IsInSwitch(ui)) 
				PopSwitchUser(ui);
			Log::formatlog("rolelogout","userid=%d:roleid=%d:localsid=%d:time=%d",
					ui.userid, ui.roleid, ui.sid, GNET::Timer::GetTime()-ui.login_time);
			if (ui.sid != sid)
				return;
			roleinfomap.erase(itui);
		}
	}

	if ( !not_ttl )
		DEBUG_PRINT("GLinkd::OnDelSession: Session Closed for TTL expired. userid=%d,sid=%d\n",userid,sid);

	if(userid)
		GDeliveryClient::GetInstance()->SendProtocol(StatusAnnounce(userid, sid, _STATUS_OFFLINE));

	TriggerListen();
}

void GLinkServer::SessionError(Session::ID sid, int errcode, const char *info)
{
	Send(sid, ErrorInfo(errcode, Octets(info,strlen(info))));
	ChangeState(sid, &state_Null);
	ActiveCloseSession( sid );
}

void GLinkServer::ActiveCloseSession( Session::ID sid )
{
	OnDelSession(sid);
}
bool GLinkServer::IsRoleOnGame( Session::ID sid )
{
	Iterator it = instance.sessions.find(sid);
	if (it == instance.sessions.end() || it->second.roleid == 0 || it->second.gsid == 0)
		return false;
	return true;
}
bool GLinkServer::IsRoleOnGame( int roleid )
{
	RoleMap::iterator it = instance.roleinfomap.find(roleid);
	return (it!=instance.roleinfomap.end()) && it->second.status==_STATUS_ONGAME;
}
void GLinkServer::SendAccumProto4Switch(const RoleData& ui,int gs_id)
{
	if (!IsInSwitch(ui)) return;
	ProtocolQueue& proto_queue = accumproto_map[ui.sid];
	for (size_t i=0; i<proto_queue.size();i++)
	{
		GProviderServer::GetInstance()->DispatchProtocol(gs_id,proto_queue[i]);
		proto_queue[i]->Destroy();
	}
	proto_queue.clear();
	PollIO::WakeUp();
}
///////////////////////////////////////////////////////////////////////////////////
//Check keepalive map and shutup user map periodically, and remove timeout items //
///////////////////////////////////////////////////////////////////////////////////

void CheckTimer::CheckConnection()
{
	GLinkServer* lsm=GLinkServer::GetInstance();
	std::vector<unsigned int> sid_vector;
	{
		MapEraser<GLinkServer::AlivetimeMap> del_keys_alive(lsm->alivetimemap);
		for (GLinkServer::AlivetimeMap::iterator it=lsm->alivetimemap.begin();it!=lsm->alivetimemap.end();it++)
		{
			(*it).second-=update_time;
			if ((*it).second<=0)
			{
				DEBUG_PRINT("glinkserver::AliveKeeper:: session %d's TTL is expired(%d). session closed.\n",(*it).first,(*it).second);
				lsm->Close((*it).first);
				sid_vector.push_back( (*it).first );
				del_keys_alive.push( it );
			}
		}
		MapEraser<GLinkServer::ReadyCloseMap> del_keys_close(lsm->readyclosemap);
		for (GLinkServer::ReadyCloseMap::iterator it=lsm->readyclosemap.begin();it!=lsm->readyclosemap.end();it++)
		{
			(*it).second-=update_time;
			if ((*it).second<=0)
			{
				DEBUG_PRINT("glinkserver::Ready Close:: session %d's is expired(%d). session closed.\n",(*it).first,(*it).second);
				lsm->Close((*it).first);
				sid_vector.push_back( (*it).first );
				del_keys_close.push( it );
			}
		}
	}
	for ( size_t i=0;i<sid_vector.size();++i )
		lsm->ActiveCloseSession( sid_vector[i] ); // active announce delivery server
}
void CheckTimer::CheckForbid()
{
	GLinkServer* lsm=GLinkServer::GetInstance();
	{
		MapEraser<GLinkServer::MuteMap> del_keys_user(lsm->shutupusermap);
		for (GLinkServer::MuteMap::iterator it=lsm->shutupusermap.begin();it!=lsm->shutupusermap.end();it++)
		{
			(*it).second.time-=update_time;
			if ((*it).second.time<=0)
			{
				DEBUG_PRINT("glinkserver::ShutupKeeper:: user %d's ShutupTimer is expired.\n",(*it).first);
				del_keys_user.push( it );
			}
		}
	}
	{	
		MapEraser<GLinkServer::MuteMap> del_keys_role(lsm->shutuprolemap);
		for (GLinkServer::MuteMap::iterator it=lsm->shutuprolemap.begin();it!=lsm->shutuprolemap.end();it++)
		{
			(*it).second.time-=update_time;
			if ((*it).second.time<=0)
			{
				DEBUG_PRINT("glinkserver::ShutupKeeper:: role %d's ShutupTimer is expired.\n",(*it).first);
				del_keys_role.push( it );
			}
		}
	}
}
void CheckTimer::CheckProtoStat()
{
	GLinkServer* lsm=GLinkServer::GetInstance();
	GLinkServer::SessionInfoMap::iterator it=lsm->sessions.begin(),ite=lsm->sessions.end();
	ACProtoStat acprotostat;
	for ( ;it!=ite;++it )
	{
		SessionInfo& info=(*it).second;
		if ( info.roleid && (info.protostat.remain_time-=update_time)<=0 )
		{
			acprotostat.roleid		=  info.roleid;
			acprotostat.keepalive		=  info.protostat.keepalive;
			acprotostat.gamedatasend	=  info.protostat.gamedatasend;
			acprotostat.publicchat		=  info.protostat.publicchat;
			info.protostat.remain_time	=  ACREPORT_TIMER;
			GDeliveryClient::GetInstance()->SendProtocol( acprotostat );
		}
	}
}	
void CheckTimer::Run()
{
	CheckConnection();
	CheckForbid();
	CheckProtoStat();
	Thread::HouseKeeper::AddTimerTask(this,update_time);
}
void GLinkServer::TriggerListen()
{       
	if (!IsListening() && UnderHalfloginLimit(halfloginset.size()) && UnderUserLimit(roleinfomap.size()))
	{

		DEBUG_PRINT("glinkd::start passive listen: halfloginuser is %d,total user is %d\n",
				halfloginset.size(),roleinfomap.size());
		StartListen();
	}
}       
void GLinkServer::RoleLogout(int roleid)
{
	RoleMap::iterator it = roleinfomap.find(roleid);
	if (it == roleinfomap.end())
		return;
	RoleData& ui = it->second;
	Log::formatlog("rolelogout","userid=%d:roleid=%d:localsid=%d:time=%d",ui.userid, ui.roleid, ui.sid, GNET::Timer::GetTime()-ui.login_time);
	roleinfomap.erase(roleid);
}       
void GLinkServer::RoleLogin(Session::ID localsid, int roleid, int gsid, ByteVector& auth)
{       
	SessionInfo * sinfo = GetSessionInfo(localsid);
	if(!sinfo)
		return;
	sinfo->m_ClientCheck = true;
	sinfo->IsOnlineSave = true;
	sinfo->roleid = roleid;
	sinfo->gsid   = gsid;
	RoleData uinfo(localsid, sinfo->userid, roleid, gsid);
	uinfo.login_time = GNET::Timer::GetTime();
	roleinfomap[roleid] = uinfo;
	ChangeState(localsid,&state_GReadyGame);

	GetUserPrivilege(sinfo->userid, auth);
	Log::formatlog("rolelogin","userid=%d:roleid=%d:lineid=%d:localsid=%d",sinfo->userid, roleid, gsid, localsid);
}

};
