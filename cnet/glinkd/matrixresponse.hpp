
#ifndef __GNET_MATRIXRESPONSE_HPP
#define __GNET_MATRIXRESPONSE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "matrixfailure.hpp"

namespace GNET
{

class MatrixResponse : public GNET::Protocol
{
	#include "matrixresponse"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GLinkServer *lsm = GLinkServer::GetInstance();

		int mc_type = 0;
		bool result = true;
		SessionInfo * sinfo = lsm->GetSessionInfo(sid);
		if (sinfo && sinfo->checker)
		{
			MatrixChecker* checker = sinfo->checker;
			mc_type = checker->GetType();
			result = checker->Verify(response);
			if(!result)
			{
				MatrixFailure mf(checker->GetUid(), checker->GetIp(), 1);
				GDeliveryClient::GetInstance()->SendProtocol(mf);
			}
			else if(mc_type == MC_TYPE_PHONETOKEN)
			{
				//将验证通过的电子码保存在sessioninfo中，并不是所有密保方式都存在电子码
				checker->GetUsedElecNumber(sinfo->used_elec_number);
			}
			delete checker;
			sinfo->checker = NULL;
		}
		if(!result)
		{
			if(mc_type == MC_TYPE_PHONETOKEN)
				lsm->SessionError(sid, ERR_MATRIXFAILURE_PHONETOKEN, "Matrix check failed.");
			else
				lsm->SessionError(sid, ERR_MATRIXFAILURE, "Matrix check failed.");
			return;
		}

		lsm->ChangeState(sid,&state_GKeyExchgSend);
		KeyExchange keyexchange(PROTOCOL_KEYEXCHANGE);
		keyexchange.Setup(sinfo->identity, sinfo->response, lsm, sid);
		lsm->Send(sid, keyexchange);

		Octets& username = sinfo->identity;
		Log::login(username, sinfo->userid, sid, inet_ntoa(((const struct sockaddr_in*)sinfo->GetPeer())->sin_addr));
	}
};

};

#endif
