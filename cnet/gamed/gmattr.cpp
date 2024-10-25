#include "gmattr.h"
#include "marshal.h"
#include "querygameserverattr.hpp"
#include "gproviderclient.hpp"
#include "privilege.hxx"

#define PARSE_SWITCH_VALUE \
Marshal::OctetsStream os(value);\
unsigned char blOn=0;\
os >> blOn;

namespace GNET
{
	//set gameserver attribute
	void OnReportAttr( unsigned char attribute,const Octets& value )
	{
		try
		{
			switch ( attribute )
			{
			case Privilege::PRV_DOUBLEEXP:
				{
					PARSE_SWITCH_VALUE;
					SetDoubleExp(blOn);	//这里的blon已经改成表示经验调整比例的10倍了
				}
				break;
			case Privilege::PRV_NOTRADE:
				{
					PARSE_SWITCH_VALUE;
					SetNoTrade(blOn);
				}
				break;	
			case Privilege::PRV_NOAUCTION:
				{
					PARSE_SWITCH_VALUE;
					SetNoAuction(blOn);
				}
				break;	
			case Privilege::PRV_NOMAIL:
				{
					PARSE_SWITCH_VALUE;
					SetNoMail(blOn);
				}
				break;	
			case Privilege::PRV_NOFACTION:
				{
					PARSE_SWITCH_VALUE;
					SetNoFaction(blOn);
				}
				break;	
			case Privilege::PRV_DOUBLEMONEY:
				{
					PARSE_SWITCH_VALUE;
					SetDoubleMoney(blOn);
				}
				break;
			case Privilege::PRV_DOUBLEOBJECT:
				{
					PARSE_SWITCH_VALUE;
					SetDoubleObject(blOn);
				}
				break;
			case Privilege::PRV_DOUBLESP:
				{
					PARSE_SWITCH_VALUE;
					SetDoubleSP(blOn);
				}
				break;
			case Privilege::PRV_NOSELLPOINT:
				{
					PARSE_SWITCH_VALUE;
					SetNoSellPoint(blOn);
				}
				break;
			case Privilege::PRV_PVP:
				{
					PARSE_SWITCH_VALUE;
					SetPVP(blOn);
				}
				break;
			}//end of switch
		}
		catch ( Marshal::Exception e )
		{
		}
	}
}
