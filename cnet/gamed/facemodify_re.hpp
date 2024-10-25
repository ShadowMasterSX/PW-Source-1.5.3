
#ifndef __GNET_FACEMODIFY_RE_HPP
#define __GNET_FACEMODIFY_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

void player_cosmetic_result(int user_id, int tiket_inv_index, int ticket_id, int result, unsigned int crc);

namespace GNET
{

class FaceModify_Re : public GNET::Protocol
{
	#include "facemodify_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		player_cosmetic_result(roleid,ticket_pos,ticket_id,retcode,crc);
	}
};

};

#endif
