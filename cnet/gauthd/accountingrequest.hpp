
#ifndef __GNET_ACCOUNTINGREQUEST_HPP
#define __GNET_ACCOUNTINGREQUEST_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "accntparam"

#include "security.h"
#include "kickoutuser.hpp"
#include "accountingresponse.hpp"
#include "gauthserver.hpp"
namespace GNET
{
class AccountingRequest : public GNET::Protocol
{
	#include "accountingrequest"
	bool need_response;
	bool VerifyAuth()
	{
		return (MD5Hash(MD5HASH).Digest(OctetsStream()<<stamp<<userid<<attributes.size()<<GAuthServer::GetInstance()->shared_key) == authenticator);
	}
	bool SendResponse(Manager::Session::ID sid)
	{
		OctetsStream buf;
		buf<<stamp<<userid<<authenticator<<GAuthServer::GetInstance()->shared_key;
		return GAuthServer::GetInstance()->Send(sid,AccountingResponse(stamp,userid,MD5Hash(MD5HASH).Digest(buf)));
	}
	bool  Write2DB()
	{
		return 1;
	}
	void GenerateFee(AccntParam& attribute)
	{
		GAuthServer* aum=GAuthServer::GetInstance();
		Thread::Mutex::Scoped (aum->locker_accntmap);
		need_response=false;
		switch (attribute.type)
		{
			case _ACCOUNT_START:
				aum->accntmap[userid]=attribute.value;
				need_response=true;
				DEBUG_PRINT("\tgauthd::start accounting user %d,time=%d.\n",userid,attribute.value);
				break;
			case _ACCOUNT_STOP:
				if (aum->accntmap.find(userid)!=aum->accntmap.end())
				{
					unsigned int elapse=attribute.value - aum->accntmap[userid];
					DEBUG_PRINT("\tgauthd::stop accounting user %d. He plays %d hours %d min %d second\n",userid,elapse / 3600,(elapse % 3600) / 60,elapse % 60);
					aum->accntmap.erase(userid);
				}
				need_response=true;
				break;
			case _ACCOUNT_ELAPSE_TIME:
				if (aum->accntmap.find(userid)!=aum->accntmap.end())
				{
					unsigned int elapse=attribute.value - aum->accntmap[userid];
					DEBUG_PRINT("\tgauthd::continue accounting user %d. He plays %d hours %d min %d second\n",userid,elapse / 3600,(elapse % 3600) / 60,elapse % 60);
				}
				else
				{
					DEBUG_PRINT("\tgauthd::warning:: _ACCOUNT_ELAPSE_TIME cannot find start. Fee leaks.\n");
					aum->accntmap[userid]=attribute.value;
				}
				break;
			case _ACCOUNT_IN_OCTETS:
				break;
			case _ACCOUNT_OUT_OCTETS:
				break;
			case _ACCOUNT_IN_PACKETS:
				break;
			case _ACCOUNT_OUT_PACKETS:
				break;
		}
	}
	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		if (!VerifyAuth()) return;  //silently discard the message
		for (size_t i=0;i<attributes.size();GenerateFee(attributes[i]),i++);
		if (need_response && Write2DB()) SendResponse(sid);
		if (!GAuthServer::GetInstance()->ValidUser(sid,userid))
			GAuthServer::GetInstance()->Send(sid,KickoutUser(userid,_SID_INVALID));
	}
};

};

#endif
