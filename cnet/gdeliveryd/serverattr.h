#ifndef __GNET_SERVERATTR_H
#define __GNET_SERVERATTR_H
#include "mutex.h"
namespace GNET
{
	class ServerAttr
	{
		union Attr{
			unsigned int _attr;
			struct {
				unsigned char load;
				unsigned char lambda;
				unsigned char anything;
				// attibutes
				unsigned char doubleExp:1;	// 因增加多种经验倍率，此位废弃，现使用ExpRate字段
				unsigned char doubleMoney:1;
				unsigned char doubleObject:1;
				unsigned char doubleSP:1;
				unsigned char freeZone:1;
				unsigned char bSellpoint:1;
				unsigned char bBattle:1;
				unsigned char pvp:1;
			};
		};
	public:	
		ServerAttr() : m_locker_("ServerAttr:m_locker_"), ExpRate(0) {
			m_Attr_._attr=0;
		}
		void SetLoad(unsigned char nload);
		unsigned char GetLoad();
		
		void SetLambda(unsigned char nlambda);
		unsigned char GetLambda();
		
		unsigned char GetDoubleExp();
		void SetDoubleExp( unsigned char doubleExp );
		
		unsigned char GetDoubleMoney();
		void SetDoubleMoney( unsigned char doubleMoney );
		
		unsigned char GetDoubleObject();
		void SetDoubleObject( unsigned char doubleObj );

		unsigned char GetDoubleSP();
		void SetDoubleSP( unsigned char doubleSP );

		unsigned char GetPVP();
		void SetPVP( unsigned char _pvp);

		unsigned char GetFreeZone();
		void SetFreeZone( unsigned char freeZone);
			
		unsigned char GetBattle();
		void SetBattle( unsigned char battle);
			
		unsigned char GetSellpoint();
		void SetSellpoint( unsigned char sellpoint);
			
		unsigned int GetAttr();

		void SetExpRate(unsigned char exp);
		unsigned char GetExpRate();

	private:
		Thread::Mutex m_locker_;
		Attr m_Attr_;
		unsigned char ExpRate;
	};
}
#endif
