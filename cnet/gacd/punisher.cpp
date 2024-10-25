
#include "punisher.hpp"
#include "gmkickoutuser.hpp"
#include "ganticheaterserver.hpp"
#include "userdatamanager.hpp"
#include "conv_charset.h"
#include "gacdutil.h"

namespace GNET
{

void KickPunisher::Process() const
{
	DEBUG_PRINT_PUNISH("kick %d, %d secs, for %s\n", m_iUserID, m_iForbidTime, m_reason.c_str());
    GMKickoutUser pro;
    pro.gmroleid = 1984;
    pro.kickuserid = USERID2ACCOUNTID(m_iUserID);
    pro.forbid_time = m_iForbidTime;
	//
	Octets uni_reason;
	
	CharsetConverter::conv_charset_g2u(Octets(m_reason.data(), m_reason.size()), uni_reason);
    pro.reason.resize(4);

	if( m_bForbid )
	    memset(pro.reason.begin(),0,4); // gmkickoutuser from gacd
	else
		*(int*)(pro.reason.begin()) = 0x01000000; // gmkickoutuser from gacd without forbid

    pro.reason.insert(pro.reason.end(), uni_reason.begin(), uni_reason.size());
    GAntiCheaterServer::GetInstance()->SendProtocol(pro);
}

};

