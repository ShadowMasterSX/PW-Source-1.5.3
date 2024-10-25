#ifndef __GNET_OPRESULT_H
#define __GNET_OPRESULT_H
#include "factionid"
#include "addfactionres"
#include "deffactionres"
#include "addmemberres"
#include "userfactionres"
#include "delfactionres"
namespace GNET
{
	struct create_result_t
	{
		int retcode;
		unsigned int fid_new;
		create_result_t() : retcode(0),fid_new(0) { }
		create_result_t(int _r, unsigned int _f) : retcode(_r) , fid_new(_f) { }
	};
};
#endif
