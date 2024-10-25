/* @file: report gameserver attribute
 *   
 */  

#ifndef __GNET_GAMESERVER_ATTR_H
#define __GNET_GAMESERVER_ATTR_H
namespace GNET
{
	class Octets;
	// this functions is called in protocol "querygameattri_re" 
	void OnReportAttr( unsigned char attribute,const Octets& value );
	
	/////////////////////////////////////////////////////////
	// below functions should be implemented by GameServer //
	/////////////////////////////////////////////////////////
	void SetDoubleExp(unsigned char blOn);
	void SetNoTrade(unsigned char blOn);
	void SetNoMail(unsigned char blOn);
	void SetNoAuction(unsigned char blOn);
	void SetNoFaction(unsigned char blOn);
	void SetDoubleMoney(unsigned char blOn);
	void SetDoubleObject(unsigned char blOn);
	void SetDoubleSP(unsigned char blOn);
	void SetNoSellPoint(unsigned char blOn);
	void SetPVP(unsigned char blOn);
}
#endif

