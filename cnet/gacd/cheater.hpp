#ifndef __GNET_CHEATER_HPP
#define __GNET_CHEATER_HPP


namespace GNET
{

struct Cheater 
{
	enum {

        CH_NOTHING				=		0,
        CH_LOGINOUT				=		1, // not cheater
        CH_FORBID_STRING		= 		6,
        //CH_RESERVED1			=		8,
        //CH_RESERVED2			= 		9,
        CH_FORBID_MEM_PATTERN	=		10,
        //CH_RESERVED3			= 		11,
        CH_ACPROTO_DECODE_ERR 	= 		103,
        CH_CLIENTINFO_FREQ_ERR  = 		104,
        CH_CLIENTINFO_ABNORMAL 	= 		105,
        CH_CLIENTINFO_ORDER_ERR = 		106,
        CH_GAMEDATA_FREQ_ERR 	= 		107,
        CH_SPEC_TIMEOUT_CODE 	= 		108,
        CH_FLASH_LOG 			= 		109,
        CH_BAOSHI_USER			= 		110,
        CH_CODE_UNKNOWN 		= 		201,
        CH_CODE_TIMEOUT 		= 		203,
        //CH_RESERVED4			=		204,
        CH_CODE_REMAIN_ON_QUIT	= 		205,
        CH_VIP_USER 			=		256,
        CH_MULTI_LOGIN			=		300,
        CH_MULTI_LOGOUT			=		301,
        CH_NO_CODE_RES			=		400,
        //CH_OVERSPEED_CODE_RES	=		401,
        CH_OVERSPEED_CODE_RES	=		402
    };

	int m_iUserID;
	int m_iCheatID;
	int m_iSubID;
	int m_iTimes;

	int GetCheatID() const { return m_iCheatID; }
	int GetLogType() const { return m_iCheatID; }
	int GetUserID()  const { return m_iUserID;  }
	int GetSubID()   const { return m_iSubID;   }
	int GetTimes()   const { return m_iTimes;   }
	unsigned int GetMaxSize() const { return 0x800; }
	unsigned int GetMaxCount() const { return 4; }

	Cheater() 
		: m_iUserID(0), m_iCheatID(0), m_iSubID(0), m_iTimes(0) { }
	Cheater(int iUserID, int iCheatID, int iSubID, int iTimes = 0) 
		: m_iUserID(iUserID), m_iCheatID(iCheatID), m_iSubID(iSubID), m_iTimes(iTimes) { }

};

};

#endif
