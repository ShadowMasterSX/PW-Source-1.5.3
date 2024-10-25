#include "factionlib.h"
#include "factiondata.hxx"
#include "ids.hxx"


namespace GNET
{
	//Octets GenerateOperParams(int optype,const Octets& clientParams,object_interface obj_if,bool& retcode)
	Octets GenFOperParams(int optype,const Octets& clientParams,bool& retcode)
	{
		retcode=true;
		try
		{
		switch (optype)
		{
		case _O_FACTION_CREATE:
			return create_param_st(
					10,//obj_if.GetBasicProp().level/*level*/,
					53120,//obj_if.GetMoney()/*money*/,
					1000,//obj_if.GetBasicProp().skill_point/*sp*/,
					create_param_ct().Create(clientParams)
					).marshal();
		case _O_FACTION_CHANGEPROCLAIM:
		case _O_FACTION_ACCEPTJOIN:
		case _O_FACTION_EXPELMEMBER:
		case _O_FACTION_APPOINT:
		case _O_FACTION_MASTERRESIGN:
		case _O_FACTION_RESIGN:
		case _O_FACTION_LEAVE:
		case _O_FACTION_BROADCAST:
		case _O_FACTION_DISMISS:
		case _O_FACTION_UPGRADE:
		case _O_FACTION_DEGRADE:
			return clientParams;	
		default:
			retcode=false;
			return Octets();
		}
		}
		catch (Marshal::Exception )
		{
			retcode=false;
			return Octets();
		}
	}
};
