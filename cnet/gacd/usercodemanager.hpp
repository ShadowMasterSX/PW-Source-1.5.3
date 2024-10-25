#ifndef __GNET_USERCODEMANAGER_HPP
#define __GNET_USERCODEMANAGER_HPP

#include <map>
#include <set>
#include <deque>

#include "codepiece.hpp"
#include "codeprovider.hpp"

namespace GNET
{

class CodeProviderManager
{
	SendingQueue m_rawQueue;
	typedef std::vector<CodeProvider*> ProviderVector;
	ProviderVector m_providers;
public:
	CodeProviderManager();
	void RegisterProvider(CodeProvider *pProvider) { m_providers.push_back(pProvider); }
	void OnUpdateConfig(const XmlConfig::Node *pRoot);
	bool IsEmpty() const { return m_rawQueue.size() == 0; }
	void UpdateSendingQueue(SendingQueue &queue);
};

class UserCodeManager
{
	static CodeProviderManager *m_pCodeProviderManager;
	static CodeProviderManager *GetCodeProviderManager() 
	{ 
		if( m_pCodeProviderManager == NULL ) 
			m_pCodeProviderManager = new CodeProviderManager();
		return m_pCodeProviderManager;
	}
public:
	UserSessionData *m_pUser;

	typedef std::map<short, WaitingCode> WaitingCodeMap;
	WaitingCodeMap m_waitingCodeMap;

	typedef std::map<int, std::set<int> > TimeoutCodeMap;
	TimeoutCodeMap m_timeoutCodeMap;

	/*
	struct Code
	{
		int m_iCodeType;
		int m_iCodeID;
	};
    SendingCodeQueue m_sendingCodeQueue;
	*/
	SendingQueue m_sendingQueue;

    int m_iSendCodeTick;
    int m_iSendCodeRound;
	int m_iTimeoutCodeCount;
	int m_bWelcomeCode;

	enum { CS_MIN = 10000, CS_MAX = 30000 };
	static int s_iCodeSeq;

	struct Config
	{
		int m_iTimeOut;
		int m_iWelcomeCodeID;
		int m_iWelcomeCodeTime;
		int m_iSpecTimeoutCodeRound;
		int m_iMinCodeInterval;
		int m_iMaxCodeInterval; 
		bool m_bWaitEveryCode;
		bool m_bPunishNewbie;
		int m_iMaxTimeoutCodeCount;
		int m_iVIPCodeID;

		TimeoutCodeMap m_specTimeoutCodeMap;
		std::set<int> m_ignoreResCodeSet;
		std::map<int,int> m_delayResCodeMap;

		Config() : m_iTimeOut(100), m_iWelcomeCodeID(0), m_iWelcomeCodeTime(-1),
			m_iSpecTimeoutCodeRound(0), m_iMinCodeInterval(60), m_iMaxCodeInterval(120),
			m_bWaitEveryCode(true), m_bPunishNewbie(true), m_iMaxTimeoutCodeCount(5)
				, m_iVIPCodeID(0) { }
	};

	static Config s_config;

	int GetUserID() const;

public:
	UserCodeManager(UserSessionData *pUser = NULL) : m_pUser(pUser), 
		m_iSendCodeTick(s_config.m_iMaxCodeInterval) , m_iSendCodeRound(0)
			, m_iTimeoutCodeCount(0), m_bWelcomeCode(true) { }

	static void OnUpdateConfig(const XmlConfig::Node *pRoot);
	void SetNextForbidTick();
	void Init();
	void OnTimer();
	static void UpdateCodeSeq();

	void CheckRes(int iSeq, int iRes, bool bNew);

	bool SendCode(int iCodeID, int iSeq, const CodePieceVector &cpv, CodeResChecker *pChecker);
	
	void Clear();

};

};

#endif
